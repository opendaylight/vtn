/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <vtn_cache_mod.hh>
#include <keytree.hh>
#include <vtn_conf_data_element_op.hh>

namespace unc {
namespace vtndrvcache {

// Initialize vtncacheutil module before all tests.
class DrvCacheEnvironment
  : public ::testing::Environment
{
public:
  explicit DrvCacheEnvironment() : drvcache(NULL) {}

  void SetUp(void)
  {
    drvcache.init();
  }

  void TearDown(void) {}

private:
  VtnDrvCacheMod  drvcache;
};

static DrvCacheEnvironment    *drvCacheEnv = new DrvCacheEnvironment();
::testing::Environment  *globalEnv =
  ::testing::AddGlobalTestEnvironment(drvCacheEnv);

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

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn,
        val_vtn, uint32_t>(&key_obj, &val_obj, &val_obj, operation);
  KeyTree_obj->append_audit_node(cfgptr);

  ConfigNode *cfgnode_obj = new ConfigNode;
  int level_index = 1;
  cfgnode_obj->add_child_to_list(cfgptr);
  cfgnode_obj->print(level_index);
  delete cfgnode_obj;
  delete KeyTree_obj;
  cfgnode_obj = NULL;
  cfgptr = NULL;
  KeyTree_obj = NULL;
}

TEST(add_child_to_list, UNC_RC_SUCCESS) {
  ConfigNode *cfgnode_obj = new ConfigNode;
  uint32_t operation = 1;
  KeyTree *KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
                       (&key_obj, &val_obj, &val_obj, operation);
  KeyTree_obj->append_audit_node(cfgptr);

  int ret = cfgnode_obj->add_child_to_list(cfgptr);
  delete cfgnode_obj;
  delete KeyTree_obj;
  cfgnode_obj = NULL;
  cfgptr = NULL;
  KeyTree_obj = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}

TEST(add_child_to_list, UNC_DRV_RC_ERR_GENERIC) {
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
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
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

  ConfigNode *cfgptr = new CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
                       (&key_obj, &val_obj, &val_obj, operation);
  int ret = cfg_obj->add_child_to_list(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  ConfigNode *cfgptr1 = new CacheElementUtil<key_vbr, val_vbr, val_vbr,
                 uint32_t>(&key1_obj, &val1_obj, &val1_obj, operation);
  ret = cfg_obj->add_child_to_list(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  ConfigNode *cfgptr2 = new CacheElementUtil<key_vbr_if, pfcdrv_val_vbr_if,
      pfcdrv_val_vbr_if, uint32_t>(&key2_obj, &val2_obj, &val2_obj, operation);
  ret = cfg_obj->add_child_to_list(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

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
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}
TEST(TypeToStrFun, keytype_switch_port) {
  std::string ret = TypeToStrFun(UNC_KT_VBR_VLANMAP);
  EXPECT_EQ(ret, "UNC_KT_VBR_VLANMAP");

  ret = TypeToStrFun(UNC_KT_SWITCH);
  EXPECT_EQ(ret, "UNC_KT_SWITCH");

  ret = TypeToStrFun(UNC_KT_PORT);
  EXPECT_EQ(ret, "UNC_KT_PORT");
}
TEST(delete_child_node, empty_switch_port) {
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  std::vector<key_information> erased_key_list;
  ConfigNode* cfgptr2 = NULL;
  uint32_t ret = KeyTree_obj->node_tree_.delete_child_node(cfgptr2,
                                                  erased_key_list);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}
TEST(delete_child_node, switch_port_config_not_present) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  std::vector<key_information> erased_key_list;
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
                      key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
                                      key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  ConfigNode *cfgptr = new CacheElementUtil<key_switch, val_switch_st,
       val_switch_st, uint32_t>(&key_switch_obj, &val_switch, &val_switch,
                                                 operation);
  uint32_t ret = KeyTree_obj->node_tree_.add_child_to_list(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  key_port key_obj;
  memcpy(key_obj.sw_key.ctr_key.controller_name, "odc1", sizeof(
                       key_obj.sw_key.ctr_key.controller_name));
  memcpy(key_obj.sw_key.switch_id, "0000-0000-0000-0001", sizeof(
                                      key_obj.sw_key.switch_id));
  memcpy(key_obj.port_id, "s2-eth3", sizeof(key_obj.port_id));

  val_port_st val_obj;
  memset(&val_obj, 0, sizeof(val_obj));
  ConfigNode *cfgptr1 = new CacheElementUtil<key_port, val_port_st, val_port_st,
                      uint32_t>(&key_obj, &val_obj, &val_obj, operation);
  ret = KeyTree_obj->node_tree_.delete_child_node(cfgptr1, erased_key_list);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  delete cfgptr1;
  cfgptr1 = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_DRV_RC_ERR_GENERIC);
}
TEST(delete_child_node, delete_switch) {
  int operation = 1;
  KeyTree* KeyTree_obj;
  KeyTree_obj = KeyTree::create_cache();
  std::vector<key_information> erased_key_list;
  //  add switch 0000-0000-0000-0001 to cache
  key_switch key_switch_obj;
  val_switch_st val_switch;
  memcpy(key_switch_obj.ctr_key.controller_name, "odc1", sizeof(
                       key_switch_obj.ctr_key.controller_name));
  memcpy(key_switch_obj.switch_id, "0000-0000-0000-0001", sizeof(
                                      key_switch_obj.switch_id));
  memset(&val_switch, 0, sizeof(val_switch));
  ConfigNode *cfgptr = new CacheElementUtil<key_switch, val_switch_st,
       val_switch_st, uint32_t>(&key_switch_obj, &val_switch, &val_switch,
                                                             operation);
  uint32_t ret = KeyTree_obj->node_tree_.add_child_to_list(cfgptr);
  EXPECT_EQ(ret, UNC_RC_SUCCESS);

  ret = KeyTree_obj->node_tree_.delete_child_node(cfgptr, erased_key_list);
  delete KeyTree_obj;
  KeyTree_obj = NULL;
  cfgptr = NULL;
  EXPECT_EQ(ret, UNC_RC_SUCCESS);
}
TEST(TypeToStrFun, keytype_kt_link) {
  std::string ret = TypeToStrFun(UNC_KT_LINK);
  EXPECT_EQ(ret, "UNC_KT_LINK");
}
TEST(TypeToStrFun, key_flowlist) {
  std::string ret = TypeToStrFun(UNC_KT_FLOWLIST);
  EXPECT_EQ(ret, "UNC_KT_FLOWLIST");
}

TEST(TypeToStrFun, key_flowlist_entry) {
  std::string ret = TypeToStrFun(UNC_KT_FLOWLIST_ENTRY);
  EXPECT_EQ(ret, "UNC_KT_FLOWLIST_ENTRY");
}

TEST(TypeToStrFun, key_vtn_flowfilter) {
  std::string ret = TypeToStrFun(UNC_KT_VTN_FLOWFILTER);
  EXPECT_EQ(ret, "UNC_KT_VTN_FLOWFILTER");
}

TEST(TypeToStrFun, key_vtn_flowfilter_entry) {
  std::string ret = TypeToStrFun(UNC_KT_VTN_FLOWFILTER_ENTRY);
  EXPECT_EQ(ret, "UNC_KT_VTN_FLOWFILTER_ENTRY");
}

TEST(TypeToStrFun, key_vbr_flowfilter) {
  std::string ret = TypeToStrFun(UNC_KT_VBR_FLOWFILTER);
  EXPECT_EQ(ret, "UNC_KT_VBR_FLOWFILTER");
}

TEST(TypeToStrFun, key_vbr_flowfilter_entry) {
  std::string ret = TypeToStrFun(UNC_KT_VBR_FLOWFILTER_ENTRY);
  EXPECT_EQ(ret, "UNC_KT_VBR_FLOWFILTER_ENTRY");
}

TEST(TypeToStrFun, key_vbr_if_flowfilter) {
  std::string ret = TypeToStrFun(UNC_KT_VBRIF_FLOWFILTER);
  EXPECT_EQ(ret, "UNC_KT_VBRIF_FLOWFILTER");
}

TEST(TypeToStrFun, key_vbr_if_flowfilter_entry) {
  std::string ret = TypeToStrFun(UNC_KT_VBRIF_FLOWFILTER_ENTRY);
  EXPECT_EQ(ret, "UNC_KT_VBRIF_FLOWFILTER_ENTRY");
}

TEST(TypeToStrFun, key_vterm) {
  std::string ret = TypeToStrFun(UNC_KT_VTERMINAL);
  EXPECT_EQ(ret, "UNC_KT_VTERMINAL");
}

TEST(TypeToStrFun, key_vterm_if) {
  std::string ret = TypeToStrFun(UNC_KT_VTERM_IF);
  EXPECT_EQ(ret, "UNC_KT_VTERM_IF");
}

TEST(TypeToStrFun, key_vterm_if_flowfilter) {
  std::string ret = TypeToStrFun(UNC_KT_VTERMIF_FLOWFILTER);
  EXPECT_EQ(ret, "UNC_KT_VTERMIF_FLOWFILTER");
}

TEST(TypeToStrFun, key_vterm_if_flowfilter_entry) {
  std::string ret = TypeToStrFun(UNC_KT_VTERMIF_FLOWFILTER_ENTRY);
  EXPECT_EQ(ret, "UNC_KT_VTERMIF_FLOWFILTER_ENTRY");

  ret = TypeToStrFun(UNC_KT_VLINK);
  EXPECT_EQ(ret, "Unknown");
}
}  // namespace vtndrvcache
}  // namespace unc
