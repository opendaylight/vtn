/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <gtest/gtest.h>
#include <rest_client.hh>
#include <string>

TEST(RestClient, test_create_request_header_Emptyurl) {
  unc::restjson::RestClient rest_obj("1111", "", 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  std::string url = "";
  uint32_t retval =  rest_obj.create_request_header();
  EXPECT_EQ(retval, uint(1));
  rest_obj.clear_http_response();
}

TEST(RestClient, test_create_request_header_InvalidIpaddress) {
  unc::restjson::RestClient rest_obj("", "111", 12,
                                     unc::restjson::HTTP_METHOD_POST);
  std::string url = "sdd";
  uint32_t retval =  rest_obj.create_request_header();
  EXPECT_EQ(retval, uint(1));
  rest_obj.clear_http_response();
}

TEST(RestClient, test_create_request_header_InvalidPortNumber) {
  unc::restjson::RestClient rest_obj("11", "" , 0 ,
                                     unc::restjson::HTTP_METHOD_POST);
  std::string url = "sdfsd";
  uint32_t retval =  rest_obj.create_request_header();
  EXPECT_EQ(retval, uint(1));
  rest_obj.clear_http_response();
}


TEST(RestClient, test_create_request_header) {
  unc::restjson::RestClient rest_obj("11", "11" , 12,
                                     unc::restjson::HTTP_METHOD_POST);
  std::string url = "sdfsd";
  uint32_t retval =  rest_obj.create_request_header();
  EXPECT_EQ(retval, uint(0));
  rest_obj.clear_http_response();
}

TEST(RestClient, test_create_request_header_PUT) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  std::string url = "sdfsd";
  uint32_t retval =  rest_obj.create_request_header();
  EXPECT_EQ(retval, uint(0));
  rest_obj.clear_http_response();
}


TEST(RestClient, test_create_request_header_DELETE) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  std::string url = "sdfsd";
  uint32_t retval =  rest_obj.create_request_header();
  EXPECT_EQ(retval, uint(0));
  rest_obj.clear_http_response();
}

TEST(RestClient, test_SetInvalidUsername) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);

  std::string password = "sdfsd";
  uint32_t retval =  rest_obj.set_username_password("", password);
  EXPECT_EQ(retval, uint(1));
  rest_obj.clear_http_response();
}


TEST(RestClient, test_SetInvalidPassword) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  uint32_t retval =  rest_obj.set_username_password("sd", "");
  EXPECT_EQ(retval, uint(1));
  rest_obj.clear_http_response();
}


TEST(RestClient, test_SetInvalidUserNamePassword) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  uint32_t retval =  rest_obj.set_username_password("", "");
  EXPECT_EQ(retval, uint(1));
  rest_obj.clear_http_response();
}


TEST(RestClient, test_SetValidUserNamePassword) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  uint32_t retval =  rest_obj.set_username_password("sdf", "dd");
  EXPECT_EQ(retval, uint(0));
  rest_obj.clear_http_response();
}

TEST(RestClient, test_SetTimeout) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  uint32_t retval =  rest_obj.set_timeout(0, 0);
  EXPECT_EQ(retval, uint(0));
  rest_obj.clear_http_response();
}

TEST(RestClient, test_SetTimeoutValue) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  uint32_t retval =  rest_obj.set_timeout(10, 10);
  EXPECT_EQ(retval, uint(0));
  rest_obj.clear_http_response();
}

TEST(RestClient, test_set_request_body_Null) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  uint32_t retval =  rest_obj.set_request_body("");
  EXPECT_EQ(retval, uint(1));
  rest_obj.clear_http_response();
}


TEST(RestClient, test_set_request_body_Value) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  uint32_t retval =  rest_obj.set_request_body("jdfgdfhdghsdffhsd");
  EXPECT_EQ(retval, uint(0));
  rest_obj.clear_http_response();
}


TEST(RestClient, test_set_request_body_LargeVal) {
  unc::restjson::RestClient rest_obj("11", "11" , 12 ,
                                     unc::restjson::HTTP_METHOD_POST);
  uint32_t retval =
      rest_obj.set_request_body("jdfgdfhdghsdffhsdhsdgksdhkjsdgsdh"
                                "fsdfgsdfshkdfhsjdfsdfksdfsjdfjerte"
                                "jkhrtekrkgjdfk;fglhljghkfjghkflgkh"
                                "fghjfkghkfgjhfkghkfjghfgjgkfghfghf"
                                "ghjhkfgjjsdghsgdfhgsdgfsdgfgsdhfgj"
                                "sgjdfhsdgfjshdgfjshdjshdgfjhsdgfhs"
                                "gdhfgsdfgsgdfhsdfhsdfh");
  EXPECT_EQ(retval, uint(0));
  rest_obj.clear_http_response();
}


