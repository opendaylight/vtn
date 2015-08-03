/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VBR_IF_MOMGR_H
#define UNC_UPLL_VBR_IF_MOMGR_H

#include <string>
#include <sstream>
#include <set>
#include "momgr_impl.hh"
#include "vnode_child_momgr.hh"
#include "vlink_momgr.hh"
namespace unc {
namespace upll {
namespace kt_momgr {

#if 0
enum vbr_if_numbers {
  VBR_IF_1 = 0x80,
  VBR_IF_2 = 0x40
};
#endif


#if 0
#define INTERFACE_TYPE 0xF0
#define INTERFACE_TYPE_BOUNDARY 0xC0
#define INTERFACE_TYPE_LINKED 0x30

enum if_type {
  kUnboundInterface = 0x0,
  kMappedInterface,
  kBoundaryInterface,
  kLinkedInterface
};
#endif


/*TODO remove when including driver header file */
typedef struct val_drv_vbr_if {
  val_vbr_if_t            vbr_if_val;
  uint8_t                 vex_name[32];
  uint8_t                 vex_if_name[32];
  uint8_t                 vex_link_name[32];
  uint8_t                 valid[4];
} val_drv_vbr_if_t;


class VbrIfMoMgr : public VnodeChildMoMgr {
 private:
  static unc_key_type_t vbr_if_child[];
  static BindInfo       vbr_if_bind_info[];
  static BindInfo       key_vbr_if_maintbl_bind_info[];

  //  Added for converttbl
  static BindInfo       convert_vbr_if_bind_info[];

