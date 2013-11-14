/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_CONTROLLER_HH_
#define _ODC_CONTROLLER_HH_

#include <driver/driver_interface.hh>
#include <pfc/ipc_struct.h>
#include <string>

namespace unc {
namespace driver {

class OdcController: public unc::driver::controller {
 public:
  OdcController() {}

  ~OdcController() {}

  unc_keytype_ctrtype_t get_controller_type() {
    return UNC_CT_ODC;
  }

  std::string get_controller_id() {
    return "ctr1";
  }

  pfc_bool_t  reset_connect() {
    return PFC_TRUE;
  }

  std::string get_host_address() {
    return "10.10.1.1";
  }

  pfc_bool_t get_audit_status() {
    return PFC_TRUE;
  }

  std::string  get_user_name() {
    return "str";
  }

  std::string get_pass_word() {
    return "pass";
  }

  pfc_bool_t update_ctr(const key_ctr_t& key_ctr, const val_ctr_t& val_ctr) {
    return PFC_TRUE;
  }
};
}  // namespace driver
}  // namespace unc
#endif
