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
#include <uncxx/restjson/rest_common_defs.hh>
#include <uncxx/restjson/json_build_parse.hh>
#include <string>

namespace librestjson {

class HttpClient {
  public:
    /**
     ** @brief  Default Constructor.
     **/
    HttpClient();

    /**
     ** @brief  Default Constructor.
     **/
    ~HttpClient();

    /**
     ** Initialises curl handle
     ** @retval int - returns 0 - for success, 1- failure
     **/
    void Init();

    /**
     ** Fini Method to release handle
     **/
    void Fini();

    /**
     **  Method to Execute the Request
     ** @restClientStruct_ - RestClientHeader structue 
     ** @curlCmdStruct_ - CurlCommandHeader structure
     ** @reval CurlReturnCode - 0 for SUCCESS, 1 for FAILURE
     **/
    uint32_t Execute();

    /**
     ** Method to get the response code from server
     ** @ret_val HttpResponse_t - structure which contains the response code from server
     **/
    HttpResponse_t GetHttpRespCode();

    /**
     ** Method to get the respnse body from server
     ** @HttpContent_t - retval - Structure which contains response body from server
     **/
    HttpContent_t* GetHttpRespBody();

    /**
     * Method to set the request Header
     * @param[in] requestheader
     * @param[out] - SUCCESS/ FAILURE
     */
    uint32_t SetRequestHeader(const std::string &requestheader);
    /**
     * Method to set the user name and password
     * @param[in] username
     * @param[in] password
     * @param[out] - SUCCESS/ FAILURE
     */
    uint32_t SetUsernamePassword(const std::string &username,
                                 const std::string &password);
    /**
     * Method to set the connection TimeOut
     * @param[in] connectionTimeOut
     * @param[out] - SUCCESS/ FAILURE
     */
    uint32_t SetConnectionTimeOut(const int connectionTimeOut);
    /**
     * Method to set the request TimeOut
     * @param[in] request TimeOut
     * @param[out] - SUCCESS/ FAILURE
     */
    uint32_t SetRequestTimeOut(const int reqTimeOut);
    /**
     * Method to set the type of operation
     * @param[in] operation
     * @param[out] - SUCCESS/ FAILURE
     */
    uint32_t SetOperationType(const HttpMethod operation);
    /**
     * Method to set the request body
     * @param[in] requestbody
     * @param[out] - SUCCESS/ FAILURE
     */
    uint32_t SetRequestBody(const char* req);

  private:
    /**
     ** Method to set Common parameters like Time out
     ** @restClientStruct_ - RestClientHeader structue
     ** @curlCmdStruct_ - CurlCommandHeader structure
     ** @retval CurlReturnCode - 0 for SUCCESS, 1 for FAILURE
     **/
    uint32_t SetOptCommon();

    /**
     ** Method to send Request
     ** @retval CurlReturnCode - 0 for SUCCESS, 1 for FAILURE
     **/
    uint32_t PerformHttpReq();

    /**
     ** Method to Write the Response from Server
     ** @retval CurlReturnCode - 0 for SUCCESS, 1 for FAILURE
     **/
    uint32_t SetOptWriteData();

    /**
     ** Callback Method  used to write response data
     **/
    static size_t WriteCallback(void* ptr, size_t size, size_t nmemb,
                                void* data);

  private:
    CURL* handle_;
    HttpResponse_t *response_;
    struct curl_slist *slist_;
};
}
#endif  // RESTJSON_HTTP_CLIENT_H_
