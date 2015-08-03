/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VBR_VLANMAP_MOMGR_H
#define UNC_UPLL_VBR_VLANMAP_MOMGR_H

#include <string>
#include "momgr_impl.hh"
#include "vnode_child_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

namespace uuds = unc::upll::dal::schema;
namespace uudst = unc::upll::dal::schema::table;

class VlanMapMoMgr : public VnodeChildMoMgr {
  private:
    static BindInfo vlan_map_bind_info[];
    static BindInfo vlan_map_maintbl_update_key_bind_info[];
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
      if (val == NULL) return UPLL_RC_ERR_GENERIC;
      if (tbl == MAINTBL) {
        switch (indx) {
          /* VlanmapOnBoundary: New attributes added*/
          case uudst::vbridge_vlanmap::kDbiVlanid:
            valid = &(reinterpret_cast<pfcdrv_val_vlan_map *>
                      (val))->vm.valid[UPLL_IDX_VLAN_ID_VM];
            break;
          case uudst::vbridge_vlanmap::kDbiBdryRefCount:
            valid = &(reinterpret_cast<pfcdrv_val_vlan_map *>
                      (val))->valid[PFCDRV_IDX_BDRY_REF_COUNT];
            break;
          default:
            return UPLL_RC_ERR_GENERIC;
        }
      }
      return UPLL_RC_SUCCESS;
    }

   /**
    * @brief  Update parent oper status on delete for Transaction commit
    *
    * @param[in]  ikey          ConfigKeyVal instance
    * @param[in]   dmi           Database connection parameter

    * @retval  UPLL_RC_SUCCESS      Completed successfully.
    * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
    */
    upll_rc_t UpdateParentOperStatus(ConfigKeyVal *ikey,
                                     DalDmlIntf *dmi,
                                     uint32_t driver_result);
    upll_rc_t UpdateVnodeOperStatus(ConfigKeyVal *ikey,
                                     DalDmlIntf *dmi,
                                     uint32_t driver_result);

    upll_rc_t UpdateConfigStatus(ConfigKeyVal *req,
                                 unc_keytype_operation_t op,
                                 uint32_t driver_result,
                                 ConfigKeyVal *upd_key, DalDmlIntf *dmi,
                                 ConfigKeyVal *ctrlr_key = NULL);

    upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                      uuc::UpdateCtrlrPhase phase,
                                      ConfigKeyVal *&ckv_running,
                                      DalDmlIntf *dmi);
    /**
     * @Brief  Validates the syntax of the specified key and value structure
     *         for KT_VBR_VLANMAP keytype
     *
     * @param[in]  req    This structure contains IpcReqRespHeader
     *                    (first 8 fields of input request structure).
     * @param[in]  ikey   ikey contains key and value structure.
     *
     * @retval  UPLL_RC_SUCCESS               Successful.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX        Syntax error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  key_vlan_map is not available.
     * @retval  UPLL_RC_ERR_GENERIC           Generic failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
     */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey);

    /**
     * @Brief  Checks if the specified key type(KT_VBR_VLANMAP) and
     *         associated attributes are supported on the given controller,
     *         based on the valid flag
     *
     * @param[in]  req               This structure contains IpcReqRespHeader
     *                               (first 8 fields of input request structure).
     * @param[in]  ikey              ikey contains key and value structure.
     * @param[in]  ctrlr_id          controller associated with the input ikey.
     *
     * @retval  UPLL_RC_SUCCESS              Validation succeeded.
     * @retval  UPLL_RC_ERR_GENERIC          Validation failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1  Option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2  Option2 is not valid.
     */
    upll_rc_t ValidateCapability(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 const char *ctrlr_name);

    /**
     * @Brief  Checks if the specified key type and
     *         associated attributes are supported on the given controller,
     *         based on the valid flag.
     *
     * @param[in]  val_vlan_map    KT_VBR_VLANMAP Value structure.
     * @param[in]  attrs           Pointer to controller attribute.
     * @param[in]  operation       Operation name.
     *
     * @retval  UPLL_RC_SUCCESS                     validation succeeded.
     * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR  Attribute NOT_SUPPORTED.
     * @retval  UPLL_RC_ERR_GENERIC                 Generic failure.
     **/
    upll_rc_t ValVlanmapAttributeSupportCheck(
        val_vlan_map *vlanmap_val,
        const uint8_t *attrs,
        unc_keytype_operation_t operation);

    /**
     * @Brief  Validates the syntax for KT_VBR_VLANMAP Keytype key structure.
     *
     * @param[in]  key_vlan_map  KT_VBR_VLANMAP key structure.
     * @param[in]  operation     Operation type.
     *
     * @retval  UPLL_RC_SUCCESS        validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX validation failed.
     *
     */
    upll_rc_t ValidateVlanmapKey(key_vlan_map *vlan_map_key,
                         unc_keytype_operation_t operation);

    /**
     * @Brief  Validates the syntax for KT_VBR_VLANMAP keytype value structure.
     *
     * @param[in]  val_vlan_map   KT_VBR_VLANMAP value structure.
     * @param[in]  operation      Operation type.
     *
     * @retval  UPLL_RC_SUCCESS                validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX         validation failed.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE   description field is not available
     **/
    upll_rc_t ValidateVlanmapValue(val_vlan_map *vlanmap_val,
                                   uint32_t op);

    /**
     * @brief  Perform Semantic Check to check Different vbridges
     *          contain same switch-id and vlan-id
     *
     * @param[in]       ikey        ConfigKeyVal
     * @param[out]      upll_rc_t   UPLL_RC_ERR_CFG_SEMANTIC on error
     *                                UPLL_RC_SUCCESS on success
     **/
    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);
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
    upll_rc_t AllocVal(ConfigVal *&ck_val,
                       upll_keytype_datatype_t dt_type,
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
    uint8_t* GetControllerId(ConfigKeyVal *ck_vbr,
                             upll_keytype_datatype_t dt_type,
                             DalDmlIntf *dmi);
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

    /* Rename */
    bool GetRenameKeyBindInfo(unc_key_type_t key_type,
                              BindInfo *&binfo,
                              int &nattr,
                              MoMgrTables tbl);
    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
                              ConfigKeyVal *ikey);
    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);
    bool ResetDataForSibling(key_vlan_map *key_vmap,
                             uudst::vbridge_vlanmap::kVbrVlanMapIndex index);
    /**
     * @brief  VlanmapOnBoundary:
     *         Verifies whether the flow filter or policing map configured
     *         in the requested boundary vlink request of logical port id
     *         is SW or SD
     *
     * @param[in]   req          This structure contains
     *                           IpcReqRespHeader(first 8 fields of
     *                           input request structure).
     * @param[out]  ikey         ConfigKeyVal instance of vlink.
     * @param[in]   dmi          pointer to DalDmlIntf
     **/
    upll_rc_t CheckIfFfPmConfigured(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey, DalDmlIntf *dmi);

    /* @brief             To return Success if all the valid flags are
     *                      INVALID during update operation
     * @param[in]  val_vlan_map    Value Structure
     *
     * @retval  true               All flags are INVALID.
     * @retval  false              One of the attribute is not INVALID.
     *
     **/
    bool IsAllAttrInvalid(val_vlan_map_t *val_vlanmap) {
      for (unsigned int loop = 0;
        loop < sizeof(val_vlanmap->valid)/sizeof(uint8_t); ++loop) {
        if (UNC_VF_INVALID != val_vlanmap->valid[loop])
          return false;
      }
      return true;
    }
    static uint16_t kVbrVlanMapNumChildKey;
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
    VlanMapMoMgr();
    virtual ~VlanMapMoMgr() {
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
    upll_rc_t IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ckv, DalDmlIntf *dmi,
                                          IpcReqRespHeader *req);

  /* @brief         READ_SIBLING_BEGIN: Gets the first MO from the sibling group
   *                under the parent
   *                specified in the key from the specified UNC database
   *                READ_SIBLING: Gets the next MO from the sibling group
   *                under the parent
   *                specified in the key from the specified UNC database
   *
   * @param[in]     req    Pointer to IpcResResHeader
   * @param[in/out] key    Pointer to the ConfigKeyVal Structure
   * @param[in]     begin  boolean variable to decide the sibling operation
   * @param[in]     dal    Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
   *
   * @Note: Overridden from base class MoMgrImpl
   **/
  virtual upll_rc_t ReadSiblingMo(IpcReqRespHeader *req,
                                  ConfigKeyVal *key,
                                  bool begin,
                                  DalDmlIntf *dal);
  /**
   * @brief  VlanmapOnBoundary
   *         To handles boundary vlink logical port id - SW or SD request
   *
   * @param[in]   req          This structure contains
   *                           IpcReqRespHeader(first 8 fields of
   *                           input request structure).
   * @param[out]  ikey         ConfigKeyVal instance of vlink.
   * @param[out]  db_vlink     ConfigKeyVal instance of vlink read from DB.
   * @param[out]  vlanmap_ckv  ConfigKeyVal instance of vlanmap.
   * @param[in]   dmi          pointer to DalDmlIntf
   **/
  upll_rc_t BoundaryMapReq(IpcReqRespHeader *req,
                               ConfigKeyVal *ikey, ConfigKeyVal *db_vlink,
                               ConfigKeyVal *vlanmap_ckv,
                               ConfigKeyVal *uppl_bdry, DalDmlIntf *dmi);

  upll_rc_t ReadSiblingBeginMo(IpcReqRespHeader *header,
                               ConfigKeyVal *ikey,
                               bool begin,
                               DalDmlIntf *dmi);
  upll_rc_t ReadSiblingCount(IpcReqRespHeader *header,
                             ConfigKeyVal *ikey,
                             DalDmlIntf *dmi);
  upll_rc_t AdaptValToVtnService(ConfigKeyVal *ikey, AdaptType adapt_type);
  upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);
  upll_rc_t DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);
  upll_rc_t TranslateVlanmapError(ConfigKeyVal **err_ckv,
                                  ConfigKeyVal *ckv_running,
                                  DalDmlIntf *dmi,
                                  upll_keytype_datatype_t datatype);
  upll_rc_t TxUpdateErrorHandler(ConfigKeyVal *req,
      ConfigKeyVal *ck_main,
      DalDmlIntf *dmi,
      upll_keytype_datatype_t dt_type,
      ConfigKeyVal **err_ckv,
      IpcResponse *ipc_resp);


  /**
   * @brief  Duplicates the input configkeyval including the key and val.
   * based on the tbl specified.
   *
   * @param[in]  okey   Output Configkeyval - allocated within the function
   * @param[in]  req    Input ConfigKeyVal to be duplicated.
   * @param[in]  tbl    specifies if the val structure belongs to the main
   *                    table/controller table or rename table.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   **/
  upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey,
                            ConfigKeyVal *&req,
                            MoMgrTables tbl = MAINTBL);
  /**
   * @brief      Method to get a configkeyval of a specified keytype
   * from an input configkeyval
   *
   * @param[in/out]  okey          pointer to output ConfigKeyVal
   * @param[in]      parent_key    pointer to the configkeyval from which the
   *                               output configkey val is initialized.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
                              ConfigKeyVal *parent_key);
  upll_rc_t CheckIfVnodeisVlanmapped(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi);

  upll_rc_t PartialMergeValidate(unc_key_type_t keytype,
                                const char *ctrlr_id,
                                ConfigKeyVal *err_ckv,
                                DalDmlIntf *dmi);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif
