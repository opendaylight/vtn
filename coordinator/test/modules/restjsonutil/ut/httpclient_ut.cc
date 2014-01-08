/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef HTTP_CLIENT_UT
#define HTTP_CLIENT_UT

#include <http_client.hh>
#include <gtest/gtest.h>
#include <string>

int test_flag = FAIL_TEST;

TEST(HttpClient , set_username_password_val) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_username_password("user", "pass");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetUsernamePassword) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_username_password("user", "i#$%^");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetUsernamePassword_failure_case1) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val =
      obj.set_username_password("idhglnqdioghegholqe;hq;er/goihqehbq/';IOGHEK",
                                "QELKJHGQEO;GI;/ihggp");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetUsernamePassword_null) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_username_password("", "");
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
  obj.clear_http_response();
}


TEST(HttpClient , SetUsernamePassword_valid) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_username_password("user", "imashndf");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}


TEST(HttpClient , SetUsernamePassword_success) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_username_password("user", "example");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetConnectionTimeOut) {
  unc::restjson::HttpClient obj;

  obj.init();
  test_flag = FAIL_TEST;
  uint32_t val = obj.set_connection_timeout(10);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetConnectionTimeOut_success) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = FAIL_TEST;
  uint32_t val = obj.set_connection_timeout(0);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetConnectionTimeOut_failure) {
  unc::restjson::HttpClient obj;

  obj.init();
  test_flag = CURLOPT_CONNECTIONTIMEOUT_FAILURE;
  uint32_t val = obj.set_connection_timeout(5);
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}


TEST(HttpClient , SetRequestTimeOut) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = FAIL_TEST;
  uint32_t val = obj.set_request_timeout(10);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetRequestTimeOut_success) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = FAIL_TEST;
  uint32_t val = obj.set_request_timeout(0);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetRequestTimeOut_failure) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_TIMEOUT_FAILURE;
  uint32_t val = obj.set_request_timeout(5);
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}


TEST(HttpClient , SetOperationType_success_HTTP_METHOD_PUT) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_PUT);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetOperationType_success_HTTP_METHOD_DELETE) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_DELETE);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetOperationType_PUT) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_PUT);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}


TEST(HttpClient , SetOperationType_delete) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_DELETE);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , set_request_header) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_url("example");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , set_request_header_failure) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_url("");
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
  obj.clear_http_response();
}
TEST(HttpClient , set_opt_common) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_opt_common();
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , set_request_header_failure_case) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_HTTPHEADER_FAILURE;
  uint32_t val = obj.set_url("example");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , set_request_body) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_request_body("example");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , set_request_body_null) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_request_body("");
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , set_request_body_null1) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_request_body(NULL);
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , set_opt_common_CURLOPT_FOLLOWLOCATION) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_FOLLOWLOCATION_FAILURE;
  uint32_t val = obj.set_opt_common();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}
TEST(HttpClient , set_opt_common_NOPROGRESS) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_NOPROGRESS_FAILURE;
  uint32_t val = obj.set_opt_common();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , set_opt_common_NOSIGNAL) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_NOSIGNAL_FAILURE;
  uint32_t val = obj.set_opt_common();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}


TEST(HttpClient , set_opt_common_VERBOSE) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_VERBOSE_FAILURE;
  uint32_t val = obj.set_opt_common();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , set_opt_write_data_WRITEFUNCTION) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_WRITEDATA_FAILURE;
  uint32_t val = obj.set_opt_write_data();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , set_opt_write_data_WRITEDATA) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_WRITEFUNCTION_FAILURE;
  uint32_t val = obj.set_opt_write_data();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}
TEST(HttpClient , perform_http_req) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.perform_http_req();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
  obj.clear_http_response();
}
TEST(HttpClient , perform_http_req_failure_RESPONSECODE_NULL) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_GETINFO_FAILURE;
  uint32_t val = obj.perform_http_req();
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , perform_http_req_failure) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.perform_http_req();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
  obj.clear_http_response();
}


TEST(HttpClient , fini) {
  unc::restjson::HttpClient obj;
  obj.init();
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , SetOperationNoPrgress_delete) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.send_request();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
  obj.clear_http_response();
}
TEST(HttpClient , send_request_set_opt_common_failure) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_VERBOSE_FAILURE;
  uint32_t val = obj.send_request();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}
TEST(HttpClient , send_request_set_opt_write_data) {
  unc::restjson::HttpClient obj;
  obj.init();
  test_flag = CURLOPT_WRITEFUNCTION_FAILURE;
  uint32_t val = obj.send_request();
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  test_flag = UNSET;
  obj.fini();
  obj.clear_http_response();
}


