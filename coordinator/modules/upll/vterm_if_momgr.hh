/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VTERM_IF_MOMGR_H
#define UNC_UPLL_VTERM_IF_MOMGR_H

#include <string>
#include <sstream>
#include <set>
#include "momgr_impl.hh"
#include "vtn_momgr.hh"
#include "vnode_child_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

class VtermIfMoMgr : public VnodeChildMoMgr {
 private:
  static unc_key_type_t vterm_if_child[];
  static BindInfo       vterm_if_bind_info[];
  static BindInfo       key_vterm_if_maintbl_bind_info[];

  /* @brief      Returns admin and portmap information if portmap is
   *             valid. Else returns NULL for portmap
   *
   * @param[in]   ikey          Pointer to VTERM_IF ConfigKeyVal.
   * @param[out]  valid_pm      portmap valid value
   * @param[out]  pm            pointer to portmap structure if
   *                            portmap is valid
   * @param[out]  valid_admin   admin_status valid value
   * @param[out]  admin_status  value of admin_status
   *
   * @retval  UPLL_RC_SUCCESS      Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
   *
   **/
  upll_rc_t GetPortMap(ConfigKeyVal *ikey,
                       uint8_t &valid_pm,
                       val_port_map_t *&pm,
                       uint8_t &valid_admin,
                       uint8_t &admin_status);

  /**
   * @brief  Gets the valid array position of the variable in the value
   *         structure from the table in the specified configuration
   *
   * @param[in]     val      pointer to the value structure
   * @param[in]     indx     database index for the variable
   * @param[out]    valid    position of the variable in the valid array -
   *                          NULL if valid does not exist.
   * @param[in]     dt_type  specifies the datatype.
   * @param[in]     tbl      specifies the table containing the given value
   *
   **/
  upll_rc_t GetValid(void *val, uint64_t indx, uint8_t *&valid,
                      upll_keytype_datatype_t dt_type, MoMgrTables tbl);

  /**
   * @brief  Filters the attributes which need not be sent to controller
   *
   * @param[in/out]  val1   first record value instance.
   * @param[in]      val2   second record value instance.
   * @param[in]      audit  Not used for VTN
   * @param[in]      op     Operation to be performed
   *
   **/
  bool FilterAttributes(void *&val1, void *val2, bool audit_status,
                        unc_keytype_operation_t op);

  /**
   * @brief  Compares the valid value between two database records.
   * 	       if both the values are same, update the valid flag for
   * 	       corresponding attribute as invalid in the first record.
   *
   * @param[in/out]  val1   first record value instance.
   * @param[in]      val2   second record value instance.
   * @param[in]      audit  if true, CompareValidValue called from audit process.
   *
   **/
  bool CompareValidValue(void *&val1, void *val2, bool audit);

  /**
   * @brief  Update config status based on commit result.
   *
   * @param[in/out]  ckv_running   ConfigKeyVal of VTERM_IF.
   * @param[in]      op            Specifies the operation.
   * @param[in]      driver_result Result received from driver.
   * @param[in]      cs_status     either UNC_CS_INVALID or UNC_CS_APPLIED.
   * @param[in]      dmi           Pointer to DalDmlIntf Class.
   * @param[in]      ctrlr_key     ConfigKeyVal of controller info.
   **/
  upll_rc_t UpdateConfigStatus(ConfigKeyVal *req,
                               unc_keytype_operation_t op,
                               uint32_t driver_result,
                               ConfigKeyVal *upd_key,
                               DalDmlIntf   *dmi,
                               ConfigKeyVal *ctrlr_key = NULL);

