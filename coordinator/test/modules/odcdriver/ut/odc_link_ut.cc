/*
 * Copyright (c) 2014-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_link.hh>
#include <odc_controller.hh>
#include <odc_driver_common_defs.hh>
#include <vtn_cache_mod.hh>
#include <vtn_drv_module.hh>
#include <gtest/gtest.h>
#include <string>


TEST(odcdriver_link, test_link_null_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  std::string  NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;
  pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcLink obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj.fetch_config(ctr, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}


TEST(odcdriver_link, test_link_invalid_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  std::string INVALID_RESPONSE = "172.0.0.0";
  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);

  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;

  const pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcLink obj(conf_file);

  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_data_add_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  std::string LINK_RESP_ONE = "172.16.0.31";
  inet_aton(LINK_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_data_add) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  std::string LINK_RESP_ONE_LINK = "172.16.0.32";
  inet_aton(LINK_RESP_ONE_LINK.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  int flag = 1;
  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * > (cfgnode_cache);
      key_link_t *key_link = cache_util_ptr->get_key_structure();
      std::string switch_id1 = reinterpret_cast<char*> (key_link->switch_id1);
      std::string switch_id2 = reinterpret_cast<char*> (key_link->switch_id2);
      std::string port_id1 = reinterpret_cast<char*> (key_link->port_id1);
      std::string port_id2 = reinterpret_cast<char*> (key_link->port_id2);

      if (flag == 1) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:03"));
        EXPECT_EQ(0, port_id1.compare("s1-eth1"));
        EXPECT_EQ(0, port_id2.compare("s3-eth3"));
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:03"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, port_id2.compare("s1-eth1"));
        EXPECT_EQ(0, port_id1.compare("s3-eth3"));
        flag++;
      } else {
        EXPECT_EQ(0, 1);
      }
    }
  }
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_data_add_wrong_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  std::string LINK_RESP_ONE_WRONG = "172.16.0.33";
  inet_aton(LINK_RESP_ONE_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_data_add_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_RESP_ADD = "172.16.0.34";
  inet_aton(LINK_RESP_ADD.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());

  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  int flag = 1;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * > (cfgnode_cache);
      key_link_t *key_link = cache_util_ptr->get_key_structure();
      std::string switch_id1 = reinterpret_cast<char*> (key_link->switch_id1);
      std::string switch_id2 = reinterpret_cast<char*> (key_link->switch_id2);
      std::string port_id1 = reinterpret_cast<char*> (key_link->port_id1);
      std::string port_id2 = reinterpret_cast<char*> (key_link->port_id2);

      if (flag == 2) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:03"));
        EXPECT_EQ(0, port_id1.compare("s1-eth2"));
        EXPECT_EQ(0, port_id2.compare("s3-eth3"));
        flag++;
      } else if (flag == 4) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:03"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, port_id2.compare("s1-eth2"));
        EXPECT_EQ(0, port_id1.compare("s3-eth3"));
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:02"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, port_id2.compare("s1-eth1"));
        EXPECT_EQ(0, port_id1.compare("s2-eth3"));
        flag++;
      } else if (flag == 1) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:02"));
        EXPECT_EQ(0, port_id2.compare("s2-eth3"));
        EXPECT_EQ(0, port_id1.compare("s1-eth1"));
        flag++;
      } else {
        EXPECT_EQ(0, 1);
      }
    }
  }
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  ctr->physical_port_cache = NULL;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_data_add_resp_dynamically) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_RESP_ADD = "172.16.0.34";
  inet_aton(LINK_RESP_ADD.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  std::string LINK_RESP_ADD_DYNAMICALLY = "172.16.0.35";
  inet_aton(LINK_RESP_ADD_DYNAMICALLY.c_str(),  &val_ctr.ip_address);
  cache_empty = PFC_FALSE;
  ctr->update_ctr(key_ctr, val_ctr);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  int flag = 1;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * > (cfgnode_cache);
      key_link_t *key_link = cache_util_ptr->get_key_structure();
      std::string switch_id1 = reinterpret_cast<char*> (key_link->switch_id1);
      std::string switch_id2 = reinterpret_cast<char*> (key_link->switch_id2);
      std::string port_id1 = reinterpret_cast<char*> (key_link->port_id1);
      std::string port_id2 = reinterpret_cast<char*> (key_link->port_id2);

      if (flag == 1) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, port_id1.compare("s4-eth4"));
        EXPECT_EQ(0, port_id2.compare("s5-eth5"));
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, port_id2.compare("s4-eth4"));
        EXPECT_EQ(0, port_id1.compare("s5-eth5"));
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, 1);
      }
    }
  }
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}


TEST(odcdriver_link,
     test_link_data_add_resp_dynamically_small_topo_large_topo) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_RESP_ADD_DYNAMICALLY = "172.16.0.35";
  std::string LINK_RESP_ADD = "172.16.0.34";
  inet_aton(LINK_RESP_ADD_DYNAMICALLY.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  inet_aton(LINK_RESP_ADD.c_str(),  &val_ctr.ip_address);
  cache_empty = PFC_FALSE;
  ctr->update_ctr(key_ctr, val_ctr);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  int flag = 1;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
        <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
        <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * > (cfgnode_cache);
      key_link_t *key_link = cache_util_ptr->get_key_structure();
      std::string switch_id1 = reinterpret_cast<char*> (key_link->switch_id1);
      std::string switch_id2 = reinterpret_cast<char*> (key_link->switch_id2);
      std::string port_id1 = reinterpret_cast<char*> (key_link->port_id1);
      std::string port_id2 = reinterpret_cast<char*> (key_link->port_id2);
      if (flag == 2) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:03"));
        EXPECT_EQ(0, port_id1.compare("s1-eth2"));
        EXPECT_EQ(0, port_id2.compare("s3-eth3"));
        flag++;
      } else if (flag == 4) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:03"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, port_id2.compare("s1-eth2"));
        EXPECT_EQ(0, port_id1.compare("s3-eth3"));
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:02"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, port_id2.compare("s1-eth1"));
        EXPECT_EQ(0, port_id1.compare("s2-eth3"));
        flag++;
      } else if (flag == 1) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:02"));
        EXPECT_EQ(0, port_id2.compare("s2-eth3"));
        EXPECT_EQ(0, port_id1.compare("s1-eth1"));
        flag++;
      } else {
        EXPECT_EQ(0, 1);
      }
    }
  }
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link,
     test_link_data_update_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_RESP_UPDATE = "172.16.0.36";
  std::string LINK_RESP_ADD_DYNAMICALLY = "172.16.0.35";
  inet_aton(LINK_RESP_ADD_DYNAMICALLY.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();
  ctr->set_connection_status(0);

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  inet_aton(LINK_RESP_UPDATE.c_str(),  &val_ctr.ip_address);
  cache_empty = PFC_FALSE;
  ctr->update_ctr(key_ctr, val_ctr);
  ctr->set_connection_status(0);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;

  int flag = 1;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * > (cfgnode_cache);
      key_link_t *key_link = cache_util_ptr->get_key_structure();
      val_link_st_t *val_link = cache_util_ptr->get_val_structure();
      std::string switch_id1 = reinterpret_cast<char*> (key_link->switch_id1);
      std::string switch_id2 = reinterpret_cast<char*> (key_link->switch_id2);
      std::string port_id1 = reinterpret_cast<char*> (key_link->port_id1);
      std::string port_id2 = reinterpret_cast<char*> (key_link->port_id2);
      if (flag == 1) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, port_id1.compare("s4-eth4"));
        EXPECT_EQ(0, port_id2.compare("s5-eth5"));
        EXPECT_EQ(1, val_link->oper_status);
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, port_id2.compare("s4-eth4"));
        EXPECT_EQ(0, port_id1.compare("s5-eth5"));
        EXPECT_EQ(0, val_link->oper_status);
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, 1);
      }
    }
  }
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_data_update_dynamically) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_RESP_ADD = "172.16.0.34";
  inet_aton(LINK_RESP_ADD.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  std::string LINK_RESP_ONE_LINK = "172.16.0.32";
  inet_aton(LINK_RESP_ONE_LINK.c_str(),  &val_ctr.ip_address);
  cache_empty = PFC_FALSE;

  ctr->update_ctr(key_ctr, val_ctr);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * > (cfgnode_cache);
      key_link_t *key_link = cache_util_ptr->get_key_structure();
      val_link_st_t *val_link = cache_util_ptr->get_val_structure();
      std::string switch_id1 = reinterpret_cast<char*> (key_link->switch_id1);
      std::string switch_id2 = reinterpret_cast<char*> (key_link->switch_id2);
      std::string port_id1 = reinterpret_cast<char*> (key_link->port_id1);
      std::string port_id2 = reinterpret_cast<char*> (key_link->port_id2);

      if (flag == 1) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:03"));
        EXPECT_EQ(0, port_id1.compare("s1-eth1"));
        EXPECT_EQ(0, port_id2.compare("s3-eth3"));
        EXPECT_EQ(1, val_link->oper_status);
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:03"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:01"));
        EXPECT_EQ(0, port_id2.compare("s1-eth1"));
        EXPECT_EQ(0, port_id1.compare("s3-eth3"));
        EXPECT_EQ(0, val_link->oper_status);
        flag++;
      } else {
        EXPECT_EQ(0, 1);
      }
    }
  }
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_data_delete) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  std::string LINK_RESP_ONE_LINK = "172.16.0.32";
  inet_aton(LINK_RESP_ONE_LINK.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  std::string LINK_RESP_DELETE = "172.16.0.37";
  inet_aton(LINK_RESP_DELETE.c_str(),  &val_ctr.ip_address);
  cache_empty = PFC_TRUE;
  ctr->update_ctr(key_ctr, val_ctr);
  EXPECT_EQ(UNC_RC_SUCCESS, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_no_data) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  std::string LINK_RESP_DELETE = "172.16.0.37";
  inet_aton(LINK_RESP_DELETE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_edge_resp_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  std::string LINK_EDGE_WRONG = "172.16.0.38";
  inet_aton(LINK_EDGE_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_tail_node_resp_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  std::string LINK_TAIL_NODE_WRONG = "172.16.0.39";
  inet_aton(LINK_TAIL_NODE_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_node_resp_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  std::string LINK_NODE_WRONG = "172.16.0.40";
  inet_aton(LINK_NODE_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_node_id_resp_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_NODE_ID_WRONG = "172.16.0.41";
  inet_aton(LINK_NODE_ID_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_tail_id_resp_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_TAIL_ID_WRONG = "172.16.0.42";

  inet_aton(LINK_TAIL_ID_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}


TEST(odcdriver_link, test_link_head_node_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_HEAD_NODE_CONN_WRONG = "172.16.0.43";

  inet_aton(LINK_HEAD_NODE_CONN_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}
TEST(odcdriver_link, test_link_head_id_resp_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_HEAD_ID_WRONG = "172.16.0.44";

  inet_aton(LINK_HEAD_ID_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_head_resp_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_HEAD_NODE_WRONG = "172.16.0.45";

  inet_aton(LINK_HEAD_NODE_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr,conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_head_node_resp_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_HEAD_NODE_ID_WRONG = "172.16.0.47";

  inet_aton(LINK_HEAD_NODE_ID_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_head_node_prop_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_HEAD_NODE_PROP_WRONG = "172.16.0.48";

  inet_aton(LINK_HEAD_NODE_PROP_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_head_node_prop_namewrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_HEAD_NODE_PROP_NAME_WRONG = "172.16.0.49";

  inet_aton(LINK_HEAD_NODE_PROP_NAME_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_head_node_prop_name_value_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_HEAD_NODE_PROP_NAME_VALUE_WRONG = "172.16.0.50";

  inet_aton(LINK_HEAD_NODE_PROP_NAME_VALUE_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}


TEST(odcdriver_link, test_link_head_node_prop_state_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_HEAD_NODE_PROP_STATE_WRONG = "172.16.0.51";

  inet_aton(LINK_HEAD_NODE_PROP_STATE_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}


TEST(odcdriver_link, test_link_head_node_prop_state_value_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_HEAD_NODE_PROP_STATE_VALUE_WRONG = "172.16.0.52";

  inet_aton(LINK_HEAD_NODE_PROP_STATE_VALUE_WRONG.c_str(),
            &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_head_node_prop_config_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_HEAD_NODE_PROP_CONFIG_WRONG = "172.16.0.53";

  inet_aton(LINK_HEAD_NODE_PROP_CONFIG_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_head_node_prop_config_value_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  std::string LINK_HEAD_NODE_PROP_CONFIG_VALUE_WRONG = "172.16.0.54";
  inet_aton(LINK_HEAD_NODE_PROP_CONFIG_VALUE_WRONG.c_str(),
            &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link, test_link_edge_prop_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();

  std::string LINK_EDGE_PROP_WRONG = "172.16.0.55";

  inet_aton(LINK_EDGE_PROP_WRONG.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link,
     test_link_data_update) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_RESP_UPDATE = "172.16.0.36";
  inet_aton(LINK_RESP_UPDATE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));
  int flag = 1;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * > (cfgnode_cache);
      key_link_t *key_link = cache_util_ptr->get_key_structure();
      val_link_st_t *val_link = cache_util_ptr->get_val_structure();
      std::string switch_id1 = reinterpret_cast<char*> (key_link->switch_id1);
      std::string switch_id2 = reinterpret_cast<char*> (key_link->switch_id2);
      std::string port_id1 = reinterpret_cast<char*> (key_link->port_id1);
      std::string port_id2 = reinterpret_cast<char*> (key_link->port_id2);
      if (flag == 1) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, port_id1.compare("s4-eth4"));
        EXPECT_EQ(0, port_id2.compare("s5-eth5"));
        EXPECT_EQ(1, val_link->oper_status);
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, port_id2.compare("s4-eth4"));
        EXPECT_EQ(0, port_id1.compare("s5-eth5"));
        EXPECT_EQ(0, val_link->oper_status);
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, 1);
      }
    }
  }
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr1(ctr->physical_port_cache->create_iterator());
  cfgnode_cache = NULL;

  std::string LINK_RESP_UPDATE_LINK = "172.16.0.56";
  inet_aton(LINK_RESP_UPDATE_LINK.c_str(),  &val_ctr.ip_address);
  cache_empty = PFC_FALSE;
  ctr->update_ctr(key_ctr, val_ctr);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  int flag1 = 1;
  for (cfgnode_cache = itr_ptr1->PhysicalNodeFirstItem();
       itr_ptr1->IsDone() == false;
       cfgnode_cache = itr_ptr1->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * > (cfgnode_cache);
      key_link_t *key_link = cache_util_ptr->get_key_structure();
      val_link_st_t *val_link = cache_util_ptr->get_val_structure();
      std::string switch_id1 = reinterpret_cast<char*> (key_link->switch_id1);
      std::string switch_id2 = reinterpret_cast<char*> (key_link->switch_id2);
      std::string port_id1 = reinterpret_cast<char*> (key_link->port_id1);
      std::string port_id2 = reinterpret_cast<char*> (key_link->port_id2);
      if (flag1 == 1) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, port_id1.compare("s4-eth4"));
        EXPECT_EQ(0, port_id2.compare("s5-eth5"));
        EXPECT_EQ(1, val_link->oper_status);
        flag1++;
      } else if (flag1 == 2) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, port_id2.compare("s4-eth4"));
        EXPECT_EQ(0, port_id1.compare("s5-eth5"));
        EXPECT_EQ(1, val_link->oper_status);
        flag1++;
      } else if (flag1 == 3) {
        EXPECT_EQ(0, 1);
      }
    }
  }
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_link,
     test_link_data_no_update) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_RESP_UPDATE = "172.16.0.36";
  inet_aton(LINK_RESP_UPDATE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  cache_empty = PFC_FALSE;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));
  int flag = 1;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
      <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
      <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * > (cfgnode_cache);
      key_link_t *key_link = cache_util_ptr->get_key_structure();
      val_link_st_t *val_link = cache_util_ptr->get_val_structure();
      std::string switch_id1 = reinterpret_cast<char*> (key_link->switch_id1);
      std::string switch_id2 = reinterpret_cast<char*> (key_link->switch_id2);
      std::string port_id1 = reinterpret_cast<char*> (key_link->port_id1);
      std::string port_id2 = reinterpret_cast<char*> (key_link->port_id2);
      if (flag == 1) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, port_id1.compare("s4-eth4"));
        EXPECT_EQ(0, port_id2.compare("s5-eth5"));
        EXPECT_EQ(1, val_link->oper_status);
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, port_id2.compare("s4-eth4"));
        EXPECT_EQ(0, port_id1.compare("s5-eth5"));
        EXPECT_EQ(0, val_link->oper_status);
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, 1);
      }
    }
  }
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}


TEST(odcdriver_link,
     test_link_data_ctr_down) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  std::string LINK_RESP_UPDATE = "172.16.0.36";
  inet_aton(LINK_RESP_UPDATE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();
  ctr->set_connection_status(1);
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcLink obj_link(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));

  cache_empty = PFC_FALSE;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_link.fetch_config(ctr, cache_empty));
  int flag = 1;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * > (cfgnode_cache);
      key_link_t *key_link = cache_util_ptr->get_key_structure();
      val_link_st_t *val_link = cache_util_ptr->get_val_structure();
      std::string switch_id1 = reinterpret_cast<char*> (key_link->switch_id1);
      std::string switch_id2 = reinterpret_cast<char*> (key_link->switch_id2);
      std::string port_id1 = reinterpret_cast<char*> (key_link->port_id1);
      std::string port_id2 = reinterpret_cast<char*> (key_link->port_id2);
      if (flag == 1) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, port_id1.compare("s4-eth4"));
        EXPECT_EQ(0, port_id2.compare("s5-eth5"));
        EXPECT_EQ(2, val_link->oper_status);
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, switch_id1.compare("00:00:00:00:00:00:00:05"));
        EXPECT_EQ(0, switch_id2.compare("00:00:00:00:00:00:00:04"));
        EXPECT_EQ(0, port_id2.compare("s4-eth4"));
        EXPECT_EQ(0, port_id1.compare("s5-eth5"));
        EXPECT_EQ(2, val_link->oper_status);
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, 1);
      }
    }
  }
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}
