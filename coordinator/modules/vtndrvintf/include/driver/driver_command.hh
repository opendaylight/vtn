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
  virtual UncRespCode fetch_config(unc::driver::controller* ctr,
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
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                                     unc::driver::controller*)=0;
  /**
   * @brief    - Method to update VTN  in the controller
   * @param[in]- key_vtn_t, val_vtn_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                                     unc::driver::controller*)=0;
  /**
   * @brief    - Method to delete VTN  in the controller
   * @param[in]- key_vtn_t, val_vtn_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
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
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                                     unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update Vbridge in the controller
   * @param[in]- key_vbr_t, val_vbr_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                                     unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete Vbridge in the controller
   * @param[in]- key_vbr_t, val_vbr_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
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
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vbr_if_t& key,
          pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn)=0;

  /**
   * @brief    - Method to update Vbr Interface in the controller
   * @param[in]- key_vbr_if_t, pfcdrv_val_vbr_if_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vbr_if_t& key,
          pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn) = 0;

  /**
   * @brief    - Method to delete Vbr Interface in the controller
   * @param[in]- key_vbr_if_t, pfcdrv_val_vbr_if_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vbr_if_t& key,
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
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vlan_map_t& key,
                                 pfcdrv_val_vlan_map_t& val,
                                 unc::driver::controller *conn)=0;

  /**
   * @brief    - Method to update Vbr Vlan-Map in the controller
   * @param[in]- key_vlan_map_t, val_vlan_map_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vlan_map_t& key,
                                 pfcdrv_val_vlan_map_t& val,
                                 unc::driver::controller *conn) = 0;

  /**
   * @brief    - Method to delete Vbr Vlan-Map in the controller
   * @param[in]- key_vlan_map_t, val_vlan_map_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vlan_map_t& key,
                                 pfcdrv_val_vlan_map_t& val,
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
   * @retval   - UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode create_cmd(key_ctr_t& key,
                             val_ctr_t & val, unc::driver::controller *conn) {
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /**
   * @brief    - Method to update controller configuration
   * @param[in]- key_ctr_t, val_ctr_t, controller*
   * @retval   - UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode update_cmd(key_ctr_t & key,
                             val_ctr_t& val, unc::driver::controller *conn) {
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /**
   * @brief    - Method to update controller configuration
   * @param[in]- key_ctr_t, val_ctr_t, controller*
   * @retval   - UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode delete_cmd(key_ctr_t & key,
                             val_ctr_t & val, unc::driver::controller *conn) {
    return UNC_DRV_RC_ERR_GENERIC;
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
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_ROOT
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_ROOT;
  }
};

/*
 * @desc:Abstract base Class to be extended for KT_FLOWLIST Commands
 */
class flowlist_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create FlowList in the controller
   * @param[in]- key_flowlist , val_flowlist , controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_flowlist& keyflowlist_,
                                 val_flowlist& valflowlist_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update FlowList in the controller
   * @param[in]- key_flowlist , val_flowlist, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_flowlist& keyflowlist_,
                                 val_flowlist& valflowlist_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete FlowList in the controller
   * @param[in]- key_flowlist , val_flowlist, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_flowlist& keyflowlist_,
                                 val_flowlist& valflowlist_,
                                unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_FLOWLIST
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_FLOWLIST;
  }
};

/*
 * @desc:Abstract base Class to be extended for UNC_KT_FLOWLIST_ENTRY Commands
 */
class flowlist_entry_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create FlowListEntry in the controller
   * @param[in]- key_flowlist_entry, val_flowlist_entry, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_flowlist_entry& keyflentry_,
                                 val_flowlist_entry& valflentry_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update FlowListEntry in the controller
   * @param[in]- key_flowlist_entry, val_flowlist_entry, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_flowlist_entry& keyflentry_,
                                 val_flowlist_entry& valflentry_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete FlowListEntry in the controller
   * @param[in]- key_flowlist_entry, val_flowlist_entry, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_flowlist_entry& keyflentry_,
                                 val_flowlist_entry& valflentry_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_FLOWLIST_ENTRY
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_FLOWLIST_ENTRY;
  }
};

/*
 * @desc:Abstract base Class to be extended for UNC_KT_VTN_FLOWFILTER Commands
 */
