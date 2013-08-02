/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef MODULES_UPLL_VTN_FLOWFILTER_ENTRY_MOMGR_HH_
#define MODULES_UPLL_VTN_FLOWFILTER_ENTRY_MOMGR_HH_

#include <string>
#include <set>
#include <list>
#include "momgr_impl.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

/* This File declares all the interface details for key type
 * KT_VTN_FLOWFILTER_ENTRY */

#define FLOWLIST 1
#define NWMONITOR 2
typedef struct val_vtn_flowfilter_entry_ctrlr {
        uint8_t                    valid[5];
        unc_keytype_configstatus_t cs_row_status;
        unc_keytype_configstatus_t cs_attr[5];
        uint8_t                    flags;
} val_vtn_flowfilter_entry_ctrlr_t;

class VtnFlowFilterEntryMoMgr: public MoMgrImpl {
  /*Private Declaration Section*/
  private:
    /* @brief Enumerator for VTN FlowFilter Entry Child(dependent) Objects */
    static unc_key_type_t Vtn_FlowFilter_entry_child[];
    /* @brief vtnflowfilter bindinfo Array, Binding to MAINTBL */
    static BindInfo       vtn_flowfilter_entry_bind_info[];
    static BindInfo       vtnflowfilterentrymaintbl_bind_info[];
    static BindInfo       vtnflowfilterentryctrlrtbl_bind_info[];
    /* @brief vtnflowfilter controllerbindinfo array, Binding to CTRLRTBL */
    static BindInfo       vtn_flowfilter_entry_ctrlr_bind_info[];

    /**
     * @brief  Member Variable for FlowListRenameBindInfo.
     */
    static BindInfo vtn_flowlist_rename_bind_info[];

 public:
    /**
    * @brief  This API is used to verify the existance of
             object  in Candidate configuration

    * @param[in] ikey     Pointer to ConfigKeyVal Class.
    * @param[in] dt_type  Configuration information
    * @param[in] dmi      Pointer to DalDmlIntf Class.
    *                     for flowlist or NetworkMonitor
    *
    * @retval  UPLL_RC_SUCCESS  Successful completion.
    * @retval  UPLL_RC_ERR      Specific Error code.
    * */
    upll_rc_t IsReferenced(ConfigKeyVal *ikey,
                        upll_keytype_datatype_t dt_type,
                        DalDmlIntf *dmi);
    /**
     * @brief  Method to compare to keys
     *
     * @param[in]  key1  Pointer to key structure for comparision
     * @param[in]  key2  Pointer to key for comparision
     *
     * @return     TRUE  Successfull completion.
     */

