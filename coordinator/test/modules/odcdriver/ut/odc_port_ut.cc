/*
 * Copyright (c) 2013 NEC Corporation
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

void read_conf_file_port(unc::restjson::ConfFileValues_t &conf_file) {
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
}

TEST(odcdriver_port, test_port_one_add) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "00:00:00:00:00:00:00:01";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP_ONE = "172.16.0.28";
  std::string PORT_RESP_ONE = "172.16.0.29";
  std::string PORT_RESP_TWO = "172.16.0.30";

  inet_aton(SWITCH_RESP_ONE.c_str(),  &val_ctr.ip_address);

  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));


  inet_aton(PORT_RESP_ONE.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  inet_aton(PORT_RESP_TWO.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  delete ctr->physical_port_cache;
  delete ctr;
}


TEST(odcdriver_port, test_port_null_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  std::string  NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));

  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  unc::restjson::ConfFileValues_t conf_file;

  const pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcPort obj(conf_file);

  EXPECT_EQ(DRVAPI_RESPONSE_FAILURE,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_port, test_port_invalid_resp) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));
  std::string INVALID_RESPONSE = "172.0.0.0";
  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));

  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  unc::restjson::ConfFileValues_t conf_file;

  const pfc_bool_t cache_empty = PFC_FALSE;
  unc::odcdriver::OdcPort obj(conf_file);

  EXPECT_EQ(DRVAPI_RESPONSE_FAILURE,
            obj.fetch_config(ctr, &key_switch, cache_empty));
  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
}

TEST(odcdriver_port, test_port_data_add) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "00:00:00:00:00:00:00:02";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          sizeof(key_switch.switch_id)-1);

  std::string SWITCH_RESP  = "172.16.0.20";
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));

  std::string PORT_RESP  = "172.16.0.23";

  inet_aton(PORT_RESP.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,
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
          <key_port_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id = reinterpret_cast<char*>
          (key_port->sw_key.switch_id);
      std::string ctr_name = reinterpret_cast<char*>
          (key_port->sw_key.ctr_key.controller_name);

      if (flag == 1) {
        EXPECT_EQ(0, port_id.compare("s2-eth1"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, port_id.compare("s2-eth3"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, port_id.compare("s2-eth2"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      }
    }
  }

  cache_empty = PFC_FALSE;
  switch_id = "00:00:00:00:00:00:00:02";
  memset(&key_switch, 0, sizeof(key_switch_t));

  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}

TEST(odcdriver_port, test_port_data_delete) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "00:00:00:00:00:00:00:02";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));
  std::string SWITCH_RESP  = "172.16.0.20";
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));

  std::string PORT_RESP  = "172.16.0.23";

  inet_aton(PORT_RESP.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,
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
          <key_port_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id =
          reinterpret_cast<char*> (key_port->sw_key.switch_id);
      std::string ctr_name =
          reinterpret_cast<char*> (key_port->sw_key.ctr_key.controller_name);

      if (flag == 1) {
        EXPECT_EQ(0, port_id.compare("s2-eth1"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, port_id.compare("s2-eth3"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, port_id.compare("s2-eth2"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      }
    }
  }

  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 5);
  std::string PORT_RESP_DELETE = "172.16.0.25";

  inet_aton(PORT_RESP_DELETE.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);
  cache_empty = PFC_FALSE;
  switch_id = "00:00:00:00:00:00:00:02";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  delete ctr->physical_port_cache;
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}


TEST(odcdriver_port, test_port_data_update) {
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "00:00:00:00:00:00:00:02";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));
  std::string SWITCH_RESP  = "172.16.0.20";
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));

  std::string PORT_RESP  = "172.16.0.23";

  inet_aton(PORT_RESP.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,
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
          <key_port_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id =
          reinterpret_cast<char*> (key_port->sw_key.switch_id);
      std::string ctr_name =
          reinterpret_cast<char*> (key_port->sw_key.ctr_key.controller_name);

      if (flag == 1) {
        EXPECT_EQ(0, port_id.compare("s2-eth1"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, port_id.compare("s2-eth3"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, port_id.compare("s2-eth2"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      }
    }
  }

  EXPECT_EQ(ctr->physical_port_cache->cfg_list_count(), 5);
  switch_id = "00:00:00:00:00:00:00:02";

  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);

  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));

  std::string PORT_RESP_UPDATE  = "172.16.0.24";
  inet_aton(PORT_RESP_UPDATE.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,
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
          <key_port_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      val_port_st_t *val_port = cache_util_ptr->get_val_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id =
          reinterpret_cast<char*> (key_port->sw_key.switch_id);
      std::string ctr_name = reinterpret_cast<char*>
          (key_port->sw_key.ctr_key.controller_name);
      if (flag == 1) {
        EXPECT_EQ(0, port_id.compare("s2-eth1"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        EXPECT_EQ(UPPL_SWITCH_ADMIN_UP, val_port->port.admin_status);
        EXPECT_EQ(UPPL_PORT_OPER_DOWN, val_port->oper_status);
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, port_id.compare("s2-eth3"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        EXPECT_EQ(UPPL_SWITCH_ADMIN_DOWN, val_port->port.admin_status);
        EXPECT_EQ(UPPL_PORT_OPER_DOWN, val_port->oper_status);
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, port_id.compare("s2-eth4"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
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
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr,  0, sizeof(val_ctr_t));

  unc::driver::VtnDrvIntf::stub_loadVtnDrvModule();
  key_switch_t key_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  std::string switch_id = "00:00:00:00:00:00:00:02";
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));
  std::string SWITCH_RESP  = "172.16.0.20";
  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  unc::restjson::ConfFileValues_t conf_file;
  unc::driver::controller *ctr =
      new unc::odcdriver::OdcController(key_ctr,  val_ctr);
  ctr->physical_port_cache = unc::vtndrvcache::KeyTree::create_cache();

  pfc_bool_t cache_empty = PFC_TRUE;
  unc::odcdriver::OdcSwitch obj_sw(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  key_ctr_t key_ctr_update;
  val_ctr_t val_ctr_update;
  memset(&key_ctr_update, 0, sizeof(key_ctr_t));
  memset(&val_ctr_update,  0, sizeof(val_ctr_t));

  std::string PORT_RESP  = "172.16.0.23";

  inet_aton(PORT_RESP.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);

  unc::odcdriver::OdcPort obj(conf_file);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,
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
          <key_port_t, val_port_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, uint32_t> * > (cfgnode_cache);
      key_port_t *key_port = cache_util_ptr->get_key_structure();
      std::string port_id = reinterpret_cast<char*> (key_port->port_id);
      std::string node_id =
          reinterpret_cast<char*> (key_port->sw_key.switch_id);
      std::string ctr_name =
          reinterpret_cast<char*> (key_port->sw_key.ctr_key.controller_name);

      if (flag == 1) {
        EXPECT_EQ(0, port_id.compare("s2-eth1"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      } else if (flag == 2) {
        EXPECT_EQ(0, port_id.compare("s2-eth3"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      } else if (flag == 3) {
        EXPECT_EQ(0, port_id.compare("s2-eth2"));
        EXPECT_EQ(0, node_id.compare("00:00:00:00:00:00:00:02"));
        flag++;
      }
    }
  }

  switch_id = "00:00:00:00:00:00:00:02";

  inet_aton(SWITCH_RESP.c_str(),  &val_ctr.ip_address);
  ctr->update_ctr(key_ctr, val_ctr);

  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, obj_sw.fetch_config(ctr, cache_empty));

  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), switch_id.c_str(),
          strlen(switch_id.c_str()));

  std::string  PORT_RESP_EMPTY = "172.16.0.26";
  inet_aton(PORT_RESP_EMPTY.c_str(),  &val_ctr_update.ip_address);
  ctr->update_ctr(key_ctr_update, val_ctr_update);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS,
            obj.fetch_config(ctr, &key_switch, cache_empty));

  cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      EXPECT_EQ(0, 1);
    }
  }
  if (ctr->physical_port_cache != NULL) {
    delete ctr->physical_port_cache;
  }
  delete ctr;
  ctr= NULL;
  unc::driver::VtnDrvIntf::stub_unloadVtnDrvModule();
}






