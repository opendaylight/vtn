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

#include <odc_vbr.hh>

namespace unc {
namespace odcdriver {

// Constructor
ODCVBRCommand::ODCVBRCommand()
: age_interval_("600") {
}

// Destructor
ODCVBRCommand::~ODCVBRCommand() {
}

// Create Command and send Request to Controller
drv_resp_code_t ODCVBRCommand::create_cmd(key_vbr_t& key_vbr,
    val_vbr_t& val_vbr, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);

  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  pfc_log_info("Controller Name in Create vbr %s", ipaddress.c_str());

  std::string vbr_url = get_vbr_url(key_vbr);
  if (0 == strlen(vbr_url.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  pfc_log_debug("Return value for SetUserName Password %d", retval);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(vbr_url,
                                    unc::restjson::HTTP_METHOD_POST);
  pfc_log_debug("Return value for Request Header %d", retval);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  const char* request = create_request_body(val_vbr);
  if (NULL != request) {
    retval = rest_util_obj.set_request_body(request);
    if (ODC_DRV_FAILURE == retval) {
      return DRVAPI_RESPONSE_FAILURE;
    }
  } else {
    pfc_log_info(" No Request Body Formed");
  }
  pfc_log_debug("Request Body for create vbr : %s", request);

  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  pfc_log_info("resp_code for create vbr is %d", resp);

  if (RESP_CREATED != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

//  Command to update vtn  and Send request to Controller
drv_resp_code_t ODCVBRCommand::update_cmd(key_vbr_t& key_vbr,
    val_vbr_t& val_vbr, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);

  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  std::string vbr_url = get_vbr_url(key_vbr);
  if (0 == strlen(vbr_url.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  pfc_log_debug("Return value for SetUserName Password %d", retval);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  retval = rest_util_obj.create_request_header(vbr_url,
                                              unc::restjson::HTTP_METHOD_PUT);
  pfc_log_debug("Return value for Request Header %d", retval);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  const char* request = create_request_body(val_vbr);
  if (NULL != request) {
    retval = rest_util_obj.set_request_body(request);
    if (ODC_DRV_FAILURE == retval) {
      return DRVAPI_RESPONSE_FAILURE;
    }
  } else {
    pfc_log_info(" No Request Body Formed");
  }

  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  if (RESP_OK != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_info("resp_code for update vbr is %d", resp);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Delete Request sned to the Controller
drv_resp_code_t ODCVBRCommand::delete_cmd(key_vbr_t& key_vbr,
    val_vbr_t& val_vbr, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);

  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  std::string vbr_url = get_vbr_url(key_vbr);
  if (0 == strlen(vbr_url.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  pfc_log_debug("Return value for SetUserName Password %d", retval);

  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(vbr_url,
      unc::restjson::HTTP_METHOD_DELETE);
  pfc_log_debug("Return value for Request Header %d", retval);

  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  uint32_t resp = rest_util_obj.send_request_and_get_response_code();

  if (RESP_OK != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Creates Request Body
const char* ODCVBRCommand::create_request_body(val_vbr_t& val_vbr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  unc::restjson::JsonBuildParse json_obj;
  json_object *jobj = json_obj.create_json_obj();
  char* description = reinterpret_cast<char*>(val_vbr.vbr_description);
  int ret_val = 1;

  if (0 != strlen(description)) {
    ret_val = json_obj.build(jobj, "description", description);
    if (ret_val) {
      return NULL;
    }
  }
  ret_val = json_obj.build(jobj, "ageInterval", age_interval_);
  if (ret_val) {
    return NULL;
  }
  const char* req_body = json_obj.json_obj_to_json_string(jobj);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return req_body;
}

// validates the operation
drv_resp_code_t ODCVBRCommand::validate_op(key_vbr_t& key_vbr,
                  val_vbr_t& val_vbr, unc::driver::controller* ctr,
                  uint32_t operation) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  drv_resp_code_t resp_code = DRVAPI_RESPONSE_FAILURE;
  switch (operation) {
  case UNC_OP_CREATE:
    resp_code = validate_create_vbr(key_vbr, ctr);
    break;
  case UNC_OP_UPDATE:
    resp_code = validate_update_vbr(key_vbr, ctr);
    break;
  case UNC_OP_DELETE:
    resp_code = validate_delete_vbr(key_vbr, ctr);
    break;
  default:
    pfc_log_info("Unknown operation received %d", operation);
    break;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return resp_code;
}

// Validates create vbr
drv_resp_code_t ODCVBRCommand::validate_create_vbr(key_vbr_t& key_vbr,
    unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  uint32_t resp_code = ODC_DRV_FAILURE;
  resp_code = is_vtn_exists_in_controller(key_vbr, ctr);
  if (RESP_NOT_FOUND == resp_code) {
      return DRVAPI_RESPONSE_FAILURE;
  }
  if (RESP_OK == resp_code) {
    resp_code = is_vbr_exists_in_controller(key_vbr, ctr);
    if (RESP_NOT_FOUND == resp_code) {
      return DRVAPI_RESPONSE_SUCCESS;
    }
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_FAILURE;
}

drv_resp_code_t ODCVBRCommand::validate_delete_vbr(key_vbr_t& key_vbr,
     unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  uint32_t vbr_resp_code = ODC_DRV_FAILURE;

  vbr_resp_code = is_vbr_exists_in_controller(key_vbr, ctr);
  if (RESP_OK == vbr_resp_code) {
    pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_SUCCESS;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_FAILURE;
}

// Validates update vbr
drv_resp_code_t ODCVBRCommand::validate_update_vbr(key_vbr_t& key_vbr,
    unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  uint32_t vbr_resp_code = ODC_DRV_FAILURE;
  vbr_resp_code = is_vbr_exists_in_controller(key_vbr, ctr);
  if (RESP_OK == vbr_resp_code) {
    pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_SUCCESS;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_FAILURE;
}

// Checks the exists in controlle ror not
uint32_t ODCVBRCommand::is_vtn_exists_in_controller(key_vbr_t& key_vbr,
    unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  char* vtnname = NULL;
  vtnname = reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    return ODC_DRV_FAILURE;
  }
  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  url.append(vtnname);
  uint32_t response_code = get_controller_response(url, ctr);
  pfc_log_debug("Response code from Controller is : %d ", response_code);
  return response_code;
}

// Checks vbr exists in controller or not
uint32_t ODCVBRCommand::is_vbr_exists_in_controller(key_vbr_t& key_vbr,
    unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  std::string vbr_url = get_vbr_url(key_vbr);
  if (0 == strlen(vbr_url.c_str())) {
    return ODC_DRV_FAILURE;
  }
  uint32_t response_code = get_controller_response(vbr_url, ctr);
  pfc_log_debug("Response code from Controller is : %d ", response_code);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return response_code;
}

// Gets the Controller Response
uint32_t ODCVBRCommand::get_controller_response(std::string url,
    unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);

  PFC_ASSERT(ctr != NULL);
  std::string ipaddress = ctr->get_host_address();
    if (0 == strlen(ipaddress.c_str())) {
    return ODC_DRV_FAILURE;
  }
  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  pfc_log_debug("Return value for SetUserName Password %d", retval);
  if (ODC_DRV_FAILURE == retval) {
    return retval;
  }
  retval = rest_util_obj.create_request_header(url,
                                      unc::restjson::HTTP_METHOD_GET);
  if (ODC_DRV_FAILURE == retval) {
    return retval;
  }
  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  pfc_log_debug("Response code from Controller is : %d ", resp);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return resp;
}

std::string ODCVBRCommand::get_vbr_url(key_vbr_t& key_vbr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  char* vtnname = NULL;
  char* vbrname = NULL;

  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  vtnname = reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    return "";
  }
  url.append(vtnname);
  vbrname = reinterpret_cast<char*>(key_vbr.vbridge_name);
  if (0 == strlen(vbrname)) {
    return "";
  }
  url.append("/vbridges/");
  url.append(vbrname);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return url;
}
}
}
