/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef RESTJSON_HTTP_CLIENT_H_
#define RESTJSON_HTTP_CLIENT_H_

#include <curl/curl.h>
#include <pfc/debug.h>
#include <pfc/log.h>
#include <rest_common_defs.hh>
#include <json_build_parse.hh>
#include <uncxx/odc_log.hh>
#include <string>


namespace unc {
namespace restjson {

class HttpClient {
 public:
  /**
   * @brief  Default Constructor.
   */
  HttpClient();

  /**
   * @brief  Default Destructor.
   */
  ~HttpClient();

  /**
   * @brief  - Initialises curl handle
   * @retval - None
   */
  void init();

  /**
   * @brief  - Fini Method to release handle
   * @retval - None
   */
  void fini();

  /**
   * @brief  - Method to Send the Request to the controller
   * @retval - rest_resp_code_t - 0 for REST_OP_SUCCESS, 1 for REST_OP_FAILURE
   */
  rest_resp_code_t send_request();

  /**
   * @brief      - Method to set the url to curl handle
   * @param[in]  - url in string
   * @retval     - rest_resp_code_t 0 -REST_OP_SUCCESS 1- REST_OP_FAILURE
   */
  rest_resp_code_t set_url(const std::string &url);

  /**
   * @brief      - Method to set the user name and password
   * @param[in]  - username - in string
   * @param[in]  - password - in string
   * @retval     - rest_resp_code_t  0- REST_OP_SUCCESS 1- REST_OP_FAILURE
   */
  rest_resp_code_t set_username_password(const std::string &user_name,
                                         const std::string &pass_word);
  /**
   * @brief      - Method to set the connection_time_out
   * @param[in]  - connection_time_out in seconds
   * @retval     - rest_resp_code_t 0 - REST_OP_SUCCESS 1- REST_OP_FAILURE
   */
  rest_resp_code_t set_connection_timeout(const uint32_t connection_time_out);

  /**
   * @brief      - Method to set the request TimeOut
   * @param[in]  - request TimeOut in seconds
   * @retval     - rest_resp_code_t  0 -REST_OP_SUCCESS 1- REST_OP_FAILURE
   */
  rest_resp_code_t set_request_timeout(const uint32_t req_time_out);

  /**
   * @brief      - Method to set the type of operation
   * @param[in]  - operation - HTTP_METHOD_POST -1, HTTP_METHOD_PUT-2 ,
   *                           HTTP_METHOD_DELETE - 3, HTTP_METHOD_GET - 4
   * @retval     - rest_resp_code_t 0 - REST_OP_SUCCESS 1- REST_OP_FAILURE
   */
  rest_resp_code_t set_operation_type(const HttpMethod operation);

  /**
   * @brief      - Method to set the request body
   * @param[in]  - requestbody in string
   * @retval     - rest_resp_code_t 0 -REST_OP_SUCCESS 1- REST_OP_FAILURE
   */
  rest_resp_code_t set_request_body(const char* req_body);

  /**
   * @brief - Gets the response from the controller
   * return - HttpResponse_t structure which contains code and response body
   */
  HttpResponse_t* get_http_response();

  /**
   * @brief - clears the memory allocated
   * @return void
   */
  void clear_http_response();

 private:
  /**
   * @brief   - Method to set Common parameters like VERBORSE, NOSIGNAL,
   *            FOLLOW_LOCATION, NOPROGRESS
   * @retval  - rest_resp_code_t - 0 for REST_OP_SUCCESS, 1 for REST_OP_FAILURE
   */
  rest_resp_code_t set_opt_common();

  /**
   * @brief  - Method to send http Request
   * @retval - rest_resp_code_t - 0 for REST_OP_SUCCESS, 1 for REST_OP_FAILURE
   */
  rest_resp_code_t perform_http_req();

  /**
   * @brief   - Method to Write the Response from Controller
   * @retval  - rest_resp_code_t - 0 for REST_OP_SUCCESS, 1 for REST_OP_FAILURE
   */
  rest_resp_code_t set_opt_write_data();

  /**
   * @brief      - Callback Method  used to write response data
   * @param[in]  - ptr, - void pointer which contains the response from
   * controller
   * @param[in]  - size*nmemb -  values give the size of data pointed by ptr
   * @param[out] - data - void pointer to which the response in copied
   * @retval     - size_t - number of bytes used
   */
  static size_t write_call_back(void* ptr, size_t size, size_t nmemb,
                                void* data);

 private:
  CURL* handle_;
  HttpResponse_t *response_;
  curl_slist *slist_;
};
}  // namespace restjson
}  // namespace unc
#endif  // RESTJSON_HTTP_CLIENT_H_
