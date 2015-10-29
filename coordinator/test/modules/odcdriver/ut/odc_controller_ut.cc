/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_controller.hh>
#include <gtest/gtest.h>
#include <string>

TEST(odcdriver, controller) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add = "172.16.1.6";
  inet_aton(ip_add.c_str(), &val_ctr.ip_address);
  std::string max_controller_name_31 = "1234567891234567891234567891234";
  strncpy(reinterpret_cast<char*>(key_ctr.controller_name),
          max_controller_name_31.c_str(), sizeof(key_ctr.controller_name) -1);

  std::string version = "version12223423428299990";
  strncpy(reinterpret_cast<char*>(val_ctr.version),
          version.c_str(), sizeof(val_ctr.version) -1);

  std::string description = "description12223423428299990";
  strncpy(reinterpret_cast<char*>
          (val_ctr.description), description.c_str(),
          sizeof(val_ctr.description) -1);

  std::string max_user_length_31 = "user111111111111111111122222222";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          max_user_length_31.c_str(), sizeof(val_ctr.user) -1);

  std::string pass =
      "password12223423428299990746537568345364"
      "56873453845636457ksjdghsdbfhhsdhfgghsdfsgdfhshdgfjhsdg";
  strncpy(reinterpret_cast<char*>
          (val_ctr.password), pass.c_str(), sizeof(val_ctr.password) -1);

  val_ctr.enable_audit = PFC_TRUE;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr, val_ctr, conf_values );;
  std::vector<unc::vtndrvcache::ConfigNode*> cfg_ptr;

  EXPECT_EQ(UNC_CT_ODC, ctr->get_controller_type());
  EXPECT_EQ("1234567891234567891234567891234" , ctr->get_controller_id());
  EXPECT_EQ(PFC_TRUE, ctr->get_audit_status());
  EXPECT_EQ(ip_add, ctr->get_host_address());
  EXPECT_EQ(PFC_FALSE, ctr->reset_connect());
  EXPECT_EQ(max_user_length_31, ctr->get_user_name());
  EXPECT_EQ(pass, ctr->get_pass_word());

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, controller_all_empty_values) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add = "0";
  inet_aton(ip_add.c_str(), &val_ctr.ip_address);
  std::string controller_name = "";
  strncpy(reinterpret_cast<char*>(key_ctr.controller_name),
          controller_name.c_str(), sizeof(key_ctr.controller_name) -1);

  std::string version = "";
  strncpy(reinterpret_cast<char*>(val_ctr.version),
          version.c_str(), sizeof(val_ctr.version) -1);

  std::string description = "";
  strncpy(reinterpret_cast<char*>(val_ctr.description),
          description.c_str(), sizeof(val_ctr.description) -1);

  std::string user = "";
  strncpy(reinterpret_cast<char*>(val_ctr.user),
          user.c_str(), sizeof(val_ctr.user) -1);

  std::string pass = "";
  strncpy(reinterpret_cast<char*>(val_ctr.password),
          pass.c_str(), sizeof(val_ctr.password) -1);

  val_ctr.enable_audit = PFC_TRUE;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr, val_ctr, conf_values);

  EXPECT_EQ(UNC_CT_ODC, ctr->get_controller_type());
  EXPECT_EQ(controller_name , ctr->get_controller_id());
  EXPECT_EQ(PFC_TRUE, ctr->get_audit_status());
  EXPECT_EQ("0.0.0.0", ctr->get_host_address());
  EXPECT_EQ(PFC_FALSE, ctr->reset_connect());
  EXPECT_EQ(user, ctr->get_user_name());
  EXPECT_EQ(pass, ctr->get_pass_word());
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

