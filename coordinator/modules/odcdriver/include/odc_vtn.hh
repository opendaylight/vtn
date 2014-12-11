/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_VTN_HH_
#define _ODC_VTN_HH_

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odl_vtn.hh>
#include <odc_flowfilter_template.hh>
#include <odc_rest.hh>
#include <unc/upll_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <tclib_module.hh>
#include <vector>
#include <string>
#include <sstream>

namespace unc {
namespace odcdriver {

class OdcVtnCommand : public unc::driver::vtn_driver_command
  <key_vtn_t, val_vtn_t>, public VtnParser {
private:
  unc::restjson::ConfFileValues_t conf_values_;
  uint32_t idle_timeout_;
  uint32_t hard_timeout_;

public:
  OdcVtnCommand(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values), idle_timeout_(DEFAULT_IDLE_TIME_OUT),
    hard_timeout_(DEFAULT_HARD_TIME_OUT){}

  UncRespCode
  create_cmd(key_vtn_t& key, val_vtn_t& val,
             unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);
    std::string url = "";
    url.append(get_base_url());
    url.append(get_url_tail(key, val));

    json_object *jobj_req_body = unc::restjson::JsonBuildParse::create_json_obj();
    int retval = create_vtn_request(jobj_req_body, key, val);
    if (retval != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC; 

    retval = set_vtn_idletime(jobj_req_body, key, val);
    if (retval != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC; 

    retval = set_vtn_hardtime(jobj_req_body, key, val);
    if (retval != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC; 

    pfc_log_debug("Request body------------>%s", unc::restjson::JsonBuildParse::get_json_string(jobj_req_body));


    unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());

    unc::restjson::HttpResponse_t* response =
         rest_util_obj.send_http_request(
         url, restjson::HTTP_METHOD_POST,
         unc::restjson::JsonBuildParse::get_json_string(jobj_req_body),
         conf_values_);
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

  UncRespCode
  update_cmd(key_vtn_t& key, val_vtn_t& val,
             unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr != NULL);
    std::string url = "";
    url.append(get_base_url());
    url.append(get_url_tail(key, val));

    std::string vtn_name_ = reinterpret_cast<char *>(key.vtn_name);
     if (0 == strlen(vtn_name_.c_str()))
       return UNC_DRV_RC_ERR_GENERIC;

    json_object *jobj = unc::restjson::JsonBuildParse::create_json_obj();
    //OdlVtn obj;
    int retval = create_vtn_request(jobj, key, val);
    if (retval != UNC_RC_SUCCESS)

    retval = set_vtn_idletime(jobj, key, val);
    if (retval != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;

    retval = set_vtn_hardtime(jobj, key, val);
    if (retval != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;

    unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                  ctr->get_user_name(), ctr->get_pass_word());
    unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(
          url, restjson::HTTP_METHOD_PUT,
          unc::restjson::JsonBuildParse::get_json_string(jobj),
           conf_values_);

    json_object_put(jobj);
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

  UncRespCode
  delete_cmd(key_vtn_t& key, val_vtn_t& val,
             unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);
    std::string url = "";
    url.append(get_base_url());
    url.append(get_url_tail(key, val));
    
    std::string vtn_name = reinterpret_cast<char *>(key.vtn_name);
        if (0 == strlen(vtn_name.c_str()))
          return UNC_DRV_RC_ERR_GENERIC;

    unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                  ctr->get_user_name(), ctr->get_pass_word());
    unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
               url, restjson::HTTP_METHOD_DELETE, NULL, conf_values_);

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

  std::string get_url_tail(key_vtn_t &key_in,
                           val_vtn_t &val_in) {
    //char vtn_name[32];
    std::string url(reinterpret_cast<char*>(key_in.vtn_name));
    //url.append(vtn_name);
    return url;
  }

  std::string get_base_url() {
    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    return url;
  }

  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    return get_vtn_list(ctr, cfgnode_vector);
  }

  UncRespCode set_vtn_idletime(json_object *&in, key_vtn_t& key, val_vtn_t& val) { 
    int ret_val = unc::restjson::JsonBuildParse::build("idleTimeout", idle_timeout_, in);
    if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        json_object_put(in);
        return UNC_DRV_RC_ERR_GENERIC;
      }
    return UNC_RC_SUCCESS; 
  }

  UncRespCode set_vtn_hardtime(json_object *&in, key_vtn_t& key, val_vtn_t& val) { 
    int ret_val = unc::restjson::JsonBuildParse::build("hardTimeout", hard_timeout_, in);
    if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        json_object_put(in);
        return UNC_DRV_RC_ERR_GENERIC;
      }
    return UNC_RC_SUCCESS; 
  }

 UncRespCode get_vtn_list(unc::driver::controller* ctr,
                               std::vector<unc::vtndrvcache::ConfigNode *>
                               &cfgnode_vector);
 
 UncRespCode parse_vtn_response(char *data,
                                std::vector< unc::vtndrvcache
                                ::ConfigNode *>
                                &cfgnode_vector);
};

}
}
#endif
