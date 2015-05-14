/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbr_vlanmap.hh>
#include <odc_controller.hh>
#include <gtest/gtest.h>
#include <string>

TEST(odcdriver,  test_fetch_conf_vtn_name_empty) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string ip  = "172.16.0.1";

  inet_aton(ip.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr,
                                                         &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  ret_val);
  EXPECT_EQ(0U, cfgnode_vector.size());

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_fetch_conf_vbr_name_empty) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string ip  = "172.16.0.1";

  inet_aton(ip.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr,
                                                         &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  ret_val);
  EXPECT_EQ(0U, cfgnode_vector.size());

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_fetch_conf_null_response) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr,
                                                         &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  ret_val);
  EXPECT_EQ(0U, cfgnode_vector.size());

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_fetch_conf_null_response_data) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr, 0,  sizeof(val_vbr_t));
  std::string NULL_RESP_DATA = "172.16.0.19";
  inet_aton(NULL_RESP_DATA.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr, &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  ret_val);
  EXPECT_EQ(0U, cfgnode_vector.size());

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}



TEST(odcdriver,  test_fetch_conf_not_found_response) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string NOT_FOUND_404         = "172.16.0.12";

  inet_aton(NOT_FOUND_404.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr, &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  ret_val);
  EXPECT_EQ(0U, cfgnode_vector.size());

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_fetch_conf_service_unavilable_response) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string SERVICE_UNAVAILABLE_503      = "172.16.0.13";

  inet_aton(SERVICE_UNAVAILABLE_503.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr, &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  ret_val);
  EXPECT_EQ(0U, cfgnode_vector.size());

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}


TEST(odcdriver,  test_fetch_conf_empty_response) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string VLAN_MAP_EMPTY   = "172.16.0.14";
  inet_aton(VLAN_MAP_EMPTY.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr, &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_RC_SUCCESS,  ret_val);
  EXPECT_EQ(0U, cfgnode_vector.size());

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_fetch_conf_vlanmap_incorrect_response) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string VLAN_MAP_INCORRECT_RESP               = "172.16.0.16";
  inet_aton(VLAN_MAP_INCORRECT_RESP.c_str(),  &val_ctr.ip_address);
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr, &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  ret_val);
  EXPECT_EQ(0U, cfgnode_vector.size());

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_fetch_conf_vlanmap_incorrect_response_body) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string VLAN_MAP_VLAN_INCORRECT_RESP               = "172.16.0.17";
  inet_aton(VLAN_MAP_VLAN_INCORRECT_RESP.c_str(),  &val_ctr.ip_address);

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr, &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,  ret_val);
  EXPECT_EQ(0U, cfgnode_vector.size());

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_fetch_conf_vlanmap_resp) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string VLAN_MAP_RESP            = "172.16.0.15";
  inet_aton(VLAN_MAP_RESP.c_str(),  &val_ctr.ip_address);

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr, &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_RC_SUCCESS,  ret_val);
  EXPECT_EQ(2U, cfgnode_vector.size());
  uint vlan_id = 0;
  std::string logical_id = "";
  int flag = 1;
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it !=  cfgnode_vector.end(); ++it ) {
    if (*it !=  NULL) {
      unc::vtndrvcache::ConfigNode *cfg_node = *it;
      if (cfg_node != NULL) {
        unc::vtndrvcache::CacheElementUtil<
          key_vlan_map_t, pfcdrv_val_vlan_map_t, pfcdrv_val_vlan_map_t,
          uint32_t> *cache_util_ptr =
           static_cast<unc::vtndrvcache::CacheElementUtil<
            key_vlan_map_t, pfcdrv_val_vlan_map_t, pfcdrv_val_vlan_map_t,
                                       uint32_t> *>(cfg_node);
        if (cache_util_ptr == NULL) {
          return;
        }
        if  (flag == 1) {
          vlan_id = 65535;
          logical_id = "SW-00:00:00:00:00:00:00:03";
        }  else {
          vlan_id = 7;
        }
        key_vlan_map_t *vlanmap_key = cache_util_ptr->get_key_structure();
        pfcdrv_val_vlan_map_t *vlanmap_val = cache_util_ptr->get_val_structure();

        if ((vlanmap_key == NULL) || (vlanmap_val == NULL)) {
          pfc_log_error("key or val strucure is NULL");
          return;
        }
        uint vlan_id_ctr = vlanmap_val->vm.vlan_id;
        std::string logical_port_ctr = reinterpret_cast<char*>
                                      (vlanmap_key->logical_port_id);
        EXPECT_EQ(vlan_id_ctr , vlan_id);
        if (vlanmap_key->logical_port_id_valid == 1) {
          EXPECT_EQ(0, logical_port_ctr.compare(logical_id));
        }
        if (flag == 2) {
          EXPECT_EQ(0, vlanmap_key->logical_port_id_valid);
        }
      }
      flag++;
    }
  }
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it !=  cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_fetch_conf_vlanmap_resp_max_vlanid) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string VLAN_MAP_RESP            = "172.16.0.15";
  inet_aton(VLAN_MAP_RESP.c_str(),  &val_ctr.ip_address);

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr, &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_RC_SUCCESS,  ret_val);
  EXPECT_EQ(2U, cfgnode_vector.size());
  std::string logical_id = "";
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  key_vlan_map.logical_port_id_valid  = 1;
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));

  val_vlan_map.vm.vlan_id = 65535;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
                                            key_vlan_map,
                                            val_vlan_map, switch_id, ctr,
                                            is_switch_exist, port_id));
  EXPECT_EQ(is_switch_exist, PFC_FALSE);
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it !=  cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  if (ctr != NULL) {
    delete ctr;
    ctr =  NULL;
  }
}