    bool CompareKey(void *key1, void *key2);


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
     * @brief  Method to check validity of Key
     *
     * @param[in]   ConfigKeyVal  input COnfigKeyVal
     * @param[out]  index          DB Column Index
     *
     * @return  TRUE   Success
     * @retval  FALSE  Failure
     **/
    bool IsValidKey(void *key,
                    uint64_t index);
    /**
    * @Brief Validates the syntax of the specified key and value structure
    *        for KT_VTN_FLOWFILTER_ENTRY keytype
    *
    * @param[in] IpcReqRespHeader contains first 8 fields of input request structure
    * @param[in] ConfigKeyVal key and value structure.
    *
    * @retval UPLL_RC_SUCCESS              Successful.
    * @retval UPLL_RC_ERR_CFG_SYNTAX       Syntax error.
    * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE key_vtn_flowfilter_entry is not available.
    * @retval UPLL_RC_ERR_GENERIC          Generic failure.
    * @retval UPLL_RC_ERR_INVALID_OPTION1  option1 is not valid.
    * @retval UPLL_RC_ERR_INVALID_OPTION2  option2 is not valid.
    */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *key);

    /**
    * @Brief Validates the syntax for KT_VTN_FLOWFILTER_ENTRY keytype key structure.
    *
    * @param[in] key_vtn_flowfilter_entry KT_VTN_FLOWFILTER_ENTRY key structure.
    * @param[in] operation                operation type.
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */
    upll_rc_t ValidateVtnFlowfilterEntryKey(
      key_vtn_flowfilter_entry_t *key_vtn_flowfilter_entry,
      unc_keytype_operation_t operation);

    /**
    * @Brief Validates the syntax for KT_VTN_FLOWFILTER_ENTRY key structure.
    *
    * @param[in] ConfigKeyVal      contains key and value structure.
    * @param[in] IpcReqRespHeader  contains first 8 fields of input.
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */
    upll_rc_t ValidateVtnFlowfilterEntryValue(ConfigKeyVal *key,
                                              IpcReqRespHeader *req,
                                              DalDmlIntf *dmi = NULL);

    /**
    * @Brief Checks if the specified key type(KT_VTN_FLOWFILTER_ENTRY) and
    *        associated attributes are supported on the given controller,
    *        based on the valid flag
    *
    * @param[in] IpcReqRespHeader  contains first 8 fields of input
    *                              request structure
    * @param[in]  ConfigKeyVal     contains key and value structure.
    * @param[in] crtlr_name        Controller name.
    *
    * @retval  UPLL_RC_SUCCESS             Validation succeeded.
    * @retval  UPLL_RC_ERR_GENERIC         Validation failure.
    * @retval  UPLL_RC_ERR_INVALID_OPTION1 Option1 is not valid.
    * @retval  UPLL_RC_ERR_INVALID_OPTION2 Option2 is not valid.
    */
    upll_rc_t ValidateCapability(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, const char* ctrlr_name = NULL);
    /**
    * @Brief Checks if the specified key type and
    *        associated attributes are supported on the given controller,
    *        based on the valid flag.
    *
    * @param[in] val_vtn_flowfilter_entry_t KT_FLOWFILTER_ENTRY value structure.
    * @param[in] attrs                      pointer to controller attribute
    *
    * @retval UPLL_RC_SUCCESS                    validation succeeded.
    * @retval UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR Attribute NOT_SUPPORTED.
    * @retval UPLL_RC_ERR_GENERIC                Generic failure.
    */
    upll_rc_t ValVtnFlowfilterEntryAttributeSupportCheck(
      val_vtn_flowfilter_entry_t *val_vtn_flowfilter_entry,
      const uint8_t* attrs);

    /**
    * @Brief Method used to get the Bind Info Structure for Rename Purpose.
    *
    * @param[in]  key_type  Describes the KT Information.
    * @param[in]  nattr     Describes the Tbl For which the Operation is
    *                       Targeted.
    * @param[in]  tbl       Describes the Table Information
    *
    * @retval  pfc_true   Successful Completion.
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
    * @retval  UPLL_RC_SUCCESS      Successfull Completion.
    * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
    */
     upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
                              ConfigKeyVal *ikey);
    /**
     * @brief Method To Compare the Valid Check of Attributes
     *
     * @param[out]  val1   Pointer to ConfigKeyVal Class which
     *                     contains only Valid Attributes
     * @param[in]   val2   Pointer to ConfigKeyVal Class.
     * @param[in]   audit  If true,Audit Process
     *
     * @retval UPLL_RC_SUCCESS;
     */
     bool  CompareValidValue(void *&val1, void *val2, bool audit);
     upll_rc_t SetConsolidatedStatus(ConfigKeyVal *ikey, DalDmlIntf *dmi);

 public:
  /* @brief Constructor*/
  VtnFlowFilterEntryMoMgr();

  /* @brief Destructor*/
  ~VtnFlowFilterEntryMoMgr() {
    for (int i = 0; i < ntable; i++) {
      if (table[i]) {
        delete table[i];
      }
    }
    delete[] table;
  }
    /**
    * @brief  This API is used to retrive the configuration details.
    *
    * @param[in] req        Pointer to IpcReqResponderHeader Class.
    * @param[in,out] ikey   Pointer to ConfigKeyVal Class.
    * @param[in] dmi        Pointer to DalDmlIntf Class.
    *
    * @retval  UPLL_RC_SUCCESS  Successfull completion.
    * @retval  UPLL_RC_ERR_GENERIC   Failure.
    *  * */

    upll_rc_t ReadMo(IpcReqRespHeader *req,
                     ConfigKeyVal *ikey, DalDmlIntf *dmi);
    /**
    *  @brief     Method used for ReadSibling Operation.
    *
    *  @param[in] req    Describes RequestResponderHeaderClass.
    *  @param[in] key    Pointer to ConfigKeyVal Class.
    *  @param[in] begin  boolean variable to decide whether to
    *                    retrieve siblings from the begin or not
    *  @param[in] dmi    Pointer to DalDmlIntf Class.
    *
    *  @retval    UPLL_RC_SUCCESS  Successfull completion.
    *  **/

    upll_rc_t ReadSiblingMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                               bool begin, DalDmlIntf *dmi);
    /**
    * @brief  This API is used to create the record  in Vtnflowfilterentry
    *         tableand also Increment the refcount or insert the record for
    *         flowlist based on the the availability in DB
    *
    * @param[in] req   Pointer to the IpcRequestResponderHeaderClass.
    * @param[in] ikey  Pointer to ConfigKeyVal Class.
    * @param[in] dmi   Pointer to DalDmlIntf(DB) Interface .
    *
    * @retval  UPLL_RC_ERR_INSTANCE_EXISTS  Record Already Exists
    * @retval  UPLL_RC_SUCCESS              Successful completion.
    * @retval  UPLL_RC_ERR_GENERIC          Generic Errors.
    * **/
    upll_rc_t CreateCandidateMo(IpcReqRespHeader *req,
              ConfigKeyVal *ikey, DalDmlIntf *dmi);

    /**
    * @brief     This API is used to delete the record from DB
    *            and decrement the refcount of Flowlist(if it is
    *            referred)based on the record availability in DB
    *
    * @param[in] req      Pointer to IpcRequestResponderHeader Class.
    * @param[in] ikey     Pointer to ConfigKeyVal Class.
    * @param[in] dmi      Pointer to DalDmlIntf Class.
    *
    * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE Record Not available
    * @retval    UPLL_RC_SUCCESS              Successful completion.
    * @retval    UPLL_RC_ERR_GENERIC          Generic Errors.
    * **/
    upll_rc_t DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);

    /**
    *  @brief  This API used to Update the record  in Db.
    *
    *  @param[in] req   Pointer to IpcRequestResponderHeader Class.
    *  @param[in] ikey  Pointer to ConfigKeyVal Class.
    *  @param[in] dmi   Pointer to DalDmlIntf Class(DB Interface).
    *
    *  @retval UPLL_RC_SUCCESS  Successfull completion.
    * **/

    upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);
    /**
    * @brief     Method used for Validation before Merge.
    *
    * @param[in] ikey      Pointer to the Class for which
    *                      fields have to be Validated before the Merge.
    * @param[in] keytype   The keyType Information.
    * @param[in] dmi       Pointer to DalDmlIntf Class.
    * @param[in] ctrlr_id  Controller Name.
    *
    * @return code UPLL_RC_SUCCESS  Successfull completion.
    * @retval  UPLL_RC_ERR_MERGE_CONFLICT Failure(Incase if same record
    *                                     exist in Candidate Configuration)
    *  **/
    upll_rc_t MergeValidate(unc_key_type_t keytype,
                          const char *ctrlr_id,
                          ConfigKeyVal *ikey,
                          DalDmlIntf *dmi);
   /* @brief The Rename Operation is not allowed for the Key type KT_VTN_FLOWFILTER_ENTRY
    *
   *  @return code UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT. Rename operation is not allowed for
   *                                                   this keytype
   * * */
    upll_rc_t RenameMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                   DalDmlIntf *dmi, const char *ctrlr_id);
    /**
    * @brief  Populate the configKeyVal with the Parent Class
    *         Information.
    *
    * @param[out] okey       This Contains the pointer to the ConfigKeyVal Class
    *                        for which fields have to be updated with values
    *                        from the parent Class.
    * @param[in] parent_key  This Contains the pointer to the ConfigKeyVal Class
    *                        which is the Parent Key  used to fill the details.
    *
    * @retval  UPLL_RC_SUCCESS  Successful completion.
    * @retval  UPLL_RC_ERR_GENERIC Generic Errors.
    * **/

    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
                             ConfigKeyVal *parent_key);
    /**
    * @brief  This Method is used to get the duplicate configkeyval
    *
    * @param[out] okey  Pointer to the Class for which
    *                   fields have to be updated with values from the Request.
    * @param[in]  req   Pointer to the Class which is
    *                   used for the generating the Duplicate KeyVal.
    * @param[in]  tbl   Enumerator member for Table name
    *
    * @retval  UPLL_RC_SUCCESS      Successful completion.
    * @retval  UPLL_RC_ERR_GENERIC  Generic Errors.
    *  **/

    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                                         MoMgrTables tbl);
    /**
    * @brief  This API is used to get the renamed controller key
    *         for vtn and FlowList if it is renamed.
    *
    * @param[out] ikey      The pointer to the ConfigKeyVal Class
    *                       which contain the PFC NAME in the KEY Structure
    * @param[in] data_type  Upll Configuration information
    *
    * @retval  UPLL_RC_SUCCESS  Successful completion.
    * @retval  UPLL_RC_ERR_GENERIC  Generic Errors.
    * **/

     upll_rc_t GetRenamedControllerKey(ConfigKeyVal *&ikey,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf *dmi,
                                    controller_domain *ctrlr_dom = NULL);
    /**
    * @brief This API is used to get the unc key name
    *
    * @param[in/out]] ikey  Pointer to the Class for which fields have to be
    *                       updated with values from the parent Class.
    * @param[in] dt_type    Configuration Information.
    * @param[in] dmi        Pointer to DalDmlIntf(DB Interface)
    * @param[in] ctrlr_id   Controller name.
    *
    * @retval UPLL_RC_SUCCESS  Successful completion.
    * @retval UPLL_RC_ERR_GENERIC  Generic Errors.
    * **/

    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ikey,
     upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, uint8_t *ctrlr_id);

    /**
    * @brief  This API is used to Copy the configuration from Candidate
    *         DB to Running DB
    *
    * @param[in]  keytype               Defines the keytype for which operation
    *                                   has to be carried out.
    * @param[in]  ctrlr_commit_status   List describes Commit Control Status
    *                                   Information.
    * @param[in]  dmi                   Pointer to DalDmlIntf(DB Interface).
    *
    * @retval  UPLL_RC_SUCCESS  Successfull completion.
    * * */
    upll_rc_t TxCopyCandidateToRunning(unc_key_type_t keytype,
                                  CtrlrCommitStatusList *ctrlr_commit_status,
                                  DalDmlIntf *dmi);

    /**
    * @brief  This method used to update the configstatus in Db during
    *          Trasaction Operation.
    *
    * @param[in] vtn_flowfilter_key   Pointer to ConfigKeyVal Class.
    * @param[in] op                   Upll Opeartion Type.
    * @param[in] driver_result        Result code from Driver.
    * @param[in] nreq                 Pointer to ConfigKeyVal Class.
    * @param[in] ctrlr_key            Pointer to ConfigKeyVal Class.
    *
    * @retval    UPLL_RC_SUCCESS    Successfull completion.
    *  **/

    upll_rc_t UpdateConfigStatus(ConfigKeyVal *vtn_flow_filter_entry_key,
                                                 unc_keytype_operation_t op,
                                                 uint32_t driver_result,
                                                 ConfigKeyVal *nreq,
                                                 DalDmlIntf *dmi,
                                                 ConfigKeyVal *ctrlr_key);
    /**
    * @brief  Allocates Memory for the Incoming Pointer to the Class.

    * @param[out] ck_val   This Contains the pointer to the Class for
                           which memory has to be allocated.
    * @param[in]  dt_type  Describes Configiration Information.
    * @param[in]  tbl      Describes the Destination table Information.

    * @retval UPLL_RC_SUCCESS  Successfull completion.
    * @retval UPLL_RC_ERR_GENERIC  Return Generic Error
    */

    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
              MoMgrTables tbl);
    /**
    * @brief  This API updates the Configuration status for AuditConfigiration.
    *
    * @param[out] ckv_db                Pointer to the Class for which
    *                                   ConfigStatus is Updated.
    * @param[in]  ctrlr_commit_status   Pointer to Commit Control Status structure.
    * @param[in]  response_code         Response Code.
    * @param[in]  dmi                   Pointer DalDmlIntf(DB) Interface.
    *
    * @retval  UPLL_RC_SUCCESS Successfull completion.
    * **/
    upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                      uuc::UpdateCtrlrPhase phase,
                                      ConfigKeyVal *&ckv_running);
   /**
    * @brief  Method for TxUpdateController for updating the controller .
    *
    * @param[in]  keytype             Contains respective keytype of Api.
    * @param[in]  session_id          Describes session information id .
    * @param[in]  config_id           Describes the configuartion id .
    * @param[in]  phase               Describes the phase of controller.
    * @param[in]  affected_ctrlr_set  Describes the resp controller list.
    * @param[in]  dmi                 Describes Pointer to DalDmlIntf class.
    *
    * @retval  UPLL_RT_SUCCESS      Successfull completion.
    * @retval  UPLL_RC_ERR_GENERIC  Failure.
    */

    upll_rc_t TxUpdateController(unc_key_type_t keytype,
                                 uint32_t session_id,
                                 uint32_t config_id,
                                 uuc::UpdateCtrlrPhase phase,
                                 set<string> *affected_ctrlr_set,
                                 DalDmlIntf *dmi,
                                 ConfigKeyVal **err_ckv);
     /**
     * @brief  Method TxUpdateProcess .
     *
     * @param[out]  ck_main   Contains the Pointer to ConfigkeyVal Class
     *                         and contains the Pfc Name.
     * @param[in]   ipc_req   Describes Pointer variable for IpcRequest Class.
     * @param[in]   ipc_resp  Describes Pointer variable for IpcResponse Class.
     * @param[in]   op        Decribes the resp operation .
     * @param[in]   dmi       Describes Pointer to DalDmlIntf Class.
     * @param[in]   ctrlr_id  Describes Controller name
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t TxUpdateProcess(ConfigKeyVal *ck_main,
                              IpcResponse *ipc_resp,
                              unc_keytype_operation_t op,
                              DalDmlIntf *dmi,
                              controller_domain *ctrlr_dom);

    /**
    * @brief  This API is used to know the value availability of
    *         val structure attributes
    *
    * @param[out] valid    This will contain the output of validity of the
    *                       specified attribute
    * @param[in]  val      This contains the value structure
    * @param[in]  indx     Gives the index of attribute for validity
    * @param[in]  dt_type  configuration for validity check
    * @param[in]  tbl      Table name
    *
    * @retval[out] UPLL_RC_SUCCESS  Successful completion.
    * @retval[out] UPLL_RC_ERR_GENERIC  Generic Errors.
    **  */
    upll_rc_t GetValid(void *val, uint64_t indx, uint8_t *&valid,
                                        upll_keytype_datatype_t dt_type,
                                        MoMgrTables tbl);
     /**
     * @brief  Method used for UpdateControllerTable Operation.
     *
     * @param[in,out]  ikey       Pointer to ConfigKeyVal Class.
     * @param[in]      op         Describes the Type of Opeartion.
     * @param[in]      dt_type    Describes Configiration Information.
     * @param[in]      dmi        Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
/*
    upll_rc_t UpdateControllerTable(ConfigKeyVal *ikey,
                                    unc_keytype_operation_t op,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf *dmi);
  */   /**
     * @brief  Method used for GetControllerKeyval Operation.
     *
     * @param[in]  ctrlckv           Pointer to ConfigKeyVal Class.
     * @param[in,] ikey          Pointer to ConfigKeyVal Class.
     * @param[in]  ctrlr_id   Pointer to ctrlr_id.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */

     upll_rc_t GetControllerKeyval(ConfigKeyVal *&ctrlckv,
                                  ConfigKeyVal *&ikey,
                                  controller_domain *ctrlr_id);
     /**
     * @brief  Method used for UpdateControllerTableForVtn Operation.
     *
     * @param[in]      vtn_name   Pointer to vtn_name
     * @param[in]      op         Describes the Type of Opeartion.
     * @param[in]      dmi        Pointer to DalDmlIntf Class.
     * @param[in]      ctrlr_id   Pointer to ctrlr_id.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */

     upll_rc_t UpdateControllerTableForVtn(
                                        uint8_t* vtn_name,
                                        controller_domain *ctrlr_dom,
                                        unc_keytype_operation_t op,
                                        DalDmlIntf *dmi);
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
  upll_rc_t UpdateFlowListInCtrl(ConfigKeyVal *ikey,
                                 unc_keytype_operation_t op,
                                 DalDmlIntf* dmi);
  upll_rc_t IsNWMReferenced(ConfigKeyVal *ikey,
                         DalDmlIntf *dmi);


  upll_rc_t GetControllerDomainSpan(ConfigKeyVal *ikey,
      upll_keytype_datatype_t dt_type, DalDmlIntf *dmi);
  upll_rc_t ValidateMessageForVtnFlowFilterController
                                  (IpcReqRespHeader *req,
                                    ConfigKeyVal *key);
  upll_rc_t ValidateMessageForVtnFlowFilterEntry
                                   (IpcReqRespHeader *req,
                                       ConfigKeyVal *key);
  upll_rc_t
     ValidateCapabilityForVtnFlowFilterEntry(IpcReqRespHeader *req,
                                   ConfigKeyVal *ikey,
                                   const char* ctrlr_name);
  upll_rc_t ValidateCapabilityForVtnFlowFilterController
                                   (IpcReqRespHeader *req,
                                   ConfigKeyVal *ikey,
                                   const char* ctrlr_name);
  upll_rc_t ValVtnFlowfilterCtrlAttributeSupportCheck(
  val_flowfilter_controller_t *val_flowfilter_controller,
  const uint8_t* attrs);
  upll_rc_t ValidateVtnFlowfilterControllerValue(
    val_flowfilter_controller_t *val_flowfilter_controller,
    uint32_t operation);

  upll_rc_t ReadFlowFilterController(IpcReqRespHeader *req,
                                     ConfigKeyVal *ikey,
                                     DalDmlIntf *dmi);

  upll_rc_t  ReadControllerStateNormal(ConfigKeyVal *ikey ,
                              upll_keytype_datatype_t dt_type,
                              unc_keytype_operation_t op,
                              DalDmlIntf *dmi);

  upll_rc_t ReadControllerStateDetail(ConfigKeyVal *ikey ,
                                      ConfigKeyVal *drv_resp_ckv,
                                      upll_keytype_datatype_t dt_type,
                                      unc_keytype_operation_t op,
                                      DalDmlIntf *dmi,
                                      controller_domain *ctrlr_dom,
                                      ConfigKeyVal **okey);
  upll_rc_t GetCtrlFlowFilterEntry(
     key_vtn_flowfilter_entry *l_key_vtn_ffe,
     val_vtn_flowfilter_entry_ctrlr_t *val_vtn_ffe_ctrlr,
     upll_keytype_datatype_t dt_type,
     DalDmlIntf *dmi,
     val_vtn_flowfilter_entry_t *&op_val_vtn_ffe);

  upll_rc_t  ReadSiblingCount(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi);

  upll_rc_t ReadSiblingFlowFilterController(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi);
  upll_rc_t ReadSiblingControllerStateNormal(
      ConfigKeyVal *ikey,
      IpcReqRespHeader *req,
      DalDmlIntf *dmi);

  upll_rc_t ReadSiblingControllerStateDetail(ConfigKeyVal *ikey,
                                             IpcReqRespHeader *req,
                                             DalDmlIntf *dmi);

  upll_rc_t ConstructReadSiblingNormalResponse(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey);

  upll_rc_t ConstructReadSiblingDetailResponse(
    ConfigKeyVal *ikey ,
    ConfigKeyVal *drv_resp_ckv,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey);

  upll_rc_t GetVtnControllerSpan(
      ConfigKeyVal *ikey,
      DalDmlIntf *dmi,
      std::list<controller_domain_t> &list_ctrlr_dom);

upll_rc_t UpdateControllerTable(
    ConfigKeyVal *ikey, unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    std::list<controller_domain_t> list_ctrlr_dom);

upll_rc_t UpdateMainTbl(ConfigKeyVal *vtn_ffe_key,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi);

  upll_rc_t DeleteChildMo(IpcReqRespHeader *req,
                        ConfigKeyVal *ikey,
                        DalDmlIntf *dmi);

  upll_rc_t IsFlowListConfigured(
      const char* flowlist_name, DalDmlIntf *dmi);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // MODULES_UPLL_VTN_FLOWFILTER_ENTRY_MOMGR_HH_
