/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vtn.hh>

namespace unc {
namespace odcdriver {

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
                  url, restjson::HTTP_METHOD_GET, NULL, conf_values_);

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
    key_vtn_t key;
    val_vtn_t val;
    memset(&key, 0, sizeof(key_vtn_t));
    memset(&val, 0, sizeof(val_vtn_t));
    VtnParser obj;
    json_object* json_idx_obj = NULL;
    json_idx_obj = json_object_array_get_idx(json_obj_vtn, arr_idx);
    pfc_log_debug("Json_obj_idx %s", restjson::JsonBuildParse::get_json_string(json_idx_obj));
    UncRespCode ret_val = obj.parse_vtn_response(json_idx_obj, key, val);
    if (UNC_RC_SUCCESS != ret_val) {
      json_object_put(jobj);
      pfc_log_error("Error return from parse_vtn_append_vector failure");
      return ret_val;
    }
    //ret_val = 
   unc::vtndrvcache::ConfigNode *cfgptr =
       new unc::vtndrvcache::CacheElementUtil<key_vtn_t, val_vtn_t, uint32_t>
      (&key, &val, uint32_t(UNC_OP_READ));
   PFC_ASSERT(cfgptr != NULL);
   cfgnode_vector.push_back(cfgptr);
   json_object_put(json_idx_obj);
  }
  json_object_put(jobj);
  json_object_put(json_obj_vtn);
  return UNC_RC_SUCCESS;
}

}  // namespace odcdriver
}  // namespace unc
