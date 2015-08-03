/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef MODULES_UPLL_POLICINGPROFILE_MOMGR_HH_
#define MODULES_UPLL_POLICINGPROFILE_MOMGR_HH_

#include <string>
#include <set>
#include "momgr_impl.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

/* This file declares interfaces for keyType KT_POLICING_PROFILE */
/**
 * @brief PolicingProfileMoMgr class handles all the request
 *  received from service.
 */
class PolicingProfileMoMgr : public MoMgrImpl {
  private:
    /**
     * Member Variable for PolicingProfileBindInfo.
     */
    static unc_key_type_t policingprofile_child[];

    /**
     * Member Variable for PolicingProfileBindInfo.
     */
    static BindInfo policingprofile_bind_info[];

    /**
     * Member Variable for PolicingProfileBindInfo.
     */
    static BindInfo policingprofile_rename_bind_info[];

    /**
     * Member Variable for PolicingProfileBindInfo.
     */
    static BindInfo policingprofile_controller_bind_info[];

    /**
     * Member Variable for PolicingProfileBindInfo.
     */
    static BindInfo rename_policingprofile_main_tbl[];

    /**
     * Member Variable for PolicingProfileBindInfo.
     */
    static BindInfo rename_policingprofile_ctrlr_tbl[];

    /**
     * Member Variable for PolicingProfileBindInfo.
     */
    static BindInfo rename_policingprofile_rename_tbl[];
   /**
    * @Brief Validates the syntax of the specified key and value structure
    *        for KT_POLICINGPROFILE keytype
    *
    * @param[in] IpcReqRespHeader contains first 8 fields of input request
    *            structure
    * @param[in] ConfigKeyVal key and value structure.
    *
    * @retval UPLL_RC_SUCCESS              Successful.
    * @retval UPLL_RC_ERR_CFG_SYNTAX       Syntax error.
    * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE policingprofile is not available.
    * @retval UPLL_RC_ERR_GENERIC          Generic failure.
    * @retval UPLL_RC_ERR_INVALID_OPTION1  option1 is not valid.
    * @retval UPLL_RC_ERR_INVALID_OPTION2  option2 is not valid.
    */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *key);

   /**
    * @Brief Validates the syntax for KT_POLICING_PROFILE keytype
    *   key structure.
    *
    * @param[in] key_flowlist KT_POLICING_PROFILE key structure.
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */
    upll_rc_t ValidatePolicingProfileRenameValue(
       val_rename_policingprofile_t *val_rename_policingprofile,
       uint32_t operation);

   /**
    * @Brief Checks if the specified key type(KT_POLICING_PROFILE) and
    *        associated attributes are supported on the given controller,
    *        based on the valid flag
    *
    * @param[in] IpcReqRespHeader  contains first 8 fields of input request
    *                              structure
    * @param[in]   ConfigKeyVal    contains key and value structure.
    * @param[in]   ctrlr_name      controller_name
    *
    * @retval  UPLL_RC_SUCCESS             Validation succeeded.
    * @retval  UPLL_RC_ERR_GENERIC         Validation failure.
    * @retval  UPLL_RC_ERR_INVALID_OPTION1 Option1 is not valid.
    * @retval  UPLL_RC_ERR_INVALID_OPTION2 Option2 is not valid.
    */
    upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                   const char* ctrlr_name = NULL);

  public:
    /**
     * @brief PolicingProfileMoMgr Class Constructor.
     */
    PolicingProfileMoMgr();
    /**
     * @brief PolicingProfileMoMgr Class Destructor.
     */
    ~PolicingProfileMoMgr();

    /**
     * @brief  Validates the Attribute of a Particular Class.
     *
     * @param[in]  kval  This contains the Class to which the fields
     *                   have to be validated.
     *
     * @retval  UPLL_RC_SUCCESS  Successfull completion.
     */
    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);

     /**
     * @brief  Gets the validity of the index in the given val structure
     *
     * @param[out] valid    This will contain the output of validity of
     *                      specified attribute
     * @param[in]  val      This contains the value structure
     * @param[in]  indx     Gives the index of attribute for validity
     * @param[in]  dt_type  Gives the configuration for validity check
     * @param[in]  tbl      Contains the table name
     *
     * @retval  UPLL_RC_SUCCESS     Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC Val is NULL.
     */
    upll_rc_t GetValid(void *val, uint64_t indx, uint8_t *&valid,
                   upll_keytype_datatype_t dt_type, MoMgrTables tbl = MAINTBL);

    /**
     * @brief  Allocates Memory for the Incoming Pointer to the Class.
     *
     * @param[out] ck_val   Contains the pointer to the Class for which
     *                      memory has to be allocated.
     * @param[in]  dt_type  Describes Configiration Information.
     * @param[in]  tbl      Describes the Destination table Information.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure
     */
    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl);

    /**
     * @brief     Method used to fill the CongigKeyVal with the Parent Class
     *            Information.
     *
     * @param[out] okey        Contains the pointer to the ConfigKeyVal Class
     *                         for which fields have to be updated with values
     *                         from the parent Class.
     * @param[in]  parent_key  Contains the pointer to the ConfigKeyVal
     *                         Class which is the Parent Class used to fill
     *                         the details.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure
     */
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);

    /**
     * @brief  Method used to get the RenamedUncKey.
     *
     * @param[out]  ctrlr_key  This Contains the pointer to the Class for which
     *                         fields have to be updated with values from the
     *                         parent Class.
     * @param[in]   dt_type    Describes Configiration Information.
     * @param[in]   dmi        Pointer to DalDmlIntf Class.
     * @param[in]   ctrlr_id   Describes the Controller Name.
     *
     * @retval  RT_SUCCESS                    Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                               upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                               uint8_t *ctrlr_id);

    /**
     * @brief  Method used for RenamedControllerkey(PfcName).
     *
     * @param[out]  ikey      Contains the Pointer to ConfigkeyVal Class and
     *                        contains the Pfc Name.
     * @param[in]   dt_type   Describes Configiration Information.
     * @param[in]   dmi       Pointer to DalDmlIntf Class.
     * @param[in]   ctrlr_id  Describes the Controller Name.
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
     * @brief  Method used to Duplicate the ConfigkeyVal.
     *
     * @param[out]  okey  This Contains the pointer to the Class for which
     *                    fields have to be updated with values from
     *                    the Request.
     * @param[in]   req   This Contains the pointer to the Class which is
     *                    used for the Duplication .
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     */
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                              MoMgrTables tbl);

    /**
     * @brief  Method Updates the ConfigStatus for AuditConfigiration.
     *
     * @param[out]  ckv_db               This Contains the pointer to the Class
     *                                   for which ConfigStatus is Updated.
     * @param[in]   ctrlr_commit_status  Describes Commit Control Status
     *                                   Information.
     * @param[in]   response_code        Describes the Response Code.
     * @param[in]   dmi                  Pinter to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t UpdateAuditConfigStatus(
        unc_keytype_configstatus_t cs_status, uuc::UpdateCtrlrPhase phase,
        ConfigKeyVal *&ckv_running, DalDmlIntf *dmi);

    /**
     * @brief  Method Swaps the Key and Val structures.
     *
     * @param[out]  okey   Contains the pointer to the class in which key and val
     *                     are swapped.
     * @param[in]   ikey   Contains the pointer to the class in which key and val
     *                     structures are to br swapped.
     * @param[in]   ctrlr  Pointer to the ctrlr_id
     * @param[in]   dmi    Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Swapping fails
     */
    upll_rc_t SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                         DalDmlIntf *dmi, uint8_t *ctrlr, bool &no_rename);

    /**
     * @brief  Method used for Validation before Merge.
     *
     * @param[in]  ikey      This Contains the pointer to the Class for which
     *                       fields have to be Validated before the Merge.
     * @param[in]  keytype   Describes the keyType Information.
     * @param[in]  dmi       Pointer to DalDmlIntf Class.
     * @param[in]  ctrlr_id  Describes the Controller Name.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     * @retval  UPLL_RC_ERR_MERGE_CONFLICT    Merge conflict
     */
    upll_rc_t MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
                            ConfigKeyVal *ikey, DalDmlIntf *dmi,
                            upll_import_type import_type);

    /**
     * @brief     Method to Copy Candidate DB to Running DB
     *
     * @param[in]  keytype              Defines the keytype for which operation
     *                                  has to be carried out.
     * @param[in]  ctrlr_commit_status  List describes Commit Control Status
     *                                  Information.
     * @param[in]  dmi                  Pinter to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t TxCopyCandidateToRunning(
        unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
        DalDmlIntf *dmi, TcConfigMode config_mode, std::string vtn_name);

    /**
     * @brief  Method used to check whether the key is referred by any entry
     *
     * @param[in]  keytype  Defines the keytype for which operation has to
     *                      be carried out.
     * @param[in]  ikey     Pointer to class ConfigKeyVal which has Key to
     *                      check for reference
     * @param[in]  dmi      Pinter to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);

    /**
     * @brief  Method to compare to keys
     *
     * @param[in]  key1  Pointer to key structure for comparision
     * @param[in]  key2  Pointer to key for comparision
     *
     * @return  TRUE   If keys match
     * @return  FALSE  If keys dont match
     */
    pfc_bool_t CompareKey(ConfigKeyVal *key1, ConfigKeyVal *key2);

    /**
     * @brief  Method used for Read Operation.
     *
     * @param[in]     req   Describes RequestResponderHeaderClass.
     * @param[in/out] ikey  Pointer to ConfigKeyVal Class.
     * @param[in]     dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);

    /**
     * @brief  Method used for ReadSibling Operation.
     *
     * @param[in]     req   Describes RequestResponderHeaderClass.
     * @param[in/out] ikey  Pointer to ConfigKeyVal Class.
     * @param[in]     dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t ReadSiblingMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                            bool begin, DalDmlIntf *dmi);

    /**
     * @brief  Method used for Trasaction Vote Operation.
     *
     * @param[in]  key            Pointer to ConfigKeyVal Class.
     * @param[in]  op             Describes the Type of Opeartion.
     * @param[in]  driver_result  Describes the result of Driver Operation.
     * @param[in]  upd_key        Pointer to ConfigKeyVal Class.
     * @param[in]  ctrlr_key      Pointer to ConfigKeyVal Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t UpdateConfigStatus(ConfigKeyVal *ikey,
                                 unc_keytype_operation_t op,
                                 uint32_t driver_result, ConfigKeyVal *nreq,
                                 DalDmlIntf *dmi,  ConfigKeyVal *ctrlr_key);

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

    /**
     * @brief  Method to get Parent ConfigKeyVal
     *
     * @param[in]   ConfigKeyVal  parent_key
     * @param[out]  ConfigKeyVal  okey
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     */
    upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                 ConfigKeyVal *ikey);

    /**
     * @brief  Method to Compare Valid val structure
     *
     * @param[in/out] val1  val structure
     * @param[out]  val2  val structure
     *
     */
    bool CompareValidValue(void *&val1, void *val2, bool audit) {
      return true;
    }

    /**
     * @brief  Method to check validity of Key
     *
     * @param[in]   ConfigKeyVal  input COnfigKeyVal
     * @param[out]  uint64_t      index
     *
     * @return  TRUE   Success
     * @retval  FALSE  Failure
     */
    bool IsValidKey(void *ikey, uint64_t index, MoMgrTables tbl = MAINTBL);

    /**
     * @brief  Method to Get the controller span
     *
     * @param[in]  ikey     Pointer to ConfigKeyVal
     * @param[in]  dt_type  Data Type.
     * @param[in]  dmi      Pointer to DalDmlIntf.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t GetControllerSpan(ConfigKeyVal *ikey,
              upll_keytype_datatype_t dt_type,
              DalDmlIntf *dmi);

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
     */
    upll_rc_t SetConsolidatedStatus(ConfigKeyVal *ikey,
        DalDmlIntf *dmi);

    /**
     * @brief  Method to Delete record in ctrlrtbl
     *
     * @param[in]  pp_ckv   Pointer to ConfigKeyVal
     * @param[in]  dt_type  Data Type.
     * @param[in]  dmi      Pointer to DalDmlIntf.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t CtrlrTblDelete(ConfigKeyVal *pp_ckv,
        DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
        TcConfigMode config_mode, string vtn_name, bool is_commit,
        uint32_t count);

    /**
     * @brief  Method to Create record in ctrlrtbl
     *
     * @param[in]  pp_ckv   Pointer to ConfigKeyVal
     * @param[in]  dt_type  Data Type.
     * @param[in]  dmi      Pointer to DalDmlIntf.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t CtrlrTblCreate(ConfigKeyVal *pp_ckv,
        DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
        TcConfigMode config_mode, string vtn_name, bool is_commit,
        uint32_t count);

    /**
     * @brief  Method to Create or Delete record in ctrlrtbl
     *
     * @param[in]  pp_ckv   Pointer to ConfigKeyVal
     * @param[in]  dt_type  Data Type.
     * @param[in]  op       Create or delete operation
     * @param[in]  dmi      Pointer to DalDmlIntf.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t PolicingProfileCtrlrTblOper(const char *policingprofile_name,
       const char *ctrlr_id, DalDmlIntf *dmi, unc_keytype_operation_t oper,
       upll_keytype_datatype_t dt_type, uint8_t pp_flag,
       TcConfigMode config_mode, string vtn_name, uint32_t count = 1,
       bool is_commit = false);

    /**
     * @brief  Method to Get Policingprofile COnfigKeyVal
     *
     * @param[in]  pp_ckv                Pointer to ConfigKeyVal
     * @param[in]  policingprofile_name  policingprofile name.
     * @param[in]  ctrlr_id              controller name.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t GetPolicingProfileCtrlrKeyval(
        ConfigKeyVal *&pp_ckv,
        const char *policingprofile_name,
        const char *ctrlr_id);

    upll_rc_t UpdateMainTbl(ConfigKeyVal *key_pp,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi);

    upll_rc_t GetDiffRecord(ConfigKeyVal *ckv_running,
                            ConfigKeyVal *ckv_audit,
                            uuc::UpdateCtrlrPhase phase, MoMgrTables tbl,
                            ConfigKeyVal *&okey,
                            DalDmlIntf *dmi,
                            bool &invalid_attr,
                            bool check_audit_phase);

    upll_rc_t SetValidAudit(ConfigKeyVal *&ikey);

    bool FilterAttributes(void *&val1,
                          void *val2,
                          bool copy_to_running,
                          unc_keytype_operation_t op);

    upll_rc_t SetPPConsolidatedStatus(ConfigKeyVal *ikey,
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

    upll_rc_t UpdateRefCountInScratchTbl(
        ConfigKeyVal *ikey,
        DalDmlIntf *dmi,
        upll_keytype_datatype_t dt_type,
        unc_keytype_operation_t op,
        TcConfigMode config_mode,
        string vtn_name,
        uint32_t count);

    upll_rc_t InsertRecInScratchTbl(
        ConfigKeyVal *ikey,
        DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
        unc_keytype_operation_t op, TcConfigMode config_mode,
        string vtn_name,
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
        const char* policingprofile_name, DalDmlIntf *dmi,
        TcConfigMode config_mode, string vtn_name);

    upll_rc_t InstanceExistsInScratchTbl(
        ConfigKeyVal *ikey, TcConfigMode config_mode, string vtn_name,
        DalDmlIntf *dmi);

    upll_rc_t ClearVirtualKtDirtyInGlobal(DalDmlIntf *dmi);

    upll_rc_t DeleteChildrenPOM(ConfigKeyVal *ikey,
        upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
        TcConfigMode config_mode, string vtn_name);
};

typedef struct val_policingprofile_ctrl {
    unc_keytype_configstatus_t cs_row_status;
    uint8_t flags;    // DBFLAGS
    uint32_t ref_count;  // DB RefCount
    uint8_t valid[1];
} val_policingprofile_ctrl_t;

typedef struct key_policingprofile_ctrl {
  uint8_t policingprofile_name[33];
  uint8_t ctrlr_name[32];
} key_policingprofile_ctrl_t;
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // MODULES_UPLL_POLICINGPROFILE_MOMGR_HH_

