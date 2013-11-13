/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_mod.hh>
#include <gtest/gtest.h>
#include <string>


TEST(odcdriver, test_get_controller_type) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  EXPECT_EQ(UNC_CT_ODC, obj.get_controller_type());
}

TEST(odcdriver, test_is_2ph_commit_support_needed) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  EXPECT_EQ(PFC_FALSE,  obj.is_2ph_commit_support_needed());
}

TEST(odcdriver, test_is_audit_collection_needed) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  EXPECT_EQ(PFC_FALSE,  obj.is_audit_collection_needed());
}

TEST(odcdriver, test_add_controller) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ctr_name = "ctr_abc";
  strncpy(reinterpret_cast<char*> (key_ctr.controller_name), ctr_name.c_str(),
          sizeof(key_ctr.controller_name));
  unc::driver::controller* ctr = NULL;
  ctr =  obj.add_controller(key_ctr,  val_ctr);
  EXPECT_EQ(ctr->get_controller_id(),  ctr_name);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, test_add_controller_empty_name) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ctr_name = "";
  strncpy(reinterpret_cast<char*> (key_ctr.controller_name), ctr_name.c_str(),
          sizeof(key_ctr.controller_name));
  unc::driver::controller* ctr;
  ctr =  obj.add_controller(key_ctr,  val_ctr);
  int val =0;
  if (ctr == NULL) {
    val = 1;
  }
  EXPECT_EQ(val, 1);
}

TEST(odcdriver, test_update_controller_empty_name) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ctr_name_new = "";
  strncpy(reinterpret_cast<char*> (key_ctr.controller_name),
          ctr_name_new.c_str(),
          sizeof(key_ctr.controller_name));
  key_ctr_t key_ctr_old;
  val_ctr_t val_ctr_old;
  memset(&key_ctr_old,  0,  sizeof(key_ctr_t));
  memset(&val_ctr_old,  0,  sizeof(val_ctr_t));

  std::string ctr_name = "old_ctr";
  strncpy(reinterpret_cast<char*> (key_ctr_old.controller_name),
          ctr_name.c_str(),
          sizeof(key_ctr_old.controller_name));
  unc::driver::controller* ctr = NULL;
  ctr =  obj.add_controller(key_ctr_old,  val_ctr_old);
  unc::driver::controller* updated_ctr = NULL;
  pfc_bool_t bool_val =  obj.update_controller(key_ctr,  val_ctr,  ctr);
  EXPECT_EQ(bool_val, 1);
  EXPECT_EQ(PFC_FALSE,  obj.delete_controller(updated_ctr));
  if (updated_ctr != NULL) {
    delete updated_ctr;
    updated_ctr = NULL;
  }
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, test_delete_controller) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ctr_name = "ctr1";
  strncpy(reinterpret_cast<char*> (key_ctr.controller_name), ctr_name.c_str(),
          sizeof(key_ctr.controller_name));
  unc::driver::controller* ctr;
  ctr =  obj.add_controller(key_ctr,  val_ctr);
  int val =0;
  if (ctr == NULL) {
    val = 1;
  }
  EXPECT_EQ(val, 0);
  EXPECT_EQ(PFC_TRUE,  obj.delete_controller(ctr));
}

TEST(odcdriver, test_delete_controller_NULL) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  key_ctr_t key_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));

  std::string ctr_name = "";
  strncpy(reinterpret_cast<char*> (key_ctr.controller_name), ctr_name.c_str(),
          sizeof(key_ctr.controller_name));
  unc::driver::controller* ctr = NULL;
  EXPECT_EQ(PFC_FALSE,  obj.delete_controller(ctr));
}

TEST(odcdriver, test_update_controller) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ctr_name_new = "new_ctr";
  strncpy(reinterpret_cast<char*>
          (key_ctr.controller_name), ctr_name_new.c_str(),
          sizeof(key_ctr.controller_name));
  key_ctr_t key_ctr_old;
  val_ctr_t val_ctr_old;
  memset(&key_ctr_old,  0,  sizeof(key_ctr_t));
  memset(&val_ctr_old,  0,  sizeof(val_ctr_t));

  std::string ctr_name = "old_ctr";
  strncpy(reinterpret_cast<char*>
          (key_ctr_old.controller_name), ctr_name.c_str(),
          sizeof(key_ctr_old.controller_name));
  unc::driver::controller* ctr;
  ctr =  obj.add_controller(key_ctr_old,  val_ctr_old);
  pfc_bool_t updated_ctr =  obj.update_controller(key_ctr,  val_ctr,  ctr);
  EXPECT_EQ(updated_ctr,  1);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, test_create_driver_command_vtn_success) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  unc::odcdriver::OdcVtnCommand*  drv = reinterpret_cast
      <unc::odcdriver::OdcVtnCommand*>(obj.create_driver_command(UNC_KT_VTN));
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string desc1 = "description";
  strncpy(reinterpret_cast<char*>
          (val_vtn.description),  desc1.c_str(),  sizeof(val_vtn.description));
  std::string ip_add = "172.16.0.1";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);;
  std::string vtnname = "vtn1";
  strncpy(reinterpret_cast<char*>(key_vtn.vtn_name) ,
          vtnname.c_str(),  sizeof(vtnname));

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
  if (drv!= NULL) {
    delete drv;
    drv = NULL;
  }
}

TEST(odcdriver, test_create_driver_command_vtn_failure) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  unc::odcdriver::OdcVtnCommand*  drv =
      reinterpret_cast<unc::odcdriver::OdcVtnCommand*>
      (obj.create_driver_command(UNC_KT_VTN));
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string desc1 = "description";
  strncpy(reinterpret_cast<char*>
          (val_vtn.description),  desc1.c_str(),  sizeof(val_vtn.description));
  std::string ip_add = "172.16.11.1";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);;
  std::string vtnname = "vtn1";
  strncpy(reinterpret_cast<char*>
          (key_vtn.vtn_name) , vtnname.c_str(),  sizeof(vtnname));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }

  if (drv != NULL) {
    delete drv;
    drv = NULL;
  }
}


TEST(odcdriver, test_create_driver_command_vbr_success) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  unc::odcdriver::OdcVbrCommand*  drv =
      reinterpret_cast<unc::odcdriver::OdcVbrCommand*>
      (obj.create_driver_command(UNC_KT_VBRIDGE));
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  std::string ip_add = "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  std::string vbrname = "vbr1";
  strncpy(reinterpret_cast<char*>(key_vbr.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname));
  std::string vtnname = "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vtnname));
  std::string description = "descrip";
  strncpy(reinterpret_cast<char*>
          (val_vbr.vbr_description), description.c_str(),  sizeof(description));

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
  if (drv != NULL) {
    delete drv;
    drv = NULL;
  }
}

TEST(odcdriver, test_create_driver_command_vbr_failure) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  unc::odcdriver::OdcVbrCommand*  drv = reinterpret_cast
      <unc::odcdriver::OdcVbrCommand*>
      (obj.create_driver_command(UNC_KT_VBRIDGE));
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  std::string ip_add = "172.16.11.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  = new
      unc::odcdriver::OdcController(key_ctr,  val_ctr);
  std::string vbrname = "vbr1";
  strncpy(reinterpret_cast<char*>
          (key_vbr.vbridge_name), vbrname.c_str(),  sizeof(vbrname));
  std::string vtnname = "vtn1";
  strncpy(reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vtnname));
  std::string description = "descrip";
  strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
          description.c_str(),  sizeof(description));

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
  if (drv != NULL) {
    delete drv;
    drv = NULL;
  }
}
TEST(odcdriver, test_create_driver_command_vbrif_success) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  unc::odcdriver::OdcVbrIfCommand*  drv =
      reinterpret_cast<unc::odcdriver::OdcVbrIfCommand*>
      (obj.create_driver_command(UNC_KT_VBR_IF));
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ip_add = "172.16.3.1";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname = "vtn1_v";
  strncpy(reinterpret_cast<char*>
          (vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vtnname));
  std::string vbrname = "vbr1_v";
  strncpy(reinterpret_cast<char*>
          (vbrif_key.vbr_key.vbridge_name),
          vbrname.c_str(), sizeof(vbrname));
  std::string intfname = "if_v";
  strncpy(reinterpret_cast<char*>
          (vbrif_key.if_name),
          intfname.c_str(),  sizeof(intfname));
  std::string descp = "desc";
  strncpy(reinterpret_cast<char*>
          (vbrif_val.val_vbrif.description),
          descp.c_str(),  sizeof(descp));

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
  if (drv != NULL) {
    delete drv;
    drv = NULL;
  }
}

