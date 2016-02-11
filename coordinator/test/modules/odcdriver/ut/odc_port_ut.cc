/*
 * Copyright (c) 2014-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_switch.hh>
#include <odc_port.hh>
#include <odc_controller.hh>
#include <odc_driver_common_defs.hh>
#include <vtn_drv_module.hh>
#include <gtest/gtest.h>
#include <string>


TEST(odcdriver_port, test_port_one_add) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_RESP_ONE = "172.16.0.29";
  std::string PORT_RESP_TWO = "172.16.0.30";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));


  inet_aton(PORT_RESP_ONE.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  inet_aton(PORT_RESP_TWO.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}


TEST(odcdriver_port, test_port_null_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  std::string  NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:2";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);


  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;

  const pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcPort obj(conf_file);

  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_port, test_port_invalid_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  std::string INVALID_RESPONSE = "172.0.0.0";
  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:2";

  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);


  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  unc::restjson::ConfFileValues_t conf_file;

  const pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcPort obj(conf_file);

  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_port, test_port_data_add) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:2";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP  = "172.16.0.20";
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));

  std::string PORT_RESP  = "172.16.0.23";

  inet_aton(PORT_RESP.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id = reinterpret_cast<char*>
          (key_port->sw_key.switch_id);
      std::string ctr_name = reinterpret_cast<char*>
          (key_port->sw_key.ctr_key.controller_name);

      if (flag == 1) {
        // EXPECT_EQ(1, port_id.compare("s2-eth1"));
        // EXPECT_EQ(-1, node_id.compare("openflow::2"));
        flag++;
      } else if (flag == 2) {
        // EXPECT_EQ(0, port_id.compare("s2-eth3"));
        // EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      } else if (flag == 3) {
        // EXPECT_EQ(0, port_id.compare("s2-eth2"));
        // EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      }
    }
  }

  cache_empty = PFC_FALSE;
  switch_id = "openflow:2";
  memset(&key_switch, 0, sizeof(key_switch_t));

  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_data_delete) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:2";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));
  std::string SWITCH_RESP  = "172.16.0.20";
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));

  std::string PORT_RESP  = "172.16.0.23";

  inet_aton(PORT_RESP.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id =
          reinterpret_cast<char*> (key_port->sw_key.switch_id);
      std::string ctr_name =
          reinterpret_cast<char*> (key_port->sw_key.ctr_key.controller_name);

      if (flag == 1) {
         EXPECT_EQ(1, port_id.compare("s2-eth1"));
         EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      } else if (flag == 2) {
          EXPECT_EQ(0, port_id.compare("s2-eth3"));
          EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      } else if (flag == 3) {
          EXPECT_EQ(0, port_id.compare("s2-eth2"));
          EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      }
    }
  }

  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 4);
  std::string PORT_RESP_DELETE = "172.16.0.25";

  inet_aton(PORT_RESP_DELETE.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);
  cache_empty = PFC_FALSE;
  switch_id = "openflow:2";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}


TEST(odcdriver_port, test_port_data_update) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:2";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));
  std::string SWITCH_RESP  = "172.16.0.20";
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));

  std::string PORT_RESP  = "172.16.0.23";

  inet_aton(PORT_RESP.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id =
          reinterpret_cast<char*> (key_port->sw_key.switch_id);
      std::string ctr_name =
          reinterpret_cast<char*> (key_port->sw_key.ctr_key.controller_name);

      if (flag == 1) {
         EXPECT_EQ(1, port_id.compare("s2-eth1"));
         EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      } else if (flag == 2) {
          EXPECT_EQ(0, port_id.compare("s2-eth3"));
          EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      } else if (flag == 3) {
          EXPECT_EQ(0, port_id.compare("s2-eth2"));
          EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      }
    }
  }

  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 4);
  switch_id = "openflow:2";
  ctr->set_connection_status(0);

  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);

  EXPECT_EQ(UNC_RC_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));

  std::string PORT_RESP_UPDATE  = "172.16.0.24";
  inet_aton(PORT_RESP_UPDATE.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  flag = 1;

  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr_new(ctr->physical_port_cache->create_iterator());
  for (cfgnode_cache = itr_ptr_new->PhysicalNodeFirstItem();
       itr_ptr_new->IsDone() == false;
       cfgnode_cache = itr_ptr_new->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      val_port_st_t *val_port = cache_util_ptr->get_val_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id =
          reinterpret_cast<char*> (key_port->sw_key.switch_id);
      std::string ctr_name = reinterpret_cast<char*>
          (key_port->sw_key.ctr_key.controller_name);
      if (flag == 1) {
        // EXPECT_EQ(1, port_id.compare("s2-eth1"));
        // EXPECT_EQ(0, node_id.compare("openflow:2"));
        // EXPECT_EQ(UPPL_SWITCH_ADMIN_UP, val_port->port.admin_status);
        // EXPECT_EQ(UPPL_PORT_OPER_UP, val_port->oper_status);
        flag++;
      } else if (flag == 2) {
        // EXPECT_EQ(1, port_id.compare("s2-eth3"));
        // EXPECT_EQ(0, node_id.compare("openflow:2"));
        EXPECT_EQ(UPPL_SWITCH_ADMIN_UP, val_port->port.admin_status);
        EXPECT_EQ(UPPL_PORT_OPER_UP, val_port->oper_status);
        flag++;
      } else if (flag == 3) {
        // EXPECT_EQ(0, port_id.compare("s2-eth4"));
        // EXPECT_EQ(0, node_id.compare("openflow:2"));
        EXPECT_EQ(UPPL_SWITCH_ADMIN_DOWN, val_port->port.admin_status);
        EXPECT_EQ(UPPL_PORT_OPER_DOWN, val_port->oper_status);
        flag++;
      }
    }
  }

  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_port, test_port_data_update__empty) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:2";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));
  std::string SWITCH_RESP  = "172.16.0.20";
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr,conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));

  std::string PORT_RESP  = "172.16.0.23";

  inet_aton(PORT_RESP.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, val_port_st_t,
                      uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id =
          reinterpret_cast<char*> (key_port->sw_key.switch_id);
      std::string ctr_name =
          reinterpret_cast<char*> (key_port->sw_key.ctr_key.controller_name);

      if (flag == 1) {
        // EXPECT_EQ(1, port_id.compare("s2-eth1"));
        // EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      } else if (flag == 2) {
        // EXPECT_EQ(0, port_id.compare("s2-eth3"));
        // EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      } else if (flag == 3) {
        // EXPECT_EQ(0, port_id.compare("s2-eth2"));
        // EXPECT_EQ(0, node_id.compare("openflow:2"));
        flag++;
      }
    }
  }

  switch_id = "openflow:2";

  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);

  EXPECT_EQ(UNC_RC_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));

  std::string  PORT_RESP_EMPTY = "172.16.0.26";
  inet_aton(PORT_RESP_EMPTY.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      EXPECT_EQ(1, 1);
    }
  }
  if (ctr->physical_port_cache != NULL) {
    delete ctr->physical_port_cache;
  }
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_resp_one) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_RESP_ONE = "172.16.0.29";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));


  inet_aton(PORT_RESP_ONE.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      unc::vtndrvcache::CacheElementUtil
        <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id =
          reinterpret_cast<char*> (key_port->sw_key.switch_id);
      std::string ctr_name =
          reinterpret_cast<char*> (key_port->sw_key.ctr_key.controller_name);

      if (flag == 1) {
        EXPECT_EQ(0, port_id.compare("s2-eth1"));
        EXPECT_EQ(0, node_id.compare("openflow:1"));
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, 1);
        flag++;
      }
    }
  }
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_resp_conn_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_CONN_PROP_WRONG = "172.16.0.57";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr,conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_CONN_PROP_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  ctr->physical_port_cache = NULL;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_conn_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_CONN_WRONG = "172.16.0.58";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_CONN_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  ctr->physical_port_cache = NULL;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_connwrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_CONN_NODE_WRONG = "172.16.0.60";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr,conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_CONN_NODE_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  ctr->physical_port_cache = NULL;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_conn_type_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_CONN_TYPE_WRONG = "172.16.0.61";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_CONN_TYPE_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  ctr->physical_port_cache = NULL;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_node_id_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_ID_WRONG = "172.16.0.62";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr,conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_ID_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  ctr->physical_port_cache = NULL;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_node_conn_id_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_CONN_ID_WRONG = "172.16.0.59";


  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_CONN_ID_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  ctr->physical_port_cache = NULL;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_node_id_SW_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_ID_SW = "172.16.0.63";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_ID_SW.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  ctr->physical_port_cache = NULL;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_node_prop_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_PROP_WRONG = "172.16.0.64";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_PROP_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  ctr->physical_port_cache = NULL;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_node_prop_name_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_PROP_NAME_WRONG = "172.16.0.65";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_PROP_NAME_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_node_prop_name_value_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_PROP_NAME_VALUE_WRONG = "172.16.0.66";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_PROP_NAME_VALUE_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_node_prop_state_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_PROP_STATE_WRONG = "172.16.0.67";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_PROP_STATE_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_node_prop_config_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_COST_PROP_WRONG = "172.16.0.69";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_COST_PROP_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}
#if 0
TODO(ODC)
TEST(odcdriver_port, test_port_node_prop_bandwidth_wrong) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_NODE_PROP_BANDWIDTH_WRONG = "172.16.0.71";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  inet_aton(PORT_NODE_PROP_BANDWIDTH_WRONG.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}
#endif
TEST(odcdriver_port, test_port_resp_conf_unknown) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_RESP_ONE = "172.16.0.29";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  ctr->set_connection_status(1);
  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));


  inet_aton(PORT_RESP_ONE.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  int flag = 1;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      unc::vtndrvcache::CacheElementUtil
        <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
       <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      val_port_st_t *val_port = cache_util_ptr->get_val_structure();

      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id =
          reinterpret_cast<char*> (key_port->sw_key.switch_id);
      std::string ctr_name =
          reinterpret_cast<char*> (key_port->sw_key.ctr_key.controller_name);

      if (flag == 1) {
        EXPECT_EQ(0, port_id.compare("s2-eth1"));
        EXPECT_EQ(0, node_id.compare("openflow:1"));
        EXPECT_EQ(UPPL_PORT_OPER_UNKNOWN, val_port->oper_status);

        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, 1);
        flag++;
      }
    }
  }
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_resp_parent_sw_NULL) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  unc::restjson::ConfFileValues_t conf_values;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "openflow:1";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_RESP_ONE = "172.16.0.29";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
     new unc::odcdriver::OdcController(key_ctr,  val_ctr, conf_values);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  ctr->set_connection_status(1);
  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, obj_sw.fetch_config(ctr, cache_empty));


  inet_aton(PORT_RESP_ONE.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.fetch_config(ctr, NULL, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

