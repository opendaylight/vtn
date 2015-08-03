/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VRT_MOMGR_H
#define UNC_UPLL_VRT_MOMGR_H

#include <string>
#include "momgr_impl.hh"
#include "vnode_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {



class VrtMoMgr : public VnodeMoMgr {
 private:
  static unc_key_type_t vrt_child[];
  static BindInfo vrt_bind_info[];
  static BindInfo vrt_rename_bind_info[];
  static BindInfo key_vrt_maintbl_update_bind_info[];
  static BindInfo key_vrt_renametbl_update_bind_info[];
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
   **/
  upll_rc_t GetValid(void *val,
                     uint64_t indx,
                     uint8_t *&valid,
                     upll_keytype_datatype_t dt_type,
                     MoMgrTables tbl) {
    if (val == NULL)
      return UPLL_RC_ERR_GENERIC;
    if (tbl == MAINTBL) {
      switch (indx) {
         case uudst::vrouter::kDbiOperStatus:
           valid = &(reinterpret_cast<val_vbr_st *>(val))->
                valid[UPLL_IDX_OPER_STATUS_VRTS];
           break;
         case uudst::vrouter::kDbiDownCount:
            valid = NULL;
            break;
        case uudst::vrouter::kDbiCtrlrName:
          valid = &(reinterpret_cast<val_vrt *>(val))->
              valid[UPLL_IDX_CONTROLLER_ID_VRT];
          break;
        case uudst::vrouter::kDbiDomainId:
          valid = &(reinterpret_cast<val_vrt *>(val))->
              valid[UPLL_IDX_DOMAIN_ID_VRT];
          break;
        case uudst::vrouter::kDbiVrtDesc:
          valid = &(reinterpret_cast<val_vrt *>(val))->
              valid[UPLL_IDX_DESC_VRT];
          break;
        case uudst::vrouter::kDbiDhcprelayAdminstatus:
          valid = &(reinterpret_cast<val_vrt *>(val))->
              valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT];
          break;
        default:
          return UPLL_RC_ERR_GENERIC;
      }
    } else if (tbl == RENAMETBL) {
       switch (indx) {
         case uudst::vnode_rename::kDbiCtrlrVtnName:
            valid = &(reinterpret_cast<val_rename_vnode *>
                    (val))->valid[UPLL_CTRLR_VTN_NAME_VALID];
            break;
         case uudst::vnode_rename::kDbiCtrlrVnodeName:
            valid = &(reinterpret_cast<val_rename_vnode *>
                    (val))->valid[UPLL_CTRLR_VNODE_NAME_VALID];
            break;
         default:
            break;
       }
    }
    return UPLL_RC_SUCCESS;
  }

  /**
   * @brief  Filters the attributes which need not be sent to controller
   *
   * @param[in/out]  val1   first record value instance.
   * @param[in]      val2   second record value instance.
   * @param[in]      audit  Not used for VTN
   * @param[in]      op     Operation to be performed
   *
   **/
  bool FilterAttributes(void *&val1,
                        void *val2,
                        bool audit_status,
                        unc_keytype_operation_t op);
  /**
   * @brief  Compares the valid value between two database records.
   * 	     if both the values are same, update the valid flag for corresponding
   * 	     attribute as invalid in the first record.
   *
   * @param[in/out]  val1   first record value instance.
   * @param[in]      val2   second record value instance.
   * @param[in]      audit  if true, CompareValidValue called from audit process.
   *
   **/
  bool CompareValidValue(void *&val1,
                         void *val2,
                         bool audit);

  upll_rc_t SwapKeyVal(ConfigKeyVal *ikey,
                       ConfigKeyVal *&okey,
                       DalDmlIntf *dmi,
                       uint8_t *ctrlr,
                       bool &no_rename);
  upll_rc_t UpdateConfigStatus(ConfigKeyVal *req,
                               unc_keytype_operation_t op,
                               uint32_t result,
                               ConfigKeyVal *upd_key,
                               DalDmlIntf *dmi,
                               ConfigKeyVal *ctrlr_key = NULL);
  /**
   * @brief  Update config status for commit result and vote result.
   *
   * @param[in/out]  ckv_running  ConfigKeyVal instance.
   * @param[in]      cs_status    either UNC_CS_INVALID or UNC_CS_APPLIED.
   * @param[in]      phase        specify the phase (CREATE,DELETE or UPDATE)
   * @param[in]      dmi          Pointer to the DalDmlIntf(DB Interface)
   *
   **/
  upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                    uuc::UpdateCtrlrPhase phase,
                                    ConfigKeyVal *&ckv_running,
                                    DalDmlIntf *dmi);
  /**
   * @Brief  Validates the syntax of the specified key and value structure
   *         for KT_VRT keytype
   *
   * @param[in]  req    This structure contains IpcReqRespHeader
   *                    (first 8 fields of input request structure).
   * @param[in]  ikey   ikey contains key and value structure.
   *
   * @retval  UPLL_RC_SUCCESS                Successful.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX         Syntax error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE   Key_Vrt is not available.
   * @retval  UPLL_RC_ERR_GENERIC            Generic failure.
   * @retval  UPLL_RC_ERR_INVALID_OPTION1    Option1 is not valid.
   * @retval  UPLL_RC_ERR_INVALID_OPTION2    Option2 is not valid.
   */
  upll_rc_t ValidateMessage(IpcReqRespHeader *req,
                            ConfigKeyVal *kval);

  /**
   * @Brief  Checks if the specified key type(KT_VRT) and
   *         associated attributes are supported on the given controller,
   *         based on the valid flag
   *
   * @param[in]  req               This structure contains IpcReqRespHeader
   *                               (first 8 fields of input request structure).
   * @param[in]  ikey              ikey contains key and value structure.
   * @param[in]  ctrlr_name        Controller name associated with ikey.
   *
   * @retval  UPLL_RC_SUCCESS               Validation succeeded.
   * @retval  UPLL_RC_ERR_GENERIC           Validation failure.
   * @retval  UPLL_RC_ERR_INVALID_OPTION1   Option1 is not valid.
   * @retval  UPLL_RC_ERR_INVALID_OPTION2   Option2 is not valid.
   */
  upll_rc_t ValidateCapability(IpcReqRespHeader *req,
                               ConfigKeyVal *ikey,
                               const char *ctrlr_name);

  /**
   * @Brief  Validates the syntax for KT_VRT Keytype key structure.
   *
   * @param[in]  key_vrt  KT_VRT key structure.
   * @param[in]  operation operation type.
   *
   * @retval  UPLL_RC_SUCCESS          validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX   validation failed.
   */
  upll_rc_t ValidateVrtKey(key_vrt *vrt_key,
                       unc_keytype_operation_t operation = UNC_OP_INVALID);

  /**
   * @Brief  Validates the syntax for KT_VRT keytype value structure.
   *
   * @param[in]  val_vrt    KT_VRT value structure.
   * @param[in]  operation  Operation type.
   *
   * @retval  UPLL_RC_SUCCESS                validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX         validation failed.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE   description field is not available
   */
  upll_rc_t ValidateVrtValue(val_vrt *vrt_val,
                             uint32_t operation);

  /**
   * @Brief Validates the syntax for KT_VROUTER keytype val_ping structure.
   *
   * @param[in]  val_ping   KT_VROUTER val_ping structure.
   *
   * @retval  UPLL_RC_SUCCESS          validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX   validation failed.
   */
  upll_rc_t ValidateVrtPingValue(val_ping *ping_val);

  /**
   *  @Brief  Validates the syntax for KT_VRT Rename structure.
   *
   * @param[in]  val_rename_vrt  KT_VRT Rename structure.
   *
   * @retval  UPLL_RC_SUCCESS         validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
   */
  upll_rc_t ValidateVrtRenameValue(val_rename_vrt *vrt_rename_val);

  /**
   * @Brief  Checks if the specified key type and
   *         associated attributes are supported on the given controller,
   *         based on the valid flag.
   *
   * @param[in]  vrt_val         KT_VROUTER Value structure.
   * @param[in]  attrs           Pointer to controller attribute.
   * @param[in]  operation       Operation name.
   *
   * @retval  UPLL_RC_SUCCESS                       validation succeeded.
   * @retval  UPLL_RC_ERR_EXCEEDS_RESOURSE_LIMIT    Instance count limit is exceeds.
   * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR    Attribute NOT_SUPPORTED.
   * @retval  UPLL_RC_ERR_GENERIC                   Generic failure.
   */
  upll_rc_t ValidateVrtAttributeSupportCheck(val_vrt_t *vrt_val,
                                             const uint8_t *attrs,
                                             unc_keytype_operation_t operation);

  /**
   * @brief  Duplicates the input configkeyval including the key and val.
   * based on the tbl specified.
   *
   * @param[in]  okey   Output Configkeyval - allocated within the function
   * @param[in]  req    Input ConfigKeyVal to be duplicated.
   * @param[in]  tbl    specifies if the val structure belongs to the main table/ controller table or rename table.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   **/
  upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey,
                            ConfigKeyVal *&req,
                            MoMgrTables tbl = MAINTBL);
  /**
   * @brief  Allocates for the specified val in the given configuration in the     * specified table.
   *
   * @param[in/out]  ck_val   Reference pointer to configval structure
   *                          allocated.
   * @param[in]      dt_type  specifies the configuration candidate/running/
   *                          state
   * @param[in]      tbl      specifies if the corresponding table is the
   *                          main table / controller table or rename table.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   **/
  upll_rc_t AllocVal(ConfigVal *&ck_val,
                     upll_keytype_datatype_t dt_type,
                     MoMgrTables tbl = MAINTBL);
  /**
   * @brief      Method to get a configkeyval of a specified keytype from an input configkeyval
   *
   * @param[in/out]  okey                 pointer to output ConfigKeyVal
   * @param[in]      parent_key           pointer to the configkeyval from which the output configkey val is initialized.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
                              ConfigKeyVal *parent_key);
  /**
   * @brief      Method to get a configkeyval of the parent keytype
   *
   * @param[in/out]  okey           pointer to parent ConfigKeyVal
   * @param[in]      ikey           pointer to the child configkeyval from
   * which the parent configkey val is obtained.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   **/
  upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                               ConfigKeyVal *ikey);
  upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf *dmi,
                                    controller_domain *ctrlr_dom);
  upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                             upll_keytype_datatype_t dt_type,
                             DalDmlIntf *dmi,
                             uint8_t *ctrlr_id);

  /* Pure virtual from VnodeMoMgrImpl */
  // uint8_t* GetControllerId(ConfigKeyVal *ikey);
  upll_rc_t GetControllerDomainId(ConfigKeyVal *ikey,
                                  controller_domain *ctrlr_dom);

  upll_rc_t GetVnodeName(ConfigKeyVal *ikey,
                         uint8_t *&vtn_name,
                         uint8_t *&vnode_name);
  upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                         DalDmlIntf *dmi);

  /* @brief     To convert the value structure read from DB to
   * 		VTNService during READ operations
   * @param[in/out] ikey      Pointer to the ConfigKeyVal Structure
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   *
   **/
  upll_rc_t AdaptValToVtnService(ConfigKeyVal *ikey, AdaptType adapt_type);

 public:
  VrtMoMgr();
  virtual ~VrtMoMgr() {
    for (int i = 0; i < ntable; i++)
      if (table[i]) {
        delete table[i];
      }
    delete[] table;
  }
  /**
   * @brief      Method to check if individual portions of a key are valid
   *
   * @param[in/out]  ikey                 pointer to ConfigKeyVal referring to a UNC resource
   * @param[in]      index                db index associated with the variable
   *
   * @retval         true                 input key is valid
   * @retval         false                input key is invalid.
   **/
  bool IsValidKey(void *tkey,
                  uint64_t index, MoMgrTables tbl = MAINTBL);
  upll_rc_t MergeValidate(unc_key_type_t keytype,
                          const char *ctrlr_id,
                          ConfigKeyVal *conflict_ckv,
                          DalDmlIntf *dmi,
                          upll_import_type import_type);
  /* Rename */
  upll_rc_t GetRenameInfo(ConfigKeyVal *ikey,
                          ConfigKeyVal *okey,
                          ConfigKeyVal *&rename_info,
                          DalDmlIntf *dmi,
                          const char *ctrlr_id,
                          bool &renamed);
  bool GetRenameKeyBindInfo(unc_key_type_t key_type,
                            BindInfo *&binfo,
                            int &nattr,
                            MoMgrTables tbl);
  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
                            ConfigKeyVal *ikey);
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
   * @brief create entry in Vnode Rename Table,
   *        with the renamed VTN details fetched from VTN rename Table
   *
   *
   * @param[in]  ikey       ConfigKeyVal pointer
   * @param[in]  dt_type    specifies the database type
   * @param[in]  ctrlr_id   pointer to the controller name
   * @param[in]  dmi        DalDmlIntf pointer
   *
   *
   * @retval UPLL_RC_SUCCESS      Successful
   * @retval UPLL_RC_ERR_GENERIC  failed to update the VbrIf
   */
  upll_rc_t CreateVnodeConfigKey(ConfigKeyVal *ikey,
                                 ConfigKeyVal *&okey);
  upll_rc_t EnableAdminStatus(ConfigKeyVal *ikey,
                              DalDmlIntf *dmi,
                              IpcReqRespHeader *req);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif
