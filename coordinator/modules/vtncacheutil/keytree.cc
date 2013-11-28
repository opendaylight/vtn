/*
 * Copyright (c) 2013 NEC Corporation
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
  { UNC_KT_VBRIDGE, UNC_KT_VTN, 0 },
  { UNC_KT_VBR_IF, UNC_KT_VBRIDGE, 0 },
  { UNC_KT_VBR_VLANMAP, UNC_KT_VBRIDGE, 1 }
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
 * @retval      : DRVAPI_RESPONSE_SUCCESS
 */
drv_resp_code_t KeyTree::get_nodelist_keytree() {
  ODC_FUNC_TRACE;
  cfg_node_list_.clear();
  return node_tree_.get_node_list(cfg_node_list_);
}

/**
 * @brief       : Method to add individual confignode to the cache for commit
 * @param [in]  : value_node
 * @retval      : DRVAPI_RESPONSE_SUCCESS/DRVAPI_RESPONSE_FAILURE
 */
drv_resp_code_t KeyTree::append_commit_node(ConfigNode* value_node) {
  ODC_FUNC_TRACE;
  cfg_node_list_.push_back(value_node);
  return DRVAPI_RESPONSE_SUCCESS;
}

/**
 * @brief : Method to clear the elements of vector using for commit cache
 */
void KeyTree::clear_audit_commit_cache() {
  ODC_FUNC_TRACE;
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
 * @brief       : Method to add confignode lists to the cache for Audit
 * @param [in]  : value_list
 * @retval      : ERR_ADD_CHILD_TO_TREE_FAILED / DRVAPI_RESPONSE_SUCCESSS
 */
drv_resp_code_t KeyTree::append_audit_configuration_list(
    const std::vector<ConfigNode*>&value_list) {
  ODC_FUNC_TRACE;
  drv_resp_code_t err = DRVAPI_RESPONSE_FAILURE;

  ConfigNode*  tmp_cfgnode_ptr = NULL;
  std::vector<ConfigNode*>::const_iterator it = value_list.begin();
  std::vector<ConfigNode*>::const_iterator itr_end = value_list.end();

  // Iterate the vector of config nodes
  for (; it!= value_list.end(); it++) {
    if (*it == NULL) {
      pfc_log_error("RunningConfig::%s:%d: ConfigNode is NULL",
                    PFC_FUNCNAME, __LINE__);
      return DRVAPI_RESPONSE_FAILURE;
    }

    tmp_cfgnode_ptr = *it;
    err = append_audit_node(tmp_cfgnode_ptr);
    if (DRVAPI_RESPONSE_SUCCESS != err) {
      pfc_log_error("%s: AddChildToTree faild err=%d", PFC_FUNCNAME, err);
      tmp_cfgnode_ptr = NULL;
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

/**
 * @brief       : Method to add individual confignode to the cache for Audit
 * @param [in]  : value_node
 * @retval      : DRVAPI_RESPONSE_FAILURE / DRVAPI_RESPONSE_SUCCESS
 */
drv_resp_code_t KeyTree::append_audit_node(
    ConfigNode* value_node) {
  ODC_FUNC_TRACE;
  drv_resp_code_t err = DRVAPI_RESPONSE_FAILURE;

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
    if (DRVAPI_RESPONSE_SUCCESS != err) {
      pfc_log_error("%s: AddChildToTree faild err=%d", PFC_FUNCNAME, err);
      tmp_cfgnode_ptr = NULL;
      return DRVAPI_RESPONSE_FAILURE;
    }
  } else {
    pfc_log_debug("%s: Node already Present in Tree...", PFC_FUNCNAME);
    delete tmp_cfgnode_ptr;
    tmp_cfgnode_ptr = NULL;
    real_cfgnode_ptr = NULL;
  }

  return DRVAPI_RESPONSE_SUCCESS;
}

/**
 * @brief       : Method to add node to the cache and search map by validation
 * @param [in]  : value_list
 * @retval      : DRVAPI_RESPONSE_SUCCESSS/DRVAPI_RESPONSE_FAILURE
 */
drv_resp_code_t KeyTree::add_node_to_tree(ConfigNode* child_ptr) {
  ODC_FUNC_TRACE;
  if (NULL == child_ptr) {
    pfc_log_error("add_node_to_tree:Child Node is NULL!!!!!!");
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::string  parent_key = child_ptr->get_parent_key_name();
  unc_key_type_t parent_type = get_parenttype(child_ptr->get_type_name());

  // Retrieve the parent from the map using parent_key & parent_type
  ConfigNode* parent_ptr = get_node_from_hash(parent_key, parent_type);
  if (NULL == parent_ptr) {
    pfc_log_error("Parent:%s  Not Present for:%s", parent_key.c_str(),
                  child_ptr->get_key_generate().c_str());
    return DRVAPI_RESPONSE_FAILURE;
  }
  // Add the new node to child list of the parentnode using parent_ptr
  drv_resp_code_t err = parent_ptr->add_child_to_list(child_ptr);
  if ( DRVAPI_RESPONSE_SUCCESS != err ) {
    pfc_log_error("add_node_to_tree:add_child_to_list Faild for:%s!!!!",
                  child_ptr->get_key_generate().c_str());
    return DRVAPI_RESPONSE_FAILURE;
  }
  // Add the new node to the search map
  err = add_child_to_hash(child_ptr);

  if ( DRVAPI_RESPONSE_SUCCESS == err ) {
    pfc_log_debug("add_node_to_tree:add_child_to_hash Succed for: %s!!!!",
                  child_ptr->get_key_generate().c_str());
    cfgnode_count_++;
  }
  return DRVAPI_RESPONSE_SUCCESS;
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
 * @retval      : DRVAPI_RESPONSE_SUCCESSS
 */
drv_resp_code_t KeyTree::add_child_to_hash(ConfigNode* child_ptr) {
  ODC_FUNC_TRACE;
  unc_key_type_t key_type = child_ptr->get_type_name();
  std::string key = child_ptr->get_key_generate();
  ConfigHashArr[key_type].insert(std::pair <std::string,
                                 ConfigNode*>(key, child_ptr));
  return DRVAPI_RESPONSE_SUCCESS;
}

}  // namespace vtndrvcache
}  // namespace unc
