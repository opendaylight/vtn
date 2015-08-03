/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VBRIDGE_PORTMAP_MOMGR_H
#define UNC_UPLL_VBRIDGE_PORTMAP_MOMGR_H

#include <string>
#include <sstream>
#include <set>
#include <map>
#include <list>
#include "vtn_momgr.hh"
#include "vnode_child_momgr.hh"

#define LABEL_MAX_RANGE 0XFFFFFFFF

namespace unc {
namespace upll {
namespace kt_momgr {


class VbrPortMapMoMgr : public VnodeChildMoMgr {
 private:
  static unc_key_type_t vbr_portmap_child[];
  static BindInfo       vbr_portmap_bind_info[];
  static BindInfo       key_vbr_portmap_maintbl_bind_info[];
  static BindInfo vbid_bind_info[];
  static BindInfo gvtnid_bind_info[];
  static BindInfo vbid_rename_bind_info[];

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
    upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) return UPLL_RC_ERR_GENERIC;
  if (tbl == MAINTBL) {
    switch (indx) {
      case uudst::vbridge_portmap::kDbiOperStatus:
        valid = &(reinterpret_cast<val_db_vbr_portmap_st *>(val)->
            vbr_portmap_val_st.valid[
            UPLL_IDX_OPER_STATUS_VBRPMS]);
        break;
      case uudst::vbridge_portmap::kDbiDownCount:
        valid = NULL;
        break;
      case uudst::vbridge_portmap::kDbiCtrlrName:
        valid = &(reinterpret_cast<pfcdrv_val_vbr_portmap *>(val))->
          vbrpm.valid[UPLL_IDX_CONTROLLER_ID_VBRPM];
        break;
      case uudst::vbridge_portmap::kDbiDomainId:
        valid = &(reinterpret_cast<pfcdrv_val_vbr_portmap *>(val))->
          vbrpm.valid[UPLL_IDX_DOMAIN_ID_VBRPM];
        break;
      case uudst::vbridge_portmap::kDbiLogicalPortId:
        valid = &(reinterpret_cast<pfcdrv_val_vbr_portmap *>(val))->
          vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM];
        break;
      case uudst::vbridge_portmap::kDbiLabelType:
        valid = &(reinterpret_cast<pfcdrv_val_vbr_portmap *>(val))->
          vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM];
        break;
      case uudst::vbridge_portmap::kDbiLabel:
        valid = &(reinterpret_cast<pfcdrv_val_vbr_portmap *>(val))->
          vbrpm.valid[UPLL_IDX_LABEL_VBRPM];
        break;
      case uudst::vbridge_portmap::kDbiBdryRefCount:
        valid = &(reinterpret_cast<pfcdrv_val_vbr_portmap *>
            (val))->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT];
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
   * @Brief  Validates the syntax of the specified key and value structure,
   *         operation and datatype for UNC_KT_VBR_PORTMAP
   *
   * @param[in]  req    This structure contains IpcReqRespHeader
   *                    (first 8 fields of input request structure).
   * @param[in]  ikey   ikey contains key and value structure of
   * UNC_KT_VBR_PORTMAP.
   *
   * @retval  UPLL_RC_SUCCESS                Successful.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX         Syntax error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE   key_vbr_portmap is not available.
   * @retval  UPLL_RC_ERR_GENERIC            Generic failure.
   * @retval  UPLL_RC_ERR_INVALID_OPTION1    option1 is not valid.
   * @retval  UPLL_RC_ERR_INVALID_OPTION2    option2 is not valid.
   */
  upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *ikey);

   /**
   * @Brief  Checks if the specified key type and
   *         associated attributes are supported on the given controller,
   *         based on the valid flag.
   *
   * @param[in]  val_vbr_portmap val structure
   * @param[in]  attrs           Pointer to controller attribute.
   * @param[in]  operation       Operation name.
   *
   * @retval  UPLL_RC_SUCCESS                     validation succeeded.
   * @retval  UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT  Instance count limit is
   * exceeds.
   * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR  Attribute Not_Supported.
   * @retval  UPLL_RC_ERR_GENERIC                 Generic failure.
   */
  upll_rc_t ValVbrPortMapAttributeSupportCheck(val_vbr_portmap *vbrpm_val,
                                               const uint8_t *attrs,
                                               unc_keytype_operation_t
                                               operation);

 /**
   * @Brief  Checks if the specified key type(KT_VBR_PORTMAP) and
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
   * @Brief  Validates the syntax of KT_VBR_PORTMAP value structure.
   *
   * @param[in]  val_vbr_portmap  KT_VBR_PORTMAP value structure.
   * @param[in]  operation     Operation type.
   *
   * @retval  UPLL_RC_SUCCESS         validation succeeded.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
   */
  upll_rc_t  ValidateVbrPortMapValue(val_vbr_portmap *val_vbr_pm,
                                unc_keytype_operation_t operation);

  /**
   * @brief  Perform Semantic Check to check Different vbridge portmap's
   *         contain same switch-id and vlan-id on the same controller and
   *         domain
   *
   * @param[in]  ikey        ConfigKeyVal of VBR_PORTMAP.
   * @param[in]  upll_rc_t   UPLL_RC_ERR_CFG_SEMANTIC on error
   *                         UPLL_RC_SUCCESS on success
   **/
  upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                              DalDmlIntf *dmi,
                              IpcReqRespHeader *req = NULL);

    /* @brief             To return Success if all the valid flags are
     *                      INVALID during update operation
     * @param[in]  val_vbr_portmap    Value Structure
     *
     * @retval  true               All flags are INVALID.
     * @retval  false              One of the attribute is not INVALID.
     *
     **/
    bool IsAllAttrInvalid(val_vbr_portmap_t *val_vbrpm) {
      for (unsigned int loop = 0;
        loop < sizeof(val_vbrpm->valid)/sizeof(uint8_t); ++loop) {
        if (UNC_VF_INVALID != val_vbrpm->valid[loop])
          return false;
      }
      return true;
    }

  /**
   * @brief  Allocates the ConfigVal of VBR_PORTMAP val for the
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
  upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                         ConfigKeyVal *ikey);
  /* Rename */
  bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                            int &nattr, MoMgrTables tbl);

  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);

  /**
   * @brief     Perform validation on key type specific,
   *            before sending to driver
   *
   * @param[in]  ck_new                   Pointer to the ConfigKeyVal Structure
   * @param[in]  ck_old                   Pointer to the ConfigKeyVal Structure
   * @param[in]  op                       Operation name.
   * @param[in]  dt_type                  Specifies the configuration CANDIDATE/
   *                                      RUNNING
   * @param[in]  keytype                  Specifies the keytype
   * @param[in]  dmi                      Pointer to the DalDmlIntf
   *                                      (DB Interface)
   * @param[out] not_send_to_drv          Decides whether the configuration
   *                                      needs to be sent to controller or not
   * @param[in]  audit_update_phase       Specifies whether the phase is commit
   *                                      or audit
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

  upll_rc_t IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ckv,
      DalDmlIntf *dmi,
      IpcReqRespHeader *req);

  upll_rc_t ValidateVbrPmCreate(IpcReqRespHeader *req,
      ConfigKeyVal *ikey, DalDmlIntf *dmi, TcConfigMode config_mode,
      string vtn_name);

  upll_rc_t ValidateParentIsUnVbr(IpcReqRespHeader *req, ConfigKeyVal *ikey,
       ConfigKeyVal *pkey, DalDmlIntf *dmi);

  upll_rc_t IsUnwExists(DalDmlIntf *dmi, TcConfigMode config_mode);

  // functions related to conversion
  upll_rc_t ConvertVNode(ConfigKeyVal *ikey, DalDmlIntf *dmi,
      TcConfigMode config_mode, string vtn_name,
      IpcReqRespHeader *req);

  upll_rc_t CompareCtrlrDomainInReqWithDB(ConfigKeyVal *ikey,
      DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name,
      bool *same_ctrlr_domain, uint32_t *counter);

  upll_rc_t GetVbid(ConfigKeyVal *ikey, DalDmlIntf *dmi, uint32_t *vbid);
  upll_rc_t CheckUsedCountExceedsMaxCount(ConfigKeyVal *spd_ckv,
                                        TcConfigMode config_mode,
                                        DalDmlIntf *dmi);

  upll_rc_t GetSpineDomainId(ConfigKeyVal *ikey,
      ConfigKeyVal *&spine_dom_ckv,
      DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name);

  upll_rc_t GetUnwSpineDomain(ConfigKeyVal **spd_ckv, DalDmlIntf *dmi,
                              upll_keytype_datatype_t datatype);

  upll_rc_t IsVbrPmVlanIdTransparent(ConfigKeyVal *ckv,
    DalDmlIntf *dmi, IpcReqRespHeader *req);


  upll_rc_t IterateSpineDomainCkvAndFindOne(ConfigKeyVal *tmp,
                                         ConfigKeyVal *&spd_ckv,
                                         DalDmlIntf *dmi,
                                         TcConfigMode config_mode);

  upll_rc_t UpdateSpineDomainId(ConfigKeyVal *spd_ckv,
      DalDmlIntf *dmi);

  upll_rc_t GetUnwLabelCkv(ConfigKeyVal *spinedom_ckv,
    ConfigKeyVal *&label_ckv, TcConfigMode config_mode, DalDmlIntf *dmi);

  upll_rc_t GetSpineDomainIdBasedOnAvailableLabel(
      ConfigKeyVal *&unw_spine_dom_ckv, DalDmlIntf *dmi,
      TcConfigMode config_mode, string vtn_name);

  upll_rc_t CreateVtunnel(ConfigKeyVal *ikey, DalDmlIntf *dmi,
      TcConfigMode config_mode, string vtn_name, ConfigKeyVal *&vtun_ckv);

  upll_rc_t CreateVtunnelIf(ConfigKeyVal *vtun_ckv, ConfigKeyVal *ikey,
     uint8_t *un_vbr_name, ConfigKeyVal *&vtun_if_ckv, TcConfigMode config_mode,
     string vtn_name, DalDmlIntf *dmi);

  upll_rc_t CreateVlink(ConfigKeyVal *vbr_if_ckv, ConfigKeyVal *vtun_if_ckv,
      DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name,
      uint32_t gvtnid);

  upll_rc_t CreateVbr(ConfigKeyVal *ikey, IpcReqRespHeader *req,
      DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name, uint32_t vbid,
      ConfigKeyVal *&convert_vbr_ckv);

  upll_rc_t CreateVbrIf(ConfigKeyVal *convert_vbr_ckv,
      ConfigKeyVal *vtun_if_ckv, DalDmlIntf *dmi,
      TcConfigMode config_mode, string vtn_name,
      ConfigKeyVal *&vbr_if_ckv);

  upll_rc_t CreateConvertVnodeExceptVtunnel(
      ConfigKeyVal *ikey, IpcReqRespHeader *req, DalDmlIntf *dmi,
      TcConfigMode config_mode, string vtn_name, ConfigKeyVal *vtun_ckv,
      uint32_t vbid);

  upll_rc_t CreateVtunnelAndOtherVnode(ConfigKeyVal *ikey,
      IpcReqRespHeader *req, DalDmlIntf *dmi, TcConfigMode config_mode,
      string vtn_name);

  upll_rc_t DeleteVNode(ConfigKeyVal *ikey, IpcReqRespHeader *req,
     DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name);

  upll_rc_t DeleteConvertedVnodeAndVtunnel(ConfigKeyVal *ikey,
      IpcReqRespHeader *req, DalDmlIntf *dmi,
      TcConfigMode config_mode, string vtn_name);

  upll_rc_t DeleteConvertedVnodeExceptVtunnel(ConfigKeyVal *ikey,
      IpcReqRespHeader *req, DalDmlIntf *dmi,
      TcConfigMode config_mode, string vtn_name);

  upll_rc_t DeleteVtunnel(ConfigKeyVal *vtun_ckv, DalDmlIntf *dmi,
      TcConfigMode config_mode, string vtn_name);

  upll_rc_t DeleteVbr(ConfigKeyVal *ikey, IpcReqRespHeader *req,
      DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name);

  upll_rc_t DeleteVlink(ConfigKeyVal *ikey, DalDmlIntf *dmi,
      bool vbr_portmap_flag, TcConfigMode config_mode, string vtn_name);

  upll_rc_t DeleteVtunnelIf(ConfigKeyVal *vtun_ckv, DalDmlIntf *dmi,
      bool vbr_portmap_flag, TcConfigMode config_mode, string vtn_name);

  upll_rc_t DeleteVbrIf(ConfigKeyVal *ikey, DalDmlIntf *dmi,
      bool vbr_portmap_flag, TcConfigMode config_mode, string vtn_name);

  upll_rc_t DeleteVbrPortMap(ConfigKeyVal *ikey, TcConfigMode config_mode,
                             string vtn_name, DalDmlIntf *dmi,
                             ConfigKeyVal *dup_ckv = NULL);

  upll_rc_t GetVtunnelData(ConfigKeyVal *ikey, DalDmlIntf *dmi,
      ConfigKeyVal *&vtun_ckv);

  upll_rc_t SeparateBoundaryAndUserRecordInDB(
      ConfigKeyVal *db_ckv, val_vbr_portmap *ikey_val,
      DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name);

  upll_rc_t GeneratePortMapId(ConfigKeyVal *&db_ckv, DalDmlIntf *dmi);

  upll_rc_t HandleSpineDomainIdChange(
      DalDmlIntf *dmi, IpcReqRespHeader *req,
      ConfigKeyVal *new_spinedom_ckv,
      ConfigKeyVal *old_vtunnel_ckv);

  upll_rc_t DeleteVlinkAndVtnGatewayPort(
      ConfigKeyVal *old_vtun_ckv, DalDmlIntf *dmi, TcConfigMode config_mode,
      string vtn_name);

  upll_rc_t CreateVtunnelWithRefCount(
      ConfigKeyVal *old_vtun_ckv, ConfigKeyVal *&new_vtun_ckv,  DalDmlIntf *dmi,
      uint32_t gvtnid, ConfigKeyVal *new_spine_dom_ckv,
      TcConfigMode config_mode, string vtn_name);

  upll_rc_t CreateVlinkForSpineIdChange(ConfigKeyVal *new_vtun_ckv,
      ConfigKeyVal *old_vlink_ckv,
      DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name);

  upll_rc_t UpdateUsedCntInScratchTbl(ConfigKeyVal *spd_ckv,
    uint8_t *ikey_vtnid, unc_keytype_operation_t operation, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name);

  // UpdateUsedCntForGlobalMode() is invoked during commit/abort in GLOBAL mode
  // to update the used_label_count in spine_domain tbl.
  upll_rc_t UpdateUsedCntForGlobalMode(DalDmlIntf *dmi, bool is_abort);

  upll_rc_t UpdateRunningUsedCntFromScratchTbl(
    TcConfigMode config_mode, string vtn_name,
    DalDmlIntf *dmi);

    upll_rc_t IncrementUsedLabelCount(ConfigKeyVal *spd_ckv,
      uint8_t *vtn_id, DalDmlIntf *dmi, TcConfigMode config_mode,
      string vtn_name);

  upll_rc_t DecrementUsedLabelCount(ConfigKeyVal *vtun_ckv, uint8_t *vtn_id,
      DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name);

  upll_rc_t CheckForUnwExistance(DalDmlIntf *dmi);

  upll_rc_t GetValVbrPortMap(ConfigKeyVal *ikey, val_vbr_portmap **vbrpm);

  upll_rc_t DeleteVtnGatewayPortExistsForAUnwVbr(ConfigKeyVal *ikey,
    DalDmlIntf *dmi, ConfigKeyVal *vbr_ckv, TcConfigMode config_mode,
    string vtn_name);

  upll_rc_t DeleteVtnRenameAndCtrlrTblData(ConfigKeyVal *ikey,
    DalDmlIntf *dmi, ConfigKeyVal *&vbr_ckv, TcConfigMode config_mode,
    string vtn_name);

  upll_rc_t GetVbrPortMap(ConfigKeyVal *ikey, IpcReqRespHeader *req,
    uint32_t *vlanid, DalDmlIntf *dmi);

  upll_rc_t IsUnwSpdExistsInRunningDB(ConfigKeyVal *unw_sd_ckv,
                                                      DalDmlIntf *dmi);
  void FilterLabelRange(std::multimap<uint32_t, uint32_t> *label_range);
  upll_rc_t GetFreeGvtnId(ConfigKeyVal *ckv_gvtn,
          ConfigKeyVal *ck_del_gvtn, uint8_t *vtn_id, uint32_t &label,
          uint32_t *default_range, DalDmlIntf *dmi,
          TcConfigMode config_mode, std::string vtn_name);

  upll_rc_t GetUniqueLabelRange(ConfigKeyVal *ck_spine_id,
                                uint32_t **bucket_span);
  upll_rc_t CalculateRange(ConfigKeyVal *spd_ckv, uint32_t *default_range,
            DalDmlIntf *dmi, TcConfigMode config_mode, std::string vtn_name);
  upll_rc_t CheckIfLabelIsInDelTable(ConfigKeyVal *ck_del_gvtn,
                            uint8_t *vtn_name, DalDmlIntf *dmi,
                            uint32_t *gvtn_label, bool *is_label_used);
  upll_rc_t PopulateGvtnId(ConfigKeyVal *ckv_sd, ConfigKeyVal *&ck_gvtn,
            DalDmlIntf *dmi, TcConfigMode config_mode, std::string vtn_name);
  upll_rc_t PopulateGvtnId(ConfigKeyVal *ckv_gvtn,
                           val_convert_vtunnel* vtun_val, DalDmlIntf *dmi,
                           upll_keytype_datatype_t dt_type,
                           TcConfigMode config_mode, std::string vtn_name);
  upll_rc_t PopulateDelGvtnIdRecords(ConfigKeyVal *ckv_sd,
            ConfigKeyVal *&ck_del_gvtn, DalDmlIntf *dmi,
            TcConfigMode config_mode, std::string vtn_name);
  upll_rc_t PopulateDelVbidRecords(ConfigKeyVal *ckv_vbr_pm,
                      ConfigKeyVal *&ckv_del_vbr, DalDmlIntf *dmi,
                      TcConfigMode config_mode, std::string vtn_name);
  upll_rc_t CheckIfLabelIsInDelTable(ConfigKeyVal *ck_vbr_pm,
                      ConfigKeyVal *ckv_del_vbr, uint32_t *vbid_label,
                      DalDmlIntf *dmi, bool *is_label_used);
  upll_rc_t GetVbIdChildConfigKey(ConfigKeyVal *&okey,
                                  ConfigKeyVal *parent_key);
  upll_rc_t GetGVtnChildConfigKey(ConfigKeyVal *&okey,
                                  ConfigKeyVal *parent_key);
  upll_rc_t DupVbIdConfigKeyVal(ConfigKeyVal *&okey,
                            ConfigKeyVal *&req);
  upll_rc_t DupGvtnIdConfigKeyVal(ConfigKeyVal *&okey,
                            ConfigKeyVal *&req);

 public:
  /* Default constructor */
  VbrPortMapMoMgr();

  /* Default destructor */
  virtual ~VbrPortMapMoMgr() {
    for (int i = 0; i < ntable; i++)
      if (table[i]) {
        delete table[i];
      }
    delete[] table;
  }

  upll_rc_t CreateCandidateMo(IpcReqRespHeader *req,
      ConfigKeyVal *ikey,
      DalDmlIntf *dmi);

  upll_rc_t DeleteMo(IpcReqRespHeader *req,
      ConfigKeyVal *ikey,
      DalDmlIntf *dmi);

  upll_rc_t GetControllerDomainId(ConfigKeyVal *ikey,
                                  controller_domain_t *ctrlr_dom);


  upll_rc_t DeleteConvertVNodeChildren(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    TcConfigMode config_mode,
    string vtn_name,
    MoMgrTables tbl);

  upll_rc_t DeleteChildren(ConfigKeyVal *ikey,
                           ConfigKeyVal *pkey,
                           upll_keytype_datatype_t dt_type,
                           DalDmlIntf *dmi,
                           TcConfigMode config_mode,
                           string vtn_name,
                           MoMgrTables tbl = MAINTBL);

  upll_rc_t ReadSiblingMo(IpcReqRespHeader *req,
      ConfigKeyVal *key,
      bool begin,
      DalDmlIntf *dal);

  upll_rc_t ReadSiblingCount(IpcReqRespHeader *req,
      ConfigKeyVal* ikey,
      DalDmlIntf *dmi);

  /* @brief         Read the configuration either from RDBMS and/or
   *                from the controller
   *
   * @param[in]     req    Pointer to IpcResResHeader
   * @param[in/out] ikey   Pointer to the ConfigKeyVal Structure
   * @param[in]     dmi    Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
   *
   **/
  virtual upll_rc_t ReadMo(IpcReqRespHeader *req,
      ConfigKeyVal *ikey,
      DalDmlIntf *dmi);


  /**
   * @brief      Method used to perform update request for requested
   *             UNC_KT_VBR_PORTMAP keytype
   *
   * @param[in]  req     contains first 8 fields of input request structure
   * @param[in]  ikey    ConfigKeyVal obj pointer of UNC_VBR_PORTMAP
   * @param[in]  dmi     Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_SUCCESS            Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC        Failure case.
   */
  upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
      DalDmlIntf *dmi);

  /**
   ** @Brief  Validates the syntax of UNC_KT_VBR_PORTMAP key structure.
   **
   ** @param[in]  key_vbr_portmap  KT_VBR_PORTMAP key structure.
   ** @param[in]  operation     operation type.
   **
   ** @retval  UPLL_RC_SUCCESS         validation succeeded.
   ** @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
   **/
  upll_rc_t ValidateVbrPortMapKey(key_vbr_portmap_t *vbr_portmap_key,
      unc_keytype_operation_t operation = UNC_OP_INVALID);


  /**
   * @brief      Method to check if individual portions of a key are valid
   *
   * @param[in/out]  ikey   Pointer to UNC_KT_VBR_PORTMAP ConfigKeyVal obj.
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

  upll_rc_t IsReferenced(ConfigKeyVal *ikey,
      upll_keytype_datatype_t dt_type,
      DalDmlIntf *dmi);


  /**
   * @brief      Method to get a UNC_KT_VBR_PORTMAP ConfigKeyVal Pointer from
   *             the input ConfigKeyVal of specified KeyType
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
  /**
   * @brief     During Commit failure, convert driver error
   *            configuration based of the keytype and set to err_ckv.
   *            This function can be invoked only during COMMIT phase.
   *
   * @param[in]  ck_unc             It contains UNC specific names.
   * @param[in]  ck_driver          It contains Controller specific name.
   * @param[in]  dt_type            Specifies the configuration
   *                                CANDIDATE/RUNNING
   * @param[in]  dmi                Pointer to the DalDmlIntf
   *                                (DB Interface)
   * @param[in]  err_ckv            Pointer to the ConfigKeyVal Structure
   * @param[in]  ipc_resp           Pointer contains driver response.
   *
   * @retval  UPLL_RC_SUCCESS               Translation required.
   * @retval  UPLL_RC_ERR_GENERIC           Generic failure.
   * @retval  UPLL_RC_ERR_DB_ACCESS         DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  Translation not required.
   *
   */
  upll_rc_t TxUpdateErrorHandler(ConfigKeyVal *ckv_unc,
      ConfigKeyVal *ck_driver,
      DalDmlIntf *dmi,
      upll_keytype_datatype_t dt_type,
      ConfigKeyVal **err_ckv,
      IpcResponse *ipc_resp);
/**
   * @brief  Compares the valid value between two database records.
   * 	       if both the values are same, update the valid flag for
   * 	       corresponding attribute as invalid in the first record.
   *
   * @param[in/out]  val1   first record value instance.
   * @param[in]      val2   second record value instance.
   * @param[in]      audit  if true, CompareValidValue called from audit
   * process.
   *
   **/
  bool CompareValidValue(void *&val1, void *val2, bool audit);

  /**
   * @brief  Update config status based on commit result.
   *
   * @param[in/out]  ckv_running   ConfigKeyVal of VBR_PORTMAP.
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


  upll_rc_t TranslateVbrPortMapError(
      ConfigKeyVal **err_ckv, ConfigKeyVal *ckv_running,
      ConfigKeyVal *resp_ckv, DalDmlIntf *dmi,
      upll_keytype_datatype_t datatype);

  upll_rc_t BoundaryMapReq(IpcReqRespHeader *req,
                               ConfigKeyVal *ikey, ConfigKeyVal *db_vlink,
                               ConfigKeyVal *vlanmap_ckv,
                               ConfigKeyVal *uppl_bdry, DalDmlIntf *dmi);

  upll_rc_t BoundaryVbrPortmapCreate(ConfigKeyVal *vbr_pm_ckv,
                                                    IpcReqRespHeader *req,
                                                    DalDmlIntf *dmi);
  upll_rc_t BoundaryVbrPortmapDelete(ConfigKeyVal *db_vlink,
                                                   ConfigKeyVal *vbr_pm_ckv,
                                                    IpcReqRespHeader *req,
                                                    DalDmlIntf *dmi);

  upll_rc_t BoundaryVbrPortmapUpdate(ConfigKeyVal *ikey,
                                                    ConfigKeyVal *db_vlink,
                                                    ConfigKeyVal *vbr_pm_ckv,
                                                    IpcReqRespHeader *req,
                                                    DalDmlIntf *dmi);
upll_rc_t MergeValidate(unc_key_type_t keytype,
                                         const char *ctrlr_id,
                                         ConfigKeyVal *ikey,
                                         DalDmlIntf *dmi,
                                         upll_import_type import_type);

upll_rc_t PartialMergeValidate(unc_key_type_t keytype,
                                                const char *ctrlr_id,
                                                ConfigKeyVal *err_ckv,
                                                DalDmlIntf *dmi);
upll_rc_t ValidateVtnRename(ConfigKeyVal *org_vtn_ckv,
                                             ConfigKeyVal *rename_vtn_ckv,
                                             DalDmlIntf *dmi);
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

  upll_rc_t ClearScratchTbl(TcConfigMode config_mode, string vtn_name,
    DalDmlIntf *dmi, bool is_abort = false);


  upll_rc_t TxCopyCandidateToRunning(
      unc_key_type_t keytype,
      list<CtrlrCommitStatus*> *ctrlr_commit_status,
      DalDmlIntf *dmi, TcConfigMode config_mode,
      std::string vtn_name);
  upll_rc_t AllocVbid(ConfigKeyVal *ckv_vbr_pm, uint32_t &label,  // NOLINT
                    DalDmlIntf *dmi, TcConfigMode config_mode,
                    std::string config_vtn_name);
  upll_rc_t AllocVbid(uint8_t *vtn_name, uint32_t label,
              DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
              TcConfigMode config_mode, string config_vtn_name);
  upll_rc_t DeAllocVbid(ConfigKeyVal *ckv_vbr_pm, uint32_t label,
                        DalDmlIntf *dmi, TcConfigMode config_mode,
                        std::string config_vtn_name);
  upll_rc_t DeAllocVbid(ConfigKeyVal *ckv_vbr_pm, DalDmlIntf *dmi,
                      TcConfigMode config_mode, std::string config_vtn_name);
  upll_rc_t AllocGvtnId(ConfigKeyVal *&ckv_spd, uint8_t *vtn_id, uint32_t &label,  // NOLINT
            DalDmlIntf *dmi, TcConfigMode config_mode, std::string vtn_name);
  upll_rc_t AllocGvtnId(ConfigKeyVal *vtun_ckv, DalDmlIntf *dmi,
                        upll_keytype_datatype_t dt_type,
                        TcConfigMode config_mode, std::string vtn_name);

  upll_rc_t DeAllocGvtnId(ConfigKeyVal *spd_ckv, DalDmlIntf *dmi,
                          TcConfigMode config_mode, std::string vtn_name);
  upll_rc_t DeAllocGvtnId(ConfigKeyVal *vtun_ckv,
            DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
            TcConfigMode config_mode, std::string vtn_name);
  upll_rc_t TxCopyLabelTblFromCandidateToRunning(unc_keytype_operation_t op,
            DalDmlIntf* dmi, TcConfigMode config_mode, std::string vtn_name);
  static std::map<std::string, std::map<std::string, uint8_t> >
              filled_bucket_span_;

  upll_rc_t GetPortMap(ConfigKeyVal *ikey,
                       uint8_t &valid_pm, val_port_map *&pm,
                       uint8_t &valid_admin, uint8_t &admin_status) {
    UPLL_FUNC_TRACE;
    if (ikey == NULL) return UPLL_RC_ERR_GENERIC;
    if (pm == NULL) return UPLL_RC_SUCCESS;
    pfcdrv_val_vbr_portmap *drv_vbr_pm =
          reinterpret_cast< pfcdrv_val_vbr_portmap *> (GetVal(ikey));
    if (!drv_vbr_pm) {
      UPLL_LOG_DEBUG("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    // todo(vishnu) : after soundaram fix uncomment below code
//    if (drv_vbr_pm->valid[0] != UNC_VF_VALID) {
//      pm->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
//      valid_pm = UNC_VF_INVALID;
//    } else {
      valid_pm = drv_vbr_pm->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM];
      if (valid_pm == UNC_VF_VALID) {
        pm->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
        uuu::upll_strncpy(
              pm->logical_port_id, drv_vbr_pm->vbrpm.logical_port_id,
              kMaxLenLogicalPortId+1);
      } else {
        pm->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
        valid_pm = UNC_VF_INVALID;
      }
//    }
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t UpdateUVbrConfigStatusFromVbrPm(DalDmlIntf *dmi,
                      std::multimap<std::string, std::string> affected_vbr_map,
                      TcConfigMode config_mode, std::string vtn_name);
  upll_rc_t FindAffectedUvbrRecordsFromVbrPm(DalDmlIntf *dmi,
                                             upll_keytype_datatype_t dt_type,
                                             uint8_t *ctrlr_id,
                                             std::multimap<std::string,
                                              std::string> *affected_vbr_map,
                                             TcConfigMode config_mode,
                                             std::string vtn_name);
  upll_rc_t AuditCommitCtrlrStatus(unc_key_type_t keytype,
                                    CtrlrCommitStatus *ctrlr_commit_status,
                                    DalDmlIntf *dmi);
  upll_rc_t IsKeyInUse(upll_keytype_datatype_t dt_type,
                       const ConfigKeyVal *ckv, bool *in_use, DalDmlIntf *dmi);

  upll_rc_t UpdateVnodeVal(ConfigKeyVal *rename_info, DalDmlIntf *dmi,
                           upll_keytype_datatype_t data_type, bool &no_rename);
  upll_rc_t ResetVBrPortmapOperStatus(ConfigKeyVal *ckey, DalDmlIntf *dmi,
                                      uint8_t oper_status);
  upll_rc_t UpdateUnwSdLabelCountInVtnModeAbort(
              TcConfigMode config_mode, string vtn_name, DalDmlIntf *dmi);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif




