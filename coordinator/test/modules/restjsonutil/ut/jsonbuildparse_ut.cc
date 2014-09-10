/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <json_type_util.hh>
#include <json_build_parse.hh>

#include <gtest/gtest.h>
#include <string>

TEST(JsonBuildParse , CreateObj) {
  unc::restjson::JsonBuildParse obj;
  json_object *jobj = obj.create_json_obj();
  EXPECT_TRUE(jobj != NULL);
  json_object_put(jobj);
}

TEST(JsonBuildParse , Build) {
  unc::restjson::JsonBuildParse obj;
  json_object *jobj = json_object_new_object();
  json_object *jstring = json_object_new_string("dnu1");
  obj.build("Name", jstring , jobj);
  char *str = const_cast<char *>("{ \"Name\": \"dnu1\" }");
  EXPECT_STREQ(str, json_object_to_json_string(jobj));
  json_object_put(jobj);
}

TEST(JsonBuildParse , Build_int) {
  unc::restjson::JsonBuildParse obj;
  json_object *jobj = json_object_new_object();
  json_object *jint = json_object_new_int(10);
  obj.build("Name", jint , jobj);
  char *str =  const_cast<char *>("{ \"Name\": 10 }");
  EXPECT_STREQ(str, json_object_to_json_string(jobj));
  json_object_put(jobj);
}

TEST(JsonBuildParse , Build_null) {
  unc::restjson::JsonBuildParse obj;
  json_object *jobj = json_object_new_object();
  json_object *jint = NULL;
  obj.build("Name", jint , jobj);
  char *str = const_cast<char *>("{ \"Name\": null }");
  EXPECT_STREQ(str, json_object_to_json_string(jobj));
  json_object_put(jobj);
}

TEST(JsonBuildParse , Build_null_pattern1) {
  unc::restjson::JsonBuildParse obj;
  json_object *jobj = json_object_new_object();
  json_object *jint = NULL;
  obj.build("", jint , jobj);
  char *str =  const_cast<char *>("{ \"\": null }");
  EXPECT_STREQ(str, json_object_to_json_string(jobj));
  json_object_put(jobj);
}

TEST(JsonBuildParse , Build_null_pattern2) {
  unc::restjson::JsonBuildParse obj;
  json_object *jobj;
  jobj = NULL;
  json_object *jint = NULL;
  obj.build("", jint , jobj);
  EXPECT_STREQ("null", json_object_to_json_string(jobj));
  json_object_put(jint);
  json_object_put(jobj);
}


TEST(JsonBuildParse , Build_str) {
  unc::restjson::JsonBuildParse obj;
  json_object *jobj = json_object_new_object();
  json_object *jobj1 = json_object_new_object();
  json_object *jstring = json_object_new_string("dnu1");
  json_object *jint = json_object_new_int(10);
  obj.build("Name", jstring , jobj);
  obj.build("Name1" , jint , jobj1);
  obj.build("Example", jobj1 , jobj);
  char *str =  const_cast<char *>
      ("{ \"Name\": \"dnu1\", \"Example\": { \"Name1\": 10 } }");
  EXPECT_STREQ(str, json_object_to_json_string(jobj));
  json_object_put(jobj);
}

TEST(JsonBuildParse , Build_str_pattern) {
  unc::restjson::JsonBuildParse obj;
  json_object *jobj = json_object_new_object();
  json_object *jobj1 = json_object_new_object();
  json_object *jobj2 = json_object_new_object();
  json_object *jstring = json_object_new_string("dnu1");
  json_object *jint = json_object_new_int(10);
  obj.build("Name" , jstring , jobj);
  obj.build("Name1" , jint , jobj1);
  obj.build("Name2", 0 , jobj2);
  obj.build("Examplee", jobj2 , jobj1);
  obj.build("Example", jobj1, jobj);
  std::string str1 = "{ \"Name\": \"dnu1\", \"Example\": { \"Name1\": 10,";
  std::string str2 = " \"Examplee\": { \"Name2\": 0 } } }";
  str1.append(str2);

  const char *str = reinterpret_cast<const char*> (str1.c_str());
  EXPECT_STREQ(str, json_object_to_json_string(jobj));
  json_object_put(jobj);
}
TEST(JsonBuildParse , get_array_length_size_case1) {
  unc::restjson::JsonBuildParse obj;
  char *str = const_cast<char *>
      ("{\"example\" : [ { \"Name\": 10, \"Example\": \"dhanu1\" } ]}");
  json_object *jobj = json_tokener_parse(str);
  int arr_len = obj.get_array_length(jobj, "example");
  int len = 1;
  EXPECT_EQ(len, arr_len);
  json_object_put(jobj);
}

TEST(JsonBuildParse , get_array_length_null) {
  unc::restjson::JsonBuildParse obj;
  char *str = const_cast<char *>
      ("{\"example\" : [ { \"Name\": 10, \"Example\": \"dhanu1\" } ]}");
  json_object *jobj = json_tokener_parse(str);
  int arr_len = obj.get_array_length(jobj, "example");
  EXPECT_NE(arr_len , 0);
  json_object_put(jobj);
}

TEST(JsonBuildParse , get_array_length_null_case1) {
  unc::restjson::JsonBuildParse obj;
  char *str = const_cast<char *>("{\"example\" : {} }");
  json_object *jobj = json_tokener_parse(str);
  int arr_len = obj.get_array_length(jobj, "example");
  EXPECT_EQ(arr_len, 0);
  json_object_put(jobj);
}

TEST(JsonBuildParse , get_array_length_null_case2) {
  unc::restjson::JsonBuildParse obj;
  char *str = const_cast<char *>("{ }");
  json_object *jobj = json_tokener_parse(str);
  int arr_len = obj.get_array_length(jobj, "");
  EXPECT_EQ(arr_len, 0);
  json_object_put(jobj);
}

TEST(JsonBuildParse , get_array_length_nested_arr) {
  unc::restjson::JsonBuildParse obj;
  std::string str1 = "{\"example\" : [ { \"Name\": 10, ";
  std::string str2 = "\"categories\": [\"c\",[\"c++\",\"c\"] ]} ]}";
  str1.append(str2);
  char *str = const_cast<char *> (str1.c_str());
  json_object *jobj = json_tokener_parse(str);
  int arr_len = obj.get_array_length(jobj, "example");
  int len = 1;
  EXPECT_EQ(len, arr_len);
  json_object_put(jobj);
}

TEST(JsonBuildParse , get_array_length_nested_arr_case1) {
  unc::restjson::JsonBuildParse obj;
  std::string str1 = "{\"example\" :  { \"Name\": 10, \"categories\"";
  std::string str2 = ": [\"c\",[\"c++\",\"c\"] ]} }";
  str1.append(str2);
  char *str = const_cast<char *>(str1.c_str());
  json_object *jobj = json_tokener_parse(str);
  int arr_len = obj.get_array_length(jobj, "example");
  int len = 0;
  EXPECT_EQ(len, arr_len);
  json_object_put(jobj);
}

TEST(JsonBuildParse , get_array_length_nested_arr_case2) {
  unc::restjson::JsonBuildParse obj;
  char *str = const_cast<char *>
      ("{\"example\" :  { \"Name\": 10, \"categories\": [{\"c\" : 10}]} }");
  json_object *jobj = json_tokener_parse(str);
  json_object *jobj_value(NULL);
  EXPECT_EQ(TRUE, json_object_object_get_ex(jobj, "example", &jobj_value));
  int arr_len = obj.get_array_length(jobj_value, "categories");
  int len = 1;
  EXPECT_EQ(len, arr_len);
  json_object_put(jobj);
}


TEST(JsonBuildParse , Parse_arr) {
  unc::restjson::JsonBuildParse obj;
  const char *str = reinterpret_cast<const char *>(
      "{\"example\" : [ { \"Name\": 10, \"Example\": \"dhanu1\" }, { \"Name\": 11, \"Sam\": \"dhanu11\" } ]}");
  json_object *jobj = json_tokener_parse(str);
  json_object *val = NULL;
  obj.parse(jobj, "example", -1, val);
  uint intval = 0;
  obj.parse(val, "Name", 0, intval);
  EXPECT_EQ(10U, intval);
  json_object_put(jobj);
}

TEST(JsonBuildParse , Parse_str) {
  unc::restjson::JsonBuildParse obj;
  const char *str =
      reinterpret_cast<const char *>(
          "{\"sitename\" : \"joys of programming\",\"categories\": \"kl\"}");
  json_object *jobj = json_tokener_parse(str);
  json_object *val = NULL;
  obj.parse(jobj, "sitename", -1, val);
  std::string str1 = obj.get_json_string(val);
  std::string str2 = "\"joys of programming\"";
  EXPECT_EQ(str2, str1);
  json_object_put(jobj);
}

TEST(JsonBuildParse , Parse_null) {
  unc::restjson::JsonBuildParse obj;
  json_object *jobj = NULL;
  json_object *val = NULL;
  int ret_val = obj.parse(jobj, "null", -1, val);
  EXPECT_EQ(ret_val, unc::restjson::REST_OP_FAILURE);
  json_object_put(jobj);
}

TEST(JsonBuildParse , Parse_str_case1) {
  unc::restjson::JsonBuildParse obj;
  const char *str =
      reinterpret_cast<const char *>(
          "{\"sitename\" : [{\"port_no\" : 1, \"name\" : \"eth0\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{ \"port_no\" : 2, \"name\" : \"eth1\", \"registered_at\": \"2013-04-24T23:59:00Z\"},{\"port_no\" : 1, \"name\" : \"vxlan1\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{\"port_no\" : 65534, \"name\" : \"br1\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{\"port_no\" : 65534, \"name\" : \"br0\", \"registered_at\": \"2013-04-24T23:59:00Z\" }]}");

  json_object *jobj = json_tokener_parse(str);
  json_object *val = NULL;
  obj.parse(jobj, "sitename", -1, val);
  std::string str1 = obj.get_json_string(val);
  std::string str2 =
      "[ { \"port_no\": 1, \"name\": \"eth0\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 2, \"name\": \"eth1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 1, \"name\": \"vxlan1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 65534, \"name\": \"br1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 65534, \"name\": \"br0\", \"registered_at\": \"2013-04-24T23:59:00Z\" } ]";
  EXPECT_EQ(str2, str1);
  int intval = 0;
  obj.parse(val, "port_no", 0, intval);
  EXPECT_EQ(1, intval);
  json_object_put(jobj);
}


TEST(JsonBuildParse , Parse_arr_ind) {
  unc::restjson::JsonBuildParse obj;
  const char *str =
      reinterpret_cast<const char *>(
          "{\"sitename\" : [{\"port_no\" : 1, \"name\" : \"eth0\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{ \"port_no\" : 2, \"name\" : \"eth1\", \"registered_at\": \"2013-04-24T23:59:00Z\"},{\"port_no\" : 1, \"name\" : \"vxlan1\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{\"port_no\" : 65534, \"name\" : \"br1\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{\"port_no\" : 65534, \"name\" : \"br0\", \"registered_at\": \"2013-04-24T23:59:00Z\" }]}");

  json_object *jobj = json_tokener_parse(str);
  json_object *val = NULL;
  obj.parse(jobj, "sitename", -1, val);
  std::string str1 = obj.get_json_string(val);
  std::string str2 =
      "[ { \"port_no\": 1, \"name\": \"eth0\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 2, \"name\": \"eth1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 1, \"name\": \"vxlan1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 65534, \"name\": \"br1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 65534, \"name\": \"br0\", \"registered_at\": \"2013-04-24T23:59:00Z\" } ]";
  EXPECT_EQ(str2, str1);
  uint intval = 0;
  obj.parse(val, "port_no", 2, intval);
  EXPECT_EQ(1U, intval);
  json_object_put(jobj);
}

TEST(JsonBuildParse , Parse_arr_index) {
  unc::restjson::JsonBuildParse obj;
  const char *str =
      reinterpret_cast<const char *>(
          "{\"sitename\" : [{\"port_no\" : 1, \"name\" : \"eth0\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{ \"port_no\" : 2, \"name\" : \"eth1\", \"registered_at\": \"2013-04-24T23:59:00Z\"},{\"port_no\" : 1, \"name\" : \"vxlan1\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{\"port_no\" : 65534, \"name\" : \"br1\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{\"port_no\" : 65534, \"name\" : \"br0\", \"registered_at\": \"2013-04-24T23:59:00Z\" }]}");

  json_object *jobj = json_tokener_parse(str);
  json_object *val = NULL;
  obj.parse(jobj, "sitename", -1, val);
  std::string str1 = obj.get_json_string(val);
  std::string str2 =
      "[ { \"port_no\": 1, \"name\": \"eth0\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 2, \"name\": \"eth1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 1, \"name\": \"vxlan1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 65534, \"name\": \"br1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 65534, \"name\": \"br0\", \"registered_at\": \"2013-04-24T23:59:00Z\" } ]";
  EXPECT_EQ(str2, str1);
  int intval = 0;
  obj.parse(val, "port_nos", 0, intval);
  EXPECT_EQ(0, intval);
  json_object_put(jobj);
}

TEST(JsonBuildParse , Parse_arr_index_case1) {
  unc::restjson::JsonBuildParse obj;
  const char *str =
      reinterpret_cast<const char *>(
          "{\"sitename\" : [{\"port_no\" : 1, \"name\" : \"eth0\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{ \"port_no\" : 2, \"name\" : \"eth1\", \"registered_at\": \"2013-04-24T23:59:00Z\"},{\"port_no\" : 1, \"name\" : \"vxlan1\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{\"port_no\" : 65534, \"name\" : \"br1\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{\"port_no\" : 65534, \"name\" : \"br0\", \"registered_at\": \"2013-04-24T23:59:00Z\" }]}");

  json_object *jobj = json_tokener_parse(str);
  json_object *val = NULL;
  obj.parse(jobj, "sitename", -1, val);
  std::string str1 = obj.get_json_string(val);
  std::string str2 =
      "[ { \"port_no\": 1, \"name\": \"eth0\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 2, \"name\": \"eth1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 1, \"name\": \"vxlan1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 65534, \"name\": \"br1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 65534, \"name\": \"br0\", \"registered_at\": \"2013-04-24T23:59:00Z\" } ]";
  EXPECT_EQ(str2, str1);
  std::string intval;
  obj.parse(val, "name", 2, intval);
  EXPECT_EQ("vxlan1", intval);
  json_object_put(jobj);
}

TEST(JsonBuildParse , Parse_arr_indexi_case2) {
  unc::restjson::JsonBuildParse obj;
  const char *str =
      reinterpret_cast<const char *>(
          "{\"sitename\" : [{\"port_no\" : NULL, \"name\" : \"eth0\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{ \"port_no\" : 2, \"name\" : \"eth1\", \"registered_at\": \"2013-04-24T23:59:00Z\"},{\"port_no\" : 1, \"name\" : \"vxlan1\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{\"port_no\" : 65534, \"name\" : \"br1\", \"registered_at\": \"2013-04-24T23:59:00Z\" },{\"port_no\" : 65534, \"name\" : \"br0\", \"registered_at\": \"2013-04-24T23:59:00Z\" }]}");

  json_object *jobj = json_tokener_parse(str);
  json_object *val = NULL;
  obj.parse(jobj, "sitename", -1, val);
  std::string str1 = obj.get_json_string(val);
  std::string str2 =
      "[ { \"port_no\": null, \"name\": \"eth0\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 2, \"name\": \"eth1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 1, \"name\": \"vxlan1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 65534, \"name\": \"br1\", \"registered_at\": \"2013-04-24T23:59:00Z\" }, { \"port_no\": 65534, \"name\": \"br0\", \"registered_at\": \"2013-04-24T23:59:00Z\" } ]";
  EXPECT_EQ(str2, str1);
  int intval = 0;
  obj.parse(val, "port_no", 1, intval);
  EXPECT_EQ(2, intval);
  json_object_put(jobj);
}


TEST(JsonBuildParse , Parse_simple_arr) {
  unc::restjson::JsonBuildParse obj;
  const char *str =
      reinterpret_cast<const char *>(
          "{\"sitename\" : [{\"port_no\" : NULL, \"name\" : \"eth0\"}, {\"registered_at\": \"2013-04-24T23:59:00Z\" }] }");
  json_object *jobj = json_tokener_parse(str);
  json_object *val = NULL;
  obj.parse(jobj, "sitename", -1, val);
  std::string str1 = obj.get_json_string(val);
  std::string str2 =
      "[ { \"port_no\": null, \"name\": \"eth0\" }, { \"registered_at\": \"2013-04-24T23:59:00Z\" } ]";
  EXPECT_EQ(str2, str1);
  std::string intval;
  obj.parse(val, "name", 0, intval);
  EXPECT_EQ("eth0", intval);
  json_object_put(jobj);
}



TEST(JsonBuildParse , getval) {
  json_object *jobj = json_object_new_string("value");
  std::string val;
  unc::restjson::JsonTypeUtil::get_value(jobj, val);
  EXPECT_EQ("value", val);
  json_object_put(jobj);
}

TEST(JsonBuildParse , getval_int) {
  json_object *jobj = json_object_new_int(10);
  uint val = 0;
  unc::restjson::JsonTypeUtil::get_value(jobj, val);
  EXPECT_EQ(10U, val);
  json_object_put(jobj);
}

TEST(JsonBuildParse , getval_arr) {
  const char *str =
      reinterpret_cast<const char *>(
          "{\"sitename\" : [{\"port_no\" : NULL, \"name\" : \"eth0\"}, {\"registered_at\": \"2013-04-24T23:59:00Z\" }] }");
  json_object *jobj = json_tokener_parse(str);
  json_object *val;
  unc::restjson::JsonTypeUtil::get_value(jobj, val);
  EXPECT_EQ(jobj, val);
  json_object_put(jobj);
}

TEST(JsonBuildParse , getval_str) {
  json_object *jobj = json_object_new_string("value");
  std::string val;
  unc::restjson::JsonTypeUtil::get_value(jobj, val);
  EXPECT_NE("value1" , val);
  json_object_put(jobj);
}

TEST(JsonBuildParse , getval_int_val) {
  json_object *jobj = json_object_new_int(10);
  uint val = 0;
  unc::restjson::JsonTypeUtil::get_value(jobj, val);
  EXPECT_NE(0U, val);
  json_object_put(jobj);
}

TEST(JsonBuildParse , getval_int_fail) {
  json_object *jobj = json_object_new_int(10);
  uint val = 0;
  unc::restjson::JsonTypeUtil::get_value(jobj, val);
  EXPECT_NE(1U, val);
  json_object_put(jobj);
}

TEST(JsonBuildParse , getJson) {
  json_object *jobj1 = json_object_new_object();
  unc::restjson::JsonTypeUtil obj(jobj1);
  obj.get_json_data();

  EXPECT_TRUE(obj.get_json_data() != NULL);
  json_object_put(jobj1);
}



TEST(JsonBuildParse , get_array_length_null1) {
  unc::restjson::JsonBuildParse obj;
  char *str = const_cast<char *>("{{\"port_no\" : NULL}");
  json_object *jobj = json_tokener_parse(str);
  int arr_len = obj.get_array_length(jobj, "port_no");
  int len = 0;
  EXPECT_EQ(len, arr_len);
  json_object_put(jobj);
}

TEST(JsonBuildParse , get_json_object) {
  unc::restjson::JsonBuildParse obj;
  json_object *jobj1 = NULL;
  char *string =  const_cast<char *>("{ \"Name\": \"dnu1\" }");
  jobj1 = obj.get_json_object(string);
  EXPECT_TRUE(jobj1 != NULL);
  json_object_put(jobj1);
}