TEST(odcdriver,  test_fetch_conf_already_exist_vector) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string VLAN_MAP_RESP            = "172.16.0.15";
  inet_aton(VLAN_MAP_RESP.c_str(),  &val_ctr.ip_address);

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);

  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  odc_ctr->vlan_vector.push_back("vtn1.vbr1.ANY.7");

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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr, &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_RC_SUCCESS,  ret_val);
  EXPECT_EQ(2U, cfgnode_vector.size());
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it !=  cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_fetch_conf_vlanmap_resp_max_vlan_id_any) {
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  memset(&key_vbr, 0 , sizeof(key_vbr_t));
  memset(&val_vbr,  0,  sizeof(val_vbr_t));
  std::string VLAN_MAP_RESP_ANY_0 = "172.16.0.18";

  inet_aton(VLAN_MAP_RESP_ANY_0.c_str(),  &val_ctr.ip_address);

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
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
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  std::vector <unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  UncRespCode ret_val = odc_vbr_vlanmap.fetch_config(ctr, &key_vbr,
                                                         cfgnode_vector);
  EXPECT_EQ(UNC_RC_SUCCESS,  ret_val);
  EXPECT_EQ(2U, cfgnode_vector.size());
  std::string logical_id = "";
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  key_vlan_map.logical_port_id_valid  = 0;
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  std::string switch_id = "ANY";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));

  val_vlan_map.vm.vlan_id = 65535;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
         key_vlan_map, val_vlan_map, switch_id, ctr, is_switch_exist, port_id));
  EXPECT_EQ(is_switch_exist, PFC_FALSE);
  for ( std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
       cfgnode_vector.begin(); it !=  cfgnode_vector.end(); ++it ) {
    if (*it != NULL) {
      delete *it;
    }
  }

  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_switch_requested_incorrect_vlan_parse) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);

  odc_ctr->vlan_vector.push_back("OF-00:00:00");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);

  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id, odc_ctr,
          is_switch_exist, port_id));
  EXPECT_EQ(is_switch_exist, PFC_FALSE);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_switch_requested_vlan_same_ANY) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);

  odc_ctr->vlan_vector.push_back("vtn1.vbr1.ANY.10");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);

  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(is_switch_exist, PFC_FALSE);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_switch_requested_vlan_same_sw) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);

  odc_ctr->vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:01.10");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);

  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id, odc_ctr,
          is_switch_exist, port_id));
  EXPECT_EQ(is_switch_exist, PFC_FALSE);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_switch_requested_vlan_same_diff_vtn) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:02.20");
  odc_ctr->vlan_vector.push_back("vtn2.vbr1.OF-00:00:00:00:00:00:00:01.10");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(is_switch_exist, PFC_FALSE);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_switch_requested_vlan_same_diff_vbr) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:02.20");
  odc_ctr->vlan_vector.push_back("vtn1.vbr2.OF-00:00:00:00:00:00:00:01.10");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(is_switch_exist, PFC_FALSE);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_switch_requested_vlan_diff) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:01.20");
  odc_ctr->vlan_vector.push_back("vtn1.vbr2.OF-00:00:00:00:00:00:00:01.10");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_switch_requested_vlan_diff_sw_exist) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:01.20");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  std::string generated_port = "OF-00:00:00:00:00:00:00:01.20";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(PFC_TRUE, is_switch_exist);
  EXPECT_EQ(0, port_id.compare(generated_port));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_switch_requested_vlan_diff_any_exist) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn1.vbr1.ANY.20");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  std::string generated_port = "ANY.20";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(PFC_FALSE, is_switch_exist);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_switch_requested_vlan_diff_vbr) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn2.vbr2.ANY.10");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  std::string generated_port = "ANY.20";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(PFC_FALSE, is_switch_exist);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_ANY_requested_vlan_wrong) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 0;
  std::string switch_id = "ANY";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn2vbr2ANY10");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(PFC_FALSE, is_switch_exist);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}
TEST(odcdriver,  test_validate_vlan_ANY_requested_vlan_same) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 0;
  std::string switch_id = "ANY";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn1.vbr1.ANY.10");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(PFC_FALSE, is_switch_exist);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}
TEST(odcdriver,  test_validate_vlan_ANY_requested_vlan_same_sw) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 0;
  std::string switch_id = "ANY";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 10;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:01.10");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(PFC_FALSE, is_switch_exist);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}
TEST(odcdriver,  test_validate_vlan_ANY_requested_vlan_same_ANY) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 0;
  std::string switch_id = "ANY";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 220;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn1.vbr1.ANY.10");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  std::string genarate_id = "ANY.10";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "";
  conf_file.password = "";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(PFC_TRUE, is_switch_exist);
  EXPECT_EQ(0, port_id.compare(genarate_id));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_validate_vlan_ANY_requested_vlan_diff_SW) {
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vbrname.c_str()));
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  strlen(vtnname.c_str()));

  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  key_vlan_map.logical_port_id_valid  = 0;
  std::string switch_id = "ANY";
  strncpy(reinterpret_cast<char*>
          (key_vlan_map.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  val_vlan_map.vm.vlan_id = 220;

  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  if (odc_ctr == NULL) {
    return;
  }
  odc_ctr->vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:01.20");
  pfc_bool_t is_switch_exist = PFC_FALSE;
  std::string port_id = "";
  std::string genarate_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS, odc_vbr_vlanmap.validate_vlan_exist(
          key_vlan_map, val_vlan_map, switch_id,
          odc_ctr, is_switch_exist, port_id));
  EXPECT_EQ(PFC_FALSE, is_switch_exist);
  EXPECT_EQ(0, port_id.compare(genarate_id));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}


TEST(odcdriver, test_create_command_null_resp) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;

  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string NULL_RESPONSE = "172.16.0.0";
  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 10;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }

  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}


TEST(odcdriver_mod, test_create_command_invalidreq) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string INVALID_RESPONSE = "172.0.0.0";
  inet_aton(INVALID_RESPONSE.c_str(),  &val_ctr.ip_address);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 10;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}


TEST(odcdriver, test_create_cmd_invalid_vbrname) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 10;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_create_cmd_invalid_vtnname) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string ip_add =  "172.16.0.2";
  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 10;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_create_cmd_invalid_format) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  vlanmap_val.vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  std::string CREATE_201  = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:0:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 10;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}


TEST(odcdriver, test_create_cmd) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  vlanmap_val.vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  std::string CREATE_201  = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 10;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_create_cmd_logicalport_in_diff_format) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  vlanmap_val.vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  std::string CREATE_201  = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-0000-0000-0000-0001";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 10;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_create_cmd_logicalport_in_diff_format_invalid) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  vlanmap_val.vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  std::string CREATE_201  = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-0000-0000-0000-00012";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 10;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_create_cmd_validatevlan_failure) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string VLAN_MAP_RESP            = "172.16.0.15";
  inet_aton(VLAN_MAP_RESP.c_str(),  &val_ctr.ip_address);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 65535;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_update_cmd_vbrvlanmap) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val1;
  pfcdrv_val_vlan_map_t vlanmap_val2;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val1, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&vlanmap_val2, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  vlanmap_val1.vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  std::string CREATE_201  = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val1.vm.vlan_id = 10;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.update_cmd(vlanmap_key, vlanmap_val1, vlanmap_val2, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_create_cmd_validate_fail) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string CREATE_201  = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "ANY";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 65535;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_create_cmd_validate_failure) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string VLAN_MAP_RESP_ANY_0 = "172.16.0.18";

  inet_aton(VLAN_MAP_RESP_ANY_0.c_str(),  &val_ctr.ip_address);


  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 0;
  std::string switch_id = "ANY";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 65535;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_create_cmd_validate_logicalportid_fail) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string CREATE_201  = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "ANY123";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 10;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_create_cmd_UNTAGGED) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  vlanmap_val.vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  std::string CREATE_201  = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 0;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 65535;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_create_update_invalid) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  vlanmap_val.vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_INVALID;
  std::string CREATE_201  = "172.16.0.1";
  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 0;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  vlanmap_val.vm.vlan_id = 65535;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.create_cmd(vlanmap_key, vlanmap_val, ctr));
  delete ctr;
  ctr = NULL;
}



TEST(odcdriver, test_delete_existing_vlan_map_null_strid) {
  key_vlan_map_t vlanmap_key;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string CREATE_201  = "172.16.0.1";

  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  const std::string str_mapping_id = "";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);

  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.del_existing_vlanmap(vlanmap_key , ctr , str_mapping_id));
  delete ctr;
  ctr = NULL;
}


TEST(odcdriver, test_delete_existing_vlan_map_url_empty) {
  key_vlan_map_t vlanmap_key;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string CREATE_201  = "172.16.0.1";

  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  std::string vbrname =  "";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  const std::string str_mapping_id = "abc";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);

  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.del_existing_vlanmap(vlanmap_key , ctr , str_mapping_id));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_delete_existing_vlan_map_resp_null) {
  key_vlan_map_t vlanmap_key;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string NULL_RESPONSE = "172.16.0.0";

  inet_aton(NULL_RESPONSE.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  const std::string str_mapping_id = "abc";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);

  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.del_existing_vlanmap(vlanmap_key , ctr , str_mapping_id));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_delete_existing_vlan_map__success) {
  key_vlan_map_t vlanmap_key;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string RESPONSE = "172.16.0.2";

  inet_aton(RESPONSE.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  const std::string str_mapping_id
      = "/controller/nb/v2/vtn/default/vtns/vtn1/vbridges/";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);

  EXPECT_EQ(UNC_RC_SUCCESS,
            obj.del_existing_vlanmap(vlanmap_key , ctr , str_mapping_id));
  delete ctr;
  ctr = NULL;
}


TEST(odcdriver, test_delete_existing_vlan_map__not_202) {
  key_vlan_map_t vlanmap_key;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string RESPONSE = "172.16.0.1";

  inet_aton(RESPONSE.c_str(),  &val_ctr.ip_address);

  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  const std::string str_mapping_id =
      "/controller/nb/v2/vtn/default/vtns/vtn1/vbridges/";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);

  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            obj.del_existing_vlanmap(vlanmap_key , ctr , str_mapping_id));
  delete ctr;
  ctr = NULL;
}

TEST(odcdriver, test_delete_existing_vlan_map_201_resp) {
  key_vlan_map_t vlanmap_key;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;

  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));

  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_201  = "172.16.0.1";

  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 1;
  const std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  std::string str_vlanid = "20";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ("OF-00:00:00:00:00:00:00:01.20",
            obj.generate_vlanmap_id(vlanmap_key, str_vlanid, switch_id));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}



TEST(odcdriver, generate_vlanmap_id_invalid) {
  key_vlan_map_t vlanmap_key;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_201  = "172.16.0.1";

  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 0;
  const std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  std::string str_vlanid = "20";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ("ANY.20",
            obj.generate_vlanmap_id(vlanmap_key, str_vlanid, switch_id));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}


TEST(odcdriver, generate_vlanmap_id_invalid_UNTAGGED_VLANID) {
  key_vlan_map_t vlanmap_key;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));

  std::string CREATE_201  = "172.16.0.1";

  inet_aton(CREATE_201.c_str(),  &val_ctr.ip_address);
  std::string vbrname =  "vbr1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbrname.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  std::string vtnname =  "vtn1";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtnname.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  std::string description =  "descrip";
  vlanmap_key.logical_port_id_valid  = 0;
  const std::string switch_id = "SW-00:00:00:00:00:00:00:01";
  strncpy(reinterpret_cast<char*>
          (vlanmap_key.logical_port_id),
          switch_id.c_str(),
          strlen(switch_id.c_str()));
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  if (ctr == NULL) {
    return;
  }
  std::string str_vlanid = "65535";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ("ANY.0",
            obj.generate_vlanmap_id(vlanmap_key, str_vlanid, switch_id));
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}


TEST(odcdriver, test_validate_logical_port_id) {
  std::string logicalportid = "SW-00:00:00:00:00:00:00:01";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(unc::odcdriver::ODC_DRV_SUCCESS,
            obj.validate_logical_port_id(logicalportid));
}


TEST(odcdriver, test_validate_logical_port_id_failure_case1) {
  std::string logicalportid = "SW-00:00:00:00:00:00:00";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(unc::odcdriver::ODC_DRV_FAILURE,
            obj.validate_logical_port_id(logicalportid));
}

TEST(odcdriver, test_validate_logical_port_id_invalid) {
  std::string logicalportid = "00:00:00:00:00:00:00:01";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(unc::odcdriver::ODC_DRV_FAILURE,
            obj.validate_logical_port_id(logicalportid));
}

TEST(odcdriver, test_validate_logical_port_id_incorrect) {
  std::string logicalportid = "SW-00:0000:00:0000:0001";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand obj(conf_file);
  EXPECT_EQ(unc::odcdriver::ODC_DRV_FAILURE,
            obj.validate_logical_port_id(logicalportid));
}

TEST(odcdriver,  test_generate_vlanmap_id_untagged) {
  key_vlan_map_t vlanmap_key;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  const std::string logical_id = "SW-00:00:00:00:00:00:00:02";
  std::string vlan_id =  "65535";
  strncpy(reinterpret_cast<char*>(vlanmap_key.logical_port_id),
          logical_id.c_str(),  sizeof(vlanmap_key.logical_port_id)-1);
  vlanmap_key.logical_port_id_valid = 1;
  std::string ret_val ="";
  std::string generated_id = "OF-00:00:00:00:00:00:00:02.0";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  ret_val = odc_vbr_vlanmap.generate_vlanmap_id(
      vlanmap_key, vlan_id, logical_id);
  EXPECT_EQ(ret_val, generated_id);
}

