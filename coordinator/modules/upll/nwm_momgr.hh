/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VBR_NWMONITOR_MOMGR_H
#define UNC_UPLL_VBR_NWMONITOR_MOMGR_H

#include <string>
#include "momgr_impl.hh"
#include "vnode_child_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

class NwMonitorMoMgr : public VnodeChildMoMgr {
  private:
    static unc_key_type_t nwm_child[];
    static BindInfo nwm_bind_info[];
    static BindInfo key_nwm_maintbl_update_bind_info[];
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
      if (val == NULL) return UPLL_RC_ERR_GENERIC;
      if (tbl == MAINTBL) {
        switch (indx) {
          case uudst::vbridge_networkmonitor_group::kDbiOperStatus:
            valid = &(reinterpret_cast<val_nwm_st *>
                      (val))->valid[UPLL_IDX_STATUS_NWMS];
            break;
          case uudst::vbridge_networkmonitor_group::kDbiAdminStatus:
            valid = &(reinterpret_cast<val_nwm *>
                      (val))->valid[UPLL_IDX_ADMIN_STATUS_NWM];
            break;
          default:
            return UPLL_RC_ERR_GENERIC;
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
    bool FilterAttributes(void *&val1, void *val2, bool audit_status,
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
    bool CompareValidValue(void *&val1, void *val2, bool audit);

    upll_rc_t UpdateConfigStatus(ConfigKeyVal *req, unc_keytype_operation_t op,
                               uint32_t driver_result, ConfigKeyVal *upd_key,
                               DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key = NULL);
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
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                              MoMgrTables tbl = MAINTBL);
    /**
     * @brief  Allocates for the specified val in the given configuration in the     * specified table.
     *
     * @param[in]  ck_val   Reference pointer to configval structure allocated.      * @param[in]  dt_type  specifies the configuration candidate/running/state
     * @param[in]  tbl      specifies if the corresponding table is the  main
     *                      table / controller table or rename table.
     *
     * @retval     UPLL_RC_SUCCESS      Successfull completion.
     * @retval     UPLL_RC_ERR_GENERIC  Failure case.
     **/
    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl = MAINTBL);
    /**
     * @brief      Method to get a configkeyval of a specified keytype from an
     * input configkeyval
     *
     * @param[in/out]  okey                 pointer to output ConfigKeyVal
     * @param[in]      parent_key           pointer to the configkeyval from
     * which the output configkey val is initialized.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     */
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);
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
    /* Rename */
    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);
    bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                              int &nattr, MoMgrTables tbl);
    /**
     * @Brief Validates the syntax of the specified key and value structure
     *        for KT_VBR_NWMONITOR keytype
     *
     * @param[in] req                       This structure contains
     *                                      IpcReqRespHeader(first 8 fields of
     *                                      input request structure).
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
    /**
     * @Brief Validates the syntax for KT_VBR_NWMONITOR keytype value structure.
     *
     * @param[in] val_nwm   KT_VBR_NWMONITOR value structure.
     * @param[in] operation Operation name.
     *
     * @retval UPLL_RC_ERR_GENERIC    Generic failure.
     * @retval UPLL_RC_SUCCESS        validation succeeded.
     * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
     */
    upll_rc_t ValidateNwMonValue(val_nwm_t *val_nwm,
        unc_keytype_operation_t operation = UNC_OP_INVALID);
    /**
     * @Brief Checks if the specified key type(KT_VBR_NWMONITOR) and
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
     * @param[in] val_nwm         Value Structure.
     * @param[in] attr            pointer to controller attribute
     * @param[in] operation       Operation Name
     *
     * @retval UPLL_RC_SUCCESS                    validation succeeded.
     * @retval UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR Attribute NOT_SUPPORTED.
     * @retval UPLL_RC_ERR_GENERIC                Generic failure.
     */
    upll_rc_t ValNwMonAttributeSupportCheck(val_nwm_t *val_nwm,
        const uint8_t* attrs,
        unc_keytype_operation_t operation);

    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);
    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);
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
    NwMonitorMoMgr();
    virtual ~NwMonitorMoMgr() {
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

    /* @brief         This method invoke when the Network MOnitor merge
     *                hapeening between Running and DT import. This will
     *                checks the network monitor name is unique or not
     *                under the parent VTN
     *
     * @param[in]     keytype       UNC KEY TYPE
     * @param[in/out] ctrlr_id      Controller ID
     * @param[in]     conflict_ckv  key and value structure
     * @param[in]     import_type   Specifies the import type
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

    upll_rc_t MergeValidate(unc_key_type_t keytype,
                            const char *ctrlr_id,
                            ConfigKeyVal *ikey,
                            DalDmlIntf *dmi,
                            upll_import_type import_type);
    /**
     * @Brief Validates the syntax for KT_VBR_NWMONITOR keytype key structure.
     *
     * @param[in] key_nwm KT_VBR_NWMONITOR key structure.
     * @param[in] operation    Operation type.
     *
     * @retval UPLL_RC_SUCCESS        validation succeeded.
     * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
     */
    upll_rc_t ValidateNwMonKey(key_nwm_t *key_nwm,
        unc_keytype_operation_t operation = UNC_OP_INVALID);
    upll_rc_t OnNwmonFault(
       string ctrlr_name ,
       string domain_id,
       const key_vtn &key_vtn,
       const pfcdrv_network_mon_alarm_data_t &alarm_data,
       bool alarm_raised,
       DalDmlIntf *dmi);

    /**
     * @Brief Validates Same network monitor group name exist
     *        during VTN stiching.
     *
     * @param[in] org_vtn_ckv       Orginal VTN ConfigKeyVal.
     * @param[in] rename_vtn_ckv    Rename VTN ConfigKeyVal.
     * @param[in] dmi               Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval UPLL_RC_SUCCESS            validation succeeded.
     * @retval UPLL_RC_ERR_MERGE_CONFLICT Same Network monitor group name exist.
     *
     */
    upll_rc_t ValidateVtnRename(ConfigKeyVal *org_vtn_ckv,
                                ConfigKeyVal *rename_vtn_ckv, DalDmlIntf *dmi);
};
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif
