/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_mod.hh>

namespace unc {
namespace odcdriver {

// Get Intsance of vtndrvintf and Register Driver
pfc_bool_t ODCModule::init() {
  ODC_FUNC_TRACE;
  pfc_log_debug("Launching Odc Module");
  unc::driver::VtnDrvIntf* disp_inst =
      static_cast<unc::driver::VtnDrvIntf*> (getInstance("vtndrvintf"));
  PFC_ASSERT(disp_inst != NULL);
  uint32_t ret_val = disp_inst->register_driver(this);
  if (ret_val) {
    pfc_log_error("Register driver failed");
    return PFC_FALSE;
  }
  pfc_log_debug(" %s Read Configuration file" , PFC_FUNCNAME);
  read_conf_file();
  return PFC_TRUE;
}

// Fini
pfc_bool_t ODCModule::fini() {
  ODC_FUNC_TRACE;
  return PFC_TRUE;
}

// Gets Conttoller type
unc_keytype_ctrtype_t ODCModule::get_controller_type() {
  ODC_FUNC_TRACE;
  return UNC_CT_ODC;
}

// Is two phase commit needed or not
pfc_bool_t ODCModule::is_2ph_commit_support_needed() {
  ODC_FUNC_TRACE;
  return PFC_FALSE;
}

// Is Audit Collection needed or not
pfc_bool_t ODCModule::is_audit_collection_needed() {
  ODC_FUNC_TRACE;
  return PFC_FALSE;
}

// Return New Contrtoller Pointer
unc::driver::controller* ODCModule::add_controller(const key_ctr_t& key_ctr,
                                                   const val_ctr_t& val_ctr) {
  ODC_FUNC_TRACE;
  const char* ctr_name =
      reinterpret_cast<const char*>(key_ctr.controller_name);
  if (0 == strlen(ctr_name)) {
    return NULL;
  }
  return new OdcController(key_ctr, val_ctr);
}

// Start audit notification to TC
uint32_t ODCModule::notify_audit_start_to_tc(
    std::string controller_id) {
  ODC_FUNC_TRACE;
  // Send start audit notification to TC
  unc::tclib::TcLibModule* ptr_tclib = NULL;
  ptr_tclib  = static_cast<unc::tclib::TcLibModule*>
      (unc::tclib::TcLibModule::getInstance(TCLIB_MODULE_NAME));
  PFC_ASSERT(ptr_tclib != NULL);
  ptr_tclib->TcLibAuditControllerRequest(controller_id);
  return ODC_DRV_SUCCESS;
}

// Return Updated Contrtoller Pointer
pfc_bool_t ODCModule::update_controller(const key_ctr_t& key_ctr,
                                        const val_ctr_t& val_ctr,
                              unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  pfc_bool_t old_audit_status = ctr_ptr->get_audit_status();
  ctr_ptr->update_ctr(key_ctr, val_ctr);

  // If previous audit status is disable and current audit status is enable,
  // then trigger sudit
  if ((!old_audit_status) && (val_ctr.enable_audit)) {
    pfc_log_debug(" %s Audit status changed, triggering audit" , PFC_FUNCNAME);
    std::string controller_name = ctr_ptr->get_controller_id();
    notify_audit_start_to_tc(controller_name);
  }
  return PFC_TRUE;
}

// Deletes the Controller Pointer
pfc_bool_t ODCModule::delete_controller(unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  if (NULL != ctr_ptr) {
    delete ctr_ptr;
    ctr_ptr = NULL;
    return PFC_TRUE;
  }
  return PFC_FALSE;
}

// Gets the driver command
unc::driver::driver_command* ODCModule::create_driver_command(
    unc_key_type_t key_type) {
  ODC_FUNC_TRACE;
  unc::driver::driver_command* driver_cmd_ptr = NULL;
  switch (key_type) {
    case UNC_KT_VTN: {
      pfc_log_debug("UNC_KT_VTN key type received");
      driver_cmd_ptr = new OdcVtnCommand();
      break;
    }
    case UNC_KT_VBRIDGE: {
      pfc_log_debug("UNC_KT_VBR key type received");
      driver_cmd_ptr = new OdcVbrCommand();
      break;
    }
    case UNC_KT_VBR_IF: {
      pfc_log_debug("UNC_KT_VBRIF key type received");
      driver_cmd_ptr = new OdcVbrIfCommand();
      break;
    }
    default:
      pfc_log_debug("Unknown keytype received : %d", key_type);
      break;
  }
  return driver_cmd_ptr;
}

// Gets the ping interval
uint32_t ODCModule::get_ping_interval() {
  ODC_FUNC_TRACE;
  return ping_interval;
}

// Gets the ping fail retry count
uint32_t ODCModule::get_ping_fail_retry_count() {
  ODC_FUNC_TRACE;
  return PING_RETRY_COUNT;
}

// Is ping need or not
pfc_bool_t ODCModule::is_ping_needed() {
  ODC_FUNC_TRACE;
  return PFC_TRUE;
}

//  ping controller
pfc_bool_t ODCModule::ping_controller(unc::driver::controller* ctr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr != NULL);
  std::string ipaddress = ctr->get_host_address();
  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr, user_name, password);
  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  std::string url = "/controller/nb/v2/vtn/version";
  restjson::RestClient rest_client_obj(ipaddress, url,
                                       odc_port, restjson::HTTP_METHOD_GET);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out, NULL);

  if (NULL == response) {
    pfc_log_error("Error in getting response from controller");
    return PFC_FALSE;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code for ping controller  :%d", resp_code);
  rest_client_obj.clear_http_response();
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("Response code is : %d", resp_code);
    return PFC_FALSE;
  }
  pfc_bool_t new_audit_status = ctr->get_audit_status();
  unc::driver::ConnectionStatus connection_status =
      ctr->get_connection_status();
  if ((connection_status) && (new_audit_status)) {
    std::string controller_name = ctr->get_controller_id();
    notify_audit_start_to_tc(controller_name);
  }
  return PFC_TRUE;
}

