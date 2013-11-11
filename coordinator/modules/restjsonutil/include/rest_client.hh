/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef RESTJSON_REST_CLIENT_H_
#define RESTJSON_REST_CLIENT_H_

#include <http_client.hh>
#include <uncxx/odc_log.hh>
#include <string>
#include <sstream>


namespace unc {
namespace restjson {

class RestClient {
 public:
  /**
   * @brief      - Constructor get ipaddress in string format
   *               as input
   *               Allocates memory for httpclient object
   */
  RestClient(const std::string &ipaddress, const std::string url,
                      const uint32_t port, const HttpMethod method);

  /**
   * @brief      - Destructor
   *             - clears memory allocated for jttp_client_obj
   */
  ~RestClient();

  /**
   * @brief       - Send the http request to http client
   * @param[in]   - username in string
   * @param[in]   - password in string
   * @param[in]   - connection time out in seconds
   * @param[in]   - request time out in seconds
   * @param[in]   - request body in char*
   */
  HttpResponse_t* send_http_request(std::string username, std::string password,
                              uint32_t connect_timeout, uint32_t req_time_out,
                              const char* request_body);
  /**
   * @brief - clears the http response memory allocated
   */
  void clear_http_response();

 private:
  /**
   * @brief                  - Method to set the username password to
   *                           authenticate request
   * @param [in] user_name    - username in string
   * @param [in] pass_word    - password in string
   * return rest_resp_code_t - returns REST_OP_SUCCESS on successfully setting username
   *                           and password/returns REST_OP_FAILURE on failure
   */
  rest_resp_code_t set_username_password(const std::string &user_name,
                                         const std::string &pass_word);

  /**
   * @brief                          - Method to set the Connection timeout and
   *                                   request time out
   * @param[in] connection_time_out  - connection timeout in seconds
   * @param[in] request_time_out     - request timeout in seconds
   * return rest_resp_code_t         - returns REST_OP_SUCCESS on setting timeout
   *                                  successfully/ returns REST_OP_FAILURE on failure
   */
  rest_resp_code_t set_timeout(const int connection_time_out,
                               const int request_time_out);

  /**
   * @brief                      - Method to Create Request Header
   * @param [in] url             - specifies the url
   * @param [in] operation       - Http method
   * return rest_resp_code_t     - returns REST_OP_SUCCESS on creating request header
   *                               successfully/returns REST_OP_FAILURE on failure
   */
  rest_resp_code_t create_request_header();

  /**
   * @brief                       - Method to set the request body
   * @param[in] str               - request body ( For instance,the request body
   *                                can be:" {"vbridge" :
   *                                {"vbr_name":"vbr1","controller_id":"odc1",
   *                                "domain_id":"Default" }}"
   * @param[out]                  - returns REST_OP_SUCCESS on successfully setting the
   *                                request body/ returns REST_OP_FAILURE on failure
   */
  rest_resp_code_t set_request_body(const char* str);

 private:
  const std::string m_ip_address_;
  const std::string m_url_;
  const uint32_t m_port_;
  const HttpMethod m_method_;
  HttpClient *http_client_obj_;
};
}  // namespace restjson
}  // namespace unc
#endif  // RESTJSON_REST_CLIENT_H_
