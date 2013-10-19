/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include "confignode.hh"
#include "keytree.hh"

namespace unc {
namespace vtndrvcache {

/**
 * @brief     : Returns the type string corresponding the the keytype.
 *              This is used to print the keytree structure
 * @param[in]  : search_type
 * @retval     : string
 */
std::string TypeToStrFun(unc_key_type_t search_type) {
  std::string typeStr;
  switch (search_type) {
    case UNC_KT_VTN:
      typeStr = std::string("UNC_KT_VTN");
      break;

    case UNC_KT_VBRIDGE:
      typeStr = std::string("UNC_KT_VBRIDGE");
      break;

    case UNC_KT_VBR_IF:
      typeStr = std::string("UNC_KT_VBR_IF");
      break;

    case UNC_KT_ROOT:
      typeStr = std::string("UNC_KT_ROOT");
      break;

    default:
      typeStr = std::string("Unknown");
      pfc_log_debug("%s: key_type = %d", PFC_FUNCNAME,
                   search_type);
      break;
  }
  return typeStr;
}

/**
 * @brief : default constructor
 */
ConfigNode::ConfigNode() : operation_(0), cfgnode_count_(0) {
  pfc_log_debug("Entering default constructor %s..", PFC_FUNCNAME);
  child_list_.clear();
}

/**
 * @brief     : Method to retrieve each node from the Keytree and populate in
                the vector
 * @param[in] : ConfigNode value_list
 * @retval    : TREE_OK
 */
uint32_t ConfigNode::get_node_list(std::vector<ConfigNode*>& value_list) {
  pfc_log_debug("Entering %s ", PFC_FUNCNAME);

  // If the node doesn't have any child, this return can be
  // from the root node or to the recursive caller
  if (child_list_.empty()) {
    pfc_log_debug("%s: No child list", PFC_FUNCNAME);
    return TREE_OK;
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

  return TREE_OK;
}

/**
 * @brief     : This method prints each node information
 * @param[in] : level_index
 * @retval    : None
 */
void ConfigNode::print(int level_index) {
  pfc_log_debug("%s: Entering function....", PFC_FUNCNAME);
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

  pfc_log_debug("%s: Exiting function....", PFC_FUNCNAME);
}

/**
 * @brief     : This method inserts the node in the cache
 * @param[in] : confignode *
 * @retval    : CACHEMGR_RESPONSE_FAILURE / TREE_OK
 */
uint32_t ConfigNode::add_child_to_list(ConfigNode *node_ptr) {
  pfc_log_debug("%s:Entering", PFC_FUNCNAME);

  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator itr;
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator itr_end =
      child_list_.end();

  if (node_ptr == NULL) {
    pfc_log_error("%s : ConfigNode is NULL", PFC_FUNCNAME);
    return CACHEMGR_RESPONSE_FAILURE;
  }

  itr = child_list_.find(node_ptr->get_type());

  //  If the child list already present for that node type, add the new
  //  child to the list..... else create new child list
  if (itr == itr_end) {
    pfc_log_debug("add_child_to_list: Child list NOT present for %s ",
                  TypeToStrFun(node_ptr->get_type()).c_str());
    child_list_.insert(
        std::pair<unc_key_type_t, std::vector<ConfigNode*> > (
            node_ptr->get_type(),
            std::vector<ConfigNode*> ()));

    itr = child_list_.find(node_ptr->get_type());
  } else {
    pfc_log_debug("ConfigNode::AddChildNode: Child list present for %s ",
                  TypeToStrFun(node_ptr->get_type()).c_str());
  }

  std::vector<ConfigNode*>& node_list = itr->second;
  node_list.push_back(node_ptr);
  cfgnode_count_++;
  pfc_log_debug("cfgnode_count_ %d", cfgnode_count_);

  pfc_log_debug("%s: AddChildNode: Exiting", PFC_FUNCNAME);
  return TREE_OK;
}

/**
 * @brief     : This method prints the Key of the
 *              particular node
 * @param[in] : level_index
 * @retval    : None
 */
void RootNode::print_key(int level_index) {
  int num_spaces = level_index* NUM_SPACES;
  std::string prefix_str;

  unc_key_type_t node_type = get_type();

  for (int i = 0; i < num_spaces; ++i)
    prefix_str.append(" ");
  prefix_str.append(TypeToStrFun(node_type));
  prefix_str.append("->");
}

/**
 * @brief : default constructor
 */
RootNode::RootNode() {
  pfc_log_debug("Entering default constructor %s..", PFC_FUNCNAME);
}

/**
 * @brief : default destructor
 */
RootNode::~RootNode() {
  pfc_log_debug("Entering default destructor %s..", PFC_FUNCNAME);
}
}  // namespace vtndrvcache
}  // namespace unc
