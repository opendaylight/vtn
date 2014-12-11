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
      url, restjson::HTTP_METHOD_GET, NULL, conf_values_);
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


// parsing function for converting controller response to driver format
UncRespCode OdcVterminalCommand::parse_vterminal_response(
    char *data,
    std::string vtn_name,
    unc::driver::controller* ctr,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfg_node_vector) {
  ODC_FUNC_TRACE;
  UncRespCode resp_code = UNC_DRV_RC_ERR_GENERIC;
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
    key_vterm_t key_vterm;
    val_vterm_t val_vterm;
    memset(&key_vterm, 0, sizeof(key_vterm_t));
    memset(&val_vterm, 0, sizeof(val_vterm_t));
    strncpy(reinterpret_cast<char*> (key_vterm.vtn_key.vtn_name),
          vtn_name.c_str(),
          sizeof(key_vterm.vtn_key.vtn_name) - 1);
    VtermParser obj;
    json_object* json_idx_obj = NULL;
    json_idx_obj = json_object_array_get_idx(json_obj_vterm, arr_idx);
    pfc_log_debug("Json_obj_idx %s", restjson::JsonBuildParse::get_json_string(json_idx_obj));
    int retval = obj.parse_vterm_response(json_idx_obj, key_vterm, val_vterm);
    if (UNC_RC_SUCCESS != retval) {
      pfc_log_error("Error in parsing vterminal resp data in %s", PFC_FUNCNAME);
      json_object_put(jobj);
      return resp_code;
    }
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
          uint32_t> (
          &key_vterm,
          &val_vterm,
          uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfg_node_vector.push_back(cfgptr);
  pfc_log_debug("parse_vterminal_append_vector Exi. cfg_node_vector size: %d",
                static_cast<int>(cfg_node_vector.size()));
 }
  json_object_put(jobj);
  pfc_log_debug("cfg_node_vector.size: %d" ,
                static_cast<int>(cfg_node_vector.size()));
  return UNC_RC_SUCCESS;
 }

}  // namespace odcdriver
}  // namespace unc
