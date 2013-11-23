/*
 * Copyright (c) 2013 NEC Corporation
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
#include <unc/unc_base.h>
#include <confignode.hh>
#include <tclib_module.hh>
#include <vector>
#include <string>

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
    virtual unc_key_type_t get_key_type()=0;

   /**
    * @brief    - Method to revoke the commit with triggring audit for any
                  failed Operation
    * @param[in]- controller pointer
    * @retval   - DRVAPI_RESPONSE_SUCCESS
    */
    virtual drv_resp_code_t revoke(unc::driver::controller* ctr_ptr) {
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
      return DRVAPI_RESPONSE_SUCCESS;
    }
    /**
     * @brief      - Method to fetch child configurations for the parent kt
     * @param[in]  - controller pointer
     * @param[in]  - parent key type pointer
     * @param[out] - list of configurations
     * @retval     - DRVAPI_RESPONSE_SUCCESS / DRVAPI_RESPONSE_FAILURE
     */
    virtual drv_resp_code_t fetch_config(unc::driver::controller* ctr,
                            void* parent_key,
                            std::vector<unc::vtndrvcache::ConfigNode *>&) = 0;
};

/*
 * @desc:Abstract base Class to be extended for VTN Commands
 */
class vtn_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create VTN  in the controller
   * @param[in]- key_vtn_t, val_vtn_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t create_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                                     unc::driver::controller*)=0;
  /**
   * @brief    - Method to update VTN  in the controller
   * @param[in]- key_vtn_t, val_vtn_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t update_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                                     unc::driver::controller*)=0;
  /**
   * @brief    - Method to delete VTN  in the controller
   * @param[in]- key_vtn_t, val_vtn_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t delete_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                                     unc::driver::controller*)=0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VTN
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VTN;
  }
};

/*
 * @desc:Abstract base Class to be extended for VBR Commands
 */
class vbr_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create Vbridge in the controller
   * @param[in]- key_vbr_t, val_vbr_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t create_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                                     unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update Vbridge in the controller
   * @param[in]- key_vbr_t, val_vbr_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t update_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                                     unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete Vbridge in the controller
   * @param[in]- key_vbr_t, val_vbr_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t delete_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                                     unc::driver::controller*)=0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VBRIDGE
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VBRIDGE;
  }
};

/*
 * @desc:Abstract base Class to be extended for VBRIf Commands
 */
class vbrif_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create Vbr Interface in the controller
   * @param[in]- key_vbr_if_t, pfcdrv_val_vbr_if_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t create_cmd(key_vbr_if_t& key,
                                     pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn)=0;

  /**
   * @brief    - Method to update Vbr Interface in the controller
   * @param[in]- key_vbr_if_t, pfcdrv_val_vbr_if_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t update_cmd(key_vbr_if_t& key,
                                     pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn) = 0;

  /**
   * @brief    - Method to delete Vbr Interface in the controller
   * @param[in]- key_vbr_if_t, pfcdrv_val_vbr_if_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t delete_cmd(key_vbr_if_t& key,
                                     pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn) = 0;

  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VBR_IF
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VBR_IF;
  }
};

class vbrvlanmap_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create Vbr Vlan-Map in the controller
   * @param[in]- key_vlan_map_t, val_vlan_map_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t create_cmd(key_vlan_map_t& key,
                                     val_vlan_map_t& val,
                                     unc::driver::controller *conn)=0;

  /**
   * @brief    - Method to update Vbr Vlan-Map in the controller
   * @param[in]- key_vlan_map_t, val_vlan_map_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t update_cmd(key_vlan_map_t& key,
                                     val_vlan_map_t& val,
                                     unc::driver::controller *conn) = 0;

  /**
   * @brief    - Method to delete Vbr Vlan-Map in the controller
   * @param[in]- key_vlan_map_t, val_vlan_map_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t delete_cmd(key_vlan_map_t& key,
                                     val_vlan_map_t& val,
                                     unc::driver::controller *conn) = 0;

  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VBR_VLANMAP
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VBR_VLANMAP;
  }
};

/*
 * @desc:Abstract base Class to be extended for VBRIf Commands
 */
class controller_command: public driver_command {
 public:
  /**
   * @brief    - Method to create controller configuration
   * @param[in]- key_ctr_t, val_ctr_t, controller*
   * @retval   - DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t create_cmd(key_ctr_t& key,
                             val_ctr_t & val, unc::driver::controller *conn) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  /**
   * @brief    - Method to update controller configuration
   * @param[in]- key_ctr_t, val_ctr_t, controller*
   * @retval   - DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t update_cmd(key_ctr_t & key,
                             val_ctr_t& val, unc::driver::controller *conn) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  /**
   * @brief    - Method to update controller configuration
   * @param[in]- key_ctr_t, val_ctr_t, controller*
   * @retval   - DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t delete_cmd(key_ctr_t & key,
                             val_ctr_t & val, unc::driver::controller *conn) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_CONTROLLER
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_CONTROLLER;
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
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t
      create_cmd(key_root_t& key,
                 val_root_t & val,
                 unc::driver::controller *conn) = 0;

  /**
   * @brief    - Method to form the KT_ROOT update oommand
   * @param[in]- key_root_t, val_root_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t
      update_cmd(key_root_t& key,
                 val_root_t & val,
                 unc::driver::controller *conn) = 0;

  /**
   * @brief    - Method to form the KT_ROOT delete oommand
   * @param[in]- key_root_t, val_root_t, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t
      delete_cmd(key_root_t& key,
                 val_root_t & val,
                 unc::driver::controller *conn)=0;

  /**
   * @brief    - Method to read configurations during Audit
   * @param[in]- vector<unc::vtndrvcache::ConfigNode*>, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t
      read_root_child(std::vector<unc::vtndrvcache::ConfigNode*>&,
                      unc::driver::controller*) = 0;

  /**
   * @brief    - Method to read configurations from controller during Audit
   * @param[in]- vector<unc::vtndrvcache::ConfigNode*>, controller*
   * @retval   - DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
   */
  virtual drv_resp_code_t
      read_all_child(unc::vtndrvcache::ConfigNode*,
                     std::vector<unc::vtndrvcache::ConfigNode*>&,
                     unc::driver::controller*)=0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_ROOT
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_ROOT;
  }
};
}  // namespace driver
}  // namespace unc
#endif
