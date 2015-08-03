/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef MODULES_UPLL_VBR_FLOWFILTER_ENTRY_MOMGR_HH_
#define MODULES_UPLL_VBR_FLOWFILTER_ENTRY_MOMGR_HH_

#include <string>
#include "momgr_impl.hh"

#define FLOWLIST  1
#define NWMONITOR 2

namespace unc {
namespace upll {
namespace kt_momgr {

/*  This file declares interfaces for keyType KT_VBR_FLOWFILER_ENTRY */
/**
 * @brief  VbrFlowFilterEntryMoMgr class handles all the request
 *         received from service.
 */
class VbrFlowFilterEntryMoMgr : public MoMgrImpl {
  private:
    /**
     * @brief  Member Variable for VbrFlowfilterEntryBindInfo.
     */
    static BindInfo vbr_flowfilterentry_bind_info[];

    /**
     * @brief  Member Variable for VbrFlowfilterEntryMainTblBindInfo.
     */
    static BindInfo vbr_flowfilter_entry_maintbl_bind_info[];

    /**
     * @brief  Member Variable for FlowListRenameBindInfo.
     */
    static BindInfo vbr_flowlist_rename_bind_info[];

  public:
    /**
     * @brief  Validates the Attribute of a Particular Class.
     *
     * @param[in]  ikey  This contains the Class to which the fields have
     *                   to be validated.
     * @param[in]  dmi   Pointer to DalDmlIntf(DBInterface)
     *
     * @retval  UPLL_RC_SUCCESS      Successfull Completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);

    upll_rc_t VerifyRedirectDestination(ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi,
                                        upll_keytype_datatype_t dt_type);

    /**
     * @Brief  Validates the syntax of the specified key and value structure
     *         for KT_VBR_FLOWFILTER_ENTRY keytype
     *
     * @param[in]  IpcReqRespHeader contains first 8 fields of input
     *             request structure
     * @param[in]  ConfigKeyVal key and value structure.
     *
     * @retval  UPLL_RC_SUCCESS               Successful.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX        Syntax error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  key_vbr_flowfilter_entry is not available.
     * @retval  UPLL_RC_ERR_GENERIC           Generic failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
     */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *key);

    /**
     * @Brief  Validates the syntax for KT_VBR_FLOWFILTER_ENTRY keytype
     *         key structure.
     *
     * @param[in]  key_vtn_flowfilter_entry  KT_VBR_FLOWFILTER_ENTRY key structure.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
    upll_rc_t ValidateVbrFlowfilterEntryKey(
         key_vbr_flowfilter_entry_t *key_vbr_flowfilter_entry,
         unc_keytype_operation_t operation);

    upll_rc_t GetControllerDomainID(ConfigKeyVal *ikey,
                                    controller_domain *ctrlr_dom,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf *dmi);
    /**
     * @Brief  Validates the syntax for val_flowfilter_entry structure.
     *
     * @param[in]  key_vbr_flowfilter_entry  key structure.
     * @param[in]  operation                 operation code
     * @param[in]  action_valid              refers valid flag value of action set in DB
     * @param[in]  action_redirect           holds true if action is set as REDIRECT in DB
     * @param[in]  modify_srcmac_valid       holds true if modify_srcmac is configured in DB
     * @param[in]  modify_srcmac_valid       holds true if modify_dstmac is configured in DB
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
    static upll_rc_t ValidateFlowfilterEntryValue(
          val_flowfilter_entry_t *val_flowfilter_entry, uint32_t operation);

    /**
     * @Brief  Validates the syntax for redirect_port, redirect_node fields of
     *         val_flowfilter_entry structure.
     *
     * @param[in]  val_flowfilter_entry  value structure
     * @param[in]  operation             operation name.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
    static upll_rc_t ValidateRedirectField(
               val_flowfilter_entry_t *val_flowfilter_entry,
                                    uint32_t operation);

    /**
     * @Brief  Validates the syntax for action field and checks whether
     *         Redirect fields and mac fields are filled only when action
     *         is REDIRECT of val_flowfilter_entry structure.
     *
     * @param[in]  val_flowfilter_entry  value structure
     * @param[in]  operation             operation name.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
    static upll_rc_t ValidateFlowfilterEntryAction(
           val_flowfilter_entry_t *val_flowfilter_entry, uint32_t operation);

    /**
     * @Brief  Checks if the specified key type and
     *         associated attributes are supported on the given controller,
     *         based on the valid flag.
     *
     * @param[in] val_flowfilter_entry  value structure
     * @param[in] attrs                 pointer to controller attribute.
     *
     * @retval  UPLL_RC_SUCCESS                     validation succeeded.
     * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR  Attribute NOT_SUPPORTED.
     * @retval  UPLL_RC_ERR_GENERIC                 Generic failure.
     */
    static upll_rc_t ValFlowFilterEntryAttributeSupportCheck(
      val_flowfilter_entry_t *val_flowfilter_entry, const uint8_t* attrs);

