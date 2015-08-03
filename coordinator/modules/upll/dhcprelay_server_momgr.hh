/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_DHCPRELAY_SERVER_MOMGR_H
#define UNC_UPLL_DHCPRELAY_SERVER_MOMGR_H

#include <string>
#include "momgr_impl.hh"
#include "vnode_child_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {


class DhcpRelayServerMoMgr : public VnodeChildMoMgr {
  private:
    static BindInfo dhcprelay_server_bind_info[];
    static BindInfo dhcprealy_server_maintbl_key_update_bind_info[];
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
      return UPLL_RC_ERR_GENERIC;
    }
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
     *         for KT_DHCPRELAY_SERVER keytype
     *
     * @param[in]  req    This structure contains IpcReqRespHeader
     *                    (first 8 fields of input request structure).
     * @param[in]  ikey   ikey contains key and value structure.
     *
     * @retval  UPLL_RC_SUCCESS                Successful.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX         Syntax error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE   key_dhcp_relay_server
     *                                         is not available.
     * @retval  UPLL_RC_ERR_GENERIC            Generic failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1    option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2    option2 is not valid.
     */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *ikey);

    /**
     * @Brief  Validates the syntax for KT_DHCP_RELAY_SERVER
     * Keytype key structure.
     *
     * @param[in]  key_dhcp_relay_server  KT_DHCP_RELAY_SERVER key structure.
     * @param[in]  Operation             operation type.
     *
     * @retval  UPLL_RC_SUCCESS          validation succeeded.
     * @retval  UPLL_RC_ERR_CFG_SYNTAX   validation failed.
     *
     */
    upll_rc_t ValidateDhcpRelayKey(key_dhcp_relay_server *dhcp_relay_key,
                                   unc_keytype_operation_t op);

    /**
     * @brief  Compares the valid value between two database records.
     * 	     if both the values are same, update the valid flag for
     * 	     corresponding attribute as invalid in the first record.
     *
     * @param[in/out]  val1   first record value instance.
     * @param[in]      val2   second record value instance.
     * @param[in]      audit  if true, CompareValidValue called from
     *                        audit process.
     *
     **/
    bool CompareValidValue(void *&val1, void *val2, bool audit) {
      return true;
    }

    /**
     * @Brief  Checks if the specified key type(KT_DHCPRELAY_SERVER) and
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
     **/
    upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                 const char *ctrlr_name);

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
     * @brief      Method to get a configkeyval of a specified keytype
     *             from an input configkeyval
     *
     * @param[in/out]  okey                 pointer to output ConfigKeyVal
     * @param[in]      parent_key           pointer to the configkeyval from
     *                                      which the output configkey val
     *                                      is initialized.
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
    /* Rename */
    bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                              int &nattr, MoMgrTables tbl);
    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);
    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);
    upll_rc_t IsAdminStatusEnable(ConfigKeyVal *ikey, DalDmlIntf *dmi);

  public:
    DhcpRelayServerMoMgr();
    virtual ~DhcpRelayServerMoMgr() {
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

    upll_rc_t GetVrtDhcpRelayServerAddress(ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif
