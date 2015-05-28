/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vterminal.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcVterminalCommand::OdcVterminalCommand(unc::restjson::ConfFileValues_t
                                         conf_values)
: conf_file_values_(conf_values) {
      ODC_FUNC_TRACE;
    }

// Destructor
OdcVterminalCommand::~OdcVterminalCommand() {
}

// Constructs command to create Vtn and send it to rest interface to send to
// controller
UncRespCode OdcVterminalCommand::create_cmd(key_vterm_t& key_vterm,
                                            val_vterm_t& val_vterm,
                                            unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vterminal_url = get_vterminal_url(key_vterm);
  if (vterminal_url.empty()) {
    pfc_log_error("vterminal url is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  json_object *jobj_request = create_request_body(val_vterm);
  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                          ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(vterminal_url,
          restjson::HTTP_METHOD_POST,
          unc::restjson::JsonBuildParse::get_json_string(jobj_request),
          conf_file_values_);
  json_object_put(jobj_request);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("response code returned in create vterminal is %d", resp_code);
  if (HTTP_201_RESP_CREATED != resp_code) {
    pfc_log_debug("check if vtn is stand-alone");
    //  check if vtn is stand-alone
    UncRespCode ret_code = UNC_DRV_RC_ERR_GENERIC;
    std::vector<unc::vtndrvcache::ConfigNode*> child_list;
    child_list.clear();
    std::string vtn_name = reinterpret_cast<const char*>
        (key_vterm.vtn_key.vtn_name);
    pfc_log_debug("VTN name str:%s", vtn_name.c_str());
    ret_code = get_vterminal_list(vtn_name, ctr_ptr, child_list);
    int vtn_child_size = static_cast<int> (child_list.size());
    pfc_log_debug("VTN child_list... size: %d", vtn_child_size);
    if (ret_code == UNC_RC_SUCCESS) {
      if (vtn_child_size == 0) {
        pfc_log_debug("delete stand-alone vtn");
        //  delete stand-alone vtn
        key_vtn_t key_vtn;
        val_vtn_t val_vtn;
        memset(&key_vtn, 0, sizeof(key_vtn_t));
        memset(&val_vtn, 0, sizeof(val_vtn_t));
        memcpy(key_vtn.vtn_name, key_vterm.vtn_key.vtn_name,
               sizeof(key_vterm.vtn_key.vtn_name));
        pfc_log_debug("VTN name:%s", key_vtn.vtn_name);
        OdcVtnCommand vtn_obj(conf_file_values_);
        vtn_obj.delete_cmd(key_vtn, val_vtn, ctr_ptr);
      }
    }
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

// Create Request body
json_object* OdcVterminalCommand::create_request_body(const val_vterm_t&
                                                      val_vterm) {
  ODC_FUNC_TRACE;
  const char* description =
      reinterpret_cast<const char*>(val_vterm.vterm_description);
  json_object *jobj = unc::restjson::JsonBuildParse::create_json_obj();
  int ret_val = 1;
  if (UNC_VF_VALID == val_vterm.valid[UPLL_IDX_DESC_VTERM]) {
    if (0 != strlen(description)) {
      ret_val = unc::restjson::JsonBuildParse::build("description", description, jobj);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        json_object_put(jobj);
        return NULL;
      }
    }
  }
  return jobj;
}

// Get Vterminal List
UncRespCode OdcVterminalCommand::get_vterminal_list(std::string vtnname,
                               unc::driver::controller* ctr,
                               std::vector<unc::vtndrvcache::ConfigNode *>
                               &cfg_node_vector) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(NULL != ctr);

  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  url.append(vtnname);
  url.append("/vterminals");
  pfc_log_debug("url:%s", url.c_str());

  unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                                        ctr->get_user_name(),
                                        ctr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
      url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code for GET vterminal : %d", resp_code);

  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("get vterminal is not succesful , resp_code %d", resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data = response->write_data->memory;
      UncRespCode ret_code = parse_vterminal_response(data, vtnname, ctr,
                                                cfg_node_vector);
      return ret_code;
    }
  }
  return UNC_DRV_RC_ERR_GENERIC;
}



// Update Vterminal
UncRespCode OdcVterminalCommand::update_cmd(key_vterm_t& key, val_vterm_t& val_old,
                                            val_vterm_t& val_new,
                                            unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  return UNC_RC_SUCCESS;
}

// Delete Vterminal
UncRespCode OdcVterminalCommand::delete_cmd(key_vterm_t& key_vterm,
                                            val_vterm_t& val_vterm,
                       unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  std::string vterminal_url = get_vterminal_url(key_vterm);
  pfc_log_debug("vterminal_url:%s", vterminal_url.c_str());
  if (vterminal_url.empty()) {
    pfc_log_error("vterminal url is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                                        ctr_ptr->get_user_name(),
                                        ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
      vterminal_url, restjson::HTTP_METHOD_DELETE, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for delete vterminal : %d ",
                resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("vterminal delete is not success , resp_code id: %d",
                  resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}


// fetch all child configuration for the parent key type(root)
UncRespCode OdcVterminalCommand::fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector) {
  ODC_FUNC_TRACE;
  pfc_log_debug("fetch_config for OdcVterminalCommand");
  key_vtn_t* parent_vtn = reinterpret_cast<key_vtn_t*> (parent_key);
  std::string vtn_name = reinterpret_cast<const char*>
      (parent_vtn->vtn_name);
  pfc_log_debug("%s:vtn_name", vtn_name.c_str());
  return get_vterminal_list(vtn_name, ctr, cfg_node_vector);
}

// parsing function for converting controller response to driver format
UncRespCode OdcVterminalCommand::parse_vterminal_response(
    char *data,
    std::string vtn_name,
    unc::driver::controller* ctr,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfg_node_vector) {
  ODC_FUNC_TRACE;
  UncRespCode resp_code = UNC_DRV_RC_ERR_GENERIC;
  key_vterm_t key_vterm;
  val_vterm_t val_vterm;
  memset(&key_vterm, 0, sizeof(key_vterm_t));
  memset(&val_vterm, 0, sizeof(val_vterm_t));
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("Exit parse_vterminal_resp_data. json_object_is_type null");
    json_object_put(jobj);
    return resp_code;
  }
  pfc_log_debug("response data from controller : %s", data);
  uint32_t array_length = 0;
  json_object *json_obj_vterm = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj, "vterminal", -1 ,
                                                     json_obj_vterm);
  if (json_object_is_type(json_obj_vterm, json_type_null)) {
    json_object_put(jobj);
    pfc_log_error("%s jobj_vterminal NULL", PFC_FUNCNAME);
    return resp_code;
  }

  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error in parsing json object in %s", PFC_FUNCNAME);
    json_object_put(jobj);
    return resp_code;
  }

  if (json_object_is_type(json_obj_vterm, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj,
                                                              "vterminal");
  }
  pfc_log_debug(" array _length %d" , array_length);
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    UncRespCode retval = fill_config_node_vector(ctr, json_obj_vterm,
                                                 vtn_name, arr_idx,
                                                 cfg_node_vector);
    if (UNC_RC_SUCCESS != retval) {
      pfc_log_error("Error in parsing vterminal resp data in %s", PFC_FUNCNAME);
      json_object_put(jobj);
      return resp_code;
    }
  }
  json_object_put(jobj);
  pfc_log_debug("cfg_node_vector.size: %d" ,
                static_cast<int>(cfg_node_vector.size()));
  return UNC_RC_SUCCESS;

  return UNC_RC_SUCCESS;
    }

UncRespCode OdcVterminalCommand::fill_config_node_vector(
    unc::driver::controller* ctr, json_object *json_obj_vterm,
    std::string vtn_name, uint32_t arr_idx,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector) {
  ODC_FUNC_TRACE;
  key_vterm_t key_vterm;
  val_vterm_t val_vterm;
  memset(&key_vterm, 0, sizeof(key_vterm_t));
  memset(&val_vterm, 0, sizeof(val_vterm_t));
  std::string vterm_name = "";
  std::string vterm_description = "";

  uint32_t ret_val = restjson::JsonBuildParse::parse(json_obj_vterm, "name",
                                                     arr_idx, vterm_name);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing vterminal name");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_vterm, "description",
                                            arr_idx, vterm_description);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing vterm description");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  //  Fills the vterm KEY structure
  strncpy(reinterpret_cast<char*> (key_vterm.vtn_key.vtn_name),
          vtn_name.c_str(),
          sizeof(key_vterm.vtn_key.vtn_name) - 1);
  pfc_log_debug("vtn :%s", reinterpret_cast<char*>
                (key_vterm.vtn_key.vtn_name));

  strncpy(reinterpret_cast<char*> (key_vterm.vterminal_name),
          vterm_name.c_str(),
          sizeof(key_vterm.vterminal_name) - 1);
  pfc_log_debug(" vterminal :%s", reinterpret_cast<char*>
                (key_vterm.vterminal_name));

  //  Fills the vterminal VAL structure
  if (0 == strlen(vterm_description.c_str())) {
    val_vterm.valid[UPLL_IDX_DESC_VTERM] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vterm.vterm_description),
            vterm_description.c_str(),
            sizeof(val_vterm.vterm_description) - 1);
    val_vterm.valid[UPLL_IDX_DESC_VTERM] = UNC_VF_VALID;
  }
  pfc_log_debug("desc :%s", reinterpret_cast<char*>
                (val_vterm.vterm_description));

  std::string controller_name = ctr->get_controller_id();
  if (0 == strlen(controller_name.c_str())) {
    val_vterm.valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vterm.controller_id),
            controller_name.c_str(), sizeof(val_vterm.controller_id) - 1);
    pfc_log_debug(" %s controller id",
                  reinterpret_cast<char*>(val_vterm.controller_id));
    val_vterm.valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_VALID;
  }
  strncpy(reinterpret_cast<char*> (val_vterm.domain_id), DOM_NAME.c_str(),
          sizeof(val_vterm.domain_id) - 1);
  val_vterm.valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_VALID;

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vterm_t, val_vterm_t,
          val_vterm_t,
          uint32_t> (
          &key_vterm,
          &val_vterm, &val_vterm,
          uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfg_node_vector.push_back(cfgptr);
  pfc_log_debug("parse_vterminal_append_vector Exi. cfg_node_vector size: %d",
                static_cast<int>(cfg_node_vector.size()));
  return UNC_RC_SUCCESS;
}


// Constructing URL for vterminal,inject request to controller
std::string OdcVterminalCommand::get_vterminal_url(key_vterm_t& key_vterm) {
  ODC_FUNC_TRACE;
  char* vtnname = NULL;
  char* vterminalname = NULL;
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  vtnname = reinterpret_cast<char*>(key_vterm.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    return "";
  }
  url.append(vtnname);
  vterminalname = reinterpret_cast<char*>(key_vterm.vterminal_name);
  if (0 == strlen(vterminalname)) {
    return "";
  }
  url.append("/vterminals/");
  url.append(vterminalname);
  return url;
}
}  // namespace odcdriver
}  // namespace unc
