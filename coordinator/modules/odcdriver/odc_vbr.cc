/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbr.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcVbrCommand::OdcVbrCommand(unc::restjson::ConfFileValues_t conf_values)
: age_interval_(DEFAULT_AGE_INTERVAL),
  conf_file_values_(conf_values) {
  ODC_FUNC_TRACE;
}

// Destructor
OdcVbrCommand::~OdcVbrCommand() {
}

// Create Command and send Request to Controller
drv_resp_code_t OdcVbrCommand::create_cmd(key_vbr_t& key_vbr,
                                          val_vbr_t& val_vbr,
                                          unc::driver::controller
                                          *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  std::string vbr_url = get_vbr_url(key_vbr);
  if (vbr_url.empty()) {
    pfc_log_error("vbrif url is empty");
    return DRVAPI_RESPONSE_FAILURE;
  }
  unc::restjson::JsonBuildParse json_obj;
  json_object *json_req_body = create_request_body(val_vbr);
  const char* request = NULL;

  if (!(json_object_is_type(json_req_body, json_type_null))) {
    request = json_obj.get_string(json_req_body);
  }
  pfc_log_debug("Request body formed in vbr create_cmd : %s", request);

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
     vbr_url, restjson::HTTP_METHOD_POST, request, conf_file_values_);

  json_object_put(json_req_body);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("resp_code for create vbr is %d", resp_code);

  if (HTTP_201_RESP_CREATED != resp_code) {
    pfc_log_debug("check if vtn is stand-alone");
    //  check if vtn is stand-alone
    drv_resp_code_t ret_code = DRVAPI_RESPONSE_FAILURE;
    std::vector<unc::vtndrvcache::ConfigNode*> child_list;
    child_list.clear();
    std::string vtn_name = reinterpret_cast<const char*>
        (key_vbr.vtn_key.vtn_name);
    pfc_log_debug("VTN name str:%s", vtn_name.c_str());
    ret_code = get_vbr_list(vtn_name, ctr_ptr, child_list);
    int vtn_child_size = static_cast<int> (child_list.size());
    pfc_log_debug("VTN child_list... size: %d", vtn_child_size);

    if (ret_code == DRVAPI_RESPONSE_SUCCESS) {
      if (vtn_child_size == 0) {
        pfc_log_debug("delete stand-alone vtn");
        //  delete stand-alone vtn
        key_vtn_t key_vtn;
        val_vtn_t val_vtn;
        memset(&key_vtn, 0, sizeof(key_vtn_t));
        memset(&val_vtn, 0, sizeof(val_vtn_t));
        memcpy(key_vtn.vtn_name, key_vbr.vtn_key.vtn_name,
               sizeof(key_vbr.vtn_key.vtn_name));
        pfc_log_debug("VTN name:%s", key_vtn.vtn_name);
        OdcVtnCommand vtn_obj(conf_file_values_);
        vtn_obj.delete_cmd(key_vtn, val_vtn, ctr_ptr);
      }
    }
    return DRVAPI_RESPONSE_FAILURE;
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

//  Command to update vtn  and Send request to Controller
drv_resp_code_t OdcVbrCommand::update_cmd(key_vbr_t& key_vbr,
                                 val_vbr_t& val_vbr,
                                 unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  std::string vbr_url = get_vbr_url(key_vbr);
  if (vbr_url.empty()) {
    pfc_log_error("vbr url is empty");
    return DRVAPI_RESPONSE_FAILURE;
  }
  json_object *json_req_body = create_request_body(val_vbr);
  unc::restjson::JsonBuildParse json_obj;
  const char* request = NULL;
  if (!(json_object_is_type(json_req_body, json_type_null))) {
    request = json_obj.get_string(json_req_body);
  }
  pfc_log_debug("Request body formed in vbr update_cmd : %s", request);

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
               vbr_url, restjson::HTTP_METHOD_PUT, request, conf_file_values_);

  json_object_put(json_req_body);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for vbr update cmd : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("vbr is not updated , resp_code is : %d", resp_code);
    return DRVAPI_RESPONSE_FAILURE;
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// Delete Request send to the Controller
drv_resp_code_t OdcVbrCommand::delete_cmd(key_vbr_t& key_vbr,
                                          val_vbr_t& val_vbr,
                                          unc::driver::controller
                                          *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  std::string vbr_url = get_vbr_url(key_vbr);
  pfc_log_debug("vbr_url:%s", vbr_url.c_str());
  if (vbr_url.empty()) {
    pfc_log_error("vbr url is empty");
    return DRVAPI_RESPONSE_FAILURE;
  }

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
               vbr_url, restjson::HTTP_METHOD_DELETE, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for delete vbr : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("vbr delete is not success , resp_code id: %d", resp_code);
    return DRVAPI_RESPONSE_FAILURE;
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// Creates Request Body
json_object* OdcVbrCommand::create_request_body(const val_vbr_t& val_vbr) {
  ODC_FUNC_TRACE;
  unc::restjson::JsonBuildParse json_obj;
  json_object *jobj = json_obj.create_json_obj();
  const char* description = reinterpret_cast<const char*>
      (val_vbr.vbr_description);
  uint32_t ret_val = ODC_DRV_FAILURE;
  if (UNC_VF_VALID == val_vbr.valid[UPLL_IDX_DESC_VBR]) {
    if (0 != strlen(description)) {
      ret_val = json_obj.build("description", description, jobj);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error in building request body - description");
        json_object_put(jobj);
        return NULL;
      }
    }
  }
  std::ostringstream age_interval_str_format;
  age_interval_str_format << age_interval_;

  ret_val = json_obj.build("ageInterval", age_interval_str_format.str(), jobj);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error in building request body - description");
    json_object_put(jobj);
    return NULL;
  }
  return jobj;
}

