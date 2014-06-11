/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __DRIVER_STUB_HH__
#define __DRIVER_STUB_HH__


#include <unc/keytype.h>
#include <unc/unc_base.h>
#include <pfc/ipc_struct.h>
#include <vtndrvintf_defs.h>
#include <keytree.hh>
#include <string>
#include "driver_command.hh"
#include "controller_interface.hh"

namespace unc {
namespace driver {


class driver {
 public:
  driver() {}
  virtual ~driver() {
    if (driver_ptr != NULL) {
      delete driver_ptr;
      driver_ptr = NULL;
    }
  }
  bool is_2ph_commit_support_needed() {
    if (set_ctrl == 0) {
      return true;
    }
    return false;
  }
  bool is_audit_collection_needed() {
    return true;
  }

  controller* add_controller(key_ctr_t& key_ctr,
                             val_ctr_t& val_ctr) {
    controller* ctrl_inst = NULL;
    if (set_ctrl)
      ctrl_inst = controller::create_controll();
    return ctrl_inst;
  }

  controller* update_controller(key_ctr_t& key_ctr,
                                val_ctr_t& val_ctr,
                                controller* ctrl_inst) {
    ctrl_inst = NULL;
    if (set_ctrl)
      ctrl_inst = controller::create_controll();
    return ctrl_inst;
  }

  unc_keytype_ctrtype_t get_controller_type() {
    return UNC_CT_ODC;
  }

  bool delete_controller(controller* delete_inst) {
    return true;
  }
  virtual driver_command* create_driver_command(unc_key_type_t key_type) {
    driver_command *ptr = NULL;
    switch (key_type) {
      case UNC_KT_CONTROLLER:
        ptr = static_cast<driver_command*>
            (new unc::driver::vtn_driver_command<key_ctr_t, val_ctr_t>);
        return ptr;
        break;
      case UNC_KT_ROOT:
        ptr = static_cast<driver_command*>
            (new root_driver_command);
        return ptr;
        break;
      case UNC_KT_VTN:
        ptr = static_cast<driver_command*>
            (new unc::driver::vtn_driver_command<key_vtn_t, val_vtn_t>);
        return ptr;
        break;
      case UNC_KT_VBRIDGE:
        ptr = static_cast<driver_command*>
            (new unc::driver::vtn_driver_command<key_vbr_t, val_vbr_t>);
        return ptr;
        break;
      case UNC_KT_VBR_IF:
        ptr = static_cast<driver_command*>
            (new unc::driver::vtn_driver_command<key_vbr_if_t, pfcdrv_val_vbr_if_t>);
        return ptr;
        break;
      default:
        return ptr;
        break;
    }
  }

  unc::tclib::TcCommonRet HandleVote(unc::driver::controller*) {
    if (set_result == 0) {
      return unc::tclib::TC_SUCCESS;
    }
    return unc::tclib::TC_FAILURE;
  }

  unc::tclib::TcCommonRet HandleCommit(unc::driver::controller*) {
    if (set_result == 0) {
      return unc::tclib::TC_SUCCESS;
    }
    return unc::tclib::TC_FAILURE;
  }

  unc::tclib::TcCommonRet HandleAbort(unc::driver::controller*) {
    if (set_result == 0) {
      return unc::tclib::TC_SUCCESS;
    }
    return unc::tclib::TC_FAILURE;
  }

  pfc_bool_t  get_physical_port_details(unc::driver::controller*) {
  return PFC_TRUE;
   }

  static driver* create_driver();
  static driver* driver_ptr;
  static void set_ctrl_instance(uint32_t ctrl_inst);
  static void set_ret_code(uint32_t ret_code);
  static uint32_t set_ctrl;
  static uint32_t set_result;
};
}  // namespace driver
}  // namespace unc
#endif
