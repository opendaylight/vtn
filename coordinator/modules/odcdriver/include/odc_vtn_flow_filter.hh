/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __VTN_FLOWFILTER_HH__
#define __VTN_FLOWFILTER_HH__

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odc_util.hh>
#include <odc_rest.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <tclib_module.hh>
#include <odl_vtn_flowfilter.hh>

namespace unc {
namespace odcdriver {

class OdcVtnFlowFilterCmd : public unc::driver::vtn_driver_command
  <key_vtn_flowfilter,val_flowfilter>, public VtnFlowfilterParser
{
private:
  std::string parent_vtn_name_;
  unc::restjson::ConfFileValues_t conf_values_;

public:
  OdcVtnFlowFilterCmd (unc::restjson::ConfFileValues_t conf_values):
    parent_vtn_name_(""), conf_values_(conf_values){}
  UncRespCode
  create_cmd(key_vtn_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr) {
    if ( key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
      return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  update_cmd(key_vtn_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr) {
    if ( key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
      return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  delete_cmd(key_vtn_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr_ptr) {
    if ( key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
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
    int resp_code = send_httprequest(ctr_ptr, url,conf_values_,  restjson::HTTP_METHOD_DELETE,NULL);
    if (0 == resp_code) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    //int resp_code = response->code;
    pfc_log_debug("response code returned in create vtn is %d", resp_code);
    if (HTTP_200_RESP_OK != resp_code)
      return UNC_DRV_RC_ERR_GENERIC;
    return UNC_RC_SUCCESS;
  }

  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    key_vtn_flowfilter_entry key;
    val_vtn_flowfilter_entry val;
    key_vtn_t* parent_vtn = reinterpret_cast<key_vtn_t*> (parent_key);
    std::string url("");
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    url.append(reinterpret_cast<char*>(parent_vtn->vtn_name));
    url.append("/");
    url.append("flowfilters");
    parent_vtn_name_.assign(reinterpret_cast<char*>(parent_vtn->vtn_name));
    
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
        pfc_log_debug("vtn flowfilter present : %s", data);
        pfc_log_debug("vtn flowfilter parent_name : %s", parent_vtn_name_.c_str());
        json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
        VtnFlowfilterParser obj(parent_vtn_name_);
        UncRespCode ret_val = obj.parse_vtn_flowfilter_response(jobj,key,val,cfgnode_vector);
        return ret_val;
      }
    }
    return UNC_RC_SUCCESS;
  }

  std::string get_url_tail(key_vtn_flowfilter &key_in,
                           val_flowfilter &val_in) {
    std::string url(reinterpret_cast<char*>(key_in.vtn_key.vtn_name));
    url.append("/");
    url.append("flowfilters");
    return url;
  }
};


class OdcVtnFlowFilterEntryCmd : public unc::driver::vtn_driver_command
  <key_vtn_flowfilter_entry,val_vtn_flowfilter_entry>, public VtnFlowfilterParser {
private:
  unc::restjson::ConfFileValues_t conf_values_;

public:
  OdcVtnFlowFilterEntryCmd(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values) {}

  UncRespCode
  create_cmd(key_vtn_flowfilter_entry& key, val_vtn_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr) {
    if ( key.flowfilter_key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
      return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;

    json_object *jobj_req_body = unc::restjson::JsonBuildParse::create_json_obj();
    pfc_log_debug("entering in to create_vtn_flowfilter_request");
    int retval = create_vtn_flowfilter_request(jobj_req_body, key, val);
    pfc_log_debug("leaving from create_vtn_flowfilter_request");
    if (retval != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;

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
         url, restjson::HTTP_METHOD_PUT,
         unc::restjson::JsonBuildParse::get_json_string(jobj_req_body),
         conf_values_);*/
    int resp_code = send_httprequest(ctr_ptr, url, conf_values_, restjson::HTTP_METHOD_PUT, jobj_req_body);
    pfc_log_debug("the url is:%s",url.c_str());
    json_object_put(jobj_req_body);
    if (0 == resp_code) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
   // int resp_code = response->code;
    pfc_log_debug("response code returned in create vtn flowfilter is %d", resp_code);
    if (HTTP_201_RESP_CREATED != resp_code) {
      return UNC_DRV_RC_ERR_GENERIC;
    }
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  update_cmd(key_vtn_flowfilter_entry& key, val_vtn_flowfilter_entry& val,
             unc::driver::controller *ctr) {
    if ( key.flowfilter_key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
      return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  delete_cmd(key_vtn_flowfilter_entry& key, val_vtn_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr) {
    if ( key.flowfilter_key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
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
    int resp_code = send_httprequest(ctr_ptr,url,conf_values_,restjson::HTTP_METHOD_DELETE,NULL);
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

  std::string get_url_tail(key_vtn_flowfilter_entry &key_in,
                           val_vtn_flowfilter_entry &val_in) {
    char index[10];
    std::string url(reinterpret_cast<char*>(key_in.flowfilter_key.vtn_key.vtn_name));
    url.append("/");
    url.append("flowfilters");
    url.append("/");
    /*if ( key_in.flowfilter_key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
     url.append("out");
    else
     url.append("in");
    url.append("/");*/
    sprintf(index,"%d",key_in.sequence_num);
    url.append(index);
    return url;
  }

};

}
}

#endif
