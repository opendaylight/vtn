/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include "include/keytree.hh"
#include "unc/keytype.h"

#define TREE_TABLE_SIZE 5

namespace unc {
namespace vtndrvcache {

/**
 ** Method to create Cache and return the pointer to it
 ** @param [out] - KeyTree*
 **/
KeyTree* KeyTree::create_cache() {
  return  new KeyTree();
}

/**
 ** Method to clear the elements of audit cache
 ** @param [in] - None
 ** @param [out] - None
 **/
CommonIterator* KeyTree::get_iterator() {
  return new CommonIterator(this);
}

key_tree key_tree_table[TREE_TABLE_SIZE] = {
  { UNC_KT_CONTROLLER, UNC_KT_ROOT, 0 },
  { UNC_KT_VTN, UNC_KT_ROOT, 3 },
  { UNC_KT_VBRIDGE, UNC_KT_VTN, 3 }, { UNC_KT_VBR_IF, UNC_KT_VBRIDGE, 2 }
};

/**
 ** Method to return the Keytype of Parent given the search Key
 ** @param [in] - search_type
 ** @param [out] - unc_key_type_t
 **/
unc_key_type_t KeyTree::get_parenttype(unc_key_type_t search_type) {
  pfc_log_debug("Entering %s function", PFC_FUNCNAME);
  int table_size = sizeof(key_tree_table) / sizeof(key_tree);
  int entry_index = 0;
  unc_key_type_t parent_type = UNC_KT_VUNKNOWN;

  pfc_log_debug("Table size %d Index %d", table_size, entry_index);

  for (entry_index = 0; entry_index < table_size; entry_index++) {
    if (key_tree_table[entry_index].id == search_type) {
      parent_type = key_tree_table[entry_index].parent;
      break;
    }
  }
  pfc_log_debug("Exiting %s function", PFC_FUNCNAME);
  return parent_type;
}

/**
 ** Method to traverse the tree and populate the nodes into
 ** the vector
 ** @param [in] - ctrlid
 ** @param [in] - value_list
 ** @param [out] - TREE_OK
 **/
uint32_t KeyTree::get_nodelist_keytree(
    std::vector<ConfigNode*>&value_list) {
  pfc_log_debug("Entering %s function", PFC_FUNCNAME);
  return node_tree.get_node_list(value_list);
}

/**
 ** Method to add confignode to the cache for commit
 ** @param [in] - value_list
 ** @param [out] - TREE_OK
 **/
uint32_t KeyTree::append_commit_node(ConfigNode* value_list) {
  pfc_log_debug("Entering %s function", PFC_FUNCNAME);
  int err = 0;
  cfg_node_list.push_back(value_list);
  err = add_child_to_hash(value_list);
  if ( TREE_OK != err ) {
    pfc_log_error("add_node_to_tree:add_child_to_hash Faild for: %s!!!!",
                  value_list->get_key().c_str());
    return PFCDRVAPI_RESPONSE_FAILURE;
  }

  return CACHEMGR_RESPONSE_SUCCESS;
}

/**
 ** Method to clear the elements of commit cache
 ** @param [in] - None
 ** @param [out] - None
 **/
void KeyTree::clear_commit_cache() {
  clear_search_cache();
  clear_cache_vector();
}

/**
 ** Method to clear the elements of vector
 ** @param [in] - None
 ** @param [out] - None
 **/
void KeyTree::clear_cache_vector() {
  vector<ConfigNode*>::const_iterator itr = cfg_node_list.begin();
  vector<ConfigNode*>::const_iterator itr_end = cfg_node_list.end();
  for (; itr != itr_end; ++itr) {
    if (*itr != NULL) {
      delete *itr;
    }
  }
  cfg_node_list.clear();
}

/**
 ** Method to clear the elements of search map
 ** @param [in] - None
 ** @param [out] - None
 **/
void KeyTree::clear_search_cache() {
  for ( int i =0; i < CONFIGARR_SIZE; i++ ) {
    configHashArr[i].clear();
  }
}

/**
 ** Method to clear the elements of audit cache
 ** @param [in] - None
 ** @param [out] - None
 **/
void KeyTree::clear_audit_cache() {
  clear_search_cache();
  node_tree.clear_kt_map();
}

uint32_t KeyTree::read_all_cfgnode(unc_key_type_t key,
                                   std::vector<ConfigNode*>& vec) {
  return node_tree.read_all_cfgnode(key, vec);
}

/**
 ** Method to add confignode to the cache for Audit
 ** @param [in] - value_list
 ** @param [out] - TREE_OK
 **/
uint32_t KeyTree::append_audit_node(
    const std::vector<ConfigNode*>&value_list) {
  pfc_log_info("%s: Entering function", PFC_FUNCNAME);
  uint32_t err = -1;

  ConfigNode*  tmp_cfgnode_ptr = NULL;
  ConfigNode*  real_cfgnode_ptr= NULL;

  vector<ConfigNode*>::const_iterator it = value_list.begin();
  vector<ConfigNode*>::const_iterator itr_end = value_list.end();

  for (; it!= value_list.end(); it++) {
    if (*it == NULL) {
      pfc_log_error("RunningConfig::%s:%d: ConfigNode is NULL",
                    PFC_FUNCNAME, __LINE__);
      continue;
    }

    tmp_cfgnode_ptr = *it;
    unc_key_type_t key_Type = tmp_cfgnode_ptr->get_type();
    std::string key = tmp_cfgnode_ptr->get_key();
    if (PFC_TRUE != is_already_present(key, key_Type)) {
      pfc_log_trace("%s: Node Not Present in Tree..for:%s", PFC_FUNCNAME,
                    key.c_str());
      err = add_node_to_tree(tmp_cfgnode_ptr);
      if (TREE_OK != err) {
        pfc_log_error("%s: AddChildToTree faild err=%d", PFC_FUNCNAME, err);
        tmp_cfgnode_ptr = NULL;
        return ERR_ADD_CHILD_TO_TREE_FAILD;
      }
      real_cfgnode_ptr = tmp_cfgnode_ptr;
    } else {
      pfc_log_trace("%s: Node already Present in Tree...", PFC_FUNCNAME);
      real_cfgnode_ptr = get_node_from_hash(key, key_Type);
      delete tmp_cfgnode_ptr;
      tmp_cfgnode_ptr = NULL;
      if (NULL == real_cfgnode_ptr) {
        pfc_log_error("%s: NULL Node Found in Hash...", PFC_FUNCNAME);
      }
    }
  }
  pfc_log_info("%s: Exiting function", PFC_FUNCNAME);
  return TREE_OK;
}

/**
 ** Method to add confignode to the cache for Audit
 ** @param [in] - value_list
 ** @param [out] - TREE_OK
 **/
uint32_t KeyTree::append_audit_node(
    ConfigNode* value_list) {
  pfc_log_info("%s: Entering function", PFC_FUNCNAME);
  uint32_t err = -1;

  ConfigNode*  tmp_cfgnode_ptr = NULL;
  ConfigNode*  real_cfgnode_ptr= NULL;

  if (value_list == NULL) {
    pfc_log_error("%s : ConfigNode is NULL ", PFC_FUNCNAME);
    return err;
  }

  tmp_cfgnode_ptr = value_list;
  unc_key_type_t key_Type = tmp_cfgnode_ptr->get_type();
  std::string key = tmp_cfgnode_ptr->get_key();
  if (PFC_TRUE != is_already_present(key, key_Type)) {
    pfc_log_trace("%s: Node Not Present in Tree..for:%s", PFC_FUNCNAME,
                  key.c_str());
    err = add_node_to_tree(tmp_cfgnode_ptr);
    if (TREE_OK != err) {
      pfc_log_error("%s: AddChildToTree faild err=%d", PFC_FUNCNAME, err);
      tmp_cfgnode_ptr = NULL;
      return ERR_ADD_CHILD_TO_TREE_FAILD;
    }
    real_cfgnode_ptr = tmp_cfgnode_ptr;
  } else {
    pfc_log_trace("%s: Node already Present in Tree...", PFC_FUNCNAME);
    real_cfgnode_ptr = get_node_from_hash(key, key_Type);
    delete tmp_cfgnode_ptr;
    tmp_cfgnode_ptr = NULL;
    if (NULL == real_cfgnode_ptr) {
      pfc_log_error("%s: NULL Node Found in Hash...", PFC_FUNCNAME);
    }
  }

  pfc_log_info("%s: Exiting function", PFC_FUNCNAME);
  return TREE_OK;
}

/**
 ** Method to validate if parent exists for this particular node
 ** @param [in] - value_list
 ** @param [out] - true/false
 **/
bool KeyTree:: validate_parentkey(ConfigNode* value_list) {
  pfc_log_debug("%s: Entering function", PFC_FUNCNAME);
  unc_key_type_t parent_type = get_parenttype(value_list->get_type());
  std::string key = value_list->get_parent_key();
  pfc_log_debug("%s: Exiting function", PFC_FUNCNAME);
  return is_already_present(key, parent_type);
}

/**
 ** Method to add node to the cache and search map by validation
 ** @param [in] - value_list
 ** @param [out] - TREE_OK/PFCDRVAPI_RESPONSE_FAILURE
 **/
uint32_t KeyTree::add_node_to_tree(ConfigNode* child_ptr) {
  pfc_log_debug("%s: Entering function", PFC_FUNCNAME);
  if (NULL == child_ptr) {
    pfc_log_fatal("add_node_to_tree:Child Node is NULL!!!!!!");
    return PFCDRVAPI_RESPONSE_FAILURE;
  }
  std::string  parent_key = child_ptr->get_parent_key();
  unc_key_type_t parent_type = get_parenttype(child_ptr->get_type());
  ConfigNode* parent_ptr = get_node_from_hash(parent_key, parent_type);
  if (NULL == parent_ptr) {
    pfc_log_fatal("Parent:%s  Not Present for:%s", parent_key.c_str(),
                  child_ptr->get_key().c_str());
    return PFCDRVAPI_RESPONSE_FAILURE;
  }
  uint32_t err = parent_ptr->add_child_to_list(child_ptr);
  if ( TREE_OK != err ) {
    pfc_log_fatal("add_node_to_tree:add_child_to_list Faild for:%s!!!!",
                  child_ptr->get_key().c_str());
    return PFCDRVAPI_RESPONSE_FAILURE;
  }
  err = add_child_to_hash(child_ptr);

  if ( TREE_OK != err ) {
    pfc_log_error("add_node_to_tree:add_child_to_hash Faild for: %s!!!!",
                  child_ptr->get_key().c_str());
    return PFCDRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("%s: Exiting function", PFC_FUNCNAME);
  return TREE_OK;
}

/**
 ** Method to search and retrieve the node from the search map
 ** @param [in] - key
 ** @param [in] - key_Type
 ** @param [out] - ConfigNode*
 **/
ConfigNode* KeyTree::get_node_from_hash(
    std::string key, unc_key_type_t key_Type) {
  pfc_log_debug("%s: Entering function", PFC_FUNCNAME);
  configNodeHash::iterator itr;
  itr = configHashArr[key_Type].find(key);
  if ( itr == configHashArr[key_Type].end() ) {
    pfc_log_debug("Node Not Present for:%s", key.c_str());
    return NULL;
  }
  pfc_log_trace("Node Present for:%s", key.c_str());
  pfc_log_debug("%s: Exiting function", PFC_FUNCNAME);
  return itr->second;
}

/**
 ** Method to insert node to the search map
 ** @param [in] - child_ptr
 ** @param [out] - TREE_OK
 **/
uint32_t KeyTree::add_child_to_hash(ConfigNode* child_ptr) {
  pfc_log_debug("%s: Entering function", PFC_FUNCNAME);
  unc_key_type_t key_type = child_ptr->get_type();
  std::string key = child_ptr->get_key();
  configHashArr[key_type].insert(pair <std::string,
                                 ConfigNode*>(key, child_ptr));
  pfc_log_debug("%s: Exiting function", PFC_FUNCNAME);
  return TREE_OK;
}

/**
 ** Method to check if the particular key is already present in search map
 ** @param [in] - key
 ** @param [in] - key_Type
 ** @param [out] - true/false
 **/
bool KeyTree::is_already_present(std::string key, unc_key_type_t key_Type) {
  pfc_log_debug("%s: Entering function", PFC_FUNCNAME);
  configNodeHash::iterator itr;
  itr = configHashArr[key_Type].find(key);
  if (itr == configHashArr[key_Type].end()) {
    pfc_log_trace("Node Not Present for: %s", key.c_str());
    return false;
  }
  pfc_log_trace("Node Present for: %s", key.c_str());
  pfc_log_debug("%s: Exiting function", PFC_FUNCNAME);
  return true;
}
}
}
