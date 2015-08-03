/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef MODULES_UPLL_VTN_POLICINGMAP_MOMGR_HH_
#define MODULES_UPLL_VTN_POLICINGMAP_MOMGR_HH_

#include <string>
#include <set>
#include "momgr_impl.hh"
namespace unc {
namespace upll {
namespace kt_momgr {

enum vtnpolicingmapMoMgrTables {
  VTNPOLICINGMAPTBL = 0, VTNPOLICINGMAPCTRLRTBL, NVTNPOLICINGMAPTABLES
};

/**
 * @Brief VtnPolicingMapMoMgr class.
 */
class VtnPolicingMapMoMgr : public MoMgrImpl {
 private:
  /**
   * Member Variable for VtnPolicingMapBinfInfo
   */
  static unc_key_type_t vtnpolicingmap_child[];
  static BindInfo vtnpolicingmap_bind_info[];
  static BindInfo vtnpolicingmap_controller_bind_info[];
  static BindInfo key_vtnpm_vtn_maintbl_rename_bind_info[];
  static BindInfo key_vtnpm_vtn_ctrlrtbl_rename_bind_info[];
  static BindInfo key_vtnpm_Policyname_maintbl_rename_bind_info[];
  static BindInfo key_vtnpm_Policyname_ctrlrtbl_rename_bind_info[];

 public:
  /**
   * @Brief VtnPolicingMapMoMgr Class Constructor.
   */
  VtnPolicingMapMoMgr();
  /**
   * @Brief VtnPolicingMapMoMgr Class Destructor.
   */
  ~VtnPolicingMapMoMgr() {
    for (int i = 0; i < ntable; i++) {
      if (table[i]) {
        delete table[i];
      }
    }
    delete[] table;
  }

  /**
   * @Brief This API is used to create the record (Vtn name with
   * Policer name) in VtnPolicingMap table and Increment the refcount
   * or insert the record based on the incoming policer name's availability
   * in policingprofilectrl table
   *
   * @param[in] req                          Describes
   *                                         RequestResponderHeaderClass.
   * @param[in] ikey                         Pointer to ConfigKeyVal Class.
   * @param[in] dmi                          Pointer to DalDmlIntf Class.
   *
   * @retval    UPLL_RC_ERR_INSTANCE_EXISTS  Policymap Record Already Exists.
   * @retval    UPLL_RC_ERR_CFG_SEMANTIC     Policy Profile Record not
   *                                         available.
   * @retval    UPLL_RC_SUCCESS              Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC          Generic Errors.
   */
  upll_rc_t CreateCandidateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                              DalDmlIntf *dmi);

  /**
   * @Brief This API is used to delete the record (Vtn name with
   * Policer name) in VtnPolicingMap table and decrement the refcount
   * based on the record availability in policingprofilectrl table
   *
   * @param[in] req                           Describes
   *                                          RequestResponderHeaderClass.
   * @param[in] ikey                          Pointer to ConfigKeyVal Class.
   * @param[in] dmi                           Pointer to DalDmlIntf Class.
   *
   * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  Record Not available.
   * @retval    UPLL_RC_SUCCESS               Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC           Generic Errors.
   */
  upll_rc_t DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);

  /**
   * @Brief This API is used to Update the record (Vtn name with
   * Policer name) in VtnPolicingMap table and Increment the refcount
   * or insert the record based on the incoming policer name's availability
   * in policingprofilectrl table. Get the existing policer name in
   * vtnpolicingmap table and decrement the refcount in policingprofilectrl
   * table in respective vtn associated controller name
   *
   * @param[in] req                           Describes
   *                                          RequestResponderHeaderClass.
   * @param[in] ikey                          Pointer to ConfigKeyVal Class.
   * @param[in] dmi                           Pointer to DalDmlIntf Class.
   *
   * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  Record Not available.
   * @retval    UPLL_RC_ERR_CFG_SEMANTIC      Policy Profile Record not
   *                                          available.
   * @retval    UPLL_RC_SUCCESS               Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC           Generic Errors.
   */
  upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);

