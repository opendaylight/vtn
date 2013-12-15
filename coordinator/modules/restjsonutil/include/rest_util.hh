/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef REST_UTIL_H
#define REST_UTIL_H

#include <rest_client.hh>
#include <string>

namespace unc {
namespace restjson {
class RestUtil {
 public:
  /**
   * @brief     - Parametrised Constructor
   * @param[in] - ipaddress in string
   * @param[in] - ctr_user_name in string
   * @param[in] - ctr_password in string
   */
  RestUtil(const std::string &ipaddress, const std::string &ctr_user_name,
                                         const std::string &ctr_password);

  /**
   * @brief      - Send http request to rest client class
   * @param[in]  - url to be sent
   * @param[in]  - method HTTP_METHOD_POST/ HTTP_METHOD_PUT/
   *               HTTP_METHOD_DELETE/HTTP_METHOD_GET
   * @param[in]  - request body
   * @param[in]  - conf_file values read from conf file
   * @return     - HttpResponse_t response structure
   */
  unc::restjson::HttpResponse_t* send_http_request(const std::string &url,
                                   const unc::restjson::HttpMethod method,
                   const char* request, const ConfFileValues_t &conf_file);

  /**
   * @brief     - Default Destructor
   */
  ~RestUtil();

 private:
  /**
   * @brief      - Gets user name pass word from controller or conf file
   * @param[in]  - conf_file_values_
   * @param[out] - user_name in string
   * @param[out] - password in string
   */
  void get_username_password(const ConfFileValues_t &conf_file_values_,
                          std::string& user_name, std::string& password);

 private:
  unc::restjson::RestClient *rest_client_obj_;
  const std::string ipaddress_;
  const std::string ctr_user_name_;
  const std::string ctr_password_;
};
}  // namespace restjson
}  // namespace unc
#endif
