/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UPLL_POLICINGPROFILE_ENTRY_MOMGR_HH
#define _UPLL_POLICINGPROFILE_ENTRY_MOMGR_HH_

#include <string>
#include <set>
#include "momgr_impl.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

/* This file declares interfaces for keyType KT_POLICINGPROFILE_ENTRY */
/*
 * @brief  PolicingProfileEntryMoMgr class handles all the request
 *         received from service.
 */

class PolicingProfileEntryMoMgr: public MoMgrImpl {
  private:
    /**
     * Member Variable for PolicingProfileEntryBindInfo.
     */
    static BindInfo       policingprofileentry_bind_info[];
    /**
     * Member Variable for PolicingProfileEntryBindInfo.
     */
    static BindInfo       policingprofileentry_controller_bind_info[];
    /**
     * Member Variable for PolicingProfileEntryBindInfo.
     */
    static BindInfo       rename_policingprofile_entry_main_tbl[];
    /**
     * Member Variable for PolicingProfileEntryBindInfo.
     */
    static BindInfo       rename_policingprofile_entry_ctrl_tbl[];
    /**
     * Member Variable for PolicingProfileEntryBindInfo.
     */
    static BindInfo       rename_flowlist_pp_entry_main_tbl[];
    /**
     * Member Variable for PolicingProfileEntryBindInfo.
    */
    static BindInfo       rename_flowlist_pp_entry_ctrl_tbl[];
    /**
     * @brief  Validates the Attribute of a Particular Class.
     *
     * @param[in]  kval  This contains the Class
     *                   to which the fields have to be validated.
     * @param[in]  dmi   Db Interface Pointer
     */
    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);

    /**
     * @Brief  Validates the syntax of rate, cir, cbs, pir, pbs
     *         fields in KT_POLICING_PROFILE_ENTRY value structure.
     *
     * @param[in]  val_policingprofile_entry  KT_POLICING_PROFILE_ENTRY
     *                                        value structure.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
    upll_rc_t ValidateRate(
       val_policingprofile_entry_t *val_policingprofile_entry,
       uint32_t operation);

    /**
     * @Brief  Validates the syntax of red_action,green_action,yellow_action
     *         fields in KT_POLICING_PROFILE_ENTRY  value structure.
     *
     * @param[in]  val_policingprofile_entry  KT_POLICING_PROFILE_ENTRY
     *                                        value structure.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
    upll_rc_t ValidateColorAction(
       val_policingprofile_entry_t *val_policingprofile_entry,
       uint32_t operation);

    /**
     * @Brief  Validates the syntax ofred_action_priority,green_action_priority,
     *         yellow_action_priority fields in KT_POLICING_PROFILE_ENTRY
     *         value structure.
     *
     * @param[in]  val_policingprofile_entry  KT_POLICING_PROFILE_ENTRY
     *                                        value structure.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
    upll_rc_t  ValidateColorPriority(
       val_policingprofile_entry_t *val_policingprofile_entry,
       uint32_t operation);

    /**
     * @Brief  Validates the syntax of Precedence for red,green,yellow
     *         in KT_POLICING_PROFILE_ENTRY value structure.
     *
     * @param[in]  val_policingprofile_entry  KT_POLICING_PROFILE_ENTRY
     *                                        value structure.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
    upll_rc_t  ValidateColorPrecedence(
       val_policingprofile_entry_t *val_policingprofile_entry,
       uint32_t operation);

    /**
     * @Brief  Validates the syntax of dscp for red,green,yellow
     *         in KT_POLICING_PROFILE_ENTRY value structure.
     *
     * @param[in]  val_policingprofile_entry  KT_POLICING_PROFILE_ENTRY
     *                                        value structure.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
    upll_rc_t  ValidateColorDscp(
       val_policingprofile_entry_t *val_policingprofile_entry,
       uint32_t operation);

    /**
     * @Brief  Validates syntax of policingprofile name and checks whether it
     *         exists in KT_POLICINGPROFILE table, if not exists sends
	           SEMANTIC error.
     *
     * @param[in]  policing_profile_name  policingprofile_name.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  syntax validation failed.
     * @retval  UPLL_RC_ERR_SEMANTIC    name is not exists in table
     */
    upll_rc_t ValidatePolicingProfileName(ConfigKeyVal *ikey, DalDmlIntf *dmi,
        IpcReqRespHeader *req);

    /**
     * @Brief  Validates the syntax of the specified key and value structure
     *         for KT_POLICING_PROFILE_ENTRY keytype
     *
     * @param[in]  IpcReqRespHeader  contains first 8 fields of input request structure
     * @param[in]  ConfigKeyVal      key and value structure.
     *
     * @retval  UPLL_RC_SUCCESS               Successful.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX        Syntax error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  key_flowlist_entry is not available.
     * @retval  UPLL_RC_ERR_GENERIC           Generic failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
     */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *key);

    /**
     * @Brief  Validates the syntax for KT_POLICING_PROFILE_ENTRY keytype
     *         key structure.
     *
     * @param[in]  key_policingprofile_entry  KT_POLICING_PROFILE_ENTRY
	                                      key structure.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
     upll_rc_t ValidatePolicingProfileEntryKey(
           ConfigKeyVal *key, uint32_t operation);

    /**
     * @Brief  Validates the syntax for KT_POLICING_PROFILE_ENTRY
               keytype value structure.
     *
     * @param[in]  key_policingprofile_entry  KT_POLICING_PROFILE_ENTRY
     *                                        key structure.
     * @param[in]  val_policingprofile_entry  KT_POLICING_PROFILE_ENTRY
     *	                                      value structure.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */

    upll_rc_t ValidatePolicingProfileEntryValue(
        ConfigKeyVal *ikey, DalDmlIntf *dmi,
        IpcReqRespHeader *req);

    /**
     * @Brief  Checks if the specified key type(KT_POLICING_PROFILE_ENTRY) and
     *         associated attributes are supported on the given controller,
     *         based on the valid flag.
     *
     * @param[in]  IpcReqRespHeader  contains first 8 fields of input request
     *	                             structure
     * @param[in]  ConfigKeyVal      contains key and value structure.
     * @param[in]   ctrlr_name       controller_name
     *
     * @retval  UPLL_RC_SUCCESS              Validation succeeded.
     * @retval  UPLL_RC_ERR_GENERIC          Validation failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1  Option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2  Option2 is not valid.
     */
    upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                   const char* ctrlr_name = NULL);

    /**
     * @Brief  Checks if the specified key type and
     *         associated attributes are supported on the given controller,
     *         based on the valid flag.
     *
     * @param[in] val_policingprofile_entry  KT_POLICINGPROFILE_ENTRY value structure.
     * @param[in] attrs                      pointer to controller attribute
     *
     * @retval  UPLL_RC_SUCCESS                     validation succeeded.
     * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR  Attribute NOT_SUPPORTED.
     */
    upll_rc_t ValPolicingProfileEntryAttributeSupportCheck(
        val_policingprofile_entry_t *val_policingprofile_entry,
       const uint8_t* attrs);

    /**
     * @Brief  Checks if the specified green_action, green_action_priority,
     *         green_action_dscp, green_action_drop_precedence  attributes are
     *         supported on the given controller,  based on the valid flag.
     *
     * @param[in]  val_policingprofile_entry  value structure.
     * @param[in]  attrs                      Refers supported capability file attributes
     *                                        for the given controller name.
     */
     void ValidateGreenFieldAttribute(val_policingprofile_entry_t *
      val_policingprofile_entry, const uint8_t *attrs);

    /**
     * @Brief  Checks if the specified yellow_action, yellow_action_priority,
     *         yellow_action_dscp, yellow_action_drop_precedence  attributes are
     *         supported on the given controller,  based on the valid flag.
     *
     * @param[in]  val_policingprofile_entry  value structure.
     * @param[in]  attrs                      Refers supported capability file
     *                                        attributes for the given controller name.
     */
      void ValidateYellowFieldAttribute(val_policingprofile_entry_t *
        val_policingprofile_entry, const uint8_t *attrs);

    /**
     * @Brief  Checks if the specified red_action, red_action_priority,
     *         red_action_dscp, red_action_drop_precedence  attributes are
     *         supported on the given controller,  based on the valid flag.
     *
     * @param[in]  val_policingprofile_entry  value structure.
     * @param[in]  attrs                      Refers supported capability file attributes
     */
     void ValidateRedFieldAttribute(val_policingprofile_entry_t *
      val_policingprofile_entry, const uint8_t *attrs);

    /**
     * @Brief  Validates the policingprofile_name in key_sttruct exists in
     *         VTN/VBR/VBRIF policingmap table, if exists UPDATE/DELETE
	 *         opearations are not allowed for val struct fields .
     *
     * @param[in]  policingprofile_name  name.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     */
    upll_rc_t ValidatePolicingProfileEntryInPolicingMap(ConfigKeyVal *ikey,
                                                        DalDmlIntf *dmi,
                                                        IpcReqRespHeader *req);


  public:
    /**
     * @brief  PolicingProfileEntryMoMgr Class Constructor.
     */
    PolicingProfileEntryMoMgr();
    /**
     * @brief  PolicingProfileEntryMoMgr Class Destructor.
     */
    ~PolicingProfileEntryMoMgr() {
      for (int i = 0; i < ntable; i++) {
        if (table[i]) {
          delete table[i];
        }
      }
      delete[] table;
    }

    /**
     * @brief  Validation or the Incoming Attributes of the Class.
     *
     * @param[out]  val      This Contains the pointer to the Class
                             for which memory has to be allocated.
     * @param[in]   indx     Describes the Attributes.
     * @param[in]   valid    Describes the Pointer to attributes of Class
     * @param[in]   dt_type  Describes Configiration Information.
     * @param[in]   tbl      Describes the Destination table Information.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t GetValid(void *val,
                    uint64_t indx,
                    uint8_t *&valid,
                    upll_keytype_datatype_t dt_type,
                    MoMgrTables tbl);
    /**
     * @brief  Allocates Memory for the Incoming Pointer to the Class.
     *
     * @param[out]  ck_val   This Contains the pointer to the Class
                             for which memory has to be allocated.
     * @param[in]   dt_type  Describes Configiration Information.
     * @param[in]   tbl      Describes the Destination table Information.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t AllocVal(ConfigVal *&ck_val,
                    upll_keytype_datatype_t dt_type,
                     MoMgrTables tbl);

    /**
     * @brief  Method Updates the ConfigStatus for AuditConfigiration.
     *
     * @param[in]   cs_status    either UNC_CS_INVALID or UNC_CS_APPLIED.
     * @param[in]   phase        Describes Commit Control Status
                                 Information.
     * @param[in]   ckv_running  Describes the Response Code.
     * @param[in]   dmi          Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */
    upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                      uuc::UpdateCtrlrPhase phase,
                                      ConfigKeyVal *&ckv_running,
                                      DalDmlIntf *dmi);

    /**
     * @brief  Method used to fill the CongigKeyVal with the Parent Class
               Information.
     *
     * @param[out]  okey        This Contains the pointer to the
     *                          ConfigKeyVal Class for which fields have to be
     *                          updated with values from the parent Class.
     * @param[in]   parent_key  This Contains the pointer to the ConfigKeyVal
     *                          Class which is the Parent Class used
     *                          to fill the details.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
                             ConfigKeyVal *parent_key);

    /**
     * @brief  Method used to get the RenamedUncKey.
     *
     * @param[out]  ctrlr_key  This Contains the pointer to the Class for which
     *                         fields have to be updated with values
                               from the parent Class.
     * @param[in]   dt_type    Describes Configiration Information.
     * @param[in]   dmi        Pointer to DalDmlIntf Class.
     * @param[in]   ctrlr_id   Describes the Controller Name.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                            upll_keytype_datatype_t dt_type,
                            DalDmlIntf *dmi, uint8_t *ctrlr_id);

    /**
     * @brief  Method used to Duplicate the ConfigkeyVal.
     *
     * @param[out]  okey  This Contains the pointer to the
     *                    Class for which fields have to be
                          updated with values from the Request.
     * @param[in]   req   This Contains the pointer to the Class
                          which is used for the Duplication .
     * @param[in]   tbl   Describes the Destination tables.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey,
                           ConfigKeyVal *&req,
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
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t MergeValidate(unc_key_type_t keytype,
                            const char *ctrlr_id,
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
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t RenameMo(IpcReqRespHeader *req,
                    ConfigKeyVal *ikey,
                    DalDmlIntf *dmi, const char *ctrlr_id);


    /**
     * @brief  Method used for IsReferenced.
     *
     * @param[in]  ikey     Contains the Pointer to ConfigkeyVal Class.
     * @param[in]  dt_type  Describes Configiration Information.
     * @param[in]  dmi      Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */
    upll_rc_t IsReferenced(IpcReqRespHeader *req,
                    ConfigKeyVal *ikey,
                    DalDmlIntf *dmi);

    /**
     * @brief  Method used for RenamedControllerkey(PfcName).
     *
     * @param[out]  ikey        Contains the Pointer to ConfigkeyVal Class
                                and contains the Pfc Name.
     * @param[in]   dt_type     Describes Configiration Information.
     * @param[in]   dmi         Pointer to DalDmlIntf Class.
     * @param[in]   ctrlr_name  Describes the Controller Name.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
                                   DalDmlIntf *dmi,
                                   controller_domain *ctrlr_dom = NULL);

    /**
     * @brief  Method used for TxCopyCandidateToRunning.
     *
     * @param[in]  keytype              Describes the followong keytype
                                        undergoing the operation.
     * @param[in]  ctrlr_commit_status  Pointer to the CtrlrCommitStatusList Class
     * @param[in]  dmi                  Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RT_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No Recorde Exists in database.
     */

    upll_rc_t TxCopyCandidateToRunning(unc_key_type_t keytype,
                                    CtrlrCommitStatusList *ctrlr_commit_status,
                                    DalDmlIntf *dmi, TcConfigMode config_mode,
                                    std::string vtn_name);

    /**
     * @brief      Method used to Update the Values in the specified key type.
     *
     * @param[in]  req                        contains first 8 fields of input request structure
     * @param[in]  ikey                       key and value structure
     * @param[in]  dmi                        Pointer to DalDmlIntf Class.
     *
     * @retval     UPLL_RC_SUCCESS            Successfull completion.
     * @retval     UPLL_RC_ERR_GENERIC        Failure case.
     */
    upll_rc_t UpdateMo(IpcReqRespHeader *req,
                             ConfigKeyVal *ikey,
                             DalDmlIntf *dmi);

    /**
     * @brief  Method used for UpdateConfigStatus.
     *
     * @param[in]  key            Pointer to ConfigKeyVal Class.
     * @param[in]  op             Specifies the Type of Opeartion.
     * @param[in]  driver_result  Describes the result of Driver Operation.
     * @param[in]  nreq           Pointer to ConfigKeyVal Class.
     * @param[in]  ctrlr_key      Pointer to ConfigKeyVal Class.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t UpdateConfigStatus(ConfigKeyVal *key,
                              unc_keytype_operation_t op,
                              uint32_t driver_result,
                              ConfigKeyVal *nreq,
                              DalDmlIntf   *dmi,
                              ConfigKeyVal *ctrlr_key);

    /**
     * @brief  Method used for Read Operation.
     *
     * @param[in]  ikey  Pointer to ConfigKeyVal Class.
     * @param[in]  req   Pointer to IpcReqRespHeader.
     * @param[in]  dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t ReadMo(IpcReqRespHeader *req,
                  ConfigKeyVal *ikey,
                  DalDmlIntf *dmi);

    /**
     * @brief  Method used for ReadSibling Operation.
     *
     * @param[in]  ikey  Pointer to ConfigKeyVal Class.
     * @param[in]  req   Pointer to IpcReqRespHeader.
     * @param[in]  dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t ReadSiblingMo(IpcReqRespHeader *req,
                         ConfigKeyVal *ikey,
                         DalDmlIntf *dmi);

    /**
     * @brief  Method used for Trasaction Vote Operation.
     *
     * @param[in]  ikey  Pointer to ConfigKeyVal Class.
     * @param[in]  op    Describes the Type of Opeartion.
     * @param[in]  req   Pointer to IpcReqRespHeader.
     * @param[in]  dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RT_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No Recorde Exists in database.
     */

    upll_rc_t ReadRecord(IpcReqRespHeader *req,
                      ConfigKeyVal *ikey,
                      DalDmlIntf *dmi, unc_keytype_operation_t op);

    /**
     * @brief  Method GetPolicingprofileKeyVal used for checking
               the refernce count for policngprofile object .

     * @param[out]  okey        Contains the Pointer to ConfigkeyVal Class
     *                          and contains the Pfc Name.
     * @param[in]   ikey        Describes Configiration Information.
     * @param[in]   ObjType     Describes the Object type .
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t GetPolicingprofileKeyVal(ConfigKeyVal *&okey,
                          ConfigKeyVal *&ikey);

    /**
     * @brief  Method to compare to keys
     *
     * @param[in]  key1  Pointer to key structure for comparision
     * @param[in]  key2  Pointer to key for comparision
     *
     * @return     TRUE  Successfull completion.
     */

    bool CompareKey(ConfigKeyVal *key1, ConfigKeyVal *key2);

    /**
     * @brief  Method to compare for validity for values.
     *
     * @param[in]  val1  Pointer to key structure for comparision
     * @param[in]  val2  Pointer to key for comparision
     *
     * @return     TRUE  Successfull completion.
     */

    bool CompareValidValue(void *&val1, void *val2, bool copy_to_running);

    /**
     * @brief  Method GetControllerDomainSpan.
     *
     * @param[out]  ikey     Contains the Pointer to ConfigkeyVal Class
     * @param[in]   dt_type  Describes Datatype.
     * @param[in]   dmi      Describes the Objct type .
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t GetControllerDomainSpan(ConfigKeyVal *ikey,
                         upll_keytype_datatype_t dt_type, DalDmlIntf *dmi);

    /**
     * @brief  Method CopyToConfigKey.
     *
     * @param[out]  ikey     Contains the Pointer to ConfigkeyVal Class
     * @param[in]   dt_type  Describes Datatype.
     * @param[in]   dmi      Describes the Objct type .
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
        ConfigKeyVal *ikey);

    /**
     * @brief  Method GetRenameKeyBindInfo.
     *
     * @param[out]  ikey     Contains the Pointer to ConfigkeyVal Class
     * @param[in]   dt_type  Describes Datatype.
     * @param[in]   dmi      Describes the Objct type .
     *
     * @return     TRUE  Successfull completion.
     */

    bool GetRenameKeyBindInfo(unc_key_type_t key_type,
        BindInfo *&binfo, int &nattr, MoMgrTables tbl);

    /**
     * @brief  Method UpdatePolicingProfileEntryRenamed.
     *
     * @param[out]  ikey     Contains the Pointer to ConfigkeyVal Class
     * @param[in]   dt_type  Describes Datatype.
     * @param[in]   dmi      Describes the Objct type .
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */

    upll_rc_t UpdatePolicingProfileEntryRenamed(
      ConfigKeyVal *rename_info,
      DalDmlIntf *dmi, upll_keytype_datatype_t data_type);

    /**
     * @brief  Method to get Parent ConfigKeyVal
     *
     * @param[in]   parent_key  parent ConfigKeyVal
     * @param[out]  okey        Output ConfigKeyVal
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     */
    upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                 ConfigKeyVal *ikey);

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
     * @brief  Method to Delete record in ctrlrtbl
     *
     * @param[in]  ppe_ckv   Pointer to ConfigKeyVal
     * @param[in]  dt_type  Data Type.
     * @param[in]  dmi      Pointer to DalDmlIntf.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t CtrlrTblDelete(ConfigKeyVal *ppe_ckv,
      DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
      TcConfigMode config_mode, string vtn_name,
      bool commit);

    /**
     * @brief  Method to Create or Delete record in ctrlrtbl
     *
     * @param[in]  policingprofile_name  Policingprofile name
     * @param[in]  ctrlr_id              Controller name.
     * @param[in]  oper                  Create or delete operation
     * @param[in]  dmi                   Pointer to DalDmlIntf.
     * @param[in]  dt_type               Data Type.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t PolicingProfileEntryCtrlrTblOper
      (const char *policingprofile_name, const char *ctrlr_id,
       DalDmlIntf *dmi, unc_keytype_operation_t oper,
       upll_keytype_datatype_t dt_type,
       TcConfigMode config_mode, string vtn_name, bool commit);

    /**
     * @brief  Method to Get Policingprofile ConfigKeyVal
     *
     * @param[in]  ppe_ckv                Pointer to ConfigKeyVal
     * @param[in]  policingprofile_name  policingprofile name.
     * @param[in]  ctrlr_id              controller name.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t GetPolicingProfileEntryCtrlrKeyval(
        ConfigKeyVal *&ppe_keyval,
        const char *policingprofile_name,
        const char *ctrlr_id);

    /**
     * @brief  Method to Create record in ctrlrtbl
     *
     * @param[in]  ppe_ckv   Pointer to ConfigKeyVal
     * @param[in]  dt_type   Data Type.
     * @param[in]  dmi       Pointer to DalDmlIntf.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t CtrlrTblCreate(ConfigKeyVal *ppe_ckv,
        DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
        TcConfigMode config_mode, string vtn_name,
        bool commit);

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
     * @brief  Method to Read policingprofileentry avl structure
     *
     * @param[in]  policingprofile_name  Policingprofile name
     * @param[in]  seq_num               sequence number
     * @param[in]  ctrlr_id              Controller name.
     * @param[in]  dmi                   Pointer to DalDmlIntf.
     * @param[in]  dt_type               Data Type.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t ReadPolicingProfileEntry(
      const char *policingprofile_name, uint8_t seq_num,
      const char *ctrlr_id, DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
      ConfigKeyVal *&ppe_ckv, unc_keytype_option1_t opt1 = UNC_OPT1_NORMAL);

    upll_rc_t IsFlowlistConfigured(const char* flowlist_name,
      upll_keytype_datatype_t dt_type,
      DalDmlIntf *dmi);

    upll_rc_t ReadDetailEntry(
      ConfigKeyVal *ff_ckv, upll_keytype_datatype_t dt_type,
      DbSubOp dbop, DalDmlIntf *dmi);

    upll_rc_t ValidatePolicingprofileEntryVal(ConfigKeyVal *key,
                                              uint32_t operation,
                                              uint32_t datatype);

    upll_rc_t ValidateFlowList(
      ConfigKeyVal *ikey, DalDmlIntf *dmi,
      IpcReqRespHeader *req);

    upll_rc_t UpdateMainTbl(ConfigKeyVal *ppe_key,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi);

    upll_rc_t FilterAttributes(ConfigKeyVal *ckv_main,
                               ConfigKeyVal *ckv_ctrlr);

    upll_rc_t ValidateValidElements(
      const char *policingprofile_name, DalDmlIntf *dmi,
      upll_keytype_datatype_t dt_type);

    upll_rc_t IsFlowListMatched(const char *flowlist_name,
      upll_keytype_datatype_t dt_type, DalDmlIntf *dmi);

    upll_rc_t SetValidAudit(ConfigKeyVal *&ikey);

    upll_rc_t DecrementRefCount(
      ConfigKeyVal *ppe_ckv, DalDmlIntf *dmi,
      upll_keytype_datatype_t dt_type,
      TcConfigMode config_mode, string vtn_name);
    upll_rc_t UpdateVnodeVal(ConfigKeyVal *ikey,
                             DalDmlIntf *dmi,
                             upll_keytype_datatype_t data_type,
                             bool &no_rename);
    bool FilterAttributes(void *&val1,
                          void *val2,
                          bool copy_to_running,
                          unc_keytype_operation_t op);

    upll_rc_t Get_Tx_Consolidated_Status(
        unc_keytype_configstatus_t &status,
        unc_keytype_configstatus_t  drv_result_status,
        unc_keytype_configstatus_t current_cs,
        unc_keytype_configstatus_t current_ctrlr_cs);

    upll_rc_t GetFlowListEntryConfigKey(
        ConfigKeyVal *&okey, ConfigKeyVal *ikey);

    upll_rc_t SetRenameFlag(ConfigKeyVal *ikey,
                            DalDmlIntf *dmi,
                            IpcReqRespHeader *req);

    upll_rc_t GetFlowlistConfigKey(
        const char *flowlist_name, ConfigKeyVal *&okey,
        DalDmlIntf *dmi);

    upll_rc_t SetPPEntryConsolidatedStatus(ConfigKeyVal *ikey,
                                           uint8_t *ctrlr_id,
                                           DalDmlIntf *dmi);

    bool IsAllAttrInvalid(
        val_policingprofile_entry_t *val);

    upll_rc_t GetOperation(uuc::UpdateCtrlrPhase phase,
                           unc_keytype_operation_t &op);

    upll_rc_t ChkProfileNameInRenameTbl(ConfigKeyVal *ctrlr_key,
      upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, const char *ctrlr_id);

    upll_rc_t GetDomainsForController(
        ConfigKeyVal *ckv_drvr,
        ConfigKeyVal *&ctrlr_ckv,
        DalDmlIntf *dmi);

    bool IsAttributeUpdated(void *val1, void *val2);

    upll_rc_t IsFlowListMatched(
        const char* flowlist_name, DalDmlIntf *dmi,
        TcConfigMode config_mode, string vtn_name);

    upll_rc_t ClearVirtualKtDirtyInGlobal(DalDmlIntf *dmi);

    upll_rc_t UpdateRefCountInFl(
        const char *policingprofile_name, DalDmlIntf *dmi,
        upll_keytype_datatype_t dt_type, unc_keytype_operation_t op,
        TcConfigMode config_mode, string vtn_name);

    upll_rc_t DeleteChildrenPOM(ConfigKeyVal *ikey,
        upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
        TcConfigMode config_mode, string vtn_name);
};

typedef struct val_policingprofile_entry_ctrl {
  uint8_t valid[18];
  unc_keytype_configstatus_t cs_row_status;
  unc_keytype_configstatus_t cs_attr[18];
  uint8_t flags;
}val_policingprofile_entry_ctrl_t;

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // _UPLL_POLICINGPROFILE_ENTRY_MOMGR_HH_
