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

#ifndef RESTJSON_REST_CLIENT_H_
#define RESTJSON_REST_CLIENT_H_

#include <http_client.hh>
#include <string>
#include <sstream>

namespace unc {
namespace restjson {

class RestClient {
 public:
  /**
   * @brief      - Constructor get ipaddress in string format, port number
   *               as input
   */
  RestClient(const std::string &ipaddress, const int portnumber);

  /**
   * @brief      - Destructor
   */
  ~RestClient();

  /**
   * @brief                  - Method to set the username password to
   *                           authenticate request
   * @param [in] username    - username
   * @param [in] password    - password
   * return rest_resp_code_t - returns SUCCESS on successfully setting username
   *                           and password/returns FAILURE on failure
   */
  rest_resp_code_t set_username_password(const std::string &username,
                                         const std::string &password);

  /**
   * @brief                       - Method to set the Connection timeout and
   *                                request time out
   * @param[in] connectionTimeOut - connection timeout
   * @param[in] reqTimeOut        - request timeout
   * return rest_resp_code_t      - returns SUCCESS on setting timeout
   *                                successfully/ returns FAILURE on failure
   */
  rest_resp_code_t set_timeout(const int connectionTimeOut,
                               const int reqTimeOut);

  /**
   * @brief                      - Method to Create Request Header
   * @param [in] url             - specifies the url
   * return rest_resp_code_t     - returns SUCCESS on creating request header
   *                               successfully/returns FAILURE on failure
   */
  rest_resp_code_t create_request_header(const std::string &url,
                                         const HttpMethod operation);

  /**
   * @brief                       - Method to Request for Create Operation - id
   *                                and value as input
   * @param[in] str               - request body
   * @param[out]                  - returns SUCCESS on successfully setting the
   *                                request body/ returns FAILURE on failure
   */
  rest_resp_code_t set_request_body(const char* str);

  /**
   * @brief         - Method to Send Request and Get Response Code
   * param[out]     - returns SUCCESS on successfully sending request
   *                  and getting the response code /returns FAILURE on failure
   */
  uint32_t send_request_and_get_response_code();

  /**
   * @brief                     - Gets the response body
   * param[out] HttpContent_t   - returns the HttpContent_t* structure
   */
  HttpContent_t* get_response_body();


 private:
  HttpClient *http_client_obj_;
  const std::string mipaddress_;
  const int mportnumber_;
};
}  // namespace restjson
}  // namespace unc
#endif  // RESTJSON_REST_CLIENT_H_
