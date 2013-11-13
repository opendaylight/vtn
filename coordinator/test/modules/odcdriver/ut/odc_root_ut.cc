
/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the
 * terms of the Eclipse Public License v1.0 which
 * accompanies this
 * distribution,  and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_root.hh>
#include <odc_controller.hh>

#include <gtest/gtest.h>
#include <string>


TEST(odcdriver, test_no_such_instance_odc_root) {
  unc::odcdriver::ODCROOTCommand obj;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ip_add = "172.16.1.6";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::ODCController(key_ctr,  val_ctr);;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname = "vtn1_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vtnname));
  std::string vbrname = "vbr1_v";
  strncpy(reinterpret_cast<char*>
          (vbrif_key.vbr_key.vbridge_name),  vbrname.c_str(), sizeof(vbrname));
  std::string intfname = "if_v";
  strncpy(reinterpret_cast<char*>(vbrif_key.if_name),
          intfname.c_str(),  sizeof(intfname));
  std::string descp = "desc";
  strncpy(reinterpret_cast<char*>
          (vbrif_val.val_vbrif.description),  descp.c_str(),  sizeof(descp));
  std::vector<unc::vtndrvcache::ConfigNode*> cfg_ptr;
  EXPECT_EQ(DRVAPI_RESPONSE_NO_SUCH_INSTANCE,
            obj.read_root_child(cfg_ptr,  ctr));

  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it
       = cfg_ptr.begin(); it != cfg_ptr.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
      *it = NULL;
    }
  }
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}


TEST(odcdriver, test_read_root_child) {
  unc::odcdriver::ODCROOTCommand obj;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string ip_add = "172.16.1.4";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  = new  unc::odcdriver::ODCController
      (key_ctr,  val_ctr);;
  std::vector<unc::vtndrvcache::ConfigNode*> cfg_ptr;
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj.read_root_child(cfg_ptr,  ctr));
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it
       = cfg_ptr.begin(); it != cfg_ptr.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
      *it = NULL;
    }
  }
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, test_read_all_child_vtn) {
  unc::odcdriver::ODCROOTCommand obj;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string vtnname = "vtn1";
  strncpy(reinterpret_cast<char *>
          (key_vtn.vtn_name), vtnname.c_str(),  sizeof(key_vtn.vtn_name));
  std::string desc1 = "descrption";
  strncpy(reinterpret_cast<char *>
          (val_vtn.description),  desc1.c_str(),  sizeof(val_vtn.description));
  std::vector< unc::vtndrvcache::ConfigNode*> child_list;

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil
      <key_vtn_t,  val_vtn_t,  uint32_t> (
          &key_vtn,
          &val_vtn,
          uint32_t(UNC_OP_READ));

  std::string ip_add = "172.16.1.1";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  = new  unc::odcdriver::ODCController
      (key_ctr,  val_ctr);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj.read_all_child
            (cfgptr,  child_list,  ctr));
  EXPECT_EQ(2,  child_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it
       = child_list.begin(); it != child_list.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
      *it = NULL;
    }
  }

  if (cfgptr != NULL) {
    delete cfgptr;
    cfgptr = NULL;
  }
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, test_read_all_child_vtn_failure) {
  unc::odcdriver::ODCROOTCommand obj;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  std::string vtnname = "vtn1";
  strncpy(reinterpret_cast<char *>
          (key_vtn.vtn_name), vtnname.c_str(),  sizeof(key_vtn.vtn_name));
  std::string desc1 = "descrption";
  strncpy(reinterpret_cast<char *>
          (val_vtn.description),  desc1.c_str(),  sizeof(val_vtn.description));
  std::vector< unc::vtndrvcache::ConfigNode*> child_list;

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil
      <key_vtn_t,  val_vtn_t,  uint32_t> (
          &key_vtn,
          &val_vtn,
          uint32_t(UNC_OP_READ));

  std::string ip_add = "172.16.11.1";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  = new  unc::odcdriver::ODCController
      (key_ctr,  val_ctr);
  EXPECT_EQ(DRVAPI_RESPONSE_FAILURE, obj.read_all_child
            (cfgptr,  child_list,  ctr));
  EXPECT_EQ(0,  child_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it
       = child_list.begin(); it != child_list.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
      *it = NULL;
    }
  }

  if (cfgptr != NULL) {
    delete cfgptr;
    cfgptr = NULL;
  }
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, test_read_all_child_vbr_failure) {
  unc::odcdriver::ODCROOTCommand obj;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string vbrname = "vbr1";
  std::string vtnname = "vtn1";
  strncpy(reinterpret_cast<char*>
          (key_vbr.vbridge_name), vbrname.c_str(),  sizeof(vbrname));
  strncpy(reinterpret_cast<char*>
          (key_vbr.vtn_key.vtn_name), vtnname.c_str(),  sizeof(vtnname));
  std::vector< unc::vtndrvcache::ConfigNode*> child_list;

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil
      <key_vbr_t,  val_vbr_t,  uint32_t> (
          &key_vbr,
          &val_vbr,
          uint32_t(UNC_OP_READ));

  std::string ip_add = "172.16.11.1";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  = new  unc::odcdriver::ODCController
      (key_ctr,  val_ctr);
  EXPECT_EQ(DRVAPI_RESPONSE_FAILURE, obj.read_all_child
            (cfgptr,  child_list,  ctr));
  EXPECT_EQ(0,  child_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it
       = child_list.begin(); it != child_list.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
      *it = NULL;
    }
  }

  if (cfgptr != NULL) {
    delete cfgptr;
    cfgptr = NULL;
  }
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, test_read_all_child_vbr_success) {
  unc::odcdriver::ODCROOTCommand obj;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;

  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string vbrname = "vbr1";
  std::string vtnname = "vtn1";
  strncpy(reinterpret_cast<char*>
          (key_vbr.vbridge_name), vbrname.c_str(),  sizeof(vbrname));
  strncpy(reinterpret_cast<char*>
          (key_vbr.vtn_key.vtn_name), vtnname.c_str(),  sizeof(vtnname));
  std::vector< unc::vtndrvcache::ConfigNode*> child_list;

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil
      <key_vbr_t,  val_vbr_t,  uint32_t> (
          &key_vbr,
          &val_vbr,
          uint32_t(UNC_OP_READ));

  std::string ip_add = "172.16.2.4";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  = new  unc::odcdriver::ODCController
      (key_ctr,  val_ctr);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,  obj.read_all_child
            (cfgptr,  child_list,  ctr));
  EXPECT_EQ(1,  child_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it
       = child_list.begin(); it != child_list.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
      *it = NULL;
    }
  }

  if (cfgptr != NULL) {
    delete cfgptr;
    cfgptr = NULL;
  }
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver, test_read_all_child_vbrif_failure) {
  unc::odcdriver::ODCROOTCommand obj;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  key_vbr_if_t vbrif_key;
  pfcdrv_val_vbr_if_t vbrif_val;
  memset(&key_ctr,  0,  sizeof(key_ctr_t));
  memset(&val_ctr,  0,  sizeof(val_ctr_t));

  memset(&vbrif_val,  0,  sizeof(pfcdrv_val_vbr_if_t));
  std::string vtnname = "vtn1_v";
  strncpy(reinterpret_cast<char*>
          (vbrif_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vtnname));
  std::string vbrname = "vbr1_v";
  strncpy(reinterpret_cast<char*>
          (vbrif_key.vbr_key.vbridge_name),  vbrname.c_str(), sizeof(vbrname));
  std::string intfname = "if_v";
  strncpy(reinterpret_cast<char*>
          (vbrif_key.if_name),  intfname.c_str(),  sizeof(intfname));
  std::string descp = "desc";
  strncpy(reinterpret_cast<char*>
          (vbrif_val.val_vbrif.description),  descp.c_str(),  sizeof(descp));

  std::vector< unc::vtndrvcache::ConfigNode*> child_list;

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil
      <key_vbr_if_t,  pfcdrv_val_vbr_if_t,  uint32_t> (
          &vbrif_key,
          &vbrif_val,
          uint32_t(UNC_OP_READ));

  std::string ip_add = "172.16.2.4";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  = new  unc::odcdriver::ODCController
      (key_ctr,  val_ctr);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,  obj.read_all_child
            (cfgptr,  child_list,  ctr));
  EXPECT_EQ(0,  child_list.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it
       = child_list.begin(); it != child_list.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
      *it = NULL;
    }
  }

  if (cfgptr != NULL) {
    delete cfgptr;
    cfgptr = NULL;
  }
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

