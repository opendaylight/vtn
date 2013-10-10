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

#include <odc_vbrif.hh>

namespace unc {
namespace odcdriver {
// Constructor
ODCVBRIfCommand::ODCVBRIfCommand() {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
}

// Destructor
ODCVBRIfCommand::~ODCVBRIfCommand() {
}

// Creates Request Body for Port Map
const char* ODCVBRIfCommand::create_request_body_port_map(
    pfcdrv_val_vbr_if_t& vbrif_val) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  std::string switch_id, port_name;
  std::string logical_port_id =
      reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id);

  if (logical_port_id.length() > 0) {
    pfc_log_info("logical_port_id is %s", logical_port_id.c_str());
    if (logical_port_id.compare(0, 3, "PP-") == 0) {
      switch_id = logical_port_id.substr(3, 23);
      port_name = logical_port_id.substr(27);
    }
  } else {
    pfc_log_info("logical_port_id is empty");
    return NULL;
  }

  unc::restjson::JsonBuildParse json_obj;
  json_object *jobj_parent = json_obj.create_json_obj();
  json_object *jobj_node = json_obj.create_json_obj();
  json_object *jobj_port = json_obj.create_json_obj();

  int ret_val = json_obj.build(jobj_node, "type", "OF");
  if (ret_val) {
    return NULL;
  }
  if (0 != strlen(switch_id.c_str())) {
    ret_val = json_obj.build(jobj_node, "id", switch_id);
    if (ret_val) {
      pfc_log_debug("Failed in framing json request body");
      return NULL;
    }
  }
  ret_val = json_obj.build(jobj_parent, "node", jobj_node);
  if (ret_val) {
    pfc_log_debug("Failed in framing json request body");
    return NULL;
  }

  if (0 != strlen(port_name.c_str())) {
    ret_val = json_obj.build(jobj_port, "name", port_name);
    if (ret_val) {
      pfc_log_debug("Failed in framing json request body");
      return NULL;
    }
  }
  ret_val = json_obj.build(jobj_parent, "port", jobj_port);
  if (ret_val) {
    pfc_log_debug("Failed in framing json request body");
    return NULL;
  }

  const char* req_body = json_obj.json_obj_to_json_string(jobj_parent);
  pfc_log_info("%s json request ", req_body);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return req_body;
}

// Creates command for portmap
uint32_t ODCVBRIfCommand::create_cmd_port_map(key_vbr_if_t& vbrif_key,
    pfcdrv_val_vbr_if_t& vbrif_val, unc::driver::controller *ctr_ptr) {

  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return ODC_DRV_FAILURE;
  }
  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);

  std::string vbrif_url = get_vbrif_url(vbrif_key);
  if (0 == strlen(vbrif_url.c_str())) {
    return ODC_DRV_FAILURE;
  }
  vbrif_url.append("/portmap");

  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(vbrif_url,
                                      unc::restjson::HTTP_METHOD_POST);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  const char* request = create_request_body_port_map(vbrif_val);
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
    return ODC_DRV_FAILURE;
  }
  pfc_log_info("response code returned in create_cmd_port_map is %d", resp);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return ODC_DRV_SUCCESS;
}

std::string ODCVBRIfCommand::get_vbrif_url(key_vbr_if_t& vbrif_key) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  char* vtnname = NULL;
  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  vtnname = reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    return "";
  }
  url.append(vtnname);

  char* vbrname = reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    return "";
  }
  url.append("/vbridges/");
  url.append(vbrname);

  char* intfname = reinterpret_cast<char*>(vbrif_key.if_name);
  if (0 == strlen(intfname)) {
    return "";
  }
  url.append("/interfaces/");
  url.append(intfname);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return url;
}

// Updates the Portmap Command and send it to conttoller
uint32_t ODCVBRIfCommand::update_cmd_port_map(key_vbr_if_t& vbrif_key,
    pfcdrv_val_vbr_if_t& vbrif_val, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return ODC_DRV_FAILURE;
  }

  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  std::string vbrif_url = get_vbrif_url(vbrif_key);
  if (0 == strlen(vbrif_url.c_str())) {
    return ODC_DRV_FAILURE;
  }
  vbrif_url.append("/portmap");

  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(vbrif_url,
                                          unc::restjson::HTTP_METHOD_PUT);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  const char* request = create_request_body_port_map(vbrif_val);
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
    return ODC_DRV_FAILURE;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return ODC_DRV_SUCCESS;
}


// Delete Command for Port map created and send to Controller
uint32_t ODCVBRIfCommand::delete_cmd_port_map(key_vbr_if_t& vbrif_key,
    pfcdrv_val_vbr_if_t& vbrif_val, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return ODC_DRV_FAILURE;
  }
  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  std::string vbrif_url = get_vbrif_url(vbrif_key);
  if (0 == strlen(vbrif_url.c_str())) {
    return ODC_DRV_FAILURE;
  }
  vbrif_url.append("/portmap");

  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(vbrif_url,
      unc::restjson::HTTP_METHOD_DELETE);

  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t resp = rest_util_obj.send_request_and_get_response_code();

  if (RESP_OK != resp) {
    return ODC_DRV_FAILURE;
  }
  pfc_log_info("response code returned in delete_cmd_port_map is %d", resp);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return ODC_DRV_SUCCESS;
}

// Create Command for vbrif
drv_resp_code_t ODCVBRIfCommand::create_cmd(key_vbr_if_t& vbrif_key,
    pfcdrv_val_vbr_if_t& val, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  pfc_log_info("Controller Name in Create vbr_if  %s", ipaddress.c_str());
  std::string vbrif_url = get_vbrif_url(vbrif_key);
  if (0 == strlen(vbrif_url.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(vbrif_url,
                              unc::restjson::HTTP_METHOD_POST);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  const char* request = create_request_body(val);
  if (NULL != request) {
    retval = rest_util_obj.set_request_body(request);
    if (ODC_DRV_FAILURE == retval) {
      return DRVAPI_RESPONSE_FAILURE;
    }
  } else {
    pfc_log_info(" No Request Body Formed");
  }

  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  if (RESP_CREATED != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  // VBR_IF successful...Check for PortMap
  if (val.val_vbrif.valid[2] == UNC_VF_VALID) {
    retval = create_cmd_port_map(vbrif_key, val, ctr_ptr);
    if (retval != ODC_DRV_SUCCESS) {
      return DRVAPI_RESPONSE_FAILURE;
    }
  } else {
    pfc_log_info("NO V-external");
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Update command for vbrif command
drv_resp_code_t ODCVBRIfCommand::update_cmd(key_vbr_if_t& vbrif_key,
    pfcdrv_val_vbr_if_t& val, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  std::string vbrif_url = get_vbrif_url(vbrif_key);
  if (0 == strlen(vbrif_url.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(vbrif_url,
                                          unc::restjson::HTTP_METHOD_PUT);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  const char* request = create_request_body(val);
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

  // VBR_IF successful...Check for PortMap
  if ((val.val_vbrif.valid[2] == UNC_VF_VALID)
      && (val.valid[0] == UNC_VF_VALID)) {
    retval = update_cmd_port_map(vbrif_key, val, ctr_ptr);
    if (retval != ODC_DRV_SUCCESS) {
      return DRVAPI_RESPONSE_FAILURE;
    }
  } else if ((val.val_vbrif.valid[2] == UNC_VF_VALID_NO_VALUE)
      && (val.valid[0] == UNC_VF_VALID)) {
    retval = delete_cmd_port_map(vbrif_key, val, ctr_ptr);
    if (retval != ODC_DRV_SUCCESS) {
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Delete Command for vbr if
drv_resp_code_t ODCVBRIfCommand::delete_cmd(key_vbr_if_t& vbrif_key,
    pfcdrv_val_vbr_if_t& val, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  std::string vbrif_url = get_vbrif_url(vbrif_key);
  if (0 == strlen(vbrif_url.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  uint32_t retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(vbrif_url,
                                            unc::restjson::HTTP_METHOD_DELETE);
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

// Creates request body for vbr if
const char* ODCVBRIfCommand::create_request_body(pfcdrv_val_vbr_if_t& val_vtn) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  unc::restjson::JsonBuildParse json_obj;
  json_object *jobj = json_obj.create_json_obj();
  char* description = reinterpret_cast<char*>(val_vtn.val_vbrif.description);
  int ret_val = 1;
  pfc_log_info("%s description", description);
  if (0 != strlen(description)) {
    ret_val = json_obj.build(jobj, "description", description);
    if (ret_val) {
      return NULL;
    }
  }
  const char* req_body = json_obj.json_obj_to_json_string(jobj);
  pfc_log_info("%s json request ", req_body);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return req_body;
}

// Validates the operation
drv_resp_code_t ODCVBRIfCommand::validate_op(key_vbr_if_t& key_vbr_if,
    pfcdrv_val_vbr_if_t& val, unc::driver::controller* ctr,
    uint32_t operation) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  drv_resp_code_t resp_code = DRVAPI_RESPONSE_FAILURE;
  PFC_ASSERT(ctr != NULL);
  switch (operation) {
  case UNC_OP_CREATE:
    resp_code = validate_create_vbrif(key_vbr_if, ctr);
    break;
  case UNC_OP_UPDATE:
    resp_code = validate_update_vbrIf(key_vbr_if, ctr);
    break;
  case UNC_OP_DELETE:
    resp_code = validate_delete_vbrif(key_vbr_if, ctr);
    break;
  default:
    pfc_log_debug("Unknown operation %d", operation);
    break;
  }
  return resp_code;
}

// Validate the create vbr if command
drv_resp_code_t ODCVBRIfCommand::validate_create_vbrif(key_vbr_if_t& key_vbr_if,
    unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  uint32_t resp_code = ODC_DRV_FAILURE;
  resp_code = is_vtn_exists_in_controller(key_vbr_if, ctr);
  if (RESP_NOT_FOUND == resp_code) {
      return DRVAPI_RESPONSE_FAILURE;
  }
  if (RESP_OK == resp_code) {
    resp_code = is_vbr_exists_in_controller(key_vbr_if, ctr);
    if (RESP_NOT_FOUND == resp_code) {
        return DRVAPI_RESPONSE_FAILURE;
    }
    if (RESP_OK == resp_code) {
      resp_code = is_vbrif_exists_in_controller(key_vbr_if, ctr);
      if (RESP_NOT_FOUND == resp_code) {
        return DRVAPI_RESPONSE_SUCCESS;
      }
    }
  }
  return DRVAPI_RESPONSE_FAILURE;
}

// Validates delete vbr if operatopn
drv_resp_code_t ODCVBRIfCommand::validate_delete_vbrif(key_vbr_if_t& key_vbr_if,
    unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  uint32_t resp_code = ODC_DRV_FAILURE;
  resp_code = is_vbrif_exists_in_controller(key_vbr_if, ctr);
  if (RESP_OK == resp_code) {
    return DRVAPI_RESPONSE_SUCCESS;
  }
  return DRVAPI_RESPONSE_FAILURE;
}

// Validates the update command
drv_resp_code_t ODCVBRIfCommand::validate_update_vbrIf(key_vbr_if_t& key_vbr_if,
    unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  uint32_t resp_code = ODC_DRV_FAILURE;
  resp_code = is_vbrif_exists_in_controller(key_vbr_if, ctr);
  if (RESP_OK == resp_code) {
    return DRVAPI_RESPONSE_SUCCESS;
  }
  return DRVAPI_RESPONSE_FAILURE;
}

// Check if vtn exist in controller or not
uint32_t ODCVBRIfCommand::is_vtn_exists_in_controller(key_vbr_if_t& key_vbr_if,
    unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  char* vtnname = NULL;
  vtnname = reinterpret_cast<char*>(key_vbr_if.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    return ODC_DRV_FAILURE;
  }
  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  url.append(vtnname);
  uint32_t response_code = get_controller_response(url, ctr);
  pfc_log_debug("Response code from Controller is : %d ", response_code);
  return response_code;
}

// Checks if vbr if exists in controller or not
uint32_t ODCVBRIfCommand::is_vbrif_exists_in_controller(
                  key_vbr_if_t& key_vbr_if, unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  std::string vbrif_url = get_vbrif_url(key_vbr_if);
  if (0 == strlen(vbrif_url.c_str())) {
    return ODC_DRV_FAILURE;
  }
  uint32_t response_code = get_controller_response(vbrif_url, ctr);
  pfc_log_debug("Response code from Controller is : %d ", response_code);
  return response_code;
}

// Checks the vbr exist in controller or not
uint32_t ODCVBRIfCommand::is_vbr_exists_in_controller(key_vbr_if_t& key_vbr_if,
    unc::driver::controller* ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  char* vtnname = NULL;
  char* vbrname = NULL;

  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  vtnname = reinterpret_cast<char*>(key_vbr_if.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    return ODC_DRV_FAILURE;
  }
  url.append(vtnname);
  vbrname = reinterpret_cast<char*>(key_vbr_if.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    return ODC_DRV_FAILURE;
  }
  url.append("/vbridges/");
  url.append(vbrname);
  uint32_t response_code = get_controller_response(url, ctr);
  pfc_log_debug("Response code from Controller is : %d ", response_code);
  return response_code;
}

// Gets the controller respnse
uint32_t ODCVBRIfCommand::get_controller_response(std::string url,
    unc::driver::controller* ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);

  PFC_ASSERT(ctr_ptr != NULL);
  std::string ipaddress = ctr_ptr->get_host_address();
  pfc_log_debug("ip address of controller is %s", ipaddress.c_str());

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
  return resp;
}
}
}