TEST(HttpClient , get_http_resp_body) {
  unc::restjson::HttpClient obj;
  obj.init();
  unc::restjson::HttpResponse_t * response;
  response = obj.get_http_response();
  EXPECT_EQ(0, response->code);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , get_http_resp_body_failure_case) {
  unc::restjson::HttpClient obj;
  obj.init();
  unc::restjson::HttpResponse_t * response = NULL;
  response = obj.get_http_response();
  EXPECT_EQ(0, response->code);
  obj.fini();
  obj.clear_http_response();
}
TEST(HttpClient , get_http_resp_body_success_result) {
  unc::restjson::HttpClient obj;
  obj.init();
  char *src = NULL;
  src =
      const_cast <
      char
      *>
      ("{\"vbridge\": [ { \"name\": \"vbridge_1\",\"description\": \"1\" }, {\"name\": \"vbridge_2\" } ] }");
  size_t size = 25;
  size_t nmemb = 16;
  unc::restjson::HttpContent_t * dest = new unc::restjson::HttpContent_t;
  dest->size = 0;
  obj.write_call_back(src, size, nmemb, dest);
  char *result = reinterpret_cast < char *>(dest->memory);
  EXPECT_STREQ(result,
               "{\"vbridge\": [ { \"name\": \"vbridge_1\",\"description\": \"1\" }, {\"name\": \"vbridge_2\" } ] }");
  free(dest->memory);
  delete dest;
  dest = NULL;
  result = NULL;
  obj.clear_http_response();
  obj.fini();
}

TEST(HttpClient , write_call_back_src_ptr_NULL) {
  unc::restjson::HttpClient obj;
  obj.init();
  char *src;
  src = NULL;
  size_t size = 25;
  size_t nmemb = 16;
  unc::restjson::HttpContent_t * dest = new unc::restjson::HttpContent_t;
  dest->size = 0;
  size_t retval = obj.write_call_back(src, size, nmemb, dest);
  EXPECT_EQ(0U, retval);
  delete dest;
  obj.clear_http_response();
  obj.fini();
}

TEST(HttpClient , write_call_back_mem_NULL) {
  unc::restjson::HttpClient obj;
  obj.init();
  char *src;
  src = NULL;
  size_t size = 0;
  size_t nmemb = 0;
  unc::restjson::HttpContent_t * dest = new unc::restjson::HttpContent_t;
  dest->size = 0;
  size_t retval = obj.write_call_back(src, size, nmemb, dest);
  EXPECT_EQ(0U, retval);
  delete dest;
  obj.clear_http_response();
  obj.fini();
}


TEST(HttpClient , write_call_back_src_dest_NULL) {
  unc::restjson::HttpClient obj;
  obj.init();
  char *src;
  src = NULL;
  size_t size = 25;
  size_t nmemb = 16;
  unc::restjson::HttpContent_t * dest = NULL;
  size_t retval = obj.write_call_back(src, size, nmemb, dest);
  EXPECT_EQ(0U, retval);
  obj.fini();
  obj.clear_http_response();
}

TEST(HttpClient , write_call_back_realloc) {
  unc::restjson::HttpClient obj;
  obj.init();
  char *src;
  src =
      const_cast <
      char
      *>
      ("{\"vbridge\": [ { \"name\": \"vbridge_1\",\"description\": \"1\" }, {\"name\": \"vbridge_2\" } ] }");
  size_t size = 25;
  size_t nmemb = 20;
  unc::restjson::HttpContent_t * dest = new unc::restjson::HttpContent_t;
  dest->size = 1;
  memset(&dest->memory, '\0', sizeof(dest->memory));
  dest->memory = reinterpret_cast < char *>
      (malloc((dest->size) * sizeof(dest->size)));
  memset(&(dest->memory[0]), '1', 1);
  size_t ret = obj.write_call_back(src, size, nmemb, dest);
  EXPECT_EQ(size * nmemb, ret);
  char *result = reinterpret_cast < char *>(dest->memory);
  EXPECT_STREQ(result,
               const_cast <
               char
               *>
               ("1{\"vbridge\": [ { \"name\": \"vbridge_1\",\"description\": \"1\" }, {\"name\": \"vbridge_2\" } ] }"));
  obj.clear_http_response();
  free(dest->memory);
  delete dest;
  dest = NULL;
  obj.fini();
}


TEST(HttpClient , get_http_response) {
  unc::restjson::HttpClient obj;
  obj.init();
  unc::restjson::HttpResponse_t * response;
  response = obj.get_http_response();
  EXPECT_EQ(response->code , 0);
  obj.clear_http_response();
  obj.fini();
}
#endif
