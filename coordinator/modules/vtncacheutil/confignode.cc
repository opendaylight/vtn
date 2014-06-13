/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include "confignode.hh"
#include "keytree.hh"
#include "vtn_cache_mod.hh"

namespace unc {
namespace vtndrvcache {

/**
 * @brief     : Returns the type string corresponding the the keytype.
 *              This is used to print the keytree structure
 * @param[in]  : search_type
 * @retval     : string
 */
std::string TypeToStrFun(unc_key_type_t search_type) {
  std::string TypeStr;
  std::map<unc_key_type_t, std::string>::const_iterator search =
      VtnDrvCacheMod::keyTypeStrMap.find(search_type);
  if (search != VtnDrvCacheMod::keyTypeStrMap.end()) {
    TypeStr = search->second;
  } else {
     TypeStr = std::string("Unknown");
  }
  return TypeStr;
}

/**
 * @brief : default constructor
 */
ConfigNode::ConfigNode() : operation_(0) {
  ODC_FUNC_TRACE;
  child_list_.clear();
}

/**
 * @brief     : Method to retrieve each node from the Keytree and populate in
                the vector
 * @param[in] : ConfigNode value_list
 * @retval    : UNC_RC_SUCCESS
 */
UncRespCode ConfigNode::get_node_list(
    std::vector<ConfigNode*>& value_list) {
  ODC_FUNC_TRACE;

  // If the node doesn't have any child, this return can be
  // from the root node or to the recursive caller
  if (child_list_.empty()) {
    pfc_log_debug("%s: No child list", PFC_FUNCNAME);
    return UNC_RC_SUCCESS;
  }

  // Get the child list of each node
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator sb_itr =
      child_list_.begin();
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator sb_itr_end =
      child_list_.end();

  // Iterate the child list and by make a recursive call to iterate
  // the child list and hence the list is traversed in DFS order
  // till the node has no children

  for (; sb_itr != sb_itr_end; ++sb_itr) {
    std::vector<ConfigNode*>& node_list = sb_itr->second;

    if (!node_list.empty()) {
      std::vector<ConfigNode*>::iterator node_itr = node_list.begin();
      std::vector<ConfigNode*>::iterator node_itr_end = node_list.end();

      for (; node_itr != node_itr_end; ++node_itr) {
        value_list.push_back(*node_itr);
        (*node_itr)->get_node_list(value_list);
      }
    }
  }

  return UNC_RC_SUCCESS;
}

/**
 * @brief     : This method prints each node information(Debug Purpose Only)
 * @param[in] : level_index
 * @retval    : None
 */
void ConfigNode::print(int level_index) {
  ODC_FUNC_TRACE;
  print_key(level_index);

  if (!child_list_.empty()) {
    std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator sb_itr =
        child_list_.begin();
    std::map<unc_key_type_t,
        std::vector<ConfigNode*> >::iterator sb_itr_end = child_list_.end();

    for (; sb_itr != sb_itr_end; sb_itr++) {
      std::vector<ConfigNode*>& node_list = sb_itr->second;
      if (!node_list.empty()) {
        std::vector<ConfigNode*>::iterator node_itr = node_list.begin();
        std::vector<ConfigNode*>::iterator node_itr_end = node_list.end();

        for (; node_itr != node_itr_end; node_itr++) {
          ConfigNode* node_ptr = *node_itr;
          node_ptr->print(level_index + 1);
        }
      }
    }
  }
}
/**
 * @brief     : This method traverse the list and delete child and then parent
 *            : from the the cache and insert key into erased_key_list before
 *            : doing delete operation
 * @param[in] : confignode *, erased_key_list(vector contain structure)
 * @retval    : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode ConfigNode::delete_child_node(ConfigNode *node_ptr,
                  std::vector<key_information>& erased_key_list) {
  ODC_FUNC_TRACE;
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator itr;
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator itr_end =
      child_list_.end();

  if (node_ptr == NULL) {
    pfc_log_error("%s : ConfigNode is NULL", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  itr = child_list_.find(node_ptr->get_type_name());
  if (itr == itr_end) {
    pfc_log_error("no such configuration present");
    return UNC_DRV_RC_ERR_GENERIC;
  } else {
    std::vector<ConfigNode*>& node_list = itr->second;
    if (!node_list.empty()) {
      std::vector<ConfigNode*>::iterator node_itr = node_list.begin();
      for (; node_itr != node_list.end(); ++node_itr) {
        if (*node_itr == node_ptr) {
          key_information key_info;
          key_info.key = node_ptr->get_key_generate();
          key_info.key_type = node_ptr->get_type_name();
          erased_key_list.push_back(key_info);
          node_list.erase(node_itr);
          node_ptr->clear_child_list(erased_key_list);
          delete node_ptr;
          break;
        }
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/**
 * @brief     : This method traverse the child list if parent have and delete
 *            :the nodes
 * @param[in] : erased_key_list
 * @retval    : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode ConfigNode::clear_child_list(std::vector<key_information>&
                                             erased_key_list) {
  ODC_FUNC_TRACE;
  if (!child_list_.empty()) {
    std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator sb_itr =
        child_list_.begin();
    std::map<unc_key_type_t,
        std::vector<ConfigNode*> >::iterator sb_itr_end = child_list_.end();

    for (; sb_itr != sb_itr_end; sb_itr++) {
      std::vector<ConfigNode*>& node_list = sb_itr->second;
      if (!node_list.empty()) {
        std::vector<ConfigNode*>::iterator node_itr = node_list.begin();
        std::vector<ConfigNode*>::iterator node_itr_end = node_list.end();

        for (; node_itr != node_itr_end; node_itr++) {
          ConfigNode* node_ptr = *node_itr;
          node_ptr->clear_child_list(erased_key_list);
          key_information key_info;
          key_info.key = node_ptr->get_key_generate();
          key_info.key_type = node_ptr->get_type_name();
          erased_key_list.push_back(key_info);
          delete node_ptr;
        }
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/**
 * @brief     : This method inserts the node in the cache
 * @param[in] : confignode *
 * @retval    : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode ConfigNode::add_child_to_list(ConfigNode *node_ptr) {
  ODC_FUNC_TRACE;

  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator itr;
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator itr_end =
      child_list_.end();

  if (node_ptr == NULL) {
    pfc_log_error("%s : ConfigNode is NULL", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  itr = child_list_.find(node_ptr->get_type_name());

  //  If the child list already present for that node type, add the new
  //  child to the list..... else create new child list
  if (itr == itr_end) {
    pfc_log_info("add_child_to_list: Child list NOT present for %s ",
                  TypeToStrFun(node_ptr->get_type_name()).c_str());
    child_list_.insert(
        std::pair<unc_key_type_t, std::vector<ConfigNode*> > (
            node_ptr->get_type_name(),
            std::vector<ConfigNode*> ()));

    itr = child_list_.find(node_ptr->get_type_name());
  } else {
    pfc_log_info("ConfigNode::AddChildNode: Child list present for %s ",
                  TypeToStrFun(node_ptr->get_type_name()).c_str());
  }

  std::vector<ConfigNode*>& node_list = itr->second;
  node_list.push_back(node_ptr);

  return UNC_RC_SUCCESS;
}

/**
 * @brief     : This method prints the Key of the
 *              particular node(Debug Purpose ONly)
 * @param[in] : level_index
 * @retval    : None
 */
void RootNode::print_key(int level_index) {
  ODC_FUNC_TRACE;
  int num_spaces = level_index* NUM_SPACES;
  std::string prefix_str;

  unc_key_type_t node_type = get_type_name();

  for (int i = 0; i < num_spaces; ++i)
    prefix_str.append(" ");
  prefix_str.append(TypeToStrFun(node_type));
  prefix_str.append("->");
}

/**
 * @brief : default constructor
 */
RootNode::RootNode() {
  ODC_FUNC_TRACE;
}

/**
 * @brief : default destructor
 */
RootNode::~RootNode() {
  ODC_FUNC_TRACE;
}
}  // namespace vtndrvcache
}  // namespace unc
