/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License v1.0 which
 * accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_mod.hh>

namespace unc {
namespace odcdriver {

// Get Intsance of vtndrvintf and Register Driver
pfc_bool_t ODCModule::init() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug("Launching Odc Module");
  unc::driver::VtnDrvIntf* disp_inst =
      static_cast<unc::driver::VtnDrvIntf*> (getInstance("vtndrvintf"));
  PFC_ASSERT(disp_inst != NULL);
  uint32_t ret_val = disp_inst->register_driver(this);
  if (ret_val) {
    pfc_log_debug("Register driver failed");
    return PFC_FALSE;
  }
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_TRUE;
}

// Fini
pfc_bool_t ODCModule::fini() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_TRUE;
}

// Gets Conttoller type
unc_keytype_ctrtype_t ODCModule::get_controller_type() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return UNC_CT_ODC;
}

// Is two phase commit needed or not
pfc_bool_t ODCModule::is_2ph_commit_support_needed() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_FALSE;
}

// Is Audit Collection needed or not
pfc_bool_t ODCModule::is_audit_collection_needed() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_FALSE;
}

// Return New Contrtoller Pointer
unc::driver::controller* ODCModule::add_controller(const key_ctr_t& key_ctr,
                                                   const val_ctr_t& val_ctr) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  const char* ctr_name =
      reinterpret_cast<const char*>(key_ctr.controller_name);
  if (0 == strlen(ctr_name)) {
    return NULL;
  }
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return new ODCController(key_ctr, val_ctr);
}

// Start audit notification to TC
uint32_t ODCModule::notify_audit_start_to_tc(
    std::string controller_id) {
  // Send start audit notification to TC
  unc::tclib::TcLibModule* ptr_tclib_key_data;
  pfc::core::Module* ptr_tclib = NULL;
  ptr_tclib  = static_cast<unc::tclib::TcLibModule*>
      (unc::tclib::TcLibModule::getInstance(TCLIB_MODULE_NAME));
  PFC_ASSERT(ptr_tclib != NULL);
  ptr_tclib_key_data = static_cast<unc::tclib::TcLibModule*> (ptr_tclib);
  ptr_tclib_key_data->TcLibAuditControllerRequest(controller_id);
  return ODC_DRV_SUCCESS;
}

// Return Updated Contrtoller Pointer
unc::driver::controller* ODCModule::update_controller(const key_ctr_t& key_ctr,
                                                      const val_ctr_t& val_ctr,
                                                      unc::driver::controller*
                                                      ctr_ptr) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  unc::driver::controller *controller_ptr = NULL;

  const char* ctr_name =
      reinterpret_cast<const char*>(key_ctr.controller_name);
  if (0 == strlen(ctr_name)) {
    return controller_ptr;
  }
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return new ODCController(key_ctr, val_ctr);
}

// Deletes the Conttoller Pointer
pfc_bool_t ODCModule::delete_controller(unc::driver::controller* ctr_ptr) {
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
unc::driver::driver_command* ODCModule::get_driver_command(
    unc_key_type_t key_type) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  unc::driver::driver_command* driver_cmd_ptr = NULL;
  switch (key_type) {
    case UNC_KT_VTN: {
      pfc_log_debug("UNC_KT_VTN key type received");
      driver_cmd_ptr = new ODCVTNCommand();
      break;
    }
    case UNC_KT_VBRIDGE: {
      pfc_log_debug("UNC_KT_VBR key type received");
      driver_cmd_ptr = new ODCVBRCommand();
      break;
    }
    case UNC_KT_VBR_IF: {
      pfc_log_debug("UNC_KT_VBRIF key type received");
      driver_cmd_ptr = new ODCVBRIfCommand();
      break;
    }
    case UNC_KT_ROOT: {
      pfc_log_debug("UNC_KT_ROOT key type received");
      driver_cmd_ptr = new ODCROOTCommand();
      break;
    }
    default:
      pfc_log_debug("Unknown keytype received : %d", key_type);
      break;
  }
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return driver_cmd_ptr;
}

// Gets the ping interval
uint32_t ODCModule::get_ping_interval() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PING_INTERVAL;
}

// Gets the ping fail retry count
uint32_t ODCModule::get_ping_fail_retry_count() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PING_RETRY_COUNT;
}

// Is ping need or not
pfc_bool_t ODCModule::is_ping_needed() {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_TRUE;
}

//  ping controller
pfc_bool_t ODCModule::ping_controller(unc::driver::controller* ctr) {
  PFC_ASSERT(ctr != NULL);
  std::string ipaddress = ctr->get_host_address();

  if ((ipaddress.compare("0.0.0.0") == 0)  ||
      (0 == strlen(ipaddress.c_str()))) {
    pfc_log_debug(" ipaddress empty");
    return PFC_FALSE;
  }
  restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  std::string url = "/controller/nb/v2/vtn/version";
  uint32_t retval = ODC_DRV_FAILURE;
  retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  if (ODC_DRV_FAILURE == retval) {
    return PFC_FALSE;
  }
  retval = rest_util_obj.set_timeout(CONNECTION_TIME_OUT, REQUEST_TIME_OUT);
  if (ODC_DRV_FAILURE == retval) {
    return PFC_FALSE;
  }
  retval = rest_util_obj.create_request_header(url, restjson::HTTP_METHOD_GET);
  if (ODC_DRV_FAILURE == retval) {
    return PFC_FALSE;
  }
  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  if (RESP_OK != resp) {
    return PFC_FALSE;
  }
  pfc_bool_t new_audit_status = ctr->get_audit_status();
  unc::driver::ConnectionStatus connection_status =
      ctr->get_connection_status();
  if ((connection_status) && (new_audit_status)) {
    std::string controller_name = ctr->get_controller_id();
    notify_audit_start_to_tc(controller_name);
  }
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return PFC_TRUE;
}

unc::tclib::TcCommonRet ODCModule::HandleVote(unc::driver::controller*) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return unc::tclib::TC_FAILURE;
}

unc::tclib::TcCommonRet ODCModule::HandleCommit(unc::driver::controller*) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return unc::tclib::TC_FAILURE;
}

unc::tclib::TcCommonRet ODCModule::HandleAbort(unc::driver::controller*) {
  pfc_log_debug(" %s Entering Function" , PFC_FUNCNAME);
  pfc_log_debug(" %s Exiting Function" , PFC_FUNCNAME);
  return unc::tclib::TC_FAILURE;
}
}  // namespace odcdriver
}  // namespace unc
PFC_MODULE_IPC_DECL(unc::odcdriver::ODCModule, 0);
