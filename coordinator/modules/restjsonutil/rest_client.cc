/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made
 * available under the
 * terms of the Eclipse Public License v1.0 which
 * accompanies this
 * distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <string.h>
#include <rest_client.hh>

namespace unc {
namespace restjson {

// Constructor
RestClient::RestClient(const std::string &ipaddress, const int portnumber)
: mipaddress_(ipaddress),
  mportnumber_(portnumber) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  http_client_obj_ = new HttpClient();
  http_client_obj_->init();
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
}

// Destructor
RestClient::~RestClient() {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  if (NULL != http_client_obj_) {
    http_client_obj_->fini();
    delete http_client_obj_;
    http_client_obj_ = NULL;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
}

// Send the Username and password to SetUserNamePassword
rest_resp_code_t RestClient::set_username_password(const std::string &username,
                                         const std::string &password) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);

  if ((0 == strlen(username.c_str())) || (0 == strlen(password.c_str()))) {
    pfc_log_error("Invalid UserName or Password ");
    return FAILURE;
  }

  PFC_ASSERT(NULL != http_client_obj_);
  rest_resp_code_t retval = http_client_obj_->set_username_password(username,
                                                                    password);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return retval;
}

// Send the timeout values to HttpClient Class
rest_resp_code_t RestClient::set_timeout(const int connectionTimeOut,
                                const int reqTimeOut) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  rest_resp_code_t retval = http_client_obj_->set_connection_timeout(
                                              connectionTimeOut);
  if (FAILURE == retval) {
    return FAILURE;
  }

  PFC_ASSERT(NULL != http_client_obj_);
  retval = http_client_obj_->set_request_timeout(reqTimeOut);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return retval;
}

// Create Request Header and send to HttpClient Class
rest_resp_code_t RestClient::create_request_header(const std::string &url,
                                         const HttpMethod operation) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  if ((HTTP_METHOD_POST != operation) && (HTTP_METHOD_PUT != operation)
      && (HTTP_METHOD_DELETE != operation) && (HTTP_METHOD_GET != operation)) {
    pfc_log_error("Invalid operation");
    return FAILURE;
  }

  if (0 == strlen(url.c_str())) {
    pfc_log_error("Invalid Url");
    return FAILURE;
  }

  if (0 == strlen(mipaddress_.c_str())) {
    pfc_log_error("Invalid ipaddress");
    return FAILURE;
  }

  if ( 0 == mportnumber_ ) {
     pfc_log_error("Invalid port number");
     return FAILURE;
  }

  std::string requestHeader = "";
  requestHeader = kProtocol;
  requestHeader.append(mipaddress_);
  requestHeader.append(":");
  std::ostringstream convert;
  convert << mportnumber_;
  requestHeader.append(convert.str());
  requestHeader.append(url);

  PFC_ASSERT(NULL != http_client_obj_);

  rest_resp_code_t retval = http_client_obj_->set_request_header(requestHeader);
  if (FAILURE == retval) {
    return FAILURE;
  }
  pfc_log_info("%s requestHeader", requestHeader.c_str());

  retval = http_client_obj_->set_operation_type(operation);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return retval;
}

// Send the Request and get Response code from the HttpClient Class
uint32_t RestClient::send_request_and_get_response_code() {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(NULL != http_client_obj_);
  rest_resp_code_t ret_val = http_client_obj_->execute();
  if (SUCCESS != ret_val) {
    return ret_val;
  }

  HttpResponse_t respStructure = http_client_obj_->get_http_resp_code();
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return respStructure.code;
}

// Invokes HttpClient Class SetRequestBody Method
rest_resp_code_t RestClient::set_request_body(const char* reqbody) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  PFC_ASSERT(NULL != http_client_obj_);

  rest_resp_code_t ret_val = http_client_obj_->set_request_body(reqbody);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return ret_val;
}
}
}
