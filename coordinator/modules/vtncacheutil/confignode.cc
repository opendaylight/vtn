/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include "include/confignode.hh"
#include "include/keytree.hh"

namespace unc {
namespace vtndrvcache {

/**
 * @brief  Returns the type string corresponding the the keytype.
 * This is used
 * to print the keytree structure
 * param[in] search_type
 * param[out] Kettype converted to
 * string
 */
std::string TypeToStrFun(unc_key_type_t search_type) {
  std::string typeStr;
  switch (search_type) {
    case UNC_KT_VTN:
      typeStr = string("UNC_KT_VTN");
      break;

    case UNC_KT_VBRIDGE:
      typeStr = string("UNC_KT_VBRIDGE");
      break;

    case UNC_KT_VBR_IF:
      typeStr = string("UNC_KT_VBR_IF");
      break;

    case UNC_KT_ROOT:
      typeStr = string("UNC_KT_ROOT");
      break;

    default:
      typeStr = string("Unknown");
      pfc_log_info("%s: key_type = %d", PFC_FUNCNAME,
                   search_type);
      break;
  }
  return typeStr;
}

/**
 * @brief : default constructor
 */
ConfigNode::ConfigNode() :node_key(""),parent_key(""),operation(0) {
  pfc_log_trace("ConfigNode Constructor");
  child_list.clear();
}

/**
 * @brief  Method to retrieve each node from the Keytree and
 * populate in
 * the vector
 * param[in] value_list
 * param[out] TREE_OK
 */
uint32_t ConfigNode::get_node_list(std::vector<ConfigNode*>&value_list) {
  pfc_log_debug("Entering %s ", PFC_FUNCNAME);
  if (child_list.size() <= 0) {
    pfc_log_debug("%s: No child list", PFC_FUNCNAME);
    return TREE_OK;
  }
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator sb_itr =
      child_list.begin();
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator sb_itr_end =
      child_list.end();

  for (; sb_itr != sb_itr_end; ++sb_itr) {
    std::vector<ConfigNode*>& node_list = sb_itr->second;

    if (!node_list.empty()) {
      std::vector<ConfigNode*>::iterator node_itr = node_list.begin();
      std::vector<ConfigNode*>::iterator node_itr_end = node_list.end();

      for (; node_itr != node_itr_end; ++node_itr) {
        value_list.push_back(*node_itr);
        pfc_log_debug("ConfigNode::get_node_list: KeyTree::getNodeList:%s "
                      " pushed in to &value_list ",
                      TypeToStrFun((*node_itr)->get_type()).c_str());
        (*node_itr)->get_node_list(value_list);
      }
    }
  }

  return TREE_OK;
}

/**
 * @brief  This method prints each node information
 * param[in] level_index
 * param[out] none
 */
void ConfigNode::print(int level_index) {
  pfc_log_debug("%s: Entering function....", PFC_FUNCNAME);
  print_key(level_index);

  if (child_list.size() > 0) {
    std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator sb_itr =
        child_list.begin();
    std::map<unc_key_type_t,
        std::vector<ConfigNode*> >::iterator sb_itr_end = child_list.end();

    for (; sb_itr != sb_itr_end; sb_itr++) {
      std::vector<ConfigNode*>& node_list = sb_itr->second;
      if (node_list.size() > 0) {
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
 * @brief  This method inserts the node in the cache
 * param[in] confignode*
 * param[out]uint32_t
 */
uint32_t ConfigNode::add_child_to_list(ConfigNode *node_ptr) {
  pfc_log_debug("%s:Entering", PFC_FUNCNAME);

  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator itr =
      child_list.begin();
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator itr_end =
      child_list.end();

  itr = child_list.find(node_ptr->get_type());

  /* If the child list already present for that node type, add the new
     child to the list..... else create new child list */
  if (itr == itr_end) {
    pfc_log_debug("add_child_to_list: Child list NOT present for %s ",
                  TypeToStrFun(node_ptr->get_type()).c_str());
    child_list.insert(
        pair<unc_key_type_t, std::vector<ConfigNode*> > (
            node_ptr->get_type(),
            vector<ConfigNode*> ()));

    itr = child_list.find(node_ptr->get_type());
  } else {
    pfc_log_debug("ConfigNode::AddChildNode: Child list present for %s ",
                  TypeToStrFun(node_ptr->get_type()).c_str());
  }

  std::vector<ConfigNode*>& node_list = itr->second;
  node_list.push_back(node_ptr);
  pfc_log_debug("%s: AddChildNode: Exiting", PFC_FUNCNAME);
  return TREE_OK;
}

/**
 * @brief  This method clear the cache
 * param[in] none
 * param[out]none
 */
void ConfigNode::clear_kt_map() {
  pfc_log_debug("%s:Entering", PFC_FUNCNAME);
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator sb_itr =
      child_list.begin();
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator sb_itr_end =
      child_list.end();

  for (; sb_itr != sb_itr_end; ++sb_itr) {
    std::vector<ConfigNode*>& node_list = sb_itr->second;

    if (!node_list.empty()) {
      std::vector<ConfigNode*>::iterator node_itr = node_list.begin();
      std::vector<ConfigNode*>::iterator node_itr_end = node_list.end();

      for (; node_itr != node_itr_end; ++node_itr) {
        if (*node_itr != NULL) {
          delete *node_itr;
        }
      }
    }
    node_list.clear();
  }
  child_list.clear();
  pfc_log_debug("%s:Exiting", PFC_FUNCNAME);
}

/**
 * @brief  This method read confignode from cache
 * param[in] keytype,confignode* vector
 * param[out]filled confignode* vector
 */
uint32_t ConfigNode::read_all_cfgnode(unc_key_type_t key,
                                      std::vector<ConfigNode*>& vec) {
  pfc_log_debug("%s:Entering", PFC_FUNCNAME);
  std::map<unc_key_type_t, std::vector<ConfigNode*> >::iterator sb_itr;
  sb_itr = child_list.find(key);
  if (sb_itr != child_list.end()) {
    vec.reserve(sb_itr->second.size());
    std::copy(sb_itr->second.begin(), sb_itr->second.end(),
              back_inserter(vec));
    return PFCDRVAPI_RESPONSE_SUCCESS;
  }
  pfc_log_debug("%s:Exiting", PFC_FUNCNAME);
  return PFCDRVAPI_RESPONSE_FAILURE;
}

/**
 * @brief  This method prints each node information
 * param[in] level_index
 * param[out] none
 */
void RootNode::print_key(int level_index) {
  int num_spaces = level_index* NUM_SPACES;
  std::string prefix_str = "";

  unc_key_type_t node_type = get_type();

  for (int i = 0; i < num_spaces; i++)
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
 * @brief : default desstructor
 */
RootNode::~RootNode() {
  pfc_log_debug("Entering default destructor %s..", PFC_FUNCNAME);
}
}  // vtndrvcache
}  // unc