TEST(odcdriver, test_create_driver_command_vbrif_failure) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  unc::odcdriver::OdcVbrIfCommand*  drv =
      reinterpret_cast<unc::odcdriver::OdcVbrIfCommand*>
      (obj.create_driver_command(UNC_KT_VBR_IF));
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ip_add = "172.16.11.1";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname = "vtn1_v";
  strncpy(reinterpret_cast<char*>
          (vbrif_key.vbr_key.vtn_key.vtn_name), vtnname.c_str(),
          sizeof(vtnname));
  std::string vbrname = "vbr1_v";
  strncpy(reinterpret_cast<char*>
          (vbrif_key.vbr_key.vbridge_name),  vbrname.c_str(), sizeof(vbrname));
  std::string intfname = "if_v";
  strncpy(reinterpret_cast<char*>
          (vbrif_key.if_name),  intfname.c_str(),  sizeof(intfname));
  std::string descp = "desc";
  strncpy(reinterpret_cast<char*>
          (vbrif_val.val_vbrif.description),  descp.c_str(),  sizeof(descp));

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
  if (drv != NULL) {
    delete drv;
    drv = NULL;
  }
}


TEST(odcdriver, test_getpinginterval) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  EXPECT_EQ(0,  obj.get_ping_interval());
}


TEST(odcdriver, test_get_ping_fail_retry_count) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  EXPECT_EQ(5,  obj.get_ping_fail_retry_count());
}

TEST(odcdriver, test_is_ping_needed) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  EXPECT_EQ(PFC_TRUE,  obj.is_ping_needed());
}

TEST(odcdriver, test_ping_controller_ip_zero) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ip_add = "0";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string ctr_name = "ctr_abc";
  strncpy(reinterpret_cast<char*>
          (key_ctr.controller_name), ctr_name.c_str(),
          sizeof(key_ctr.controller_name));
  unc::driver::controller* ctr = NULL;
  ctr =  obj.add_controller(key_ctr,  val_ctr);
  EXPECT_EQ(PFC_FALSE,  obj.ping_controller(ctr));
  if (ctr!= NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, test_ping_controller__user_name) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&val_ctr,  0,  sizeof(val_ctr_t));
  std::string ip_add = "172.16.0.0";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string ctr_name = "ctr_abc";
  std::string user = "user";
  strncpy(reinterpret_cast<char*>
          (val_ctr.user),  user.c_str(),  sizeof(val_ctr.user) -1);
  std::string pass = "password";
  strncpy(reinterpret_cast<char*>
          (val_ctr.password),  pass.c_str(),  sizeof(val_ctr.password) -1);
  strncpy(reinterpret_cast<char*>
          (key_ctr.controller_name), ctr_name.c_str(),
          sizeof(key_ctr.controller_name));
  unc::driver::controller* ctr;
  ctr =  obj.add_controller(key_ctr,  val_ctr);
  EXPECT_EQ(PFC_FALSE,  obj.ping_controller(ctr));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}


TEST(odcdriver, test_ping_controller_SUCCESS) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ip_add = "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string ctr_name = "ctr_abc";
  std::string user = "user";
  strncpy(reinterpret_cast<char*>
          (val_ctr.user),  user.c_str(),  sizeof(val_ctr.user) -1);
  std::string pass = "password";
  strncpy(reinterpret_cast<char*>
          (val_ctr.password),  pass.c_str(),  sizeof(val_ctr.password) -1);
  strncpy(reinterpret_cast<char*>
          (key_ctr.controller_name), ctr_name.c_str(),
          sizeof(key_ctr.controller_name));
  unc::driver::controller* ctr = NULL;
  ctr =  obj.add_controller(key_ctr,  val_ctr);
  EXPECT_EQ(PFC_TRUE,  obj.ping_controller(ctr));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, test_HandleVote) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  unc::driver::controller* ctr = NULL;
  EXPECT_EQ(unc::tclib::TC_FAILURE,  obj.HandleVote(ctr));
}

TEST(odcdriver, test_HandleCommit) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  unc::driver::controller* ctr = NULL;
  EXPECT_EQ(unc::tclib::TC_FAILURE,  obj.HandleCommit(ctr));
}


TEST(odcdriver, test_HandleAbort) {
  const pfc_modattr_t* obj_module = NULL;
  unc::odcdriver::ODCModule obj(obj_module);
  unc::driver::controller* ctr = NULL;
  EXPECT_EQ(unc::tclib::TC_FAILURE,  obj.HandleAbort(ctr));
}


