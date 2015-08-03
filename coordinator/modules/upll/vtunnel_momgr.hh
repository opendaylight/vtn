/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VTUNNEL_MOMGR_H
#define UNC_UPLL_VTUNNEL_MOMGR_H

#include <string>
#include "momgr_impl.hh"
#include "vnode_momgr.hh"
#include "vtn_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {


class VtunnelMoMgr : public VnodeMoMgr {
 private:
  static unc_key_type_t vtunnel_child[];
  static BindInfo       vtunnel_bind_info[];
  static BindInfo       conv_vtunnel_bind_info[];
  static BindInfo       key_vtunnel_maintbl_bind_info[];

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
  upll_rc_t GetValid(void *val, uint64_t indx, uint8_t *&valid,
      upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
    UPLL_FUNC_TRACE;
    if (val == NULL) return UPLL_RC_ERR_GENERIC;
    if (tbl == MAINTBL) {
      switch (indx) {
        case uudst::vtunnel::kDbiDesc:
          valid = &(reinterpret_cast<val_vtunnel *>(val)->
              valid[UPLL_IDX_DESC_VTNL]);
          break;
        case uudst::vtunnel::kDbiCtrlrName:
          valid = &(reinterpret_cast<val_vtunnel *>(val)->
              valid[UPLL_IDX_CONTROLLER_ID_VTNL]);
          break;
        case uudst::vtunnel::kDbiDomainId:
          valid = &(reinterpret_cast<val_vtunnel *>(val)->
              valid[UPLL_IDX_DOMAIN_ID_VTNL]);
          break;
        case uudst::vtunnel::kDbiUnderlayVtnName:
          valid = &(reinterpret_cast<val_vtunnel *>(val)->
              valid[UPLL_IDX_VTN_NAME_VTNL]);
          break;
        case uudst::vtunnel::kDbiVtepgrpName:
          valid = &(reinterpret_cast<val_vtunnel *>(val)->
              valid[UPLL_IDX_VTEP_GRP_NAME_VTNL]);
          break;
        case uudst::vtunnel::kDbiLabel:
          valid = &(reinterpret_cast<val_vtunnel *>(val)->
              valid[UPLL_IDX_LABEL_VTNL]);
          break;
        case uudst::vtunnel::kDbiOperStatus:
          valid = &(reinterpret_cast<val_vtunnel_st *>(val)->
              valid[UPLL_IDX_OPER_STATUS_VTNLS]);
          break;
        case uudst::vtunnel::kDbiDownCount:
          valid = NULL;
          break;
        default:
          return UPLL_RC_ERR_GENERIC;
      }
    } else if (tbl == CONVERTTBL) {
      switch (indx) {
        case uudst::convert_vtunnel::kDbiRefCount:
          valid = &(reinterpret_cast<val_convert_vtunnel *>(val)->
              valid[UPLL_IDX_REF_COUNT_CONV_VTNL]);
          break;
        case uudst::convert_vtunnel::kDbiLabel:
          valid = &(reinterpret_cast<val_convert_vtunnel *>(val)->
              valid[UPLL_IDX_LABEL_CONV_VTNL]);
          break;
        case uudst::convert_vtunnel::kDbiOperStatus:
          valid = &(reinterpret_cast<val_vtunnel_st *>(val)->
              valid[UPLL_IDX_OPER_STATUS_VTNLS]);
          break;
        case uudst::convert_vtunnel::kDbiDownCount:
          valid = NULL;
          break;
        default:
          return UPLL_RC_ERR_GENERIC;
      }
    }
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t UpdateConfigStatus(ConfigKeyVal *vtunnel_key,
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
  upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
          MoMgrTables tbl = MAINTBL);
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
  upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);
  /**
   * @brief Rename operation not needed for Overlay Keytype
   **/
  upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
         upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
         uint8_t *ctrlr_name ) {
    return UPLL_RC_SUCCESS;
  }
  /**
   * @brief Rename operation not needed for Overlay Keytype
   **/
  upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
         upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
         uint8_t *ctrlr_id) {
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
  bool FilterAttributes(void *&val1, void *val2, bool copy_to_running,
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
  bool CompareValidValue(void *&val1, void *val2, bool copy_to_running);

  /* Pure virtual from VnodeMoMgrImpl */
  upll_rc_t GetControllerDomainId(ConfigKeyVal *ikey,
                                 controller_domain_t *ctrlr_dom);
  upll_rc_t GetVnodeName(ConfigKeyVal *ikey,
                      uint8_t *&vtn_name, uint8_t *&vnode_name);

  /**
   * @Brief Get the VtepGroupConfigKeyVal from VtepGroupTbl
   *
   * @param[in] vtunnelVal                value structure
   * @param[in] dt_type                   database type
   * @param[in] dmi                       pointer to DalDmlIntf
   *
   * @retval UPLL_RC_SUCCESS                      Successful.
   * @retval UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT  Operation not allowed
   * @retval UPLL_RC_ERR_GENERIC          Generic failure.
   */
  upll_rc_t GetVtepGroup(val_vtunnel_t *vtunnelVal,
                                        upll_keytype_datatype_t dt_type,
                                        DalDmlIntf *dmi);
  /**
   * @Brief Validates the syntax of the specified key and value structure
   *        for KT_VTUNNEL keytype
   *
   * @param[in] req                       This structure contains
   *                                      IpcReqRespHeader(first 8 fields of input request structure).
   * @param[in] ikey                      ikey contains key and value structure.
   *
   * @retval UPLL_RC_SUCCESS              Successful.
   * @retval UPLL_RC_ERR_CFG_SYNTAX       Syntax error.
   * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE key_vlink is not available.
   * @retval UPLL_RC_ERR_GENERIC          Generic failure.
   * @retval UPLL_RC_ERR_INVALID_OPTION1  option1 is not valid.
   * @retval UPLL_RC_ERR_INVALID_OPTION2  option2 is not valid.
   */
  upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *ikey);
  upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                              DalDmlIntf *dmi,
                              IpcReqRespHeader *req);
  /**
   * @Brief Validates the syntax for KT_VTUNNEL keytype value structure.
   *
   * @param[in] val_vtunnel KT_VTUNNEL value structure.
   *
   * @retval UPLL_RC_ERR_GENERIC    Generic failure.
   * @retval UPLL_RC_SUCCESS        validation succeeded.
   * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
   */
  upll_rc_t ValidateVTunnelValue(val_vtunnel_t *val_vtunnel,
      uint32_t operation = UNC_OP_INVALID);
  /**
   * @Brief Checks if the specified key type(KT_VTUNNEL) and
   *        associated attributes are supported on the given controller,
   *        based on the valid flag
   *
   * @param[in] req               This structure contains
   *                              IpcReqRespHeader(first 8 fields of input request structure).
   * @param[in] ikey              ikey contains key and value structure.
   * @param[in] crtlr_name        Controller name.
   *
   * @retval  UPLL_RC_SUCCESS             Validation succeeded.
   * @retval  UPLL_RC_ERR_GENERIC         Validation failure.
   * @retval  UPLL_RC_ERR_INVALID_OPTION1 Option1 is not valid.
   * @retval  UPLL_RC_ERR_INVALID_OPTION2 Option2 is not valid.
   */

  upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
      const char * crtlr_name);
  /**
   * @Brief Checks if the specified key type and
   *        associated attributes are supported on the given controller,
   *        based on the valid flag.
   *
   * @param[in] val_vtunnel         Value Structure.
   * @param[in] attr                pointer to controller attribute
   * @param[in] operation           Operation Name
   *
   * @retval UPLL_RC_SUCCESS                    validation succeeded.
   * @retval UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR Attribute NOT_SUPPORTED.
   * @retval UPLL_RC_ERR_GENERIC                Generic failure.
   */
  upll_rc_t ValVTunnelAttributeSupportCheck(
       val_vtunnel_t *val_vtunnel,
  const uint8_t* attrs, unc_keytype_operation_t operation);

  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey) {
    UPLL_LOG_INFO("Not supported for this keytype. Returning Generic Error");
    return UPLL_RC_ERR_GENERIC;
  }
  bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                            int &nattr,
                            MoMgrTables tbl) {
    return true;
  }
  /* @brief         This is semantic check for KEY_VTUNNEL key type
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
                         DalDmlIntf *dmi);

 public:
  VtunnelMoMgr();
  virtual ~VtunnelMoMgr() {
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
  bool IsValidKey(void *tkey, uint64_t index, MoMgrTables tbl = MAINTBL);
/**
   * @Brief Validates the syntax for KT_VTUNNEL keytype key structure.
   *
   * @param[in] key_vtunnel KT_VTUNNEL key structure.
   *
   * @retval UPLL_RC_SUCCESS        validation succeeded.
   * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
   */
  upll_rc_t ValidateVTunnelKey(key_vtunnel_t *key_vtunnel,
                               uint32_t operation);

  upll_rc_t CreateVnodeConfigKey(ConfigKeyVal *ikey, ConfigKeyVal *&okey);
  /* @brief         This method invoke when the VTN merge hapeening between
     *                Running and DT import. This will checks the vnode name
     *                unique or not.
     *
     * @param[in]     keytype       UNC KEY TYPE
     * @param[in/out] ctrlr_id      Controller ID
     * @param[in]     conflict_ckv  key and value structure
     * @param[in]     dal    Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RC_SUCCESS                    Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
     * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
     * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
     * @retval  UPLL_RC_ERR_MERGE_CONFLICT         Semantic check error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
     *
     **/
  upll_rc_t MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
                     ConfigKeyVal *ikey, DalDmlIntf *dmi) {
     return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

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
  upll_rc_t AdaptValToVtnService(ConfigKeyVal *ikey,
                                 AdaptType adapt_type);

 /**
  * @Brief   Assign auto generated name to vtunnel, if ref_count is zero and
  *          increment the ref_count.Else ref_count greater than zero,
  *          use same vtunnel name which is there in input key.
  *
  * @param[in]  ikey                 ikey contains convert 
  *                                  vtunnel key and value structure
  * @param[in]  unified_vbr_name     Unified vbridge name
  * @param[in]  op                   operation type
  * @param[in]  config_mode          Get Configuration mode
  * @param[in]  vtn_name             vtn name
  * @param[in]  dmi                  Pointer to the DalDmlIntf(DB Interface)
  * 
  * @retval  UPLL_RC_SUCCESS                    Entries updated to database.
  * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
  * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
  * @retval  UPLL_RC_ERR_BAD_REQUEST            Input/Request invalid.
  * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
  * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
  */
  upll_rc_t ConvertVtunnel(ConfigKeyVal *ikey,
                           uint8_t *unified_vbr_name,
                           unc_keytype_operation_t op,
                           TcConfigMode config_mode,
                           string vtn_name,
                           DalDmlIntf *dmi);
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
                   ConfigKeyVal *&req, MoMgrTables tbl = MAINTBL);
  upll_rc_t GetChildConvertConfigKey(ConfigKeyVal *&okey,
                                     ConfigKeyVal *parent_key);
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
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif
