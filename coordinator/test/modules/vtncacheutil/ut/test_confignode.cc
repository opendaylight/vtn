/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <keytree.hh>
#include <vtn_conf_data_element_op.hh>

namespace unc {
namespace vtndrvcache {
TEST(TypeToStrFun, keytype) {
  std::string ret = TypeToStrFun(UNC_KT_VTN);
  EXPECT_EQ(ret, "UNC_KT_VTN");

  ret = TypeToStrFun(UNC_KT_VBRIDGE);
  EXPECT_EQ(ret, "UNC_KT_VBRIDGE");

  ret = TypeToStrFun(UNC_KT_VBR_IF);
  EXPECT_EQ(ret, "UNC_KT_VBR_IF");

  ret = TypeToStrFun(UNC_KT_ROOT);
  EXPECT_EQ(ret, "UNC_KT_ROOT");

  ret = TypeToStrFun(UNC_KT_VLINK);
  EXPECT_EQ(ret, "Unknown");
}

TEST(print_key, check) {
  RootNode *rootnode_obj = new RootNode;
  int level_index = 1;
  rootnode_obj->print_key(level_index);
  delete rootnode_obj;
  rootnode_obj = NULL;
}

TEST(print, check) {
  uint32_t operation = 1;
  KeyTree *KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
                       (&key_obj, &val_obj, operation);
  KeyTree_obj->append_audit_node(cfgptr);

  ConfigNode *cfgnode_obj = new ConfigNode;
  int level_index = 1;
  cfgnode_obj->add_child_to_list(cfgptr);
  cfgnode_obj->print(level_index);
  delete cfgnode_obj;
  delete cfgptr;
  delete KeyTree_obj;
  cfgnode_obj = NULL;
  cfgptr = NULL;
  KeyTree_obj = NULL;
}

TEST(add_child_to_list, DRVAPI_RESPONSE_SUCCESS) {
  ConfigNode *cfgnode_obj = new ConfigNode;
  uint32_t operation = 1;
  KeyTree *KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
                       (&key_obj, &val_obj, operation);
  KeyTree_obj->append_audit_node(cfgptr);

  int ret = cfgnode_obj->add_child_to_list(cfgptr);
  delete cfgptr;
  delete cfgnode_obj;
  delete KeyTree_obj;
  cfgnode_obj = NULL;
  cfgptr = NULL;
  KeyTree_obj = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

TEST(add_child_to_list, DRVAPI_RESPONSE_FAILURE) {
  ConfigNode *cfgnode_obj = new ConfigNode;
  KeyTree *KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  ConfigNode *cfgptr = NULL;

  int ret = cfgnode_obj->add_child_to_list(cfgptr);
  delete cfgptr;
  delete KeyTree_obj;
  delete cfgnode_obj;
  cfgptr = NULL;
  KeyTree_obj = NULL;
  cfgnode_obj = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}

TEST(get_node_list, check) {
  int operation = 1;
  std::vector<ConfigNode*> value_list;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  ConfigNode *cfg_obj = new ConfigNode;

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  key_vbr key1_obj;
  val_vbr val1_obj;
  memcpy(key1_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(key1_obj.vbridge_name, "vbr1", sizeof(key1_obj.vbridge_name));
  memcpy(val1_obj.vbr_description, "vbr1_des",
         sizeof(val1_obj.vbr_description));

  key_vbr_if key2_obj;
  pfcdrv_val_vbr_if val2_obj;
  memcpy(key2_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key2_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key2_obj.vbr_key.vbridge_name, "vbr1",
         sizeof(key2_obj.vbr_key.vbridge_name));
  memcpy(key2_obj.if_name, "vbrif1", sizeof(key2_obj.if_name));
  memcpy(val2_obj.vext_name, "vbrif1_des", sizeof(val2_obj.vext_name));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
                       (&key_obj, &val_obj, operation);
  int ret = cfg_obj->add_child_to_list(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, uint32_t>
                        (&key1_obj, &val1_obj, operation);
  ret = cfg_obj->add_child_to_list(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  ConfigNode *cfgptr2 = new CacheElementUtil<key_vbr_if, pfcdrv_val_vbr_if,
                        uint32_t>(&key2_obj, &val2_obj, operation);
  ret = cfg_obj->add_child_to_list(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  ret = cfg_obj->get_node_list(value_list);
  delete cfg_obj;
  delete cfgptr2;
  delete cfgptr1;
  delete cfgptr;
  delete KeyTree_obj;
  cfg_obj = NULL;
  cfgptr2 = NULL;
  cfgptr1 = NULL;
  cfgptr = NULL;
  KeyTree_obj = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

}  // namespace vtndrvcache
}  // namespace unc
