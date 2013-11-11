/*
 * Copyright (c) 2013 NEC Corporation
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
OdcVtnCommand::OdcVtnCommand()
: idle_timeout_(DEFAULT_IDLE_TIME_OUT),
  hard_timeout_(DEFAULT_HARD_TIME_OUT) {
  ODC_FUNC_TRACE;
}

// Destructor
OdcVtnCommand::~OdcVtnCommand() {
}

// Constructs command to create Vtn and send it to rest interface to send to
// controller
drv_resp_code_t OdcVtnCommand::create_cmd(key_vtn_t& key_vtn,
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
  std::string ipaddress = ctr_ptr->get_host_address();
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  url.append(vtnname);
  unc::restjson::JsonBuildParse json_obj;
  json_object* jobj_req_body = create_request_body(val_vtn);
  const char* request = NULL;
  if (!(json_object_is_type(jobj_req_body, json_type_null))) {
    request = json_obj.get_string(jobj_req_body);
  }

  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr_ptr , user_name, password);
  pfc_log_debug("Request body formed in create_cmd : %s", request);
  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  restjson::RestClient rest_client_obj(ipaddress, url, odc_port,
                                       restjson::HTTP_METHOD_POST);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
                                            user_name, password,
                                            connect_time_out,
                                            req_time_out, request);
  json_object_put(jobj_req_body);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  rest_client_obj.clear_http_response();
  response = NULL;
  pfc_log_debug("response code returned in create vtn is %d", resp_code);
  if (HTTP_201_RESP_CREATED != resp_code) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// Gets the user name password from the controller else from conf file
void OdcVtnCommand::get_username_password(unc::driver::controller* ctr_ptr,
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

// reads username password from conf file else from default value
void OdcVtnCommand::read_user_name_password(std::string &user_name,
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
    pfc_log_debug("%s: Block Handle is InValid, get default user_name_ %s",
                  PFC_FUNCNAME, user_name.c_str());
  }
}

// Creates Request Body
json_object* OdcVtnCommand::create_request_body(const val_vtn_t& val_vtn) {
  ODC_FUNC_TRACE;
  unc::restjson::JsonBuildParse json_obj;
  const char* description = reinterpret_cast<const char*>(val_vtn.description);
  json_object *jobj = json_obj.create_json_obj();
  int ret_val = 1;
  if (UNC_VF_VALID == val_vtn.valid[UPLL_IDX_DESC_VTN]) {
    if (0 != strlen(description)) {
      ret_val = json_obj.build("description", description, jobj);
      if (ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        json_object_put(jobj);
        return NULL;
      }
    }
  }
  std::ostringstream hard_timeout_str_format;
  hard_timeout_str_format << hard_timeout_;
  std::ostringstream idle_timeout_str_format;
  idle_timeout_str_format << idle_timeout_;
  std::string idle_time_out = idle_timeout_str_format.str();
  ret_val = json_obj.build("idleTimeout", idle_timeout_str_format.str(), jobj);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
    json_object_put(jobj);
    return NULL;
  }
  ret_val = json_obj.build("hardTimeout", hard_timeout_str_format.str(), jobj);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
    json_object_put(jobj);
    return NULL;
  }
  return jobj;
}

//  Command to update vtn  and Send request to Controller
drv_resp_code_t OdcVtnCommand::update_cmd(key_vtn_t& key_vtn,
                                          val_vtn_t& val_vtn,
                                          unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  char* vtnname = NULL;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  std::string ipaddress = ctr_ptr->get_host_address();
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in : %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  url.append(vtnname);
  unc::restjson::JsonBuildParse json_obj;

  json_object* jobj_request = create_request_body(val_vtn);
  const char* request = NULL;
  if (!(json_object_is_type(jobj_request, json_type_null))) {
    request = json_obj.get_string(jobj_request);
  }
  pfc_log_debug(" Request body formed %s", request);

  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr_ptr, user_name, password);
  pfc_log_debug("Request body formed in create_cmd : %s", request);
  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  restjson::RestClient rest_client_obj(ipaddress, url,
                       odc_port, restjson::HTTP_METHOD_PUT);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out,
      request);
  json_object_put(jobj_request);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("response code returned in updatevtn is %d", resp_code);
  rest_client_obj.clear_http_response();
  if (HTTP_200_RESP_OK != resp_code) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

//  fetch all child configuration for the parent key type(root)
drv_resp_code_t OdcVtnCommand::fetch_config(unc::driver::controller* ctr,
                                   void* parent_key,
                                   std::vector<unc::vtndrvcache::ConfigNode *>
                                   &cfgnode_vector) {
  ODC_FUNC_TRACE;
  return get_vtn_list(ctr, cfgnode_vector);
}

// Gets the Controller Response code
drv_resp_code_t OdcVtnCommand::get_vtn_list(unc::driver::controller* ctr,
                               std::vector<unc::vtndrvcache::ConfigNode *>
                               &cfgnode_vector) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(NULL != ctr);
  std::string ip_address = ctr->get_host_address();
  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr, user_name, password);
  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  restjson::RestClient rest_client_obj(ip_address, url,
                                       odc_port, restjson::HTTP_METHOD_GET);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out,
      NULL);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse -- ");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("%d error resp ", resp_code);
    rest_client_obj.clear_http_response();
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data = response->write_data->memory;
      pfc_log_debug("vtns present : %s", data);
      drv_resp_code_t ret_val =  parse_vtn_response(data, cfgnode_vector);
      rest_client_obj.clear_http_response();
      pfc_log_debug("read_all_--  size, %d",
                    static_cast<int>(cfgnode_vector.size()));
      return ret_val;
    }
  }
  rest_client_obj.clear_http_response();
  return DRVAPI_RESPONSE_FAILURE;
}

// parse each vtn node append to cache
drv_resp_code_t OdcVtnCommand::fill_config_node_vector(
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
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing vtn name");
    return DRVAPI_RESPONSE_FAILURE;
  }
  ret_val = restjson::JsonBuildParse::parse(json_obj_vtn, "description",
                                            arr_idx, vtn_description);
  pfc_log_debug(" vtn_description %s:", vtn_description.c_str());
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
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
  return DRVAPI_RESPONSE_SUCCESS;
}

// parsing function for converting controller response to driver format
drv_resp_code_t OdcVtnCommand::parse_vtn_response(char *data,
                                               std::vector< unc::vtndrvcache
                                               ::ConfigNode *>
                                               &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_type error");
    json_object_put(jobj);
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t array_length =0;
  json_object *json_obj_vtn = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj, "vtn", -1,
                                                     json_obj_vtn);

  if (json_object_is_type(json_obj_vtn, json_type_null)) {
    json_object_put(jobj);
    pfc_log_error("Parsing Error json_obj_vtn is null");
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    json_object_put(jobj);
    pfc_log_error("Error in parsing the json_obj_vtn");
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (json_object_is_type(json_obj_vtn, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj, "vtn");
  }
  pfc_log_debug("vtn array length : %d", array_length);
  if (0 == array_length) {
    pfc_log_debug("inside 0==arraylength");
    json_object_put(jobj);
    return DRVAPI_RESPONSE_NO_SUCH_INSTANCE;
  }
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    drv_resp_code_t ret_val = fill_config_node_vector(json_obj_vtn, arr_idx,
                                                      cfgnode_vector);
    if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
      json_object_put(jobj);
      pfc_log_error("Error return from parse_vtn_append_vector failure");
      return ret_val;
    }
  }
  json_object_put(jobj);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Delete Request send to the Contoller
drv_resp_code_t OdcVtnCommand::delete_cmd(key_vtn_t& key_vtn,
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
  std::string ipaddress = ctr_ptr->get_host_address();
  vtnname = reinterpret_cast<char*>(key_vtn.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  url.append(vtnname);

  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr_ptr , user_name, password);
  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  restjson::RestClient rest_client_obj(ipaddress, url,
                                odc_port, restjson::HTTP_METHOD_DELETE);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out, NULL);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("response code returned in delete vtn is %d", resp_code);
  rest_client_obj.clear_http_response();
  if (HTTP_200_RESP_OK != resp_code) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// Reads set opt params from conf file if it fails reads from defs file
void OdcVtnCommand:: read_conf_file(uint32_t &odc_port,
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
    pfc_log_debug("%s: Block Handle is Valid, odc_port_ %d ", PFC_FUNCNAME,
                  odc_port);

  } else {
    odc_port   =  DEFAULT_ODC_PORT;
    connection_time_out = DEFAULT_CONNECT_TIME_OUT;
    request_time_out = DEFAULT_REQ_TIME_OUT;
    pfc_log_debug("%s: Block Handle is Invalid,set default Value %d ",
                  PFC_FUNCNAME, odc_port);
  }
}
}  // namespace odcdriver
}  // namespace unc
