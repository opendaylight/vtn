/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vtn.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcVtnCommand::OdcVtnCommand(unc::restjson::ConfFileValues_t conf_values)
: conf_file_values_(conf_values) {
  ODC_FUNC_TRACE;
}

// Destructor
OdcVtnCommand::~OdcVtnCommand() {
}

// Constructs command to create Vtn and send it to rest interface to send to
// controller
UncRespCode OdcVtnCommand::create_cmd(key_vtn_t& key_vtn,
                                          val_vtn_t& val_vtn,
                                          unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  char* vtnname = NULL;
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  url.append(vtnname);
  // unc::restjson::JsonBuildParse json_obj;
  json_object* jobj_req_body = create_request_body(val_vtn);
 /* const char* request = NULL;
  if (!(json_object_is_type(jobj_req_body, json_type_null))) {
     json_obj.get_string(jobj_req_body,request);
  }

  pfc_log_debug("Request body formed in create_cmd : %s", request);*/
  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());

  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(
         url, restjson::HTTP_METHOD_POST,
         unc::restjson::JsonBuildParse::get_json_string(jobj_req_body),
         conf_file_values_);
  json_object_put(jobj_req_body);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("response code returned in create vtn is %d", resp_code);
  if (HTTP_201_RESP_CREATED != resp_code) {
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

// Creates Request Body
json_object* OdcVtnCommand::create_request_body(const val_vtn_t& val_vtn) {
  ODC_FUNC_TRACE;
  //unc::restjson::JsonBuildParse json_obj;
  const char* description = reinterpret_cast<const char*>(val_vtn.description);
  json_object *jobj = unc::restjson::JsonBuildParse::create_json_obj();
  int ret_val = 1;
  if (UNC_VF_VALID == val_vtn.valid[UPLL_IDX_DESC_VTN]) {
    if (0 != strlen(description)) {
      ret_val = unc::restjson::JsonBuildParse::build("description", description,
                                                     jobj);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        json_object_put(jobj);
        return NULL;
      }
    }
  }
  return jobj;
}

//  Command to update vtn  and Send request to Controller
UncRespCode OdcVtnCommand::update_cmd(key_vtn_t& key_vtn,
                                          val_vtn_t& val_old_vtn,
                                          val_vtn_t& val_new_vtn,
                                          unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  char* vtnname = NULL;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in : %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  url.append(vtnname);
  // unc::restjson::JsonBuildParse json_obj;

  json_object* jobj_request = create_request_body(val_new_vtn);
/*  const char* request = NULL;
  if (!(json_object_is_type(jobj_request, json_type_null))) {
     json_obj.get_string(jobj_request,request);
  }
  pfc_log_debug("Request body formed in create_cmd : %s", request);*/

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(
          url, restjson::HTTP_METHOD_PUT,
          unc::restjson::JsonBuildParse::get_json_string(jobj_request),
           conf_file_values_);

  json_object_put(jobj_request);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("response code returned in updatevtn is %d", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

//  fetch all child configuration for the parent key type(root)
UncRespCode OdcVtnCommand::fetch_config(unc::driver::controller* ctr,
                                   void* parent_key,
                                   std::vector<unc::vtndrvcache::ConfigNode *>
                                   &cfgnode_vector) {
  ODC_FUNC_TRACE;
  return get_vtn_list(ctr, cfgnode_vector);
}

// Gets the Controller Response code
UncRespCode OdcVtnCommand::get_vtn_list(unc::driver::controller* ctr,
                               std::vector<unc::vtndrvcache::ConfigNode *>
                               &cfgnode_vector) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(NULL != ctr);
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);

  unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                          ctr->get_user_name(), ctr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
                  url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse -- ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("%d error resp ", resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data = response->write_data->memory;
      pfc_log_debug("vtns present : %s", data);
      UncRespCode ret_val =  parse_vtn_response(data, cfgnode_vector);
      pfc_log_debug("read_all_--  size, %d",
                    static_cast<int>(cfgnode_vector.size()));
      return ret_val;
    }
  }
  return UNC_DRV_RC_ERR_GENERIC;
}

// parse each vtn node append to cache
UncRespCode OdcVtnCommand::fill_config_node_vector(
    json_object *json_obj_vtn, int arr_idx,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_vtn, 0, sizeof(key_vtn_t));
  memset(&val_vtn, 0, sizeof(val_vtn_t));

  std::string vtn_name = "";
  std::string vtn_description = "";

  uint32_t ret_val = restjson::JsonBuildParse::parse(json_obj_vtn, "name",
                                              arr_idx, vtn_name);
  pfc_log_debug(" vtn_name %s:", vtn_name.c_str());
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing vtn name");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ret_val = restjson::JsonBuildParse::parse(json_obj_vtn, "description",
                                            arr_idx, vtn_description);
  pfc_log_debug(" vtn_description %s:", vtn_description.c_str());
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing vtn description");
    return UNC_DRV_RC_ERR_GENERIC;
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
      new unc::vtndrvcache::CacheElementUtil<key_vtn_t, val_vtn_t, val_vtn_t, uint32_t>
      (&key_vtn, &val_vtn, &val_vtn, uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  return UNC_RC_SUCCESS;
}

// parsing function for converting controller response to driver format
UncRespCode OdcVtnCommand::parse_vtn_response(char *data,
                                               std::vector< unc::vtndrvcache
                                               ::ConfigNode *>
                                               &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_type error");
    json_object_put(jobj);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  uint32_t array_length =0;
  json_object *json_obj_vtn = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj, "vtn", -1,
                                                     json_obj_vtn);

  if (json_object_is_type(json_obj_vtn, json_type_null)) {
    json_object_put(jobj);
    pfc_log_error("Parsing Error json_obj_vtn is null");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (restjson::REST_OP_SUCCESS != ret_val) {
    json_object_put(jobj);
    pfc_log_error("Error in parsing the json_obj_vtn");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (json_object_is_type(json_obj_vtn, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj, "vtn");
  }
  pfc_log_debug("vtn array length : %d", array_length);
  if (0 == array_length) {
    pfc_log_debug("inside 0==arraylength");
    json_object_put(jobj);
    return UNC_RC_NO_SUCH_INSTANCE;
  }
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    UncRespCode ret_val = fill_config_node_vector(json_obj_vtn, arr_idx,
                                                      cfgnode_vector);
    if (UNC_RC_SUCCESS != ret_val) {
      json_object_put(jobj);
      pfc_log_error("Error return from parse_vtn_append_vector failure");
      return ret_val;
    }
  }
  json_object_put(jobj);
  return UNC_RC_SUCCESS;
}

// Delete Request send to the Contoller
UncRespCode OdcVtnCommand::delete_cmd(key_vtn_t& key_vtn,
                                          val_vtn_t& val_vtn,
                                          unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  char* vtnname = NULL;
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  PFC_ASSERT(ctr_ptr != NULL);
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  url.append(vtnname);

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
               url, restjson::HTTP_METHOD_DELETE, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("response code returned in delete vtn is %d", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