  /* @brief      Returns admin and portmap information if portmap is
   *             valid. Else returns NULL for portmap
   *
   * @param[in]   ikey     Pointer to ConfigKeyVal
   * @param[out]  valid_pm portmap is valid
   * @param[out]  pm       pointer to portmap informtation if valid_pm
   *
   * @retval  UPLL_RC_SUCCESS      Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
   *
   **/
  virtual upll_rc_t GetPortMap(ConfigKeyVal *ikey,
                               uint8_t &valid_pm,
                               val_port_map_t *&pm, uint8_t &valid_admin,
                               uint8_t &admin_status) {
     UPLL_FUNC_TRACE;
     if (ikey == NULL) return UPLL_RC_ERR_GENERIC;
     if (ikey->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf)
      return UPLL_RC_SUCCESS;
     val_drv_vbr_if *drv_vbrif = reinterpret_cast<val_drv_vbr_if *>
                                                (GetVal(ikey));
     if (!drv_vbrif) {
       UPLL_LOG_DEBUG("Invalid param");
       return UPLL_RC_ERR_GENERIC;
     }
     val_vbr_if *ifval = &drv_vbrif->vbr_if_val;
     valid_pm = ifval->valid[UPLL_IDX_PM_VBRI];
     if (valid_pm == UNC_VF_VALID)
       pm = &ifval->portmap;
     else
       pm = NULL;
     valid_admin = ifval->valid[UPLL_IDX_ADMIN_STATUS_VBRI];
     admin_status = ifval->admin_status;
     return UPLL_RC_SUCCESS;
  }

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
    if (val == NULL) return UPLL_RC_ERR_GENERIC;
    if (tbl == MAINTBL) {
      switch (indx) {
        case uudst::vbridge_interface::kDbiOperStatus:
          valid = &(reinterpret_cast<val_vbr_if_st *>(val))->
                                  valid[UPLL_IDX_OPER_STATUS_VBRIS];
          break;
        case uudst::vbridge_interface::kDbiDownCount:
          valid = NULL;
          break;
        case uudst::vbridge_interface::kDbiAdminStatus:
          valid = &(reinterpret_cast<val_vbr_if *>(val))->
                                  valid[UPLL_IDX_ADMIN_STATUS_VBRI];
          break;
        case uudst::vbridge_interface::kDbiDesc:
          valid = &(reinterpret_cast<val_vbr_if *>(val))->
                                  valid[UPLL_IDX_DESC_VBRI];
          break;
        case uudst::vbridge_interface::kDbiValidPortMap:
          valid = &(reinterpret_cast<val_vbr_if *>(val))->
                                  valid[UPLL_IDX_PM_VBRI];
          break;
        case uudst::vbridge_interface::kDbiLogicalPortId:
          valid = &(reinterpret_cast<val_vbr_if *>(val))->portmap.
                                valid[UPLL_IDX_LOGICAL_PORT_ID_PM];
          break;
        case uudst::vbridge_interface::kDbiVlanId:
          valid = &(reinterpret_cast<val_vbr_if *>(val))->portmap.
                                valid[UPLL_IDX_VLAN_ID_PM];
          break;
        case uudst::vbridge_interface::kDbiTagged:
          valid = &(reinterpret_cast<val_vbr_if *>(val))->portmap.
                                valid[UPLL_IDX_TAGGED_PM];
          break;
        case uudst::vbridge_interface::kDbiVexName:
            valid = &(reinterpret_cast<val_drv_vbr_if_t *>(val))->
                                valid[PFCDRV_IDX_VEXT_NAME_VBRIF];
          break;
        case uudst::vbridge_interface::kDbiVexIfName:
            valid = &(reinterpret_cast<val_drv_vbr_if_t *>(val))->
                                valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF];
          break;
        case uudst::vbridge_interface::kDbiVexLinkName:
            valid = &(reinterpret_cast<val_drv_vbr_if_t *>(val))->
                                valid[PFCDRV_IDX_VLINK_NAME_VBRIF];
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
   * 	     if both the values are same, update the valid flag for
   * 	     corresponding attribute as invalid in the first record.
   *
   * @param[in/out]  val1   first record value instance.
   * @param[in]      val2   second record value instance.
   * @param[in]      audit  if true, CompareValidValue called from audit process.
   *
   **/
  bool CompareValidValue(void *&val1, void *val2, bool audit);

  upll_rc_t UpdateConfigStatus(ConfigKeyVal *req,
                               unc_keytype_operation_t op,
                               uint32_t driver_result,
                               ConfigKeyVal *upd_key,
                               DalDmlIntf   *dmi,
                               ConfigKeyVal *ctrlr_key = NULL);
  /**
   * @Brief  Validates the syntax of the specified key and value structure
   *         for KT_VBR_IF keytype
   *
   * @param[in]  req    This structure contains IpcReqRespHeader
   *                    (first 8 fields of input request structure).
   * @param[in]  ikey   ikey contains key and value structure.
   *
   * @retval  UPLL_RC_SUCCESS                Successful.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX         Syntax error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE   key_vbr_if is not available.
   * @retval  UPLL_RC_ERR_GENERIC            Generic failure.
   * @retval  UPLL_RC_ERR_INVALID_OPTION1    option1 is not valid.
   * @retval  UPLL_RC_ERR_INVALID_OPTION2    option2 is not valid.
   */

  upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *ikey);

   /**
     * @brief  Update config status for commit result and vote result.
     *
     * @param[in/out]  ckv_running  ConfigKeyVal instance.
     * @param[in]      cs_status    either UNC_CS_INVALID or UNC_CS_APPLIED.
     * @param[in]      phase        specify the phase (CREATE,DELETE or UPDATE)
     * @param[in]      dmi          Pointer to the DalDmlIntf(DB Interface)
     **/
  upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                    uuc::UpdateCtrlrPhase phase,
                                    ConfigKeyVal *&ckv_running,
                                    DalDmlIntf *dmi);

  /**
   * @Brief  Checks if the specified key type(KT_VBR_IF) and
   *         associated attributes are supported on the given controller,
   *         based on the valid flag
   *
   * @param[in]  req              This structure contains IpcReqRespHeader
   *                              (first 8 fields of input request structure).
   * @param[in]  ikey             ikey contains key and value structure.
   * @param[in]  ctrlr_name       Controller id associated with ikey.
   *
   * @retval  UPLL_RC_SUCCESS        Validation succeeded.
   * @retval  UPLL_RC_ERR_GENERIC    Validation failure.
   * @retval  UPLL_RC_ERR_INVALID_OPTION1   Option1 is not valid.
   * @retval  UPLL_RC_ERR_INVALID_OPTION2   Option2 is not valid.
   */
  upll_rc_t ValidateCapability(IpcReqRespHeader *req,
               ConfigKeyVal *ikey, const char *ctrlr_name);

  /**
   * @Brief  Validates the syntax for KT_VBR_IF keytype value structure.
   *
   * @param[in]  val_vbr_if  KT_VBR_IF value structure.
   * @param[in]  operation  Operation type.
   *
   * @retval  UPLL_RC_SUCCESS         validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
   */
  upll_rc_t  ValidateVbrIfValue(val_vbr_if *vbr_if_val,
                                unc_keytype_operation_t operation);

  /**
   * @Brief  Validates the syntax of the specified value structure
   *         for KT_VBR_IF keytype
   * @param[in]  val_vtn_neighbor  vtn neighbor value structure
   * @param[in]  operation   operation type.
   *
   * @retval  UPLL_RC_SUCCESS            Successful.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX     Syntax error.
   *
   */
  upll_rc_t ValidateVtnNeighborValue(val_vtn_neighbor *vtn_neighbor,
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
  upll_rc_t ValVbrIfAttributeSupportCheck(const uint8_t *attrs,
                                          ConfigKeyVal *ikey,
                                          unc_keytype_operation_t operation,
                                          upll_keytype_datatype_t dt_type);

  /**
   * @Brief  Checks if the specified key type and
   *         associated attributes are supported on the given controller,
   *         based on the valid flag.
   *
   * @param[in]  crtlr_name      Controller name.
   * @param[in]  ikey            Corresponding key and value structure.
   *
   * @retval  UPLL_RC_SUCCESS                     validation succeeded.
   * @retval  UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT  Instance count limit is exceeds.
   * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR  Attribute Not_Supported.
   * @retval  UPLL_RC_ERR_GENERIC                 Generic failure.
   */
  upll_rc_t ValVtnNeighborAttributeSupportCheck(const char *ctrlr_name,
      ConfigKeyVal *ikey);

   /**
   * @brief  Perform Semantic Check to check Different vbridges
   *          contain same switch-id and vlan-id
   *
   * @param[in]      ikey        ConfigKeyVal
   * @param[in]      upll_rc_t   UPLL_RC_ERR_CFG_SEMANTIC on error
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
  upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
      MoMgrTables tbl = MAINTBL);
  /* Rename */
  bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
       int &nattr, MoMgrTables tbl);
  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
                            ConfigKeyVal *ikey);

  /* @brief     To convert the value structure read from DB to
   *            VTNService during READ operations
   * @param[in/out] ikey      Pointer to the ConfigKeyVal Structure
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   *
   **/
  upll_rc_t AdaptValToVtnService(ConfigKeyVal *ikey, AdaptType adapt_type);
  /* @brief             To return Success if all the valid flags are
   *                      INVALID during update operation
   * @param[in]  val_vbrif       Value Structure
   *
   * @retval  true               All flags are INVALID.
   * @retval  false              One of the attribute is not INVALID.
   *
   **/
  bool IsAllAttrInvalid(val_vbr_if *val_vbrif) {
    bool attr_invalid = true;
    for (unsigned int loop = 0;
      loop < sizeof(val_vbrif->valid)/sizeof(uint8_t); ++loop) {
      if (UNC_VF_INVALID != val_vbrif->valid[loop]) {
        if (loop == UPLL_IDX_PM_VBRI) {
          for (unsigned int port_loop = 0;
          port_loop < sizeof(val_vbrif->portmap.valid)/sizeof(uint8_t);
          ++port_loop) {
            if (UNC_VF_INVALID != val_vbrif->portmap.valid[port_loop]) {
              attr_invalid = false;
              break;
            }
          }
        } else {
          attr_invalid = false;
        }
        if (!attr_invalid) break;
      }
    }
    return attr_invalid;
  }
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
  VbrIfMoMgr();
  virtual ~VbrIfMoMgr() {
    for (int i = 0; i < ntable; i++)
      if (table[i]) {
        delete table[i];
      }
    delete[] table;
  }

