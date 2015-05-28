/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_FLOWFILTER_CMN_HH_
#define _ODC_FLOWFILTER_CMN_HH_

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odc_flow_filter.hh>
#include <odc_util.hh>
#include <odc_rest.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <tclib_module.hh>
#include <vector>
#include <string>
#include <cstdlib>

namespace unc {
namespace odcdriver {


class odl_flowfilter_http_request: public odl_http_rest_intf {

public:
  odl_flowfilter_http_request(flowfilter *value, std::string url,
                              flowfilterlist *read_value):
    url_(url),
    filter_(value),
    filter_list_(read_value) {}

  // Is multiple requests need to be sent to handle the request?
  pfc_bool_t is_multiple_requests(unc::odcdriver::OdcDriverOps Op) {
    return PFC_FALSE;
  }

// Provide a list of request indicators for multiple request scenario.
  UncRespCode get_multi_request_indicator(
    unc::odcdriver::OdcDriverOps Op,
    std::set<std::string> *arg_list) {
    return UNC_RC_SUCCESS;
  }

// Construct the URL for the operation
// The request indicator will ne SINGLE as default
  UncRespCode construct_url(unc::odcdriver::OdcDriverOps Op,
                            std::string &request_indicator,
                            std::string &url) {
    url=url_;
    return UNC_RC_SUCCESS;
  }

// Construct Request body for the operation
  UncRespCode construct_request_body(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     json_object *object) {
    if ( Op != unc::odcdriver::CONFIG_READ &&
         Op != unc::odcdriver::CONFIG_DELETE ) {
      unc::odcdriver::flowfilterlistUtil util_;
      UncRespCode ret(util_.SetValue(object,filter_));
      if ( ret != UNC_RC_SUCCESS ) {
        pfc_log_error("Failed to Copy from flowcondtion to json!!");
        return ret;
      }
    }
    return UNC_RC_SUCCESS;
  }
// Return the HTTP operation intended
  restjson::HttpMethod get_http_method(
    unc::odcdriver::OdcDriverOps Op,
    std::string &request_indicator) {

    if ( Op == unc::odcdriver::CONFIG_READ )
      return unc::restjson::HTTP_METHOD_GET;
    if ( Op == unc::odcdriver::CONFIG_CREATE )
      return unc::restjson::HTTP_METHOD_POST;
    if ( Op == unc::odcdriver::CONFIG_DELETE)
      return unc::restjson::HTTP_METHOD_DELETE;
    if ( Op == unc::odcdriver::CONFIG_UPDATE)
      return unc::restjson::HTTP_METHOD_PUT;
    return unc::restjson::HTTP_METHOD_GET;
  }

// Validate the response code handed
  UncRespCode validate_response_code(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     int resp_code) {
    pfc_log_info("Response Code Received %d",resp_code);
    if (HTTP_200_RESP_OK != resp_code &&
        HTTP_201_RESP_CREATED != resp_code &&
        HTTP_204_NO_CONTENT != resp_code) {
      return UNC_DRV_RC_ERR_GENERIC;
    }
    return UNC_RC_SUCCESS;
  }

// Read the response of the http request from data
  UncRespCode handle_response(unc::odcdriver::OdcDriverOps Op,
                              std::string &request_indicator,
                              char* data) {
    if ( Op == unc::odcdriver::CONFIG_READ ) {
      unc::odcdriver::flowfilterlistUtil util_;
      json_object *parse(unc::restjson::JsonBuildParse::get_json_object(data));
      if ( parse != NULL ) {
        // Clear memory when variable(parse) is out of scope
        unc::restjson::json_obj_destroy_util delete_obj(parse);
        UncRespCode ret(util_.GetValue( parse, filter_list_));
        if ( ret != UNC_RC_SUCCESS )
          return ret;
      } else {
        return UNC_DRV_RC_ERR_GENERIC;
      }
    }
    return UNC_RC_SUCCESS;
  }

private:
  std::string url_;
  flowfilter* filter_;
  flowfilterlist* filter_list_;
};

template <typename key,typename value>
class OdcFlowFilterCmd {

private:
  unc::restjson::ConfFileValues_t conf_values_;

public:
  OdcFlowFilterCmd(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values) {}

  ~OdcFlowFilterCmd() {}

  virtual std::string get_url_tail(key &key_in,value &val_in)=0;

  // Copy from Key Value Structure to FlowFilter Structure
  virtual void copy(flowfilter* out, key &key_in, value &value_in)=0;
  virtual void copy(flowfilter* out, key &key_in,
                           value &value_old_in, value &value_new_in)=0;

  // Copy from FlowFilter List and send to platform
  virtual UncRespCode r_copy(flowfilterlist* in,
               std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) =0;


  // Method to CREATE/DELETE
  UncRespCode run_command(key& key_in,
                          value& val_in,
                          unc::driver::controller *ctr,
                          unc::odcdriver::OdcDriverOps Op) {
    ODC_FUNC_TRACE;
    flowfilter command_;
    //Copy Contents from key and val
    pfc_log_error("CREATE FLOWFILTER ENTRY ENTERED!!");
    copy(&command_,key_in,val_in);

    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    url.append(get_url_tail(key_in,val_in));
    odl_flowfilter_http_request flow_filter_request(&command_,
        url,NULL);
    odl_http_request odl_fc_create;
    return odl_fc_create.handle_request(ctr,
                                        Op,
                                        &flow_filter_request,
                                        conf_values_);
  }
  // Method to UPDATE
  UncRespCode run_command(key& key_in,
                          value& val_old_in,
                          value& val_new_in,
                          unc::driver::controller *ctr,
                          unc::odcdriver::OdcDriverOps Op) {
    ODC_FUNC_TRACE;
    flowfilter command_;
    //Copy Contents from key and val
    copy(&command_,key_in,val_old_in, val_new_in);

    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    //url.append(get_url_tail(key_in,val_old_in));
    url.append(get_url_tail(key_in,val_new_in));
    odl_flowfilter_http_request flow_filter_request(&command_,
        url,NULL);

    odl_http_request odl_fc_create;
    return odl_fc_create.handle_request(ctr,
                                        Op,
                                        &flow_filter_request,
                                        conf_values_);
  }
  //Method to read all for AUDIT
  UncRespCode odl_flow_filter_read_all( unc::driver::controller *ctr,
                       std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
                                        std::string url) {
    ODC_FUNC_TRACE;
    flowfilterlist read_all_;
    pfc_log_info("The Flow list URL: %s",url.c_str());

    odl_flowfilter_http_request flow_filter_request(NULL,
        url,&read_all_);
    odl_http_request odl_fc_create;
    UncRespCode ret (odl_fc_create.handle_request(ctr,
                     unc::odcdriver::CONFIG_READ,
                     &flow_filter_request,
                     conf_values_));
    if ( ret != UNC_RC_SUCCESS )
      return ret;

    return r_copy(&read_all_,cfgnode_vector);
  }

};

}  // namespace odcdriver
}  // namespace unc
#endif
