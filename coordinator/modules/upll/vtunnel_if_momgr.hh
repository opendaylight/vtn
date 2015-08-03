/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VTUNNEL_IF_MOMGR_H
#define UNC_UPLL_VTUNNEL_IF_MOMGR_H

#include <string>

#include <set>
#include "momgr_impl.hh"
#include "vnode_child_momgr.hh"
#include "unc/vnpdriver_ipc_enum.h"

namespace unc {
namespace upll {
namespace kt_momgr {

enum VtunnelIfMoMgrTables {
  VTUNNELIFTBL = 0,
  NVTUNNELIFTABLES
};


class VtunnelIfMoMgr : public VnodeChildMoMgr {
 private:
  static BindInfo       vtunnel_if_bind_info[];
  static BindInfo       conv_vtunnel_if_bind_info[];

  /* @brief      Returns admin and portmap information if portmap is
   *             valid. Else returns NULL for portmap
   *
   * @param[in]   ikey     Pointer to ConfigKeyVal
   * @param[out]  valid_pm portmap is valid
   * @param[out]  pm       pointer to portmap informtation if valid_pm
   * @param[out]  valid_admin admin_status valid value
   * @param[out]  admin_status  value of admin_status
   *
   * @retval  UPLL_RC_SUCCESS      Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
   *
   **/
  virtual upll_rc_t GetPortMap(ConfigKeyVal *ikey,
                               uint8_t &valid_pm,
                               val_port_map_t *&pm,
                               uint8_t &valid_admin,
                               uint8_t &admin_status) {
     UPLL_FUNC_TRACE;
     if (ikey == NULL) return UPLL_RC_ERR_GENERIC;
     val_vtunnel_if *ifval = reinterpret_cast<val_vtunnel_if *>
                                                (GetVal(ikey));
     if (!ifval) {
       UPLL_LOG_DEBUG("Invalid param");
       return UPLL_RC_ERR_GENERIC;
     }
     valid_pm = ifval->valid[UPLL_IDX_PORT_MAP_VTNL_IF];
     if (valid_pm == UNC_VF_VALID)
       pm = &ifval->portmap;
     else
       pm = NULL;
     valid_admin = ifval->valid[UPLL_IDX_ADMIN_ST_VTNL_IF];
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
           upll_keytype_datatype_t dt_type, MoMgrTables tbl);

  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey) {
    UPLL_LOG_INFO("Not supported for this keytype. Returning Generic Error");
    return UPLL_RC_ERR_GENERIC;
  }
  bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                                    int &nattr,
                                    MoMgrTables tbl) {
    return true;
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
   * @param[in]      audit  if true, CompareValidValue called from audit process..
   *
   **/
  bool CompareValidValue(void *&val1, void *val2, bool copy_to_running);

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

  /* @brief     To convert the value structure read from DB to
   *            VTNService during READ operations
   * @param[in/out] ikey      Pointer to the ConfigKeyVal Structure
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   *
   **/
  upll_rc_t AdaptValToVtnService(ConfigKeyVal *ikey, AdaptType adapt_type);

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

#if 0
  upll_rc_t IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ckv,
                                        DalDmlIntf *dmi,
                                        IpcReqRespHeader *req);
#endif

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

  upll_rc_t UpdateConfigStatus(ConfigKeyVal *vtunnelif_keyval,
                               unc_keytype_operation_t op,
                               uint32_t result,
                               ConfigKeyVal *upd_key,
                               DalDmlIntf *dmi,
                               ConfigKeyVal *ctrlr_key = NULL);

  /**
   * @Brief Validates the syntax of the specified key and value structure
   *        for KT_VTUNNEL_IF keytype
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
   * @Brief Validates the syntax for KT_VTUNNEL_IF keytype key structure.
   *
   * @param[in] key_vtunnel_if  KT_VTUNNEL_IF key structure.
   *
   * @retval UPLL_RC_SUCCESS        validation succeeded.
   * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
   */
  upll_rc_t ValidateVTunnelIfKey(key_vtunnel_if_t *key_vtunnel_if,
                                 uint32_t operation);

  /**
   * @Brief Validates the syntax for KT_VTUNNEL_IF keytype value structure.
   *
   * @param[in] val_vlink KT_VTUNNEL_IF value structure.
   *
   * @retval UPLL_RC_ERR_GENERIC    Generic failure.
   * @retval UPLL_RC_SUCCESS        validation succeeded.
   * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
   */
  upll_rc_t ValidateVTunnelIfValue(val_vtunnel_if_t *val_vtunnel_if,
      uint32_t operation = UNC_OP_INVALID);

  /**
   * @Brief Validates the syntax for KT_VTUNNEL_IF keytype value structure.
   *
   * @param[in] val_vtn_neighbor KT_VTUNNEL_IF value structure.
   *
   * @retval UPLL_RC_ERR_GENERIC    Generic failure.
   * @retval UPLL_RC_SUCCESS        validation succeeded.
   * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
   */
  upll_rc_t ValidateVtnNeighborValue(val_vtn_neighbor_t *val_vtn_neighbor);

  /**
   * @Brief Checks if the specified key type(KT_VTUNNEL_IF) and
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
   * @param[in] val_vtunnel_if        Value Structure.
   * @param[in] attr                  pointer to controller attribute
   * @param[in] operation             Operation Name
   *
   * @retval UPLL_RC_SUCCESS                    validation succeeded.
   * @retval UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR Attribute NOT_SUPPORTED.
   * @retval UPLL_RC_ERR_GENERIC                Generic failure.
   */
  upll_rc_t ValVTunnelIfAttributeSupportCheck(
      val_vtunnel_if_t *val_vtunnel_if,
  const uint8_t* attrs, unc_keytype_operation_t operation);

  /* @brief         This is semantic check for KEY_VTUNNEL_IF key type
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
  /**
   * @Brief   Assign auto generated name to vtunnel interface or
   *          Delete the vtunnnel interface from database.
   *
   * @param[in]  ikey             ikey contains key and value structure.
   * @param[in]  unified_vbr_name Unified vbridge name
   * @param[in]  op               operation type
   * @param[in]  config_mode      Get Configuration mode
   * @param[in]  vtn_name         vtn name
   * @param[in]  dmi              Pointer to the DalDmlIntf(DB Interface) 

   * @retval  UPLL_RC_SUCCESS                    Entries updated to database.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   * @retval  UPLL_RC_ERR_BAD_REQUEST            Input/Request invalid.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
   */
  upll_rc_t ConvertVtunnelIf(ConfigKeyVal *ikey,
                             unc_keytype_operation_t op,
                             TcConfigMode config_mode,
                             string vtn_name,
                             DalDmlIntf *dmi);

 public:
  VtunnelIfMoMgr();
  virtual ~VtunnelIfMoMgr() {
    for (int i = VTUNNELIFTBL; i < NVTUNNELIFTABLES; i++)
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

  /* @brief         Updates vtunnelif structure
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
                            DalDmlIntf *dmi);
  upll_rc_t GetChildConvertConfigKey(ConfigKeyVal *&okey,
                                             ConfigKeyVal *parent_key);
  upll_rc_t DeleteVtunnelIf(ConfigKeyVal *ikey, TcConfigMode config_mode,
                            std::string vtn_name, DalDmlIntf *dmi);
};
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif
