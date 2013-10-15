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

namespace unc {
namespace odcdriver {

// Constructor
ODCVTNCommand::ODCVTNCommand()
: idle_timeout_("300"),
  hard_timeout_("0") {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
}

// Destructor
ODCVTNCommand::~ODCVTNCommand() {
}

// validates the operation
drv_resp_code_t ODCVTNCommand::validate_op(key_vtn_t& key_vtn,
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
drv_resp_code_t ODCVTNCommand::validate_create_vtn(const key_vtn_t& key_vtn,
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
drv_resp_code_t ODCVTNCommand::validate_delete_vtn(const key_vtn_t& key_vtn,
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
drv_resp_code_t ODCVTNCommand::validate_update_vtn(const key_vtn_t& key_vtn,
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
uint32_t ODCVTNCommand::is_vtn_exists_in_controller(const key_vtn_t& key_vtn,
                                              unc::driver::controller* ctr) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  const char* vtnname = NULL;
  vtnname = reinterpret_cast<const char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    return ODC_DRV_FAILURE;
  }
  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  url.append(vtnname);
  uint32_t response = get_controller_response_code(url, ctr,
                                         unc::restjson::HTTP_METHOD_GET, NULL);
  pfc_log_debug("Response code from Controller is for vtn : %d ", response);
  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
  return response;
}

uint32_t ODCVTNCommand::get_controller_response_code(std::string url,
                                            unc::driver::controller* ctr,
                                            unc::restjson::HttpMethod method,
                                             const char* request_body) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  PFC_ASSERT(ctr != NULL);
  std::string ipaddress = ctr->get_host_address();
  pfc_log_debug("ip address of controller is %s", ipaddress.c_str());
  if (0 == strlen(ipaddress.c_str())) {
    return ODC_DRV_FAILURE;
  }
  if (( method != unc::restjson::HTTP_METHOD_POST ) &&
      (method != unc::restjson::HTTP_METHOD_PUT) &&
      (method != unc::restjson::HTTP_METHOD_DELETE) &&
      (method != unc::restjson::HTTP_METHOD_GET)) {
    pfc_log_debug("Invalid Method : %d", method);
    return ODC_DRV_FAILURE;
  }

  std::string username_ctr = ctr->get_user_name();
  std::string password_ctr = ctr->get_pass_word();

  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  uint32_t retval = ODC_DRV_FAILURE;
  if ((0 == strlen(username_ctr.c_str()))
      || (0 == strlen(password_ctr.c_str()))) {
    retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  } else {
    retval = rest_util_obj.set_username_password(username_ctr, password_ctr);
  }
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.set_timeout(CONNECTION_TIME_OUT, REQUEST_TIME_OUT);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(url, method);
  if (ODC_DRV_FAILURE == retval) {
    pfc_log_debug("Failure in create request header : %d", retval);
    return retval;
  }
  if ((method == unc::restjson::HTTP_METHOD_POST)
      || (method == unc::restjson::HTTP_METHOD_PUT)) {
    if (NULL != request_body) {
      pfc_log_debug("Request Body  : %s", request_body);
      retval = rest_util_obj.set_request_body(request_body);
      if (ODC_DRV_FAILURE == retval) {
        pfc_log_debug("Failure in set Request Body %d", retval);
        return DRVAPI_RESPONSE_FAILURE;
      }
    }
  }
  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  pfc_log_debug("Response code from Controller is : %d ", resp);
  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
  return resp;
}

// Create Command and send Request to Controller
drv_resp_code_t ODCVTNCommand::create_cmd(key_vtn_t& key_vtn,
                        val_vtn_t& val_vtn, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  char* vtnname = NULL;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  url.append(vtnname);
  const char* request = create_request_body(val_vtn);
  pfc_log_debug("Request body formed %s", request);
  uint32_t resp = get_controller_response_code(url, ctr_ptr,
                                   unc::restjson::HTTP_METHOD_POST, request);
  pfc_log_debug("response code returned in create vtn is %d", resp);
  if (RESP_CREATED != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Creates Request Body
const char* ODCVTNCommand::create_request_body(const val_vtn_t& val_vtn) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  unc::restjson::JsonBuildParse json_obj;
  const char* description = reinterpret_cast<const char*>(val_vtn.description);
  json_object *jobj = json_obj.create_json_obj();
  int ret_val = 1;
  pfc_log_debug("%s description", description);
  if (0 != strlen(description)) {
    ret_val = json_obj.build(jobj, "description", description);
    if (ret_val) {
      return NULL;
    }
  }
  ret_val = json_obj.build(jobj, "idleTimeout", idle_timeout_);
  if (ret_val) {
    return NULL;
  }
  ret_val = json_obj.build(jobj, "hardTimeout", hard_timeout_);
  if (ret_val) {
    return NULL;
  }
  const char* req_body = json_obj.json_obj_to_json_string(jobj);
  pfc_log_debug("%s json request ", req_body);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return req_body;
}

//  Command to update vtn  and Send request to Controller
drv_resp_code_t ODCVTNCommand::update_cmd(key_vtn_t& key_vtn,
                     val_vtn_t& val_vtn, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  char* vtnname = NULL;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  url.append(vtnname);
  const char* request = create_request_body(val_vtn);
  pfc_log_debug(" Request body formed %s", request);
  uint32_t resp = get_controller_response_code(url, ctr_ptr,
                              unc::restjson::HTTP_METHOD_PUT, request);
  if (RESP_OK != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("response code returned in updatevtn is %d", resp);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);

  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

drv_resp_code_t ODCVTNCommand::get_vtn_child(string vtn_name,
                    unc::driver::controller* ctr,
                    vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  if (0 == strlen(vtn_name.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  string url = "/controller/nb/v2/vtn/default/vtns/";
  url.append(vtn_name);
  url.append("/vbridges");
  pfc_log_debug("%s Entering function, url:%s", PFC_FUNCNAME, url.c_str());

  PFC_ASSERT(NULL != ctr);
  string ipaddress = ctr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  std::string username_ctr = ctr->get_user_name();
  std::string password_ctr = ctr->get_pass_word();

  uint32_t retval = ODC_DRV_FAILURE;
  if ((0 == strlen(username_ctr.c_str()))
      || (0 == strlen(password_ctr.c_str()))) {
    retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  } else {
    retval = rest_util_obj.set_username_password(username_ctr, password_ctr);
  }
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.set_timeout(CONNECTION_TIME_OUT, REQUEST_TIME_OUT);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(url, restjson::HTTP_METHOD_GET);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  if (RESP_OK != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  restjson::HttpContent_t* content =  rest_util_obj.get_response_body();
  if (content == NULL) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  char *data = content->memory;
  return parse_vbr_resp_data(data, vtn_name, ctr, cfgnode_vector);
}

// Gets the Controller Response code
drv_resp_code_t ODCVTNCommand::read_all(unc::driver::controller* ctr,
                      vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  PFC_ASSERT(NULL != ctr);
  string ipaddress = ctr->get_host_address();

  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  std::string username_ctr = ctr->get_user_name();
  std::string password_ctr = ctr->get_pass_word();

  uint32_t retval = ODC_DRV_FAILURE;
  if ((0 == strlen(username_ctr.c_str()))
      || (0 == strlen(password_ctr.c_str()))) {
    retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  } else {
    retval = rest_util_obj.set_username_password(username_ctr, password_ctr);
  }
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.set_timeout(CONNECTION_TIME_OUT, REQUEST_TIME_OUT);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  retval = rest_util_obj.create_request_header(READ_VTN_URL,
                                               restjson::HTTP_METHOD_GET);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  pfc_log_debug("%d resp ", resp);
  if (RESP_OK != resp) {
    pfc_log_debug("Error in response code");
    return DRVAPI_RESPONSE_FAILURE;
  }
  restjson::HttpContent_t* content =  rest_util_obj.get_response_body();
  if (content == NULL) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  char *data = content->memory;
  pfc_log_debug("data in ODCVTN %s", data);
  if (0 == strlen(data)) {
    pfc_log_debug(" %s No vtn present in controller ", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_NO_SUCH_INSTANCE;
  }
  drv_resp_code_t ret_val =  parse_resp_data(data, cfgnode_vector);
  pfc_log_debug("read_all_--  size, %d",
                static_cast<int>(cfgnode_vector.size()));
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return ret_val;
}

drv_resp_code_t ODCVTNCommand::parse_vbr_resp_data(char *data,
                    std::string vtn_name,  unc::driver::controller* ctr,
                    vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_vbr, 0, sizeof(key_vbr_t));
  memset(&val_vbr, 0, sizeof(val_vbr_t));

  json_object* jobj = restjson::JsonBuildParse::string_to_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("Exit parse_vbr_resp_data. json_object_is_type error");
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("response data from controller : %s", data);
  int array_length = 0;
  json_object *json_obj_vbr = NULL;

  int ret_val = restjson::JsonBuildParse::parse(jobj, "vbridge", -1 ,
                                                json_obj_vbr);
  if (ret_val) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (json_object_is_type(json_obj_vbr, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj, "vbridge");
  }
  pfc_log_debug(" array _length %d" , array_length);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_debug("%s jobj NULL", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  for (int arr_idx = 0; arr_idx < array_length; arr_idx++) {
    drv_resp_code_t retval = parse_vbr_append_vector(ctr, json_obj_vbr,
                                      vtn_name, arr_idx, cfgnode_vector);
    if (DRVAPI_RESPONSE_FAILURE == retval) {
      pfc_log_error("Error in parsing vbr resp data in %s", PFC_FUNCNAME);
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  pfc_log_debug("cfgnode_vector.size: %d" ,
                static_cast<int>(cfgnode_vector.size()));
  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

drv_resp_code_t ODCVTNCommand::parse_vbr_append_vector(
    unc::driver::controller* ctr, json_object *json_obj_vbr,
    std::string vtn_name, int arr_idx,
    vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_vbr, 0, sizeof(key_vbr_t));
  memset(&val_vbr, 0, sizeof(val_vbr_t));
  string vbr_name = "";
  string vbr_description = "";

  int ret_val = restjson::JsonBuildParse::parse(json_obj_vbr, "name",
                                                arr_idx, vbr_name);
  if (ret_val) {
    pfc_log_error("Error occured while parsing vbr name");
    return DRVAPI_RESPONSE_FAILURE;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_vbr, "description",
                                            arr_idx, vbr_description);
  if (ret_val) {
    pfc_log_error(" Error occured while parsing vbr description");
    return DRVAPI_RESPONSE_FAILURE;
  }
  //  Fills the vbr KEY structure
  strncpy(reinterpret_cast<char*> (key_vbr.vtn_key.vtn_name), vtn_name.c_str(),
          sizeof(key_vbr.vtn_key.vtn_name) - 1);
  pfc_log_debug("vtn :%s", reinterpret_cast<char*> (key_vbr.vtn_key.vtn_name));

  strncpy(reinterpret_cast<char*> (key_vbr.vbridge_name), vbr_name.c_str(),
          sizeof(key_vbr.vbridge_name) - 1);
  pfc_log_error(" vbr :%s", reinterpret_cast<char*> (key_vbr.vbridge_name));

  //  Fills the vbr VAL structure
  if (0 == strlen(vbr_description.c_str())) {
    val_vbr.valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
            vbr_description.c_str(), sizeof(val_vbr.vbr_description) - 1);
    val_vbr.valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  }
  pfc_log_debug("desc :%s", reinterpret_cast<char*> (val_vbr.vbr_description));

  string controller_name = ctr->get_controller_id();
  if (0 == strlen(controller_name.c_str())) {
    val_vbr.valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vbr.controller_id),
            controller_name.c_str(), sizeof(val_vbr.controller_id) - 1);
    pfc_log_debug(" %s controller id",
                  reinterpret_cast<char*>(val_vbr.controller_id));
    val_vbr.valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
  }
  string host_address = ctr->get_host_address();
  if (0 == strlen(host_address.c_str())) {
    val_vbr.valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_INVALID;
  } else {
    int val = inet_aton(host_address.c_str(), &val_vbr.host_addr);
    if (val) {
      pfc_log_debug(" %s host address", inet_ntoa(val_vbr.host_addr));
      val_vbr.valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_VALID;
    }
  }
  strncpy(reinterpret_cast<char*> (val_vbr.domain_id), DOM_NAME.c_str(),
          sizeof(val_vbr.domain_id) - 1);
  val_vbr.valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vbr_t, val_vbr_t, uint32_t> (
          &key_vbr,
          &val_vbr,
          uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  pfc_log_debug("parse_vbr_append_vector Exiting. cfgnode_vector size: %d",
               static_cast<int>(cfgnode_vector.size()));

  return DRVAPI_RESPONSE_SUCCESS;
}
drv_resp_code_t ODCVTNCommand::parse_vtn_append_vector(
    json_object *json_obj_vtn, int arr_idx,
    vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_vtn, 0, sizeof(key_vtn_t));
  memset(&val_vtn, 0, sizeof(val_vtn_t));

  string vtn_name = "";
  string vtn_description = "";

  int ret_val = restjson::JsonBuildParse::parse(json_obj_vtn, "name", arr_idx,
                                                vtn_name);
  pfc_log_debug(" vtn_name %s:", vtn_name.c_str());
  if (ret_val) {
    pfc_log_debug(" Error while parsing vtn name");
    return DRVAPI_RESPONSE_FAILURE;
  }
  ret_val = restjson::JsonBuildParse::parse(json_obj_vtn, "description",
                                            arr_idx, vtn_description);
  pfc_log_debug(" vtn_description %s:", vtn_description.c_str());
  if (ret_val) {
    pfc_log_debug(" Error while parsing vtn description");
    return DRVAPI_RESPONSE_FAILURE;
  }
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_vtn.vtn_name), vtn_name.c_str(),
          sizeof(key_vtn.vtn_name) - 1);

  // Fill Value Structure
  if (0 == strlen(vtn_description.c_str())) {
    val_vtn.valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vtn.description),
            vtn_description.c_str(), sizeof(val_vtn.description) - 1);
    val_vtn.valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  }
  pfc_log_debug("key_vtn.vtn_name: %s",
               reinterpret_cast<char*> (key_vtn.vtn_name));
  pfc_log_debug("val_vtn.description: %s",
               reinterpret_cast<char*> (val_vtn.description));

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vtn_t, val_vtn_t, uint32_t>
      (&key_vtn, &val_vtn, uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

drv_resp_code_t ODCVTNCommand::parse_resp_data(char *data,
                      vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  pfc_log_debug("Entering function ... %s", PFC_FUNCNAME);
  json_object* jobj = restjson::JsonBuildParse::string_to_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("Exit. json_object_is_type error");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int array_length =0;
  json_object *json_obj_vtn = NULL;
  int ret_val = restjson::JsonBuildParse::parse(jobj, "vtn", -1, json_obj_vtn);
  if (ret_val) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (json_object_is_type(json_obj_vtn, json_type_null)) {
    pfc_log_debug("Parsing Error json_obj_vtn is null");
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (json_object_is_type(json_obj_vtn, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj, "vtn");
  }
  pfc_log_debug("vtn array length : %d", array_length);
  if (0 == array_length) {
    pfc_log_debug("inside 0==arraylength");
    return DRVAPI_RESPONSE_NO_SUCH_INSTANCE;
  }
  for (int arr_idx = 0; arr_idx < array_length; arr_idx++) {
    drv_resp_code_t ret_val = parse_vtn_append_vector(json_obj_vtn, arr_idx,
                                                      cfgnode_vector);
    if (ret_val == DRVAPI_RESPONSE_FAILURE) {
    pfc_log_debug(" return from parse_vtn_append_vector failure");
      return ret_val;
    }
  }

  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Delete Request send to the Contoller
drv_resp_code_t ODCVTNCommand::delete_cmd(key_vtn_t& key_vtn,
                        val_vtn_t& val_vtn, unc::driver::controller *ctr_ptr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  char* vtnname = NULL;
  std::string url = "/controller/nb/v2/vtn/default/vtns/";
  PFC_ASSERT(ctr_ptr != NULL);
  std::string ipaddress = ctr_ptr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  url.append(vtnname);

  uint32_t resp = get_controller_response_code(url, ctr_ptr,
                                    unc::restjson::HTTP_METHOD_DELETE, NULL);
  if (RESP_OK != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("response code returned in delete vtn is %d", resp);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);

  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
