/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <string.h>
#include <rest_client.hh>

namespace unc {
namespace restjson {

// Constructor
RestClient::RestClient(const std::string &ipaddress,
                       const std::string url,
                       const uint32_t port,
                       const HttpMethod method)
: m_ip_address_(ipaddress),
  m_url_(url),
  m_port_(port),
  m_method_(method) {
  ODC_FUNC_TRACE;
  http_client_obj_ = new HttpClient();
  PFC_ASSERT(http_client_obj_ != NULL);
  http_client_obj_->init();
}

// Destructor
RestClient::~RestClient() {
  ODC_FUNC_TRACE;
  if (http_client_obj_ != NULL) {
    http_client_obj_->fini();
    delete http_client_obj_;
    http_client_obj_ = NULL;
  }
}

// Send the Username and password to SetUserNamePassword
rest_resp_code_t RestClient::set_username_password(const std::string &user_name,
                                                const std::string &pass_word) {
  ODC_FUNC_TRACE;
  if (((user_name).empty()) || ((pass_word).empty())) {
    pfc_log_error("Invalid UserName or Password ");
    return REST_OP_FAILURE;
  }

  PFC_ASSERT(NULL != http_client_obj_);
  rest_resp_code_t retval = http_client_obj_->set_username_password(user_name,
                                                                    pass_word);
  return retval;
}

// Send the timeout values to HttpClient Class
rest_resp_code_t RestClient::set_timeout(const int connection_time_out,
                                         const int request_time_out) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(NULL != http_client_obj_);
  rest_resp_code_t retval = http_client_obj_->set_connection_timeout(
      connection_time_out);
  if (REST_OP_FAILURE == retval) {
    pfc_log_error("Error while setting connection timeout");
    return retval;
  }
  retval = http_client_obj_->set_request_timeout(request_time_out);
  return retval;
}

// Create Request Header and send to HttpClient Class
rest_resp_code_t RestClient::create_request_header() {
  ODC_FUNC_TRACE;
  PFC_ASSERT(NULL != http_client_obj_);
  if ((HTTP_METHOD_POST != m_method_) &&
      (HTTP_METHOD_PUT != m_method_) &&
      (HTTP_METHOD_DELETE != m_method_) &&
      (HTTP_METHOD_GET != m_method_)) {
    pfc_log_error("Invalid Method : %d", m_method_);
    return REST_OP_FAILURE;
  }

  if (m_url_.empty()) {
    pfc_log_error("Invalid Url");
    return REST_OP_FAILURE;
  }

  if (m_ip_address_.empty()) {
    pfc_log_error("Invalid ipaddress");
    return REST_OP_FAILURE;
  }

  if (0 == m_port_) {
    pfc_log_error("Invalid port number");
    return REST_OP_FAILURE;
  }

  std::string request_url = "";
  request_url = PROTOCOL;
  request_url.append(m_ip_address_);
  request_url.append(COLON);
  std::ostringstream port_number_str_format;
  port_number_str_format << m_port_;
  request_url.append(port_number_str_format.str());
  request_url.append(m_url_);

  rest_resp_code_t retval = http_client_obj_->set_url(request_url);
  if (REST_OP_SUCCESS != retval) {
    pfc_log_error("Error in set url");
    return REST_OP_FAILURE;
  }

  pfc_log_debug("%s Url formed ", request_url.c_str());
  retval = http_client_obj_->set_operation_type(m_method_);
  return retval;
}

// Sends http request to the http client util it sends to VTN Manager
HttpResponse_t* RestClient::send_http_request(std::string username,
                                              std::string password,
                                              uint32_t connect_time_out,
                                              uint32_t req_time_out,
                                              const char* request_body) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(http_client_obj_ != NULL);
  rest_resp_code_t retval = REST_OP_FAILURE;
  retval = create_request_header();
  if (REST_OP_SUCCESS != retval) {
    pfc_log_error("Error in create request header in %s", PFC_FUNCNAME);
    clear_http_response();
    return NULL;
  }

  retval = set_username_password(username, password);
  if (REST_OP_SUCCESS != retval) {
    pfc_log_error("Error in set username password in %s", PFC_FUNCNAME);
    clear_http_response();
    return NULL;
  }

  retval = set_timeout(connect_time_out, req_time_out);
  if (REST_OP_SUCCESS != retval) {
    pfc_log_error("Error in set timeout in %s", PFC_FUNCNAME);
    clear_http_response();
    return NULL;
  }

  if ((HTTP_METHOD_POST == m_method_) ||
      (HTTP_METHOD_PUT == m_method_)) {
    if (NULL != request_body) {
      pfc_log_debug("Request Body  : %s", request_body);
      retval = set_request_body(request_body);
      if (REST_OP_SUCCESS != retval) {
        pfc_log_error("Error in set Request Body %s", PFC_FUNCNAME);
        clear_http_response();
        return NULL;
      }
    }
  }
  retval = http_client_obj_->send_request();
  if (REST_OP_SUCCESS != retval) {
    pfc_log_error("Error in send request %s", PFC_FUNCNAME);
    clear_http_response();
    return NULL;
  }
  return http_client_obj_->get_http_response();
}

// Invokes HttpClient Class SetRequestBody Method
rest_resp_code_t RestClient::set_request_body(const char* req_body) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(NULL != http_client_obj_);
  rest_resp_code_t ret_val = http_client_obj_->set_request_body(req_body);
  return ret_val;
}

// Clears http response memory allocated
void RestClient::clear_http_response() {
  ODC_FUNC_TRACE;
  PFC_ASSERT(NULL != http_client_obj_);
  http_client_obj_->clear_http_response();
}
}  // namespace restjson
}  // namespace unc
