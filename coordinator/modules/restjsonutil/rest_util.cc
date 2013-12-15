/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <rest_util.hh>

namespace unc {
namespace restjson {

//  Parametrised Constructor
RestUtil::RestUtil(const std::string &ipaddress,
                   const std::string &ctr_user_name,
                   const std::string &ctr_password)
: rest_client_obj_(NULL),
  ipaddress_(ipaddress),
  ctr_user_name_(ctr_user_name),
  ctr_password_(ctr_password) {
}


// Destructor Clear http response and delete rest client object
RestUtil::~RestUtil() {
  if (rest_client_obj_ != NULL) {
    rest_client_obj_->clear_http_response();
    delete rest_client_obj_;
    rest_client_obj_ = NULL;
  }
}

//  Sends request to restjson module, which sends to VTN Manager
unc::restjson::HttpResponse_t* RestUtil::send_http_request(
            const std::string &url, const unc::restjson::HttpMethod method,
          const char* request, const ConfFileValues_t &conf_file_values_) {
  ODC_FUNC_TRACE;
  std::string user_name = "";
  std::string password = "";
  get_username_password(conf_file_values_, user_name, password);
  if (rest_client_obj_ != NULL) {
    pfc_log_debug("Rest object already exists");
    rest_client_obj_->clear_http_response();
    delete rest_client_obj_;
    rest_client_obj_ = NULL;
  }
  rest_client_obj_ = new unc::restjson::RestClient(
      ipaddress_, url, conf_file_values_.odc_port, method);
  PFC_ASSERT(rest_client_obj_ != NULL);
  unc::restjson::HttpResponse_t* response =
      rest_client_obj_->send_http_request(user_name, password,
                                conf_file_values_.connection_time_out,
                                conf_file_values_.request_time_out, request);
  return response;
}

//  Gets user name password from controller , if it is empty then conf file
//  values are taken
void RestUtil::get_username_password(
    const ConfFileValues_t &conf_file_values_,
    std::string& user_name, std::string& password) {
  ODC_FUNC_TRACE;

  //  If Controller values are empty then conf files values are taken
  if ((ctr_user_name_.empty()) || (ctr_password_.empty())) {
    pfc_log_debug("User name and password is taken from conf file");
    user_name = conf_file_values_.user_name;
    password = conf_file_values_.password;
  } else {
    pfc_log_debug("User name and password is taken from controller pointer");
    user_name = ctr_user_name_;
    password = ctr_password_;
  }
}
}  // namespace restjson
}  // namespace unc
