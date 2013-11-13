/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <http_client.hh>
#include <gtest/gtest.h>
#include <string>
TEST(set_username_password_val, Test1) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_username_password("user", "pass");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetUsernamePassword, Test2) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_username_password("user", "i#$%^");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetUsernamePassword_failure_case1, Test3) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val =
      obj.set_username_password("idhglnqdioghegholqe;hq;er/goihqehbq/';IOGHEK",
                                "QELKJHGQEO;GI;/ihggp");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetUsernamePassword_success, Test4) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_username_password("user", "example");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetConnectionTimeOut, Test5) {
  unc::restjson::HttpClient obj;

  obj.init();
  uint32_t val = obj.set_connection_timeout(10);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetConnectionTimeOut_success, Test6) {
  unc::restjson::HttpClient obj;

  obj.init();
  uint32_t val = obj.set_connection_timeout(0);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetConnectionTimeOut_failure, Test8) {
  unc::restjson::HttpClient obj;

  obj.init();
  uint32_t val = obj.set_connection_timeout(-1);
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
}


TEST(SetRequestTimeOut, Test9) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_request_timeout(10);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetRequestTimeOut_success, Test10) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_request_timeout(0);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetRequestTimeOut_failure, Test11) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_request_timeout(-1);
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
}

TEST(SetOperationType, Test12) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_POST);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetOperationType_failure_HTTP_METHOD_PUT, Test13) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_PUT);
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
}

TEST(SetOperationType_failure_HTTP_METHOD_POST, Test14) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_POST);
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
}

TEST(SetOperationType_failure_HTTP_METHOD_DELETE, Test15) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_DELETE);
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
}

TEST(SetOperationType_failure_HTTP_METHOD_GET, Test16) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_GET);
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
}

TEST(SetOperationType_PUT, Test17) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_PUT);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetOperationType_GET, Test18) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_GET);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(SetOperationType_delete, Test19) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_operation_type(unc::restjson::HTTP_METHOD_DELETE);
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(set_request_header, Test20) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_url("example");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(set_request_header_failure, Test21) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_url("");
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
}

TEST(set_request_header_failure1, Test22) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_url("example");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}


TEST(set_request_body, Test23) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_request_body("example");
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(set_request_body_null, Test24) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_request_body("");
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
}

TEST(set_request_body_null1, Test25) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_request_body(NULL);
  EXPECT_EQ(val, unc::restjson::REST_OP_FAILURE);
  obj.fini();
}

TEST(execute, Test26) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.send_request();
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}

TEST(set_opt_common, Test27) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_opt_common();
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}


TEST(set_opt_write_data, Test28) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.set_opt_write_data();
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}


TEST(perform_http_req, Test30) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.perform_http_req();
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}


TEST(fini, Test31) {
  unc::restjson::HttpClient obj;
  obj.init();
  obj.fini();
}

TEST(SetOperationNoPrgress_delete, Test32) {
  unc::restjson::HttpClient obj;
  obj.init();
  uint32_t val = obj.send_request();
  obj.fini();
  EXPECT_EQ(val, unc::restjson::REST_OP_SUCCESS);
  obj.fini();
}


TEST(get_http_resp_body, Test33) {
  unc::restjson::HttpClient obj;
  obj.init();
  unc::restjson::HttpResponse_t * response;
  response = obj.get_http_response();
  EXPECT_EQ(0, response->code);
  obj.fini();
}

TEST(get_http_resp_body_failure_case, Test37) {
  unc::restjson::HttpClient obj;
  obj.init();
  unc::restjson::HttpResponse_t * response;
  response = obj.get_http_response();
  EXPECT_EQ(0, response->code);
  obj.fini();
}

TEST(get_http_resp_body_success, Test34) {
  unc::restjson::HttpClient obj;
  obj.init();
  unc::restjson::HttpResponse_t * response;
  response = obj.get_http_response();
  EXPECT_EQ(0, response->code);
  obj.fini();
}

TEST(get_http_resp_body_success, Test_write_call_back) {
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
  obj.fini();
}

TEST(get_http_resp_body_success, Test_write_call_back_src_ptr_NULL) {
  unc::restjson::HttpClient obj;
  obj.init();
  char *src;
  src = NULL;
  size_t size = 25;
  size_t nmemb = 16;
  unc::restjson::HttpContent_t * dest = new unc::restjson::HttpContent_t;
  dest->size = 0;
  size_t retval = obj.write_call_back(src, size, nmemb, dest);
  EXPECT_EQ(retval, 0);
  delete dest;
  obj.fini();
}

TEST(get_http_resp_body_success, Test_write_call_back_src_dest_NULL) {
  unc::restjson::HttpClient obj;
  obj.init();
  char *src;
  src = NULL;
  size_t size = 25;
  size_t nmemb = 16;
  unc::restjson::HttpContent_t * dest = NULL;
  size_t retval = obj.write_call_back(src, size, nmemb, dest);
  EXPECT_EQ(retval, 0);
  obj.fini();
  obj.clear_http_response();
}

TEST(get_http_resp_body_success, Test_write_call_back_realloc) {
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
  EXPECT_EQ(ret, size * nmemb);
  char *result = reinterpret_cast < char *>(dest->memory);
  EXPECT_STREQ(result,
               const_cast <
               char
               *>
               ("1{\"vbridge\": [ { \"name\": \"vbridge_1\",\"description\": \"1\" }, {\"name\": \"vbridge_2\" } ] }"));
  free(dest->memory);
  delete dest;
  dest = NULL;
  obj.fini();
}
