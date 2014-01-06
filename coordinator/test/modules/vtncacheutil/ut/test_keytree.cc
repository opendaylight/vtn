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
TEST(append_commit_node, vtn) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));

  val_vtn val_obj;
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

TEST(append_commit_node, vbr) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

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

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, 0);

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

TEST(append_commit_node, Reterive_key_val) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

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

  key_vlan_map key2_obj;
  val_vlan_map_t val2_obj;
  memcpy(key2_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key2_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key2_obj.vbr_key.vbridge_name, "vbr1",
         sizeof(key2_obj.vbr_key.vbridge_name));
  memcpy(key2_obj.logical_port_id, "SW-00:00:00:00:00:00:00:01",
         sizeof(key2_obj.logical_port_id));
  key2_obj.logical_port_id_valid = 1;
  val2_obj. vlan_id = 100;

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, operation);
  ret = KeyTree_obj->append_commit_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  ConfigNode *cfgptr2 = new CacheElementUtil<key_vlan_map_t, val_vlan_map_t,
             uint32_t>(&key2_obj, &val2_obj, operation);
  ret = KeyTree_obj->append_commit_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vtn, val_vtn, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vtn, val_vtn, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->vtn_name));
  EXPECT_STREQ(reinterpret_cast<char*>(val_obj.description),
          reinterpret_cast<char*>(tmp_ptr->get_val_structure()->description));

  cfgptr1 = itr_ptr->NextItem();
  CacheElementUtil<key_vbr, val_vbr, uint32_t> *tmp1_ptr =
      static_cast<CacheElementUtil<key_vbr, val_vbr, uint32_t>*> (cfgptr1);

  EXPECT_STREQ(reinterpret_cast<char*>(key1_obj.vbridge_name),
       reinterpret_cast<char*>(tmp1_ptr->get_key_structure()->vbridge_name));
  EXPECT_STREQ(reinterpret_cast<char*>(val1_obj.vbr_description),
      reinterpret_cast<char*>(tmp1_ptr->get_val_structure()->vbr_description));

  cfgptr2 = itr_ptr->NextItem();
  CacheElementUtil<key_vlan_map, val_vlan_map, uint32_t> *tmp2_ptr =
      static_cast<CacheElementUtil<key_vlan_map, val_vlan_map,
      uint32_t>*> (cfgptr2);

  EXPECT_EQ(1, (tmp2_ptr->get_key_structure()->logical_port_id_valid));
  EXPECT_STREQ(reinterpret_cast<char*>(key2_obj.logical_port_id),
      reinterpret_cast<char*>(tmp2_ptr->get_key_structure()->logical_port_id));
  EXPECT_EQ(100, (tmp2_ptr->get_val_structure()->vlan_id));

  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
}

TEST(add_node_to_tree, null) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  ConfigNode *cfgptr = NULL;
  int ret = KeyTree_obj->add_node_to_tree(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}

TEST(add_node_to_tree, null_parent) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  int operation = 1;
  key_vbr key_obj;
  val_vbr val_obj;

  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  memcpy(key_obj.vbridge_name, "vbr1", sizeof(key_obj.vbridge_name));
  memcpy(val_obj.vbr_description, "vbr1_des", sizeof(val_obj.vbr_description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vbr, val_vbr, uint32_t>
      (&key_obj, &val_obj, operation);
  int ret = KeyTree_obj->add_node_to_tree(cfgptr);
  delete KeyTree_obj;
  delete cfgptr;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}

TEST(add_node_to_tree, parent_exist) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  int operation = 1;

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);
  int ret = KeyTree_obj->add_node_to_tree(cfgptr);

  key_vbr key1_obj;
  val_vbr val1_obj;

  memcpy(key1_obj.vtn_key.vtn_name, "vtn1", sizeof(key1_obj.vtn_key.vtn_name));
  memcpy(key1_obj.vbridge_name, "vbr1", sizeof(key1_obj.vbridge_name));
  memcpy(val1_obj.vbr_description,
         "vbr1_des", sizeof(val1_obj.vbr_description));

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, operation);
  ret = KeyTree_obj->add_node_to_tree(cfgptr1);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

TEST(add_child_to_hash, DRVAPI_RESPONSE_SUCCESS) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  int operation = 1;

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);
  int ret = KeyTree_obj->add_child_to_hash(cfgptr);
  delete KeyTree_obj;
  delete cfgptr;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

TEST(get_parenttype, vtn) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  unc_key_type_t keytype = KeyTree_obj->get_parenttype(UNC_KT_VBRIDGE);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VTN);
}

TEST(get_parenttype, root) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  unc_key_type_t keytype = KeyTree_obj->get_parenttype(UNC_KT_VTN);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_ROOT);
}

TEST(append_audit_node, Node_not_exist) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

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
  pfcdrv_val_vbr_if_t val2_obj;
  memcpy(key2_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key2_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key2_obj.vbr_key.vbridge_name, "vbr1",
         sizeof(key2_obj.vbr_key.vbridge_name));
  memcpy(key2_obj.if_name, "vbrif1", sizeof(key2_obj.if_name));
  memcpy(val2_obj.vext_name, "vbrif1_des", sizeof(val2_obj.vext_name));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);

  int ret = KeyTree_obj->append_audit_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, operation);
  ret = KeyTree_obj->append_audit_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  ConfigNode *cfgptr2 = new CacheElementUtil<key_vbr_if, pfcdrv_val_vbr_if_t,
             uint32_t>(&key2_obj, &val2_obj, operation);
  ret = KeyTree_obj->append_audit_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr2 = NULL;
  cfgptr1 = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

TEST(append_audit_node, null) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  ConfigNode *cfgptr = NULL;
  int ret = KeyTree_obj->append_audit_node(cfgptr);
  delete cfgptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}

TEST(append_audit_node, parent_not_exist) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr key1_obj;
  val_vbr val1_obj;

  memcpy(key1_obj.vtn_key.vtn_name, "vtn1", sizeof(key1_obj.vtn_key.vtn_name));
  memcpy(key1_obj.vbridge_name, "vbr1", sizeof(key1_obj.vbridge_name));
  memcpy(val1_obj.vbr_description, "vbr1_des",
         sizeof(val1_obj.vbr_description));

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, operation);
  int ret = KeyTree_obj->append_audit_node(cfgptr1);

  delete cfgptr1;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}

TEST(append_audit_node, parent_exist_already) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_audit_node(cfgptr);

  key_vtn key1_obj;
  val_vtn val1_obj;
  memcpy(key1_obj.vtn_name, "vtn1", sizeof(key1_obj.vtn_name));
  memcpy(val1_obj.description, "vtn1_des", sizeof(val1_obj.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key1_obj, &val1_obj, operation);
  ret = KeyTree_obj->append_audit_node(cfgptr1);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

TEST(append_audit_configuration_list, vector_arg_sucess) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  std::vector<ConfigNode*> vec;
  KeyTree_obj = KeyTree::create_cache();

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

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);
  vec.push_back(cfgptr);

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, operation);
  vec.push_back(cfgptr1);

  int ret = KeyTree_obj->append_audit_configuration_list(vec);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr1 = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

TEST(append_audit_configuration_list, vector_arg_failure) {
  KeyTree* KeyTree_obj;
  std::vector<ConfigNode*> vec;
  KeyTree_obj = KeyTree::create_cache();

  ConfigNode *cfgptr = NULL;
  vec.push_back(cfgptr);

  int ret = KeyTree_obj->append_audit_configuration_list(vec);

  delete cfgptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}

TEST(append_audit_configuration_list, vector_arg_failure1) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  std::vector<ConfigNode*> vec;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr key1_obj;
  val_vbr val1_obj;

  memcpy(key1_obj.vtn_key.vtn_name, "vtn1", sizeof(key1_obj.vtn_key.vtn_name));
  memcpy(key1_obj.vbridge_name, "vbr1", sizeof(key1_obj.vbridge_name));
  memcpy(val1_obj.vbr_description, "vbr1_des",
         sizeof(val1_obj.vbr_description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, operation);
  vec.push_back(cfgptr);

  int ret = KeyTree_obj->append_audit_configuration_list(vec);

  delete cfgptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}

TEST(clear_audit_commit_cache, check) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_commit_node(cfgptr);

  KeyTree_obj->clear_audit_commit_cache();
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

TEST(clear_root_cache, check) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_commit_node(cfgptr);

  KeyTree_obj->clear_root_cache();
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

TEST(get_node_from_hash, get_node) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_audit_node(cfgptr);

  ConfigNode *tmp_ptr = NULL;
  tmp_ptr = KeyTree_obj->get_node_from_hash("vtn1", UNC_KT_VTN);

  uint32_t check_keytype = tmp_ptr->get_type_name();
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(UNC_KT_VTN, check_keytype);
}

TEST(get_node_from_hash, get_null) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);

  int ret = KeyTree_obj->append_audit_node(cfgptr);

  ConfigNode *tmp_ptr = NULL;
  tmp_ptr =  KeyTree_obj->get_node_from_hash("vtn2", UNC_KT_VBRIDGE);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(NULL, tmp_ptr);
}

TEST(get_node_from_hash, get_null1) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  ConfigNode *tmp_ptr = NULL;
  tmp_ptr =  KeyTree_obj->get_node_from_hash("vtn1", UNC_KT_VTN);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(NULL, tmp_ptr);
}

TEST(get_nodelist_keytree, check) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

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
  pfcdrv_val_vbr_if_t val2_obj;
  memcpy(key2_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key2_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key2_obj.vbr_key.vbridge_name, "vbr1",
         sizeof(key2_obj.vbr_key.vbridge_name));
  memcpy(key2_obj.if_name, "vbrif1", sizeof(key2_obj.if_name));
  memcpy(val2_obj.vext_name, "vbrif1_des", sizeof(val2_obj.vext_name));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_audit_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, operation);
  ret = KeyTree_obj->append_audit_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  ConfigNode *cfgptr2 = new CacheElementUtil<key_vbr_if, pfcdrv_val_vbr_if_t,
             uint32_t>(&key2_obj, &val2_obj, operation);
  ret = KeyTree_obj->append_audit_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  ret =  KeyTree_obj->get_nodelist_keytree();

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr2 = NULL;
  cfgptr1 = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}
//  Append switch-port in keytree in bulk operation
TEST(append_physical_attribute_configuration_list, append_single_switchNode) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  std::vector<ConfigNode*> vec_list;
  KeyTree_obj = KeyTree::create_cache();
  //  add single switch 0000-0000-0000-0001 to cache using
  //  append_physical_attribute_configuration_list method
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
                       key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
                                      key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  vec_list.push_back(cfgptr);
  uint32_t ret = KeyTree_obj->append_physical_attribute_configuration_list(
                                                                   vec_list);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}

//  Append switch-port in keytree in bulk operation
TEST(append_physical_attribute_configuration_list, append_multiple_switchNode) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  std::vector<ConfigNode*> vec_list;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache using
  //  append_physical_attribute_configuration_list method

  key_switch key_switch_obj;
  val_switch_st val_switch;
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
                       key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
                                      key_switch_obj.switch_id));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  vec_list.push_back(cfgptr);

  //  add switch 0000-0000-0000-0002 to cache using
  //  append_physical_attribute_configuration_list method

  key_switch key_switch_obj1;
  val_switch_st val_switch1;
  memcpy(key_switch_obj1.ctr_key.controller_name, "odc1", sizeof(
                        key_switch_obj1.ctr_key.controller_name));
  memcpy(key_switch_obj1.switch_id, "0000-0000-0000-0002", sizeof(
                                       key_switch_obj1.switch_id));
  memset(&val_switch1, 0, sizeof(val_switch1));
  ConfigNode *cfgptrone =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1, operation);
  vec_list.push_back(cfgptrone);

  uint32_t ret = KeyTree_obj->append_physical_attribute_configuration_list(
                                                                  vec_list);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptrone = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}
//  Append switch-port in keytree with empty list
TEST(append_physical_attribute_configuration_list, append_empty_list) {
  KeyTree* KeyTree_obj;
  std::vector<ConfigNode*> vec_list;
  KeyTree_obj = KeyTree::create_cache();
  //  add single empty switch listto cache using
  //  append_physical_attribute_configuration_list method
  ConfigNode *cfgptr = NULL;
  vec_list.push_back(cfgptr);
  uint32_t ret = KeyTree_obj->append_physical_attribute_configuration_list(
                                                                  vec_list);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  Append port in keytree with  list ,which parent not found
TEST(append_physical_attribute_configuration_list,
     append_empty_list_parentnotfound) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  std::vector<ConfigNode*> vec_list;
  KeyTree_obj = KeyTree::create_cache();
  //  add single port which parent not found in keytree
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
                        key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
                                      key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth1", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port", sizeof(val_obj.port.description));
  ConfigNode *cfgptr = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  vec_list.push_back(cfgptr);
  uint32_t ret = KeyTree_obj->append_physical_attribute_configuration_list(
                                                                  vec_list);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  std::vector<ConfigNode*>::iterator itr = vec_list.begin();
  std::vector<ConfigNode*>::iterator itr_end = vec_list.end();
  for (; itr != itr_end; ++itr) {
  if (*itr != NULL) {
     delete *itr;
     *itr = NULL;
  }
  }
  vec_list.clear();
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  Append one switch and one port individual to keytree with
//  append_Physical_attribute_node method
TEST(append_Physical_attribute_node, switch_port_success) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0001
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}
//  Append switch/port in keytree with empty
TEST(append_Physical_attribute_node, append_empty_switch) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  //  add single empty switch to  cache using
  //  append_append_Physical_attribute_node method
  ConfigNode *cfgptr = NULL;
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  append port, which parent is not found in cache,return failure
TEST(append_Physical_attribute_node, switch_port_failure) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0002(whcih is not present
  //  in cache)
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0002", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  delete cfgptr1;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  append switch, which is already exist  in cache,ignore the configuration and
//  return success
TEST(append_Physical_attribute_node, switch_port_exist_success) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add another duplicate switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj1;
  val_switch_st val_switch1;
  memcpy(key_switch_obj1.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj1.ctr_key.controller_name));
  memcpy(key_switch_obj1.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj1.switch_id));
  memset(&val_switch1, 0, sizeof(val_switch1));
  ConfigNode *cfgptr1 =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}
//  Append one switch and two port to keytree with
//  append_Physical_attribute_node method and update one port details
TEST(update_physical_attribute_node, update_port) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0001
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-s2-eth3-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s1-eth4 to switch 0000-0000-0000-0001
  key_port key_obj1;
  memcpy(key_obj1.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj1.sw_key.ctr_key.controller_name));
  memcpy(key_obj1.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj1.sw_key.switch_id));
  memcpy(key_obj1.port_id, "s1-eth4", sizeof(key_obj.port_id));

  val_port_st val_obj1;
  memset(&val_obj1, 0, sizeof(val_obj1));
  memcpy(val_obj1.port.description, "port-s1-eth4-description", sizeof(
          val_obj1.port.description));
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  //  update port s2-eth3
  key_port key_obj2;
  memcpy(key_obj2.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj2.sw_key.ctr_key.controller_name));
  memcpy(key_obj2.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj2.sw_key.switch_id));
  memcpy(key_obj2.port_id, "s1-eth4", sizeof(key_obj.port_id));

  val_port_st val_obj2;
  memset(&val_obj2, 0, sizeof(val_obj2));
  memcpy(val_obj2.port.description,
         "port-s1-eth4-description,change to new description",
         sizeof(val_obj2.port.description));
  ConfigNode *cfgptr3= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj2, &val_obj2, operation);

  ret = KeyTree_obj->update_physical_attribute_node(cfgptr3);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}
//  Append one switch and two port to keytree with
//  append_Physical_attribute_node method and update switch detail
TEST(update_physical_attribute_node, update_switch) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(val_switch.manufacturer, "switch-NEC-manufacturer", sizeof(
          val_switch.manufacturer));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0001
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-s2-eth3-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s1-eth4 to switch 0000-0000-0000-0001
  key_port key_obj1;
  memcpy(key_obj1.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj1.sw_key.ctr_key.controller_name));
  memcpy(key_obj1.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj1.sw_key.switch_id));
  memcpy(key_obj1.port_id, "s1-eth4", sizeof(key_obj.port_id));

  val_port_st val_obj1;
  memset(&val_obj1, 0, sizeof(val_obj1));
  memcpy(val_obj1.port.description, "port-s1-eth4-description", sizeof(
          val_obj1.port.description));
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  //  update switch 0000-0000-0000-0001
  key_switch key_switch_obj1;
  val_switch_st val_switch1;
  memcpy(key_switch_obj1.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj1.ctr_key.controller_name));
  memcpy(key_switch_obj1.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj1.switch_id));
  memset(&val_switch1, 0, sizeof(val_switch1));
  memcpy(val_switch1.manufacturer, "switch-cicso-manufacturer", sizeof(
          val_switch1.manufacturer));
  ConfigNode *cfgptr3 =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1, operation);

  ret = KeyTree_obj->update_physical_attribute_node(cfgptr3);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}
//  Update switch/port in keytree with empty
TEST(update_physical_attribute_node, append_empty_switch) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  //  add single empty switch to  cache using
  //  append_append_Physical_attribute_node method
  ConfigNode *cfgptr = NULL;
  uint32_t ret = KeyTree_obj->update_physical_attribute_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  Append one switch and two port to keytree with
//  append_Physical_attribute_node method and update  switch fail
TEST(update_physical_attribute_node, update_fail) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(val_switch.manufacturer, "switch-NEC-manufacturer", sizeof(
          val_switch.manufacturer));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0001
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-s2-eth3-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s1-eth4 to switch 0000-0000-0000-0001
  key_port key_obj1;
  memcpy(key_obj1.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj1.sw_key.ctr_key.controller_name));
  memcpy(key_obj1.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj1.sw_key.switch_id));
  memcpy(key_obj1.port_id, "s1-eth4", sizeof(key_obj.port_id));

  val_port_st val_obj1;
  memset(&val_obj1, 0, sizeof(val_obj1));
  memcpy(val_obj1.port.description, "port-s1-eth4-description", sizeof(
          val_obj1.port.description));
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  //  update switch 0000-0000-0000-0002(different switch,which not available in
  //  keytree)
  key_switch key_switch_obj1;
  val_switch_st val_switch1;
  memcpy(key_switch_obj1.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj1.ctr_key.controller_name));
  memcpy(key_switch_obj1.switch_id, "0000-0000-0000-0002", sizeof(
          key_switch_obj1.switch_id));
  memset(&val_switch1, 0, sizeof(val_switch1));
  memcpy(val_switch1.manufacturer, "switch-cicso-manufacturer", sizeof(
          val_switch1.manufacturer));
  ConfigNode *cfgptr3 =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1, operation);

  ret = KeyTree_obj->update_physical_attribute_node(cfgptr3);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  Append one switch and two port to keytree with
//  append_Physical_attribute_node method and update any key_type other than
//  switch and port
TEST(update_physical_attribute_node, update_fail_otherkey_type) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(val_switch.manufacturer, "switch-NEC-manufacturer", sizeof(
          val_switch.manufacturer));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0001
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-s2-eth3-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s1-eth4 to switch 0000-0000-0000-0001
  key_port key_obj1;
  memcpy(key_obj1.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj1.sw_key.ctr_key.controller_name));
  memcpy(key_obj1.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj1.sw_key.switch_id));
  memcpy(key_obj1.port_id, "s1-eth4", sizeof(key_obj.port_id));

  val_port_st val_obj1;
  memset(&val_obj1, 0, sizeof(val_obj1));
  memcpy(val_obj1.port.description, "port-s1-eth4-description", sizeof(
          val_obj1.port.description));
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  append vtn into keytree
  key_vtn vtn_key;
  memset(&vtn_key, 0, sizeof(vtn_key));
  memcpy(vtn_key.vtn_name, "vtn1", sizeof(vtn_key.vtn_name));

  val_vtn vtn_val;
  memset(&vtn_val, 0, sizeof(vtn_val));
  memcpy(vtn_val.description, "vtn1_des", sizeof(vtn_val.description));

  ConfigNode *cfgptr3 = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&vtn_key, &vtn_val, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr3);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  update other key_type than kt_switch,kt_port
  key_vtn vtn_key1;
  memset(&vtn_key1, 0, sizeof(vtn_key1));
  memcpy(vtn_key1.vtn_name, "vtn1", sizeof(vtn_key1.vtn_name));

  val_vtn vtn_val1;
  memset(&vtn_val1, 0, sizeof(vtn_val1));
  memcpy(vtn_val1.description, "vtn2_des", sizeof(vtn_val1.description));

  ConfigNode *cfgptr4 = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&vtn_key1, &vtn_val1, operation);
  ret = KeyTree_obj->update_physical_attribute_node(cfgptr4);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  cfgptr3 = NULL;
  delete cfgptr4;
  cfgptr4 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  Delete switch/port in keytree with empty
TEST(delete_physical_attribute_node, delete_empty_switch) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  //  delete single empty switch to  cache using
  //  delete_physical_attribute_node method
  ConfigNode *cfgptr = NULL;
  uint32_t ret = KeyTree_obj->delete_physical_attribute_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  Try to delete a non-existing port,which parent not available in keytree
TEST(delete_physical_attribute_node, parent_not_present_fail) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(val_switch.manufacturer, "switch-NEC-manufacturer", sizeof(
          val_switch.manufacturer));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0001
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-s2-eth3-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s1-eth4 to switch 0000-0000-0000-0001
  key_port key_obj1;
  memcpy(key_obj1.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj1.sw_key.ctr_key.controller_name));
  memcpy(key_obj1.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj1.sw_key.switch_id));
  memcpy(key_obj1.port_id, "s1-eth4", sizeof(key_obj1.port_id));

  val_port_st val_obj1;
  memset(&val_obj1, 0, sizeof(val_obj1));
  memcpy(val_obj1.port.description, "port-s1-eth4-description", sizeof(
          val_obj1.port.description));
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  delete port which is not available in keytree
  key_port key_obj2;
  memcpy(key_obj2.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj2.sw_key.ctr_key.controller_name));
  memcpy(key_obj2.sw_key.switch_id, "0000-0000-0000-0002", sizeof(
          key_obj2.sw_key.switch_id));
  memcpy(key_obj2.port_id, "s1-eth8", sizeof(key_obj2.port_id));

  val_port_st val_obj2;
  memset(&val_obj2, 0, sizeof(val_obj2));
  memcpy(val_obj2.port.description, "port-s1-eth8-description", sizeof(
          val_obj2.port.description));
  ConfigNode *cfgptr3= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj2, &val_obj2, operation);

  ret = KeyTree_obj->delete_physical_attribute_node(cfgptr3);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  Try to delete a non-existing port,which parent not available in keytree
TEST(delete_physical_attribute_node, child_not_present_fail) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(val_switch.manufacturer, "switch-NEC-manufacturer", sizeof(
          val_switch.manufacturer));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0001
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-s2-eth3-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s1-eth4 to switch 0000-0000-0000-0001
  key_port key_obj1;
  memcpy(key_obj1.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj1.sw_key.ctr_key.controller_name));
  memcpy(key_obj1.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj1.sw_key.switch_id));
  memcpy(key_obj1.port_id, "s1-eth4", sizeof(key_obj1.port_id));

  val_port_st val_obj1;
  memset(&val_obj1, 0, sizeof(val_obj1));
  memcpy(val_obj1.port.description, "port-s1-eth4-description", sizeof(
          val_obj1.port.description));
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  delete port which is not available in keytree
  key_port key_obj2;
  memcpy(key_obj2.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj2.sw_key.ctr_key.controller_name));
  memcpy(key_obj2.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj2.sw_key.switch_id));
  memcpy(key_obj2.port_id, "s1-eth8", sizeof(key_obj2.port_id));

  val_port_st val_obj2;
  memset(&val_obj2, 0, sizeof(val_obj2));
  memcpy(val_obj2.port.description, "port-s1-eth8-description", sizeof(
          val_obj2.port.description));
  ConfigNode *cfgptr3= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj2, &val_obj2, operation);

  ret = KeyTree_obj->delete_physical_attribute_node(cfgptr3);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  Delete a existing port.
TEST(delete_physical_attribute_node, delete_port) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(val_switch.manufacturer, "switch-NEC-manufacturer", sizeof(
          val_switch.manufacturer));
  ConfigNode *cfgptr = new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0001
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-s2-eth3-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s1-eth4 to switch 0000-0000-0000-0001
  key_port key_obj1;
  memcpy(key_obj1.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj1.sw_key.ctr_key.controller_name));
  memcpy(key_obj1.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj1.sw_key.switch_id));
  memcpy(key_obj1.port_id, "s1-eth4", sizeof(key_obj1.port_id));

  val_port_st val_obj1;
  memset(&val_obj1, 0, sizeof(val_obj1));
  memcpy(val_obj1.port.description, "port-s1-eth4-description", sizeof(
          val_obj1.port.description));
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  ConfigNode* cfgnode;
  for (cfgnode = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode = itr_ptr->NextItem() ) {
    pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 cfgnode->get_key_name().c_str(), cfgnode->get_type_name());
  }
  delete itr_ptr;
  //  delete existing port "s1-eth4"
  key_port key_obj2;
  memcpy(key_obj2.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj2.sw_key.ctr_key.controller_name));
  memcpy(key_obj2.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj2.sw_key.switch_id));
  memcpy(key_obj2.port_id, "s1-eth4", sizeof(key_obj2.port_id));

  val_port_st val_obj2;
  memset(&val_obj2, 0, sizeof(val_obj2));
  ConfigNode *cfgptr3= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj2, &val_obj2, operation);

  ret = KeyTree_obj->delete_physical_attribute_node(cfgptr3);
  itr_ptr = KeyTree_obj->create_iterator();
  ConfigNode* cfgnode_iter;
  for (cfgnode_iter = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_iter = itr_ptr->NextItem() ) {
    pfc_log_info(
        "Node Present in Tree After delete portis1-eth4 ..for:%s keytype %d",
        cfgnode_iter->get_key_name().c_str(), cfgnode_iter->get_type_name());
  }
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}
//  Delete a existing switch having two-port configuration.
TEST(delete_physical_attribute_node, delete_switch) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(val_switch.manufacturer, "switch-NEC-manufacturer", sizeof(
          val_switch.manufacturer));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add switch 0000-0000-0000-0002 to cache
  key_switch key_switch_obj1;
  val_switch_st val_switch1;
  memcpy(key_switch_obj1.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj1.ctr_key.controller_name));
  memcpy(key_switch_obj1.switch_id, "0000-0000-0000-0002", sizeof(
          key_switch_obj1.switch_id));
  memset(&val_switch1, 0, sizeof(val_switch1));
  memcpy(val_switch1.manufacturer, "switch-CICSO-manufacturer", sizeof(
          val_switch1.manufacturer));
  ConfigNode *cfgptr_sw2 =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_sw2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s1-eth1 to switch 0000-0000-0000-0002
  key_port key_obj_sw2;
  memcpy(key_obj_sw2.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj_sw2.sw_key.ctr_key.controller_name));
  memcpy(key_obj_sw2.sw_key.switch_id, "0000-0000-0000-0002", sizeof(
          key_obj_sw2.sw_key.switch_id));
  memcpy(key_obj_sw2.port_id, "s1-eth1", sizeof(key_obj_sw2.port_id));

  val_port_st val_obj_sw2;
  memset(&val_obj_sw2, 0, sizeof(val_obj_sw2));
  ConfigNode *cfgptr_sw2_port1 =
      new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj_sw2, &val_obj_sw2, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_sw2_port1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0001
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-s2-eth3-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s1-eth4 to switch 0000-0000-0000-0001
  key_port key_obj1;
  memcpy(key_obj1.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj1.sw_key.ctr_key.controller_name));
  memcpy(key_obj1.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj1.sw_key.switch_id));
  memcpy(key_obj1.port_id, "s1-eth4", sizeof(key_obj1.port_id));

  val_port_st val_obj1;
  memset(&val_obj1, 0, sizeof(val_obj1));
  memcpy(val_obj1.port.description, "port-s1-eth4-description", sizeof(
          val_obj1.port.description));
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  ConfigNode* cfgnode;
  for (cfgnode = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode = itr_ptr->NextItem() ) {
    pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 cfgnode->get_key_name().c_str(), cfgnode->get_type_name());
  }
  delete itr_ptr;
  //  delete existing switch 0000-0000-0000-0001 having two port configuration
  key_switch key_switch_obj2;
  val_switch_st val_switch2;
  memcpy(key_switch_obj2.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj2.ctr_key.controller_name));
  memcpy(key_switch_obj2.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj2.switch_id));
  memset(&val_switch2, 0, sizeof(val_switch2));
  memcpy(val_switch1.manufacturer, "switch-NEC-manufacturer", sizeof(
          val_switch1.manufacturer));
  ConfigNode *cfgptr3 =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj2, &val_switch2, operation);

  ret = KeyTree_obj->delete_physical_attribute_node(cfgptr3);
  itr_ptr = KeyTree_obj->create_iterator();
  ConfigNode* cfgnode_iter;
  for (cfgnode_iter = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_iter = itr_ptr->NextItem() ) {
    pfc_log_info(
        "Node Present in Tree After delete switch-02 ..for:%s keytype %d",
        cfgnode_iter->get_key_name().c_str(), cfgnode_iter->get_type_name());
  }
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  cfgptr_sw2 = NULL;
  cfgptr_sw2_port1 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
}
//  delete failed other than switch/port
TEST(delete_physical_attribute_node, delete_fail_otherkey_type) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(val_switch.manufacturer, "switch-NEC-manufacturer", sizeof(
          val_switch.manufacturer));
  ConfigNode *cfgptr =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s2-eth3 to switch 0000-0000-0000-0001
  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(val_obj.port.description, "port-s2-eth3-description", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  //  add port s1-eth4 to switch 0000-0000-0000-0001
  key_port key_obj1;
  memcpy(key_obj1.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj1.sw_key.ctr_key.controller_name));
  memcpy(key_obj1.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj1.sw_key.switch_id));
  memcpy(key_obj1.port_id, "s1-eth4", sizeof(key_obj.port_id));

  val_port_st val_obj1;
  memset(&val_obj1, 0, sizeof(val_obj1));
  memcpy(val_obj1.port.description, "port-s1-eth4-description", sizeof(
          val_obj1.port.description));
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  //  append vtn into keytree

  key_vtn vtn_key;
  memset(&vtn_key, 0, sizeof(vtn_key));
  memcpy(vtn_key.vtn_name, "vtn1", sizeof(vtn_key.vtn_name));

  val_vtn vtn_val;
  memset(&vtn_val, 0, sizeof(vtn_val));
  memcpy(vtn_val.description, "vtn1_des", sizeof(vtn_val.description));

  ConfigNode *cfgptr3 = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&vtn_key, &vtn_val, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr3);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);

  //  delete other key_type than kt_switch,kt_port

  key_vtn vtn_key1;
  memset(&vtn_key1, 0, sizeof(vtn_key1));
  memcpy(vtn_key1.vtn_name, "vtn1", sizeof(vtn_key1.vtn_name));

  val_vtn vtn_val1;
  memset(&vtn_val1, 0, sizeof(vtn_val1));
  memcpy(vtn_val1.description, "vtn2_des", sizeof(vtn_val1.description));

  ConfigNode *cfgptr4 = new CacheElementUtil<key_vtn, val_vtn, uint32_t>
      (&vtn_key1, &vtn_val1, operation);
  ret = KeyTree_obj->delete_physical_attribute_node(cfgptr4);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  cfgptr3 = NULL;
  delete cfgptr4;
  cfgptr4 = NULL;
  EXPECT_EQ(ret, DRVAPI_RESPONSE_FAILURE);
}
//  Try to find empty switch/port from keytree
TEST(compare_is_physical_node_found, empty_switch_port_fail) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  ConfigNode *new_compare_node = NULL;
  ConfigNode *old_node_for_update = NULL;
  pfc_bool_t check = false;
  check = KeyTree_obj->compare_is_physical_node_found(new_compare_node,
                                                      old_node_for_update);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(check, false);
}
//  Try to find empty switch/port from keytree
TEST(compare_is_physical_node_found, switch_port_not_present_fail) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  ConfigNode *new_compare_node =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  ConfigNode *old_node_for_update = NULL;
  pfc_bool_t check = false;
  check = KeyTree_obj->compare_is_physical_node_found(new_compare_node,
                                                      old_node_for_update);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  delete new_compare_node;
  new_compare_node = NULL;
  EXPECT_EQ(check, false);
}
//  Try to find empty switch/port from keytree
TEST(compare_is_physical_node_found, switch_port_not_present_pass) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  val_switch.oper_status = 1;
  memcpy(val_switch.manufacturer, "Nicira Networks, Inc.", sizeof(
          val_switch.manufacturer));
  memcpy(val_switch.hardware, "Open vSwitch", sizeof(val_switch.hardware));
  memcpy(val_switch.software, "1.4.3", sizeof(val_switch.software));
  val_switch.alarms_status = 0;
  memcpy(val_switch.switch_val.description, "Openflow Switch", sizeof(
          val_switch.switch_val.description));
  memcpy(val_switch.switch_val.model, "OFS", sizeof(
          val_switch.switch_val.model));
  memcpy(val_switch.switch_val.domain_name, "DEFAULT", sizeof(
          val_switch.switch_val.domain_name));
  ConfigNode *cfgptr = new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, DRVAPI_RESPONSE_SUCCESS);
  key_switch key_switch_obj1;
  val_switch_st val_switch1;
  memcpy(key_switch_obj1.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj1.ctr_key.controller_name));
  memcpy(key_switch_obj1.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj1.switch_id));
  memset(&val_switch1, 0, sizeof(val_switch1));
  ConfigNode *new_compare_node =
      new CacheElementUtil<key_switch, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1, operation);

  ConfigNode *old_node_for_update = NULL;
  pfc_bool_t check = false;
  check = KeyTree_obj->compare_is_physical_node_found(new_compare_node,
                                                      old_node_for_update);
  CacheElementUtil<key_switch, val_switch_st, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_switch, val_switch_st, uint32_t>*> (
          old_node_for_update);
  pfc_log_info("manufacturer= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->manufacturer));
  pfc_log_info("oper_status= %d", tmp_ptr->get_val_structure()->oper_status);
  pfc_log_info("hardware= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->hardware));
  pfc_log_info("software= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->software));
  pfc_log_info("alarms_status= %lu", tmp_ptr->get_val_structure()->
               alarms_status);
  pfc_log_info("description= %s", reinterpret_cast<char*>(tmp_ptr->
                                 get_val_structure()->switch_val.description));
  pfc_log_info("model= %s", reinterpret_cast<char*>(tmp_ptr->
                                 get_val_structure()->switch_val.model));
  pfc_log_info("admin_status= %d", tmp_ptr->get_val_structure()->
               switch_val.admin_status);
  pfc_log_info("domain_name= %s", reinterpret_cast<char*>(tmp_ptr->
                                 get_val_structure()->switch_val.domain_name));
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  delete new_compare_node;
  new_compare_node = NULL;
  cfgptr = NULL;
  EXPECT_EQ(check, true);
}
}  // namespace vtndrvcache
}  // namespace unc