  /**
   * @Brief  Validates the syntax of the specified key and value structure,
   *         operation and datatype for UNC_KT_VTERM_IF
   *
   * @param[in]  req    This structure contains IpcReqRespHeader
   *                    (first 8 fields of input request structure).
   * @param[in]  ikey   ikey contains key and value structure of UNC_KT_VTERM_IF.
   *
   * @retval  UPLL_RC_SUCCESS                Successful.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX         Syntax error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE   key_vterm_if is not available.
   * @retval  UPLL_RC_ERR_GENERIC            Generic failure.
   * @retval  UPLL_RC_ERR_INVALID_OPTION1    option1 is not valid.
   * @retval  UPLL_RC_ERR_INVALID_OPTION2    option2 is not valid.
   */
  upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *ikey);

  /**
   * @brief  Update config status for commit result and vote result.
   *
   * @param[in]      cs_status    either UNC_CS_INVALID or UNC_CS_APPLIED.
   * @param[in]      phase        specify the phase (CREATE,DELETE or UPDATE)
   * @param[in/out]  ckv_running  ConfigKeyVal instance.
   * @param[in]      dmi           Pointer to DalDmlIntf Class.
   *
   **/
  upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                    uuc::UpdateCtrlrPhase phase,
                                    ConfigKeyVal *&ckv_running,
                                    DalDmlIntf *dmi);
  /**
   * @Brief  Checks if the specified key type(KT_VTERM_IF) and
   *         associated attributes are supported for the given controller,
   *         based on the valid flag
   *
   * @param[in]  req              This structure contains IpcReqRespHeader
   *                              (first 8 fields of input request structure).
   * @param[in]  ikey             ikey contains key and value structure.
   * @param[in]  ctrlr_name       Controller id associated with ikey.
   *
   * @retval  UPLL_RC_SUCCESS                       Validation succeeded.
   * @retval  UPLL_RC_ERR_GENERIC                   Validation failure.
   * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR    Attribute Not supported by
   *                                                controller.
   */
  upll_rc_t ValidateCapability(IpcReqRespHeader *req,
               ConfigKeyVal *ikey, const char *ctrlr_name);

  /**
   * @Brief  Validates the syntax of KT_VTERM_IF value structure.
   *
   * @param[in]  val_vterm_if  KT_VTERM_IF value structure.
   * @param[in]  operation     Operation type.
   *
   * @retval  UPLL_RC_SUCCESS         validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
   */
  upll_rc_t  ValidateVtermIfValue(val_vterm_if_t *vterm_if_val,
                                unc_keytype_operation_t operation);

  /**
   * @Brief  Checks if the specified key type and
   *         associated attributes are supported on the given controller,
   *         based on the valid flag.
   *
   * @param[in]  attrs           Pointer to controller attribute.
   * @param[in]  ikey            Corresponding key and value structure.
   * @param[in]  operation       Operation name.
   * @param[in]  dt_type         Data type name
   *
   * @retval  UPLL_RC_SUCCESS                     validation succeeded.
   * @retval  UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT  Instance count limit is exceeds.
   * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR  Attribute Not_Supported.
   * @retval  UPLL_RC_ERR_GENERIC                 Generic failure.
   */
  upll_rc_t ValVtermIfAttributeSupportCheck(const uint8_t *attrs,
                                            ConfigKeyVal *ikey,
                                            unc_keytype_operation_t operation,
                                            upll_keytype_datatype_t dt_type);

  /**
   * @brief  Perform Semantic Check to check Different vbridges or vterminal's
   *         contain same switch-id and vlan-id
   *
   * @param[in]  ikey        ConfigKeyVal of VTERM_IF.
   * @param[in]  upll_rc_t   UPLL_RC_ERR_CFG_SEMANTIC on error
   *                         UPLL_RC_SUCCESS on success
   **/
  upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                              DalDmlIntf *dmi,
                              IpcReqRespHeader *req = NULL);

  /**
   * @brief  Allocates the ConfigVal of VTERM_IF val for the
   *         specified table.
   *
   * @param[out] ck_val   Reference pointer to configval structure allocated.
   * @param[in]  dt_type  specifies the datatype candidate/running/state
   * @param[in]  tbl      specifies if the corresponding table is the  main
   *                      table / controller table or rename table.
   *
   * @retval     UPLL_RC_SUCCESS      Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC  Failure case.
   **/
  upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                     MoMgrTables tbl = MAINTBL);

  /* Rename */
  bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                            int &nattr, MoMgrTables tbl);

  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);

  /* @brief     To convert value structure read from DB to
   *            VTNService structure during response handling
   * @param[in/out] ikey      Pointer to the ConfigKeyVal Structure
   *
   * @retval  UPLL_RC_SUCCESS      Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
   *
   **/
  upll_rc_t AdaptValToVtnService(ConfigKeyVal *ikey, AdaptType adapt_type);

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

 public:
  /* Default constructor */
  VtermIfMoMgr();

  /* Default destructor */
  virtual ~VtermIfMoMgr() {
    for (int i = 0; i < ntable; i++)
      if (table[i]) {
        delete table[i];
      }
    delete[] table;
  }

  /**
   * @brief      Method used to perform update request for requested
   *             UNC_KT_VTERM_IF keytype
   *
   * @param[in]  req     contains first 8 fields of input request structure
   * @param[in]  ikey    ConfigKeyVal obj pointer of UNC_VTERM_IF
   * @param[in]  dmi     Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_SUCCESS            Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC        Failure case.
   */
  upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);

  /**
   ** @Brief  Validates the syntax of UNC_KT_VTERM_IF key structure.
   **
   ** @param[in]  key_vterm_if  KT_VTERM_IF key structure.
   ** @param[in]  operation     operation type.
   **
   ** @retval  UPLL_RC_SUCCESS         validation succeeded.
   ** @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
   **/
  upll_rc_t ValidateVtermIfKey(key_vterm_if_t *vterm_if_key,
                 unc_keytype_operation_t operation = UNC_OP_INVALID);


  /**
   * @brief      Method to check if individual portions of a key are valid
   *
   * @param[in/out]  ikey   Pointer to UNC_KT_VTERM_IF ConfigKeyVal obj.
   * @param[in]      index  db index associated with the variable
   *
   * @retval         true      input key is valid
   * @retval         false     input key is invalid.
   **/
  bool IsValidKey(void *tkey, uint64_t index, MoMgrTables tbl = MAINTBL);

  /**
   * @brief  Duplicates the input configkeyval including the key and val.
   * based on the tbl specified.
   *
   * @param[out] okey   Duplicate ConfigKeyVal pointer of input ConfigKeyVal
   * @param[in]  req    Input ConfigKeyVal to be duplicated.
   * @param[in]  tbl    specifies the val structure belongs to the
   *                    main table/ controller table or rename table.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   **/
  upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey,
                            ConfigKeyVal *&req, MoMgrTables tbl = MAINTBL);

  /* In VTERM_IF nothing to be performed */
  upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                         DalDmlIntf *dmi);

  /**
    * @brief      Method to get a UNC_KT_VTERM_IF ConfigKeyVal Pointer from the
    *             input ConfigKeyVal of specified KeyType
    *
    * @param[in/out]  okey           pointer to output ConfigKeyVal
    * @param[in]      parent_key     pointer to the configkeyval from which
    *                                the output configkey val is initialized.
    *
    * @retval         UPLL_RC_SUCCESS      Successfull completion.
    * @retval         UPLL_RC_ERR_GENERIC  Failure case.
    */
  upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
                              ConfigKeyVal *parent_key);

  upll_rc_t GetVtermIfValfromDB(ConfigKeyVal *ikey,
                              ConfigKeyVal *&ck_drv_vbr_if,
                              upll_keytype_datatype_t dt_type,
                              DalDmlIntf *dmi);
