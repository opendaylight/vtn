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

#ifndef RESTJSON_HTTP_CLIENT_H_
#define RESTJSON_HTTP_CLIENT_H_

#include <curl/curl.h>
#include <pfc/debug.h>
#include <pfc/log.h>
#include <rest_common_defs.hh>
#include <json_build_parse.hh>
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
   * @brief  - Method to Execute the Request for restClientStruct
   *            and RestClientHeader structue
   * @retval - Curlrest_resp_code_t - 0 for SUCCESS, 1 for FAILURE
   */
  rest_resp_code_t execute();

  /**
   * @brief   - Method to get the response code from server
   * @ret_val - HttpResponse_t - structure which contains the
   *            response code from server
   */
  HttpResponse_t get_http_resp_code();

  /**
   * @brief  - Method to get the respnse body from server
   * @retval - HttpContent_t Structure which contains response body from server
   */
  HttpContent_t* get_http_resp_body();

  /**
   * @brief      - Method to set the request Header
   * @param[in]  - requestheader
   * @retval     - SUCCESS/ FAILURE
   */
  rest_resp_code_t set_request_header(const std::string &requestheader);

  /**
   * @brief      - Method to set the user name and password
   * @param[in]  - username
   * @param[in]  - password
   * @retval     - SUCCESS/ FAILURE
   */
  rest_resp_code_t set_username_password(const std::string &username,
                                         const std::string &password);
  /**
   * @brief      - Method to set the connection TimeOut
   * @param[in]  - connectionTimeOut
   * @retval     - SUCCESS/ FAILURE
   */
  rest_resp_code_t set_connection_timeout(const int connectionTimeOut);

  /**
   * @brief      - Method to set the request TimeOut
   * @param[in]  - request TimeOut
   * @retval     - SUCCESS/ FAILURE
   */
  rest_resp_code_t set_request_timeout(const int reqTimeOut);

  /**
   * @brief      - Method to set the type of operation
   * @param[in]  - operation
   * @retval     - SUCCESS/ FAILURE
   */
  rest_resp_code_t set_operation_type(const HttpMethod operation);

  /**
   * @brief      - Method to set the request body
   * @param[in]  - requestbody
   * @retval     - SUCCESS/ FAILURE
   */
  rest_resp_code_t set_request_body(const char* req);

 private:
  /**
   * @brief   - Method to set Common parameters like Time out using
   *            RestClientHeader structue and CurlCommandHeader structure
   * @retval  - Curlrest_resp_code_t - 0 for SUCCESS, 1 for FAILURE
   */
  rest_resp_code_t set_opt_common();

  /**
   * @brief  - Method to send http Request
   * @retval - Curlrest_resp_code_t - 0 for SUCCESS, 1 for FAILURE
   */
  rest_resp_code_t perform_http_req();

  /**
   * @brief   - Method to Write the Response from Server
   * @retval  - Curlrest_resp_code_t - 0 for SUCCESS, 1 for FAILURE
   */
  rest_resp_code_t set_opt_write_data();

  /**
   * @brief      - Callback Method  used to write response data
   * @param[in]  - size, nmemb
   * @param[out] - ptr, data
   * @retval     - size_t
   */
  static size_t write_call_back(void* ptr, size_t size, size_t nmemb,
                                void* data);

 private:
  CURL* handle_;
  HttpResponse_t *response_;
  struct curl_slist *slist_;
};
}  // namespace restjson
}  // namespace unc
#endif  // RESTJSON_HTTP_CLIENT_H_
