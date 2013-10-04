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

#include <odc_vtn.hh>

namespace odc {
namespace driver {

// Constructor
ODLVTNCommand::ODLVTNCommand()
: idle_timeout_("300"),
  hard_timeout_("0") {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
}

// Destructor
ODLVTNCommand::~ODLVTNCommand() {
}

// validates the operation
drv_resp_code_t ODLVTNCommand::validate_op(key_vtn_t& key_vtn,
    val_vtn_t& val_vtn, unc::driver::controller* ctr,
    uint32_t op) {
  PFC_ASSERT(ctr != NULL);
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  drv_resp_code_t resp_code = DRVAPI_RESPONSE_FAILURE;

  pfc_log_debug("operation received %d", op);
  switch (op) {
  case UNC_OP_CREATE:
    resp_code = validate_create_vtn(key_vtn, ctr);
    break;
  case UNC_OP_UPDATE:
    resp_code = validate_update_vtn(key_vtn, ctr);
    break;
  case UNC_OP_DELETE:
    resp_code = validate_delete_vtn(key_vtn, ctr);
    break;
  default:
    pfc_log_debug("Unknown operation %d", op);
    break;
  }
  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
  return resp_code;
}

// Validates create vtn
drv_resp_code_t ODLVTNCommand::validate_create_vtn(key_vtn_t& key_vtn,
    unc::driver::controller* ctr) {

  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  uint32_t resp_code = is_vtn_exists_in_controller(key_vtn, ctr);
  pfc_log_debug("Response code for is_vtn_exists_in_controller %d", resp_code);

  if (RESP_NOT_FOUND == resp_code) {
    pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_SUCCESS;
  } else {
    pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
}

// Validates delete vtn
drv_resp_code_t ODLVTNCommand::validate_delete_vtn(key_vtn_t& key_vtn,
     unc::driver::controller* ctr) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  uint32_t resp_code = is_vtn_exists_in_controller(key_vtn, ctr);
  pfc_log_debug("Response code for is_vtn_exists_in_controller %d", resp_code);
  if (RESP_OK == resp_code) {
    pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_SUCCESS;
  } else {
    pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
}

// Validates update vtn
drv_resp_code_t ODLVTNCommand::validate_update_vtn(key_vtn_t& key_vtn,
     unc::driver::controller* ctr) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  uint32_t resp_code = is_vtn_exists_in_controller(key_vtn, ctr);
  pfc_log_debug("Response code for is_vtn_exists_in_controller %d", resp_code);

  if (RESP_OK == resp_code) {
    pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_SUCCESS;
  } else {
    pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
}

// Checks if vtn exist in Controller or not
uint32_t ODLVTNCommand::is_vtn_exists_in_controller(key_vtn_t& key_vtn,
     unc::driver::controller* ctr) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  char* vtnname = NULL;
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    return ODC_DRV_FAILURE;
  }
  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  url.append(vtnname);
  uint32_t response = get_controller_response(url, ctr);
  pfc_log_debug("Response code from Controller is : %d ", response);
  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
  return response;
}

// Gets the Controller Response code
uint32_t ODLVTNCommand::get_controller_response(std::string url,
    unc::driver::controller* ctr) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  ODCController *conn = reinterpret_cast<ODCController*> (ctr);
  PFC_ASSERT(NULL != conn);
  std::string ipaddress = conn->get_host_address();
  pfc_log_debug("ip address of controller is %s", ipaddress.c_str());
  if (0 == strlen(ipaddress.c_str())) {
    return ODC_DRV_FAILURE;
  }
  restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  pfc_log_debug("Return value for SetUserName Password %d", retval);

  if (ODC_DRV_FAILURE == retval) {
    return retval;
  }
  retval = rest_util_obj.create_request_header(url, restjson::HTTP_METHOD_GET);
  if (ODC_DRV_FAILURE == retval) {
    return retval;
  }

  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  pfc_log_debug("Response code from Controller is : %d ", resp);
  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
  return resp;
}

// Create Command and send Request to Controller
drv_resp_code_t ODLVTNCommand::create_cmd(key_vtn_t& key_vtn,
    val_vtn_t& val_vtn, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  char* vtnname = NULL;
  PFC_ASSERT(ctr_ptr != NULL);
  ODCController *odc_ctr = reinterpret_cast<ODCController*> (ctr_ptr);
  PFC_ASSERT(odc_ctr != NULL);
  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  std::string ipaddress = odc_ctr->get_host_address();
  pfc_log_debug("ip address of controller is %s", ipaddress.c_str());
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  url.append(vtnname);

  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  pfc_log_debug("Return value for SetUserName Password %d", retval);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  retval = rest_util_obj.create_request_header(url, restjson::HTTP_METHOD_POST);
  pfc_log_debug("Return value for Request Header %d", retval);

  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  const char* request = create_request_body(val_vtn);
  if (NULL != request) {
    pfc_log_debug("Request Body for create vtn : %s", request);
    retval = rest_util_obj.set_request_body(request);
    pfc_log_debug("Return value to set Request Body %d", retval);
    if (ODC_DRV_FAILURE == retval) {
      return DRVAPI_RESPONSE_FAILURE;
    }
  } else {
    pfc_log_info(" No Request body formed");
  }

  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  pfc_log_info("response code returned in createvtn is %d", resp);
  if (RESP_CREATED != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Creates Request Body
const char* ODLVTNCommand::create_request_body(val_vtn_t& val_vtn) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  restjson::JsonBuildParse json_obj;
  char* description = reinterpret_cast<char*>(val_vtn.description);
  json_object *jobj = json_obj.create_json_obj();

  int ret_val = 1;
  pfc_log_info("%s description", description);
  if (0 != strlen(description)) {
    ret_val = json_obj.build(jobj, "@description", description);
    if (ret_val) {
      return NULL;
    }
  }
  ret_val = json_obj.build(jobj, "@idleTimeout", idle_timeout_);
  if (ret_val) {
    return NULL;
  }
  ret_val = json_obj.build(jobj, "@hardTimeout", hard_timeout_);
  if (ret_val) {
    return NULL;
  }

  const char* req_body = json_obj.json_obj_to_json_string(jobj);
  pfc_log_info("%s json request ", req_body);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return req_body;
}

//  Command to update vtn  and Send request to Controller
drv_resp_code_t ODLVTNCommand::update_cmd(key_vtn_t& key_vtn,
    val_vtn_t& val_vtn, unc::driver::controller *ctr_ptr) {

  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  char* vtnname = NULL;
  PFC_ASSERT(ctr_ptr != NULL);
  ODCController *odc_ctr = reinterpret_cast<ODCController*> (ctr_ptr);
  PFC_ASSERT(odc_ctr != NULL);
  std::string url = "/controller/nb/v2/vtn/default/vtns/";

  std::string ipaddress = odc_ctr->get_host_address();
  pfc_log_debug("ip address of controller is %s", ipaddress.c_str());
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  url.append(vtnname);
  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  pfc_log_debug("Return value for SetUserName Password %d", retval);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  retval = rest_util_obj.create_request_header(url, restjson::HTTP_METHOD_PUT);
  pfc_log_debug("Return value for Request Header %d", retval);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  const char* request = create_request_body(val_vtn);
  if (NULL != request) {
    pfc_log_debug("Request Body for create vtn : %s", request);
    retval = rest_util_obj.set_request_body(request);
    pfc_log_debug("Return value to set Request Body %d", retval);
    if (ODC_DRV_FAILURE == retval) {
      return DRVAPI_RESPONSE_FAILURE;
    }
  } else {
    pfc_log_info(" No Request body formed");
  }
  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  if (RESP_OK != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_info("response code returned in updatevtn is %d", resp);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Delete Request send to the Contoller
drv_resp_code_t ODLVTNCommand::delete_cmd(key_vtn_t& key_vtn,
    val_vtn_t& val_vtn, unc::driver::controller *ctr_ptr) {

  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  char* vtnname = NULL;
  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  PFC_ASSERT(ctr_ptr != NULL);
  ODCController *odc_ctr = reinterpret_cast<ODCController*> (ctr_ptr);
  PFC_ASSERT(odc_ctr != NULL);
  std::string ipaddress = odc_ctr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  url.append(vtnname);
  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  retval = rest_util_obj.create_request_header(url,
      restjson::HTTP_METHOD_DELETE);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  if (RESP_OK != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_info("response code returned in delete vtn is %d", resp);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}
}
}