    /**
     * @brief  VbrFlowFilterEntryMoMgr Class Constructor.
     */
    VbrFlowFilterEntryMoMgr();

    /**
     * @brief  VbrFlowFilterEntryMoMgr Class Destructor.
     */
    ~VbrFlowFilterEntryMoMgr() {
      for (int i = 0; i < ntable; i++) {
        if (table[i]) {
          delete table[i];
        }
      }
      delete[] table;
    }

    /**
     * @Brief  Validates operation and datatype to validate syntax for
     *         KT_VBR_FLOWFILTER_ENTRY/KT_VBRIF_FLOWFILTER_ENTRY/KT_VRTIF_FLOWFILTER_ENTRY
     *         keytype key structure.
     *
     * @param[in]  val_flowfilter_entry  value structure.
     * @param[in]  IpcReqRespHeader      contains first 8 fields of input
     *                                   Request Structure
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
     upll_rc_t ValidateValFlowfilterEntry(IpcReqRespHeader *req,
           ConfigKeyVal *key);

    /**
     * @Brief  Checks if the specified key type(KT_VBR_FLOWFILTER_ENTRY) and
     *         associated attributes are supported on the given controller,
     *         based on the valid flag
     *
     * @param[in]  IpcReqRespHeader  contains first 8 fields of input request structure
     * @param[in]  ConfigKeyVal      contains key and value structure.
     * @param[in]  ctrlr_name        Describes the Controller Name.
     *
     * @retval  UPLL_RC_SUCCESS              Validation succeeded.
     * @retval  UPLL_RC_ERR_GENERIC          Validation failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1  Option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2  Option2 is not valid.
     */
     upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                   const char* ctrlr_name = NULL);

    /**
     * @brief  Methods Used for getting Value Attribute.
     *
     * @param[out]  valid    Describes the Valid Attribute.
     * @param[in]   val      This Contains the pointer to the class
     *                       for which iValid has to be checked.
     * @param[in]   indx     Describes the Index of the Attribute.
     * @param[in]   dt_type  Describes Configiration Information.
     * @param[in]   tbl      Describes the Destination table Information.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */

    upll_rc_t GetValid(void *val, uint64_t indx, uint8_t *&valid,
                       upll_keytype_datatype_t dt_type, MoMgrTables tbl);

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
     * @brief  Allocates Memory for the Incoming Pointer to the Class.
     *
     * @param[out]  ck_val   This Contains the pointer to the Class for whichi
     *                       memory has to be allocated.
     * @param[in]   dt_type  Describes Configiration Information.
     * @param[in]   tbl      Describes the Destination table Information.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl);

