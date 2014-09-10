/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef __ODC_REST_HH__
#define __ODC_REST_HH__

#include <json_read_util.hh>
#include <driver/driver_interface.hh>
#include <rest_util.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/unc_base.h>
#include <vector>
#include <string>
#include <set>

namespace unc {
namespace odcdriver {

typedef enum {
  CONFIG_CREATE = 0,
  CONFIG_UPDATE = 1,
  CONFIG_DELETE = 2,
  CONFIG_READ = 3
}OdcDriverOps;

// Interface Class for ODC REST requests
class odl_http_rest_intf {
 public:
  // Is multiple requests need to be sent to handle the request?
  virtual pfc_bool_t is_multiple_requests(unc::odcdriver::OdcDriverOps Op)=0;

  // Provide a list of request indicators for multiple request scenario.
  virtual UncRespCode get_multi_request_indicator(
      unc::odcdriver::OdcDriverOps Op,
      std::set<std::string> *arg_list)=0;

  // Construct the URL for the operation
  // The request indicator will ne SINGLE as default
  virtual UncRespCode construct_url(unc::odcdriver::OdcDriverOps Op,
                                    std::string &request_indicator,
                                    std::string &url)=0;

  // Construct Request body for the operation
  virtual UncRespCode construct_request_body(unc::odcdriver::OdcDriverOps Op,
                                             std::string &request_indicator,
                                             json_object *object)=0;

  // Return the HTTP operation intended
  virtual restjson::HttpMethod get_http_method(
      unc::odcdriver::OdcDriverOps Op,
      std::string &request_indicator) = 0;

  // Validate the response code handed
  virtual UncRespCode validate_response_code(unc::odcdriver::OdcDriverOps Op,
                                             std::string &request_indicator,
                                             int resp_code)=0;


  // Read the response of the http request from data
  virtual UncRespCode handle_response(unc::odcdriver::OdcDriverOps Op,
                                      std::string &request_indicator,
                                      char* data)=0;
};


class odl_http_request {
 private:
  UncRespCode handle_request_internal(
      unc::driver::controller *ctr_ptr,
      unc::odcdriver::OdcDriverOps Op,
      unc::odcdriver::odl_http_rest_intf*,
      unc::restjson::ConfFileValues_t conf_values,
      std::string &request_indicator);



 public:
  UncRespCode handle_request(unc::driver::controller *ctr_ptr,
                             unc::odcdriver::OdcDriverOps Op,
                             unc::odcdriver::odl_http_rest_intf*,
                             unc::restjson::ConfFileValues_t conf_values);
};
}  // namespace odcdriver
}  // namespace unc
#endif
