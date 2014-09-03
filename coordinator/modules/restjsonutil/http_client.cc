/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <http_client.hh>

namespace unc {
namespace restjson {

// Constructor
HttpClient::HttpClient()
: handle_(NULL),
  response_(NULL),
  slist_(NULL) {
  ODC_FUNC_TRACE;
}

// Destructor
HttpClient::~HttpClient() {
}

// Init Method to allocate memory to the Curl handle, structures used
void HttpClient::init() {
  ODC_FUNC_TRACE;

  handle_ = curl_easy_init();
  PFC_ASSERT(NULL != handle_);

  response_ = new HttpResponse_t;
  PFC_ASSERT(NULL != response_);

  response_->code = 0;
  response_->write_data = new HttpContent_t;

  PFC_ASSERT(NULL != response_->write_data);
  response_->write_data->memory = NULL;
  response_->write_data->size = 0;
}

// Fini method to release the memory allocate by Init method
void HttpClient::fini() {
  ODC_FUNC_TRACE;
  if (NULL != handle_) {
    curl_easy_cleanup(handle_);
    handle_ = NULL;
  }
  if (NULL != slist_) {
    curl_slist_free_all(slist_);
    slist_ = NULL;
  }
}

// Sets the Username and password to the CURL handle
rest_resp_code_t HttpClient::set_username_password(const std::string &username,
                                                   const std::string
                                                   &password) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(handle_ != NULL);

  std::string username_password = "";
  username_password.append(username);
  username_password.append(COLON);
  username_password.append(password);

  CURLcode curl_ret_code = curl_easy_setopt(handle_, CURLOPT_USERPWD,
                                          username_password.c_str());
  if (CURLE_OK != curl_ret_code) {
    pfc_log_error("Failed in setusername password : %d" , curl_ret_code);
    return REST_OP_FAILURE;
  }
  return REST_OP_SUCCESS;
}

// Sets the Connection TimeOut to the CURL handle
rest_resp_code_t HttpClient::set_connection_timeout(
    const uint32_t connection_time_out) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(handle_ != NULL);

  CURLcode curl_ret_code = curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT,
                                          connection_time_out);
  if (CURLE_OK != curl_ret_code) {
    pfc_log_error("Failed in set connection timeout : %d" , curl_ret_code);
    return REST_OP_FAILURE;
  }
  return REST_OP_SUCCESS;
}

// Sets the request time out to the CURL handle
rest_resp_code_t
HttpClient::set_request_timeout(const uint32_t request_time_out) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(handle_ != NULL);
  CURLcode curl_ret_code = curl_easy_setopt(handle_, CURLOPT_TIMEOUT,
                                          request_time_out);
  if (CURLE_OK != curl_ret_code) {
    pfc_log_error("Failed in set curl timeout : %d" , curl_ret_code);
    return REST_OP_FAILURE;
  }
  return REST_OP_SUCCESS;
}

// Sets the Operation Type to the CURL handle
rest_resp_code_t HttpClient::set_operation_type(const HttpMethod operation) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(handle_ != NULL);

  rest_resp_code_t ret_val = REST_OP_SUCCESS;
  CURLcode curl_ret_code = CURLE_OK;
  switch (operation) {
    case HTTP_METHOD_POST: {
      pfc_log_debug("HTTP_METHOD_POST");
      curl_ret_code = curl_easy_setopt(handle_, CURLOPT_POST, CURLOPT_ENABLE);
      if (CURLE_OK != curl_ret_code) {
        pfc_log_error(" Set operation failed with curl error code %d",
                      curl_ret_code);
        ret_val = REST_OP_FAILURE;
        break;
      }
    }
    break;

    case HTTP_METHOD_PUT: {
      curl_ret_code = curl_easy_setopt(handle_, CURLOPT_CUSTOMREQUEST, "PUT");
      if (CURLE_OK != curl_ret_code) {
        pfc_log_error(" Setoperation failed with curl error code %d",
                      curl_ret_code);
        ret_val = REST_OP_FAILURE;
        break;
      }
    }
    break;
    case HTTP_METHOD_DELETE: {
      curl_ret_code = curl_easy_setopt(handle_,
                                       CURLOPT_CUSTOMREQUEST,
                                       "DELETE");
      if (CURLE_OK != curl_ret_code) {
        pfc_log_error(" Setoperation failed with curl error code %d",
                      curl_ret_code);
        ret_val = REST_OP_FAILURE;
        break;
      }
    }
    break;
    case HTTP_METHOD_GET: {
      curl_ret_code = curl_easy_setopt(handle_, CURLOPT_HTTPGET,
                                       CURLOPT_ENABLE);
      if (CURLE_OK != curl_ret_code) {
        pfc_log_error(" Setoperation failed with curl error code %d",
                      curl_ret_code);
        ret_val = REST_OP_FAILURE;
        break;
      }
    }
    break;

    default: {
      pfc_log_error("Unsupported case type %d ", operation);
      ret_val = REST_OP_FAILURE;
      break;
    }
  }
  return ret_val;
}

// Sets the Request Body to the CURL handle
rest_resp_code_t HttpClient::set_request_body(const char* request_body) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(handle_ != NULL);
  pfc_log_info("requestbody : %s", request_body);


  CURLcode curl_ret_code = curl_easy_setopt(handle_, CURLOPT_POSTFIELDS,
                                          request_body);

  if (CURLE_OK != curl_ret_code) {
    pfc_log_error(" SetRequest Body failed with curl error code %d",
                  curl_ret_code);
    return REST_OP_FAILURE;
  }
  return REST_OP_SUCCESS;
}

// Sets Common Values to the CURL hanlde, Post the handle and writes the reponse
rest_resp_code_t HttpClient::send_request() {
  ODC_FUNC_TRACE;
  PFC_ASSERT(handle_ != NULL);

  rest_resp_code_t ret_val = set_opt_common();
  if (REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Failed in setoptcommon : %d" , ret_val);
    return REST_OP_FAILURE;
  }

  ret_val = set_opt_write_data();
  if (REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Failed in setopt write data : %d" , ret_val);
    return REST_OP_FAILURE;
  }

  ret_val = perform_http_req();
  if (REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Failed in perform http request : %d" , ret_val);
    return REST_OP_FAILURE;
  }
  return REST_OP_SUCCESS;
}
// Gets the Response Code from Controller
HttpResponse_t* HttpClient::get_http_response() {
  ODC_FUNC_TRACE;
  if (NULL != response_) {
    return response_;
  }
  return NULL;
}

// Sets the Request Header to the CURL handle
rest_resp_code_t HttpClient::set_url(const std::string& url) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(handle_ != NULL);

  CURLcode curl_ret_code = CURLE_OK;
  curl_ret_code = curl_easy_setopt(handle_, CURLOPT_URL,
                                   (url).c_str());

  if (CURLE_OK != curl_ret_code) {
    pfc_log_error(" SetUrl failed with curl error code %d",
                  curl_ret_code);
    return REST_OP_FAILURE;
  }
  if (NULL != slist_) {
    curl_slist_free_all(slist_);
    slist_ = NULL;
  }

  slist_ = curl_slist_append(slist_, CONTENT_TYPE);
  slist_ = curl_slist_append(slist_, ACCEPT_TYPE);

  if (NULL != slist_) {
    CURLcode curl_ret_code = curl_easy_setopt(handle_,
                                              CURLOPT_HTTPHEADER, slist_);
    if (CURLE_OK != curl_ret_code) {
      pfc_log_error(" set http header failed with curl error code %d",
                    curl_ret_code);
      return REST_OP_FAILURE;
    }
  }
  return REST_OP_SUCCESS;
}

// Sets the Common Values to the CURL handle
rest_resp_code_t HttpClient::set_opt_common() {
  ODC_FUNC_TRACE;
  PFC_ASSERT(handle_ != NULL);

  CURLcode curl_ret_code = CURLE_OK;

  curl_ret_code = curl_easy_setopt(handle_, CURLOPT_NOPROGRESS, CURLOPT_ENABLE);
  if (CURLE_OK != curl_ret_code) {
    pfc_log_error("Set no progress failed with curl error code %d",
                  curl_ret_code);
    return REST_OP_FAILURE;
  }
  curl_ret_code = curl_easy_setopt(handle_, CURLOPT_NOSIGNAL, CURLOPT_ENABLE);
  if (CURLE_OK != curl_ret_code) {
    pfc_log_error("set no signal failed with curl error code %d",
                  curl_ret_code);
    return REST_OP_FAILURE;
  }
  curl_ret_code = curl_easy_setopt(handle_, CURLOPT_VERBOSE, CURLOPT_ENABLE);
  if (CURLE_OK != curl_ret_code) {
    pfc_log_error(" Set verborse failed with curl error code %d",
                  curl_ret_code);
    return REST_OP_FAILURE;
  }
  curl_ret_code =  curl_easy_setopt(handle_,
                                    CURLOPT_FOLLOWLOCATION,
                                    CURLOPT_DISABLE);
  if (CURLE_OK != curl_ret_code) {
    pfc_log_error(" Set no follow locatione failed with curl error code %d",
                  curl_ret_code);
    return REST_OP_FAILURE;
  }
  return REST_OP_SUCCESS;
}

// Write the response data to the response structure
rest_resp_code_t HttpClient::set_opt_write_data() {
  ODC_FUNC_TRACE;
  PFC_ASSERT(handle_ != NULL);

  CURLcode curl_ret_code = curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION,
                                            write_call_back);
  if (CURLE_OK != curl_ret_code) {
    pfc_log_error(" Set write function failed with curl error code %d",
                  curl_ret_code);
    return REST_OP_FAILURE;
  }
  if (NULL != response_) {
    curl_ret_code = curl_easy_setopt(handle_, CURLOPT_WRITEDATA,
                                     reinterpret_cast<void *>
                                     (response_->write_data));
  }
  if (CURLE_OK != curl_ret_code) {
    pfc_log_error(" Set write data failed with curl error code %d",
                  curl_ret_code);
    return REST_OP_FAILURE;
  }
  return REST_OP_SUCCESS;
}

// Post the CURL handle
rest_resp_code_t HttpClient::perform_http_req() {
  ODC_FUNC_TRACE;
  PFC_ASSERT(handle_ != NULL);
  CURLcode curl_ret_code = curl_easy_perform(handle_);

  if (CURLE_OK != curl_ret_code) {
    pfc_log_error("%d Perform failed with error code", curl_ret_code);
    return REST_OP_FAILURE;
  }

  curl_ret_code = curl_easy_getinfo(handle_,
                                  CURLINFO_RESPONSE_CODE, &(response_->code));

  if (CURLE_OK != curl_ret_code) {
    pfc_log_error(" get response code failed with curl error code %d",
                      curl_ret_code);
    return REST_OP_FAILURE;
  }
  return REST_OP_SUCCESS;
}

// Clears the response memory allocated
void HttpClient::clear_http_response() {
  ODC_FUNC_TRACE;
  if (NULL != response_) {
    if (NULL != response_->write_data) {
      if (NULL != response_->write_data->memory) {
        free(response_->write_data->memory);
        response_->write_data->memory = NULL;
      }
      delete response_->write_data;
      response_->write_data = NULL;
    }
    delete response_;
    response_ = NULL;
  }
  // Free the slist memory
  if (NULL != slist_) {
    curl_slist_free_all(slist_);
    slist_ = NULL;
  }
}
// Call back to get the response from the CURL and
// write it to the resp structure
size_t HttpClient::write_call_back(void *ptr, size_t size, size_t nmemb,
                                   void *data) {
  ODC_FUNC_TRACE;
  HttpContent_t *mem = reinterpret_cast<HttpContent_t*>(data);

  if (NULL == ptr) {
    pfc_log_error("pointer points to response data is NULL");
    return SIZE_NULL;
  }

  if (NULL == mem) {
    pfc_log_error("HttpContent memory is NULL");
    return SIZE_NULL;
  }

  size_t realsize = size * nmemb;

  if (mem->size) {
    pfc_log_debug("Reallocate Memory............... ");
    mem->memory = reinterpret_cast<char*>(realloc(mem->memory,
                                                  mem->size + realsize + 1));
  } else {
    pfc_log_debug("Allocates Memory ............");
    mem->memory = reinterpret_cast<char*>(malloc(realsize + 1));
  }

  if (NULL == mem->memory) {
    pfc_log_error("memory is NULL ");
    return SIZE_NULL;
  }

  memset(&(mem->memory[mem->size]), '\0', realsize + 1);
  memcpy(reinterpret_cast<void *>(&(mem->memory[mem->size])), ptr, realsize);
  mem->size += realsize;
  return realsize;
}
}  // namespace restjson
}  // namespace unc