  /* @brief         Updates vbrif structure with vexternal information
   *                based on valid[PORTMAP] flag.
   *
   * @param[in/out] ikey     Pointer to the ConfigKeyVal Structure
   * @param[in]     datatype DB type.
   * @param[in]     dmi      Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   *
   **/
  upll_rc_t UpdateConfigVal(ConfigKeyVal *ikey,
                            upll_keytype_datatype_t datatype,
                            DalDmlIntf *dmi,
                            TcConfigMode config_mode,
                            string vtn_name);

  /* @brief         Updates vbrif portmap structure
   *                based on valid[PORTMAP] flag.
   *
   * @param[in]     db_key          Pointer to the ConfigKeyVal structure in db
   * @param[in]     datatype        DB type.
   * @param[in]     dmi             Pointer to the DalDmlIntf(DB Interface)
   * @param[in]     ikey            Pointer to input ConfigKeyVal
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   *
   **/
  upll_rc_t UpdatePortMap(ConfigKeyVal *db_key,
                            upll_keytype_datatype_t datatype,
                            DalDmlIntf *dmi,
                            ConfigKeyVal *ikey,
                            TcConfigMode config_mode,
                            string vtn_name);

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
    * @brief      Method to check if individual portions of a key are valid
    *
    * @param[in/out]  ikey                 pointer to ConfigKeyVal referring to a UNC resource
    * @param[in]      index                db index associated with the variable
    *
    * @retval         true                 input key is valid
    * @retval         false                input key is invalid.
    **/
  bool IsValidKey(void *tkey, uint64_t index, MoMgrTables tbl = MAINTBL);

  /*
   * @param[out] flag 
   * @parm[out] vexternal, vex_if ==> Not touched if ctrlr_id is ODC type
   */
  upll_rc_t GetVexternal(ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
     DalDmlIntf *dmi, uint8_t *vexternal, uint8_t *vex_if,
     InterfacePortMapInfo &flag, const char *ctrlr_id);
#if 0
  /**
   * @brief Returns success if member of Boundary vlink
   *
   * @param[in]       ck_vbrif         ConfigKeyVal of the vbrif
   * @param[in]       dt_type        Configuration type
   * @param[in/out]   ck_vlink       ConfigKeyVal of the vlink key formed
   * @param[in]       dmi            DB Connection
   * @param[out]      upll_rc_t      UPLL_RC_SUCCESS if member
   *                                 UPLL_RC_ERR_NO_SUCH_INSTANCE if not
   *                                 UPLL_RC_SUCCESS on success
   *
   */
  upll_rc_t CheckIfMemberOfBoundaryVlink(ConfigKeyVal *ck_vbrif,
                                 upll_keytype_datatype_t dt_type,
                                 ConfigKeyVal *&ck_vlink,
                                 DalDmlIntf *dmi);
#endif
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
  /* create mo has to handle vex creation */

  /**
   * @Brief  Validates the syntax for KT_VBR_IF Keytype key structure.
   *
   * @param[in]  key_vbr_if  KT_VBR_IF key structure.
   * @param[in]  operation operation type.
   *
   * @retval  UPLL_RC_SUCCESS         validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
   */
  upll_rc_t ValidateVbrifKey(key_vbr_if *vbr_if_key,
                           unc_keytype_operation_t operation = UNC_OP_INVALID);


  /* @brief         This is semantic check for KEY_VBR_IF key type
   *                in the update operation.
   *
   * @param[in/out] ikey     Pointer to the ConfigKeyVal Structure
   * @param[in]     datatype DB type.
   * @param[in]     dmi      Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   *
   **/


  upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                         DalDmlIntf *dmi);

  upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                       DalDmlIntf *dmi, const char *ctrlr_id);
  upll_rc_t updateVbrIf(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi);
  upll_rc_t ConverttoDriverPortMap(ConfigKeyVal *ck_port_map, DalDmlIntf *dmi);
