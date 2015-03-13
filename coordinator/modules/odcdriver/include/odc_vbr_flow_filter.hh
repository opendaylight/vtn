/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VBR_FLOWFILTER_HH__
#define __VBR_FLOWFILTER_HH__

#include <odl_vbr_flowfilter.hh>
#include <string>
#include <list>
#include <algorithm>
#include <odc_kt_utils.hh>

namespace unc {
namespace odcdriver {

class OdcVbrFlowFilterCmd : public unc::driver::vtn_driver_command
  <key_vbr_flowfilter,val_flowfilter>, public ParseVbrFlowfilter
{
private:
  unc::restjson::ConfFileValues_t conf_values_;
  std::string parent_vtn_name_;
  std::string parent_vbr_name_;
  pfc_bool_t is_out;

public:
  OdcVbrFlowFilterCmd (unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values) {}
  
  UncRespCode
  create_cmd(key_vbr_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  update_cmd(key_vbr_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  delete_cmd(key_vbr_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr) {
 if ( key.direction == UPLL_FLOWFILTER_DIR_OUT)
          return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;

        std::string url = "";
        url.append(BASE_URL);
        url.append(CONTAINER_NAME);
        url.append(VTNS);
        url.append("/");
        url.append(get_url_tail(key, val));

        /*unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                      ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());

        unc::restjson::HttpResponse_t* response =
             rest_util_obj.send_http_request(
             url, restjson::HTTP_METHOD_DELETE, NULL,
             conf_values_);*/
        int resp_code = send_httprequest(ctr,url,conf_values_,restjson::HTTP_METHOD_DELETE,NULL);
        if (0 == resp_code) {
          pfc_log_error("Error Occured while getting httpresponse");
          return UNC_DRV_RC_ERR_GENERIC;
        }
        //int resp_code = response->code;
        pfc_log_debug("response code returned in delete vtn is %d", resp_code);
        if (HTTP_200_RESP_OK != resp_code)
          return UNC_DRV_RC_ERR_GENERIC;

    return UNC_RC_SUCCESS;
  }

  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    key_vbr_flowfilter_entry key;
    val_flowfilter_entry val;
    std::string redirect_node;
    key_vbr_t* parent_vbr = reinterpret_cast<key_vbr_t*> (parent_key);
    std::string url("");
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    url.append(reinterpret_cast<char*>(parent_vbr->vtn_key.vtn_name));
    url.append("/");
    url.append("vbridges");
    url.append("/");
    url.append(reinterpret_cast<char*>(parent_vbr->vbridge_name));
    url.append("/");
    url.append("flowfilters");
    url.append("/");
    url.append("in");
    parent_vtn_name_.assign(reinterpret_cast<char*>(parent_vbr->vtn_key.vtn_name));
    parent_vbr_name_.assign(reinterpret_cast<char*>(parent_vbr->vbridge_name));
    
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
        pfc_log_debug("vbr flowfilter present : %s", data);
        pfc_log_debug("vbr flowfilter parent_name : %s", parent_vbr_name_.c_str());
        json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
        ParseVbrFlowfilter obj(parent_vtn_name_, parent_vbr_name_,redirect_node);
        UncRespCode ret_val = obj.parse_vbr_flowfilter_response(jobj, key,val,cfgnode_vector);
        return ret_val;
      }
    }
    return UNC_RC_SUCCESS;
  }

  std::string get_url_tail(key_vbr_flowfilter &key_in,
                           val_flowfilter &val_in) {
    std::string vtn_name(reinterpret_cast<char*>(key_in.vbr_key.vtn_key.vtn_name));
    std::string vbr_name (reinterpret_cast<char*>(key_in.vbr_key.vbridge_name));

    std::string url(vtn_name);
    url.append("/");
    url.append("vbridges");
    url.append("/");
    url.append(vbr_name);
    url.append("/");
    url.append("flowfilters");
    if ( key_in.direction == UPLL_FLOWFILTER_DIR_OUT)
      url.append("/out");
    else
      url.append("/in");
    return url;
  }
};

class OdcVbrFlowFilterEntryCmd : public unc::driver::vtn_driver_command
  <key_vbr_flowfilter_entry,val_flowfilter_entry>, public ParseVbrFlowfilter {
private:
  unc::restjson::ConfFileValues_t conf_values_;
  std::set <std::string> bridges_;
  std::set <std::string> terminals_;

public:
  OdcVbrFlowFilterEntryCmd(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values) {}


  UncRespCode
  create_cmd(key_vbr_flowfilter_entry& key, val_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr) {
    
    std::string vtn_name(reinterpret_cast<char*>(key.flowfilter_key.vbr_key.vtn_key.vtn_name));
    
    if (val.valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID) {
      unc::odcdriver::odlutils::get_vbridge_names(ctr_ptr, conf_values_, vtn_name,
                                                  &bridges_);
      std::string redirect_node(reinterpret_cast <char *>(val.redirect_node));
      std::set <std::string>::iterator find_iter;
      find_iter = std::find (bridges_.begin(), bridges_.end(), redirect_node);
      if (find_iter == bridges_.end()) { 
        pfc_log_error("Redirect to bridge not found in the vbridge list");
        return UNC_DRV_RC_ERR_GENERIC;
      }
    }
      
    json_object *jobj_req_body = unc::restjson::JsonBuildParse::create_json_obj();
    int retval = create_vbr_flowfilter_request(jobj_req_body, key, val);
    if (retval != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;
    
    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    url.append(get_url_tail(key,val));
    
    unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());

    unc::restjson::HttpResponse_t* response =
         rest_util_obj.send_http_request(
         url, restjson::HTTP_METHOD_PUT,
         unc::restjson::JsonBuildParse::get_json_string(jobj_req_body),
         conf_values_);
    json_object_put(jobj_req_body);
    if (NULL == response) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    int resp_code = response->code;
    pfc_log_debug("response code returned in create vtn flowfilter is %d", resp_code);
    if (HTTP_201_RESP_CREATED != resp_code) {
      return UNC_DRV_RC_ERR_GENERIC;
    }
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  update_cmd(key_vbr_flowfilter_entry& key, val_flowfilter_entry& val,
             unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  delete_cmd(key_vbr_flowfilter_entry& key, val_flowfilter_entry& val,
             unc::driver::controller *ctr) {
  if ( key.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_OUT)
          return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;

        std::string url = "";
        url.append(BASE_URL);
        url.append(CONTAINER_NAME);
        url.append(VTNS);
        url.append("/");
        url.append(get_url_tail(key, val));

        /*unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                      ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());

        unc::restjson::HttpResponse_t* response =
             rest_util_obj.send_http_request(
             url, restjson::HTTP_METHOD_DELETE, NULL,
             conf_values_);*/
        int resp_code = send_httprequest(ctr,url,conf_values_,restjson::HTTP_METHOD_DELETE,NULL);
        if (0 == resp_code) {
          pfc_log_error("Error Occured while getting httpresponse");
          return UNC_DRV_RC_ERR_GENERIC;
        }
        //int resp_code = response->code;
        pfc_log_debug("response code returned in delete vtn is %d", resp_code);
        if (HTTP_200_RESP_OK != resp_code)
          return UNC_DRV_RC_ERR_GENERIC;
 
    return UNC_RC_SUCCESS;
  }

  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    return UNC_RC_SUCCESS;
  }

  std::string get_url_tail(key_vbr_flowfilter_entry &key_in,
                           val_flowfilter_entry &val_in) {
    char index[10];
    std::string url(reinterpret_cast<char*>(key_in.flowfilter_key.vbr_key.vtn_key.vtn_name));
    url.append("/");
    url.append("vbridges");
    url.append("/");
    url.append(reinterpret_cast<char*>(key_in.flowfilter_key.vbr_key.vbridge_name));
    url.append("/");
    url.append("flowfilters");
    url.append("/");
    if ( key_in.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_OUT)
      url.append("out");
    else
      url.append("in");
    url.append("/");
    sprintf(index,"%d",key_in.sequence_num);
    url.append(index);
    return url;
  }

};

}
}
#endif