// fetch child configurations for the parent kt(vtn)
drv_resp_code_t OdcVbrCommand::fetch_config(unc::driver::controller* ctr,
                                    void* parent_key,
                                    std::vector<unc::vtndrvcache::ConfigNode *>
                                    &cfgnode_vector) {
  ODC_FUNC_TRACE;
  pfc_log_debug("fetch_config for OdcVbrCommand");

  key_vtn_t* parent_vtn = reinterpret_cast<key_vtn_t*> (parent_key);
  std::string vtn_name = reinterpret_cast<const char*>
                   (parent_vtn->vtn_name);
  pfc_log_debug("%s:vtn_name", vtn_name.c_str());

  return get_vbr_list(vtn_name, ctr, cfgnode_vector);
}

// Get all vbridges under specified vtn
drv_resp_code_t OdcVbrCommand::get_vbr_list(std::string vtn_name,
                                             unc::driver::controller* ctr,
                                             std::vector< unc::vtndrvcache
                                             ::ConfigNode *>
                                             &cfgnode_vector) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(NULL != ctr);

  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  url.append(vtn_name);
  url.append("/vbridges");
  pfc_log_debug("url:%s", url.c_str());

  unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                  ctr->get_user_name(), ctr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
                    url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code for GET vbridges : %d", resp_code);

  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("get vbridges is not succesful , resp_code %d", resp_code);
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
       char *data = response->write_data->memory;
       drv_resp_code_t ret_code = parse_vbr_response(data,
                                                     vtn_name,
                                                     ctr,
                                                     cfgnode_vector);
       return ret_code;
    }
  }
  return DRVAPI_RESPONSE_FAILURE;
}

// Prsing fuction for vbridge after getting vbridge from controller
drv_resp_code_t OdcVbrCommand::parse_vbr_response(char *data,
                                         std::string vtn_name,
                                  unc::driver::controller* ctr,
  std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_vbr, 0, sizeof(key_vbr_t));
  memset(&val_vbr, 0, sizeof(val_vbr_t));
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("Exit parse_vbr_resp_data. json_object_is_type null");
    json_object_put(jobj);
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("response data from controller : %s", data);
  uint32_t array_length = 0;
  json_object *json_obj_vbr = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj, "vbridge", -1 ,
                                                     json_obj_vbr);
  if (json_object_is_type(json_obj_vbr, json_type_null)) {
    json_object_put(jobj);
    pfc_log_error("%s jobj_vbr NULL", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error in parsing json object in %s", PFC_FUNCNAME);
    json_object_put(jobj);
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (json_object_is_type(json_obj_vbr, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj, "vbridge");
  }
  pfc_log_debug(" array _length %d" , array_length);
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    drv_resp_code_t retval = fill_config_node_vector(ctr, json_obj_vbr,
                                          vtn_name, arr_idx, cfgnode_vector);
    if (DRVAPI_RESPONSE_SUCCESS != retval) {
      pfc_log_error("Error in parsing vbr resp data in %s", PFC_FUNCNAME);
      json_object_put(jobj);
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  json_object_put(jobj);
  pfc_log_debug("cfgnode_vector.size: %d" ,
                static_cast<int>(cfgnode_vector.size()));
  return DRVAPI_RESPONSE_SUCCESS;
}

// each vbridge node append to cache
drv_resp_code_t OdcVbrCommand::fill_config_node_vector(
    unc::driver::controller* ctr, json_object *json_obj_vbr,
    std::string vtn_name, uint32_t arr_idx,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_vbr, 0, sizeof(key_vbr_t));
  memset(&val_vbr, 0, sizeof(val_vbr_t));
  std::string vbr_name = "";
  std::string vbr_description = "";

  uint32_t ret_val = restjson::JsonBuildParse::parse(json_obj_vbr, "name",
                                                arr_idx, vbr_name);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing vbr name");
    return DRVAPI_RESPONSE_FAILURE;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_vbr, "description",
                                            arr_idx, vbr_description);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing vbr description");
    return DRVAPI_RESPONSE_FAILURE;
  }
  //  Fills the vbr KEY structure
  strncpy(reinterpret_cast<char*> (key_vbr.vtn_key.vtn_name), vtn_name.c_str(),
          sizeof(key_vbr.vtn_key.vtn_name) - 1);
  pfc_log_debug("vtn :%s", reinterpret_cast<char*> (key_vbr.vtn_key.vtn_name));

  strncpy(reinterpret_cast<char*> (key_vbr.vbridge_name), vbr_name.c_str(),
          sizeof(key_vbr.vbridge_name) - 1);
  pfc_log_debug(" vbr :%s", reinterpret_cast<char*> (key_vbr.vbridge_name));

  //  Fills the vbr VAL structure
  if (0 == strlen(vbr_description.c_str())) {
    val_vbr.valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
            vbr_description.c_str(), sizeof(val_vbr.vbr_description) - 1);
    val_vbr.valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  }
  pfc_log_debug("desc :%s", reinterpret_cast<char*> (val_vbr.vbr_description));

  std::string controller_name = ctr->get_controller_id();
  if (0 == strlen(controller_name.c_str())) {
    val_vbr.valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vbr.controller_id),
            controller_name.c_str(), sizeof(val_vbr.controller_id) - 1);
    pfc_log_debug(" %s controller id",
                  reinterpret_cast<char*>(val_vbr.controller_id));
    val_vbr.valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
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

// Constructing URL for vbridge,inject request to controller
std::string OdcVbrCommand::get_vbr_url(key_vbr_t& key_vbr) {
  ODC_FUNC_TRACE;
  char* vtnname = NULL;
  char* vbrname = NULL;
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
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
  return url;
}
}  // namespace odcdriver
}  // namespace unc
