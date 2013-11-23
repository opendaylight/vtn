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
  delete cfgptr;
  delete cfgptr1;
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
  delete cfgptr2;
  delete cfgptr1;
  delete cfgptr;
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

  delete cfgptr;
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

  delete cfgptr1;
  delete cfgptr;
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
  delete cfgptr;
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
  delete cfgptr;
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
}  // namespace vtndrvcache
}  // namespace unc
