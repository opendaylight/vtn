/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __CONTROLLER_STUB_HH__
#define __CONTROLLER_STUB_HH__

#include <driver/driver_interface.hh>
#include <driver/controller_interface.hh>
#include <string>

namespace unc {
namespace driver {

class ControllerFramework  {
 public:
  ControllerFramework() {}

  ~ControllerFramework() {}


  UncRespCode GetDriverByControllerName(std::string& controller_name,
                                            controller** ctl, driver** drv) {
    pfc_log_info("%s: res_code:%u ", PFC_FUNCNAME, res_code);
    if (res_code)
      return UNC_DRV_RC_ERR_GENERIC;
    *ctl = controller::create_controll();
    *drv = driver::create_driver();
    return UNC_RC_SUCCESS;
  }


  void AddController(std::string& controller_name,
                     controller* ctl , driver* drv) { }


  UncRespCode UpdateControllerConfiguration(std::string& controller_name,
                     controller*, driver*, const key_ctr&, const val_ctr&) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode RemoveControllerConfiguration(std::string& controller_name) {
    return UNC_RC_SUCCESS;
  }


  driver* GetDriverInstance(unc_keytype_ctrtype_t controller_type) {
    driver *ptr = driver::create_driver();
    return ptr;
  }

  void RegisterDriver(unc_keytype_ctrtype_t controller_type, driver*) { }

  UncRespCode GetControllerInstance(
      std::string& controller_name,
      controller** controller_instance,
      driver** driver_instance) {
    if (res_code)
      return UNC_DRV_RC_ERR_GENERIC;
    *controller_instance = controller::create_controll();
    *driver_instance = driver::create_driver();
    return UNC_RC_SUCCESS;
  }

  UncRespCode
      RemoveControllerConfiguration(
          std::string& controller_name,
          controller* controller_instance,
          driver* driver_instance) {
        return UNC_RC_SUCCESS;
      }
  static void set_result(uint32_t resp);
  static void set_root_result(uint32_t resp);
  static uint32_t res_code;
};
}  // namespace driver
}  // namespace unc
#endif

