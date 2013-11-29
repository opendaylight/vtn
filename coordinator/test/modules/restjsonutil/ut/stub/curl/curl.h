/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef VTN_COORDINATOR_TEST_MODULES_RESTJSONUTIL_UT_STUB_CURL_CURL_H_
#define VTN_COORDINATOR_TEST_MODULES_RESTJSONUTIL_UT_STUB_CURL_CURL_H_

#include <pfc/debug.h>
#include <string>
#include <cstring>

#define CURLOPT_NOPROGRESS_FAILURE 55
#define CURLOPT_NOSIGNAL_FAILURE 66
#define CURLOPT_VERBOSE_FAILURE 77
#define CURLOPT_FOLLOWLOCATION_FAILURE 88
#define CURLOPT_WRITEFUNCTION_FAILURE 11
#define CURLOPT_WRITEDATA_FAILURE 22
#define CURLOPT_CONNECTIONTIMEOUT_FAILURE 44
#define CURLOPT_TIMEOUT_FAILURE 444
#define CURLOPT_HTTPHEADER_FAILURE 333
#define CURLOPT_GETINFO_FAILURE 33
#define FAIL_TEST 99
#define UNSET 0

extern int test_flag;

class CURL {
 public:
  CURL() {
  }
  int value;
  int get_val() {
    return value;
  }
  void set_val(int val) {
    value = val;
  }
};

typedef enum {
  CURLE_OK = 0,
  CURLE_FAILURE = 1,
  CURLOPT_CONNECTTIMEOUT = 55,
  CURLOPT_TIMEOUT = 45,
  CURLOPT_POST = 47,
  CURLOPT_USERPWD = 10005,
  CURLOPT_CUSTOMREQUEST =88,
  CURLOPT_HTTPGET = 34,
  CURLOPT_POSTFIELDS =24,
  CURLOPT_URL = 56,
  CURLOPT_HTTPHEADER =67,
  CURLOPT_NOPROGRESS = 38,
  CURLOPT_NOSIGNAL = 46,
  CURLOPT_VERBOSE = 456,
  CURLOPT_FOLLOWLOCATION = 657,
  CURLOPT_WRITEFUNCTION = 56,
  CURLOPT_WRITEDATA = 35,
  CURLINFO_RESPONSE_CODE = 95,
} CURLcode;

typedef enum {
  SUCCESS1 = 0,
  FAILURE1
} CURLoption;


struct curl_slist;

typedef enum {
  val = 1
} CURLINFO;

static CURL *handle_ = NULL;

inline CURL *
curl_easy_init() {
  handle_ = new CURL();
  return handle_;
}


inline CURLcode
curl_easy_perform(CURL * handle) {
  if (handle_ == NULL) {
    return CURLE_FAILURE;
  }
  return CURLE_OK;
}

inline void
curl_easy_cleanup(CURL * handle) {
  if (handle_ != NULL) {
    delete handle_;
    handle_ = NULL;
  }
}

template < typename T > T * curl_slist_append(T * list, const char *string1) {
return list;
}

inline void curl_slist_free_all(struct curl_slist * list) {
}

inline CURLcode
curl_easy_getinfo(CURL * curl, CURLcode info, ...) {
  if (test_flag == CURLOPT_GETINFO_FAILURE) {
    return CURLE_OK;
  }
  if (curl != NULL) {
    return CURLE_FAILURE;
  }
  return CURLE_OK;
}

template < typename T >
inline CURLcode curl_easy_setopt(CURL * &handle_, CURLcode option, T val) {
  if ((CURLOPT_NOPROGRESS == option) &&
      (test_flag == CURLOPT_NOPROGRESS_FAILURE))  {
    return CURLE_FAILURE;
  }
  if ((CURLOPT_NOSIGNAL == option)  &&
      (test_flag == CURLOPT_NOSIGNAL_FAILURE)) {
    return CURLE_FAILURE;
  }
  if ((CURLOPT_VERBOSE == option) &&
      (test_flag == CURLOPT_VERBOSE_FAILURE))   {
    return CURLE_FAILURE;
  }
  if ((CURLOPT_FOLLOWLOCATION == option) &&
      (test_flag == CURLOPT_FOLLOWLOCATION_FAILURE))  {
    return CURLE_FAILURE;
  }
  if ((CURLOPT_WRITEFUNCTION == option) &&
      (test_flag == CURLOPT_WRITEFUNCTION_FAILURE)) {
    return CURLE_FAILURE;
  }
  if ((CURLOPT_WRITEDATA == option) &&
      (test_flag == CURLOPT_WRITEDATA_FAILURE)) {
    return CURLE_FAILURE;
  }
  if ((CURLOPT_CONNECTTIMEOUT == option) &&
      (test_flag == CURLOPT_CONNECTIONTIMEOUT_FAILURE)) {
    return CURLE_FAILURE;
  }
  if ((CURLOPT_TIMEOUT ==option) &&
      (test_flag == CURLOPT_TIMEOUT_FAILURE)) {
    return CURLE_FAILURE;
  }
  if (test_flag == FAIL_TEST) {
    return CURLE_OK;
  }
    if ((CURLOPT_HTTPHEADER == option) &&
        (test_flag == CURLOPT_HTTPHEADER_FAILURE)) {
    return CURLE_FAILURE;
  }
  return CURLE_OK;
}

inline CURLcode
curl_easy_setopt(CURL * &handle_, CURLcode option, const char *val) {
  if (0 == val) {
    return CURLE_FAILURE;
  }
  if (CURLOPT_VERBOSE == option) {
    return CURLE_FAILURE;
  }

  if ((CURLOPT_USERPWD == option) && (strcmp(":",  val)) == 0) {
    return CURLE_FAILURE;
  }
  if ((CURLOPT_WRITEDATA == option) && ('\0' == val)) {
    return CURLE_OK;
  }
  if (strcmp("http://11:12IN", val) == 0) {
    return CURLE_FAILURE;
  }
  if ('\0' == val) {
    return CURLE_FAILURE;
  }
  if (NULL == val) {
    return CURLE_FAILURE;
  }
  if (strcmp("", val) == 0) {
    return CURLE_FAILURE;
  }
  if (0 == strlen(val)) {
    return CURLE_FAILURE;
  }
  if (handle_ == NULL) {
    return CURLE_FAILURE;
  }
  if (strcmp("-1" , val) == 0) {
    return CURLE_FAILURE;
  }
  return CURLE_OK;
}

#endif  // VTN_COORDINATOR_TEST_MODULES_RESTJSONUTIL_UT_STUB_CURL_CURL_H_