/**
    * @brief      Method to get a configkeyval of a specified keytype from an input configkeyval
    *
    * @param[in/out]  okey                 pointer to output ConfigKeyVal
    * @param[in]      parent_key           pointer to the configkeyval from which the output
    *                                      configkey val is initialized.
    *
    * @retval         UPLL_RC_SUCCESS      Successfull completion.
    * @retval         UPLL_RC_ERR_GENERIC  Failure case.
    */
  upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
                      ConfigKeyVal *parent_key);


  upll_rc_t GetVbrIfValfromDB(ConfigKeyVal *ikey,
                              ConfigKeyVal *&ck_drv_vbr_if,
                              upll_keytype_datatype_t dt_type,
                              DalDmlIntf *dmi);

  upll_rc_t GetVbrIfFromVExternal(
      uint8_t *vtn_name,
      uint8_t *vext_name,
      ConfigKeyVal *&tmpckv,
      DalDmlIntf *dmi,
      controller_domain_t ctr_dom,
      upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING);

/**
    * @brief      Method to get a configkeyval of the parent keytype
    *
    * @param[in/out]  okey           pointer to parent ConfigKeyVal 
    * @param[in]      ikey           pointer to the child configkeyval from 
    *                                which the parent configkey val is obtained.
    * @param[in]      dmi            Pointer to the DalDmlIntf(DB Interface)
    *
    * @retval         UPLL_RC_SUCCESS      Successfull completion.
    * @retval         UPLL_RC_ERR_GENERIC  Failure case.
    **/
  upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
      ConfigKeyVal *parent_key);
  upll_rc_t PartialMergeValidate(unc_key_type_t key_type,
                                 const char *ctrlr_name,
                                 ConfigKeyVal *err_ckv,
                                 DalDmlIntf *dmi);

  /* @brief         Translate errors to correct VTN service specific error.
   *
   *                Convert vlink keytype error to corresponding vbrif keytype
   *                if the given vlink is part of vbrif portmap in PFC.
   *                If the input KT is KT_VLINK, the output KT is KT_VBR_IF on
   *                successful conversion.
   *
   * @param[inout] err_key    Contains error key information
   * @param[in]  dmi          Pointer to the DalDmlIntf(DB Interface)
   * @param[in]  datatype     Specifies the configuration
   *                          CANDIDATE(Txvote)/RUNNING(AuditVote)
   *
   * @retval  UPLL_RC_SUCCESS                 Translation done and successful.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE    Translation not required.
   * @retval  UPLL_RC_ERR_DB_ACCESS           DB Read/Write error.
   * @retval  UPLL_RC_ERR_GENERIC             Generic failure.
   **/
  upll_rc_t TranslateError(ConfigKeyVal *err_key,
                           DalDmlIntf *dmi,
                           upll_keytype_datatype_t datatype);
  upll_rc_t TxUpdateErrorHandler(ConfigKeyVal *req,
                                 ConfigKeyVal *ck_main,
                                 DalDmlIntf *dmi,
                                 upll_keytype_datatype_t dt_type,
                                 ConfigKeyVal **err_ckv,
                                 IpcResponse *ipc_resp);
  upll_rc_t CreateConvertVbridgeInterfaceFromVlink(ConfigKeyVal *vlink_ckv,
      ConfigKeyVal *vbr_pm_ckv, IpcReqRespHeader *req, DalDmlIntf *dmi);
  upll_rc_t DeleteConvertVbridgeInterfaceFromVlink(ConfigKeyVal *vlink_ckv,
      ConfigKeyVal *vbr_pm_ckv, IpcReqRespHeader *req, DalDmlIntf *dmi);
  upll_rc_t GetChildConvertConfigKey(ConfigKeyVal *&okey,
                                             ConfigKeyVal *parent_key);
  upll_rc_t ConvertVbrIf(DalDmlIntf *dmi, bool match_ctrlr_dom,
                         ConfigKeyVal *ikey, TcConfigMode config_mode,
                         string vtn_name, unc_keytype_operation_t op);
  upll_rc_t UpdateUvbrIfConfigStatus(ConfigKeyVal *ikey,
                                           unc_keytype_operation_t op,
                                           uint32_t driver_result,
                                           ConfigKeyVal *upd_key,
                                           DalDmlIntf *dmi);
  upll_rc_t UpdateConvVbrIfConfigStatus(ConfigKeyVal *ikey,
                                         unc_keytype_operation_t op,
                                         uint32_t driver_result,
                                         ConfigKeyVal *upd_key,
                                         DalDmlIntf *dmi);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif
