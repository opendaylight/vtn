/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_IPROUTE_MOMGR_H
#define UNC_UPLL_IPROUTE_MOMGR_H
#include <string>
#include "momgr_impl.hh"
#include "vnode_child_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

#define INVALID_PREFIX_LENGTH 33
#define INVALID_DST_IP_ADDR 0xffffffff
#define INVALID_NEXT_HOP_ADDR 0xffffffff

namespace uuds = unc::upll::dal::schema;
namespace uudst = unc::upll::dal::schema::table;

class IpRouteMoMgr : public VnodeChildMoMgr {
  private:
    static BindInfo ip_route_bind_info[];
    static BindInfo key_ip_route_maintbl_update_bind_info[];
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
          case uudst::static_ip_route::kDbiNwmName2:
            valid = &(reinterpret_cast<val_static_ip_route*>
                      (val))->valid[UPLL_IDX_NWM_NAME_SIR];
            break;
          case uudst::static_ip_route::kDbiMetric:
            valid = &(reinterpret_cast<val_static_ip_route*>
                      (val))->valid[UPLL_IDX_GROUP_METRIC_SIR];
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
     * @param[in]      audit  if true,CompareValidValue is called from
     *                        audit process.
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
     * @Brief  Validates the syntax of the specified key and value structure
     *         for KT_VRT_IPROUTE keytype
     *
     * @param[in]  req    This structure contains IpcReqRespHeader
     *                    (first 8 fields of input request structure).
     * @param[in]  ikey   ikey contains key and value structure.
     *
     * @retval  UPLL_RC_SUCCESS                Successful.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX         Syntax error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE   key_vlan_map is not available.
     * @retval  UPLL_RC_ERR_GENERIC            Generic failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1    option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2    option2 is not valid.
     *
     */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *ikey);

    /**
     * @Brief  Checks if the specified key type(KT_VRT_IPROUTE) and
     *         associated attributes are supported on the given controller,
     *         based on the valid flag
     *
     * @param[in]  req               This structure contains IpcReqRespHeader
     *                               (first 8 fields of input request structure)
     * @param[in]  ikey              ikey contains key and value structure.
     * @param[in]  ctrlr_name        Controller name associated with ikey.
     *
     * @retval   UPLL_RC_SUCCESS               Validation succeeded.
     * @retval   UPLL_RC_ERR_GENERIC           Validation failure.
     * @retval   UPLL_RC_ERR_INVALID_OPTION1   Option1 is not valid.
     * @retval   UPLL_RC_ERR_INVALID_OPTION2   Option2 is not valid.
     */
    upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                 const char *ctrlr_name);

    /**
     * @Brief  Validates the syntax for KT_VRT_IPROUTE Keytype key structure.
     *
     * @param[in]  key_static_ip_route  KT_VRT_IPROUTE key structure.
     * @param[in]  operation   Operation type.
     *
     * @retval  UPLL_RC_SUCCESS          validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX   validation failed.
     *
     */
    upll_rc_t ValidateIpRouteKey(key_static_ip_route *iproute_key,
                                 unc_keytype_operation_t operation);

    /**
     * @Brief  Validates the syntax for KT_VRT_IPROUTE keytype value structure.
     *
     * @param[in]  val_vrt_iproute  KT_VRT_IPROUTE value structure.
     * @param[in]  operation  Operation type.
     *
     * @retval  UPLL_RC_SUCCESS          validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX   validation failed.
     */
    upll_rc_t ValidateIpRouteValue(val_static_ip_route *iprout_val,
                                   uint32_t op);

    /**
     * @Brief  Checks if the specified key type and
     *         associated attributes are supported on the given controller,
     *         based on the valid flag.
     *
     * @param[in]  iproute_val     KT_VRT_IPROUTE value structure.
     * @param[in]  attrs           Pointer to controller attribute.
     * @param[in]  operation       Operation name.
     *
     * @retval  UPLL_RC_SUCCESS                      validation succeeded.
     * @retval  UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT   Instance count resource
     *                                               limit is exceeds.
     * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR   Attribute NOT_SUPPORTED.
     * @retval  UPLL_RC_ERR_GENERIC                  Generic failure.
     */
    upll_rc_t ValIpRouteAttributeSupportCheck(
        val_static_ip_route *iproute_val,
        const uint8_t *attrs,
        unc_keytype_operation_t operation);

    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);
    /**
     * @brief  Duplicates the input configkeyval including the key and val.
     * based on the tbl specified.
     *
     * @param[in]  okey   Output Configkeyval - allocated within the function
     * @param[in]  req    Input ConfigKeyVal to be duplicated.
     * @param[in]  tbl    specifies if the val structure belongs to the
     *                    main table/ controller table or rename table.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     **/
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                              MoMgrTables tbl = MAINTBL);
    /**
     * @brief  Allocates for the specified val in the given configuration in the
     *         specified table.
     *
     * @param[in]  ck_val   Reference pointer to configval structure allocated.
     * @param[in]  dt_type  specifies the configuration candidate/running/state
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
     *             input configkeyval
     *
     * @param[in/out]  okey                 pointer to output ConfigKeyVal
     * @param[in]      parent_key           pointer to the configkeyval from
     *                                      which the output configkey val is
     *                                      initialized.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     */
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);
    /**
     * @brief  Allocates for the specified val in the given configuration in the
     *         specified table.
     *
     * @param[in]  ck_val   Reference pointer to configval structure allocated.
     * @param[in]  dt_type  specifies the configuration candidate/running/state
     * @param[in]  tbl      specifies if the corresponding table is the  main
     *                      table / controller table or rename table.
     *
     * @retval     UPLL_RC_SUCCESS      Successfull completion.
     * @retval     UPLL_RC_ERR_GENERIC  Failure case.
     **/
    upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);
    /* RENAME */
    bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                              int &nattr, MoMgrTables tbl);
    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);
    upll_rc_t IsReferenced(ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
                           DalDmlIntf *dmi);
    upll_rc_t MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
                            ConfigKeyVal *ikey, DalDmlIntf *dmi);

    bool ResetDataForSibling(key_static_ip_route *key_ipr,
                   uudst::static_ip_route::kStaticIpRouteIndex index);

    static uint16_t kIpRouteNumChildKey;

  public:
    IpRouteMoMgr();
    virtual ~IpRouteMoMgr() {
      for (int i = 0; i < ntable; i++)
        if (table[i]) {
          delete table[i];
        }
      delete[] table;
    }

    /**
     * @brief      Method to check if individual portions of a key are valid
     *
     * @param[in/out]  ikey    pointer to ConfigKeyVal referring to a
     *                         UNC resource
     * @param[in]      index   db index associated with the variable
     *
     * @retval         true    input key is valid
     * @retval         false   input key is invalid.
     **/
    bool IsValidKey(void *tkey, uint64_t index, MoMgrTables tbl = MAINTBL);

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

  /* @brief         ValidateUpdateMo: Method to validate the valid flags 
   *                for update operation
   *
   * @param[in]     ikey   Pointer to the ConfigKeyVal Structure 
   * @param[in]     db_ckv Pointer to the ConfigKeyVal Structure
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX             Syntax error.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure. 
   * @Note: Overridden from base class MoMgrImpl
   **/
  upll_rc_t ValidateUpdateMo(ConfigKeyVal *ikey, ConfigKeyVal *db_ckv);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif
