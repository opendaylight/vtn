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

#include <string.h>
#include <uncxx/restjson/http_client.hh>

namespace librestjson {

HttpClient::HttpClient()
: handle_(NULL),
  response_(NULL),
  slist_(NULL) {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
}

HttpClient::~HttpClient() {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
}

void HttpClient::Init() {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);

  handle_ = curl_easy_init();
  PFC_ASSERT(NULL != handle_);

  response_ = new HttpResponse_t;
  PFC_ASSERT(NULL != response_);

  response_->code = 0;
  response_->write_data = new HttpContent_t;

  PFC_ASSERT(NULL != response_->write_data);
  response_->write_data->memory = NULL;
  response_->write_data->size = 0;
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
}

void HttpClient::Fini() {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  if (NULL != handle_) {
    curl_easy_cleanup(handle_);
    handle_ = NULL;
  }
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
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
}

uint32_t HttpClient::SetUsernamePassword(const std::string &username,
                                         const std::string &password) {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  std::string username_password = "";
  username_password.append(username);
  username_password.append(":");
  username_password.append(password);
  pfc_log_debug("%d CURLOPT_USERPWD", CURLOPT_USERPWD);

  CURLcode curlRetCode = curl_easy_setopt(handle_, CURLOPT_USERPWD,
      username_password.c_str());
  if (CURLE_OK != curlRetCode) {
    return FAILURE;
  }
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return SUCCESS;
}

uint32_t HttpClient::SetConnectionTimeOut(const int connectionTimeOut) {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  pfc_log_debug("%d CURLOPT_CONNECTTIMEOUT", CURLOPT_CONNECTTIMEOUT);

  CURLcode curlRetCode = curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT,
      connectionTimeOut);
  if (CURLE_OK != curlRetCode) {
    return FAILURE;
  }
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return SUCCESS;
}

uint32_t HttpClient::SetRequestTimeOut(const int reqTimeOut) {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  CURLcode curlRetCode = curl_easy_setopt(handle_, CURLOPT_TIMEOUT,
      reqTimeOut);
  if (CURLE_OK != curlRetCode) {
    return FAILURE;
  }
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return SUCCESS;
}

uint32_t HttpClient::SetOperationType(const HttpMethod operation) {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  uint32_t ret_val = SUCCESS;
  CURLcode curlRetCode = CURLE_OK;
  pfc_log_debug("%d, CURLE_OK", CURLE_OK);
  pfc_log_debug("%d,HTTP_METHOD_POST ", CURLOPT_POST);

  switch (operation) {
    case HTTP_METHOD_POST: {
      pfc_log_info("HTTP_METHOD_POST");
      curlRetCode = curl_easy_setopt(handle_, CURLOPT_POST, 1);
      if (CURLE_OK != curlRetCode) {
        pfc_log_error(" SetRequest Body failed with curl error code %d",
                      curlRetCode);
        ret_val = FAILURE;
        break;
      }
    }
      break;

    case HTTP_METHOD_PUT: {
      curlRetCode = curl_easy_setopt(handle_, CURLOPT_CUSTOMREQUEST, "PUT");
      if (CURLE_OK != curlRetCode) {
        ret_val = FAILURE;
        break;
      }
    }
      break;
    case HTTP_METHOD_DELETE: {
      curlRetCode = curl_easy_setopt(handle_, CURLOPT_CUSTOMREQUEST, "DELETE");
      if (CURLE_OK != curlRetCode) {
        ret_val = FAILURE;
        break;
      }
    }
      break;
    case HTTP_METHOD_GET: {
      curlRetCode = curl_easy_setopt(handle_, CURLOPT_HTTPGET, 1);
      if (CURLE_OK != curlRetCode) {
        ret_val = FAILURE;
        break;
      }
    }
      break;

    default: {
      pfc_log_debug("Unsupported case type %d ", operation);
      ret_val = FAILURE;
      break;
    }
  }
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return ret_val;
}

uint32_t HttpClient::SetRequestBody(const char* requestbody) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);

  pfc_log_debug("requestbody : %s", requestbody);

  CURLcode curlRetCode = curl_easy_setopt(handle_, CURLOPT_POSTFIELDS,
      requestbody);

  if (CURLE_OK != curlRetCode) {
    pfc_log_error(" SetRequest Body failed with curl error code %d",
                  curlRetCode);
    return FAILURE;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);

  return SUCCESS;
}

uint32_t HttpClient::Execute() {
  pfc_log_info("%s Entering method ", PFC_FUNCNAME);

  uint32_t curl_ret_code = SetOptCommon();
  if (SUCCESS != curl_ret_code) {
    return FAILURE;
  }

  curl_ret_code = SetOptWriteData();
  if (SUCCESS != curl_ret_code) {
    return FAILURE;
  }

  curl_ret_code = PerformHttpReq();
  if (SUCCESS != curl_ret_code) {
    return FAILURE;
  }
  pfc_log_info("%s Exiting Execute method ", PFC_FUNCNAME);
  return SUCCESS;
}

uint32_t HttpClient::SetRequestHeader(const std::string& requestheader) {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  CURLcode curlRetCode = CURLE_OK;
  curlRetCode = curl_easy_setopt(handle_, CURLOPT_URL,
      (requestheader).c_str());

  if (CURLE_OK != curlRetCode) {
    return FAILURE;
  }

  slist_ = curl_slist_append(slist_, kContentTypeJson);
  slist_ = curl_slist_append(slist_, kAcceptTypeJson);

  if (NULL != slist_) {
    CURLcode curlRetCode = curl_easy_setopt(handle_,
                                            CURLOPT_HTTPHEADER, slist_);
    if (CURLE_OK != curlRetCode) {
      return FAILURE;
    }
  }

  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return SUCCESS;
}

uint32_t HttpClient::SetOptCommon() {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  CURLcode curlRetCode = CURLE_OK;

  curlRetCode = curl_easy_setopt(handle_, CURLOPT_NOPROGRESS, 1);
  if (CURLE_OK != curlRetCode) {
    return FAILURE;
  }
  curlRetCode = curl_easy_setopt(handle_, CURLOPT_NOSIGNAL, 1);
  if (CURLE_OK != curlRetCode) {
    return FAILURE;
  }
  curlRetCode = curl_easy_setopt(handle_, CURLOPT_VERBOSE, 1L);
  if (CURLE_OK != curlRetCode) {
    return FAILURE;
  }
  curlRetCode =  curl_easy_setopt(handle_, CURLOPT_FOLLOWLOCATION, 0);
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return SUCCESS;
}

uint32_t HttpClient::SetOptWriteData() {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  CURLcode curlRetCode = curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION,
      WriteCallback);
  if (CURLE_OK != curlRetCode) {
    return FAILURE;
  }
  curlRetCode = curl_easy_setopt(handle_, CURLOPT_WRITEDATA,
      reinterpret_cast<void *>
      (response_->write_data));

  if (CURLE_OK != curlRetCode) {
    return FAILURE;
  }
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return SUCCESS;
}

HttpResponse_t HttpClient::GetHttpRespCode() {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  HttpResponse_t response_to_return;
  response_to_return.code = 0;
  response_to_return.write_data = NULL;
  if (NULL != response_) {
    response_to_return.code = response_->code;
  }
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return response_to_return;
}

HttpContent_t* HttpClient::GetHttpRespBody() {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  if (NULL != response_) {
    if (NULL != response_->write_data) {
      pfc_log_info("Output from server %s ", response_->write_data->memory);
      return response_->write_data;
    }
  }
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return NULL;
}

uint32_t HttpClient::PerformHttpReq() {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);

  CURLcode curlRetCode = curl_easy_perform(handle_);

  if (CURLE_OK != curlRetCode) {
    pfc_log_info("%d Error", curlRetCode);
    return FAILURE;
  }

  curlRetCode = curl_easy_getinfo(handle_,
      CURLINFO_RESPONSE_CODE, &(response_->code));

  if (CURLE_OK != curlRetCode) {
    return FAILURE;
  }
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return SUCCESS;
}

size_t HttpClient::WriteCallback(void *ptr, size_t size, size_t nmemb,
                                 void *data) {
  pfc_log_debug("%s, Entering function", PFC_FUNCNAME);
  HttpContent_t *mem = reinterpret_cast<HttpContent_t*>(data);

  if (NULL == ptr) {
    return kSizeNull;
  }

  if (NULL == mem) {
    return kSizeNull;
  }

  size_t realsize = size * nmemb;

  if (mem->size) {
    mem->memory = reinterpret_cast<char*>(realloc(mem->memory,
                                                  mem->size + realsize));
  } else {
    mem->memory = reinterpret_cast<char*>(malloc(realsize + 1));
  }

  if (NULL == mem->memory) {
    return kSizeNull;
  }

  memset(&(mem->memory[mem->size]), '\0', realsize + 1);
  if (mem->memory) {
    memcpy(reinterpret_cast<void *>(&(mem->memory[mem->size])), ptr, realsize);
    mem->size += realsize;
  }
  pfc_log_debug("%s, Exiting function", PFC_FUNCNAME);
  return realsize;
}
}
