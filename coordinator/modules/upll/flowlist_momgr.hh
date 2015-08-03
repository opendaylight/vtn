/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef MODULES_UPLL_FLOWLIST_MOMGR_HH_
#define MODULES_UPLL_FLOWLIST_MOMGR_HH_

#include <string>
#include <set>
#include "momgr_impl.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

class FlowListMoMgr : public MoMgrImpl {
  private:
    static unc_key_type_t flowlist_child[];
    static BindInfo flowlist_bind_info[];
    static BindInfo flowlist_rename_bind_info[];
    static BindInfo flowlist_controller_bind_info[];

    /**
     * Member Variable for FlowListBindInfo.
     */
    static BindInfo rename_flowlist_main_tbl[];

    /**
     * Member Variable for FlowListBindInfo.
     */
    static BindInfo rename_flowlist_ctrlr_tbl[];

    /**
     * Member Variable for FlowListBindInfo.
     */
    static BindInfo rename_flowlist_rename_tbl[];

    /**
     * @Brief Validates the syntax of the specified key and value structure
     *        for KT_FLOWLIST keytype
     *
     * @param[in] req  Pointer to IpcResResHeader
     * @param[in] key  Pointer to the ConfigKeyVal Structure
     *
     * @retval UPLL_RC_SUCCESS               Successful.
     * @retval UPLL_RC_ERR_CFG_SYNTAX        Syntax error.
     * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE  key_flowlist is not available.
     * @retval UPLL_RC_ERR_GENERIC           Generic failure.
     * @retval UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
     * @retval UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
     */
     upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *key);

     upll_rc_t ValidateFlowListKey(ConfigKeyVal *key,
                                   unc_keytype_operation_t op);

    /**
     *  @Brief Validates the syntax for KT_FLOWLIST Rename value structure.
     *
     *   @param[in] val_rename_flowlist KT_FLOWLIST rename value structure.
     *
     *    @retval UPLL_RC_SUCCESS        validation succeeded.
     *    @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
     */
     upll_rc_t ValidateFlowListValRename(ConfigKeyVal *key,
                                         uint32_t operation,
                                         uint32_t datatype);

    /**
     *  @Brief Validates the syntax for KT_FLOWLIST keytype value structure.
     *
     *   @param[in] val_flowlist KT_FLOWLIST value structure.
     *   @param[in] operation    Describes operation code
     *
     *    @retval UPLL_RC_SUCCESS        validation succeeded.
     *    @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
     */
     upll_rc_t ValidateFlowListVal(ConfigKeyVal *key,
                                   uint32_t operation,
                                   uint32_t datatype);

    /**
     * @brief     Methods Used for  Validating Attribute.
     *
     * @param[in]  kval     The pointer to the ConfigKeyVal class
     *
     * @param[in]  dmi      Pointer to the Database Interface.
     *
     * @retval  UPLL_RC_SUCCESS      Validation succeeded.
     * @retval  UPLL_RC_ERR_GENERIC  Validation failure.
     */
     upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                 DalDmlIntf *dmi,
                                 IpcReqRespHeader *req = NULL);

     /**
     * @Brief Checks if the specified key type(KT_FLOWLIST) and
     *        associated attributes are supported on the given controller,
     *        based on the valid flag
     *
     * @param[in] IpcReqRespHeader  contains first 8 fields of input
     *                              request structure
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
     * @Brief Checks if the specified key type and
     *        associated attributes are supported on the given controller,
     *        based on the valid flag.
     *
     * @param[in] val_flowlist  KT_FLOWLIST value structure.
     * @param[in] attrs         pointer to controller attribute
     *
     * @retval UPLL_RC_SUCCESS                    validation succeeded.
     * @retval UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR Attribute NOT_SUPPORTED.
     * @retval UPLL_RC_ERR_GENERIC                Generic failure.
     */
     upll_rc_t ValFlowlistAttributeSupportCheck(val_flowlist_t *val_flowlist,
       const uint8_t* attrs);

    upll_rc_t UpdateMainTbl(ConfigKeyVal *key_fl,
                            unc_keytype_operation_t op,
                            uint32_t driver_result,
                            ConfigKeyVal *nreq,
                            DalDmlIntf *dmi);

  public:
    /**
     * @brief constructor
     */
    FlowListMoMgr();

   /**
    * @brief Destructor
    **/
    ~FlowListMoMgr() {
      for (int i = 0; i < ntable; i++) {
        if (table[i]) {
          delete table[i];
        }
      }
      delete[] table;
    }

   /**
    * @Brief Compares two flowlist key
    *
    * @param[in] key1  pointer to the configkeyval structure containing the
    *                  key which needs to compare
    * @param[in] key2  pointer to the configkeyval structure containing the
    *                  key which needs to compare
    *
    * @return  Returns true if both the keys are same
    * */
    bool CompareKey(void *key1, void *key2);

   /* @brief         Copies the new flowlist name to the newly created key
    *                and assign this key to the output ConfigKeyVal structure
    *
    * @param[in] ikey   Pointer to the ConfigKeyval containing the Key and
    *                   value structure
    * @param[in] dmi    Pointer to the DalDmlIntf(DB Interface)
    * @param[in] ctrlr  controller name
    *
    * @retval  UPLL_RC_SUCCECSS     Successful Completion
    * @retval  UPLL_RC_ERR_GENERIC  For failue case GENERIC ERROR
    */
    upll_rc_t SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                         DalDmlIntf *dmi, uint8_t *ctrlr, bool &no_rename);

    upll_rc_t UpdateConfigStatus(ConfigKeyVal *flowlist_key,
                                 unc_keytype_operation_t op,
                                 uint32_t driver_result, ConfigKeyVal *nreq,
                                 DalDmlIntf *dmi,
                                 ConfigKeyVal *ctrlr_key);
    /**
     * @brief  Update config status for commit result and vote result.
     *
     * @param[in,out]  ckv_running  ConfigKeyVal instance.
     * @param[in]      cs_status    either UNC_CS_INVALID or UNC_CS_APPLIED.
     * @param[in]      phase        specify the phase (CREATE,DELETE or UPDATE)
     * @param[in]      dmi          Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RC_SUCCECSS     Successful Completion
     * @retval  UPLL_RC_ERR_GENERIC  For failue case GENERIC ERROR
     *
     ***/

     upll_rc_t UpdateAuditConfigStatus(
                           unc_keytype_configstatus_t cs_status,
                           uuc::UpdateCtrlrPhase phase,
                           ConfigKeyVal *&ckv_running,
                           DalDmlIntf *dmi);

    /* @brief            Retrieve the NewConfigKeyVal based on the input  parent
     *                   ConfigKey.
     *                   If parent configkey is not available then create new
     *                    configkeyval and return that keyval.
     *
     * @param[out] okey  Out Pointer to the configkeyval
     * @param[in]  parent_key Input configkeyval pointer
     *
     * @retval  UPLL_RC_SUCCECSS     Successful Completion
     * @retval  UPLL_RC_ERR_GENERIC  On Failure
     **/
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);

    /* @brief        Checkes whether the key exists in DB
     *
     * @param[in]  ikey     Pointer to the ConfigKeyval containing the Key and
     *                      value structure of Import Configuration
     * @param[in]  dt_type  Given UNC Datatype at which reference needs to check
     * @param[in]  dmi      Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RC_SUCCECSS     Successful Completion
     * @retval  UPLL_RC_ERR_GENERIC  For failue case GENERIC ERROR
     **/
    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);

    /* @brief  Read the configuration from DB based on the operation code
    *
    *  @param[in]     req   Pointer to IpcResResHeader
    *  @param[in,out] ikey  Pointer to the ConfigKeyVal Structure
    *  @param[in]     dmi   Pointer to the DalDmlIntf(DB Interface)
    *
    *  @retval  UPLL_RC_SUCCESS                    Completed successfully.
    *  @retval  UPLL_RC_ERR_GENERIC                Generic failure.
    *  @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
    *  @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
    *  @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
    **/
    upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);

    /* @brief  Validates the Import configuration with the Running Configuration
     *         during Import Operations
     *
     * @param[in]  keytype    UNC Keytype
     * @param[in]  ctrl_id    Controller Name
     * @param[in]  configkey  Pointer to the ConfigKeyval containing the Key
     *                        and Value structure of Import Configuration
     * @param[in]  dmi        Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval UPLL_RC_SUCCECSS                     Successful Completion
     * @retval UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT  Read Operation is
     *                                              not allowed for the given
     *                                              datatype
     * @retval UPLL_RC_ERR_MERGE_CONFLICT           For failue case
     *                                              (if configuration already
     *                                              exists in RunningDB)
     **/
    upll_rc_t MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
                            ConfigKeyVal *conflict_ckv, DalDmlIntf *dmi,
                            upll_import_type import_type);

   /* @brief      Read the configuration from DB based on the operation code
    *
    * @param[in]     req    Pointer to IpcResResHeader
    * @param[in,out] ikey   Pointer to the ConfigKeyVal Structure
    * @param[in]     begin  boolean variable to decide the sibling operation
    * @param[in]     dmi    Pointer to the DalDmlIntf(DB Interface )

    * @retval  UPLL_RC_SUCCESS                    Completed successfully.
    * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
    * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
    * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
    * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
    **/
    upll_rc_t ReadSiblingMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                            bool begin, DalDmlIntf *dmi);

   /** @brief Performs the update operation
    *         This operation is not allowed for FlowListMomgr
    *
    *  @retval UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT
    **/
    upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                       DalDmlIntf *dmi);


   /**
    * @brief     This method is used for copying configuration from Candidate
    *            to Running DB during transaction operation.
    *
    * @param[in]  keytype               Defines the keytype for which operation
    *                                   has to be carried out.
    * @param[in]  ctrlr_commit_status   List describes Commit Control Status
    *                                   Information.
    * @param[in]  dmi                   Pointer to DalDmlIntf Class.
    *
    * @retval  UPLL_RC_SUCCESS        successful completion
    * @retval  UPLL_RC_ERR_DB_ACCESS  DB Read/Write error.
    * @retval  UPLL_RC_ERR_GENERIC    For generic failure
    **/
    upll_rc_t TxCopyCandidateToRunning(
        unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
        DalDmlIntf *dmi, TcConfigMode config_mode, std::string vtn_name);

    /** @brief  Allocate the memory for the value structure depending upon
     *          the tbl ARG
     *
     *  @param[in,out]  ck_val   pointer to configval structure
     *  @param[in]      dt_type  UPLL Datatype
     *  @param[in]      tbl      enum for Tabletypes
     *                           Possible values could be
     *                           MAINTBL
     *                           RENAMETBL
     *                           CTRLRTBL
     *
     *  @retval  UPLL_RC_SUCCESS     successful completion
     *  @retval  UPLL_RC_ERR_GENERIC For failure
     * * */
    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl = MAINTBL);

    /**
     * @brief       Fetch the valid array value and assign it to the valid flag
     *              values to the outparam based on the table type and the
     *              datatype.
     *
     * @param[in]   val    pointer to the value structure.
     * @param[out]  valid  reference to the enum containing the possible
     *                     values of Valid flag.
     * @param[out]  indx   reference for the DbFlag.
     * @param[in]   tbl    Containing the values for Tabletype
     *
     * @retval  UPLL_RC_SUCCESS      Successful completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure occurred.
     **/
    upll_rc_t GetValid(void*val, uint64_t indx, uint8_t *&valid,
                       upll_keytype_datatype_t dt_type, MoMgrTables tbl =
                           MAINTBL);

    /* @brief  Creates a duplicate configkeyval structure from the existing
     *         configkey val structure
     * @param[out]  okey  Pointer to the ConfigKeyval containing the Key and
     *                    value structure
     * @param[in]   req   Pointer to the ConfigKeyval containing the Key and
     *                    value structure
     * @param[in]   tbl   Enumerator variable contaiing the Table type
     *
     * @retval  UPLL_RC_SUCCECSS     Successful Completion
     * @retval  UPLL_RC_ERR_GENERIC  For failue case GENERIC ERROR
     * **/
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                              MoMgrTables tbl = MAINTBL);

    /* @brief     Retrieve the controller key based on the UNC KEY which
     *            got renamed.
     *
     * @param[in,out] ikey       Pointer to configkeyval structure of inputkey
     * @param[in]     dt_type    upll datatype
     * @param[in]     dmi        pointer to the DalDmlIntf Interface
     * @param[in]     ctrl_name  Controller Name
     *
     * @retval  UPLL_RC_SUCCECSS     Successful Completion
     * @retval  UPLL_RC_ERR_GENERIC  On Failure
     **/
    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi,
                                      controller_domain *ctrlr_dom = NULL);

    /* @brief  Retrieve the UNC key which got renamed  based on the input
     *         ConfigKeyval in which PFC key is available.
     *
     * @param[in/out] ctrl_key  Pointer to configkeyval structure containing
     *                          controller key
     * @param[in]     dt_type   upll datatype
     * @param[in]     dmi       pointer to the DalDmlIntf Interface
     * @param[in]     ctrl_id   Controller Name
     *
     * @retval  UPLL_RC_SUCCECSS Successful Completion
     *          UPLL_RC_ERR_GENERIC On Failure(if unable to generate output key)
     **/
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                               upll_keytype_datatype_t dt_type,
                               DalDmlIntf *dmi,
                               uint8_t *ctrlr_id);

    /* @brief  Read the configuration from DB based on the operation code
     *
     * @param[in]      req    Pointer to IpcResResHeader
     * @param[in,out]  ikey   Pointer to the ConfigKeyVal Structure
     * @param[in]      dmi    Pointer to the DalDmlIntf(DB Interface)
     * @param[in]      op     Upll Operation code
     *
     * @retval  UPLL_RC_SUCCESS                    Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
     * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
     * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
     *
     **/
    upll_rc_t ReadRecord(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                         DalDmlIntf *dmi);
    /**
     * @brief  Method to old policingprofile name from rename struct to
     *         output ConfigKeyVal
     *
     * @param[in]   ikey         Pointer to input ConfigKeyVal
     * @param[out]  okey         Pointer to output ConfigKeyVal.
     * @param[in]   rename_info  Pointer to ConfigKeyVal with rename struct
     *                           as key.
     * @param[in]   dmi          Pointer to DalDmlIntf Class.
     * @param[in]   ctrlr_id     Pointer to ctrlr_id
     * @param[in]   renamed      bool variable to check whether its renamed or
     *                           not
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     */
    upll_rc_t GetRenameInfo(ConfigKeyVal *ikey,
        ConfigKeyVal *okey, ConfigKeyVal *&rename_info, DalDmlIntf *dmi,
        const char *ctrlr_id, bool &renamed);

    /**
     * @brief  Method to Create ConfigKeyVal with rename struct as key
     *
     * @param[in]   ikey  Pointer to input ConfigKeyVal
     * @param[out]  okey  Pointer to output ConfigKeyVal.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     */
    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);

    /**
     * @brief  Method to get the bind struct depending on table
     *
     * @param[in]   key_type  Keytype for bind attr selection
     * @param[out]  binfo     Pointer to bindinfo structure
     * @param[out]  nattr     Number of attributes to bind
     * @param[in]   tbl       Table for which bind attr is to be selected
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     */
    bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                              int &nattr, MoMgrTables tbl);
   /** @brief Method to Update the Controller Table entry
     *
     * @param[in] ikey     Pointer to ConkeyValClass
     * @param[in] op       Operation code
     * @param[in] dmi      Pointer to DB Interface
     * @param[in] ctrl_id  Controller Name
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure
     *
   */
    upll_rc_t UpdateControllerTable(ConfigKeyVal *ikey,
                                    unc_keytype_operation_t op,
                                    DalDmlIntf *dmi,
                                    char* ctrl_id,
                                    TcConfigMode config_mode,
                                    string vtn_name);
    upll_rc_t AddFlowListToController(char *flowlist_name,
                                      DalDmlIntf *dmi,
                                      char* ctrl_id,
                                      upll_keytype_datatype_t dt_type,
                                      unc_keytype_operation_t op,
                                      TcConfigMode config_mode,
                                      string vtn_name, bool is_commit = false);
    /**
     * @brief  Method to Set the Consolidated status
     *
     * @param[in]  ikey     Pointer to ConfigKeyVal
     * @param[in]  dmi      Pointer to DalDmlIntf.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     **/
    upll_rc_t SetConsolidatedStatus(ConfigKeyVal *ikey,
                                               DalDmlIntf *dmi);

    /**
     * @brief Method To Compare the Valid Check of Attributes
     *
     * @param[out]  val1   Pointer to ConfigKeyVal Class which
     *                     contains only Valid Attributes
     * @param[in]   val2   Pointer to ConfigKeyVal Class.
     * @param[in]   audit  If true,Audit Process
     * @retval UPLL_RC_SUCCESS;
     */
     bool  CompareValidValue(void *&val1, void *val2, bool audit);

    /**
     * @brief  Method to check validity of Key
     *
     * @param[in]   ConfigKeyVal  input COnfigKeyVal
     * @param[out]  index     Column Index
     *
     * @return  TRUE   Success
     * @retval  FALSE  Failure
     * */
     bool IsValidKey(void *kaey, uint64_t index, MoMgrTables tbl = MAINTBL);
    /**
     * @brief  Method to get Parent ConfigKeyVal
     *
     * @param[in]   ConfigKeyVal  parent_key
     * @param[out]  ConfigKeyVal  okey
     *
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     **/
     upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                  ConfigKeyVal *ikey);

    upll_rc_t CreateFlowListToController(
        char *flowlist_name, DalDmlIntf *dmi, char* ctrl_id,
        upll_keytype_datatype_t dt_type, unc_keytype_operation_t op,
        TcConfigMode config_mode, string vtn_name, bool is_commit,
        int count);

    upll_rc_t DeleteFlowListToController(
        char *flowlist_name, DalDmlIntf *dmi, char* ctrl_id,
        upll_keytype_datatype_t dt_type, unc_keytype_operation_t op,
        TcConfigMode config_mode, string vtn_name, bool is_commit,
        int count);

    upll_rc_t SetValidAudit(ConfigKeyVal *&ikey);

    bool FilterAttributes(void *&val1,
                          void *val2,
                          bool copy_to_running,
                          unc_keytype_operation_t op);

    upll_rc_t SetFlowListConsolidatedStatus(ConfigKeyVal *ikey,
                                            uint8_t *ctrlr_id,
                                            DalDmlIntf *dmi);

    upll_rc_t UpdateRefCountInCtrlrTbl(ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi,
                                       upll_keytype_datatype_t dt_type,
                                       TcConfigMode config_mode,
                                       string vtn_name);

    upll_rc_t GetOperation(uuc::UpdateCtrlrPhase phase,
                           unc_keytype_operation_t &op);
    upll_rc_t CopyKeyToVal(ConfigKeyVal *ikey,
                           ConfigKeyVal *&okey);

    upll_rc_t GetControllerDomainSpan(
        ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
        DalDmlIntf *dmi);

    upll_rc_t GetDomainsForController(
        ConfigKeyVal *ckv_drvr,
        ConfigKeyVal *&ctrlr_ckv,
        DalDmlIntf *dmi);

    upll_rc_t UpdateRefCountInScratchTbl(
        ConfigKeyVal *ikey,
        DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
        unc_keytype_operation_t op,
        TcConfigMode config_mode, string vtn_name,
        uint32_t count);

    upll_rc_t InsertRecInScratchTbl(
        ConfigKeyVal *ikey,
        DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
        unc_keytype_operation_t op,
        TcConfigMode config_mode, string vtn_name,
        uint32_t count);

    upll_rc_t ComputeRefCountInScratchTbl(
        ConfigKeyVal *ikey,
        DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
        TcConfigMode config_mode, string vtn_name,
        int &ref_count);

    upll_rc_t ReadCtrlrTbl(
        ConfigKeyVal *&okey,
        DalDmlIntf *dmi, upll_keytype_datatype_t dt_type);

    upll_rc_t ComputeCtrlrTblRefCountFromScratchTbl(
        ConfigKeyVal *ikey,
        DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
        TcConfigMode config_mode, string vtn_name);

    upll_rc_t ClearScratchTbl(
        TcConfigMode config_mode, string vtn_name,
        DalDmlIntf *dmi, bool is_abort = false);

    upll_rc_t RevertCtlrTblEntries(
        TcConfigMode config_mode, string vtn_name,
        DalDmlIntf *dmi);

    upll_rc_t RefCountSemanticCheck(
        const char* flowlist_name, DalDmlIntf *dmi,
        TcConfigMode config_mode, string vtn_name);

    upll_rc_t InstanceExistsInScratchTbl(
        ConfigKeyVal *ikey, TcConfigMode config_mode, string vtn_name,
        DalDmlIntf *dmi);

    upll_rc_t ClearVirtualKtDirtyInGlobal(DalDmlIntf *dmi);

    upll_rc_t DeleteChildrenPOM(ConfigKeyVal *ikey,
        upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
        TcConfigMode config_mode, string vtn_name);
};

typedef struct val_flowlist_ctrl {
    uint8_t valid[2];  // valid[1] is for refcount
    unc_keytype_configstatus_t cs_row_status;
    unc_keytype_configstatus_t cs_attr[1];
    uint8_t flags;    // DBFLAGS
    uint32_t refcount;  // DB RefCount
} val_flowlist_ctrl_t;
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // MODULES_UPLL_FLOWLIST_MOMGR_HH_
