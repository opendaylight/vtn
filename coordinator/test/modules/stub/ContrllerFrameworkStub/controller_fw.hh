/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __CONTROLLER_STUB_HH__
#define __CONTROLLER_STUB_HH__

#include <driver/driver_interface.hh>
#include <string>

namespace unc {
namespace driver {

class ControllerFramework  {
 public:
  ControllerFramework() {}

  ~ControllerFramework() {}


  drv_resp_code_t GetDriverByControllerName(std::string& controller_name,
                                            controller** ctl, driver** drv) {
    pfc_log_info("%s: res_code:%u ", PFC_FUNCNAME, res_code);
    if (res_code)
      return DRVAPI_RESPONSE_FAILURE;
    *ctl = controller::create_controll();
    *drv = driver::create_driver();
    return DRVAPI_RESPONSE_SUCCESS;
  }


  void AddController(std::string& controller_name,
                     controller* ctl , driver* drv) { }


  drv_resp_code_t UpdateControllerConfiguration(std::string& controller_name,
                     controller*, driver*, const key_ctr&, const val_ctr&) {
    return DRVAPI_RESPONSE_SUCCESS;
  }

  drv_resp_code_t RemoveControllerConfiguration(std::string& controller_name) {
    return DRVAPI_RESPONSE_SUCCESS;
  }


  driver* GetDriverInstance(unc_keytype_ctrtype_t controller_type) {
    driver *ptr = driver::create_driver();
    return ptr;
  }

  void RegisterDriver(unc_keytype_ctrtype_t controller_type, driver*) { }

  drv_resp_code_t GetControllerInstance(
      std::string& controller_name,
      controller** controller_instance,
      driver** driver_instance) {
    if (res_code)
      return DRVAPI_RESPONSE_FAILURE;
    *controller_instance = controller::create_controll();
    *driver_instance = driver::create_driver();
    return DRVAPI_RESPONSE_SUCCESS;
  }

  drv_resp_code_t
      RemoveControllerConfiguration(
          std::string& controller_name,
          controller* controller_instance,
          driver* driver_instance) {
        return DRVAPI_RESPONSE_SUCCESS;
      }
  static void set_result(uint32_t resp);
  static void set_root_result(uint32_t resp);
  static uint32_t res_code;
};
}  // namespace driver
}  // namespace unc
#endif

