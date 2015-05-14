/*
 * Copyright (c) 2013-2014 NEC Corporation
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

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj,operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, 0);

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, &val1_obj, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  pfcdrv_val_vlan_map_t val2_obj;
  memcpy(key2_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key2_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key2_obj.vbr_key.vbridge_name, "vbr1",
         sizeof(key2_obj.vbr_key.vbridge_name));
  memcpy(key2_obj.logical_port_id, "SW-00:00:00:00:00:00:00:01",
         sizeof(key2_obj.logical_port_id));
  key2_obj.logical_port_id_valid = 1;
  val2_obj.vm.vlan_id = 100;

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, &val1_obj, operation);
  ret = KeyTree_obj->append_commit_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  ConfigNode *cfgptr2 = new CacheElementUtil<key_vlan_map_t,
             pfcdrv_val_vlan_map_t,
             pfcdrv_val_vlan_map_t,
             uint32_t>(&key2_obj, &val2_obj, &val2_obj, operation);
  ret = KeyTree_obj->append_commit_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->vtn_name));
  EXPECT_STREQ(reinterpret_cast<char*>(val_obj.description),
          reinterpret_cast<char*>(tmp_ptr->get_val_structure()->description));

  cfgptr1 = itr_ptr->NextItem();
  CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t> *tmp1_ptr =
      static_cast<CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t>*> (cfgptr1);

  EXPECT_STREQ(reinterpret_cast<char*>(key1_obj.vbridge_name),
       reinterpret_cast<char*>(tmp1_ptr->get_key_structure()->vbridge_name));
  EXPECT_STREQ(reinterpret_cast<char*>(val1_obj.vbr_description),
      reinterpret_cast<char*>(tmp1_ptr->get_val_structure()->vbr_description));

  cfgptr2 = itr_ptr->NextItem();
  CacheElementUtil<key_vlan_map, pfcdrv_val_vlan_map_t, pfcdrv_val_vlan_map_t,
                          uint32_t> *tmp2_ptr =
      static_cast<CacheElementUtil<key_vlan_map, pfcdrv_val_vlan_map_t,
                 pfcdrv_val_vlan_map_t, uint32_t>*> (cfgptr2);

  EXPECT_EQ(1, (tmp2_ptr->get_key_structure()->logical_port_id_valid));
  EXPECT_STREQ(reinterpret_cast<char*>(key2_obj.logical_port_id),
      reinterpret_cast<char*>(tmp2_ptr->get_key_structure()->logical_port_id));
  EXPECT_EQ(100, (tmp2_ptr->get_val_structure()->vm.vlan_id));

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
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(add_node_to_tree, null_parent) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  int operation = 1;
  key_vbr key_obj;
  val_vbr val1_obj;
  val_vbr val2_obj;
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  memcpy(key_obj.vbridge_name, "vbr1", sizeof(key_obj.vbridge_name));
  memcpy(val1_obj.vbr_description, "vbr1_des", sizeof(val1_obj.vbr_description));
  memcpy(val2_obj.vbr_description, "vbr1_des", sizeof(val2_obj.vbr_description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t>
      (&key_obj, &val1_obj, &val2_obj, operation);
  int ret = KeyTree_obj->add_node_to_tree(cfgptr);
  delete KeyTree_obj;
  delete cfgptr;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(add_node_to_tree, parent_exist) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  int operation = 1;

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->add_node_to_tree(cfgptr);

  key_vbr key1_obj;
  val_vbr val1_obj;

  memcpy(key1_obj.vtn_key.vtn_name, "vtn1", sizeof(key1_obj.vtn_key.vtn_name));
  memcpy(key1_obj.vbridge_name, "vbr1", sizeof(key1_obj.vbridge_name));
  memcpy(val1_obj.vbr_description,
         "vbr1_des", sizeof(val1_obj.vbr_description));

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj,  &val1_obj, operation);
  ret = KeyTree_obj->add_node_to_tree(cfgptr1);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, UNC_RC_SUCCESS) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  int operation = 1;

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->add_child_to_hash(cfgptr);
  delete KeyTree_obj;
  delete cfgptr;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  int ret = KeyTree_obj->append_audit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, &val1_obj,  operation);
  ret = KeyTree_obj->append_audit_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  ConfigNode *cfgptr2 = new CacheElementUtil<key_vbr_if,
                   pfcdrv_val_vbr_if_t,  pfcdrv_val_vbr_if_t,
             uint32_t>(&key2_obj, &val2_obj, &val2_obj, operation);
  ret = KeyTree_obj->append_audit_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr2 = NULL;
  cfgptr1 = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
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

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, &val1_obj, operation);
  int ret = KeyTree_obj->append_audit_node(cfgptr1);

  delete cfgptr1;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(append_audit_node, parent_exist_already) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_audit_node(cfgptr);

  key_vtn key1_obj;
  val_vtn val1_obj;
  val_vtn val2_obj;
  memcpy(key1_obj.vtn_name, "vtn1", sizeof(key1_obj.vtn_name));
  memcpy(val1_obj.description, "vtn1_des", sizeof(val1_obj.description));
  memcpy(val2_obj.description, "vtn1_des", sizeof(val2_obj.description));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key1_obj, &val1_obj,  &val2_obj, operation);
  ret = KeyTree_obj->append_audit_node(cfgptr1);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  val_vbr val2_obj;

  memcpy(key1_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(key1_obj.vbridge_name, "vbr1", sizeof(key1_obj.vbridge_name));
  memcpy(val1_obj.vbr_description, "vbr1_des",
         sizeof(val1_obj.vbr_description));
  memcpy(val2_obj.vbr_description, "vbr1_des",
          sizeof(val2_obj.vbr_description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  vec.push_back(cfgptr);

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, &val2_obj, operation);
  vec.push_back(cfgptr1);

  int ret = KeyTree_obj->append_audit_configuration_list(vec);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr1 = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(append_audit_configuration_list, vector_arg_failure1) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  std::vector<ConfigNode*> vec;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr key1_obj;
  val_vbr val1_obj;
  val_vbr val2_obj;
  memcpy(key1_obj.vtn_key.vtn_name, "vtn1", sizeof(key1_obj.vtn_key.vtn_name));
  memcpy(key1_obj.vbridge_name, "vbr1", sizeof(key1_obj.vbridge_name));
  memcpy(val1_obj.vbr_description, "vbr1_des",
         sizeof(val1_obj.vbr_description));
  memcpy(val2_obj.vbr_description, "vbr1_des",
         sizeof(val2_obj.vbr_description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, &val2_obj, operation);
  vec.push_back(cfgptr);

  int ret = KeyTree_obj->append_audit_configuration_list(vec);

  delete cfgptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(clear_audit_commit_cache, check) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_commit_node(cfgptr);

  KeyTree_obj->clear_audit_commit_cache();
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(clear_root_cache, check) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_commit_node(cfgptr);

  KeyTree_obj->clear_root_cache();
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(get_node_from_hash, get_node) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
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

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  int ret = KeyTree_obj->append_audit_node(cfgptr);

  ConfigNode *tmp_ptr = NULL;
  ConfigNode *compare_ptr = NULL;
  tmp_ptr =  KeyTree_obj->get_node_from_hash("vtn2", UNC_KT_VBRIDGE);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(compare_ptr, tmp_ptr);
}

TEST(get_node_from_hash, get_null1) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  ConfigNode *tmp_ptr = NULL;
  ConfigNode *compare_ptr = NULL;
  tmp_ptr =  KeyTree_obj->get_node_from_hash("vtn1", UNC_KT_VTN);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(compare_ptr, tmp_ptr);
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
  val_vbr val3_obj;
  memcpy(key1_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(key1_obj.vbridge_name, "vbr1", sizeof(key1_obj.vbridge_name));
  memcpy(val1_obj.vbr_description, "vbr1_des",
         sizeof(val1_obj.vbr_description));
  memcpy(val3_obj.vbr_description, "vbr1_des",
         sizeof(val3_obj.vbr_description));
  key_vbr_if key2_obj;
  pfcdrv_val_vbr_if_t val2_obj;
  memcpy(key2_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key2_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key2_obj.vbr_key.vbridge_name, "vbr1",
         sizeof(key2_obj.vbr_key.vbridge_name));
  memcpy(key2_obj.if_name, "vbrif1", sizeof(key2_obj.if_name));
  memcpy(val2_obj.vext_name, "vbrif1_des", sizeof(val2_obj.vext_name));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->append_audit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, val_vbr, uint32_t>
      (&key1_obj, &val1_obj, &val1_obj, operation);
  ret = KeyTree_obj->append_audit_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  ConfigNode *cfgptr2 = new CacheElementUtil<key_vbr_if, pfcdrv_val_vbr_if_t,
             pfcdrv_val_vbr_if_t, uint32_t>(&key2_obj, &val2_obj, &val2_obj, operation);
  ret = KeyTree_obj->append_audit_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  ret =  KeyTree_obj->get_nodelist_keytree();

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr2 = NULL;
  cfgptr1 = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  vec_list.push_back(cfgptr);
  uint32_t ret = KeyTree_obj->append_physical_attribute_configuration_list(
                                                                   vec_list);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1, &val_switch1, operation);
  vec_list.push_back(cfgptrone);

  uint32_t ret = KeyTree_obj->append_physical_attribute_configuration_list(
                                                                  vec_list);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptrone = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
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
  ConfigNode *cfgptr = new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

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
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}
//  Append one switch and one port individual to keytree with
//  append_Physical_attribute_node method
TEST(append_physical_attribute_node, switch_port_success) {
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st,
              uint32_t>(&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}
//  Append switch/port in keytree with empty
TEST(append_physical_attribute_node, append_empty_switch) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  //  add single empty switch to  cache using
  //  append_append_Physical_attribute_node method
  ConfigNode *cfgptr = NULL;
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}
//  append port, which parent is not found in cache,return failure
TEST(append_physical_attribute_node, switch_port_failure) {
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  delete cfgptr1;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}
//  append switch, which is already exist  in cache,ignore the configuration and
//  return success
TEST(append_physical_attribute_node, switch_port_exist_success) {
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add another duplicate switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj1;
  val_switch_st val_switch1;
  memcpy(key_switch_obj1.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj1.ctr_key.controller_name));
  memcpy(key_switch_obj1.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj1.switch_id));
  memset(&val_switch1, 0, sizeof(val_switch1));
  ConfigNode *cfgptr1 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1, &val_switch1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st,
                    uint32_t>(&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, val_port_st,
                 uint32_t>(&key_obj1, &val_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

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
  ConfigNode *cfgptr3= new CacheElementUtil<key_port, val_port_st, val_port_st,
             uint32_t>(&key_obj2, &val_obj2, &val_obj2, operation);

  ret = KeyTree_obj->update_physical_attribute_node(cfgptr3);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
                   (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st,
                  uint32_t>(&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1,  &val_switch1, operation);

  ret = KeyTree_obj->update_physical_attribute_node(cfgptr3);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st,
          val_port_st, uint32_t>(&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1, &val_switch1, operation);

  ret = KeyTree_obj->update_physical_attribute_node(cfgptr3);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st,
                uint32_t>(&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj1, &val_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  append vtn into keytree
  key_vtn vtn_key;
  memset(&vtn_key, 0, sizeof(vtn_key));
  memcpy(vtn_key.vtn_name, "vtn1", sizeof(vtn_key.vtn_name));

  val_vtn vtn_val;
  memset(&vtn_val, 0, sizeof(vtn_val));
  memcpy(vtn_val.description, "vtn1_des", sizeof(vtn_val.description));

  ConfigNode *cfgptr3 = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&vtn_key, &vtn_val, &vtn_val, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr3);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  update other key_type than kt_switch,kt_port
  key_vtn vtn_key1;
  memset(&vtn_key1, 0, sizeof(vtn_key1));
  memcpy(vtn_key1.vtn_name, "vtn1", sizeof(vtn_key1.vtn_name));

  val_vtn vtn_val1;
  memset(&vtn_val1, 0, sizeof(vtn_val1));
  memcpy(vtn_val1.description, "vtn2_des", sizeof(vtn_val1.description));

  ConfigNode *cfgptr4 = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&vtn_key1, &vtn_val1, &vtn_val1, operation);
  ret = KeyTree_obj->update_physical_attribute_node(cfgptr4);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  cfgptr3 = NULL;
  delete cfgptr4;
  cfgptr4 = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
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
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st,
                       uint32_t>(&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, val_port_st,
      uint32_t>(&key_obj1, &val_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr3= new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj2, &val_obj2, &val_obj2, operation);

  ret = KeyTree_obj->delete_physical_attribute_node(cfgptr3);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st,
                 uint32_t>(&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, val_port_st,
             uint32_t>(&key_obj1, &val_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr3= new CacheElementUtil<key_port, val_port_st, val_port_st,
                uint32_t>(&key_obj2, &val_obj2, &val_obj2, operation);

  ret = KeyTree_obj->delete_physical_attribute_node(cfgptr3);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  delete cfgptr3;
  cfgptr3 = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
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
  ConfigNode *cfgptr = new CacheElementUtil<key_switch, val_switch_st,
             val_switch_st, uint32_t>(&key_switch_obj, &val_switch,
                                      &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, val_port_st,
                    uint32_t>(&key_obj1, &val_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr3= new CacheElementUtil<key_port, val_port_st, val_port_st,
                      uint32_t>(&key_obj2, &val_obj2, &val_obj2, operation);

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
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1, &val_switch1, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_sw2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj_sw2, &val_obj_sw2, &val_obj_sw2, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_sw2_port1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st,
              uint32_t>(&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st, val_port_st,
             uint32_t>(&key_obj1, &val_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st,
            uint32_t>(&key_switch_obj2, &val_switch2, &val_switch2, operation);

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
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st,
          uint32_t>(&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
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
  ConfigNode *cfgptr2= new CacheElementUtil<key_port, val_port_st,
        val_port_st, uint32_t>(&key_obj1, &val_obj1, &val_obj1, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  //  append vtn into keytree

  key_vtn vtn_key;
  memset(&vtn_key, 0, sizeof(vtn_key));
  memcpy(vtn_key.vtn_name, "vtn1", sizeof(vtn_key.vtn_name));

  val_vtn vtn_val;
  memset(&vtn_val, 0, sizeof(vtn_val));
  memcpy(vtn_val.description, "vtn1_des", sizeof(vtn_val.description));

  ConfigNode *cfgptr3 = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&vtn_key, &vtn_val, &vtn_val, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr3);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  //  delete other key_type than kt_switch,kt_port

  key_vtn vtn_key1;
  memset(&vtn_key1, 0, sizeof(vtn_key1));
  memcpy(vtn_key1.vtn_name, "vtn1", sizeof(vtn_key1.vtn_name));

  val_vtn vtn_val1;
  memset(&vtn_val1, 0, sizeof(vtn_val1));
  memcpy(vtn_val1.description, "vtn2_des", sizeof(vtn_val1.description));

  ConfigNode *cfgptr4 = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&vtn_key1, &vtn_val1, &vtn_val1, operation);
  ret = KeyTree_obj->delete_physical_attribute_node(cfgptr4);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  cfgptr2 = NULL;
  cfgptr3 = NULL;
  delete cfgptr4;
  cfgptr4 = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
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
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
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
  ConfigNode *cfgptr = new CacheElementUtil<key_switch, val_switch_st,
                               val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_switch key_switch_obj1;
  val_switch_st val_switch1;
  memcpy(key_switch_obj1.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj1.ctr_key.controller_name));
  memcpy(key_switch_obj1.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj1.switch_id));
  memset(&val_switch1, 0, sizeof(val_switch1));
  ConfigNode *new_compare_node =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj1, &val_switch1,  &val_switch1, operation);

  ConfigNode *old_node_for_update = NULL;
  pfc_bool_t check = false;
  check = KeyTree_obj->compare_is_physical_node_found(new_compare_node,
                                                      old_node_for_update);
  CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>*> (
          old_node_for_update);
  pfc_log_info("manufacturer= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->manufacturer));
  pfc_log_info("oper_status= %d", tmp_ptr->get_val_structure()->oper_status);
  pfc_log_info("hardware= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->hardware));
  pfc_log_info("software= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->software));
  pfc_log_info("alarms_status= %" PFC_PFMT_u64, tmp_ptr->get_val_structure()->
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
  EXPECT_EQ(PFC_TRUE, check);
}
//  Append twoswitches and two ports  to keytree with
//  append_Physical_attribute_node method and create one link between them
TEST(append_Physical_attribute_node, single_link_sucess) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memset(&key_switch_obj, 0, sizeof(key_switch_obj));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  ConfigNode *cfgptr_switch01 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_switch01);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add port s1-eth1 to switch 0000-0000-0000-0001
  key_port key_obj;
  val_port_st val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s1-eth1", sizeof(key_obj.port_id));

  memcpy(val_obj.port.description, "port-s1-eth1 connect to switch 01", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr_switch01_port_s1eth1 =
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(
      cfgptr_switch01_port_s1eth1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add switch 0000-0000-0000-0002 to cache
  key_switch key_switch_obj_sw02;
  val_switch_st val_switch_sw02;
  memset(&key_switch_obj_sw02, 0, sizeof(key_switch_obj_sw02));
  memset(&val_switch_sw02, 0, sizeof(val_switch_sw02));
  memcpy(key_switch_obj_sw02.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj_sw02.ctr_key.controller_name));
  memcpy(key_switch_obj_sw02.switch_id, "0000-0000-0000-0002", sizeof(
          key_switch_obj_sw02.switch_id));
  ConfigNode *cfgptr_switch02 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj_sw02, &val_switch_sw02, &val_switch_sw02, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_switch02);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add port s2-eth2 to switch 0000-0000-0000-0002
  key_port key_obj_s2eth2;
  val_port_st val_obj_s2eth2;
  memset(&key_obj_s2eth2, 0, sizeof(key_obj_s2eth2));
  memset(&val_obj_s2eth2, 0, sizeof(val_obj_s2eth2));
  memcpy(key_obj_s2eth2.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj_s2eth2.sw_key.ctr_key.controller_name));
  memcpy(key_obj_s2eth2.sw_key.switch_id, "0000-0000-0000-0002", sizeof(
          key_obj_s2eth2.sw_key.switch_id));
  memcpy(key_obj_s2eth2.port_id, "s2-eth2", sizeof(key_obj.port_id));

  memcpy(val_obj_s2eth2.port.description, "port-s2-eth2 connect to switch 02",
         sizeof(val_obj_s2eth2.port.description));
  ConfigNode *cfgptr_switch02_port_s2eth2 =
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj_s2eth2, &val_obj_s2eth2, &val_obj_s2eth2, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(
      cfgptr_switch02_port_s2eth2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  // add one link between switch01 to switch02
  key_link  key_link_sw01sw02;
  val_link_st  val_link_sw01sw02;
  memset(&key_link_sw01sw02, 0, sizeof(key_link_sw01sw02));
  memset(&val_link_sw01sw02, 0, sizeof(val_link_sw01sw02));

  memcpy(key_link_sw01sw02.ctr_key.controller_name, "odc1", sizeof(
         key_link_sw01sw02.ctr_key.controller_name));
  memcpy(key_link_sw01sw02.switch_id1, "0000-0000-0000-0001", sizeof(
          key_link_sw01sw02.switch_id1));
  memcpy(key_link_sw01sw02.port_id1, "s1-eth1", sizeof(
          key_link_sw01sw02.port_id1));
  memcpy(key_link_sw01sw02.switch_id2, "0000-0000-0000-0002", sizeof(
          key_link_sw01sw02.switch_id2));
  memcpy(key_link_sw01sw02.port_id2, "s2-eth2", sizeof(
          key_link_sw01sw02.port_id2));
  memcpy(val_link_sw01sw02.link.description, "link sw01 to sw02", sizeof(
          val_link_sw01sw02.link.description));
  val_link_sw01sw02.oper_status = 1;
  ConfigNode *cfgptr_link_sw01sw02 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_sw01sw02, &val_link_sw01sw02, &val_link_sw01sw02, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_link_sw01sw02);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  ConfigNode* cfgnode;
  for (cfgnode = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode = itr_ptr->NextItem() ) {
    pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 cfgnode->get_key_name().c_str(), cfgnode->get_type_name());
  }
  delete itr_ptr;

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr_switch01 = NULL;
  cfgptr_switch01_port_s1eth1 = NULL;
  cfgptr_switch02 = NULL;
  cfgptr_switch02_port_s2eth2 = NULL;
  cfgptr_link_sw01sw02 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}
//  Append twoswitches and two ports  to keytree with
//  append_Physical_attribute_node method and create bidirectional link
//  between switches
TEST(append_Physical_attribute_node, bidirectional_link_sucess) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memset(&key_switch_obj, 0, sizeof(key_switch_obj));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  ConfigNode *cfgptr_switch01 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch,  &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_switch01);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add port s1-eth1 to switch 0000-0000-0000-0001
  key_port key_obj;
  val_port_st val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s1-eth1", sizeof(key_obj.port_id));

  memcpy(val_obj.port.description, "port-s1-eth1 connect to switch 01", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr_switch01_port_s1eth1 =
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(
      cfgptr_switch01_port_s1eth1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add switch 0000-0000-0000-0002 to cache
  key_switch key_switch_obj_sw02;
  val_switch_st val_switch_sw02;
  memset(&key_switch_obj_sw02, 0, sizeof(key_switch_obj_sw02));
  memset(&val_switch_sw02, 0, sizeof(val_switch_sw02));
  memcpy(key_switch_obj_sw02.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj_sw02.ctr_key.controller_name));
  memcpy(key_switch_obj_sw02.switch_id, "0000-0000-0000-0002", sizeof(
          key_switch_obj_sw02.switch_id));
  ConfigNode *cfgptr_switch02 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj_sw02, &val_switch_sw02, &val_switch_sw02, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_switch02);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add port s2-eth2 to switch 0000-0000-0000-0002
  key_port key_obj_s2eth2;
  val_port_st val_obj_s2eth2;
  memset(&key_obj_s2eth2, 0, sizeof(key_obj_s2eth2));
  memset(&val_obj_s2eth2, 0, sizeof(val_obj_s2eth2));
  memcpy(key_obj_s2eth2.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj_s2eth2.sw_key.ctr_key.controller_name));
  memcpy(key_obj_s2eth2.sw_key.switch_id, "0000-0000-0000-0002", sizeof(
          key_obj_s2eth2.sw_key.switch_id));
  memcpy(key_obj_s2eth2.port_id, "s2-eth2", sizeof(key_obj.port_id));

  memcpy(val_obj_s2eth2.port.description, "port-s2-eth2 connect to switch 02",
         sizeof(val_obj_s2eth2.port.description));
  ConfigNode *cfgptr_switch02_port_s2eth2 =
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj_s2eth2, &val_obj_s2eth2, &val_obj_s2eth2, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(
      cfgptr_switch02_port_s2eth2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  // add one link between switch01 to switch02
  key_link  key_link_sw01sw02;
  val_link_st  val_link_sw01sw02;
  memset(&key_link_sw01sw02, 0, sizeof(key_link_sw01sw02));
  memset(&val_link_sw01sw02, 0, sizeof(val_link_sw01sw02));

  memcpy(key_link_sw01sw02.ctr_key.controller_name, "odc1", sizeof(
         key_link_sw01sw02.ctr_key.controller_name));
  memcpy(key_link_sw01sw02.switch_id1, "0000-0000-0000-0001", sizeof(
          key_link_sw01sw02.switch_id1));
  memcpy(key_link_sw01sw02.port_id1, "s1-eth1", sizeof(
          key_link_sw01sw02.port_id1));
  memcpy(key_link_sw01sw02.switch_id2, "0000-0000-0000-0002", sizeof(
          key_link_sw01sw02.switch_id2));
  memcpy(key_link_sw01sw02.port_id2, "s2-eth2", sizeof(
          key_link_sw01sw02.port_id2));
  memcpy(val_link_sw01sw02.link.description, "link sw01 to sw02", sizeof(
          val_link_sw01sw02.link.description));
  val_link_sw01sw02.oper_status = 1;
  ConfigNode *cfgptr_link_sw01sw02 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_sw01sw02, &val_link_sw01sw02, &val_link_sw01sw02, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_link_sw01sw02);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  // add one link between switch02 to switch01
  key_link  key_link_sw02sw01;
  val_link_st  val_link_sw02sw01;
  memset(&key_link_sw02sw01, 0, sizeof(key_link_sw02sw01));
  memset(&val_link_sw02sw01, 0, sizeof(val_link_sw02sw01));

  memcpy(key_link_sw02sw01.ctr_key.controller_name, "odc1", sizeof(
         key_link_sw02sw01.ctr_key.controller_name));
  memcpy(key_link_sw02sw01.switch_id1, "0000-0000-0000-0002", sizeof(
          key_link_sw02sw01.switch_id1));
  memcpy(key_link_sw02sw01.port_id1, "s2-eth2", sizeof(
          key_link_sw02sw01.port_id1));
  memcpy(key_link_sw02sw01.switch_id2, "0000-0000-0000-0001", sizeof(
          key_link_sw02sw01.switch_id2));
  memcpy(key_link_sw02sw01.port_id2, "s1-eth1", sizeof(
          key_link_sw02sw01.port_id2));
  memcpy(val_link_sw02sw01.link.description, "link sw02 to sw01", sizeof(
          val_link_sw02sw01.link.description));
  val_link_sw02sw01.oper_status = 1;
  ConfigNode *cfgptr_link_sw02sw01 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_sw02sw01, &val_link_sw02sw01, &val_link_sw02sw01, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_link_sw02sw01);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  ConfigNode* cfgnode;
  for (cfgnode = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode = itr_ptr->NextItem() ) {
    pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 cfgnode->get_key_name().c_str(), cfgnode->get_type_name());
  }
  delete itr_ptr;

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr_switch01 = NULL;
  cfgptr_switch01_port_s1eth1 = NULL;
  cfgptr_switch02 = NULL;
  cfgptr_switch02_port_s2eth2 = NULL;
  cfgptr_link_sw01sw02 = NULL;
  cfgptr_link_sw02sw01 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}
//  Append twoswitches and two ports  to keytree with
//  append_Physical_attribute_node method and create one link between
//  two switches and delete that link
TEST(delete_physical_attribute_node, single_link__delete_sucess) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memset(&key_switch_obj, 0, sizeof(key_switch_obj));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  ConfigNode *cfgptr_switch01 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_switch01);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add port s1-eth1 to switch 0000-0000-0000-0001
  key_port key_obj;
  val_port_st val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s1-eth1", sizeof(key_obj.port_id));

  memcpy(val_obj.port.description, "port-s1-eth1 connect to switch 01", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr_switch01_port_s1eth1 =
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(
      cfgptr_switch01_port_s1eth1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add switch 0000-0000-0000-0002 to cache
  key_switch key_switch_obj_sw02;
  val_switch_st val_switch_sw02;
  memset(&key_switch_obj_sw02, 0, sizeof(key_switch_obj_sw02));
  memset(&val_switch_sw02, 0, sizeof(val_switch_sw02));
  memcpy(key_switch_obj_sw02.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj_sw02.ctr_key.controller_name));
  memcpy(key_switch_obj_sw02.switch_id, "0000-0000-0000-0002", sizeof(
          key_switch_obj_sw02.switch_id));
  ConfigNode *cfgptr_switch02 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj_sw02, &val_switch_sw02, &val_switch_sw02, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_switch02);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add port s2-eth2 to switch 0000-0000-0000-0002
  key_port key_obj_s2eth2;
  val_port_st val_obj_s2eth2;
  memset(&key_obj_s2eth2, 0, sizeof(key_obj_s2eth2));
  memset(&val_obj_s2eth2, 0, sizeof(val_obj_s2eth2));
  memcpy(key_obj_s2eth2.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj_s2eth2.sw_key.ctr_key.controller_name));
  memcpy(key_obj_s2eth2.sw_key.switch_id, "0000-0000-0000-0002", sizeof(
          key_obj_s2eth2.sw_key.switch_id));
  memcpy(key_obj_s2eth2.port_id, "s2-eth2", sizeof(key_obj.port_id));

  memcpy(val_obj_s2eth2.port.description, "port-s2-eth2 connect to switch 02",
         sizeof(val_obj_s2eth2.port.description));
  ConfigNode *cfgptr_switch02_port_s2eth2 =
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj_s2eth2, &val_obj_s2eth2, &val_obj_s2eth2, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(
      cfgptr_switch02_port_s2eth2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  // add one link between switch01 to switch02
  key_link  key_link_sw01sw02;
  val_link_st  val_link_sw01sw02;
  memset(&key_link_sw01sw02, 0, sizeof(key_link_sw01sw02));
  memset(&val_link_sw01sw02, 0, sizeof(val_link_sw01sw02));

  memcpy(key_link_sw01sw02.ctr_key.controller_name, "odc1", sizeof(
         key_link_sw01sw02.ctr_key.controller_name));
  memcpy(key_link_sw01sw02.switch_id1, "0000-0000-0000-0001", sizeof(
          key_link_sw01sw02.switch_id1));
  memcpy(key_link_sw01sw02.port_id1, "s1-eth1", sizeof(
          key_link_sw01sw02.port_id1));
  memcpy(key_link_sw01sw02.switch_id2, "0000-0000-0000-0002", sizeof(
          key_link_sw01sw02.switch_id2));
  memcpy(key_link_sw01sw02.port_id2, "s2-eth2", sizeof(
          key_link_sw01sw02.port_id2));
  memcpy(val_link_sw01sw02.link.description, "link sw01 to sw02", sizeof(
          val_link_sw01sw02.link.description));
  val_link_sw01sw02.oper_status = 1;
  ConfigNode *cfgptr_link_sw01sw02 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_sw01sw02, &val_link_sw01sw02, &val_link_sw01sw02, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_link_sw01sw02);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  ConfigNode* cfgnode;
  for (cfgnode = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode = itr_ptr->NextItem() ) {
    pfc_log_info("Before Delete link, Node Present in Tree for:%s keytype %d",
                 cfgnode->get_key_name().c_str(), cfgnode->get_type_name());
  }
  delete itr_ptr;
  // delete existing link between switch01 to switch02
  key_link  key_link_delete_sw01sw02;
  val_link_st  val_link_delete_sw01sw02;
  memset(&key_link_delete_sw01sw02, 0, sizeof(key_link_delete_sw01sw02));
  memset(&val_link_delete_sw01sw02, 0, sizeof(val_link_delete_sw01sw02));

  memcpy(key_link_delete_sw01sw02.ctr_key.controller_name, "odc1", sizeof(
         key_link_delete_sw01sw02.ctr_key.controller_name));
  memcpy(key_link_delete_sw01sw02.switch_id1, "0000-0000-0000-0001", sizeof(
          key_link_delete_sw01sw02.switch_id1));
  memcpy(key_link_delete_sw01sw02.port_id1, "s1-eth1", sizeof(
          key_link_delete_sw01sw02.port_id1));
  memcpy(key_link_delete_sw01sw02.switch_id2, "0000-0000-0000-0002", sizeof(
          key_link_delete_sw01sw02.switch_id2));
  memcpy(key_link_delete_sw01sw02.port_id2, "s2-eth2", sizeof(
          key_link_delete_sw01sw02.port_id2));
  memcpy(val_link_delete_sw01sw02.link.description, "link sw01 to sw02",
         sizeof(val_link_delete_sw01sw02.link.description));
  val_link_delete_sw01sw02.oper_status = 1;
  ConfigNode *cfgptr_link_delete_sw01sw02 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_delete_sw01sw02, &val_link_delete_sw01sw02,
                             &val_link_delete_sw01sw02, operation);
  ret = KeyTree_obj->delete_physical_attribute_node(
      cfgptr_link_delete_sw01sw02);
  CommonIterator* itr = KeyTree_obj->create_iterator();
  ConfigNode* cfgnode_it;
  for (cfgnode_it = itr->PhysicalNodeFirstItem();
       itr->IsDone() == false;
       cfgnode_it = itr->NextItem() ) {
    pfc_log_info("After Delete link, Node Present in Tree for:%s keytype %d",
                  cfgnode_it->get_key_name().c_str(),
                  cfgnode_it->get_type_name());
  }
  delete itr;

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr_switch01 = NULL;
  cfgptr_switch01_port_s1eth1 = NULL;
  cfgptr_switch02 = NULL;
  cfgptr_switch02_port_s2eth2 = NULL;
  cfgptr_link_sw01sw02 = NULL;
  delete cfgptr_link_delete_sw01sw02;
  cfgptr_link_delete_sw01sw02 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}
//  Append twoswitches and two ports  to keytree with
//  append_Physical_attribute_node method and create bidirectional link
//  between them and delete both link
TEST(delete_physical_attribute_node, bidirectional_link_delete_sucess) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memset(&key_switch_obj, 0, sizeof(key_switch_obj));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  ConfigNode *cfgptr_switch01 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_switch01);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add port s1-eth1 to switch 0000-0000-0000-0001
  key_port key_obj;
  val_port_st val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s1-eth1", sizeof(key_obj.port_id));

  memcpy(val_obj.port.description, "port-s1-eth1 connect to switch 01", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr_switch01_port_s1eth1 =
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(
      cfgptr_switch01_port_s1eth1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add switch 0000-0000-0000-0002 to cache
  key_switch key_switch_obj_sw02;
  val_switch_st val_switch_sw02;
  memset(&key_switch_obj_sw02, 0, sizeof(key_switch_obj_sw02));
  memset(&val_switch_sw02, 0, sizeof(val_switch_sw02));
  memcpy(key_switch_obj_sw02.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj_sw02.ctr_key.controller_name));
  memcpy(key_switch_obj_sw02.switch_id, "0000-0000-0000-0002", sizeof(
          key_switch_obj_sw02.switch_id));
  ConfigNode *cfgptr_switch02 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj_sw02, &val_switch_sw02, &val_switch_sw02, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_switch02);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add port s2-eth2 to switch 0000-0000-0000-0002
  key_port key_obj_s2eth2;
  val_port_st val_obj_s2eth2;
  memset(&key_obj_s2eth2, 0, sizeof(key_obj_s2eth2));
  memset(&val_obj_s2eth2, 0, sizeof(val_obj_s2eth2));
  memcpy(key_obj_s2eth2.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj_s2eth2.sw_key.ctr_key.controller_name));
  memcpy(key_obj_s2eth2.sw_key.switch_id, "0000-0000-0000-0002", sizeof(
          key_obj_s2eth2.sw_key.switch_id));
  memcpy(key_obj_s2eth2.port_id, "s2-eth2", sizeof(key_obj.port_id));

  memcpy(val_obj_s2eth2.port.description, "port-s2-eth2 connect to switch 02",
         sizeof(val_obj_s2eth2.port.description));
  ConfigNode *cfgptr_switch02_port_s2eth2 =
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj_s2eth2, &val_obj_s2eth2, &val_obj_s2eth2, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(
      cfgptr_switch02_port_s2eth2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  // add one link between switch01 to switch02
  key_link  key_link_sw01sw02;
  val_link_st  val_link_sw01sw02;
  memset(&key_link_sw01sw02, 0, sizeof(key_link_sw01sw02));
  memset(&val_link_sw01sw02, 0, sizeof(val_link_sw01sw02));

  memcpy(key_link_sw01sw02.ctr_key.controller_name, "odc1", sizeof(
         key_link_sw01sw02.ctr_key.controller_name));
  memcpy(key_link_sw01sw02.switch_id1, "0000-0000-0000-0001", sizeof(
          key_link_sw01sw02.switch_id1));
  memcpy(key_link_sw01sw02.port_id1, "s1-eth1", sizeof(
          key_link_sw01sw02.port_id1));
  memcpy(key_link_sw01sw02.switch_id2, "0000-0000-0000-0002", sizeof(
          key_link_sw01sw02.switch_id2));
  memcpy(key_link_sw01sw02.port_id2, "s2-eth2", sizeof(
          key_link_sw01sw02.port_id2));
  memcpy(val_link_sw01sw02.link.description, "link sw01 to sw02", sizeof(
          val_link_sw01sw02.link.description));
  val_link_sw01sw02.oper_status = 1;
  ConfigNode *cfgptr_link_sw01sw02 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_sw01sw02, &val_link_sw01sw02, &val_link_sw01sw02, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_link_sw01sw02);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  // add one link between switch02 to switch01
  key_link  key_link_sw02sw01;
  val_link_st  val_link_sw02sw01;
  memset(&key_link_sw02sw01, 0, sizeof(key_link_sw02sw01));
  memset(&val_link_sw02sw01, 0, sizeof(val_link_sw02sw01));

  memcpy(key_link_sw02sw01.ctr_key.controller_name, "odc1", sizeof(
         key_link_sw02sw01.ctr_key.controller_name));
  memcpy(key_link_sw02sw01.switch_id1, "0000-0000-0000-0002", sizeof(
          key_link_sw02sw01.switch_id1));
  memcpy(key_link_sw02sw01.port_id1, "s2-eth2", sizeof(
          key_link_sw02sw01.port_id1));
  memcpy(key_link_sw02sw01.switch_id2, "0000-0000-0000-0001", sizeof(
          key_link_sw02sw01.switch_id2));
  memcpy(key_link_sw02sw01.port_id2, "s1-eth1", sizeof(
          key_link_sw02sw01.port_id2));
  memcpy(val_link_sw02sw01.link.description, "link sw02 to sw01", sizeof(
          val_link_sw02sw01.link.description));
  val_link_sw02sw01.oper_status = 1;
  ConfigNode *cfgptr_link_sw02sw01 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_sw02sw01, &val_link_sw02sw01, &val_link_sw02sw01, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_link_sw02sw01);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  ConfigNode* cfgnode;
  for (cfgnode = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode = itr_ptr->NextItem() ) {
    pfc_log_info("Before delete links, Node Present in Tree for:%s keytype %d",
                 cfgnode->get_key_name().c_str(), cfgnode->get_type_name());
  }
  delete itr_ptr;
  // delete existing link between switch01 to switch02
  key_link  key_link_delete_sw01sw02;
  val_link_st  val_link_delete_sw01sw02;
  memset(&key_link_delete_sw01sw02, 0, sizeof(key_link_delete_sw01sw02));
  memset(&val_link_delete_sw01sw02, 0, sizeof(val_link_delete_sw01sw02));

  memcpy(key_link_delete_sw01sw02.ctr_key.controller_name, "odc1", sizeof(
         key_link_delete_sw01sw02.ctr_key.controller_name));
  memcpy(key_link_delete_sw01sw02.switch_id1, "0000-0000-0000-0001", sizeof(
          key_link_delete_sw01sw02.switch_id1));
  memcpy(key_link_delete_sw01sw02.port_id1, "s1-eth1", sizeof(
          key_link_delete_sw01sw02.port_id1));
  memcpy(key_link_delete_sw01sw02.switch_id2, "0000-0000-0000-0002", sizeof(
          key_link_delete_sw01sw02.switch_id2));
  memcpy(key_link_delete_sw01sw02.port_id2, "s2-eth2", sizeof(
          key_link_delete_sw01sw02.port_id2));
  memcpy(val_link_delete_sw01sw02.link.description, "delete link sw01 to sw02",
         sizeof(val_link_delete_sw01sw02.link.description));
  val_link_delete_sw01sw02.oper_status = 1;
  ConfigNode *cfgptr_link_delete_sw01sw02 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_delete_sw01sw02, &val_link_delete_sw01sw02,
                         &val_link_delete_sw01sw02, operation);
  ret = KeyTree_obj->delete_physical_attribute_node(
      cfgptr_link_delete_sw01sw02);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  // delete existing link between switch02 to switch01
  key_link  key_link_delete_sw02sw01;
  val_link_st  val_link_delete_sw02sw01;
  memset(&key_link_delete_sw02sw01, 0, sizeof(key_link_delete_sw02sw01));
  memset(&val_link_delete_sw02sw01, 0, sizeof(val_link_delete_sw02sw01));

  memcpy(key_link_delete_sw02sw01.ctr_key.controller_name, "odc1", sizeof(
         key_link_delete_sw02sw01.ctr_key.controller_name));
  memcpy(key_link_delete_sw02sw01.switch_id1, "0000-0000-0000-0002", sizeof(
          key_link_delete_sw02sw01.switch_id1));
  memcpy(key_link_delete_sw02sw01.port_id1, "s2-eth2", sizeof(
          key_link_delete_sw02sw01.port_id1));
  memcpy(key_link_delete_sw02sw01.switch_id2, "0000-0000-0000-0001", sizeof(
          key_link_delete_sw02sw01.switch_id2));
  memcpy(key_link_delete_sw02sw01.port_id2, "s1-eth1", sizeof(
          key_link_delete_sw02sw01.port_id2));
  memcpy(val_link_delete_sw02sw01.link.description, "delete link sw02 to sw01",
         sizeof(val_link_delete_sw02sw01.link.description));
  val_link_delete_sw02sw01.oper_status = 1;
  ConfigNode *cfgptr_link_delete_sw02sw01 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_delete_sw02sw01, &val_link_delete_sw02sw01,
                      &val_link_delete_sw02sw01, operation);
  ret = KeyTree_obj->delete_physical_attribute_node(
      cfgptr_link_delete_sw02sw01);
  CommonIterator* itr = KeyTree_obj->create_iterator();
  ConfigNode* cfgnode_it;
  for (cfgnode_it = itr->PhysicalNodeFirstItem();
       itr->IsDone() == false;
       cfgnode_it = itr->NextItem() ) {
    pfc_log_info("After Delete link, Node Present in Tree for:%s keytype %d",
                 cfgnode_it->get_key_name().c_str(),
                 cfgnode_it->get_type_name());
  }
  delete itr;

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr_switch01 = NULL;
  cfgptr_switch01_port_s1eth1 = NULL;
  cfgptr_switch02 = NULL;
  cfgptr_switch02_port_s2eth2 = NULL;
  cfgptr_link_sw01sw02 = NULL;
  cfgptr_link_sw02sw01 = NULL;
  delete cfgptr_link_delete_sw01sw02;
  cfgptr_link_delete_sw01sw02 = NULL;
  delete cfgptr_link_delete_sw02sw01;
  cfgptr_link_delete_sw02sw01 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}
//  Append twoswitches and two ports  to keytree with
//  append_Physical_attribute_node method and create one link between
//  switches and update the link parameter oper_status/description
TEST(update_physical_attribute_node, single_link_update_sucess) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memset(&key_switch_obj, 0, sizeof(key_switch_obj));
  memset(&val_switch, 0, sizeof(val_switch));
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
          key_switch_obj.switch_id));
  ConfigNode *cfgptr_switch01 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj, &val_switch, &val_switch, operation);
  uint32_t ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_switch01);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add port s1-eth1 to switch 0000-0000-0000-0001
  key_port key_obj;
  val_port_st val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
          key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s1-eth1", sizeof(key_obj.port_id));

  memcpy(val_obj.port.description, "port-s1-eth1 connect to switch 01", sizeof(
          val_obj.port.description));
  ConfigNode *cfgptr_switch01_port_s1eth1 =
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(
      cfgptr_switch01_port_s1eth1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add switch 0000-0000-0000-0002 to cache
  key_switch key_switch_obj_sw02;
  val_switch_st val_switch_sw02;
  memset(&key_switch_obj_sw02, 0, sizeof(key_switch_obj_sw02));
  memset(&val_switch_sw02, 0, sizeof(val_switch_sw02));
  memcpy(key_switch_obj_sw02.ctr_key.controller_name, "odc1", sizeof(
          key_switch_obj_sw02.ctr_key.controller_name));
  memcpy(key_switch_obj_sw02.switch_id, "0000-0000-0000-0002", sizeof(
          key_switch_obj_sw02.switch_id));
  ConfigNode *cfgptr_switch02 =
      new CacheElementUtil<key_switch, val_switch_st, val_switch_st, uint32_t>
      (&key_switch_obj_sw02, &val_switch_sw02, &val_switch_sw02, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_switch02);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  //  add port s2-eth2 to switch 0000-0000-0000-0002
  key_port key_obj_s2eth2;
  val_port_st val_obj_s2eth2;
  memset(&key_obj_s2eth2, 0, sizeof(key_obj_s2eth2));
  memset(&val_obj_s2eth2, 0, sizeof(val_obj_s2eth2));
  memcpy(key_obj_s2eth2.sw_key.ctr_key.controller_name, "odc1", sizeof(
          key_obj_s2eth2.sw_key.ctr_key.controller_name));
  memcpy(key_obj_s2eth2.sw_key.switch_id, "0000-0000-0000-0002", sizeof(
          key_obj_s2eth2.sw_key.switch_id));
  memcpy(key_obj_s2eth2.port_id, "s2-eth2", sizeof(key_obj.port_id));

  memcpy(val_obj_s2eth2.port.description, "port-s2-eth2 connect to switch 02",
         sizeof(val_obj_s2eth2.port.description));
  ConfigNode *cfgptr_switch02_port_s2eth2 =
      new CacheElementUtil<key_port, val_port_st, val_port_st, uint32_t>
      (&key_obj_s2eth2, &val_obj_s2eth2, &val_obj_s2eth2, operation);

  ret = KeyTree_obj->append_Physical_attribute_node(
      cfgptr_switch02_port_s2eth2);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  // add one link between switch01 to switch02
  key_link  key_link_sw01sw02;
  val_link_st  val_link_sw01sw02;
  memset(&key_link_sw01sw02, 0, sizeof(key_link_sw01sw02));
  memset(&val_link_sw01sw02, 0, sizeof(val_link_sw01sw02));

  memcpy(key_link_sw01sw02.ctr_key.controller_name, "odc1", sizeof(
         key_link_sw01sw02.ctr_key.controller_name));
  memcpy(key_link_sw01sw02.switch_id1, "0000-0000-0000-0001", sizeof(
          key_link_sw01sw02.switch_id1));
  memcpy(key_link_sw01sw02.port_id1, "s1-eth1", sizeof(
          key_link_sw01sw02.port_id1));
  memcpy(key_link_sw01sw02.switch_id2, "0000-0000-0000-0002", sizeof(
          key_link_sw01sw02.switch_id2));
  memcpy(key_link_sw01sw02.port_id2, "s2-eth2", sizeof(
          key_link_sw01sw02.port_id2));
  memcpy(val_link_sw01sw02.link.description, "link sw01 to sw02", sizeof(
          val_link_sw01sw02.link.description));
  val_link_sw01sw02.oper_status = 1;
  ConfigNode *cfgptr_link_sw01sw02 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_sw01sw02, &val_link_sw01sw02,
                              &val_link_sw01sw02, operation);
  ret = KeyTree_obj->append_Physical_attribute_node(cfgptr_link_sw01sw02);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t> *tmp_ptr_one =
      static_cast<CacheElementUtil<key_link, val_link_st,
             val_link_st, uint32_t>*> (cfgptr_link_sw01sw02);
  pfc_log_info("Before update link parameter");
  pfc_log_info("Description= %s", reinterpret_cast<char*>(
          tmp_ptr_one->get_val_structure()->link.description));
  pfc_log_info("oper_status= %d", tmp_ptr_one->get_val_structure()->
               oper_status);
  // Update the link parameter between switch01 to switch02
  key_link  key_link_update_sw01sw02;
  val_link_st  val_link_update_sw01sw02;
  memset(&key_link_update_sw01sw02, 0, sizeof(key_link_update_sw01sw02));
  memset(&val_link_update_sw01sw02, 0, sizeof(val_link_update_sw01sw02));

  memcpy(key_link_update_sw01sw02.ctr_key.controller_name, "odc1", sizeof(
         key_link_update_sw01sw02.ctr_key.controller_name));
  memcpy(key_link_update_sw01sw02.switch_id1, "0000-0000-0000-0001", sizeof(
          key_link_update_sw01sw02.switch_id1));
  memcpy(key_link_update_sw01sw02.port_id1, "s1-eth1", sizeof(
          key_link_update_sw01sw02.port_id1));
  memcpy(key_link_update_sw01sw02.switch_id2, "0000-0000-0000-0002", sizeof(
          key_link_update_sw01sw02.switch_id2));
  memcpy(key_link_update_sw01sw02.port_id2, "s2-eth2", sizeof(
          key_link_update_sw01sw02.port_id2));
  memcpy(val_link_update_sw01sw02.link.description,
         "link sw01 to sw02 parameter changes", sizeof(
          val_link_update_sw01sw02.link.description));
  val_link_update_sw01sw02.oper_status = 2;
  ConfigNode *cfgptr_link_update_sw01sw02 =
      new CacheElementUtil<key_link, val_link_st, val_link_st, uint32_t>
      (&key_link_update_sw01sw02, &val_link_update_sw01sw02,
                &val_link_update_sw01sw02, operation);
  ret = KeyTree_obj->update_physical_attribute_node(
      cfgptr_link_update_sw01sw02);
  CacheElementUtil<key_link, val_link_st,  val_link_st, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_link, val_link_st,
           val_link_st, uint32_t>*> (cfgptr_link_sw01sw02);
  pfc_log_info("After update link parameter");
  pfc_log_info("Description= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->link.description));
  pfc_log_info("oper_status= %d", tmp_ptr->get_val_structure()->oper_status);

  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr_switch01 = NULL;
  cfgptr_switch01_port_s1eth1 = NULL;
  cfgptr_switch02 = NULL;
  cfgptr_switch02_port_s2eth2 = NULL;
  cfgptr_link_sw01sw02 = NULL;
  delete cfgptr_link_update_sw01sw02;
  cfgptr_link_update_sw01sw02 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, FlowList) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_flowlist_t key_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  val_flowlist_t val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.flowlist_name, "flowlistone", sizeof(key_obj.flowlist_name));
  ConfigNode *cfgptr = new CacheElementUtil<key_flowlist_t, val_flowlist_t,
           val_flowlist_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, Retrive_Flow_List) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_flowlist_t key_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  val_flowlist_t val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.flowlist_name, "flowlistone", sizeof(key_obj.flowlist_name));
  ConfigNode *cfgptr = new CacheElementUtil<key_flowlist_t, val_flowlist_t,
            val_flowlist_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_flowlist_t, val_flowlist_t,
                val_flowlist_t, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_flowlist_t, val_flowlist_t,
      val_flowlist_t, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.flowlist_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->flowlist_name));
  pfc_log_info("Flow-list name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->flowlist_name));
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
}

TEST(get_parenttype, KT_FLOW_LIST) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  unc_key_type_t keytype = KeyTree_obj->get_parenttype(UNC_KT_FLOWLIST);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_ROOT);
}

TEST(append_commit_node, same_root_level) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));

  val_vtn val_obj;
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  key_flowlist_t key_obj1;
  memset(&key_obj1, 0, sizeof(key_obj1));
  val_flowlist_t val_obj1;
  memset(&val_obj1, 0, sizeof(val_obj1));
  memcpy(key_obj1.flowlist_name, "flowlistone", sizeof(key_obj1.flowlist_name));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_flowlist_t,
           val_flowlist_t, val_flowlist_t, uint32_t>
      (&key_obj1, &val_obj1, &val_obj1, operation);
  ret = KeyTree_obj->append_commit_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vtn, val_vtn,
                     val_vtn, uint32_t>*> (cfgptr);
  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->vtn_name));
  EXPECT_STREQ(reinterpret_cast<char*>(val_obj.description),
          reinterpret_cast<char*>(tmp_ptr->get_val_structure()->description));
  pfc_log_info("vtn name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vtn_name));
  pfc_log_info("Description= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->description));
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  cfgptr1 = itr_ptr->NextItem();
  CacheElementUtil<key_flowlist_t, val_flowlist_t,
                  val_flowlist_t, uint32_t> *tmp_ptr1 =
      static_cast<CacheElementUtil<key_flowlist_t, val_flowlist_t,
      val_flowlist_t, uint32_t>*> (cfgptr1);

  EXPECT_STREQ(reinterpret_cast<char*>(key_obj1.flowlist_name),
         reinterpret_cast<char*>(tmp_ptr1->get_key_structure()->flowlist_name));
  pfc_log_info("Flow-list name= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->flowlist_name));
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr1->get_key_name().c_str(), tmp_ptr1->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr1->get_parent_key_name().c_str(),
               tmp_ptr1->get_key_generate().c_str());

  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(get_parenttype, KT_FLOW_LIST_ENTRY) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  unc_key_type_t keytype = KeyTree_obj->get_parenttype(UNC_KT_FLOWLIST_ENTRY);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_FLOWLIST);
}

TEST(append_commit_node, FlowList_Entry) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  key_flowlist_t key_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  val_flowlist_t val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.flowlist_name, "flowlistone", sizeof(key_obj.flowlist_name));

  ConfigNode *cfgptr = new CacheElementUtil<key_flowlist_t,
           val_flowlist_t, val_flowlist_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_flowlist_entry  key_flolistentry;
  val_flowlist_entry  val_flowlistentry;
  memset(&key_flolistentry, 0, sizeof(key_flolistentry));
  memset(&val_flowlistentry, 0, sizeof(val_flowlistentry));
  memcpy(key_flolistentry.flowlist_key.flowlist_name, "flowlistone",
         sizeof(key_obj.flowlist_name));
  key_flolistentry.sequence_num = 20;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_flowlist_entry,
             val_flowlist_entry, val_flowlist_entry, uint32_t>
      (&key_flolistentry, &val_flowlistentry,
                       &val_flowlistentry, operation);
  ret = KeyTree_obj->append_commit_node(cfgptr1);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, FlowList_Entry_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  key_flowlist_t key_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  val_flowlist_t val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.flowlist_name, "flowlistone", sizeof(key_obj.flowlist_name));

  ConfigNode *cfgptr = new CacheElementUtil<key_flowlist_t, val_flowlist_t,
        val_flowlist_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_flowlist_entry  key_flolistentry;
  val_flowlist_entry  val_flowlistentry;
  memset(&key_flolistentry, 0, sizeof(key_flolistentry));
  memset(&val_flowlistentry, 0, sizeof(val_flowlistentry));
  memcpy(key_flolistentry.flowlist_key.flowlist_name, "flowlistone",
         sizeof(key_obj.flowlist_name));
  key_flolistentry.sequence_num = 20;
  memcpy(val_flowlistentry.mac_dst, "10", sizeof(val_flowlistentry.mac_dst));
  memcpy(val_flowlistentry.mac_src, "11", sizeof(val_flowlistentry.mac_src));
  val_flowlistentry.mac_eth_type = 12;
  val_flowlistentry.vlan_priority = 13;
  val_flowlistentry.ip_dscp = 14;
  val_flowlistentry.ip_proto = 15;
  val_flowlistentry.l4_dst_port = 16;
  val_flowlistentry.l4_src_port = 17;
  val_flowlistentry.l4_src_port_endpt = 18;
  val_flowlistentry.l4_dst_port_endpt = 19;
  val_flowlistentry.icmp_type = 21;
  val_flowlistentry.icmp_code = 22;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_flowlist_entry,
         val_flowlist_entry, val_flowlist_entry, uint32_t>
      (&key_flolistentry, &val_flowlistentry,
                       &val_flowlistentry, operation);
  ret = KeyTree_obj->append_commit_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_flowlist_t, val_flowlist_t,
             val_flowlist_t, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_flowlist_t, val_flowlist_t,
      val_flowlist_t, uint32_t>*> (cfgptr);
  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.flowlist_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->flowlist_name));
  pfc_log_info("kt_flowlist:flowlistname= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->flowlist_name));
  pfc_log_info(":Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  cfgptr1 = itr_ptr->NextItem();
  CacheElementUtil<key_flowlist_entry, val_flowlist_entry,
               val_flowlist_entry, uint32_t> *tmp_ptr1 =
      static_cast<CacheElementUtil<key_flowlist_entry, val_flowlist_entry,
      val_flowlist_entry, uint32_t>*> (cfgptr1);
  EXPECT_STREQ(reinterpret_cast<char*>(key_flolistentry.flowlist_key.
                                       flowlist_name),
         reinterpret_cast<char*>(tmp_ptr1->get_key_structure()->
                                 flowlist_key.flowlist_name));
  pfc_log_info("kt_flowlist_entry:flowlistname= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->flowlist_key.flowlist_name));
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr1->get_key_name().c_str(), tmp_ptr1->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr1->get_parent_key_name().c_str(),
               tmp_ptr1->get_key_generate().c_str());
  pfc_log_info("kt_flowlist_entry:sequence number= %d", tmp_ptr1->
               get_key_structure()->sequence_num);
  pfc_log_info("kt_flowlistentry:mac_dst= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_val_structure()->mac_dst));
  pfc_log_info("kt_flowlistentry:mac_src= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_val_structure()->mac_src));
  pfc_log_info("kt_flowlistentry:mac_eth_type= %d", tmp_ptr1->
               get_val_structure()->mac_eth_type);
  pfc_log_info("kt_flowlistentry:vlan_priority= %d", tmp_ptr1->
               get_val_structure()->vlan_priority);
  pfc_log_info("kt_flowlistentry:ip_dscp= %d", tmp_ptr1->
               get_val_structure()->ip_dscp);
  pfc_log_info("kt_flowlistentry:ip_proto= %d", tmp_ptr1->
               get_val_structure()->ip_proto);
  pfc_log_info("kt_flowlistentry:l4_dst_port= %d", tmp_ptr1->
               get_val_structure()->l4_dst_port);
  pfc_log_info("kt_flowlistentry:l4_src_port= %d", tmp_ptr1->
               get_val_structure()->l4_src_port);
  pfc_log_info("kt_flowlistentry:l4_src_port_endpt= %d", tmp_ptr1->
               get_val_structure()->l4_src_port_endpt);
  pfc_log_info("kt_flowlistentry:l4_dst_port_endpt= %d", tmp_ptr1->
               get_val_structure()->l4_dst_port_endpt);
  pfc_log_info("kt_flowlistentry:icmp_type= %d", tmp_ptr1->
               get_val_structure()->icmp_type);
  pfc_log_info("kt_flowlistentry:icmp_code= %d", tmp_ptr1->
               get_val_structure()->icmp_code);
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
}

TEST(append_audit_node, parent_not_exist_flowlist_entry) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  key_flowlist_entry  key_flolistentry;
  val_flowlist_entry  val_flowlistentry;
  memset(&key_flolistentry, 0, sizeof(key_flolistentry));
  memset(&val_flowlistentry, 0, sizeof(val_flowlistentry));
  memcpy(key_flolistentry.flowlist_key.flowlist_name, "flowlistone",
         sizeof(key_flolistentry.flowlist_key.flowlist_name));
  key_flolistentry.sequence_num = 20;
  memcpy(val_flowlistentry.mac_dst, "10", sizeof(val_flowlistentry.mac_dst));
  memcpy(val_flowlistentry.mac_src, "11", sizeof(val_flowlistentry.mac_src));

  ConfigNode *cfgptr = new CacheElementUtil<key_flowlist_entry,
             val_flowlist_entry, val_flowlist_entry, uint32_t>
      (&key_flolistentry, &val_flowlistentry, &val_flowlistentry, operation);
  int ret = KeyTree_obj->append_audit_node(cfgptr);

  delete cfgptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(add_child_to_hash, FlowList_Success) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  int operation = 1;
  key_flowlist_t key_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  val_flowlist_t val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.flowlist_name, "flowlistone", sizeof(key_obj.flowlist_name));

  ConfigNode *cfgptr = new CacheElementUtil<key_flowlist_t,
             val_flowlist_t, val_flowlist_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->add_child_to_hash(cfgptr);
  delete KeyTree_obj;
  delete cfgptr;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, FlowListEntry_Success) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  int operation = 1;
  key_flowlist_t key_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  val_flowlist_t val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.flowlist_name, "flowlistone", sizeof(key_obj.flowlist_name));

  ConfigNode *cfgptr = new CacheElementUtil<key_flowlist_t,
             val_flowlist_t, val_flowlist_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->add_child_to_hash(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_flowlist_entry  key_flolistentry;
  val_flowlist_entry  val_flowlistentry;
  memset(&key_flolistentry, 0, sizeof(key_flolistentry));
  memset(&val_flowlistentry, 0, sizeof(val_flowlistentry));
  memcpy(key_flolistentry.flowlist_key.flowlist_name, "flowlistone",
         sizeof(key_flolistentry.flowlist_key.flowlist_name));
  key_flolistentry.sequence_num = 20;
  memcpy(val_flowlistentry.mac_dst, "10", sizeof(val_flowlistentry.mac_dst));
  memcpy(val_flowlistentry.mac_src, "11", sizeof(val_flowlistentry.mac_src));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_flowlist_entry,
             val_flowlist_entry, val_flowlist_entry, uint32_t>
      (&key_flolistentry, &val_flowlistentry,
                     &val_flowlistentry, operation);
  ret = KeyTree_obj->add_child_to_hash(cfgptr1);
  delete KeyTree_obj;
  delete cfgptr;
  delete cfgptr1;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(get_parenttype, VtnFlowFilter) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  unc_key_type_t keytype = KeyTree_obj->get_parenttype(UNC_KT_VTN_FLOWFILTER);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VTN);
}

TEST(get_parenttype, VtnFlowFilter_Entry) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  unc_key_type_t keytype = KeyTree_obj->
      get_parenttype(UNC_KT_VTN_FLOWFILTER_ENTRY);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VTN_FLOWFILTER);
}

TEST(append_commit_node, VtnFlowFilter) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn_flowfilter key_obj;
  val_flowfilter val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  key_obj.input_direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn_flowfilter,
             val_flowfilter, val_flowfilter, uint32_t>
      (&key_obj, &val_obj,  &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, VtnFlowFilter_Entry) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn_flowfilter key_obj;
  val_flowfilter val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  key_obj.input_direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn_flowfilter,
             val_flowfilter, val_flowfilter, uint32_t>
      (&key_obj, &val_obj,  &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vtn_flowfilter_entry key_VFlowFilterEntry;
  val_vtn_flowfilter_entry val_VFlowFilterEntry;
  memset(&key_VFlowFilterEntry, 0, sizeof(key_VFlowFilterEntry));
  memset(&val_VFlowFilterEntry, 0, sizeof(val_VFlowFilterEntry));
  memcpy(key_VFlowFilterEntry.flowfilter_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_VFlowFilterEntry.flowfilter_key.vtn_key.vtn_name));
  key_VFlowFilterEntry.flowfilter_key.input_direction = 1;
  key_VFlowFilterEntry.sequence_num = 20;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vtn_flowfilter_entry,
             val_vtn_flowfilter_entry,val_vtn_flowfilter_entry, uint32_t>
      (&key_VFlowFilterEntry, &val_VFlowFilterEntry,
                        &val_VFlowFilterEntry, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, VtnFlowFilter_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn_flowfilter key_obj;
  val_flowfilter val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  key_obj.input_direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn_flowfilter,
      val_flowfilter, val_flowfilter, uint32_t>
        (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vtn_flowfilter, val_flowfilter,
            val_flowfilter, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vtn_flowfilter, val_flowfilter,
       val_flowfilter, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->
                                 vtn_key.vtn_name));
  pfc_log_info("VtnFlowFilter Vtn Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vtn_key.vtn_name));
  pfc_log_info("VtnFlowFilter Input_direction= %d",
          tmp_ptr->get_key_structure()->input_direction);
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, VtnFlowFilter_Entry_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn_flowfilter key_obj;
  val_flowfilter val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  key_obj.input_direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn_flowfilter,
             val_flowfilter, val_flowfilter, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vtn_flowfilter_entry key_VFlowFilterEntry;
  val_vtn_flowfilter_entry val_VFlowFilterEntry;
  memset(&key_VFlowFilterEntry, 0, sizeof(key_VFlowFilterEntry));
  memset(&val_VFlowFilterEntry, 0, sizeof(val_VFlowFilterEntry));
  memcpy(key_VFlowFilterEntry.flowfilter_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_VFlowFilterEntry.flowfilter_key.vtn_key.vtn_name));
  key_VFlowFilterEntry.flowfilter_key.input_direction = 1;
  key_VFlowFilterEntry.sequence_num = 20;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vtn_flowfilter_entry,
        val_vtn_flowfilter_entry, val_vtn_flowfilter_entry, uint32_t>
      (&key_VFlowFilterEntry, &val_VFlowFilterEntry,
                      &val_VFlowFilterEntry, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vtn_flowfilter, val_flowfilter,
                    val_flowfilter, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vtn_flowfilter, val_flowfilter,
      val_flowfilter, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->
                                 vtn_key.vtn_name));
  pfc_log_info("VtnFlowFilter Vtn Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vtn_key.vtn_name));
  pfc_log_info("VtnFlowFilter Input_direction= %d",
          tmp_ptr->get_key_structure()->input_direction);
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  cfgptr1 = itr_ptr->NextItem();
  CacheElementUtil<key_vtn_flowfilter_entry, val_vtn_flowfilter_entry,
      val_vtn_flowfilter_entry, uint32_t> *tmp_ptr1 =
      static_cast<CacheElementUtil<key_vtn_flowfilter_entry,
      val_vtn_flowfilter_entry,
              val_vtn_flowfilter_entry, uint32_t>*> (cfgptr1);
  EXPECT_STREQ(reinterpret_cast<char*>(key_VFlowFilterEntry.flowfilter_key.
                                       vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr1->get_key_structure()->
                                 flowfilter_key.vtn_key.vtn_name));
  pfc_log_info("VtnFlowFilter Vtn Name= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->flowfilter_key.vtn_key.vtn_name));
  pfc_log_info("VtnFlowFilter Input_direction= %d",
          tmp_ptr1->get_key_structure()->flowfilter_key.input_direction);
  pfc_log_info("vtnflowfilterentry:sequence number= %d", tmp_ptr1->
               get_key_structure()->sequence_num);
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr1->get_key_name().c_str(), tmp_ptr1->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr1->get_parent_key_name().c_str(),
               tmp_ptr1->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, VtnFlowFilter_success) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn_flowfilter key_obj;
  val_flowfilter val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  key_obj.input_direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn_flowfilter,
             val_flowfilter, val_flowfilter, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  int ret = KeyTree_obj->add_child_to_hash(cfgptr);
  delete KeyTree_obj;
  delete cfgptr;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, VtnFlowFilterEntry_Success) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn_flowfilter key_obj;
  val_flowfilter val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  key_obj.input_direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn_flowfilter,
             val_flowfilter, val_flowfilter, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->add_child_to_hash(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vtn_flowfilter_entry key_VFlowFilterEntry;
  val_vtn_flowfilter_entry val_VFlowFilterEntry;
  memset(&key_VFlowFilterEntry, 0, sizeof(key_VFlowFilterEntry));
  memset(&val_VFlowFilterEntry, 0, sizeof(val_VFlowFilterEntry));
  memcpy(key_VFlowFilterEntry.flowfilter_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_VFlowFilterEntry.flowfilter_key.vtn_key.vtn_name));
  key_VFlowFilterEntry.flowfilter_key.input_direction = 1;
  key_VFlowFilterEntry.sequence_num = 20;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vtn_flowfilter_entry,
             val_vtn_flowfilter_entry, val_vtn_flowfilter_entry, uint32_t>
      (&key_VFlowFilterEntry, &val_VFlowFilterEntry,
                                  &val_VFlowFilterEntry, operation);

  ret = KeyTree_obj->add_child_to_hash(cfgptr1);
  delete KeyTree_obj;
  delete cfgptr;
  delete cfgptr1;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_audit_node, parent_not_exist_vtnflofilter_entry) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  key_vtn_flowfilter_entry key_VFlowFilterEntry;
  val_vtn_flowfilter_entry val_VFlowFilterEntry;
  memset(&key_VFlowFilterEntry, 0, sizeof(key_VFlowFilterEntry));
  memset(&val_VFlowFilterEntry, 0, sizeof(val_VFlowFilterEntry));
  memcpy(key_VFlowFilterEntry.flowfilter_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_VFlowFilterEntry.flowfilter_key.vtn_key.vtn_name));
  key_VFlowFilterEntry.flowfilter_key.input_direction = 1;
  key_VFlowFilterEntry.sequence_num = 20;
  ConfigNode *cfgptr = new CacheElementUtil<key_vtn_flowfilter_entry,
             val_vtn_flowfilter_entry, val_vtn_flowfilter_entry, uint32_t>
      (&key_VFlowFilterEntry, &val_VFlowFilterEntry,
                      &val_VFlowFilterEntry, operation);
  int ret = KeyTree_obj->append_audit_node(cfgptr);

  delete cfgptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(get_parenttype, VbrFlowFilter) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  unc_key_type_t keytype = KeyTree_obj->get_parenttype(UNC_KT_VBR_FLOWFILTER);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VBRIDGE);
}

TEST(append_commit_node, VbrFlowFilter) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_flowfilter_t key_obj;
  val_flowfilter_t val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.vbr_key.vbridge_name));
  key_obj.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_flowfilter_t,
             val_flowfilter_t, val_flowfilter_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, VbrFlowFilter_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_flowfilter_t key_obj;
  val_flowfilter_t val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.vbr_key.vbridge_name));
  key_obj.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_flowfilter_t,
             val_flowfilter_t,  val_flowfilter_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vbr_flowfilter_t, val_flowfilter_t,
                     val_flowfilter_t, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vbr_flowfilter_t, val_flowfilter_t,
       val_flowfilter_t, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.vbr_key.vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->
                                 vbr_key.vtn_key.vtn_name));
  pfc_log_info("Vtn Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vbr_key.vtn_key.vtn_name));
  pfc_log_info("Vbridge Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vbr_key.vbridge_name));
  pfc_log_info("Direction= %d", tmp_ptr->get_key_structure()->direction);
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, VbrFlowFilter_Success) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_flowfilter_t key_obj;
  val_flowfilter_t val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.vbr_key.vbridge_name));
  key_obj.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_flowfilter_t,
             val_flowfilter_t, val_flowfilter_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  int ret = KeyTree_obj->add_child_to_hash(cfgptr);
  delete KeyTree_obj;
  delete cfgptr;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(get_parenttype, VbrFlowFilterEntry) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  unc_key_type_t keytype = KeyTree_obj->
      get_parenttype(UNC_KT_VBR_FLOWFILTER_ENTRY);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VBR_FLOWFILTER);
}

TEST(append_commit_node, VbrFlowFilterEntry) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_flowfilter_t key_obj;
  val_flowfilter_t val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.vbr_key.vbridge_name, "vbridge1", sizeof(key_obj.vbr_key.
                                                          vbridge_name));
  key_obj.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_flowfilter_t,
             val_flowfilter_t, val_flowfilter_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vbr_flowfilter_entry key_vbrflowfilterentry;
  val_flowfilter_entry val_vbrflowfilterentry;
  memset(&key_vbrflowfilterentry, 0, sizeof(key_vbrflowfilterentry));
  memset(&val_vbrflowfilterentry, 0, sizeof(val_vbrflowfilterentry));
  memcpy(key_vbrflowfilterentry.flowfilter_key.vbr_key.vtn_key.vtn_name,
         "vtn1", sizeof(key_vbrflowfilterentry.flowfilter_key.vbr_key.vtn_key.
                        vtn_name));
  memcpy(key_vbrflowfilterentry.flowfilter_key.vbr_key.vbridge_name,
         "vbridge1", sizeof(key_vbrflowfilterentry.flowfilter_key.vbr_key.
                            vbridge_name));
  key_vbrflowfilterentry.flowfilter_key.direction = 1;
  key_vbrflowfilterentry.sequence_num = 20;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr_flowfilter_entry,
             val_flowfilter_entry, val_flowfilter_entry, uint32_t>
      (&key_vbrflowfilterentry, &val_vbrflowfilterentry,
                       &val_vbrflowfilterentry, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, VbrFlowFilterEntry_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_flowfilter_t key_obj;
  val_flowfilter_t val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.vbr_key.vbridge_name));
  key_obj.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_flowfilter_t,
             val_flowfilter_t, val_flowfilter_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vbr_flowfilter_entry key_vbrflowfilterentry;
  val_flowfilter_entry val_vbrflowfilterentry;
  memset(&key_vbrflowfilterentry, 0, sizeof(key_vbrflowfilterentry));
  memset(&val_vbrflowfilterentry, 0, sizeof(val_vbrflowfilterentry));
  memcpy(key_vbrflowfilterentry.flowfilter_key.vbr_key.vtn_key.vtn_name,
         "vtn1", sizeof(key_vbrflowfilterentry.flowfilter_key.vbr_key.
                        vtn_key.vtn_name));
  memcpy(key_vbrflowfilterentry.flowfilter_key.vbr_key.vbridge_name,
         "vbridge1", sizeof(key_vbrflowfilterentry.flowfilter_key.vbr_key.
                            vbridge_name));
  key_vbrflowfilterentry.flowfilter_key.direction = 1;
  key_vbrflowfilterentry.sequence_num = 20;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr_flowfilter_entry,
             val_flowfilter_entry, val_flowfilter_entry, uint32_t>
      (&key_vbrflowfilterentry, &val_vbrflowfilterentry,
                         &val_vbrflowfilterentry, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vbr_flowfilter_t, val_flowfilter_t,
                   val_flowfilter_t, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vbr_flowfilter_t, val_flowfilter_t,
       val_flowfilter_t, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.vbr_key.vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->
                                 vbr_key.vtn_key.vtn_name));
  pfc_log_info("Vtn Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vbr_key.vtn_key.vtn_name));
  pfc_log_info("Vbridge Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vbr_key.vbridge_name));
  pfc_log_info("Direction= %d", tmp_ptr->get_key_structure()->direction);
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  cfgptr1 = itr_ptr->NextItem();
  CacheElementUtil<key_vbr_flowfilter_entry, val_flowfilter_entry,
      val_flowfilter_entry, uint32_t> *tmp_ptr1 =
      static_cast<CacheElementUtil<key_vbr_flowfilter_entry,
      val_flowfilter_entry, val_flowfilter_entry, uint32_t>*> (cfgptr1);
  EXPECT_STREQ(reinterpret_cast<char*>(key_vbrflowfilterentry.flowfilter_key.
                                       vbr_key.vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr1->get_key_structure()->
                                 flowfilter_key.vbr_key.vtn_key.vtn_name));
  pfc_log_info("Entry:Vtn Name= %s", reinterpret_cast<char*>(
        tmp_ptr1->get_key_structure()->
        flowfilter_key.vbr_key.vtn_key.vtn_name));
  pfc_log_info("Entry:Vbridge Name= %s", reinterpret_cast<char*>(
        tmp_ptr1->get_key_structure()->flowfilter_key.vbr_key.vbridge_name));
  pfc_log_info("Entry:Direction= %d", tmp_ptr1->get_key_structure()->
               flowfilter_key.direction);
  pfc_log_info("Entry:SequenceNumber= %d", tmp_ptr1->get_key_structure()->
               sequence_num);
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr1->get_key_name().c_str(), tmp_ptr1->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr1->get_parent_key_name().c_str(),
               tmp_ptr1->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, VbrFlowFilterEntry_Success) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_flowfilter_t key_obj;
  val_flowfilter_t val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.vbr_key.vbridge_name));
  key_obj.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_flowfilter_t,
      val_flowfilter_t, val_flowfilter_t, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->add_child_to_hash(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vbr_flowfilter_entry key_vbrflowfilterentry;
  val_flowfilter_entry val_vbrflowfilterentry;
  memset(&key_vbrflowfilterentry, 0, sizeof(key_vbrflowfilterentry));
  memset(&val_vbrflowfilterentry, 0, sizeof(val_vbrflowfilterentry));
  memcpy(key_vbrflowfilterentry.flowfilter_key.vbr_key.vtn_key.vtn_name,
         "vtn1", sizeof(key_vbrflowfilterentry.flowfilter_key.vbr_key.vtn_key.
                        vtn_name));
  memcpy(key_vbrflowfilterentry.flowfilter_key.vbr_key.vbridge_name,
         "vbridge1", sizeof(key_vbrflowfilterentry.flowfilter_key.vbr_key.
                            vbridge_name));
  key_vbrflowfilterentry.flowfilter_key.direction = 1;
  key_vbrflowfilterentry.sequence_num = 20;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr_flowfilter_entry,
             val_flowfilter_entry, val_flowfilter_entry, uint32_t>
      (&key_vbrflowfilterentry, &val_vbrflowfilterentry,
                      &val_vbrflowfilterentry, operation);

  ret = KeyTree_obj->add_child_to_hash(cfgptr1);
  delete KeyTree_obj;
  delete cfgptr;
  delete cfgptr1;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_audit_node, parent_not_exist_vbrflowfilter_entry) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  key_vbr_flowfilter_entry key_vbrflowfilterentry;
  val_flowfilter_entry val_vbrflowfilterentry;
  memset(&key_vbrflowfilterentry, 0, sizeof(key_vbrflowfilterentry));
  memset(&val_vbrflowfilterentry, 0, sizeof(val_vbrflowfilterentry));
  memcpy(key_vbrflowfilterentry.flowfilter_key.vbr_key.vtn_key.vtn_name,
         "vtn1", sizeof(key_vbrflowfilterentry.flowfilter_key.vbr_key.
                        vtn_key.vtn_name));
  memcpy(key_vbrflowfilterentry.flowfilter_key.vbr_key.vbridge_name,
         "vbridge1", sizeof(key_vbrflowfilterentry.flowfilter_key.
                            vbr_key.vbridge_name));
  key_vbrflowfilterentry.flowfilter_key.direction = 1;
  key_vbrflowfilterentry.sequence_num = 20;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_flowfilter_entry,
             val_flowfilter_entry, val_flowfilter_entry, uint32_t>
      (&key_vbrflowfilterentry, &val_vbrflowfilterentry,
                        &val_vbrflowfilterentry, operation);
  uint32_t ret = KeyTree_obj->append_audit_node(cfgptr);
  delete cfgptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(get_parenttype, VbrIfFlowFilter) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  unc_key_type_t keytype = KeyTree_obj->get_parenttype(UNC_KT_VBRIF_FLOWFILTER);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VBR_IF);
}

TEST(append_commit_node, VbrIfFlowFilter) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_if_flowfilter key_obj;
  pfcdrv_val_vbrif_vextif val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.if_key.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.if_key.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.if_key.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.if_key.vbr_key.vbridge_name));
  memcpy(key_obj.if_key.if_name, "Interface1", sizeof(key_obj.if_key.if_name));
  key_obj.direction = 2;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_if_flowfilter,
          pfcdrv_val_vbrif_vextif, pfcdrv_val_vbrif_vextif, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, VbrIfFlowFilter_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_if_flowfilter key_obj;
  pfcdrv_val_vbrif_vextif val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.if_key.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.if_key.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.if_key.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.if_key.vbr_key.vbridge_name));
  memcpy(key_obj.if_key.if_name, "Interface1", sizeof(key_obj.if_key.if_name));
  key_obj.direction = 2;
  val_obj.interface_type = 3;
  memcpy(val_obj.vexternal_name, "Vexterna1", sizeof(val_obj.vexternal_name));
  memcpy(val_obj.vext_if_name, "vextifname1", sizeof(val_obj.vext_if_name));
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_if_flowfilter,
             pfcdrv_val_vbrif_vextif, pfcdrv_val_vbrif_vextif, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vbr_if_flowfilter, pfcdrv_val_vbrif_vextif,
                 pfcdrv_val_vbrif_vextif, uint32_t> *tmp_ptr =
                 static_cast<CacheElementUtil<key_vbr_if_flowfilter,
      pfcdrv_val_vbrif_vextif, pfcdrv_val_vbrif_vextif, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.if_key.vbr_key.vtn_key.
                                       vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->
                                 if_key.vbr_key.vtn_key.vtn_name));
  pfc_log_info("vtnname= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.vbr_key.vtn_key.vtn_name));
  pfc_log_info("vbridgename= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.vbr_key.vbridge_name));
  pfc_log_info("InterfaceName= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.if_name));
  pfc_log_info("Direction= %d", tmp_ptr->get_key_structure()->direction);
  pfc_log_info("Interface_type= %d", tmp_ptr->get_val_structure()->
               interface_type);
  pfc_log_info("VexternalName= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->vexternal_name));
  pfc_log_info("VexternalIfName= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->vext_if_name));
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info(":ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, VbrIfFlowFilter_Success) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_if_flowfilter key_obj;
  pfcdrv_val_vbrif_vextif val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.if_key.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.if_key.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.if_key.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.if_key.vbr_key.vbridge_name));
  memcpy(key_obj.if_key.if_name, "Interface1", sizeof(key_obj.if_key.if_name));
  key_obj.direction = 2;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_if_flowfilter,
      pfcdrv_val_vbrif_vextif, pfcdrv_val_vbrif_vextif, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->add_child_to_hash(cfgptr);
  delete KeyTree_obj;
  delete cfgptr;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(get_parenttype, VbrIfFlowFilterEntry) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  unc_key_type_t keytype = KeyTree_obj->
      get_parenttype(UNC_KT_VBRIF_FLOWFILTER_ENTRY);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VBRIF_FLOWFILTER);
}

TEST(append_commit_node, VbrIfFlowFilterEntry) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_if_flowfilter key_obj;
  pfcdrv_val_vbrif_vextif val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.if_key.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.if_key.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.if_key.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.if_key.vbr_key.vbridge_name));
  memcpy(key_obj.if_key.if_name, "Interface1", sizeof(key_obj.if_key.if_name));
  key_obj.direction = 2;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_if_flowfilter,
             pfcdrv_val_vbrif_vextif, pfcdrv_val_vbrif_vextif, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vbr_if_flowfilter_entry key_vbrifflowfilterentry;
  pfcdrv_val_flowfilter_entry val_vbrifflowfilterentry;
  memset(&key_vbrifflowfilterentry, 0, sizeof(key_vbrifflowfilterentry));
  memset(&val_vbrifflowfilterentry, 0, sizeof(val_vbrifflowfilterentry));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.vbr_key.vtn_key.
         vtn_name, "vtn1", sizeof(key_vbrifflowfilterentry.flowfilter_key.
                                  if_key.vbr_key.vtn_key.vtn_name));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.vbr_key.vbridge_name,
         "vbridge1", sizeof(key_vbrifflowfilterentry.flowfilter_key.if_key.
                            vbr_key.vbridge_name));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.if_name,
         "Interface1", sizeof(key_vbrifflowfilterentry.flowfilter_key.
                              if_key.if_name));
  key_vbrifflowfilterentry.flowfilter_key.direction = 2;
  key_vbrifflowfilterentry.sequence_num = 3;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr_if_flowfilter_entry,
        pfcdrv_val_flowfilter_entry, pfcdrv_val_flowfilter_entry, uint32_t>
      (&key_vbrifflowfilterentry, &val_vbrifflowfilterentry,
                           &val_vbrifflowfilterentry, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, VbrIfFlowFilterEntry_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_if_flowfilter key_obj;
  pfcdrv_val_vbrif_vextif val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.if_key.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.if_key.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.if_key.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.if_key.vbr_key.vbridge_name));
  memcpy(key_obj.if_key.if_name, "Interface1", sizeof(key_obj.if_key.if_name));
  key_obj.direction = 2;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_if_flowfilter,
             pfcdrv_val_vbrif_vextif, pfcdrv_val_vbrif_vextif, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vbr_if_flowfilter_entry key_vbrifflowfilterentry;
  pfcdrv_val_flowfilter_entry val_vbrifflowfilterentry;
  memset(&key_vbrifflowfilterentry, 0, sizeof(key_vbrifflowfilterentry));
  memset(&val_vbrifflowfilterentry, 0, sizeof(val_vbrifflowfilterentry));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.vbr_key.vtn_key.
         vtn_name, "vtn1", sizeof(key_vbrifflowfilterentry.flowfilter_key.
                                  if_key.vbr_key.vtn_key.vtn_name));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.vbr_key.vbridge_name,
         "vbridge1", sizeof(key_vbrifflowfilterentry.flowfilter_key.
                            if_key.vbr_key.vbridge_name));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.if_name,
         "Interface1", sizeof(key_vbrifflowfilterentry.flowfilter_key.
                              if_key.if_name));
  key_vbrifflowfilterentry.flowfilter_key.direction = 2;
  key_vbrifflowfilterentry.sequence_num = 3;
  val_vbrifflowfilterentry.val_vbrif_vextif.interface_type = 4;
  val_vbrifflowfilterentry.val_ff_entry.priority = 5;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr_if_flowfilter_entry,
             pfcdrv_val_flowfilter_entry, pfcdrv_val_flowfilter_entry, uint32_t>
      (&key_vbrifflowfilterentry, &val_vbrifflowfilterentry,
                 &val_vbrifflowfilterentry, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vbr_if_flowfilter,
      pfcdrv_val_vbrif_vextif, pfcdrv_val_vbrif_vextif, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vbr_if_flowfilter,
      pfcdrv_val_vbrif_vextif, pfcdrv_val_vbrif_vextif, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.if_key.vbr_key.vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->
                                 if_key.vbr_key.vtn_key.vtn_name));
  pfc_log_info("vtnname= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.vbr_key.vtn_key.vtn_name));
  pfc_log_info("vbridgename= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.vbr_key.vbridge_name));
  pfc_log_info("InterfaceName= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.if_name));
  pfc_log_info("Direction= %d", tmp_ptr->get_key_structure()->direction);
  pfc_log_info("Interface_type= %d", tmp_ptr->get_val_structure()->
               interface_type);
  pfc_log_info("VexternalName= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->vexternal_name));
  pfc_log_info("VexternalIfName= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->vext_if_name));
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  cfgptr1 = itr_ptr->NextItem();
  CacheElementUtil<key_vbr_if_flowfilter_entry, pfcdrv_val_flowfilter_entry,
      pfcdrv_val_flowfilter_entry, uint32_t> *tmp_ptr1 =
      static_cast<CacheElementUtil<key_vbr_if_flowfilter_entry,
      pfcdrv_val_flowfilter_entry, pfcdrv_val_flowfilter_entry,
                              uint32_t>*> (cfgptr1);

  EXPECT_STREQ(reinterpret_cast<char*>(key_vbrifflowfilterentry.flowfilter_key.
                                       if_key.vbr_key.vtn_key.vtn_name),
       reinterpret_cast<char*>(tmp_ptr1->get_key_structure()->
                               flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
  pfc_log_info("Entry:vtnname= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->flowfilter_key.if_key.vbr_key.vtn_key.
          vtn_name));
  pfc_log_info("Entry:VbridgeName= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->flowfilter_key.if_key.vbr_key.
          vbridge_name));
  pfc_log_info("Entry:Interfacename= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->flowfilter_key.if_key.if_name));
  pfc_log_info("Entry:Direction= %d", tmp_ptr1->get_key_structure()->
               flowfilter_key.direction);
  pfc_log_info("Entry:sequence_num= %d", tmp_ptr1->get_key_structure()->
               sequence_num);
  pfc_log_info("Entry:interface_type= %d", tmp_ptr1->get_val_structure()->
               val_vbrif_vextif.interface_type);
  pfc_log_info("Entry:Priority= %d", tmp_ptr1->get_val_structure()->
               val_ff_entry.priority);
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr1->get_key_name().c_str(), tmp_ptr1->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr1->get_parent_key_name().c_str(),
               tmp_ptr1->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, VbrIfFlowFilterEntry_Success) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_if_flowfilter key_obj;
  pfcdrv_val_vbrif_vextif val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.if_key.vbr_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_obj.if_key.vbr_key.vtn_key.vtn_name));
  memcpy(key_obj.if_key.vbr_key.vbridge_name, "vbridge1",
         sizeof(key_obj.if_key.vbr_key.vbridge_name));
  memcpy(key_obj.if_key.if_name, "Interface1", sizeof(key_obj.if_key.if_name));
  key_obj.direction = 2;
  ConfigNode *cfgptr = new CacheElementUtil<key_vbr_if_flowfilter,
             pfcdrv_val_vbrif_vextif, pfcdrv_val_vbrif_vextif, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->add_child_to_hash(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vbr_if_flowfilter_entry key_vbrifflowfilterentry;
  pfcdrv_val_flowfilter_entry val_vbrifflowfilterentry;
  memset(&key_vbrifflowfilterentry, 0, sizeof(key_vbrifflowfilterentry));
  memset(&val_vbrifflowfilterentry, 0, sizeof(val_vbrifflowfilterentry));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.vbr_key.vtn_key.
         vtn_name, "vtn1", sizeof(key_vbrifflowfilterentry.flowfilter_key.
                                  if_key.vbr_key.vtn_key.vtn_name));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.vbr_key.vbridge_name,
         "vbridge1", sizeof(key_vbrifflowfilterentry.flowfilter_key.
                            if_key.vbr_key.vbridge_name));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.if_name,
         "Interface1", sizeof(key_vbrifflowfilterentry.flowfilter_key.
                              if_key.if_name));
  key_vbrifflowfilterentry.flowfilter_key.direction = 2;
  key_vbrifflowfilterentry.sequence_num = 3;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr_if_flowfilter_entry,
             pfcdrv_val_flowfilter_entry, pfcdrv_val_flowfilter_entry, uint32_t>
      (&key_vbrifflowfilterentry, &val_vbrifflowfilterentry,
                          &val_vbrifflowfilterentry, operation);

  ret = KeyTree_obj->add_child_to_hash(cfgptr1);
  delete KeyTree_obj;
  delete cfgptr;
  delete cfgptr1;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, parent_not_exist_vbrifflowfilterentry) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vbr_if_flowfilter_entry key_vbrifflowfilterentry;
  pfcdrv_val_flowfilter_entry val_vbrifflowfilterentry;
  memset(&key_vbrifflowfilterentry, 0, sizeof(key_vbrifflowfilterentry));
  memset(&val_vbrifflowfilterentry, 0, sizeof(val_vbrifflowfilterentry));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.vbr_key.vtn_key.
         vtn_name, "vtnr2", sizeof(key_vbrifflowfilterentry.flowfilter_key.
                                   if_key.vbr_key.vtn_key.vtn_name));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.
         vbr_key.vbridge_name,
         "vbridge2", sizeof(key_vbrifflowfilterentry.flowfilter_key.if_key.
                            vbr_key.vbridge_name));
  memcpy(key_vbrifflowfilterentry.flowfilter_key.if_key.if_name,
         "Interface3", sizeof(key_vbrifflowfilterentry.
                              flowfilter_key.if_key.if_name));
  key_vbrifflowfilterentry.flowfilter_key.direction = 4;
  key_vbrifflowfilterentry.sequence_num = 5;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr_if_flowfilter_entry,
             pfcdrv_val_flowfilter_entry, pfcdrv_val_flowfilter_entry, uint32_t>
      (&key_vbrifflowfilterentry, &val_vbrifflowfilterentry,
                    &val_vbrifflowfilterentry, operation);

  uint32_t ret = KeyTree_obj->append_audit_node(cfgptr1);
  delete KeyTree_obj;
  delete cfgptr1;
  KeyTree_obj = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(get_parenttype, Vterminal) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  unc_key_type_t keytype = KeyTree_obj->get_parenttype(UNC_KT_VTERMINAL);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VTN);
}

TEST(append_commit_node, Vterminal) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm key_obj;
  val_vterm val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  memcpy(key_obj.vterminal_name, "vtermina1", sizeof(key_obj.vterminal_name));
  memcpy(val_obj.controller_id, "odc1", sizeof(val_obj.controller_id));
  memcpy(val_obj.domain_id, "domain1", sizeof(val_obj.domain_id));
  memcpy(val_obj.vterm_description, "VterminalDescription",
         sizeof(val_obj.vterm_description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm,
                   val_vterm, val_vterm, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, Vterminal_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm key_obj;
  val_vterm val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  memcpy(key_obj.vterminal_name, "vtermina1", sizeof(key_obj.vterminal_name));
  memcpy(val_obj.controller_id, "odc1", sizeof(val_obj.controller_id));
  memcpy(val_obj.domain_id, "domain1", sizeof(val_obj.domain_id));
  memcpy(val_obj.vterm_description, "VterminalDescription",
         sizeof(val_obj.vterm_description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm, val_vterm,  val_vterm, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vterm, val_vterm,  val_vterm, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vterm,
                 val_vterm,  val_vterm, uint32_t>*> (cfgptr);
  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->
                                 vtn_key.vtn_name));
  pfc_log_info("vtnname= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vtn_key.vtn_name));
  pfc_log_info("vterminalname= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vterminal_name));
  pfc_log_info("controllerid= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->controller_id));
  pfc_log_info("domainid= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->domain_id));
  pfc_log_info("vterm_description= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->vterm_description));
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, Vterminal) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm key_obj;
  val_vterm val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  memcpy(key_obj.vterminal_name, "vtermina1", sizeof(key_obj.vterminal_name));
  memcpy(val_obj.controller_id, "odc1", sizeof(val_obj.controller_id));
  memcpy(val_obj.domain_id, "domain1", sizeof(val_obj.domain_id));
  memcpy(val_obj.vterm_description, "VterminalDescription",
         sizeof(val_obj.vterm_description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm,
                 val_vterm, val_vterm, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->add_child_to_hash(cfgptr);
  delete KeyTree_obj;
  delete cfgptr;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(get_parenttype, VterminalIf) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  unc_key_type_t keytype = KeyTree_obj->get_parenttype(UNC_KT_VTERM_IF);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VTERMINAL);
}

TEST(append_commit_node, VterminalIf) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm key_obj;
  val_vterm val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  memcpy(key_obj.vterminal_name, "vtermina1", sizeof(key_obj.vterminal_name));
  memcpy(val_obj.controller_id, "odc1", sizeof(val_obj.controller_id));
  memcpy(val_obj.domain_id, "domain1", sizeof(val_obj.domain_id));
  memcpy(val_obj.vterm_description, "VterminalDescription",
         sizeof(val_obj.vterm_description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm,
                       val_vterm, val_vterm, uint32_t>
      (&key_obj, &val_obj,  &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vterm_if key_vtermif;
  val_vterm_if val_vtermif;
  memset(&key_vtermif, 0, sizeof(key_vtermif));
  memset(&val_vtermif, 0, sizeof(val_vtermif));
  memcpy(key_vtermif.vterm_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_vtermif.vterm_key.vtn_key.vtn_name));
  memcpy(key_vtermif.vterm_key.vterminal_name, "vtermina1",
         sizeof(key_vtermif.vterm_key.vterminal_name));
  memcpy(key_vtermif.if_name, "vterminalIf", sizeof(key_vtermif.if_name));
  val_vtermif.admin_status = 1;
  val_vtermif.portmap.vlan_id = 567;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vterm_if,
             val_vterm_if, val_vterm_if, uint32_t>
      (&key_vtermif, &val_vtermif, &val_vtermif, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, VterminalIf_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm key_obj;
  val_vterm val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  memcpy(key_obj.vterminal_name, "vtermina1", sizeof(key_obj.vterminal_name));
  memcpy(val_obj.controller_id, "odc1", sizeof(val_obj.controller_id));
  memcpy(val_obj.domain_id, "domain1", sizeof(val_obj.domain_id));
  memcpy(val_obj.vterm_description, "VterminalDescription",
         sizeof(val_obj.vterm_description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm,
                    val_vterm, val_vterm, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vterm, val_vterm, val_vterm, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vterm, val_vterm,
                          val_vterm, uint32_t>*> (cfgptr);
  EXPECT_STREQ(reinterpret_cast<char*>(key_obj.vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->
                                 vtn_key.vtn_name));
  pfc_log_info("vtnname= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vtn_key.vtn_name));
  pfc_log_info("vterminalname= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->vterminal_name));
  pfc_log_info("controllerid= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->controller_id));
  pfc_log_info("domainid= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->domain_id));
  pfc_log_info("vterm_description= %s", reinterpret_cast<char*>(
          tmp_ptr->get_val_structure()->vterm_description));
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  key_vterm_if key_vtermif;
  val_vterm_if val_vtermif;
  memset(&key_vtermif, 0, sizeof(key_vtermif));
  memset(&val_vtermif, 0, sizeof(val_vtermif));
  memcpy(key_vtermif.vterm_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_vtermif.vterm_key.vtn_key.vtn_name));
  memcpy(key_vtermif.vterm_key.vterminal_name, "vtermina1",
         sizeof(key_vtermif.vterm_key.vterminal_name));
  memcpy(key_vtermif.if_name, "vterminalIf", sizeof(key_vtermif.if_name));
  val_vtermif.admin_status = 1;
  val_vtermif.portmap.vlan_id = 567;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vterm_if,
             val_vterm_if, val_vterm_if, uint32_t>
      (&key_vtermif, &val_vtermif, &val_vtermif, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  cfgptr1 = itr_ptr->NextItem();
  CacheElementUtil<key_vterm_if, val_vterm_if,
               val_vterm_if, uint32_t> *tmp_ptr1 =
      static_cast<CacheElementUtil<key_vterm_if,
      val_vterm_if, val_vterm_if, uint32_t>*> (cfgptr1);
  EXPECT_STREQ(reinterpret_cast<char*>(key_vtermif.vterm_key.vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr1->get_key_structure()->
                                 vterm_key.vtn_key.vtn_name));
  pfc_log_info("VtermIF:vtnname= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->vterm_key.vtn_key.vtn_name));
  pfc_log_info("VtermIF:vterminalname= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->vterm_key.vterminal_name));
  pfc_log_info("VtermIF:VterminalIfName= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->if_name));
  pfc_log_info("VtermIF:AdminStatus= %d", tmp_ptr1->get_val_structure()->
               admin_status);
  pfc_log_info("VtermIF:VlanId= %d", tmp_ptr1->get_val_structure()->
               portmap.vlan_id);
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr1->get_key_name().c_str(), tmp_ptr1->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr1->get_parent_key_name().c_str(),
               tmp_ptr1->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, VterminalIf) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm key_obj;
  val_vterm val_obj;
  memset(&key_obj, 0, sizeof(key_obj));
  memset(&val_obj, 0, sizeof(val_obj));
  memcpy(key_obj.vtn_key.vtn_name, "vtn1", sizeof(key_obj.vtn_key.vtn_name));
  memcpy(key_obj.vterminal_name, "vtermina1", sizeof(key_obj.vterminal_name));
  memcpy(val_obj.controller_id, "odc1", sizeof(val_obj.controller_id));
  memcpy(val_obj.domain_id, "domain1", sizeof(val_obj.domain_id));
  memcpy(val_obj.vterm_description, "VterminalDescription",
         sizeof(val_obj.vterm_description));
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm, val_vterm,
           val_vterm, uint32_t>(&key_obj, &val_obj, &val_obj, operation);

  uint32_t ret = KeyTree_obj->add_child_to_hash(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vterm_if key_vtermif;
  val_vterm_if val_vtermif;
  memset(&key_vtermif, 0, sizeof(key_vtermif));
  memset(&val_vtermif, 0, sizeof(val_vtermif));
  memcpy(key_vtermif.vterm_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_vtermif.vterm_key.vtn_key.vtn_name));
  memcpy(key_vtermif.vterm_key.vterminal_name, "vtermina1",
         sizeof(key_vtermif.vterm_key.vterminal_name));
  memcpy(key_vtermif.if_name, "vterminalIf", sizeof(key_vtermif.if_name));
  val_vtermif.admin_status = 1;
  val_vtermif.portmap.vlan_id = 567;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vterm_if,
             val_vterm_if, val_vterm_if, uint32_t>
      (&key_vtermif, &val_vtermif, &val_vtermif, operation);

  ret = KeyTree_obj->add_child_to_hash(cfgptr1);
  delete KeyTree_obj;
  delete cfgptr;
  delete cfgptr1;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, Parent_not_exist_VterminalIf) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  key_vterm_if key_vtermif;
  val_vterm_if val_vtermif;
  memset(&key_vtermif, 0, sizeof(key_vtermif));
  memset(&val_vtermif, 0, sizeof(val_vtermif));
  memcpy(key_vtermif.vterm_key.vtn_key.vtn_name, "vtn1",
         sizeof(key_vtermif.vterm_key.vtn_key.vtn_name));
  memcpy(key_vtermif.vterm_key.vterminal_name, "vtermina1",
         sizeof(key_vtermif.vterm_key.vterminal_name));
  memcpy(key_vtermif.if_name, "vterminalIf", sizeof(key_vtermif.if_name));
  val_vtermif.admin_status = 1;
  val_vtermif.portmap.vlan_id = 567;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vterm_if,
             val_vterm_if, val_vterm_if, uint32_t>
      (&key_vtermif, &val_vtermif, &val_vtermif, operation);

  uint32_t ret = KeyTree_obj->append_audit_node(cfgptr1);
  delete KeyTree_obj;
  delete cfgptr1;
  KeyTree_obj = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(get_parenttype, VterminalIf_flowfilter) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  unc_key_type_t keytype = KeyTree_obj->
      get_parenttype(UNC_KT_VTERMIF_FLOWFILTER);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VTERM_IF);
}

TEST(get_parenttype, VterminalIf_flowfilter_entry) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  unc_key_type_t keytype = KeyTree_obj->
      get_parenttype(UNC_KT_VTERMIF_FLOWFILTER_ENTRY);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(keytype, UNC_KT_VTERMIF_FLOWFILTER);
}

TEST(append_commit_node, vterminalif_flowfilter) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm_if_flowfilter key_vtermifflowfilter;;
  val_flowfilter val_vtermifflowfilter;
  memset(&key_vtermifflowfilter, 0, sizeof(key_vtermifflowfilter));
  memset(&val_vtermifflowfilter, 0, sizeof(val_vtermifflowfilter));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vtn_key.vtn_name,
         "vtn1", sizeof(key_vtermifflowfilter.if_key.vterm_key.vtn_key.
                        vtn_name));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vterminal_name,
         "vtermina1", sizeof(key_vtermifflowfilter.if_key.vterm_key.
                             vterminal_name));
  memcpy(key_vtermifflowfilter.if_key.if_name, "VtermIfFFinterface",
         sizeof(key_vtermifflowfilter.if_key.if_name));
  key_vtermifflowfilter.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm_if_flowfilter,
             val_flowfilter, val_flowfilter, uint32_t>
      (&key_vtermifflowfilter, &val_vtermifflowfilter, &val_vtermifflowfilter, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, vterminalif_flowfilter) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm_if_flowfilter key_vtermifflowfilter;;
  val_flowfilter val_vtermifflowfilter;
  memset(&key_vtermifflowfilter, 0, sizeof(key_vtermifflowfilter));
  memset(&val_vtermifflowfilter, 0, sizeof(val_vtermifflowfilter));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vtn_key.vtn_name,
         "vtn1", sizeof(key_vtermifflowfilter.if_key.vterm_key.vtn_key.
                        vtn_name));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vterminal_name,
         "vtermina1", sizeof(key_vtermifflowfilter.if_key.vterm_key.
                             vterminal_name));
  memcpy(key_vtermifflowfilter.if_key.if_name, "VtermIfFFinterface",
         sizeof(key_vtermifflowfilter.if_key.if_name));
  key_vtermifflowfilter.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm_if_flowfilter,
             val_flowfilter, val_flowfilter,  uint32_t>
      (&key_vtermifflowfilter, &val_vtermifflowfilter,
                         &val_vtermifflowfilter, operation);

  uint32_t ret = KeyTree_obj->add_child_to_hash(cfgptr);
  delete cfgptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, Vterminalif_Flowfilter_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm_if_flowfilter key_vtermifflowfilter;;
  val_flowfilter val_vtermifflowfilter;
  memset(&key_vtermifflowfilter, 0, sizeof(key_vtermifflowfilter));
  memset(&val_vtermifflowfilter, 0, sizeof(val_vtermifflowfilter));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vtn_key.vtn_name,
         "vtn1", sizeof(key_vtermifflowfilter.if_key.vterm_key.vtn_key.
                        vtn_name));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vterminal_name,
         "vtermina1", sizeof(key_vtermifflowfilter.if_key.vterm_key.
                             vterminal_name));
  memcpy(key_vtermifflowfilter.if_key.if_name, "VtermIfFFinterface",
         sizeof(key_vtermifflowfilter.if_key.if_name));
  key_vtermifflowfilter.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm_if_flowfilter,
             val_flowfilter,  val_flowfilter, uint32_t>
      (&key_vtermifflowfilter, &val_vtermifflowfilter,
                        &val_vtermifflowfilter, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vterm_if_flowfilter, val_flowfilter,
                   val_flowfilter, uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vterm_if_flowfilter,
      val_flowfilter, val_flowfilter, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_vtermifflowfilter.if_key.vterm_key.
                                       vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->
                                 if_key.vterm_key.vtn_key.vtn_name));
  pfc_log_info("Vtn Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.vterm_key.vtn_key.vtn_name));
  pfc_log_info("Vterminal Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.vterm_key.vterminal_name));
  pfc_log_info("VterminalFFInterface Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.if_name));
  pfc_log_info("VterminalFF Direction= %d",
          tmp_ptr->get_key_structure()->direction);
  pfc_log_info("Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_commit_node, vterminalif_flowfilter_entry) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm_if_flowfilter key_vtermifflowfilter;;
  val_flowfilter val_vtermifflowfilter;
  memset(&key_vtermifflowfilter, 0, sizeof(key_vtermifflowfilter));
  memset(&val_vtermifflowfilter, 0, sizeof(val_vtermifflowfilter));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vtn_key.vtn_name,
         "vtn1", sizeof(key_vtermifflowfilter.if_key.vterm_key.vtn_key.
                        vtn_name));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vterminal_name,
         "vtermina1", sizeof(key_vtermifflowfilter.if_key.vterm_key.
                             vterminal_name));
  memcpy(key_vtermifflowfilter.if_key.if_name, "VtermIfFFinterface",
         sizeof(key_vtermifflowfilter.if_key.if_name));
  key_vtermifflowfilter.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm_if_flowfilter,
             val_flowfilter, val_flowfilter, uint32_t>
      (&key_vtermifflowfilter, &val_vtermifflowfilter,
                   &val_vtermifflowfilter, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vterm_if_flowfilter_entry key_vtermiflowfilterentry;
  val_flowfilter_entry val_vtermiflowfilterentry;
  memset(&key_vtermiflowfilterentry, 0, sizeof(key_vtermiflowfilterentry));
  memset(&val_vtermiflowfilterentry, 0, sizeof(val_vtermiflowfilterentry));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.vterm_key.vtn_key.
         vtn_name, "vtn1", sizeof(key_vtermiflowfilterentry.flowfilter_key.
                                  if_key.vterm_key.vtn_key.vtn_name));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.vterm_key.
         vterminal_name, "vtermina1", sizeof(key_vtermiflowfilterentry.
                                             flowfilter_key.if_key.vterm_key.
                                             vterminal_name));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.if_name,
         "VtermIfFFinterface", sizeof(key_vtermiflowfilterentry.flowfilter_key.
                                      if_key.if_name));
  key_vtermiflowfilterentry.flowfilter_key.direction = 1;
  key_vtermiflowfilterentry.sequence_num = 33;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vterm_if_flowfilter_entry,
             val_flowfilter_entry, val_flowfilter_entry, uint32_t>
      (&key_vtermiflowfilterentry, &val_vtermiflowfilterentry,
                   &val_vtermiflowfilterentry, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_hash, vterminalif_flowfilter_entry) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm_if_flowfilter key_vtermifflowfilter;;
  val_flowfilter val_vtermifflowfilter;
  memset(&key_vtermifflowfilter, 0, sizeof(key_vtermifflowfilter));
  memset(&val_vtermifflowfilter, 0, sizeof(val_vtermifflowfilter));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vtn_key.vtn_name,
         "vtn1", sizeof(key_vtermifflowfilter.if_key.vterm_key.vtn_key.
                        vtn_name));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vterminal_name,
         "vtermina1", sizeof(key_vtermifflowfilter.if_key.vterm_key.
                             vterminal_name));
  memcpy(key_vtermifflowfilter.if_key.if_name, "VtermIfFFinterface",
         sizeof(key_vtermifflowfilter.if_key.if_name));
  key_vtermifflowfilter.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm_if_flowfilter,
             val_flowfilter, val_flowfilter, uint32_t>
   (&key_vtermifflowfilter, &val_vtermifflowfilter, &val_vtermifflowfilter, operation);
  uint32_t ret = KeyTree_obj->add_child_to_hash(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vterm_if_flowfilter_entry key_vtermiflowfilterentry;
  val_flowfilter_entry val_vtermiflowfilterentry;
  memset(&key_vtermiflowfilterentry, 0, sizeof(key_vtermiflowfilterentry));
  memset(&val_vtermiflowfilterentry, 0, sizeof(val_vtermiflowfilterentry));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.vterm_key.vtn_key.
         vtn_name, "vtn1", sizeof(key_vtermiflowfilterentry.flowfilter_key.
                                  if_key.vterm_key.vtn_key.vtn_name));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.vterm_key.
         vterminal_name, "vtermina1", sizeof(key_vtermiflowfilterentry.
                                             flowfilter_key.if_key.vterm_key.
                                             vterminal_name));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.if_name,
         "VtermIfFFinterface", sizeof(key_vtermiflowfilterentry.
                                      flowfilter_key.if_key.if_name));
  key_vtermiflowfilterentry.flowfilter_key.direction = 1;
  key_vtermiflowfilterentry.sequence_num = 33;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vterm_if_flowfilter_entry,
             val_flowfilter_entry, val_flowfilter_entry, uint32_t>
      (&key_vtermiflowfilterentry, &val_vtermiflowfilterentry,
                &val_vtermiflowfilterentry, operation);

  ret = KeyTree_obj->add_child_to_hash(cfgptr1);
  delete cfgptr;
  delete cfgptr1;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(append_audit_node, parent_not_exist_vterminalif_flowfilter_entry) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();
  key_vterm_if_flowfilter_entry key_vtermiflowfilterentry;
  val_flowfilter_entry val_vtermiflowfilterentry;
  memset(&key_vtermiflowfilterentry, 0, sizeof(key_vtermiflowfilterentry));
  memset(&val_vtermiflowfilterentry, 0, sizeof(val_vtermiflowfilterentry));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.vterm_key.vtn_key.
         vtn_name, "vtn1", sizeof(key_vtermiflowfilterentry.flowfilter_key.
                                  if_key.vterm_key.vtn_key.vtn_name));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.vterm_key.
         vterminal_name, "vtermina1", sizeof(key_vtermiflowfilterentry.
                                             flowfilter_key.if_key.
                                             vterm_key.vterminal_name));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.if_name,
         "VtermIfFFinterface",
         sizeof(key_vtermiflowfilterentry.flowfilter_key.if_key.if_name));
  key_vtermiflowfilterentry.flowfilter_key.direction = 1;
  key_vtermiflowfilterentry.sequence_num = 33;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vterm_if_flowfilter_entry,
             val_flowfilter_entry, val_flowfilter_entry, uint32_t>
      (&key_vtermiflowfilterentry, &val_vtermiflowfilterentry,
                        &val_vtermiflowfilterentry, operation);

  uint32_t ret = KeyTree_obj->append_audit_node(cfgptr1);
  delete cfgptr1;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}

TEST(append_commit_node, Vterminalif_Flowfilter_Entry_Retrive) {
  int operation = 1;
  KeyTree* KeyTree_obj = NULL;
  KeyTree_obj = KeyTree::create_cache();

  key_vterm_if_flowfilter key_vtermifflowfilter;;
  val_flowfilter val_vtermifflowfilter;
  memset(&key_vtermifflowfilter, 0, sizeof(key_vtermifflowfilter));
  memset(&val_vtermifflowfilter, 0, sizeof(val_vtermifflowfilter));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vtn_key.vtn_name,
         "vtn1", sizeof(key_vtermifflowfilter.if_key.vterm_key.vtn_key.
                        vtn_name));
  memcpy(key_vtermifflowfilter.if_key.vterm_key.vterminal_name,
         "vtermina1", sizeof(key_vtermifflowfilter.if_key.
                             vterm_key.vterminal_name));
  memcpy(key_vtermifflowfilter.if_key.if_name, "VtermIfFFinterface",
         sizeof(key_vtermifflowfilter.if_key.if_name));
  key_vtermifflowfilter.direction = 1;
  ConfigNode *cfgptr = new CacheElementUtil<key_vterm_if_flowfilter,
      val_flowfilter, val_flowfilter, uint32_t>
      (&key_vtermifflowfilter, &val_vtermifflowfilter,
                &val_vtermifflowfilter, operation);

  uint32_t ret = KeyTree_obj->append_commit_node(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  key_vterm_if_flowfilter_entry key_vtermiflowfilterentry;
  val_flowfilter_entry val_vtermiflowfilterentry;
  memset(&key_vtermiflowfilterentry, 0, sizeof(key_vtermiflowfilterentry));
  memset(&val_vtermiflowfilterentry, 0, sizeof(val_vtermiflowfilterentry));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.vterm_key.
         vtn_key.vtn_name, "vtn1",
         sizeof(key_vtermiflowfilterentry.flowfilter_key.if_key.
                vterm_key.vtn_key.vtn_name));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.vterm_key.
         vterminal_name, "vtermina1",
         sizeof(key_vtermiflowfilterentry.flowfilter_key.if_key.vterm_key.
                vterminal_name));
  memcpy(key_vtermiflowfilterentry.flowfilter_key.if_key.if_name,
         "VtermIfFFinterface",
         sizeof(key_vtermiflowfilterentry.flowfilter_key.if_key.if_name));
  key_vtermiflowfilterentry.flowfilter_key.direction = 1;
  key_vtermiflowfilterentry.sequence_num = 33;
  ConfigNode *cfgptr1 = new CacheElementUtil<key_vterm_if_flowfilter_entry,
             val_flowfilter_entry, val_flowfilter_entry, uint32_t>
      (&key_vtermiflowfilterentry, &val_vtermiflowfilterentry,
                       &val_vtermiflowfilterentry, operation);

  ret = KeyTree_obj->append_commit_node(cfgptr1);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
  CommonIterator* itr_ptr = KeyTree_obj->create_iterator();
  cfgptr = itr_ptr->FirstItem();
  CacheElementUtil<key_vterm_if_flowfilter, val_flowfilter, val_flowfilter,
                             uint32_t> *tmp_ptr =
      static_cast<CacheElementUtil<key_vterm_if_flowfilter, val_flowfilter,
         val_flowfilter, uint32_t>*> (cfgptr);

  EXPECT_STREQ(reinterpret_cast<char*>(key_vtermifflowfilter.if_key.vterm_key.
                                       vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr->get_key_structure()->
                                 if_key.vterm_key.vtn_key.vtn_name));
  pfc_log_info("FF:Vtn Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.vterm_key.vtn_key.vtn_name));
  pfc_log_info("FF:Vterminal Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.vterm_key.vterminal_name));
  pfc_log_info("FF:VterminalFFInterface Name= %s", reinterpret_cast<char*>(
          tmp_ptr->get_key_structure()->if_key.if_name));
  pfc_log_info("FF:VterminalFF Direction= %d",
          tmp_ptr->get_key_structure()->direction);
  pfc_log_info("FF:Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr->get_key_name().c_str(), tmp_ptr->get_type_name());;
  pfc_log_info("FF:ParentKey %s SearchKey %s",
               tmp_ptr->get_parent_key_name().c_str(),
               tmp_ptr->get_key_generate().c_str());
  cfgptr1 = itr_ptr->NextItem();
  CacheElementUtil<key_vterm_if_flowfilter_entry, val_flowfilter_entry,
         val_flowfilter_entry, uint32_t> *tmp_ptr1 =
      static_cast<CacheElementUtil<key_vterm_if_flowfilter_entry,
      val_flowfilter_entry, val_flowfilter_entry, uint32_t>*> (cfgptr1);
  EXPECT_STREQ(reinterpret_cast<char*>(key_vtermiflowfilterentry.
                                       flowfilter_key.if_key.vterm_key.
                                       vtn_key.vtn_name),
         reinterpret_cast<char*>(tmp_ptr1->get_key_structure()->
                                 flowfilter_key.if_key.vterm_key.
                                 vtn_key.vtn_name));
  pfc_log_info("FFEntry:Vtn Name= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->
          flowfilter_key.if_key.vterm_key.vtn_key.vtn_name));
  pfc_log_info("FFEntry:Vterminal Name= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->
          flowfilter_key.if_key.vterm_key.vterminal_name));
  pfc_log_info("FFEntry:VterminalFFInterface Name= %s", reinterpret_cast<char*>(
          tmp_ptr1->get_key_structure()->flowfilter_key.if_key.if_name));
  pfc_log_info("FFEntry:VterminalFF Direction= %d",
          tmp_ptr1->get_key_structure()->flowfilter_key.direction);
  pfc_log_info("FFEntry:VterminalFF Sequence Number= %d",
          tmp_ptr1->get_key_structure()->sequence_num);
  pfc_log_info("FFEntry:Node Present in Tree ..for:%s keytype %d",
                 tmp_ptr1->get_key_name().c_str(), tmp_ptr1->get_type_name());;
  pfc_log_info("FFEntry:ParentKey %s SearchKey %s",
               tmp_ptr1->get_parent_key_name().c_str(),
               tmp_ptr1->get_key_generate().c_str());
  delete itr_ptr;
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  cfgptr1 = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}
}  // namespace vtndrvcache
}  // namespace unc
