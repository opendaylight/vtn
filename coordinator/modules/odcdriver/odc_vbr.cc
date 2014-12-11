/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbr.hh>

namespace unc {
namespace odcdriver {

// Get all vbridges under specified vtn
UncRespCode OdcVbrCommand::get_vbr_list(std::string vtn_name,
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
                    url, restjson::HTTP_METHOD_GET, NULL, conf_values_);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code for GET vbridges : %d", resp_code);

  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("get vbridges is not succesful , resp_code %d", resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
       char *data = response->write_data->memory;
       UncRespCode ret_code = parse_vbr_response(data,
                                                 vtn_name,
                                                 ctr,
                                                 cfgnode_vector);
       return ret_code;
    }
  }
  return UNC_DRV_RC_ERR_GENERIC;
}

// Prsing fuction for vbridge after getting vbridge from controller
UncRespCode OdcVbrCommand::parse_vbr_response(char *data,
                                         std::string vtn_name,
                                  unc::driver::controller* ctr,
  std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("Exit parse_vbr_resp_data. json_object_is_type null");
    json_object_put(jobj);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_debug("response data from controller : %s", data);
  uint32_t array_length = 0;
  json_object *json_obj_vbr = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj, "vbridge", -1 ,
                                                     json_obj_vbr);
  if (json_object_is_type(json_obj_vbr, json_type_null)) {
    json_object_put(jobj);
    pfc_log_error("%s jobj_vbr NULL", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error in parsing json object in %s", PFC_FUNCNAME);
    json_object_put(jobj);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if (json_object_is_type(json_obj_vbr, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj, "vbridge");
  }
  pfc_log_debug(" array _length %d" , array_length);
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    key_vbr_t key;
    val_vbr_t val;
    memset(&key, 0, sizeof(key_vbr_t));
    memset(&val, 0, sizeof(val_vbr_t));
    VbrParser obj;
    strncpy(reinterpret_cast<char*> (key.vtn_key.vtn_name), vtn_name.c_str(),
          sizeof(key.vtn_key.vtn_name) - 1);
    json_object* json_idx_obj = NULL;
    json_idx_obj = json_object_array_get_idx(json_obj_vbr, arr_idx);
    pfc_log_debug("Json_obj_idx %s", restjson::JsonBuildParse::get_json_string(json_idx_obj));
    UncRespCode retval = obj.parse_vbr_response(json_idx_obj, key, val);
    if (UNC_RC_SUCCESS != retval) {
      pfc_log_error("Error in parsing vbr resp data in %s", PFC_FUNCNAME);
      json_object_put(jobj);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    std::string controller_name = ctr->get_controller_id();
    if (0 == strlen(controller_name.c_str())) {
      val.valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
    } else {
      strncpy(reinterpret_cast<char*>(val.controller_id),
            controller_name.c_str(), sizeof(val.controller_id) - 1);
      pfc_log_debug(" %s controller id",
                  reinterpret_cast<char*>(val.controller_id));
      val.valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
    }
    strncpy(reinterpret_cast<char*> (val.domain_id), DOM_NAME.c_str(),
          sizeof(val.domain_id) - 1);
    val.valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;

    unc::vtndrvcache::ConfigNode *cfgptr =
       new unc::vtndrvcache::CacheElementUtil<key_vbr_t, val_vbr_t, uint32_t>
      (&key, &val, uint32_t(UNC_OP_READ));
    PFC_ASSERT(cfgptr != NULL);
    cfgnode_vector.push_back(cfgptr);
    json_object_put(json_idx_obj);
   }
  json_object_put(jobj);
  pfc_log_debug("cfgnode_vector.size: %d" ,
                static_cast<int>(cfgnode_vector.size()));
  return UNC_RC_SUCCESS;
}

}  // namespace odcdriver
}  // namespace unc
