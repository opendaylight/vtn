/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_switch.hh>
#include <odc_controller.hh>
#include <odc_driver_common_defs.hh>
#include <vtn_drv_module.hh>
#include <gtest/gtest.h>
#include <string>

void read_conf_file(unc::restjson::ConfFileValues_t &conf_file) {
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
}

TEST(odcdriver_switch, test_switch_null_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  std::string  NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  unc::restjson::ConfFileValues_t conf_file;

  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_FAILURE, obj.fetch_config(ctr, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_switch, test_switch_invalid_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  std::string INVALID_RESPONSE = "172.0.0.0";

  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  unc::restjson::ConfFileValues_t conf_file;

  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_FAILURE, obj.fetch_config(ctr, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_switch, test_switch_data) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  std::string SWITCH_RESP  = "172.16.0.20";
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();
  unc::restjson::ConfFileValues_t conf_file;
  read_conf_file(conf_file);
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj.fetch_config(ctr, cache_empty));
  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    if (flag == 1) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:02"));
      flag++;
    } else if (flag == 2) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:03"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 2);
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_switch, test_switch_data_update) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  std::string SWITCH_RESP  = "172.16.0.20";
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();
  unc::restjson::ConfFileValues_t conf_file;
  read_conf_file(conf_file);
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj.fetch_config(ctr, cache_empty));
  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    if (flag == 1) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:02"));
      flag++;
    } else if (flag == 2) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:03"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 2);

  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));
  std::string SWITCH_RESP_UPDATE  = "172.16.0.21";
  inet_aton(SWITCH_RESP_UPDATE.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  flag = 1;
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj.fetch_config(ctr, cache_empty));
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    val_switch_st_t *val_switch = cache_util_ptr->get_val_structure();

    std::string sw_id =
        reinterpret_cast<char*> (key_switch->switch_id);
    std::string desc =
        reinterpret_cast<char*> (val_switch->switch_val.description);
    if (flag == 1) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:02"));
      EXPECT_EQ(0, desc.compare("ONE"));
      flag++;
    } else if (flag == 2) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:04"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 2);

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_switch, test_switch_data_delete) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  std::string SWITCH_RESP  = "172.16.0.20";
  std::string SWITCH_RESP_DELETE = "172.16.0.22";

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();
  unc::restjson::ConfFileValues_t conf_file;
  read_conf_file(conf_file);
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj.fetch_config(ctr, cache_empty));
  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    if (flag == 1) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:02"));
      flag++;
    } else if (flag == 2) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:03"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 2);
  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));
  inet_aton(SWITCH_RESP_DELETE.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  flag = 1;
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj.fetch_config(ctr, cache_empty));
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    val_switch_st_t *val_switch = cache_util_ptr->get_val_structure();

    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    std::string desc =  reinterpret_cast<char*>
        (val_switch->switch_val.description);
    if (flag == 1) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:02"));
      EXPECT_EQ(0, desc.compare("ONE"));
      flag++;
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 1);
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}


TEST(odcdriver_switch, test_switch_data_update_same) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  std::string SWITCH_RESP  = "172.16.0.20";
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();
  unc::restjson::ConfFileValues_t conf_file;
  read_conf_file(conf_file);
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj.fetch_config(ctr, cache_empty));
  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    if (flag == 1) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:02"));
      flag++;
    } else if (flag == 2) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:03"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 2);


  flag = 1;
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj.fetch_config(ctr, cache_empty));
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    val_switch_st_t *val_switch = cache_util_ptr->get_val_structure();

    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    std::string desc =  reinterpret_cast<char*>
        (val_switch->switch_val.description);
    if (flag == 1) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:02"));
      flag++;
    } else if (flag == 2) {
      EXPECT_EQ(0, sw_id.compare("00:00:00:00:00:00:00:03"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 2);

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