  /**
   * @Brief This API is used to check the policymap object availability
   * in vtnpolicingmaptbl CANDIDATE DB
   *
   * @param[in] ikey                          Pointer to ConfigKeyVal Class.
   * @param[in] dt_type                       Configuration information.
   * @param[in] dmi                           Pointer to DalDmlIntf Class.
   * @retval    UPLL_RC_SUCCESS               Successful completion.
   *
   * @retval    UPLL_RC_ERR_GENERIC           Generic Error code.
   * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  No Record in DB.
   * @retval    UPLL_RC_ERR_INSTANCE_EXISTS   Record exists in DB.
   */
  upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                         DalDmlIntf *dmi);

  /**
   * @Brief This API is used to check the policer name availability
   * in policingprofiletbl CANDIDATE DB
   *
   * @param[in] ikey                          Pointer to ConfigKeyVal Class.
   * @param[in] dt_type                       Configuration information.
   * @param[in] dmi                           Pointer to DalDmlIntf Class.
   * @param[in] op                            Describes the Type of Opeartion.
   *
   * @retval    UPLL_RC_SUCCESS               Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC           Generic Error code.
   * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  No Record in DB.
   * @retval    UPLL_RC_ERR_INSTANCE_EXISTS   Record exists in DB.
   */
  upll_rc_t IsPolicyProfileReferenced(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf* dmi,
                                      unc_keytype_operation_t op);

  /**
   * @Brief This API is used to get the vtn associated controller name
   * and update (increment/decrement) the refcount in policingprofilectrlrtbl
   *
   * @param[in] ikey                   Pointer to ConfigKeyVal Class.
   * @param[in] dt_type                Configuration information.
   * @param[in] dmi                    Pointer to DalDmlIntf Class.
   * @param[in] op                     Describes the Type of Opeartion.
   *
   * @retval    UPLL_RC_SUCCESS        Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC    Generic Errors.
   * @retval    UPLL_RC_ERR_DB_ACCESS  Error accessing DB.
   */
  upll_rc_t UpdateRefCountInPPCtrlr(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf* dmi,
                                    unc_keytype_operation_t op,
                                    TcConfigMode config_mode,
                                    string vtn_name);

  /**
   * @Brief This API is used to get the memory allocated
   * vtnpolicingprofilectrl key and value structure
   *
   * @param[out] ctrlckv              This Contains the pointer to the
   *                                  ConfigKeyVal Class for which
   *                                  fields have to be updated.
   * @param[in]  ikey                 Pointer to ConfigKeyVal Class.
   * @param[in]  ctrlr_id             Ctrlr Name.
   *
   * @retval     UPLL_RC_SUCCESS      Successful completion.
   * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
   */
  upll_rc_t GetPMCtrlKeyval(ConfigKeyVal *&ctrlckv, ConfigKeyVal *&ikey,
                            controller_domain *ctrlr_dom);

  /**
   * @Brief This API is used to check the policer name availability
   * in policingprofiletbl CANDIDATE DB
   *
   * @param[in] ikey                          Pointer to ConfigKeyVal Class.
   * @param[in] dt_type                       Configuration information.
   * @param[in] op                            Describes the Type of Opeartion.
   * @param[in] dmi                           Pointer to DalDmlIntf Class.
   *
   * @retval    UPLL_RC_SUCCESS               Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC           Generic Error code.
   * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  No Record in DB.
   * @retval    UPLL_RC_ERR_INSTANCE_EXISTS   Record exists in DB.
   */
  upll_rc_t UpdateRecordInVtnPmCtrlr(ConfigKeyVal *ikey,
                                     upll_keytype_datatype_t dt_type,
                                     unc_keytype_operation_t op,
                                     DalDmlIntf *dmi,
                                     TcConfigMode config_mode,
                                     string vtn_name);
  upll_rc_t IsPolicingProfileConfigured(
      const char* policingprofile_name, upll_keytype_datatype_t dt_type,
      DalDmlIntf *dmi);
  /**
   * @Brief This API is used to read the configuration and statistics
   *
   * @param[in]  req                           Describes
   *                                           RequestResponderHeaderClass.
   * @param[out] ikey                          Pointer to ConfigKeyVal Class.
   * @param[in]  dmi                           Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_ERR_NO_SUCH_INSTANCE  Record Not available
   * @retval     UPLL_RC_ERR_CFG_SEMANTIC      Policy Profile Record not
   *                                           available
   * @retval     UPLL_RC_SUCCESS               Successful completion.
   * @retval     UPLL_RC_ERR_GENERIC           Generic Errors.
   * @retval     UPLL_RC_ERR_DB_ACCESS         Error accessing DB.
   */
  upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
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
   * @retval     UPLL_RC_ERR_CFG_SEMANTIC      PolicyProfile Record not found
   * @retval     UPLL_RC_ERR_DB_ACCESS         Error accessing DB.
   */
  upll_rc_t ReadSiblingMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                          bool begin, DalDmlIntf *dmi);

  /**
   * @Brief Method used to get the Bind Info Structure for Rename Purpose.
   *
   * @param[in] key_type   Describes the KT Information.
   * @param[in] binfo
   * @param[in] nattr      Describes the Tbl For which the Operation is
   *                       Targeted.
   * @param[in] tbl        Describes the Table Information.
   *
   * @retval    pfc_true   Successful Completion.
   * @retval    pfc_fasle  Failure.
   */
  bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                            int &nattr, MoMgrTables tbl);

  /**
   * @Brief Method to Copy The ConfigkeyVal with the Input Key.
   *
   * @param[out]  okey                 Pointer to ConfigKeyVal Class for
   *                                   which attributes have to be copied.
   * @param[in]   ikey                 Pointer to ConfigKeyVal Class.
   *
   * @retval      UPLL_RC_SUCCESS      Successfull Completion.
   * @retval      UPLL_RC_ERR_GENERIC  Returned Generic Error.
   */
  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);

  /**
   * @Brief This API is used to read the policing map record in IMPORT's
   * vtnpolicingmap table and check with CANDIDATES's policingprofile table
   * If record exists return merge conflict
   *
   * @param[in] keytype                     Describes the keyType Information.
   * @param[in] ctrlr_id                    Describes the Controller Name.
   * @param[in] ikey                        This Contains the pointer to the
   *                                        Class for which fields have to be
   *                                        Validated before the Merge.
   * @param[in] dmi                         Pointer to DalDmlIntf Class.
   *
   * @retval    UPLL_RC_SUCCESS             Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC         Generic Errors.
   * @retval    UPLL_RC_ERR_MERGE_CONFLICT  Record already avilable
   */
  upll_rc_t MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
                          ConfigKeyVal *ikey, DalDmlIntf *dmi,
                          upll_import_type import_type);

  /**
   * @Brief This API is used to get the unc key name
   * @param[out] ctrlr_key            This Contains the pointer to the Class
   for which fields have to be updated
   with values from the parent Class.
   *
   * @param[in]  dt_type              Configuration Information.
   * @param[in]  dmi                  Pointer to DalDmlIntf Class.
   * @param[in]  ctrlr_id             Controller name.
   *
   * @retval     UPLL_RC_SUCCESS      Successful completion.
   * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
   */
  upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                             upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                             uint8_t *ctrlr_id);

  /**
   * @Brief This API is used to get the renamed Controller's key
   *
   * @param[out] ikey                Contains the Pointer to ConfigkeyVal
   *                                 Class and contains the Pfc Name.
   * @param[in] dt_type              Configuratin information.
   * @param[in] dmi                  Pointer to DalDmlIntf Class.
   * @param[in] ctrlr_name           Controller name.
   *
   * @retval    UPLL_RC_SUCCESS      Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC  Generic Errors.
   */
  upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf *dmi,
                                    controller_domain *ctrlr_dom = NULL);

  /**
   * @Brief Method is used to copy the configuration info to Running DB
   *
   * @param[in] keytype              Describes the keyType Information.
   * @param[in] ctrlr_commit_status  List describes Commit Control Status
   *                                 Information.
   * @param[in] dmi                  Pointer to DalDmlIntf Class.
   *
   * @retval    UPLL_RC_SUCCESS      Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC  Generic Errors.
   */
  upll_rc_t TxCopyCandidateToRunning(
      unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
      DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name);

  /**
   * @Brief This method used to update the configstatus in Db during
   *        Trasaction Operation
   *
   * @param[in]  vtn_key              Pointer to ConfigkeyVal class.
   * @param[in]  op                   Type of operation
   * @param[in]  driver_result        Result code from Driver.
   * @param[in]  nreq                 Pointer to ConfigkeyVal.
   * @param[out] ctrlr_key            Pointer to ConfigkeyVal for controller.
   *
   * @retval     UPLL_RC_SUCCESS      Successful completion.
   * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
   */
  upll_rc_t UpdateConfigStatus(ConfigKeyVal *vtn_key,
                               unc_keytype_operation_t op,
                               uint32_t driver_result, ConfigKeyVal *nreq,
                               DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key);

  /**
   * @Brief This API updates the Configuration status for AuditConfigiration todo
   *
   * @param[in]  phase                Describes the phase of controller.
   * @param[in]  cs_status            either UNC_CS_INVALID or UNC_CS_APPLIED.
   * @param[in]  ckv_running          Pointer to ConfigkeyVal.
   * @param[in]  dmi                  Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval     UPLL_RC_SUCCESS      Successful completion.
   * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
   */

  upll_rc_t UpdateAuditConfigStatus(
      unc_keytype_configstatus_t cs_status,
      uuc::UpdateCtrlrPhase phase,
      ConfigKeyVal *&ckv_running,
      DalDmlIntf *dmi);

  /**
   * @Brief Method To Compare the Valid Check of Attributes
   *
   * @param[out] val1             Pointer to ConfigKeyVal Class which
   *                              contains only Valid Attributes.
   * @param[in]  val2             Pointer to ConfigKeyVal Class.
   * @param[in]  audit            Audit purpose.
   *
   * @retval     UPLL_RC_SUCCESS  Successful completion.
   */
  bool CompareValidValue(void *&val1, void *val2, bool audit);

  /**
   * @Brief This API is used to know the value availability of
   *         val structure attributes
   *
   * @param[in]  val                  pointer to the value structure.
   * @param[out] valid                reference to the enum containing
   *                                  the possible values of Valid flag.
   * @param[in]  indx                 Gives the index of attribute for
   *                                  validity.
   * @param[in]  dt_type              configuration for validity check.
   * @param[in]  tbl                  Table name.
   *
   * @retval     UPLL_RC_SUCCESS      Successful completion.
   * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
   */
  upll_rc_t GetValid(void *val, uint64_t indx, uint8_t *&valid,
                     upll_keytype_datatype_t dt_type, MoMgrTables tbl);

  /**
   * @Brief This API is used to allocate the memory for incoming in configval
   *
   * @param[out] ck_val               This Contains the pointer to the Class
   *                                  for which memory has to be allocated.
   * @param[in]  dt_type              Configuration information.
   * @param[in]  tbl                  Table name.
   *
   * @retval     UPLL_RC_SUCCESS      Successful completion.
   * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
   */
  upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                     MoMgrTables tbl);

  /**
   * @Brief This API is used to get the duplicate configkeyval
   *
   * @param[out] okey                 This Contains the pointer to the Class
   *                                  for which fields have to be updated
   *                                  with values from the Request.
   * @param[in]  req                  This Contains the pointer to the Class
   *                                  which is used for the Duplication.
   * @param[in]  tbl                  Table name.
   *
   * @retval     UPLL_RC_SUCCESS      Successful completion.
   * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
   */
  upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                            MoMgrTables tbl);

  /**
   * @Brief Method used to fill the CongigKeyVal with the Parent Class
   *            Information.
   *
   * @param[out] okey                This Contains the pointer to the
   *                                 ConfigKeyVal Class for which fields have
   *                                 to be updated with values from the
   *                                 parent Class.
   * @param[in] parent_key           This Contains the pointer to the
   *                                 ConfigKeyVal Class which is the
   *                                 Parent class used to fill the details.
   *
   * @retval    UPLL_RC_SUCCESS      Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC  Generic Errors.
   */
  upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);

  /**
   * @Brief Method used to fill the CongigKeyVal with the Parent Class
   *            Information.
   *
   * @param[out] okey                This Contains the pointer to the
   *                                 ConfigKeyVal Class for which fields have
   *                                 to be updated with values from the
   *                                 parent Class.
   * @param[in] parent_key           This Contains the pointer to the
   *                                 ConfigKeyVal Class which is the
   *                                 Parent class used to fill the details.
   *
   * @retval    UPLL_RC_SUCCESS      Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC  Generic Errors.
   */
  upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                               ConfigKeyVal *ikey);

  /**
   * @Brief Validates the syntax of the specified key and value structure
   *        for KT_VTN_POLICINGMAP keytype
   *
   * @param[in] key                           ConfigKeyVal key and value
   *                                          structure.
   * @param[in] req                           Describes IpcReqRespHeader class.
   *
   * @retval    UPLL_RC_SUCCESS               Successful.
   * @retval    UPLL_RC_ERR_CFG_SYNTAX        Syntax error.
   * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  Record is not available.
   * @retval    UPLL_RC_ERR_GENERIC           Generic failure.
   * @retval    UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
   * @retval    UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
   */
  upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *key);

  /**
   * @Brief Checks if the specified key type(KT_VTN_POLICINGMAP) and
   *        associated attributes are supported on the given controller,
   *        based on the valid flag
   *
   * @param[in] req                          Describes IpcReqRespHeader class.
   * param[in]  ikey                         Pointer to ConfigKeyVal.
   *
   * @retval    UPLL_RC_SUCCESS              Validation succeeded.
   * @retval    UPLL_RC_ERR_GENERIC          Validation failure.
   * @retval    UPLL_RC_ERR_INVALID_OPTION1  Option1 is not valid.
   * @retval    UPLL_RC_ERR_INVALID_OPTION2  Option2 is not valid.
   */
  upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                               const char* ctrlr_name = NULL);

  /**
   * @Brief Method is used to validate the policingmap value structure
   *        attributes bases on the operation
   *
   * @param[in] key                           Pointer to ConfigKeyVal.
   * @param[in] operation                     Type of operation.
   *
   * @retval    UPLL_RC_SUCCESS               Successful.
   * @retval    UPLL_RC_ERR_CFG_SYNTAX        Syntax error.
   * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  Record is not available.
   * @retval    UPLL_RC_ERR_GENERIC           Generic failure.
   * @retval    UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
   * @retval    UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
   */
  static upll_rc_t ValidatePolicingMapValue(ConfigKeyVal *key,
                                        uint32_t operation);

  /**
   * @Brief Method is used to validate the policingmap controller value
   * structure attributes bases on the operation
   *
   * @param[in] key                           Pointer to ConfigKeyVal.
   * @param[in] req                          Describes IpcReqRespHeader class.
   *
   * @retval    UPLL_RC_SUCCESS               Successful.
   * @retval    UPLL_RC_ERR_CFG_SYNTAX        Syntax error.
   * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  Record is not available.
   * @retval    UPLL_RC_ERR_GENERIC           Generic failure.
   * @retval    UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
   * @retval    UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
   */
  upll_rc_t ValidateVtnPolicingMapControllerValue(ConfigKeyVal *key,
                                                  IpcReqRespHeader *req);

  /**
   * @Brief Checks if the specified key type(KT_VTN_POLICINGMAP) and
   *        associated attributes are supported on the given controller,
   *        based on the valid flag
   *
   *
   * @param[in] ikey   Corresponding key and value structure.
   * @param[in] attrs  Pointer to controller attribute
   *
   * @retval    UPLL_RC_SUCCESS              Validation succeeded.
   * @retval    UPLL_RC_ERR_GENERIC          Validation failure.
   * @retval    UPLL_RC_ERR_INVALID_OPTION1  Option1 is not valid.
   * @retval    UPLL_RC_ERR_INVALID_OPTION2  Option2 is not valid.
   */
  upll_rc_t ValVtnPolicingMapAttributeSupportCheck(ConfigKeyVal *ikey,
                                                   const uint8_t* attrs);

  /**
   * @Brief Checks if the specified key type(KT_VTN_POLICINGMAP) and
   *        associated attributes are supported on the given controller,
   *        based on the valid flag
   *
   * @param[in] req                          Describes IpcReqRespHeader class.
   * param[in]  ikey                         Pointer to ConfigKeyVal.
   * param[in]  ctrlr_name                   Controller name.
   *
   * @retval    UPLL_RC_SUCCESS              Validation succeeded.
   * @retval    UPLL_RC_ERR_GENERIC          Validation failure.
   * @retval    UPLL_RC_ERR_INVALID_OPTION1  Option1 is not valid.
   * @retval    UPLL_RC_ERR_INVALID_OPTION2  Option2 is not valid.
   */
  upll_rc_t ValidateVtnPolicingMapControllerCapability(
      IpcReqRespHeader *req, ConfigKeyVal *ikey, const char *ctrlr_name,
      uint32_t max_attrs, const uint8_t *attrs);

  /** @brief  Validate the input key attributes
   *      *
   *           *  @param[in]  key   Pointer to Input Key Structure
   *                *  @param[in]  index Index of the attribute which needs to validate
   *                     *
   *                          *  @return  true if validation is successful
   *                               **/
  bool IsValidKey(void *key, uint64_t index, MoMgrTables tbl = MAINTBL);
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

  /**
   * @brief  Method used for Restoring PolicingProfile in the Controller Table
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

  bool CompareKey(void *key1, void *key2) {
    return true;
  }
  upll_rc_t ReadSiblingPolicingMapController(
      IpcReqRespHeader *req,
      ConfigKeyVal *ikey,
      DalDmlIntf *dmi);
  upll_rc_t ReadControllerStateDetail(ConfigKeyVal *&ikey,
                                      ConfigKeyVal *vtn_dup_key,
                                      IpcResponse *ipc_response,
                                      upll_keytype_datatype_t dt_type,
                                      unc_keytype_operation_t op,
                                      DalDmlIntf *dmi);
  upll_rc_t ReadSibDTStateNormal(ConfigKeyVal *ikey,
                                 ConfigKeyVal *vtn_dup_key,
                                 ConfigKeyVal *tctrl_key,
                                 upll_keytype_datatype_t dt_type,
                                 unc_keytype_operation_t op,
                                 DbSubOp dbop,
                                 DalDmlIntf *dmi,
                                 uint8_t* ctrlr_id);

  upll_rc_t SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                       DalDmlIntf *dmi, uint8_t *ctrlr) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                              DalDmlIntf *dmi,
                              IpcReqRespHeader *req = NULL);
  upll_rc_t IsKeyInUse(upll_keytype_datatype_t dt_type,
                       const ConfigKeyVal *ckv,
                       bool *in_use,
                       DalDmlIntf *dmi);

  upll_rc_t UpdateMainTbl(ConfigKeyVal *vtn_key,
                          unc_keytype_operation_t op, uint32_t driver_result,
                          ConfigKeyVal *nreq, DalDmlIntf *dmi);

  upll_rc_t GetControllerDomainSpan(
      ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
      DalDmlIntf *dmi);
  upll_rc_t ReadVtnPolicingMapController(
      IpcReqRespHeader *req,
      ConfigKeyVal *ikey,
      DalDmlIntf *dmi);
  upll_rc_t ReadControllerStateNormal(ConfigKeyVal *&ikey,
                                      upll_keytype_datatype_t dt_type,
                                      unc_keytype_operation_t op,
                                      DalDmlIntf *dmi);
  upll_rc_t CopyVtnControllerCkv(ConfigKeyVal *ikey, ConfigKeyVal *&okey);

  upll_rc_t SendIpcrequestToDriver(
      ConfigKeyVal *ikey, IpcReqRespHeader *req,
      IpcResponse *ipc_response, DalDmlIntf *dmi);

  upll_rc_t GetChildControllerConfigKey(
      ConfigKeyVal *&okey, ConfigKeyVal *parent_key,
      controller_domain *ctrlr_dom);
  upll_rc_t  ReadSiblingControllerStateNormal(
      ConfigKeyVal *ikey,
      IpcReqRespHeader *req,
      DalDmlIntf *dmi);
  upll_rc_t  ReadSiblingControllerStateDetail(
      ConfigKeyVal *ikey,
      IpcReqRespHeader *req,
      DalDmlIntf *dmi);

  upll_rc_t ConstructReadSiblingNormalResponse(ConfigKeyVal *&ikey,
                                               upll_keytype_datatype_t dt_type,
                                               DalDmlIntf *dmi,
                                               ConfigKeyVal *ctrlr_ckv,
                                               ConfigKeyVal *&resp_ckv);

  upll_rc_t GetChildVtnCtrlrKey(
      ConfigKeyVal *&okey, ConfigKeyVal *ikey);

  upll_rc_t ReadSiblingCount(IpcReqRespHeader *req,
                             ConfigKeyVal* ikey,
                             DalDmlIntf *dmi);

  upll_rc_t ValidateReadAttribute(ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi,
                                  IpcReqRespHeader *req);

  upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                              DalDmlIntf *dmi,
                              const char *ctrlr_id);

  upll_rc_t DeleteChildrenPOM(ConfigKeyVal *ikey,
                              upll_keytype_datatype_t dt_type,
                              DalDmlIntf *dmi,
                              TcConfigMode config_mode,
                              string vtn_name);

  upll_rc_t SetValidAudit(ConfigKeyVal *&ikey);

  upll_rc_t UpdateVnodeVal(ConfigKeyVal *ikey,
                           DalDmlIntf *dmi,
                           upll_keytype_datatype_t data_type,
                           bool &no_rename);

  bool FilterAttributes(void *&val1,
                          void *val2,
                          bool copy_to_running,
                          unc_keytype_operation_t op);

  upll_rc_t MergeImportToCandidate(unc_key_type_t keytype,
                                   const char *ctrlr_name,
                                   DalDmlIntf *dmi,
                                   upll_import_type import_type);

  upll_rc_t IsRenamed(ConfigKeyVal *ikey,
                      upll_keytype_datatype_t dt_type,
                      DalDmlIntf *dmi,
                      uint8_t &rename);

  upll_rc_t GetPolicingProfileConfigKey(
        const char *pp_name, ConfigKeyVal *&okey,
        DalDmlIntf *dmi);

  upll_rc_t SetRenameFlag(ConfigKeyVal *ikey,
                          DalDmlIntf *dmi,
                          IpcReqRespHeader *req);

  upll_rc_t SetVtnPmConsolidatedStatus(ConfigKeyVal *ikey,
                                       uint8_t *ctrlr_id,
                                       DalDmlIntf *dmi);

  upll_rc_t SetConsolidatedStatus(ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi);
/*  upll_rc_t DecRefCountInPPCtrlTbl(ConfigKeyVal *pm_imkey,
                                   DalDmlIntf *dmi);*/

  static bool IsAllAttrInvalid(val_policingmap_t *val);