class vtn_flowfilter_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create VtnFlowFilter in the controller
   * @param[in]- key_vtn_flowfilter, val_flowfilter, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vtn_flowfilter& key_, val_flowfilter& val_,
                                     unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update VtnFlowFilter in the controller
   * @param[in]- key_vtn_flowfilter, val_flowfilter, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vtn_flowfilter& key_, val_flowfilter& val_,
                                     unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete VtnFlowFilter in the controller
   * @param[in]- key_vtn_flowfilter, val_flowfilter, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vtn_flowfilter& key_, val_flowfilter& val_,
                                     unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VTN_FLOWFILTER
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VTN_FLOWFILTER;
  }
};

/*
 * @desc:Abstract base Class to be extended for UNC_KT_VTN_FLOWFILTER_ENTRY Commands
 */
class vtn_flowfilter_entry_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create VtnFlowFilterEntry in the controller
   * @param[in]- key_vtn_flowfilter_entry, val_vtn_flowfilter_entry, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vtn_flowfilter_entry& key_,
                                 val_vtn_flowfilter_entry& val_,
                                  unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update VtnFlowFilterEntry in the controller
   * @param[in]- key_vtn_flowfilter_entry, val_vtn_flowfilter_entry, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vtn_flowfilter_entry& key_,
                                 val_vtn_flowfilter_entry& val_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete VtnFlowFilterEntry in the controller
   * @param[in]- key_vtn_flowfilter_entry, val_vtn_flowfilter_entry, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vtn_flowfilter_entry& key_,
                                 val_vtn_flowfilter_entry& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VTN_FLOWFILTER_ENTRY
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VTN_FLOWFILTER_ENTRY;
  }
};

/*
 * @desc:Abstract base Class to be extended for UNC_KT_VBR_FLOWFILTER Commands
 */
class vbr_flowfilter_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create VbrFlowFilter in the controller
   * @param[in]- key_vbr_flowfilter, val_flowfilter_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vbr_flowfilter& key_,
                                 val_flowfilter_t& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update VbrFlowFilter in the controller
   * @param[in]- key_vbr_flowfilter, val_flowfilter_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vbr_flowfilter& key_,
                                 val_flowfilter_t& val_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete VbrFlowFilter in the controller
   * @param[in]- key_vbr_flowfilter, val_flowfilter_t, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vbr_flowfilter& key_,
                                 val_flowfilter_t& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VBR_FLOWFILTER
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VBR_FLOWFILTER;
  }
};

/*
 * @desc:Abstract base Class to be extended for
 * UNC_KT_VBR_FLOWFILTER_ENTRY Commands
 */
class vbr_flowfilter_entry_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create VbrFlowFilterEntry in the controller
   * @param[in]- key_vbr_flowfilter_entry, val_flowfilter_entry, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vbr_flowfilter_entry& key_,
                                 val_flowfilter_entry& val_,
                                  unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update VbrFlowFilterEntry in the controller
   * @param[in]- key_vbr_flowfilter_entry, val_flowfilter_entry, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vbr_flowfilter_entry& key_,
                                 val_flowfilter_entry& val_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete VbrFlowFilterEntry in the controller
   * @param[in]- key_vbr_flowfilter_entry, val_flowfilter_entry, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vbr_flowfilter_entry& key_,
                                 val_flowfilter_entry& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VBR_FLOWFILTER_ENTRY
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VBR_FLOWFILTER_ENTRY;
  }
};

/*
 * @desc:Abstract base Class to be extended for
 * UNC_KT_VBRIF_FLOWFILTER Commands
 */
class vbrif_flowfilter_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create VbrifFlowFilter in the controller
   * @param[in]- key_vbr_if_flowfilter, pfcdrv_val_vbrif_vextif, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vbr_if_flowfilter& key_,
                                 pfcdrv_val_vbrif_vextif& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update VbrifFlowFilter in the controller
   * @param[in]- key_vbr_if_flowfilter, pfcdrv_val_vbrif_vextif, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vbr_if_flowfilter& key_,
                                 pfcdrv_val_vbrif_vextif& val_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete VbrifFlowFilter in the controller
   * @param[in]- key_vbr_if_flowfilter, pfcdrv_val_vbrif_vextif, controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vbr_if_flowfilter& key_,
                                 pfcdrv_val_vbrif_vextif& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VBRIF_FLOWFILTER
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VBRIF_FLOWFILTER;
  }
};

/*
 * @desc:Abstract base Class to be extended for
 * UNC_KT_VBRIF_FLOWFILTER_ENTRY Commands
 */
