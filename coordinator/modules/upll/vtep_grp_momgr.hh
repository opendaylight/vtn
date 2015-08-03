/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VTEP_GRP_MOMGR_H
#define UNC_UPLL_VTEP_GRP_MOMGR_H

#include "momgr_impl.hh"
#include "vnode_child_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {


enum VtepGrpMoMgrTables {
  VTEPGRPTBL = 0,
  NVTEPGRPTABLES
};


class VtepGrpMoMgr : public VnodeChildMoMgr {
  private:
    static unc_key_type_t vtep_grp_child[];
    static BindInfo       vtep_grp_bind_info[];
    //  static BindInfo       key_vtep_grp_maintbl_update_bind_info[];
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
        upll_keytype_datatype_t dt_type, MoMgrTables tbl ) {
      UPLL_FUNC_TRACE;
      if (val == NULL) return UPLL_RC_ERR_GENERIC;
      if (tbl == MAINTBL) {
        switch (indx) {
          case uudst::vtep_group::kDbiCtrlrName:
            valid = &(reinterpret_cast<val_vtep_grp *>(val)->
                      valid[UPLL_IDX_CONTROLLER_ID_VTEPG]);
            break;
          case uudst::vtep_group::kDbiDesc:
            valid = &(reinterpret_cast<val_vtep_grp *>(val)->
                      valid[UPLL_IDX_DESCRIPTION_VTEPG]);
            break;
          default:
            return UPLL_RC_ERR_GENERIC;
        }
      }
      return UPLL_RC_SUCCESS;
    }

    upll_rc_t GetControllerDomainId(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    controller_domain *ctrlr_dom,
                                    DalDmlIntf *dmi);

    upll_rc_t UpdateConfigStatus(ConfigKeyVal *vtepgrp_key,
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

    upll_rc_t ValidateMessage(ConfigKeyVal *kval,
        upll_keytype_datatype_t dt_type,
        unc_keytype_operation_t op);

    /**
    * @brief  Perform Semantic Check
    *
    * @param[in]      ikey        ConfigKeyVal
    * @param[in]      upll_rc_t   UPLL_RC_ERR_CFG_SEMANTIC on error
    *                             UPLL_RC_SUCCESS on success
    **/
    upll_rc_t ValidateAttribute(ConfigKeyVal *ikey,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req);
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
    upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);

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

    /**
     * @Brief Validates the syntax of the specified key and value structure
     *        for KT_VTEP_GRP keytype
     *
     * @param[in] req                       This structure contains
     *                                      IpcReqRespHeader(first 8 fields of input request structure).
     * @param[in] ikey                      ikey contains key and value structure.
     *
     * @retval UPLL_RC_SUCCESS              Successful.
     * @retval UPLL_RC_ERR_CFG_SYNTAX       Syntax error.
     * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE key struct is not available.
     * @retval UPLL_RC_ERR_GENERIC          Generic failure.
     * @retval UPLL_RC_ERR_INVALID_OPTION1  option1 is not valid.
     * @retval UPLL_RC_ERR_INVALID_OPTION2  option2 is not valid.
     */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *ikey);

    /**
     * @Brief Validates the syntax for KT_VTEP_GRP keytype value structure.
     *
     * @param[in] val_vtep_grp KT_VTEP_GRP value structure.
     *
     * @retval UPLL_RC_ERR_GENERIC    Generic failure.
     * @retval UPLL_RC_SUCCESS        validation succeeded.
     * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
     */
    upll_rc_t ValidateVTepGrpValue(val_vtep_grp_t *val_vtep_grp,
        uint32_t operation = UNC_OP_INVALID);

    /**
     * @Brief Checks if the specified key type(KT_VTEP_GRP) and
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
     * @param[in] val_vtep_grp         Value Structure.
     * @param[in] attr                 pointer to controller attribute
     * @param[in] operation            Operation Name
     *
     * @retval UPLL_RC_SUCCESS                    validation succeeded.
     * @retval UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR Attribute NOT_SUPPORTED.
     * @retval UPLL_RC_ERR_GENERIC                Generic failure.
     */
    upll_rc_t ValVTepGrpAttributeSupportCheck(
        val_vtep_grp_t *val_vtep_grp,
        const uint8_t* attrs, unc_keytype_operation_t operation);

    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey) {
      UPLL_LOG_INFO("Not supported for this keytype. Returning Generic Error");
      return UPLL_RC_ERR_GENERIC;
    }
    bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
        int &nattr,
        MoMgrTables tbl ) {
      return true;
    }
    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
        upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
        uint8_t *ctrlr_name ) {
      return UPLL_RC_SUCCESS;
    }
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
        upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, uint8_t *ctrlr_id) {
      return UPLL_RC_SUCCESS;
    }
    upll_rc_t CreateVtunnelKey(ConfigKeyVal *ikey, ConfigKeyVal *&okey);
    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);
    upll_rc_t GetVnodeName(ConfigKeyVal *ikey,
        uint8_t *&vtn_name, uint8_t *&vnode_name) {
      return UPLL_RC_SUCCESS;
    }

 public:
    VtepGrpMoMgr();
    virtual ~VtepGrpMoMgr() {
      for (int i = VTEPGRPTBL; i < NVTEPGRPTABLES; i++)
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
     * @Brief Validates the syntax for KT_VTEP_GRP keytype key structure.
     *
     * @param[in] key_vtep_grp KT_VTEP_GRP key structure.
     *
     * @retval UPLL_RC_SUCCESS        validation succeeded.
     * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
     */
    upll_rc_t ValidateVTepGrpKey(key_vtep_grp_t *key_vtep_grp,
                                 uint32_t operation);
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
};


}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif
