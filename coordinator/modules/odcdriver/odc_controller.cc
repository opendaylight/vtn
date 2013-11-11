/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_controller.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcController::OdcController(const key_ctr_t& key_ctr, const val_ctr_t& val_ctr)
: ip_addr_(inet_ntoa(val_ctr.ip_address)),
  controller_name_(reinterpret_cast<const char*>(key_ctr.controller_name)),
  version_(reinterpret_cast<const char*>(val_ctr.version)),
  description_(reinterpret_cast<const char*>(val_ctr.description)),
  user_name_(reinterpret_cast<const char*>(val_ctr.user)),
  pass_word_(reinterpret_cast<const char*>(val_ctr.password)),
  audit_(val_ctr.enable_audit) {
    ODC_FUNC_TRACE;
}

// Destructor
OdcController::~OdcController() {
}

pfc_bool_t OdcController::update_ctr(const key_ctr_t& key_ctr,
                                     const val_ctr_t& val_ctr) {
  ODC_FUNC_TRACE;
  controller_name_ = (reinterpret_cast<const char*>(key_ctr.controller_name));
  version_         = (reinterpret_cast<const char*>(val_ctr.version));
  description_     = (reinterpret_cast<const char*>(val_ctr.description));
  user_name_       = (reinterpret_cast<const char*>(val_ctr.user));
  pass_word_       = (reinterpret_cast<const char*>(val_ctr.password));
  ip_addr_         = (inet_ntoa(val_ctr.ip_address));
  audit_           = val_ctr.enable_audit;
  return PFC_TRUE;
}

// Gets the controller type
unc_keytype_ctrtype_t OdcController::get_controller_type() {
  ODC_FUNC_TRACE;
  return UNC_CT_ODC;
}

pfc_bool_t OdcController::get_audit_status() {
  ODC_FUNC_TRACE;
  return audit_;
}

// Gets the controller id
std::string OdcController::get_controller_id() {
  ODC_FUNC_TRACE;
  return controller_name_;
}

// Gets the host address
std::string OdcController::get_host_address() {
  ODC_FUNC_TRACE;
  return ip_addr_;
}

// Reset connect or not
pfc_bool_t OdcController::reset_connect() {
  ODC_FUNC_TRACE;
  return PFC_FALSE;
}

// Gets the user name
std::string OdcController::get_user_name() {
  ODC_FUNC_TRACE;
  return user_name_;
}

// Gets the pass word
std::string OdcController::get_pass_word() {
  ODC_FUNC_TRACE;
  return pass_word_;
}
}  //  namespace odcdriver
}  //  namespace unc
