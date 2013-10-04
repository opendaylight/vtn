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

#include <odc_mod.hh>

namespace odc {
namespace driver {

// Get Intsance of vtndrvintf and Register Driver
pfc_bool_t OdcModule::init() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_info("Launching Odc Module");
  unc::driver::VtnDrvIntf* disp_inst =
       static_cast<unc::driver::VtnDrvIntf*> (getInstance("vtndrvintf"));
  PFC_ASSERT(disp_inst != NULL);
  uint32_t ret_val = disp_inst->register_driver(this);
  if (ret_val) {
    pfc_log_info("Register driver failed");
    return PFC_FALSE;
  }
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_TRUE;
}

// Fini
pfc_bool_t OdcModule::fini() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_TRUE;
}

// Gets Conttoller type
unc_keytype_ctrtype_t OdcModule::get_controller_type() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return UNC_CT_ODC;
}

// Is two phase commit needed or not
pfc_bool_t OdcModule::is_2ph_commit_support_needed() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_FALSE;
}

// Is Audit Collection needed or not
pfc_bool_t OdcModule::is_audit_collection_needed() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_FALSE;
}

// Return New Contrtoller Pointer
unc::driver::controller* OdcModule::add_controller(const key_ctr_t& key_ctr,
    const val_ctr_t& val_ctr) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return new ODCController(key_ctr, val_ctr);
}

// Return Updated Contrtoller Pointer
unc::driver::controller* OdcModule::update_controller(const key_ctr_t& key_ctr,
    const val_ctr_t& val_ctr, unc::driver::controller* ctr_ptr) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  unc::driver::controller *controller_ptr = NULL;

  const char* ctr_name = reinterpret_cast<const char*>(key_ctr.controller_name);
  if (0 == strlen(ctr_name)) {
    return controller_ptr;
  }

  std::string ip_addr = inet_ntoa(val_ctr.ip_address);
  if (ip_addr.compare("0.0.0.0") != 0) {
    return controller_ptr;
  }
  const char* version = reinterpret_cast<const char*>(val_ctr.version);
  if (0 == strlen(version)) {
    return controller_ptr;
  }

  if (ctr_ptr != NULL) {
    delete ctr_ptr;
  }
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return new ODCController(key_ctr, val_ctr);
}

// Deletes the Conttoller Pointer
pfc_bool_t OdcModule::delete_controller(unc::driver::controller* ctr_ptr) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  if (NULL != ctr_ptr) {
    delete ctr_ptr;
    ctr_ptr = NULL;
    return PFC_TRUE;
  }
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_FALSE;
}

// Gets the driver command
unc::driver::driver_command* OdcModule::get_driver_command(
    unc_key_type_t key_type) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  unc::driver::driver_command* driver_cmd_ptr = NULL;
  switch (key_type) {
  case UNC_KT_VTN: {
    pfc_log_debug("UNC_KT_VTN key type received");
    driver_cmd_ptr = new ODLVTNCommand();
    break;
  }
  case UNC_KT_VBRIDGE: {
    pfc_log_debug("UNC_KT_VBR key type received");
    driver_cmd_ptr = new ODLVBRCommand();
    break;
  }
  case UNC_KT_VBR_IF: {
    pfc_log_debug("UNC_KT_VBRIF key type received");
    driver_cmd_ptr = new ODLVBRIfCommand();
    break;
  }
  default:
    pfc_log_debug("Unknown keytype received : %d", key_type);
    break;
  }
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return driver_cmd_ptr;
}

unc::tclib::TcCommonRet OdcModule::HandleVote(unc::driver::controller*) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return unc::tclib::TC_FAILURE;
}

unc::tclib::TcCommonRet OdcModule::HandleCommit(unc::driver::controller*) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return unc::tclib::TC_FAILURE;
}

unc::tclib::TcCommonRet OdcModule::HandleAbort(unc::driver::controller*) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return unc::tclib::TC_FAILURE;
}
}
}
// Declare C++ module
PFC_MODULE_IPC_DECL(odc::driver::OdcModule, 0);