#if 0
  upll_rc_t PortStatusHandler(const char *ctrlr_name, const char *domain_name,
                              const char *portid, bool oper_status,
                              DalDmlIntf *dmi);
#endif

  /**
    * @brief Method to get a parent ConfigKeyVal of UNC_KT_VTERMINAL
    *        from UNC_KT_VTERM_IF ConfigKeyVal pointer
    *
    * @param[in/out]  okey  pointer to parent ConfigKeyVal
    * @param[in]      ikey  pointer to the child configkeyval from
    *                       which the parent configkey val is obtained.
    *
    * @retval  UPLL_RC_SUCCESS      Successfull completion.
    * @retval  UPLL_RC_ERR_GENERIC  Failure case.
    **/
  upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                               ConfigKeyVal *parent_key);


  /**
    * @brief Verifies whether an interface is already created
    *        under the requested vterminal node.
    *
    * @param[in]  okey  pointer to the requested vterm_if ConfigKeyVal.
    * @param[in]  dmi   Pointer to DalDmlIntf Class.
    * @param[in]  req   This structure contains IpcReqRespHeader
    *                   (first 8 fields of input request structure).
    *
    * @retval  UPLL_RC_SUCCESS      Successfull completion.
    * @retval  UPLL_RC_ERR_GENERIC  Failure case.
    **/
  upll_rc_t IsVtermIfAlreadyExists(ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                   IpcReqRespHeader *req);

   /**
    * @brief Informs whether port_map is configured or not
    *        under the requested vterminal node.
    *
    * @param[in]  okey  pointer to the requested vterm_if ConfigKeyVal.
    * @param[in]  dmi   Pointer to DalDmlIntf Class.
    * @param[in]  req   This structure contains IpcReqRespHeader
    *                   (first 8 fields of input request structure).
    *
    * @retval  UPLL_RC_SUCCESS      Successfull completion.
    * @retval  UPLL_RC_ERR_GENERIC  Failure case.
    **/
  upll_rc_t GetPortmapInfo(ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
                           DalDmlIntf *dmi, InterfacePortMapInfo &iftype);
  upll_rc_t TranslateVbrIfToVtermIfError(ConfigKeyVal *&translated_ckv,
                           ConfigKeyVal *ckv_drv,
                           upll_keytype_datatype_t datatype, DalDmlIntf *dmi,
                           uint8_t* ctrlr_id);
   /**
    * @brief Gets vTerminal Interfcae name based on vtn_name and vterminal name.
    *
    * @param[in/out]   vtn_name   pointer to the requested vterm_if ConfigKeyVal.
    * @param[in/out]   vterminal  Pointer to DalDmlIntf Class.
    * @param[out]      vtermif    vTerminal Interface name.
    * @param[in]       ctrlr_id   Controller Id.
    * @param[in]       dmi        Pointer to DalDmlIntf Class.
    *
    * @retval  UPLL_RC_SUCCESS               Successfull completion.
    * @retval  UPLL_RC_ERR_GENERIC           Failure case.
    * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No entry in Datatbase.
    **/
  upll_rc_t GetVtermIfFromVexternal(uint8_t *vtn_name, uint8_t *vterminal,
                                    uint8_t *vtermif, uint8_t *ctrlr_id,
                                    DalDmlIntf *dmi);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif



