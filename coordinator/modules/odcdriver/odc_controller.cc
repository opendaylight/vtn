/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License v1.0 which
 * accompanies this  distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_controller.hh>

namespace unc {
namespace odcdriver {

// Constructor
ODCController::ODCController(const key_ctr_t& key_ctr, const val_ctr_t& val_ctr)
    : ip_addr_(inet_ntoa(val_ctr.ip_address)),
    controller_name_(reinterpret_cast<const char*>(key_ctr.controller_name)),
    version_(reinterpret_cast<const char*>(val_ctr.version)),
    description_(reinterpret_cast<const char*>(val_ctr.description)),
    user_name_(reinterpret_cast<const char*>(val_ctr.user)),
    pass_word_(reinterpret_cast<const char*>(val_ctr.password)),
    audit_(val_ctr.enable_audit) {
      pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
      pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
    }

// Destructor
ODCController::~ODCController() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
}

// Gets the controller type
unc_keytype_ctrtype_t ODCController::get_controller_type() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return UNC_CT_ODC;
}

pfc_bool_t ODCController::get_audit_status() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return audit_;
}

// Gets the controller id
std::string ODCController::get_controller_id() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return controller_name_;
}

// Gets the host address
std::string ODCController::get_host_address() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return ip_addr_;
}

// Reset connect or not
pfc_bool_t ODCController::reset_connect() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_FALSE;
}

// Gets the user name
std::string ODCController::get_user_name() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return user_name_;
}

// Gets the pass word
std::string ODCController::get_pass_word() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return pass_word_;
}
}  //  namespace odcdriver
}  //  namespace unc