    /**
     * @brief  Method Updates the ConfigStatus for AuditConfigiration.
     *
     * @param[out]  ckv_running  This Contains the pointer to the Class
     *                           for which Audit ConfigStatus is Updated.
     * @param[in]   cs_status    Describes CsStatus Infomation.
     *                           Information.
     * @param[in]   phase        Describes the Phase of the Operation.
     * @param[in]      dmi          Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
     upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                       uuc::UpdateCtrlrPhase phase,
                                       ConfigKeyVal *&ckv_running,
                                       DalDmlIntf *dmi);

    /**
     * @brief  Method used to fill the CongigKeyVal with the Parent
     *         Class Information.
     *
     * @param[out]  okey        This Contains the pointer to the ConfigKeyVal
     *                          Class for which fields have to be updated with
     *                          values from the parent Class.
     * @param[in]   parent_key  This Contains the pointer to the ConfigKeyVal
     *                          Class which is the Parent Class used to fill
     *                          the details.
     *
     * @return  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);

    /**
     * @brief  Method used to get the RenamedUncKey.
     *
     * @param[out]  ctrlr_key   This Contains the pointer to the Class for which
     *                          fields have to be updated with values from the
     *                          parent Class.
     * @param[in]   dt_type     Describes Configiration Information.
     * @param[in]   dmi         Pointer to DalDmlIntf Class.
     * @param[in]   ctrlr_id    Describes the Controller Name.
     *
     * @return  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                               upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                               uint8_t *ctrlr_id);

    /**
     * @brief  Method used to Duplicate the ConfigkeyVal.
     *
     * @param[out]  okey  This Contains the pointer to the Class for
     *                    which fields have to be updated with values
     *                    from the Request.
     * @param[in]   req   This Contains the pointer to the Class which
     *                    is used for the Duplication.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                              MoMgrTables tbl);

    /**
     * @brief  Method used for Validation before Merge.
     *
     * @param[in]  ikey      This Contains the pointer to the Class for which
     *                       fields have to be Validated before the Merge.
     * @param[in]  keytype   Describes the keyType Information.
     * @param[in]  dmi       Pointer to DalDmlIntf Class.
     * @param[in]  ctrlr_id  Describes the Controller Name.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
                            ConfigKeyVal *ikey, DalDmlIntf *dmi,
                            upll_import_type import_type);

    /**
     * @brief  Method used for Rename Operation.
     *
     * @param[in]  req       Describes RequestResponderHeaderClass.
     * @param[in]  ikey      Pointer to ConfigKeyVal Class.
     * @param[in]  dmi       Pointer to DalDmlIntf Class.
     * @param[in]  ctrlr_id  Describes the Controller Name.
     *
     * @retval  UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT  Opearation Not Allowed for
     *                                               this KT.
     */
    upll_rc_t RenameMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                       DalDmlIntf *dmi, const char *ctrlr_id);

    /**
     * @brief  Method used for RenamedControllerkey(PfcName).
     *
     * @param[out]  ikey      Contains the Pointer to ConfigkeyVal Class
     *                        and contains the Pfc Name.
     * @param[in]   dt_type   Describes Configiration Information.
     * @param[in]   dmi       Pointer to DalDmlIntf Class.
     * @param[in]   ctrlr_id  Describes the Controller Name.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi,
                                      controller_domain *ctrlr_dom);

    /**
     * @brief  Method used for Trasaction UpdateConfig Status Operation.
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
    upll_rc_t UpdateConfigStatus(ConfigKeyVal *key, unc_keytype_operation_t op,
                                 uint32_t driver_result, ConfigKeyVal *upd_key,
                                 DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key);

    /**
     * @brief  Method used for Creating Object in Candidate Configiration.
     *
     * @param[in]  req   Describes RequestResponderHeaderClass.
     * @param[in]  ikey  Pointer to ConfigKeyVal Class.
     * @param[in]  dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t CreateCandidateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                DalDmlIntf *dmi);

    /**
     * @brief  Method used for Read Operation.
     *
     * @param[in]      req   Describes RequestResponderHeaderClass.
     * @param[in,out]  ikey  Pointer to ConfigKeyVal Class.
     * @param[in]      dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);

    /**
     * @brief  Method used for Reference Count Updation.
     *
     * @param[in]  ikey     Pointer to ConfigKeyVal Class.
     * @param[in]  dt_type  Describes the Configiration Information.
     * @param[in]  ObjType  Describes the Type if the Object.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t IsReferenced(ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
                           DalDmlIntf *dmi, int ObjType,
                           unc_keytype_operation_t op);

    /**
     * @brief  Method used for Delete Operation.
     * @param[in]  req   Describes RequestResponderHeaderClass.
     * @param[in]  ikey  Pointer to ConfigKeyVal Class.
     * @param[in]  dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                       DalDmlIntf *dmi);

    /**
     * @brief  Method used to fill the CongigKeyVal with the Parent
     *         Class Information.
     *
     * @param[out]  okey     This Contains the pointer to the ConfigKeyVal
     *                       Class for which fields have to be updated with
     *                       values from the parent Class.
     * @param[in]   iKey     This Contains the pointer to the ConfigKeyVal
     *                       Class which is the Parent Class used to fill
     *                       the details.
     * @param[in]   ObjType  Describes the type of the Object.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t GetObjectConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&ikey,
                                    int ObjType);

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
    * @brief  Method Used for Getting the Controller Id.
    *
    * @param[in]  ikey  Pointer to ConfigKeyVal Class.
    * @param[in]  dmi   Pointer to DalDmiIntf Class
    *
    * @retval  UPLL_RC_SUCCESS      Successfull Operation.
    * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
    */
    upll_rc_t GetControllerId(ConfigKeyVal *ikey,
                              DalDmlIntf *dmi);

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
     * @param[out]  val1   Pointer to ConfigKeyVal Class which contains only Valid Attributes
     * @param[in]   val2   Pointer to ConfigKeyVal Class.
     * @param[in]   audit  If true,Audit Process.
     *
     * @return No Return Value;
     */
    bool CompareValidValue(void *&val1, void *val2, bool copy_to_running);

    /**
     * @brief  Method used for Restoring FlowList in the Controller Table
     *
     * @param[in]      ikey       Pointer to ConfigKeyVal Class
     * @param[in]      dt_type    Describes Configiration Information.
     * @param[in]      tbl        Describe the destination table
     * @param[in]      dmi        Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
     * @retval  UPLL_RC_ERR_INSTANCE_EXISTS       Record already exists
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */

    upll_rc_t RestorePOMInCtrlTbl(ConfigKeyVal *ikey,
                                  upll_keytype_datatype_t dt_type,
                                  MoMgrTables tbl,
                                  DalDmlIntf* dmi);
    upll_rc_t ReadSiblingMo(IpcReqRespHeader *req,
                            ConfigKeyVal *ikey,
                            bool begin,
                            DalDmlIntf *dmi);

    upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                 ConfigKeyVal *ikey);
    upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                                DalDmlIntf *dmi,
                                const char *ctrlr_id);
    /**
     * @brief  Method to check validity of Key
     *
     * @param[in]   ConfigKeyVal  input COnfigKeyVal
     * @param[out]  index          DB Column Index
     *
     * @return  TRUE   Success
     * @retval  FALSE  Failure
     **/
    bool IsValidKey(void *key, uint64_t index, MoMgrTables tbl = MAINTBL);
    upll_rc_t ConstructReadDetailResponse(ConfigKeyVal *ikey,
                                          ConfigKeyVal *drv_resp_ckv,
                                          controller_domain ctrlr_dom,
                                          DalDmlIntf *dmi ,
                                          ConfigKeyVal **okey);

    upll_rc_t DeleteChildrenPOM(ConfigKeyVal *ikey,
                                upll_keytype_datatype_t dt_type,
                                DalDmlIntf *dmi,
                                TcConfigMode config_mode,
                                string vtn_name);

    upll_rc_t SetValidAudit(ConfigKeyVal *&ikey);
    upll_rc_t SetRedirectDestination(
                        ConfigKeyVal *okey ,
                        upll_keytype_datatype_t dt_type,
                        DalDmlIntf *dmi,
          val_flowfilter_entry_t* val_flowfilter_entry);
    upll_rc_t UpdateVnodeVal(ConfigKeyVal *ikey, DalDmlIntf *dmi,
                             upll_keytype_datatype_t data_type,
                             bool &no_rename);

    bool FilterAttributes(void *&val1,
                          void *val2,
                          bool copy_to_running,
                          unc_keytype_operation_t op);

    upll_rc_t GetFlowlistConfigKey(
          const char *flowlist_name, ConfigKeyVal *&okey,
          DalDmlIntf *dmi);

    upll_rc_t SetRenameFlag(ConfigKeyVal *ikey,
          DalDmlIntf *dmi,
          IpcReqRespHeader *req);

    static bool IsAllAttrInvalid(val_flowfilter_entry_t *val);

    /**
     * @brief     Perform validation on key type specific,
     *            before sending to driver
     *
     * @param[in]  ck_new                   Pointer to the ConfigKeyVal Structure
     * @param[in]  ck_old                   Pointer to the ConfigKeyVal Structure
     * @param[in]  op                       Operation name.
     * @param[in]  dt_type                  Specifies the configuration CANDIDATE/RUNNING
     * @param[in]  keytype                  Specifies the keytype
     * @param[in]  dmi                      Pointer to the DalDmlIntf(DB Interface)
     * @param[out] not_send_to_drv          Decides whether the configuration needs
     *                                      to be sent to controller or not
     * @param[in]  audit_update_phase       Specifies whether the phase is commit or audit
     *
     * @retval  UPLL_RC_SUCCESS             Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC         Generic failure.
     * @retval  UPLL_RC_ERR_CFG_SEMANTIC    Failure due to semantic validation.
     * @retval  UPLL_RC_ERR_DB_ACCESS       DB Read/Write error.
     *
     */
    upll_rc_t AdaptValToDriver(ConfigKeyVal *ck_new,
        ConfigKeyVal *ck_old,
        unc_keytype_operation_t op,
        upll_keytype_datatype_t dt_type,
        unc_key_type_t keytype,
        DalDmlIntf *dmi,
        bool &not_send_to_drv,
        bool audit_update_phase);
};
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // MODULES_UPLL_VBR_FLOWFILTER_ENTRY_MOMGR_HH_