// Gets the user name password from the controller pointer else from conf file
void ODCModule::get_username_password(unc::driver::controller* ctr_ptr,
                                      std::string &user_name,
                                      std::string &password) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string user_ctr = ctr_ptr->get_user_name();
  std::string pass_ctr = ctr_ptr->get_pass_word();

  if ((user_ctr.empty()) ||
      (pass_ctr.empty())) {
    read_user_name_password(user_name, password);
  } else {
    user_name = ctr_ptr->get_user_name();
    password = ctr_ptr->get_pass_word();
  }
}

// Reads username password from conf file else take default values
void ODCModule::read_user_name_password(std::string &user_name,
                                        std::string &password) {
  ODC_FUNC_TRACE;
  pfc::core::ModuleConfBlock set_user_password_blk(SET_USER_PASSWORD_BLK);
  if (set_user_password_blk.getBlock() != PFC_CFBLK_INVALID) {
    user_name = set_user_password_blk.getString(
        CONF_USER_NAME, DEFAULT_USER_NAME.c_str());

    password  = set_user_password_blk.getString(
        CONF_PASSWORD, DEFAULT_PASSWORD.c_str());
    pfc_log_debug("%s: Block Handle is Valid,user_name_ %s", PFC_FUNCNAME,
                  user_name.c_str());
  }  else {
    user_name = DEFAULT_USER_NAME;
    password  = DEFAULT_PASSWORD;
    pfc_log_debug("%s: Block Handle is InValid,set default user_name_ %s",
                  PFC_FUNCNAME, user_name.c_str());
  }
}

// Reads set opt params from conf file if it fails reads from defs file
void ODCModule:: read_conf_file(uint32_t &odc_port,
                                uint32_t &connection_time_out,
                                uint32_t &request_time_out) {
  ODC_FUNC_TRACE;
  pfc::core::ModuleConfBlock set_opt_blk(SET_OPT_BLK);
  if (set_opt_blk.getBlock() != PFC_CFBLK_INVALID) {
    odc_port  = set_opt_blk.getUint32(CONF_ODC_PORT, DEFAULT_ODC_PORT);

    request_time_out = set_opt_blk.getUint32(
        CONF_REQ_TIME_OUT, DEFAULT_REQ_TIME_OUT);

    connection_time_out = set_opt_blk.getUint32(
        CONF_CONNECT_TIME_OUT, DEFAULT_CONNECT_TIME_OUT);
    pfc_log_debug("%s: Block Handle is Valid, odc_port_ %d", PFC_FUNCNAME,
                  odc_port);

  } else {
    odc_port   =  DEFAULT_ODC_PORT;
    connection_time_out = DEFAULT_CONNECT_TIME_OUT;
    request_time_out = DEFAULT_REQ_TIME_OUT;
    pfc_log_debug("%s: Block Handle is Invalid,set default Value %d",
                  PFC_FUNCNAME, odc_port);
  }
}

unc::tclib::TcCommonRet ODCModule::HandleVote(unc::driver::controller*) {
  ODC_FUNC_TRACE;
  return unc::tclib::TC_FAILURE;
}

unc::tclib::TcCommonRet ODCModule::HandleCommit(unc::driver::controller*) {
  ODC_FUNC_TRACE;
  return unc::tclib::TC_FAILURE;
}

unc::tclib::TcCommonRet ODCModule::HandleAbort(unc::driver::controller*) {
  ODC_FUNC_TRACE;
  return unc::tclib::TC_FAILURE;
}

void ODCModule::read_conf_file() {
  ODC_FUNC_TRACE;
  pfc::core::ModuleConfBlock drv_block(drv_ping_conf_blk);
  if (drv_block.getBlock() != PFC_CFBLK_INVALID) {
    ping_interval = drv_block.getUint32("odcdrv_ping_interval",
                                        ping_default_interval);
    pfc_log_debug("%s: Block Handle is Valid,Ping Timeout %d", PFC_FUNCNAME,
                  ping_interval);
  } else {
    ping_interval = ping_default_interval;
    pfc_log_debug("%s: Block Handle is Invalid,set default Value %d",
                  PFC_FUNCNAME, ping_interval);
  }
  pfc_log_debug("%s: Exiting function", PFC_FUNCNAME);
}
}  // namespace odcdriver
}  // namespace unc
PFC_MODULE_IPC_DECL(unc::odcdriver::ODCModule, 0);
