/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __CDF_DRIVER_COMMANDS_HH__
#define __CDF_DRIVER_COMMANDS_HH__

#include <driver/controller_interface.hh>
#include <pfc/ipc_struct.h>
#include <uncxx/dataflow.hh>
#include <unc/unc_base.h>
#include <confignode.hh>
#include <tclib_module.hh>
#include <vector>
#include <string>
#include <driver/vtn_read_value_util.hh>

namespace unc {
namespace driver {

typedef struct val_root {
  uint8_t     root_key;
} val_root_t;

/*
 * @desc:Base Class For Driver Commands
 */
class driver_command {
 public:
  virtual ~driver_command() {}

  /**
   * @brief    - Method to revoke the commit with triggring audit for any
                 failed Operation
   * @param[in]- controller pointer
   * @retval   - UNC_RC_SUCCESS
   */
  virtual UncRespCode revoke(unc::driver::controller* ctr_ptr) {
    pfc_log_debug("%s Entering function", PFC_FUNCNAME);

    // Send start audit notification to TC
    unc::tclib::TcLibModule* ptr_tclib_key_data = NULL;
    ptr_tclib_key_data  = static_cast<unc::tclib::TcLibModule*>
        (unc::tclib::TcLibModule::getInstance("tclib"));

    PFC_ASSERT(ptr_tclib_key_data != NULL);

    std::string controller_name = ctr_ptr->get_controller_id();
    pfc_log_debug("revoke controller_name:%s", controller_name.c_str());
    ptr_tclib_key_data->TcLibAuditControllerRequest(controller_name);

    pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
    return UNC_RC_SUCCESS;
  }

  /**
   * @brief      - Method to fetch child configurations for the parent kt
   * @param[in]  - controller pointer
   * @param[in]  - parent key type pointer
   * @param[out] - list of configurations
   * @retval     - UNC_RC_SUCCESS / UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode fetch_config(
      unc::driver::controller* ctr,
      void* parent_key,
      std::vector<unc::vtndrvcache::ConfigNode *>&) = 0;
};

/*
 * @desc:Template Class For Driver Commands
 */
template<class key_cmd, class val_cmd>
class vtn_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create VTN/Vbridge/Vbridge Interface/VLANMAP in
   *             controller
   * @param[in]- key_vtn_t, val_vtn_t, key_vbr_t, val_vbr_t, key_vbr_if_t,
   *             pfcdrv_val_vbr_if_t, key_vbr_if_t, pfcdrv_val_vbr_if_t,
   *             key_vlan_map_t, pfcdrv_val_vlan_map_t controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_cmd& key_st,
                                 val_cmd& val_st,
                                 unc::driver::controller*)=0;

  /**
   * @brief    - Method to update VTN/Vbridge/Vbridge Interface/VLANMAP in
   *             controller
   * @param[in]- key_vtn_t, val_vtn_t, key_vbr_t, val_vbr_t, key_vbr_if_t,
   *             pfcdrv_val_vbr_if_t, key_vbr_if_t, pfcdrv_val_vbr_if_t,
   *             key_vlan_map_t, pfcdrv_val_vlan_map_t controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_cmd& key_st,
                                 val_cmd& val_old_st,
                                 val_cmd& val_new_st,
                                 unc::driver::controller*)=0;

  /**
   * @brief    - Method to delete VTN/Vbridge/Vbridge Interface/VLANMAP in
   *             controller
   * @param[in]- key_vtn_t, val_vtn_t, key_vbr_t, val_vbr_t, key_vbr_if_t,
   *             pfcdrv_val_vbr_if_t, key_vbr_if_t, pfcdrv_val_vbr_if_t,
   *             key_vlan_map_t, pfcdrv_val_vlan_map_t controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_cmd& key_st,
                                 val_cmd& val_st,
                                 unc::driver::controller*)=0;
};

class vtn_driver_read_command : public driver_command {
  public:
  /**
   * @brief    - Method to read only Dataflow config from controller
   * @param[in]- key_dataflow , key_port, key_vtnstation
   *             val_dataflow,  val_port, val_vtnstation
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode read_cmd(unc::driver::controller*,
                               unc::vtnreadutil::driver_read_util*)=0;

  virtual UncRespCode fetch_config(
             unc::driver::controller* ctr,
             void* parent_key,
             std::vector<unc::vtndrvcache::ConfigNode *>&) {
    return UNC_RC_SUCCESS;
  }
};

/*
 * @desc:Abstract base Class to be extended for Audit KT_ROOT Commands
 */
class root_driver_command : public driver_command {
 public:
  /**
   * @brief    - Method to form the create command for Audit
   * @param[in]- key_root_t, val_root_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode
      create_cmd(key_root_t& key,
                 val_root_t & val,
                 unc::driver::controller *conn) = 0;

  /**
   * @brief    - Method to form the KT_ROOT update oommand
   * @param[in]- key_root_t, val_root_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode
      update_cmd(key_root_t& key,
                 val_root_t & val,
                 unc::driver::controller *conn) = 0;

  /**
   * @brief    - Method to form the KT_ROOT delete oommand
   * @param[in]- key_root_t, val_root_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode
      delete_cmd(key_root_t& key,
                 val_root_t & val,
                 unc::driver::controller *conn)=0;

  /**
   * @brief    - Method to read configurations during Audit
   * @param[in]- vector<unc::vtndrvcache::ConfigNode*>, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode
      read_root_child(std::vector<unc::vtndrvcache::ConfigNode*>&,
                      unc::driver::controller*) = 0;

  /**
   * @brief    - Method to read configurations from controller during Audit
   * @param[in]- vector<unc::vtndrvcache::ConfigNode*>, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode
      read_all_child(unc::vtndrvcache::ConfigNode*,
                     std::vector<unc::vtndrvcache::ConfigNode*>&,
                     unc::driver::controller*)=0;
};
}  // namespace driver
}  // namespace unc
#endif
