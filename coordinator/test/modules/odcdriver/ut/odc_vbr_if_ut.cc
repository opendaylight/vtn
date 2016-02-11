/*
 * Copyright (c) 2013-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbrif.hh>
#include <odc_controller.hh>

#include <gtest/gtest.h>
#include <string>


TEST(odcdriver,  create_cmd_valid) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string CREATE_201      = "172.16.0.1";

  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  vbrif_val.val_vbrif.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID;
  vbrif_val.val_vbrif.valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  vbrif_val.val_vbrif.admin_status = UPLL_ADMIN_ENABLE;
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}



TEST(odcdriver,  create_cmd_valid_admin_disable) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string CREATE_201      = "172.16.0.1";

  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr,conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  vbrif_val.val_vbrif.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID;
  vbrif_val.val_vbrif.valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  vbrif_val.val_vbrif.admin_status = UPLL_ADMIN_DISABLE;
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_with_no_decription) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string CREATE_201      = "172.16.0.1";

  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  vbrif_val.val_vbrif.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  vbrif_val.val_vbrif.valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  vbrif_val.val_vbrif.admin_status = UPLL_ADMIN_ENABLE;
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_invalid_resp) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string INVALID_RESPONSE = "172.0.0.0";

  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  std::string pass_word =  "pass";
  strncpy(reinterpret_cast<char*>(val_ctr.password),
          pass_word.c_str(),  sizeof(val_ctr.password));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}
TEST(odcdriver,  update_cmd_invalid_resp) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string INVALID_RESPONSE = "172.0.0.0";

  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  std::string pass_word =  "pass";
  strncpy(reinterpret_cast<char*>(val_ctr.password),
          pass_word.c_str(),  sizeof(val_ctr.password));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.update_cmd(vbrif_key,  vbrif_val1, vbrif_val2,  ctr));
  delete ctr;
}
TEST(odcdriver,  delete_cmd_invalid_resp) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string INVALID_RESPONSE = "172.0.0.0";

  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  std::string pass_word =  "pass";
  strncpy(reinterpret_cast<char*>(val_ctr.password),
          pass_word.c_str(),  sizeof(val_ctr.password));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.delete_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}
TEST(odcdriver,  create_cmd_null_resp) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string NULL_RESPONSE = "172.16.0.0";


  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  std::string pass_word =  "pass";
  strncpy(reinterpret_cast<char*>(val_ctr.password),
          pass_word.c_str(),  sizeof(val_ctr.password));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  update_cmd_null_resp) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string NULL_RESPONSE = "172.16.0.0";


  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  std::string pass_word =  "pass";
  strncpy(reinterpret_cast<char*>(val_ctr.password),
          pass_word.c_str(),  sizeof(val_ctr.password));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}

TEST(odcdriver,  delete_cmd_null_resp) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string NULL_RESPONSE = "172.16.0.0";


  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  std::string pass_word =  "pass";
  strncpy(reinterpret_cast<char*>(val_ctr.password),
          pass_word.c_str(),  sizeof(val_ctr.password));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.delete_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}



TEST(odcdriver,  create_cmd_empty_vtn) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.5";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}


TEST(odcdriver,  create_cmd_empty_vbr) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.5";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_empty_vbr_if) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.5";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}
TEST(odcdriver,  create_cmd_port_map_valid_no_values) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";

  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_create";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>
          (vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}
TEST(odcdriver,  create_cmd_port_map_valid_values) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.3.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "ifvalid";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";

  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;

  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_valid_with_values) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.3.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "ifvalid";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";

  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  std::string logical_port =
      "PP-00:00:00:00:00:00:00:-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_valid_invalid_logical_port) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.3.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "ifvalid";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";

  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;

  std::string logical_port =
      "-00:00:00:00:00:00:00:-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_valid_with_invalid_port_values) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.3.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";

  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-1111-2222-3333-444-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_valid_with_invalid_logical_port) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.3.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";

  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =
      "TP-1111-2222-3333-444-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_invalid_data) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
      = UNC_VF_INVALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-00:00:00:00:00:00:00:00-namendfsjdfhsdbfhsd";
  vbrif_val.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_valid_with_vlanid_zero) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8282;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-openflow:2-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_in_diff_format) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8282;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-openflow:22-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_in_proper_format_invalid) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8282;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =
      "PP-openflooow:22-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_in_diff_format_invalid) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-1111-2222-3333-44444-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_valid_with_invlanid) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update1";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-00:00:00:00:00:00:00:00-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}


TEST(odcdriver,  create_cmd_port_map_valid_with_vlanid) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.tagged = UPLL_VLAN_TAGGED;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-openflow:22-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  2;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  update_cmd_port_map_valid) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string UPDATE_DELETE_VBRIF_PORTMAP = "172.16.0.5";

  inet_aton(UPDATE_DELETE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val1.valid[PFCDRV_IDX_VAL_VBRIF] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-openflow:22-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}
TEST(odcdriver,  update_cmd_no_port_map) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string UPDATE_DELETE_VBRIF_PORTMAP = "172.16.0.5";

  inet_aton(UPDATE_DELETE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::string descp =  "desc";
  vbrif_val1.valid[PFCDRV_IDX_VAL_VBRIF] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-openflow:22-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}


TEST(odcdriver,  update_cmd_port_map_delete_resp_null) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string UPDATE_DELETE_VBRIF_PORTMAP = "172.16.0.5";

  inet_aton(UPDATE_DELETE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_invalid";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val1.valid[PFCDRV_IDX_VAL_VBRIF] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID_NO_VALUE;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-00:00:00:00:00:00:00:00-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}


TEST(odcdriver,  update_cmd_port_map_invalid) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string UPDATE_DELETE_VBRIF_PORTMAP = "172.16.0.5";

  inet_aton(UPDATE_DELETE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_invalid";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-00:00:00:00:00:00:00:00-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}
TEST(odcdriver,  update_cmd_port_map_valid_vlan) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string UPDATE_DELETE_VBRIF_PORTMAP = "172.16.0.5";

  inet_aton(UPDATE_DELETE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val1.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_INVALID;
  std::string logical_port =  "PP-00:00:-0:00:00:00:00:00-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_valid_with_null_resp) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_invalid";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-00:00:00:00:00:00:00:00-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}


TEST(odcdriver,  create_cmd_port_map_valid_with_invalid_logical) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
      = UNC_VF_INVALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-00:00:00:00:00:00:00:00-";
  vbrif_val.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_valid_with_no_port_name) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
  inet_aton(CREATE_VBRIF_PORTMAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_valid_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  vbrif_val.val_vbrif.portmap.tagged = 34;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-00:00:00:00:00:00:00:00-";
  vbrif_val.val_vbrif.portmap.vlan_id =  0;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  create_cmd_port_map_invalid) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.3.3";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";

  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id)-1);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}
TEST(odcdriver,  create_cmd_port_map_invalid_portmap) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.3.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";

  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_INVALID;
  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  obj.create_cmd
            (vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  update_cmd_port_map_portmap_invalid) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_INVALID;
  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.update_cmd
            (vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}

TEST(odcdriver,  update_cmd_port_map_portmap) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}

TEST(odcdriver,  update_cmd_port_map_portmap_failure) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.3.3";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}



TEST(odcdriver,  update_cmd_port_map_update_portmap) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  vbrif_val1.valid[PFCDRV_IDX_VAL_VBRIF]  =  UNC_VF_VALID;
  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}

TEST(odcdriver,  update_cmd_empty_vtnname) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID_NO_VALUE;
  vbrif_val1.valid[PFCDRV_IDX_VAL_VBRIF] =  UNC_VF_VALID;
  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}

TEST(odcdriver,  delete_cmd_empty_vtnname) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val.val_vbrif.description)-1);
  vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID_NO_VALUE;
  vbrif_val.valid[PFCDRV_IDX_VAL_VBRIF] =  UNC_VF_VALID;
  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.delete_cmd(vbrif_key,  vbrif_val,  ctr));
  delete ctr;
}

TEST(odcdriver,  update_cmd_delete_portmap) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID_NO_VALUE;
  vbrif_val1.valid[PFCDRV_IDX_VAL_VBRIF] =  UNC_VF_VALID;
  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}


TEST(odcdriver,  update_cmd_vbrif_success_portmap_failure) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.3.3";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =  new
      unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name)-1);
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val1.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID;
  vbrif_val1.valid[PFCDRV_IDX_VAL_VBRIF] =  UNC_VF_VALID;
  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.update_cmd(vbrif_key,  vbrif_val1,vbrif_val2,  ctr));
  delete ctr;
}


TEST(odcdriver,  update_cmd_vbrif_success_portmap_delete_failure) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));


  std::string ip_add =  "172.16.3.3";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =  new
      unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val1;
  pfcdrv_val_vbr_if_t vbrif_val2;
  memset(&vbrif_val1,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_val2,  0,  sizeof(pfcdrv_val_vbr_if_t));
  memset(&vbrif_key,  0,  sizeof(key_vbr_if_t));
  std::string vtnname =  "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vbrif_key.vbr_key.vtn_key.vtn_name)-1);
  std::string vbrname =  "vbr1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrif_key.vbr_key.vbridge_name)-1);
  std::string intfname =  "if_update";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(vbrif_key.if_name));
  std::string descp =  "desc";
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.description),
          descp.c_str(),  sizeof(vbrif_val2.val_vbrif.description)-1);
  vbrif_val1.val_vbrif.valid[UPLL_IDX_PM_VBRI] =  UNC_VF_VALID_NO_VALUE;
  vbrif_val2.valid[PFCDRV_IDX_VAL_VBRIF] =  UNC_VF_VALID;

  std::string logical_port =  "PP-1111-2222-3333-4444-namendfsjdfhsdbfhsdg";
  vbrif_val1.val_vbrif.portmap.vlan_id =  10;
  strncpy(reinterpret_cast<char*>(vbrif_val1.val_vbrif.portmap.logical_port_id),
          logical_port.c_str(),
          sizeof(vbrif_val1.val_vbrif.portmap.logical_port_id));
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.update_cmd(vbrif_key,  vbrif_val1, vbrif_val2,  ctr));
  delete ctr;
}
TEST(odcdriver,  fetch_config) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;

  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string VBRIF_GET_RESP = "172.16.0.6";
  inet_aton(VBRIF_GET_RESP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::vector<unc::vtndrvcache::ConfigNode *>
      cfgnode_vector;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_vbr, cfgnode_vector));
  EXPECT_EQ(0, cfgnode_vector.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
}

TEST(odcdriver,  read_portmap_empty_if) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_if_t key_vbr_if;
  std::list<unc::odcdriver::portmap_interface> vbrif_port_detail;

  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string ifname =  "";
  strncpy(reinterpret_cast<char*>(key_vbr_if.if_name),
          ifname.c_str(),  sizeof(key_vbr_if.if_name)-1);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr_if.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr_if.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr_if.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr_if.vbr_key.vtn_key.vtn_name)-1);
  std::string VBRIF_GET_RESP = "172.16.0.6";
  inet_aton(VBRIF_GET_RESP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::vector<unc::vtndrvcache::ConfigNode *>
      cfgnode_vector;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.read_portmap(ctr, key_vbr_if, vbrif_port_detail, cfgnode_vector));
  EXPECT_EQ(0U, cfgnode_vector.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
}

TEST(odcdriver,  fetch_config_empty_vtn) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;

  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string VBRIF_GET_RESP = "172.16.0.6";
  inet_aton(VBRIF_GET_RESP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::vector<unc::vtndrvcache::ConfigNode *>
      cfgnode_vector;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_vbr, cfgnode_vector));
  EXPECT_EQ(0U, cfgnode_vector.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
}

TEST(odcdriver,  fetch_config_empty_vtn_data) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;

  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string VBRIF_GET_RESP = "172.16.0.6";
  inet_aton(VBRIF_GET_RESP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::vector<unc::vtndrvcache::ConfigNode *>
      cfgnode_vector;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_vbr, cfgnode_vector));
  EXPECT_EQ(0U, cfgnode_vector.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
}


TEST(odcdriver,  fetch_config_null_response) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;

  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::vector<unc::vtndrvcache::ConfigNode *>
      cfgnode_vector;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_vbr, cfgnode_vector));
  EXPECT_EQ(0U, cfgnode_vector.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
}

TEST(odcdriver,  fetch_config_invalid_resp) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;

  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string CREATE_201      = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::vector<unc::vtndrvcache::ConfigNode *>
      cfgnode_vector;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_vbr, cfgnode_vector));
  EXPECT_EQ(0U, cfgnode_vector.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
}

TEST(odcdriver,  fetch_config_with_portmap) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;

  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string  VBRIF_GET_RESP_PORT_MAP = "172.16.0.10";
  inet_aton(VBRIF_GET_RESP_PORT_MAP.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::vector<unc::vtndrvcache::ConfigNode *>
      cfgnode_vector;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_vbr, cfgnode_vector));
  EXPECT_EQ(0U, cfgnode_vector.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
}
TEST(odcdriver,  fetch_config_with_portmap_no_vlan) {
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrIfCommand obj(conf_file);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  key_vbr_t key_vbr;

  memset(&key_ctr , 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(key_vbr.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(key_vbr.vtn_key.vtn_name)-1);
  std::string VBRIF_GET_RESP_PORT_MAP_NO_VLAN = "172.16.0.11";

  inet_aton(VBRIF_GET_RESP_PORT_MAP_NO_VLAN.c_str(),  &val_ctr.ip_address);
  std::string user_name =  "user";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user_name.c_str(),  sizeof(val_ctr.user));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  std::vector<unc::vtndrvcache::ConfigNode *>
      cfgnode_vector;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_vbr, cfgnode_vector));
  EXPECT_EQ(0U, cfgnode_vector.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  delete ctr;
}