TEST(odcdriver,  test_generate_vlanmap_id_valid) {
  key_vlan_map_t vlanmap_key;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  const std::string logical_id = "SW-00:00:00:00:00:00:00:02";
  std::string vlan_id =  "10";
  strncpy(reinterpret_cast<char*>(vlanmap_key.logical_port_id),
          logical_id.c_str(),  sizeof(vlanmap_key.logical_port_id)-1);
  vlanmap_key.logical_port_id_valid = 1;
  std::string ret_val ="";
  std::string generated_id = "OF-00:00:00:00:00:00:00:02.10";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  ret_val = odc_vbr_vlanmap.generate_vlanmap_id(
      vlanmap_key, vlan_id, logical_id);
  EXPECT_EQ(ret_val, generated_id);
}

TEST(odcdriver,  test_generate_vlanmap_id_logical_port_id_invalid) {
  key_vlan_map_t vlanmap_key;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  const std::string logical_id = "SW-00:00:00:00:00:00:00:02";
  std::string vlan_id =  "10";
  strncpy(reinterpret_cast<char*>(vlanmap_key.logical_port_id),
          logical_id.c_str(),  sizeof(vlanmap_key.logical_port_id)-1);
  vlanmap_key.logical_port_id_valid = 0;
  std::string ret_val ="";
  std::string generated_id = "ANY.10";
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  ret_val = odc_vbr_vlanmap.generate_vlanmap_id(
      vlanmap_key, vlan_id, logical_id);
  EXPECT_EQ(ret_val, generated_id);
}

//  delete_cmd

TEST(odcdriver,  test_delete_cmd_invalid_logicalport_id_1) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string vtn_name = "vtn1";
  std::string vbr_name = "vbr1";
  std::string logical_id = "PP-00:00:00:00:00:00:00:02";
  std::string vlan_id =  "20";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtn_name.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbr_name.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.logical_port_id),
          logical_id.c_str(),  sizeof(vlanmap_key.logical_port_id)-1);
  vlanmap_key.logical_port_id_valid = 1;
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  UncRespCode ret_val = odc_vbr_vlanmap.delete_cmd(vlanmap_key,
                                                       vlanmap_val,
                                                       ctr);
  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_delete_cmd_invalid_logicalport_id_2) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string ip_add  = "172.16.0.0";

  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vtn_name = "vtn1";
  std::string vbr_name = "vbr1";
  std::string logical_id = "SW-00:00:00:00:00:00:00:02";
  std::string vlan_id =  "20";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtn_name.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbr_name.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.logical_port_id),
          logical_id.c_str(),  sizeof(vlanmap_key.logical_port_id)-1);
  vlanmap_key.logical_port_id_valid = 0;
  std::vector<std::string> vtn_vbr_vlan_vector;
  vtn_vbr_vlan_vector.push_back("vtn1.vbr1.ANY.20");
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  odc_ctr->vlan_vector = vtn_vbr_vlan_vector;
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  UncRespCode ret_val = odc_vbr_vlanmap.delete_cmd(vlanmap_key,
                                                       vlanmap_val,
                                                       ctr);
  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}
TEST(odcdriver,  test_delete_cmd_httpresponse_error) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string ip_add  = "172.16.0.0";

  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vtn_name = "vtn1";
  std::string vbr_name = "vbr1";
  std::string logical_id = "SW-00:00:00:00:00:00:00:02";
  std::string vlan_id =  "20";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtn_name.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbr_name.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.logical_port_id),
          logical_id.c_str(),  sizeof(vlanmap_key.logical_port_id)-1);
  vlanmap_key.logical_port_id_valid = 1;
  std::vector<std::string> vtn_vbr_vlan_vector;
  vtn_vbr_vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:02.20");
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  odc_ctr->vlan_vector = vtn_vbr_vlan_vector;
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  UncRespCode ret_val = odc_vbr_vlanmap.delete_cmd(vlanmap_key, vlanmap_val,
                                                       ctr);
  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_delete_cmd_invalid_http_responce) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string ip_add  = "172.0.0.0";

  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vtn_name = "vtn1";
  std::string vbr_name = "vbr1";
  std::string logical_id = "SW-00:00:00:00:00:00:00:02";
  std::string vlan_id =  "20";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtn_name.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbr_name.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.logical_port_id),
          logical_id.c_str(),  sizeof(vlanmap_key.logical_port_id)-1);
  vlanmap_key.logical_port_id_valid = 1;
  std::vector<std::string> vtn_vbr_vlan_vector;
  vtn_vbr_vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:02.20");
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  odc_ctr->vlan_vector = vtn_vbr_vlan_vector;
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  UncRespCode ret_val = odc_vbr_vlanmap.delete_cmd(vlanmap_key, vlanmap_val,
                                                       ctr);
  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_delete_cmd_vtn_name_empty) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string ip_add  = "172.0.0.0";

  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vbr_name = "vbr1";
  std::string logical_id = "SW-00:00:00:00:00:00:00:02";
  std::string vlan_id =  "20";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbr_name.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.logical_port_id),
          logical_id.c_str(),  sizeof(vlanmap_key.logical_port_id)-1);
  vlanmap_key.logical_port_id_valid = 1;
  std::vector<std::string> vtn_vbr_vlan_vector;
  vtn_vbr_vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:02.20");
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  odc_ctr->vlan_vector = vtn_vbr_vlan_vector;
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  UncRespCode ret_val = odc_vbr_vlanmap.delete_cmd(vlanmap_key, vlanmap_val,
                                                       ctr);
  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}

TEST(odcdriver,  test_delete_cmd_valid) {
  key_vlan_map_t vlanmap_key;
  pfcdrv_val_vlan_map_t vlanmap_val;
  key_ctr_t key_ctr;
  val_ctr_t val_ctr;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr, 0, sizeof(val_ctr_t));
  std::string ip_add  = "172.16.0.14";

  inet_aton(ip_add.c_str(),  &val_ctr.ip_address);
  memset(&vlanmap_key, 0, sizeof(key_vlan_map_t));
  memset(&vlanmap_val, 0, sizeof(pfcdrv_val_vlan_map_t));
  std::string vtn_name = "vtn1";
  std::string vbr_name = "vbr1";
  std::string logical_id = "SW-00:00:00:00:00:00:00:02";
  std::string vlan_id =  "20";
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name),
          vtn_name.c_str(),  sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name),
          vbr_name.c_str(),  sizeof(vlanmap_key.vbr_key.vbridge_name)-1);
  strncpy(reinterpret_cast<char*>(vlanmap_key.logical_port_id),
          logical_id.c_str(),  sizeof(vlanmap_key.logical_port_id)-1);
  vlanmap_key.logical_port_id_valid = 1;
  std::vector<std::string> vtn_vbr_vlan_vector;
  vtn_vbr_vlan_vector.push_back("vtn1.vbr1.OF-00:00:00:00:00:00:00:02.20");
  unc::driver::controller* ctr  =
      new  unc::odcdriver::OdcController(key_ctr,  val_ctr);
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  odc_ctr->vlan_vector = vtn_vbr_vlan_vector;
  unc::restjson::ConfFileValues_t conf_file;
  conf_file.odc_port = 8080;
  conf_file.user_name = "admin";
  conf_file.password = "admin";
  unc::odcdriver::OdcVbrVlanMapCommand odc_vbr_vlanmap(conf_file);
  UncRespCode ret_val = odc_vbr_vlanmap.delete_cmd(vlanmap_key, vlanmap_val,
                                                       ctr);
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
  if (ctr != NULL) {
    delete ctr;
    ctr = NULL;
  }
}


