/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VTERMINAL_MOMGR_H
#define UNC_UPLL_VTERMINAL_MOMGR_H

#include "momgr_impl.hh"
#include "vnode_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

class VterminalMoMgr : public VnodeMoMgr {
 public:
  /**
   * @brief  Default Constructor.
   */
  VterminalMoMgr();
  /**
   * @brief Default Desctuctor.
   */
  virtual ~VterminalMoMgr() {
    for (int i = 0; i < ntable; ++i)
      if (table[i]) {
        delete table[i];
      }
    delete[] table;
  }
  /**
   * @brief          Method to check if individual portions of a key are valid
   *
   * @param[in/out]  ikey                 pointer to ConfigKeyVal referring to a UNC resource
   * @param[in]      index                db index associated with the variable
   *
   * @retval         true                 input key is valid
   * @retval         false                input key is invalid.
   */
  bool IsValidKey(void *key,
      uint64_t index, MoMgrTables tbl = MAINTBL);
  /**
   * @brief         This method invoke when the VTN merge hapeening between
   *                Running and DT import. This will checks the vnode name
   *                unique or not.
   *
   * @param[in]     keytype       UNC KEY TYPE
   * @param[in/out] ctrlr_id      Controller ID
   * @param[in]     conflict_ckv  key and value structure
   * @param[in]     dal           Pointer to the DalDmlIntf(DB Interface)
   * @param[in]     import_type   specifies the import_type.
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   * @retval  UPLL_RC_ERR_MERGE_CONFLICT         Semantic check error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
   */
  upll_rc_t MergeValidate(unc_key_type_t keytype,
      const char *ctrlr_id,
      ConfigKeyVal *conflict_ckv,
      DalDmlIntf *dmi,
      upll_import_type import_type);
  /**
   * @Brief  Validates the syntax for KT_VTERMINAL Keytype key structure.
   *
   * @param[in]  key_VTERMINAL  KT_VTERMINAL key structure.
   * @param[in]  operation operation type.
   *
   * @retval  UPLL_RC_SUCCESS         validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
   *
   */
  upll_rc_t ValidateVterminalKey(key_vterm_t *vterm_key,
      unc_keytype_operation_t operation = UNC_OP_INVALID);
  /**
   *  @brief        This is semantic check for KEY_VTERMINAL key type
   *                in the update operation.
   *
   * @param[in/out] ikey     Pointer to the ConfigKeyVal Structure
   * @param[in]     datatype DB type.
   * @param[in]     dmi      Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval UPLL_RC_SUCCESS                    Not Referenced.
   * @retval UPLL_RC_ERR_CFG_SEMANTIC           Referenced
   * @retval result_code                        Generic failure/DB error
   */
  upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
      DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }
  /**
   * @brief  create entry in Vnode Rename Table,
   *         with the renamed VTN details fetched from VTN rename Table
   *
   *
   * @param[in]  ikey       ConfigKeyVal pointer
   * @param[in]  dt_type    specifies the database type
   * @param[in]  ctrlr_id   pointer to the controller name
   * @param[in]  dmi        DalDmlIntf pointer
   *
   * @retval UPLL_RC_SUCCESS      Successful
   * @retval UPLL_RC_ERR_GENERIC  failed to update the VTERMINALIf
   */
  upll_rc_t CreateVnodeConfigKey(ConfigKeyVal *ikey,
      ConfigKeyVal *&okey);
  /**
   * @brief      Method to get a configkeyval of a specified keytype from an input configkeyval
   *
   * @param[in/out]  okey                 pointer to output ConfigKeyVal
   * @param[in]      parent_key           pointer to the configkeyval from
   * which the output configkey val is initialized.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
      ConfigKeyVal *parent_key);
  /**
   * @brief  returns the controller and domain name from ConfigKeyVal
   *
   * @param[in] @param[in]  ikey         ConfigKeyVal pointer
   * @param[in] @param[out] ctrlr_dom    Controller Domain pointer
   *
   * @retval UPLL_RC_ERR_GENERIC     ConfigKeyVal is empty
   * @retval UPLL_RC_SUCCESS         Successfull completion
   */
  upll_rc_t GetControllerDomainId(ConfigKeyVal *ikey,
      controller_domain_t *ctrlr_dom);


 private:
  static unc_key_type_t vterminal_child[];
  static BindInfo vterminal_bind_info[];
  static BindInfo vterminal_rename_bind_info[];
  static BindInfo key_vterminal_maintbl_bind_info[];
  static BindInfo key_vterminal_renametbl_update_bind_info[];
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
   */
  upll_rc_t GetValid(void *val,
      uint64_t indx,
      uint8_t *&valid,
      upll_keytype_datatype_t dt_type,
      MoMgrTables tbl);
  /**
   * @brief  Filters the attributes which need not be sent to controller
   *
   * @param[in/out]  val1   first record value instance.
   * @param[in]      val2   second record value instance.
   * @param[in]      audit  Not used for VTN
   * @param[in]      op     Operation to be performed
   *
   */
  bool FilterAttributes(void *&val1,
      void *val2,
      bool audit_status,
      unc_keytype_operation_t op);

  /**
   * @brief  Compares the valid value between two database records.
   * 	     if both the values are same, update the valid flag for
   * 	     corresponding attribute as invalid in the first record.
   *
   * @param[in/out]  val1   first record value instance.
   * @param[in]      val2   second record value instance.
   * @param[in]      audit  if true, CompareValidValue called from audit
   *  			    process..
   *
   */
  bool CompareValidValue(void *&val1,
      void *val2,
      bool audit);
  /**
   * @brief swap the value after read operation in Import table
   *
   * @param[in]  ikey       ConfigKeyVal pointer
   * @param[in]  okey       pointer to output ConfigKeyVal
   * @param[in]  dmi        DalDmlIntf pointer
   * @param[in]  ctrlr      pointer to the controller name
   * @param[in]  no_rename  rename check flag
   *
   * @retval UPLL_RC_ERR_GENERIC
   */
  upll_rc_t SwapKeyVal(ConfigKeyVal *ikey,
      ConfigKeyVal *&okey,
      DalDmlIntf *dmi,
      uint8_t *ctrlr,
      bool &no_rename);
  /**
   * @brief  Update config status for Transaction commit
   *
   * @param[in/out]  req          ConfigKeyVal instance
   * @param[in]      op           Operation(CREATE, DELETE or UPDATE)
   * @param[in]      result       UNC_CS_APPLIED or UNC_CS_NOT_APPLIED
   *
   */
  upll_rc_t UpdateConfigStatus(ConfigKeyVal *req,
      unc_keytype_operation_t op,
      uint32_t driver_result,
      ConfigKeyVal *upd_key,
      DalDmlIntf *dmi,
      ConfigKeyVal *ctrlr_key = NULL);
  /**
   * @brief  Update config status for commit result and vote result.
   *
   * @param[in/out]  ckv_running  ConfigKeyVal instance.
   * @param[in]      cs_status    either UNC_CS_INVALID or UNC_CS_APPLIED.
   * @param[in]      phase        specify the phase (CREATE,DELETE or UPDATE)
   *
   */
  upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
      uuc::UpdateCtrlrPhase phase,
      ConfigKeyVal *&ckv_running,
      DalDmlIntf *dmi);
  /**
   * @Brief  Checks if the specified key type(KT_VTERMINAL) and
   *         associated attributes are supported on the given controller,
   *         based on the valid flag
   *
   * @param[in]  req               This structure contains IpcReqRespHeader
   *                               (first 8 fields of input request structure).
   * @param[in]  ikey              ikey contains key and value structure.
   * @param[in]  ctrlr_id          Controller id associated with ikey.
   *
   * @retval  UPLL_RC_SUCCESS              Validation succeeded.
   * @retval  UPLL_RC_ERR_GENERIC          Validation failure.
   * @retval  UPLL_RC_ERR_INVALID_OPTION1  Option1 is not valid.
   * @retval  UPLL_RC_ERR_INVALID_OPTION2  Option2 is not valid.
   */
  upll_rc_t ValidateCapability(IpcReqRespHeader *req,
      ConfigKeyVal *ikey,
      const char *ctrlr_id);

  /**
   * @Brief  Validates the syntax of the specified key and value structure
   *         for KT_VTERMINAL keytype
   *
   * @param[in]  req    This structure contains IpcReqRespHeader
   *                    (first 8 fields of input request structure).
   * @param[in]  ikey   ikey contains key and value structure.
   *
   * @retval  UPLL_RC_SUCCESS               Successful.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX        Syntax error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  key_VTERMINAL is not available.
   * @retval  UPLL_RC_ERR_GENERIC           Generic failure.
   * @retval  UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
   * @retval  UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
   */
  upll_rc_t ValidateMessage(IpcReqRespHeader *req,
      ConfigKeyVal *ikey);
  /**
   * @Brief  Checks if the specified key type and
   *         associated attributes are supported on the given controller,
   *         based on the valid flag.
   *
   * @param[in]  vterminal_val         KT_VTERMINAL value structure.
   * @param[in]  attrs                 Pointer to controller attribute.
   * @param[in]  operation             Operation name.
   *
   * @retval  UPLL_RC_SUCCESS                     validation succeeded.
   * @retval  UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT  Instance count limit is exceedes.
   * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR  Attribute NOT_SUPPORTED.
   * @retval  UPLL_RC_ERR_GENERIC                 Generic failure.
   */

  upll_rc_t ValVterminalAttributeSupportCheck(val_vterm_t *vterm_val,
      const uint8_t *attrs,
      unc_keytype_operation_t operation);
  /*
   * @Brief  Validates the syntax for KT_VTERMINAL keytype value structure.
   *
   * @param[in]  val_VTERMINAL  KT_VTERMINAL value structure.
   * @param[in]  operation  Operation type.
   *
   * @retval  UPLL_RC_SUCCESS               validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX        validation failed.
   */
  upll_rc_t ValidateVterminalValue(val_vterm_t *vterm_val,
      uint32_t operation);
  /**
   * @Brief  Validates the syntax for KT_VTERMINAL keytype Rename structure.
   *
   * @param[in]  val_rename_vterm  KT_VTERMINAL Rename structure.
   *
   * @retval  UPLL_RC_SUCCESS         validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
   *
   */
  upll_rc_t ValidateVterminalRenameValue(val_rename_vterm *vterm_rename);
  /**
   * @Brief  compares controller id and domain id before
   *         updating the value to DB.
   *
   * @param[in]  ikey  ikey contains key and value structure.
   * @param[in]  okey  okey contains key and value structure.
   *
   * @retval  UPLL_RC_SUCCESS            Successful.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX     Syntax error.
   */
  upll_rc_t CtrlrIdAndDomainIdUpdationCheck(ConfigKeyVal *ikey,
      ConfigKeyVal *okey);
  /**
   * @brief  Duplicates the input configkeyval including the key and val.
   * based on the tbl specified.
   *
   * @param[in]  okey   Output Configkeyval - allocated within the function
   * @param[in]  req    Input ConfigKeyVal to be duplicated.
   * @param[in]  tbl    specifies if the val structure belongs to the main
   * table/ controller table or rename table.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey,
      ConfigKeyVal *&req,
      MoMgrTables tbl = MAINTBL);
  /**
   * @brief  Allocates for the specified val in the given configuration in the
   *         specified table.
   *
   * @param[in]  ck_val   Reference pointer to configval structure allocated.
   * @param[in]  dt_type  specifies the configuration candidate/running/state
   * @param[in]  tbl      specifies if the corresponding table is the  main
   *                      table / controller table or rename table.
   *
   * @retval     UPLL_RC_SUCCESS      Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC  Failure case.
   */
  upll_rc_t AllocVal(ConfigVal *&ck_val,
      upll_keytype_datatype_t dt_type,
      MoMgrTables tbl = MAINTBL);

  /**
   * @brief      Method to get a configkeyval of the parent keytype
   *
   * @param[in/out]  okey           pointer to parent ConfigKeyVal
   * @param[in]      ikey           pointer to the child configkeyval from
                                    which the parent configkey val is obtained.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
      ConfigKeyVal *parent_key);
  /**
   * @brief      Method to get renamed controller key from unc key.
   * On success unc key contains the controller key if renamed.
   *
   * @param[in/out]  ikey          ConfigVal pointer to input unc key
   * @param[in]      dt_type       specifies the configuration type
   * @param[in]      dmi           specifies the db connection info
   * @param[in]      ctrlr_name    specifies the pointer to the controller id
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
      upll_keytype_datatype_t dt_type,
      DalDmlIntf *dmi,
      controller_domain *ctrlr_dom);
  /**
   * @brief      Method to get renamed unc key from controller key.
   * On success controller key contains the renamed unc key.
   *
   * @param[in/out]  ctrlr_key     ConfigVal pointer to input ctrlr key
   * @param[in]      dt_type       specifies the configuration type
   * @param[in]      dmi           specifies the db connection info
   * @param[in]      ctrlr_id      specifies the pointer to the controller id
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
      upll_keytype_datatype_t dt_type,
      DalDmlIntf *dmi,
      uint8_t *ctrlr_id);

  /**
   * @brief  Get the specific vtn_name and vnode_name from ConfigKeyVal
   *
   * @param[in]  ikey        ConfigKeyVal pointer
   * @param[out] vtn_name    vnode vtn name
   * @param[out] vnode_name  vnode specific name
   *
   * @retval UPLL_RC_SUCCESS      Successful
   * @retval UPLL_RC_ERR_GENERIC  failed to retrieve the values
   */
  upll_rc_t GetVnodeName(ConfigKeyVal *ikey,
      uint8_t *&vtn_name,
      uint8_t *&vnode_name);
  /**
   * @brief     Method used in Rename Operation.
   *            This function collects the Unc new name, Unc old name and Ctrlr name
   *            informations and creats the configkeyval.
   *
   * @param[in]  ikey                        key and value structure.
   * @param[in]  okey                        key and value structure.
   * @param[out]  rename_info                key and value structure.
   * @param[in]  dmi                         Pointer to DalDmlIntf Class.
   * @param[in]  ctrlr_id                    Controller Id.
   * @param[in]  renamed                     Flag for Already renamed or not.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */

  upll_rc_t GetRenameInfo(ConfigKeyVal *ikey,
      ConfigKeyVal *okey,
      ConfigKeyVal *&rename_info,
      DalDmlIntf *dmi,
      const char *ctrlr_id,
      bool &renamed);
  /**
   * @brief     Method used in rename opertaion while update the new name into the tables
   *            to Gets the bindinfo detail for the specifed key type.
   *
   * @param[in]  key_type                    unc key type.
   * @param[out] binfo                       Bindinfo details.
   * @param[out] nattr                       Number of Attributes.
   * @param[in]  tbl                         Table Name.
   *
   * @retval     PFC_TRUE                    Successfull completion.
   */
  bool GetRenameKeyBindInfo(unc_key_type_t key_type,
      BindInfo *&binfo,
      int &nattr,
      MoMgrTables tbl);
  /**
   * @brief     Method create configkey for the specified key type.
   *            Copy the old name from the rename_info into okey.
   *
   * @param[in]  okey                        key and value structure.
   * @param[in]  rename_info                 key and value structure.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */
  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
      ConfigKeyVal *ikey);

  /** @brief     To convert the value structure read from DB to
   *            VTNService during READ operations
   * @param[in/out] ikey      Pointer to the ConfigKeyVal Structure
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   *
   */
  upll_rc_t AdaptValToVtnService(ConfigKeyVal *ikey, AdaptType adapt_type);


  /** @brief     This function validates the vTerminal name
   *             Exists on ru_vbr_if_tbl as vexternal during
   *             partial import
   * @param[out] ikey      Which vTerminal violates the condidition
   *
   * @retval  UPLL_RC_SUCCESS             No Record exists.
   * @retval  UPLL_RC_ERR_MERGE_CONFLICT  vTermianl exists in running vbr_if_tbl.
   *
   */
  upll_rc_t PartialMergeValidate(unc_key_type_t keytype,
                                 const char *ctrlr_id,
                                 ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi);
};
}  //  namespace kt_momgr
}  //  namespace upll
}  //  namespace unc
#endif
