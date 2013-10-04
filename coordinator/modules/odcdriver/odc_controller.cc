/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the
 * terms of the Eclipse Public License v1.0 which
 * accompanies this
 * distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_controller.hh>

namespace odc {
namespace driver {

// Constructor
ODCController::ODCController(const key_ctr_t& key_ctr, const val_ctr_t& val_ctr)
: ip_addr_(inet_ntoa(val_ctr.ip_address)),
  controller_name_(reinterpret_cast<const char*>(key_ctr.controller_name)),
  version_(reinterpret_cast<const char*>(val_ctr.version)),
  description_(reinterpret_cast<const char*>(val_ctr.description)),
  audit_(val_ctr.enable_audit) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
}

// Destructor
ODCController::~ODCController() {
}

// Gets the contoleer type
unc_keytype_ctrtype_t ODCController::get_controller_type() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return UNC_CT_ODC;
}

// Is ping need or not
pfc_bool_t ODCController::is_ping_needed() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_FALSE;
}

// Gets the ping interval
uint32_t ODCController::get_ping_interval() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return 0;
}

// Gets the ping fail retry count
uint32_t ODCController::get_ping_fail_retry_count() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return 0;
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

// Ping controller or not
pfc_bool_t ODCController::ping_controller() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_FALSE;
}

// Reset connect or not
pfc_bool_t ODCController::reset_connect() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_FALSE;
}
}
}
