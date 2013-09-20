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

#include <uncxx/restjson/rest_common_defs.hh>
#include <uncxx/restjson/json_build_parse.hh>
#include <uncxx/restjson/http_client.hh>
#include <string>
#include <sstream>

namespace librestjson {

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
    uint32_t SetUsernamePassword(const std::string &username,
                                 const std::string &password);

    /**
     * Method to set the Connection timeout and request time out
     * @param[in] - connectionTimeOut
     * @param[in] - reqTimeOut
     */
    uint32_t SetTimeOut(const int connectionTimeOut, const int reqTimeOut);

    /**
     * Method to Create Request Header
     * @param [in] - operation - Operation 1- POST, 2 for PUT, 3 for DELETE, 4- GET
     ** @param [in] -url - specifies the url
     **/

    uint32_t CreateRequestHeader(const std::string &url,
                                 const HttpMethod operation);

    /**
     ** Method to Request for Create Operation - id and value as input
     ** @param[in] - map idValuePair - contains the request body as id and value pair
     ** @param[in] - delimiter
     ** @param[out] - SUCCESS/ FAILURE
     **/
    uint32_t SetRequestBody(const char* str);

    /**
     * Method to Send Request and Get Response Code
     * param[out] - SUCCESS/ FAILURE
     */
    uint32_t SendRequestAndGetResponseCode();

  private:
    HttpClient *http_client_obj_;
    const std::string mipaddress_;
    const int mportnumber_;
};
}
#endif  // RESTJSON_REST_CLIENT_H_
