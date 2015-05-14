/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "keytree.hh"
#include "unc/keytype.h"

namespace unc {
namespace vtndrvcache {

/**
 * @brief   : Method to create Cache and return the pointer to it
 * @retval  : KeyTree*
 */
KeyTree* KeyTree::create_cache() {
  ODC_FUNC_TRACE;
  return  new KeyTree();
}

/**
 * @brief       : Method to get Iterator for iterating each element from tree
 * @retval      : CommonIterator*
 */
CommonIterator* KeyTree::create_iterator() {
  ODC_FUNC_TRACE;
  return new CommonIterator(this);
}

/**
 * @brief :  Keytree constructor
 */
KeyTree::KeyTree():cfgnode_count_(0) {
  ODC_FUNC_TRACE;
  ConfigHashArr[0].insert(std::pair <std::string, ConfigNode*>
                          ("ROOT", &node_tree_));
}

/**
 * @brief : Keytree destructor
 */
KeyTree::~KeyTree() {
  ODC_FUNC_TRACE;
  clear_root_cache();
  clear_audit_commit_cache();
  cfgnode_count_ = 0;
}

/**
 * @brief : Constructing parent/child relation for supported KT
 */
key_tree key_tree_table[] = {
  { UNC_KT_CONTROLLER, UNC_KT_ROOT, 0 },
  { UNC_KT_VTN, UNC_KT_ROOT, 1 },
  { UNC_KT_VBRIDGE, UNC_KT_VTN, 0 }, { UNC_KT_VBR_IF, UNC_KT_VBRIDGE, 0 },
  { UNC_KT_VBR_VLANMAP, UNC_KT_VBRIDGE, 1 },
  { UNC_KT_SWITCH, UNC_KT_ROOT, 0 },
  { UNC_KT_PORT, UNC_KT_SWITCH, 1 },
  { UNC_KT_LINK, UNC_KT_ROOT, 0 },
  { UNC_KT_FLOWLIST_ENTRY, UNC_KT_FLOWLIST, 0 },
  { UNC_KT_VTN_FLOWFILTER, UNC_KT_VTN, 0 },
  { UNC_KT_VTN_FLOWFILTER_ENTRY, UNC_KT_VTN_FLOWFILTER, 0 },
  { UNC_KT_VBR_FLOWFILTER, UNC_KT_VBRIDGE, 0},
  { UNC_KT_VBR_FLOWFILTER_ENTRY, UNC_KT_VBR_FLOWFILTER, 0 },
  { UNC_KT_VBRIF_FLOWFILTER, UNC_KT_VBR_IF, 0 },
  { UNC_KT_VBRIF_FLOWFILTER_ENTRY, UNC_KT_VBRIF_FLOWFILTER, 0 },
  { UNC_KT_VTERMINAL, UNC_KT_VTN, 6 },
  { UNC_KT_VTERM_IF, UNC_KT_VTERMINAL, 0},
  { UNC_KT_VTERMIF_FLOWFILTER, UNC_KT_VTERM_IF, 0},
  { UNC_KT_VTERMIF_FLOWFILTER_ENTRY, UNC_KT_VTERMIF_FLOWFILTER, 0}
};

/**
 * @brief       : Method to return the Keytype of Parent given the search Key
 * @param [in]  : child_type
 * @retval      : unc_key_type_t
 */
unc_key_type_t KeyTree::get_parenttype(unc_key_type_t child_type) {
  ODC_FUNC_TRACE;
  uint32_t table_size = sizeof(key_tree_table) / sizeof(key_tree);
  uint32_t entry_index = 0;
  unc_key_type_t parent_type = UNC_KT_ROOT;

  pfc_log_debug("Table size %d Index %d", table_size, entry_index);

  for (entry_index = 0; entry_index < table_size; entry_index++) {
    if (key_tree_table[entry_index].id == child_type) {
      parent_type = key_tree_table[entry_index].parent;
      break;
    }
  }
  return parent_type;
}

/**
 * @brief       : Method to traverse the tree and populate the nodes into
 *                the vector
 * @param [in]  : value_list
 * @retval      : UNC_RC_SUCCESS
 */
UncRespCode KeyTree::get_nodelist_keytree() {
  ODC_FUNC_TRACE;
  cfg_node_list_.clear();
  return node_tree_.get_node_list(cfg_node_list_);
}

/**
 * @brief       : Method to add individual confignode to the cache for commit
 * @param [in]  : value_node
 * @retval      : UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
 */
UncRespCode KeyTree::append_commit_node(ConfigNode* value_node) {
  ODC_FUNC_TRACE;
  cfg_node_list_.push_back(value_node);
  return UNC_RC_SUCCESS;
}

/**
 * @brief : Method to clear the elements of vector using for commit cache
 */
void KeyTree::clear_audit_commit_cache() {
  ODC_FUNC_TRACE;
  if (cfg_node_list_.empty()) {
    get_nodelist_keytree();
  }
  if (!cfg_node_list_.empty()) {
    std::vector<ConfigNode*>::iterator itr = cfg_node_list_.begin();
    std::vector<ConfigNode*>::iterator itr_end = cfg_node_list_.end();
    for (; itr != itr_end; ++itr) {
      if (*itr != NULL) {
        delete *itr;
        *itr = NULL;
      }
    }
    cfg_node_list_.clear();
  }
}

/**
 * @brief : Method to clear the elements of search map
 */
void KeyTree::clear_root_cache() {
  ODC_FUNC_TRACE;
  for (uint32_t ConfigArray  = 0; ConfigArray < CACHEMGR_CONFIGARR_SIZE;
       ConfigArray++) {
    ConfigHashArr[ConfigArray].clear();
  }
}

/**
 * @brief       : Method to add confignode lists to the cache for
 *              : Physical nodes
 * @param [in]  : value_list
 * @retval      : ERR_ADD_CHILD_TO_TREE_FAILED / UNC_RC_SUCCESSS
 */
UncRespCode KeyTree::append_physical_attribute_configuration_list(
    const std::vector<ConfigNode*>&value_list) {
  ODC_FUNC_TRACE;
  UncRespCode err = UNC_DRV_RC_ERR_GENERIC;
  ConfigNode*  tmp_cfgnode_ptr = NULL;
  std::vector<ConfigNode*>::const_iterator it = value_list.begin();
  std::vector<ConfigNode*>::const_iterator itr_end = value_list.end();
  // Iterate the vector of config nodes
  for (; it!= itr_end; it++) {
    if (*it == NULL) {
      pfc_log_error("RunningConfig::%s:%d: ConfigNode is NULL",
                    PFC_FUNCNAME, __LINE__);
      return UNC_DRV_RC_ERR_GENERIC;
    }

    tmp_cfgnode_ptr = *it;
    err = append_audit_node(tmp_cfgnode_ptr);
    if (UNC_RC_SUCCESS != err) {
      pfc_log_error("%s: AddChildToTree faild err=%d", PFC_FUNCNAME, err);
      tmp_cfgnode_ptr = NULL;
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  return UNC_RC_SUCCESS;
}

/**
 * @brief       : Method to add individual confignode to the cache for
 *              : Physical node
 * @param [in]  : value_node
 * @retval      : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode KeyTree::append_Physical_attribute_node(
    ConfigNode* value_node) {
  ODC_FUNC_TRACE;
  UncRespCode err = UNC_DRV_RC_ERR_GENERIC;
  err = append_audit_node(value_node);
  if (UNC_RC_SUCCESS != err) {
    pfc_log_error("%s: AddChildToTree faild err=%d", PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

/**
 * @brief       : Method to update confignode to the cache for physical-node
 * @param [in]  : child_ptr(confignode)
 * @retval      : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode KeyTree:: update_physical_attribute_node(
    ConfigNode* child_ptr) {
  ODC_FUNC_TRACE;
  if (NULL == child_ptr) {
    pfc_log_error("update_port_to_list:Child Node is NULL!!!!!!");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  unc_key_type_t key_type = child_ptr->get_type_name();
  std::string key_name= child_ptr->get_key_name();
  ConfigNode* old_cfgptr = get_node_from_hash(key_name, key_type);
  if (NULL == old_cfgptr) {
    pfc_log_error("no such configuration exist in cache");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string type_check = TypeToStrFun(child_ptr->get_type_name());
  pfc_log_info("type check for port/switch %s", type_check.c_str());
  if (type_check == "UNC_KT_PORT") {
    CacheElementUtil<key_port_t, val_port_st_t, val_port_st_t, uint32_t> *update_port_ptr =
        static_cast <CacheElementUtil
        <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * >(child_ptr);

    CacheElementUtil<key_port_t, val_port_st_t, val_port_st_t, uint32_t> *old_port_ptr =
        static_cast <CacheElementUtil
        <key_port_t, val_port_st_t, val_port_st_t, uint32_t> * >(old_cfgptr);

    old_port_ptr->set_val_structure(update_port_ptr->get_val_structure());
  } else if (type_check == "UNC_KT_SWITCH") {
    CacheElementUtil<key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>
        *update_switch_ptr = static_cast <CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * >(child_ptr);


    CacheElementUtil<key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> *old_switch_ptr =
        static_cast <CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * >(old_cfgptr);

    old_switch_ptr->set_val_structure(update_switch_ptr->get_val_structure());
  } else if (type_check == "UNC_KT_LINK") {
    CacheElementUtil<key_link_t, val_link_st_t, val_link_st_t, uint32_t> *update_link_ptr =
        static_cast <CacheElementUtil
        <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * >(child_ptr);

    CacheElementUtil<key_link_t, val_link_st_t, val_link_st_t, uint32_t> *old_link_ptr =
        static_cast <CacheElementUtil
        <key_link_t, val_link_st_t, val_link_st_t, uint32_t> * >(old_cfgptr);

    old_link_ptr->set_val_structure(update_link_ptr->get_val_structure());
  } else {
    pfc_log_error("unmatched update request");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  return UNC_RC_SUCCESS;
}

/**
 * @brief       : Method to delete confignode from the cache for physical node
 * @param [in]  : child_ptr(confignode)
 * @retval      : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode KeyTree:: delete_physical_attribute_node(
    ConfigNode* child_ptr) {
  ODC_FUNC_TRACE;
  UncRespCode err = UNC_DRV_RC_ERR_GENERIC;
  if (NULL == child_ptr) {
    pfc_log_error("delete_port_to_list:Child Node is NULL!!!!!!");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string  parent_key = child_ptr->get_parent_key_name();
  unc_key_type_t parent_type = get_parenttype(child_ptr->get_type_name());
  // Retrieve the parent from the map using parent_key & parent_type
  ConfigNode* parent_ptr = get_node_from_hash(parent_key, parent_type);
  if (NULL == parent_ptr) {
    pfc_log_error("Parent:%s  Not Present for:%s", parent_key.c_str(),
                  child_ptr->get_key_generate().c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  }
  unc_key_type_t key_type = child_ptr->get_type_name();
  std::string key_name= child_ptr->get_key_name();
  ConfigNode* old_cfgptr = get_node_from_hash(key_name, key_type);
  if (NULL == old_cfgptr) {
    pfc_log_error("no such configuration exist in cache");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string type_check = TypeToStrFun(child_ptr->get_type_name());
  pfc_log_info("type check for port/switch %s", type_check.c_str());
  std::vector<key_information> erased_key_list;
  if ((type_check == "UNC_KT_PORT") || (type_check == "UNC_KT_SWITCH") ||
      (type_check == "UNC_KT_LINK")) {
    err = parent_ptr->delete_child_node(old_cfgptr, erased_key_list);
    if ( UNC_RC_SUCCESS != err ) {
      pfc_log_error("delete_child_to_list Faild for:%s!!!!",
                    child_ptr->get_key_generate().c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
  cfg_node_list_.clear();
  } else {
    pfc_log_error("unmatched delete request");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::vector<key_information>:: iterator itr_err = erased_key_list.begin();
  for (; itr_err != erased_key_list.end(); ++itr_err) {
    pfc_log_debug("enter into hash remove %s %d", itr_err->key.c_str(),
                  itr_err->key_type);
    ConfigHashArr[itr_err->key_type].erase(itr_err->key);
  }
  return UNC_RC_SUCCESS;
}

/**
 * @brief       : Method to return existing config node as per compare node
 *              : attribute
 * @param [in]  : new_compare_node, old_node_for_update
 * @retval      : boolean(true/false)
 */
pfc_bool_t KeyTree::compare_is_physical_node_found(
    ConfigNode* new_compare_node,
    ConfigNode*& old_node_for_update) {
  ODC_FUNC_TRACE;
  pfc_bool_t found = PFC_FALSE;
  if (NULL == new_compare_node) {
    pfc_log_error("IsSwitchPortFound:Child Node is NULL!!!!!!");
    return found;
  }
  unc_key_type_t key_type = new_compare_node->get_type_name();
  std::string key_name= new_compare_node->get_key_name();
  ConfigNode* old_cfgptr = get_node_from_hash(key_name, key_type);
  if (NULL == old_cfgptr) {
    pfc_log_error("no such configuration exist in cache");
    return found;
  }
  found = PFC_TRUE;
  old_node_for_update = old_cfgptr;
  return found;
}

/**
 * @brief       : Method to add confignode lists to the cache for Audit
 * @param [in]  : value_list
 * @retval      : ERR_ADD_CHILD_TO_TREE_FAILED / UNC_RC_SUCCESSS
 */
UncRespCode KeyTree::append_audit_configuration_list(
    const std::vector<ConfigNode*>&value_list) {
  ODC_FUNC_TRACE;
  UncRespCode err = UNC_DRV_RC_ERR_GENERIC;

  ConfigNode*  tmp_cfgnode_ptr = NULL;
  std::vector<ConfigNode*>::const_iterator it = value_list.begin();
  std::vector<ConfigNode*>::const_iterator itr_end = value_list.end();

  // Iterate the vector of config nodes
  for (; it!= itr_end; it++) {
    if (*it == NULL) {
      pfc_log_error("RunningConfig::%s:%d: ConfigNode is NULL",
                   PFC_FUNCNAME, __LINE__);
      return UNC_DRV_RC_ERR_GENERIC;
    }

    tmp_cfgnode_ptr = *it;
    err = append_audit_node(tmp_cfgnode_ptr);
    if (UNC_RC_SUCCESS != err) {
      pfc_log_error("%s: AddChildToTree faild err=%d", PFC_FUNCNAME, err);
      tmp_cfgnode_ptr = NULL;
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  return UNC_RC_SUCCESS;
}

/**
 * @brief       : Method to add individual confignode to the cache for Audit
 * @param [in]  : value_node
 * @retval      : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode KeyTree::append_audit_node(
    ConfigNode* value_node) {
  ODC_FUNC_TRACE;
  UncRespCode err = UNC_DRV_RC_ERR_GENERIC;

  ConfigNode*  tmp_cfgnode_ptr = NULL;
  ConfigNode*  real_cfgnode_ptr= NULL;

  if (value_node == NULL) {
    pfc_log_error("%s : ConfigNode is NULL ", PFC_FUNCNAME);
    return err;
  }

  tmp_cfgnode_ptr = value_node;
  unc_key_type_t key_Type = tmp_cfgnode_ptr->get_type_name();
  std::string key = tmp_cfgnode_ptr->get_key_generate();
  real_cfgnode_ptr = get_node_from_hash(key, key_Type);
  if (NULL == real_cfgnode_ptr) {
    pfc_log_debug("%s: Node Not Present in Tree..for:%s keytype %d",
                 PFC_FUNCNAME, key.c_str(), key_Type);
    pfc_log_debug("%s: Parent Type Check %s ", PFC_FUNCNAME,
                 tmp_cfgnode_ptr->get_parent_key_name().c_str());
    err = add_node_to_tree(tmp_cfgnode_ptr);
    if (UNC_RC_SUCCESS != err) {
      pfc_log_error("%s: AddChildToTree faild err=%d", PFC_FUNCNAME, err);
      tmp_cfgnode_ptr = NULL;
      return UNC_DRV_RC_ERR_GENERIC;
    }
  } else {
    pfc_log_debug("%s: Node already Present in Tree...", PFC_FUNCNAME);
    delete tmp_cfgnode_ptr;
    tmp_cfgnode_ptr = NULL;
    real_cfgnode_ptr = NULL;
  }

  return UNC_RC_SUCCESS;
}

/**
 * @brief       : Method to add node to the cache and search map by validation
 * @param [in]  : value_list
 * @retval      : UNC_RC_SUCCESSS/UNC_DRV_RC_ERR_GENERIC
 */
UncRespCode KeyTree::add_node_to_tree(ConfigNode* child_ptr) {
  ODC_FUNC_TRACE;
  if (NULL == child_ptr) {
    pfc_log_error("add_node_to_tree:Child Node is NULL!!!!!!");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string  parent_key = child_ptr->get_parent_key_name();
  unc_key_type_t parent_type = get_parenttype(child_ptr->get_type_name());

  // Retrieve the parent from the map using parent_key & parent_type
  ConfigNode* parent_ptr = get_node_from_hash(parent_key, parent_type);
  if (NULL == parent_ptr) {
    pfc_log_error("Parent:%s  Not Present for:%s", parent_key.c_str(),
                 child_ptr->get_key_generate().c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  }
  // Add the new node to child list of the parentnode using parent_ptr
  UncRespCode err = parent_ptr->add_child_to_list(child_ptr);
  if ( UNC_RC_SUCCESS != err ) {
    pfc_log_error("add_node_to_tree:add_child_to_list Faild for:%s!!!!",
                 child_ptr->get_key_generate().c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  }
  // Add the new node to the search map
  err = add_child_to_hash(child_ptr);

  if ( UNC_RC_SUCCESS == err ) {
    pfc_log_debug("add_node_to_tree:add_child_to_hash Succed for: %s!!!!",
                 child_ptr->get_key_generate().c_str());
    cfgnode_count_++;
  }
  return UNC_RC_SUCCESS;
}

/**
 * @brief       : Method to search and retrieve the node from the search map
 * @param [in] : key
 * @param [in] : key_Type
 * @retval     : ConfigNode*
 */
ConfigNode* KeyTree::get_node_from_hash(
    std::string key, unc_key_type_t key_type) {
  ODC_FUNC_TRACE;
  ConfigNodeHash::iterator itr;
  itr = ConfigHashArr[key_type].find(key);
  if ( itr == ConfigHashArr[key_type].end() ) {
    pfc_log_debug("Node Not Present for:%s", key.c_str());
    return NULL;
  }
  pfc_log_debug("Node Present for:%s", key.c_str());
  return itr->second;
}

/**
 * @brief       : Method to insert node to the search map
 * @param [in]  : child_ptr
 * @retval      : UNC_RC_SUCCESSS
 */
UncRespCode KeyTree::add_child_to_hash(ConfigNode* child_ptr) {
  ODC_FUNC_TRACE;
  unc_key_type_t key_type = child_ptr->get_type_name();
  std::string key = child_ptr->get_key_generate();
  ConfigHashArr[key_type].insert(std::pair <std::string,
                                 ConfigNode*>(key, child_ptr));
  return UNC_RC_SUCCESS;
}

}  // namespace vtndrvcache
}  // namespace unc