virtual  upll_rc_t GetOperation(uuc::UpdateCtrlrPhase phase,
                         unc_keytype_operation_t &op);

  upll_rc_t CreatePIForVtnPom(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                              DalDmlIntf *dmi,
                              const char *ctrlr_id);

  upll_rc_t CompareValueStructure(ConfigKeyVal *tmp_ckv,
                                  upll_keytype_datatype_t datatype,
                                  DalDmlIntf *dmi);

  upll_rc_t CompareValStructure(void *val1, void *val2);

  upll_rc_t UpdateRefCountInPPCtrlrTbl(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    const char *ctrlr_id, TcConfigMode config_mode, string vtn_name);

  upll_rc_t UpdateRecordInVtnPmCtrlrTbl(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name);

  upll_rc_t GetDomainsForController(
      ConfigKeyVal *ckv_drvr,
      ConfigKeyVal *&ctrlr_ckv,
      DalDmlIntf *dmi);
};

typedef struct val_vtnpolicingmap_ctrl {
    uint8_t valid[1];
    uint8_t cs_row_status;
    uint8_t cs_attr[1];
    uint8_t policer_name[33];
} val_vtnpolicingmap_ctrl_t;
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // MODULES_UPLL_VTN_POLICINGMAP_MOMGR_HH_
