/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbr.hh>
#include <odc_controller.hh>
#include <gtest/gtest.h>
#include <string>


TEST(odcdriver,  test_create_cmd_vbr) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string CREATE_201  = "172.16.0.1";

  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(val_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS,  obj.create_cmd(key_vbr,  val_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_delete_cmd_vbr) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string UPDATE_DELETE_200  = "172.16.0.2";
  inet_aton(UPDATE_DELETE_200.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(val_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.delete_cmd(key_vbr,  val_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_update_cmd_vbr) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val1_vbr;
  val_vbr_t val2_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val1_vbr,  0,  sizeof(val_vbr_t));
  memset(&val2_vbr,  0,  sizeof(val_vbr_t));
  std::string UPDATE_DELETE_200  = "172.16.0.2";
  inet_aton(UPDATE_DELETE_200.c_str(),  &val_ctr.ip_address);
  val1_vbr.valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val1_vbr.vbr_description),
          description.c_str(),  sizeof(val1_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.update_cmd(key_vbr,  val1_vbr, val2_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}


TEST(odcdriver,  test_invalid_vtnname_create_cmd_vbr) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(val_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(key_vbr,  val_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_create_cmd_invalid_vbr_name) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(val_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  obj.create_cmd(key_vbr,  val_vbr,  ctr));
  delete ctr;
}
TEST(odcdriver,  test_invalid_vtnname_update_cmd) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val1_vbr;
  val_vbr_t val2_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val1_vbr,  0,  sizeof(val_vbr_t));
  memset(&val2_vbr,  0,  sizeof(val_vbr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val1_vbr.vbr_description),
          description.c_str(),  sizeof(val2_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  obj.update_cmd(key_vbr,  val1_vbr, val2_vbr, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_update_cmd_invalid_vbr_name) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val1_vbr;
  val_vbr_t val2_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val1_vbr,  0,  sizeof(val_vbr_t));
  memset(&val2_vbr,  0,  sizeof(val_vbr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val1_vbr.vbr_description),
          description.c_str(),  sizeof(val1_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.update_cmd(key_vbr,  val1_vbr, val2_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}


TEST(odcdriver,  test_invalid_vtnname_delete_cmd_vbr) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(val_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.delete_cmd(key_vbr,  val_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_delete_cmd_invalid_vbr_name) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(val_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  obj.delete_cmd(key_vbr,  val_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}



TEST(odcdriver,  test_delete_cmd_invalid) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string INVALID_RESPONSE = "172.0.0.0";
  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(val_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.delete_cmd(key_vbr,  val_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_create_cmd_invalid) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string INVALID_RESPONSE = "172.0.0.0";
  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(val_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(key_vbr,  val_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}


TEST(odcdriver,  test_update_cmd_invalid) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val1_vbr;
  val_vbr_t val2_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val1_vbr,  0,  sizeof(val_vbr_t));
  memset(&val2_vbr,  0,  sizeof(val_vbr_t));

  std::string INVALID_RESPONSE = "172.0.0.0";

  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val1_vbr.vbr_description),
          description.c_str(),  sizeof(val1_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.update_cmd(key_vbr,  val1_vbr, val2_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_create_cmd_null_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));

  std::string NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(val_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(key_vbr,  val_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_update_cmd_null_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val1_vbr;
  val_vbr_t val2_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val1_vbr,  0,  sizeof(val_vbr_t));
  memset(&val2_vbr,  0,  sizeof(val_vbr_t));

  std::string NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val1_vbr.vbr_description),
          description.c_str(),  sizeof(val1_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.update_cmd(key_vbr,  val1_vbr, val2_vbr, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_delete_cmd_null_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(val_vbr.vbr_description)-1);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.delete_cmd(key_vbr,  val_vbr,  ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_get_vbr_list_empty_vtn_name) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  key_vtn_t key_vtn;
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char *>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  std::vector<unc::vtndrvcache::ConfigNode *> value_list;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_vtn, value_list));
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       value_list.begin(); it !=  value_list.end(); ++it ) {
    if (*it !=  NULL) {
      delete *it;
    }
  }

  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_get_vbr_list_null_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  key_vtn_t key_vtn;
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char *>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  std::vector<unc::vtndrvcache::ConfigNode *> value_list;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_vtn, value_list));
  EXPECT_EQ(0U, value_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       value_list.begin(); it !=  value_list.end(); ++it ) {
    if (*it !=  NULL) {
      delete *it;
    }
  }

  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_get_vbr_list_invalid_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  key_vtn_t key_vtn;
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char *>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string INVALID_RESPONSE = "172.0.0.0";
  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  std::vector<unc::vtndrvcache::ConfigNode *> value_list;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_vtn, value_list));
  EXPECT_EQ(0U, value_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       value_list.begin(); it !=  value_list.end(); ++it ) {
    if (*it !=  NULL) {
      delete *it;
    }
  }

  delete ctr;
  ctr = NULL;
}
TEST(odcdriver,  test_get_vbr_list_valid_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  key_vtn_t key_vtn;
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char *>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string READ_VBR_VALID = "172.16.0.8";

  inet_aton(READ_VBR_VALID.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  std::vector<unc::vtndrvcache::ConfigNode *> value_list;
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.fetch_config(ctr, &key_vtn, value_list));
  EXPECT_EQ(2U, value_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       value_list.begin(); it !=  value_list.end(); ++it ) {
    if (*it !=  NULL) {
      delete *it;
    }
  }

  delete ctr;
  ctr = NULL;
}

TEST(odcdriver,  test_get_vbr_list_valid_resp_no_vbr) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  key_vtn_t key_vtn;
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char *>(key_vtn.vtn_name),
          vtnname.c_str(),  sizeof(key_vtn.vtn_name)-1);
  std::string READ_VBR_NULL_DATA = "172.16.0.9";

  inet_aton(READ_VBR_NULL_DATA.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrCommand obj(conf_file);
  std::vector<unc::vtndrvcache::ConfigNode *> value_list;
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.fetch_config(ctr, &key_vtn, value_list));
  EXPECT_EQ(0U, value_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       value_list.begin(); it !=  value_list.end(); ++it ) {
    if (*it !=  NULL) {
      delete *it;
    }
  }

  delete ctr;
  ctr = NULL;
}

