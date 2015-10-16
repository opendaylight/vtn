/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vtn.hh>
#include <odc_controller.hh>
#include <odc_driver_common_defs.hh>
#include <gtest/gtest.h>
#include <string>

TEST(odcdriver, test_in_valid_vtnname_empty_create_cmd) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";

  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC , obj.create_cmd(key_vtn, val_vtn, ctr));
  delete ctr;
  ctr= NULL;
}




TEST(odcdriver, test_in_valid_vtnname_empty_update_cmd) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val1_vtn;
  val_vtn_t val2_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC , obj.update_cmd(key_vtn, val1_vtn, val2_vtn, ctr));
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver, test_in_valid_vtnname_empty_delete_cmd) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC , obj.delete_cmd(key_vtn, val_vtn, ctr));

  delete ctr;
  ctr= NULL;
}

TEST(odcdriver, test_invalid_ipaddress_empty_create_cmd) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC , obj.create_cmd(key_vtn, val_vtn, ctr));
  if (ctr != NULL) {
    delete ctr;
    ctr= NULL;
  }
}

TEST(odcdriver, test_null_response_update_cmd) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val1_vtn;
  val_vtn_t val2_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val1_vtn,  0,  sizeof(val_vtn_t));
  memset(&val2_vtn,  0,  sizeof(val_vtn_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string NULL_RESPONSE = "172.16.0.0";

  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC ,
            obj.update_cmd(key_vtn, val1_vtn, val2_vtn, ctr));
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver, test_null_response_delete_cmd) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));
  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));

  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string NULL_RESPONSE = "172.16.0.0";

  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC ,
            obj.delete_cmd(key_vtn, val_vtn, ctr));
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver,  test_delete_cmd) {
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  unc::restjson::ConfFileValues_t conf_values;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));
  std::string desc1 =  "description";
  strncpy(reinterpret_cast<char*>(val_vtn.description),
          desc1.c_str(),  sizeof(val_vtn.description)-1);
  std::string vtnname =  "vtn1";
  val_vtn.valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string UPDATE_DELETE_200  = "172.16.0.2";

  inet_aton(UPDATE_DELETE_200.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj.delete_cmd(key_vtn, val_vtn, ctr));
  delete ctr;
  ctr= NULL;
}



TEST(odcdriver,  test_delete_cmd_failure) {
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  unc::restjson::ConfFileValues_t conf_values;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));
  std::string desc1 =  "description";
  strncpy(reinterpret_cast<char*>(val_vtn.description),
          desc1.c_str(),  sizeof(val_vtn.description)-1);
  unc::driver::controller* ctr  =  new  unc::odcdriver::OdcController
      (key_ctr,  val_ctr, conf_values);;
  std::string INVALID_RESPONSE = "172.0.0.0";
  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);

  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name) ,
          vtnname.c_str(),  sizeof(vtnname));
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.delete_cmd(key_vtn, val_vtn, ctr));
  delete ctr;
  ctr= NULL;
}



TEST(odcdriver,  invalid_response_create_cmd) {
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  unc::restjson::ConfFileValues_t conf_values;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));
  std::string INVALID_RESPONSE = "172.0.0.0";
  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name) ,
          vtnname.c_str(),  sizeof(vtnname));
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.create_cmd(key_vtn, val_vtn, ctr));
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver,  invalid_response_update_cmd) {
  key_vtn_t key_vtn;
  val_vtn_t val1_vtn;
  val_vtn_t val2_vtn;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val1_vtn,  0,  sizeof(val_vtn_t));
  memset(&val2_vtn,  0,  sizeof(val_vtn_t));
  std::string INVALID_RESPONSE = "172.0.0.0";
  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name) ,
          vtnname.c_str(),  sizeof(vtnname));
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.update_cmd(key_vtn, val1_vtn, val2_vtn,  ctr));
  delete ctr;
  ctr= NULL;
}


TEST(odcdriver,  test_update_cmd) {
  key_vtn_t key_vtn;
  val_vtn_t val1_vtn;
  val_vtn_t val2_vtn;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val1_vtn,  0,  sizeof(val_vtn_t));
  memset(&val2_vtn,  0,  sizeof(val_vtn_t));
  std::string desc1 =  "description";
  strncpy(reinterpret_cast<char*>(val1_vtn.description),
          desc1.c_str(),  sizeof(val1_vtn.description)-1);
  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name) ,
          vtnname.c_str(),  sizeof(vtnname));
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj.update_cmd(key_vtn, val1_vtn, val2_vtn, ctr));
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver,  test_valid_create_cmd) {
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));
  std::string desc1 =  "description";
  strncpy(reinterpret_cast<char*>(val_vtn.description),
          desc1.c_str(),  sizeof(val_vtn.description)-1);
  std::string CREATE_201      = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name) ,
          vtnname.c_str(),  sizeof(vtnname));
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj.create_cmd(key_vtn, val_vtn, ctr));
  delete ctr;
  ctr= NULL;
}



TEST(odcdriver,  read_all_valid_) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char *>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string desc1 =  "descrption";
  strncpy(reinterpret_cast<char *>(val_vtn.description),
          desc1.c_str(),  sizeof(val_vtn.description)-1);
  val_vtn.valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  std::string UPDATE_DELETE_200  = "172.16.0.2";
  inet_aton(UPDATE_DELETE_200.c_str(),  &val_ctr.ip_address);
  std::string controller_name =  "ctr1";
  strncpy(reinterpret_cast<char *>(key_ctr.controller_name),
          controller_name.c_str(),  sizeof(key_ctr.controller_name)-1);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  std::vector<unc::vtndrvcache::ConfigNode *> value_list;
  void *ptr = NULL;
  EXPECT_EQ(UNC_RC_SUCCESS,  obj.fetch_config(ctr, ptr, value_list));
  EXPECT_EQ(2U,  value_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       value_list.begin(); it != value_list.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
  ctr= NULL;
}
TEST(odcdriver,  read_all_invalid_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char *>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string desc1 =  "descrption";
  strncpy(reinterpret_cast<char *>(val_vtn.description),
          desc1.c_str(),  sizeof(val_vtn.description)-1);
  val_vtn.valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  std::string INVALID_RESPONSE = "172.0.0.0";

  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  std::string controller_name =  "ctr1";
  strncpy(reinterpret_cast<char *>(key_ctr.controller_name),
          controller_name.c_str(),  sizeof(key_ctr.controller_name)-1);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  std::vector<unc::vtndrvcache::ConfigNode *> value_list;
  void *ptr = NULL;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  obj.fetch_config(ctr, ptr, value_list));
  EXPECT_EQ(0U,  value_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       value_list.begin(); it != value_list.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
  ctr= NULL;
}

TEST(odcdriver,  get_vtn_list_invalid_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char *>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string desc1 =  "descrption";
  strncpy(reinterpret_cast<char *>(val_vtn.description),
          desc1.c_str(),  sizeof(val_vtn.description)-1);

  std::string  NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  std::string controller_name =  "ctr1";
  strncpy(reinterpret_cast<char *>(key_ctr.controller_name),
          controller_name.c_str(),  sizeof(key_ctr.controller_name)-1);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  std::vector<unc::vtndrvcache::ConfigNode *> value_list;
  void *ptr = NULL;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, ptr, value_list));
  EXPECT_EQ(0U,  value_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       value_list.begin(); it != value_list.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
  ctr= NULL;
}

TEST(odcdriver,  read_all_valid_resp_no_vtn) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char *>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string desc1 =  "descrption";
  strncpy(reinterpret_cast<char *>(val_vtn.description),
          desc1.c_str(),  sizeof(val_vtn.description)-1);
  std::string READ_VTN_NULL_DATA     = "172.16.0.3";

  inet_aton(READ_VTN_NULL_DATA.c_str(),  &val_ctr.ip_address);
  std::string controller_name =  "ctr1";
  strncpy(reinterpret_cast<char *>(key_ctr.controller_name),
          controller_name.c_str(),  sizeof(key_ctr.controller_name)-1);
  unc::driver::controller* ctr  =  new
      unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVtnCommand obj(conf_file);
  std::vector<unc::vtndrvcache::ConfigNode *> value_list;
  void *ptr = NULL;
  EXPECT_EQ(UNC_RC_NO_SUCH_INSTANCE, obj.fetch_config(ctr, ptr, value_list));
  EXPECT_EQ(0U,  value_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       value_list.begin(); it != value_list.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
  ctr= NULL;
}

TEST(odcdriver, test_create_cmd_controller_username) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr, 0 , sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&key_vtn, 0 , sizeof(key_vtn_t));
  memset(&val_vtn,  0,  sizeof(val_vtn_t));
  std::string vtnname =  "ff";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "";
  conf_file.password = "";

  std::string user = "ctr_user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user.c_str(),  sizeof(val_ctr.user)-1);

  std::string pass = "ctr_pass";
  strncpy(reinterpret_cast<char*>(val_ctr.password),
          pass.c_str(),  sizeof(val_ctr.password)-1);

  unc::odcdriver::OdcVtnCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC , obj.create_cmd(key_vtn, val_vtn, ctr));
  delete ctr;
  ctr= NULL;
}

