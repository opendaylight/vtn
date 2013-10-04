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

namespace restjson {

class RestClient {
  public:
    /**
     ** Constructor get ipaddress in string format, port number as input
     **/
    RestClient(const std::string &ipaddress, const int portnumber);

    /**
     * Destructor
     **/
    ~RestClient();

    /**
     ** Method to set the username password to authenticate request
     ** @param [in] - username
     ** @param [in] - password
     **/
    rest_resp_code_t set_username_password(const std::string &username,
                                 const std::string &password);

    /**
     * Method to set the Connection timeout and request time out
     * @param[in] - connectionTimeOut
     * @param[in] - reqTimeOut
     */
    rest_resp_code_t set_timeout(const int connectionTimeOut,
                                 const int reqTimeOut);

    /**
     * Method to Create Request Header
     * @param [in] - operation - Operation 1- POST, 2 for PUT, 3 for DELETE, 4- GET
     ** @param [in] -url - specifies the url
     **/

    rest_resp_code_t create_request_header(const std::string &url,
                                 const HttpMethod operation);

    /**
     ** Method to Request for Create Operation - id and value as input
     ** @param[in] - map idValuePair - contains the request body as id and value pair
     ** @param[in] - delimiter
     ** @param[out] - SUCCESS/ FAILURE
     **/
    rest_resp_code_t set_request_body(const char* str);

    /**
     * Method to Send Request and Get Response Code
     * param[out] - SUCCESS/ FAILURE
     */
    uint32_t send_request_and_get_response_code();

  private:
    HttpClient *http_client_obj_;
    const std::string mipaddress_;
    const int mportnumber_;
};
}
#endif  // RESTJSON_REST_CLIENT_H_