class vbrif_flowfilter_entry_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create VbrifFlowFilterEntry in the controller
   * @param[in]- key_vbr_if_flowfilter_entry, pfcdrv_val_flowfilter_entry,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vbr_if_flowfilter_entry& key_,
                                 pfcdrv_val_flowfilter_entry& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update VbrifFlowFilterEntry in the controller
   * @param[in]- key_vbr_if_flowfilter_entry, pfcdrv_val_flowfilter_entry,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vbr_if_flowfilter_entry& key_,
                                 pfcdrv_val_flowfilter_entry& val_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete VbrifFlowFilterEntry in the controller
   * @param[in]- key_vbr_if_flowfilter_entry, pfcdrv_val_flowfilter_entry,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vbr_if_flowfilter_entry& key_,
                                 pfcdrv_val_flowfilter_entry& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VBRIF_FLOWFILTER_ENTRY
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VBRIF_FLOWFILTER_ENTRY;
  }
};

/*
 * @desc:Abstract base Class to be extended for
 * UNC_KT_VTERMINAL Commands
 */

class vterm_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create Vterminal in the controller
   * @param[in]- key_vterm, val_vterm,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vterm& key_,
                                 val_vterm& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update Vterminal in the controller
   * @param[in]- key_vterm, val_vterm,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vterm& key_,
                                 val_vterm& val_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete Vterminal in the controller
   * @param[in]- key_vterm, val_vterm,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vterm& key_,
                                 val_vterm& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VTERMINAL
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VTERMINAL;
  }
};

/*
 * @desc:Abstract base Class to be extended for
 * UNC_KT_VTERM_IF Commands
 */
class vterm_if_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create Vterminalif in the controller
   * @param[in]- key_vterm_if, val_vterm_if,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vterm_if& key_,
                                 val_vterm_if& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update Vterminalif in the controller
   * @param[in]- key_vterm_if, val_vterm_if,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vterm_if& key_,
                                 val_vterm_if& val_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete Vterminalif in the controller
   * @param[in]- key_vterm_if, val_vterm_if,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vterm_if& key_,
                                 val_vterm_if& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VTERM_IF
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VTERM_IF;
  }
};


/*
 * @desc:Abstract base Class to be extended for
 * UNC_KT_VTERMIF_FLOWFILTER Commands
 */
class vtermif_flowfilter_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create VtermFlowFilter in the controller
   * @param[in]- key_vterm_if_flowfilter, val_flowfilter,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vterm_if_flowfilter& key_,
                                 val_flowfilter& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update VtermFlowFilter in the controller
   * @param[in]- key_vterm_if_flowfilter, val_flowfilter,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vterm_if_flowfilter& key_,
                                 val_flowfilter& val_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete VtermFlowFilter in the controller
   * @param[in]- key_vterm_if_flowfilter, val_flowfilter,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vterm_if_flowfilter& key_,
                                 val_flowfilter& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VTERMIF_FLOWFILTER
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VTERMIF_FLOWFILTER;
  }
};

/*
 * @desc:Abstract base Class to be extended for
 * UNC_KT_VTERMIF_FLOWFILTER_ENTRY Commands
 */
class vtermif_flowfilter_entry_driver_command: public driver_command {
 public:
  /**
   * @brief    - Method to create VtermFlowFilterEntry in the controller
   * @param[in]- key_vterm_if_flowfilter_entry, val_flowfilter_entry,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode create_cmd(key_vterm_if_flowfilter_entry& key_,
                                 val_flowfilter_entry& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to update VtermFlowFilterEntry in the controller
   * @param[in]- key_vterm_if_flowfilter_entry, val_flowfilter_entry,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode update_cmd(key_vterm_if_flowfilter_entry& key_,
                                 val_flowfilter_entry& val_,
                                 unc::driver::controller*) = 0;

  /**
   * @brief    - Method to delete VtermFlowFilterEntry in the controller
   * @param[in]- key_vterm_if_flowfilter_entry, val_flowfilter_entry,
   *             controller*
   * @retval   - UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  virtual UncRespCode delete_cmd(key_vterm_if_flowfilter_entry& key_,
                                 val_flowfilter_entry& val_,
                                 unc::driver::controller*) = 0;
  /**
   * @brief    - Method to return the Keytype
   * @param[in]- None
   * @retval   - unc_key_type_t - UNC_KT_VTERMIF_FLOWFILTER_ENTRY
   */
  unc_key_type_t get_key_type() {
    return UNC_KT_VTERMIF_FLOWFILTER_ENTRY;
  }
};
}  // namespace driver
}  // namespace unc
#endif
