/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VBR_MOMGR_H
#define UNC_UPLL_VBR_MOMGR_H

// `#include <iostream>
#include <string>
#include <set>
#include "momgr_impl.hh"
#include "vnode_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {


class VbrMoMgr : public VnodeMoMgr {
  private:
    static unc_key_type_t vbr_child[];
    static BindInfo vbr_bind_info[];
    static BindInfo vbr_rename_bind_info[];
    static BindInfo key_vbr_maintbl_bind_info[];
    static BindInfo key_vbr_renametbl_update_bind_info[];
    //  Added for Vbridge converttbl
    static BindInfo conv_vbr_bind_info[];
    static BindInfo key_conv_vbr_converttbl_bind_info[];
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
      UPLL_FUNC_TRACE;
      if (val == NULL) return UPLL_RC_ERR_GENERIC;
      if (tbl == MAINTBL) {
        switch (indx) {
          case uudst::vbridge::kDbiOperStatus:
            valid = &(reinterpret_cast<val_db_vbr_st *>(val))->
                                vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS];
            break;
          case uudst::vbridge::kDbiDownCount:
          case uudst::vbridge::kDbiUnknownCount:
            valid = NULL;
            break;
          case uudst::vbridge::kDbiCtrlrName:
            valid = &(reinterpret_cast<val_vbr *>(val))->
                                valid[UPLL_IDX_CONTROLLER_ID_VBR];
            break;
          case uudst::vbridge::kDbiDomainId:
            valid = &(reinterpret_cast<val_vbr *>(val))->
                                valid[UPLL_IDX_DOMAIN_ID_VBR];
            break;
          case uudst::vbridge::kDbiVbrDesc:
            valid = &(reinterpret_cast<val_vbr *>(val))->
                                valid[UPLL_IDX_DESC_VBR];
            break;
          case uudst::vbridge::kDbiHostAddr:
            valid = &(reinterpret_cast<val_vbr *>(val))->
                                valid[UPLL_IDX_HOST_ADDR_VBR];
            break;
          case uudst::vbridge::kDbiHostAddrMask:
            valid = &(reinterpret_cast<val_vbr *>(val))->
                                valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR];
            break;
          default:
            return UPLL_RC_ERR_GENERIC;
        }
      } else if (tbl == RENAMETBL) {
        switch (indx) {
          case uudst::vnode_rename::kDbiCtrlrVtnName:
            valid = &(reinterpret_cast<val_rename_vnode*>
                    (val))->valid[UPLL_CTRLR_VTN_NAME_VALID];
            break;
          case  uudst::vnode_rename::kDbiCtrlrVnodeName:
            valid = &(reinterpret_cast<val_rename_vnode*>
                    (val))->valid[UPLL_CTRLR_VNODE_NAME_VALID];
            break;
          default:
            break;
        }
        return UPLL_RC_SUCCESS;
      } else if (tbl == CONVERTTBL) {
        switch (indx) {
          case uudst::convert_vbridge::kDbiOperStatus:
            valid = &(reinterpret_cast<val_db_vbr_st *>(val))->
                                vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS];
            break;
          case uudst::convert_vbridge::kDbiDownCount:
          case uudst::convert_vbridge::kDbiUnknownCount:
            valid = NULL;
            break;
          case uudst::convert_vbridge::kDbiLabel:
            valid = &(reinterpret_cast<val_convert_vbr_t *>(val))->
                                valid[UPLL_IDX_LABEL_CONV_VBR];
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
     * @param[in]      dmi          Pointer to the DalDmlIntf(DB Interface)
     *
     **/
    upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                      uuc::UpdateCtrlrPhase phase,
                                      ConfigKeyVal *&ckv_running,
                                      DalDmlIntf *dmi);

    /**
     * @Brief  Checks if the specified key type(KT_VBR) and
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
     **/
    upll_rc_t ValidateCapability(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 const char *ctrlr_id);

    /**
     * @Brief  Validates the syntax of the specified key and value structure
     *         for KT_VBR keytype
     *
     * @param[in]  req    This structure contains IpcReqRespHeader
     *                    (first 8 fields of input request structure).
     * @param[in]  ikey   ikey contains key and value structure.
     *
     * @retval  UPLL_RC_SUCCESS               Successful.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX        Syntax error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  key_vbr is not available.
     * @retval  UPLL_RC_ERR_GENERIC           Generic failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
     */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req,
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
     * @Brief  Checks if the specified key type and
     *         associated attributes are supported on the given controller,
     *         based on the valid flag.
     *
     * @param[in]  vbr_val         KT_VBRIDGE value structure.
     * @param[in]  attrs           Pointer to controller attribute.
     * @param[in]  operation       Operation name.
     *
     * @retval  UPLL_RC_SUCCESS                     validation succeeded.
     * @retval  UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT  Instance count limit is exceedes.
     * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR  Attribute NOT_SUPPORTED.
     * @retval  UPLL_RC_ERR_GENERIC                 Generic failure.
     */
    upll_rc_t ValVbrAttributeSupportCheck(val_vbr_t *vbr_val,
                                          const uint8_t *attrs,
                                          unc_keytype_operation_t operation);

    /**
     * @Brief  Validates the syntax for KT_VBR keytype Rename structure.
     *
     * @param[in]  val_rename_vbr  KT_VBR Rename structure.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     *
     */
    upll_rc_t ValidateVbrRenameValue(val_rename_vbr *vbr_rename);

    /**
     * @Brief  Validates the syntax for KT_VBR keytype val_ping structure.
     *
     * @param[in]  val_ping  KT_VBR val_ping structure.
     *
     * @retval  UPLL_RC_SUCCESS          validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX   validation failed.
     */
    upll_rc_t ValidateVbrPingValue(val_ping *ping_val);

    /*
     * @Brief  Validates the syntax for KT_VBR keytype value structure.
     *
     * @param[in]  val_vbr  KT_VBR value structure.
     * @param[in]  operation  Operation type.
     *
     * @retval  UPLL_RC_SUCCESS               validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX        validation failed.
     */
    upll_rc_t ValidateVbrValue(val_vbr_t *vbr_val,
                               uint32_t operation);

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
     **/
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey,
                              ConfigKeyVal *&req,
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
    upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                 ConfigKeyVal *parent_key);
    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi,
                                      controller_domain *ctrlr_dom);
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                               upll_keytype_datatype_t dt_type,
                               DalDmlIntf *dmi,
                               uint8_t *ctrlr_id);

    /* Pure virtual from VnodeMoMgrImpl */
    upll_rc_t GetVnodeName(ConfigKeyVal *ikey,
                           uint8_t *&vtn_name,
                           uint8_t *&vnode_name);
    /* Rename */
    upll_rc_t GetRenameInfo(ConfigKeyVal *ikey,
                            ConfigKeyVal *okey,
                            ConfigKeyVal *&rename_info,
                            DalDmlIntf *dmi,
                            const char *ctrlr_id,
                            bool &renamed);

    upll_rc_t IsRenamed(ConfigKeyVal *ikey,
                        upll_keytype_datatype_t dt_type,
                        DalDmlIntf *dmi,
                        uint8_t &rename);

    bool GetRenameKeyBindInfo(unc_key_type_t key_type,
                              BindInfo *&binfo,
                              int &nattr,
                              MoMgrTables tbl);
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

  public:
    VbrMoMgr();
    virtual ~VbrMoMgr() {
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
    bool IsValidKey(void *key,
                    uint64_t index, MoMgrTables tbl = MAINTBL);
    /* @brief         This method invoke when the VTN merge hapeening between
     *                Running and DT import. This will checks the vnode name
     *                unique or not and semantic checks like IP Address, Mac
     *                Address and network host address.
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

    upll_rc_t MergeValidate(unc_key_type_t keytype,
                            const char *ctrlr_id,
                            ConfigKeyVal *conflict_ckv,
                            DalDmlIntf *dmi,
                            upll_import_type import_type);
    /*
     * @Brief  Validates the syntax for KT_VBR Keytype key structure.
     *
     * @param[in]  key_vbr  KT_VBR key structure.
     * @param[in]  operation operation type.
     *
     * @retval  UPLL_RC_SUCCESS         validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
     *
     */
    upll_rc_t ValidateVbrKey(key_vbr_t *vbr_key,
                         unc_keytype_operation_t operation = UNC_OP_INVALID);

    upll_rc_t ValidateVbrRead(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi);
    /**
     * @brief     Method used in Delete opertaion. Its semantic checks
     *            for the VBR key.
     *
     * @param[in]  ikey                        key and value structure.
     * @param[in]  dt_type                     key  type.
     * @param[in]  dmi                         Pointer to DalDmlIntf Class.
     *
     * @retval     UPLL_RC_SUCCESS             Successfull completion.
     * @retval     UPLL_RC_ERR_GENERIC         Failure case.
     * @retval     UPLL_RC_ERR_CFG_SEMANTIC    Failue dueto Semantic.
     */

    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);

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
     *
     * @retval UPLL_RC_SUCCESS      Successful
     * @retval UPLL_RC_ERR_GENERIC  failed to update the VbrIf
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

    upll_rc_t GetUnifiedVbridgeConfigKey(ConfigKeyVal *&vbr_ckv,
                                       ConfigKeyVal *parent_key);

    upll_rc_t GetChildConvertConfigKey(ConfigKeyVal *&okey,
                                       ConfigKeyVal *parent_key);

    upll_rc_t GetControllerDomainId(ConfigKeyVal *ikey,
                                    controller_domain_t *ctrlr_dom);

    upll_rc_t DeleteConvertVbrMo(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 TcConfigMode config_mode,
                                 string vtn_name,
                                 DalDmlIntf *dmi);

    upll_rc_t ConvertVbr(DalDmlIntf *dmi,
                         IpcReqRespHeader *req,
                         ConfigKeyVal *ikey,
                         TcConfigMode config_mode,
                         string vtn_name,
                         unc_keytype_operation_t op);

    upll_rc_t AdaptValToDriver(ConfigKeyVal *&ck_vbr,
                               ConfigKeyVal *ck_conv_vbr,
                               MoMgrTables tbl_indx,
                               bool &is_unified);

    upll_rc_t TxUpdateController(unc_key_type_t keytype,
                                 uint32_t session_id,
                                 uint32_t config_id,
                                 uuc::UpdateCtrlrPhase phase,
                                 set<string> *affected_ctrlr_set,
                                 DalDmlIntf *dmi,
                                 ConfigKeyVal **err_ckv,
                                 TxUpdateUtil *tx_util,
                                 TcConfigMode config_mode,
                                 std::string vtn_name);
    upll_rc_t AuditUpdateController(unc_key_type_t keytype,
                                    const char *ctrlr_id,
                                    uint32_t session_id,
                                    uint32_t config_id,
                                    uuc::UpdateCtrlrPhase phase,
                                    DalDmlIntf *dmi,
                                    ConfigKeyVal **err_ckv,
                                    KTxCtrlrAffectedState *ctrlr_affected);

    upll_rc_t TranslateVbrPortmapToVbrErr(ConfigKeyVal *ikey,
                                     ConfigKeyVal **err_ckv,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi);

    upll_rc_t ReadExpandMo(IpcReqRespHeader *header,
                           ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);

    upll_rc_t ReadConvertVbridge(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi,
                                 ConfigKeyVal *con_vbr,
                                 ConfigKeyVal *vbr_pm,
                                 ConfigKeyVal *con_vbrif,
                                 ConfigKeyVal *con_vlink);

    upll_rc_t FrameVbrExpand(ConfigKeyVal *ikey,
                             ConfigVal *vbr_ex,
                             IpcReqRespHeader *req,
                             DalDmlIntf *dmi);

    upll_rc_t FrameVbrPmExpand(ConfigKeyVal *ikey,
                               ConfigVal *val_ex);

    upll_rc_t FrameVbrIfExpandUsingConvVlink(ConfigKeyVal *ikey,
                                             ConfigVal *tmp_vbrifx);

    upll_rc_t FrameVbrIfExpandUsingVlink(ConfigKeyVal *vbrif_ckv,
                                         ConfigKeyVal *vlink_ckv,
                                         ConfigVal *vbrif_ex,
                                         bool *uvb_vnode1);

    upll_rc_t GetVlinkMaintblFromConvVbrIf(ConfigKeyVal *vbrif_ckv,
                                           ConfigKeyVal *&vlnk_ckv,
                                           IpcReqRespHeader *req,
                                           DalDmlIntf *dmi,
                                           bool *uvb_vnode1);

    upll_rc_t FrameVtunnelExpand(ConfigKeyVal *ikey,
                                 ConfigVal *tmp_vtunnx);

    upll_rc_t FrameVtunIfExpand(ConfigKeyVal *ikey,
                                ConfigVal *vtunif_ex);

    upll_rc_t FrameVlinkExpand(ConfigKeyVal *ikey,
                               ConfigVal *tmp_vlnkx);

    upll_rc_t HandleVbrIfRead(ConfigKeyVal *ikey,
                              IpcReqRespHeader *req,
                              DalDmlIntf *dmi,
                              ConfigKeyVal *con_vbrif,
                              ConfigKeyVal *con_vlink,
                              ConfigVal *vbr_ex);

    upll_rc_t HandleVtunnelRead(ConfigKeyVal *vlink_ckv,
                                ConfigVal *vtun_ex);

    upll_rc_t HandleVtunnelIfRead(const std::string &vtun_name,
                                  ConfigKeyVal *vlink_ckv,
                                  ConfigVal *vtun_ex);

    upll_rc_t HandleVlinkRead(ConfigKeyVal *vlink_ckv,
                              ConfigVal *vlink_ex);

    bool CompareVbrIf(ConfigKeyVal *ckv_cvbr,
                      ConfigKeyVal *ckv_vbrif);

    bool CompareVlink(ConfigKeyVal *ckv_vbrif,
                      ConfigKeyVal *ckv_vlink);

    upll_rc_t HandleVbrPortmapRead(ConfigKeyVal *ikey,
                                   ConfigKeyVal *vbr_pm,
                                   ConfigVal *vbr_ex);

    bool CompareVbrPm(ConfigKeyVal *ckv_cvbr,
                      ConfigKeyVal *ckv_vbrpm);

    upll_rc_t ConvertVbrToUvbr(ConfigKeyVal *cvbr_ckv,
                               ConfigKeyVal *&uvbr_ckv);

    /**
     * @Brief Validates Same host address present in another vnode
     *        during VTN stiching.
     *
     * @param[in] org_vtn_ckv       Orginal VTN ConfigKeyVal.
     * @param[in] rename_vtn_ckv    Rename VTN ConfigKeyVal.
     * @param[in] dmi               Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval UPLL_RC_SUCCESS            validation succeeded.
     * @retval UPLL_RC_ERR_MERGE_CONFLICT Same host address exist.
     *
     */
    upll_rc_t ValidateVtnRename(ConfigKeyVal *org_vtn_ckv,
                                ConfigKeyVal *rename_vtn_ckv, DalDmlIntf *dmi);

    upll_rc_t ValidateConvertVbrMessage(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi);

    upll_rc_t CreateConvertVbrMo(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 TcConfigMode config_mode,
                                 string vtn_name,
                                 DalDmlIntf *dmi);
#if 0
    upll_rc_t IsHostAddrAndPrefixLenInUse(ConfigKeyVal *ckv, DalDmlIntf *dmi,
                                          IpcReqRespHeader *req);
#endif

    upll_rc_t CreateImportConvertVbridge(ConfigKeyVal* unc_vbr_ckv,
                                         IpcReqRespHeader *req,
                                         DalDmlIntf *dmi,
                                         upll_import_type import_type,
                                         const char *ctrlr_id);

    upll_rc_t IsVbIdInUse(ConfigKeyVal *ikey,
                          DalDmlIntf *dmi,
                          upll_keytype_datatype_t dt_type);

    upll_rc_t CheckUnifiedVbridgeTransparency(ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi,
                                              upll_keytype_datatype_t datatype);
    upll_rc_t MergeValidateConvertVbridge(ConfigKeyVal *uni_vbr_ckv,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi,
                                          const char *ctrlr_id,
                                          upll_import_type import_type);
    upll_rc_t MergeValidateVbid(ConfigKeyVal* ikey,
                                upll_keytype_datatype_t dt_type,
                                DalDmlIntf *dmi, const char *ctrlr_id,
                                upll_import_type import_type);
    upll_rc_t MergeValidateTransparentvBridge(ConfigKeyVal* ikey,
                                              upll_keytype_datatype_t dt_type,
                                              DalDmlIntf *dmi,
                                              const char *ctrlr_id,
                                              upll_import_type import_type);
    upll_rc_t PartialMergeValidate(unc_key_type_t keytype,
                                   const char *ctrlr_id, ConfigKeyVal *err_ckv,
                                   DalDmlIntf *dmi);
    upll_rc_t MergeConvertVbridgeImportToCandidate(const char *ctrlr_name,
                                DalDmlIntf *dmi, upll_import_type import_type);
    upll_rc_t UpdateUvbrConfigStatus(ConfigKeyVal *vbr_key,
                                     unc_keytype_operation_t op,
                                     uint32_t driver_result,
                                     ConfigKeyVal *upd_key,
                                     DalDmlIntf *dmi);
    upll_rc_t UnifiedVbrVnodeChecks(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t datatype,
                                    const char *ctrlr_id,
                                    DalDmlIntf *dmi);
    upll_rc_t UpdateConvVbrConfigStatus(ConfigKeyVal *vbr_key,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi);
    upll_rc_t SetValidAudit(ConfigKeyVal *&ikey);
    upll_rc_t UpdateUVbrConfigStatusAuditVote(ConfigKeyVal *ckv_drv_rslt,
                                              DalDmlIntf *dmi);
    upll_rc_t UpdateUvbrConfigStatusAuditUpdate(uuc::UpdateCtrlrPhase phase,
                               ConfigKeyVal *ikey, DalDmlIntf *dmi);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif
