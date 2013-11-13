/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "controller_fw.hh"
#include <controller_interface.hh>

namespace unc {
namespace driver {
controller* controller :: controller_ptr = NULL;
driver* driver::driver_ptr = NULL;
uint32_t driver::set_ctrl = 1;
uint32_t ControllerFramework::res_code = 0;
uint32_t root_driver_command::set_root_child = 0;
controller* controller :: create_controll() {
    if (controller_ptr == NULL)
      controller_ptr = new controller();

    return controller_ptr;
}
void ControllerFramework ::set_result(uint32_t resp) {
  res_code = resp;
}

void ControllerFramework ::set_root_result(uint32_t resp) {
  root_driver_command::set_root_child = resp;
}

driver* driver::create_driver() {
     if (driver_ptr == NULL)
        driver_ptr = new driver();
     return driver_ptr;
}


void driver::set_ctrl_instance(uint32_t ctrl_inst) {
  set_ctrl = ctrl_inst;
}

}  // namespace driver
}  // namespace unc
