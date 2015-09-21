/*
 * Copyright (c) 2014-2015 NEC Corporation
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
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.fetch_config(ctr, cache_empty));
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
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.fetch_config(ctr, cache_empty));
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
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj.fetch_config(ctr, cache_empty));
  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == true;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    if (flag == 1) {
      EXPECT_EQ(-1, sw_id.compare("openflow:2"));
      flag++;
    } else if (flag == 2) {
      EXPECT_EQ(-1, sw_id.compare("openflow:3"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 3);
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_switch, DISABLED_test_switch_data_update) {
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
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj.fetch_config(ctr, cache_empty));
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
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    if (flag == 1) {
      EXPECT_EQ(-1, sw_id.compare("openflow:2"));
      flag++;
    } else if (flag == 2) {
      // EXPECT_EQ(-1, sw_id.compare("openflow:3"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 3);

  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));
  std::string SWITCH_RESP_UPDATE  = "172.16.0.21";
  inet_aton(SWITCH_RESP_UPDATE.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  flag = 1;
  EXPECT_EQ(UNC_RC_SUCCESS, obj.fetch_config(ctr, cache_empty));
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    val_switch_st_t *val_switch = cache_util_ptr->get_val_structure();

    std::string sw_id =
        reinterpret_cast<char*> (key_switch->switch_id);
    std::string desc =
        reinterpret_cast<char*> (val_switch->switch_val.description);
    if (flag == 1) {
      EXPECT_EQ(-1, sw_id.compare("openflow:2"));
      EXPECT_EQ(-1, desc.compare("ONE"));
      flag++;
    } else if (flag == 2) {
      // EXPECT_EQ(0, sw_id.compare("openflow:4"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 3);

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_switch, DISABLED_test_switch_data_delete) {
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
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::restjson::ConfFileValues_t conf_file;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj.fetch_config(ctr, cache_empty));
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
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    if (flag == 1) {
      EXPECT_EQ(-1, sw_id.compare("openflow:2"));
      flag++;
    } else if (flag == 2) {
      EXPECT_EQ(-1, sw_id.compare("openflow:4"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 3);
  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));
  inet_aton(SWITCH_RESP_DELETE.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  flag = 1;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.fetch_config(ctr, cache_empty));
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t,
             uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t,
                        uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    val_switch_st_t *val_switch = cache_util_ptr->get_val_structure();

    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    std::string desc =  reinterpret_cast<char*>
        (val_switch->switch_val.description);
    if (flag == 1) {
      EXPECT_EQ(-1, sw_id.compare("openflow:2"));
      EXPECT_EQ(-1, desc.compare("ONE"));
      flag++;
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 3);
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}


TEST(odcdriver_switch, DISABLED_test_switch_data_update_same) {
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
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj.fetch_config(ctr, cache_empty));
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
        <key_switch_t, val_switch_st_t, val_switch_st_t,
                            uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t,
                           uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    if (flag == 1) {
      EXPECT_EQ(-1, sw_id.compare("openflow:2"));
      flag++;
    } else if (flag == 2) {
      EXPECT_EQ(-1, sw_id.compare("openflow:4"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 3);


  flag = 1;
  EXPECT_EQ(UNC_RC_SUCCESS, obj.fetch_config(ctr, cache_empty));
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    EXPECT_EQ(key_type, UNC_KT_SWITCH);
    unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t,
                             uint32_t> * cache_util_ptr =
        static_cast <unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t,
                        uint32_t> * > (cfgnode_cache);
    key_switch_t *key_switch = cache_util_ptr->get_key_structure();
    val_switch_st_t *val_switch = cache_util_ptr->get_val_structure();

    std::string sw_id =  reinterpret_cast<char*> (key_switch->switch_id);
    std::string desc =  reinterpret_cast<char*>
        (val_switch->switch_val.description);
    if (flag == 1) {
      EXPECT_EQ(-1, sw_id.compare("openflow:2"));
      flag++;
    } else if (flag == 2) {
      EXPECT_EQ(-1, sw_id.compare("openflow:4"));
    }
  }
  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 3);

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_switch, test_switch_resp_nodeprop_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string SWITCH_NODE_PROP_WRONG = "172.16.0.72";

  inet_aton(SWITCH_NODE_PROP_WRONG.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();
  unc::restjson::ConfFileValues_t conf_file;
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.fetch_config(ctr, cache_empty));

  delete ctr->physical_port_cache;
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver_switch, test_switch_resp_node_prop_node_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string SWITCH_NODE_PROP_NODE_WRONG = "172.16.0.73";
  inet_aton(SWITCH_NODE_PROP_NODE_WRONG.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();
  unc::restjson::ConfFileValues_t conf_file;
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver_switch, test_switch_resp_node_prop_id_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string SWITCH_NODE_PROP_ID_WRONG = "172.16.0.74";

  inet_aton(SWITCH_NODE_PROP_ID_WRONG.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();
  unc::restjson::ConfFileValues_t conf_file;
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver_switch, test_switch_node_prop_id_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string SWITCH_NODE_PROPERTIES_WRONG = "172.16.0.75";

  inet_aton(SWITCH_NODE_PROPERTIES_WRONG.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();
  unc::restjson::ConfFileValues_t conf_file;
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcSwitch obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr = NULL;
}

