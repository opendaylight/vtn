/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef KEYTREE_CONFIG_HH_
#define KEYTREE_CONFIG_HH_

#include <vector>
#include <utility>
#include <map>
#include <string>
#include "confignode.hh"
#include "vtn_conf_data_element_op.hh"

namespace unc {
namespace vtndrvcache {

struct key_tree {
  unc_key_type_t id; /* Key type */
  unc_key_type_t parent; /* Parent */
  int order; /* Sibling order */
};

/* forward declaration for CommonIterator class */
class CommonIterator;

/* ConfigNodeHash map storing entry of each node constructing in tree */

typedef std::map<std::string, ConfigNode* > ConfigNodeHash;

class KeyTree {
 public:
  friend class CommonIterator;

  /**
   * @brief : Method to create Cache and return the pointer to it
   * @retval : KeyTree*(new keytree instance)
   */
  static
      KeyTree* create_cache();

  /**
   * @brief       : Method to add confignode lists to the cache for Audit
   * @param [in]  : value_list(contain multiple nodes in a vector)
   * @retval      : UncRespCode(UNC_RC_SUCCESS/
   *                UNC_DRV_RC_ERR_GENERIC)
   */
  UncRespCode append_audit_configuration_list(
        const std::vector<ConfigNode*>&value_list);

  /**
   * @brief      : Method to add individual confignode to the cache for Audit
   * @param [in] : value_node
   * @retval     : UncRespCode(UNC_RC_SUCCESS/
   *               UNC_DRV_RC_ERR_GENERIC)
   */
  UncRespCode append_audit_node(ConfigNode* value_node);

  /**
   * @brief      : Method to add list of confignode to the cache to support
   *               physical node audit
   * @param [in] : value_list(a vector contain list of confignodes)
   * @retval     : UncRespCode(UNC_RC_SUCCESS)
   */
  UncRespCode append_physical_attribute_configuration_list(
        const std::vector<ConfigNode*>&value_list);

  /**
   * @brief      : Method to add individual confignode to the cache for physical
   *               node
   * @param [in] : value_node
   * @retval     : UncRespCode(UNC_RC_SUCCESS/
   *               UNC_DRV_RC_ERR_GENERIC)
   */
  UncRespCode append_Physical_attribute_node(ConfigNode* value_node);

  /**
   * @brief      : Method to update individual confignode to the cache for
   *               physical node
   * @param [in] : child_ptr
   * @retval     : UncRespCode(UNC_RC_SUCCESS/
   *               UNC_DRV_RC_ERR_GENERIC)
   */
  UncRespCode update_physical_attribute_node(ConfigNode* child_ptr);

  /**
   * @brief      : Method to delete individual confignode to the cache for
   *               physical node
   * @param [in] : child_ptr
   * @retval     : UncRespCode(UNC_RC_SUCCESS/
   *               UNC_DRV_RC_ERR_GENERIC)
   */
  UncRespCode delete_physical_attribute_node(ConfigNode* child_ptr);

  /**
   * @brief      : Method to return existing node from cache as per new compare
   *               node
   * @param [in] : new_compare_node
   * @param [out]: old_node_for_update
   * @retval     : boolean(true/false)
   */
  pfc_bool_t compare_is_physical_node_found(ConfigNode* new_compare_node,
                                              ConfigNode*& old_node_for_update);

  /**
   * @brief      : Method to add individual confignode to the cache for Audit
   * @param [in] : value_node
   * @retval     : UncRespCode(UNC_RC_SUCCESS/
   *               UNC_DRV_RC_ERR_GENERIC)
   */
  UncRespCode append_commit_node(ConfigNode* value_node);

  /**
   * @brief  : Method to create the iterator of Keytree
   * @retval : CommonIterator*(return commoniterator instance)
   */
  CommonIterator* create_iterator();

  /**
   * @brief  : Overloaded [] operator that returns each element
   *           index of the container
   * @retval : ConfigNode*
   */
  ConfigNode* operator[](uint32_t itemIndex) {
    if (cfg_node_list_.size() > itemIndex) {
    return cfg_node_list_[itemIndex];
    }
    return NULL;
  }


  /**
   * @brief  : Method to calculate the size of root node's cfg_node_list_
   * @retval : uint32_t(return count)
   */
  uint32_t audit_cfg_list_count() {
    return cfgnode_count_;
  }

  /* Initialization of ConfigNodeHash map */

  ConfigNodeHash ConfigHashArr[CACHEMGR_CONFIGARR_SIZE];

  /**
   * @brief  : Method to calculate the size of container cfg_node_list_
   * @retval : uint32_t(return the size)
   */
  uint32_t cfg_list_count() {
    return cfg_node_list_.size();
  }

  /**
   * @brief : Destructor
   **/
  ~KeyTree();

 private:
  /**
   * @brief : Constructor to insert the root node in the array of map
   */
  KeyTree();

  /**
   * @brief : Copy Constructor  to prevent create object
   */
  explicit KeyTree(KeyTree const&);

  /**
   * @brief : Assignment operator  to prevent copies
   */
  KeyTree& operator = (KeyTree const&);


  void print();
  /**
   * @brief      : Method to traverse the tree in DFS order and populate
   *               the nodes into the vector
   * @param [in] : value_list
   * @retval     : UNC_RC_SUCCESS
   */
  UncRespCode get_nodelist_keytree();
  /**
   * @brief      : Method to return the Parent given the child_type
   * @param [in] : child_type
   * @retval     : unc_key_type_t
   */
  unc_key_type_t get_parenttype(unc_key_type_t child_type);

  /**
   * @brief      : Method to add individual Config node to the cache
   * @param [in] : child_ptr
   * @retval     : UncRespCode(UNC_RC_SUCCESS/
   *               UNC_DRV_RC_ERR_GENERIC)
   */
  UncRespCode add_node_to_tree(ConfigNode* child_ptr);

  /**
   * @brief      : Method to search and retrieve the node from the search map
   * @param [in] : key
   * @param [in] : key_type
   * @retval     : ConfigNode*
   */
  ConfigNode* get_node_from_hash(std::string key,
    unc_key_type_t key_type);
  /**
   * @brief      : Method to insert node to the search map
   * @param [in] : child_ptr
   * @retval     : UncRespCode(UNC_RC_SUCCESS)
   */
  UncRespCode add_child_to_hash(ConfigNode* child_ptr);


  /**
   * @brief : Method to clear the elements of commit/audit cache
   */
  void clear_audit_commit_cache();

  /**
   * @brief  : Method to clear the elements of search array
   */
  void clear_root_cache();

  /*
   * @brief : vector config_node_list_ contain configuration for commit
   *          purpose to controller
   */
  typedef std::vector<ConfigNode*> config_node_list_;
  config_node_list_ cfg_node_list_;

  /*
   * @brief : count for configuration in tree
   */
  uint32_t cfgnode_count_;

  /* Root node object responsible for create a tree */
  RootNode node_tree_;
};

class CommonIterator {
 public:
  /**
   * @brief     : Constructor to initialize the members
   * @param[in] : KeyTree *
   **/
  explicit CommonIterator(KeyTree *aggregate)
      : aggregate_(aggregate), currentIndex_(0) {
      ODC_FUNC_TRACE;
      }

  /**
   * @brief  : Method to fetch first element from Keytree container
   * @retval : ConfigNode*
   */
  ConfigNode* FirstItem() {
    ODC_FUNC_TRACE;
    currentIndex_ = 0;
    if (aggregate_ != NULL) {
      return (*aggregate_)[currentIndex_];
    }
    return NULL;
  }

  /**
   * @brief  : Method to fetch next element from Keytree container
   * @retval : ConfigNode*
   */
  ConfigNode* NextItem() {
    ODC_FUNC_TRACE;
    currentIndex_ += 1;

    if (IsDone() == PFC_FALSE) {
      if (aggregate_ != NULL)
        return (*aggregate_)[currentIndex_];
    }
    return NULL;
  }
  /**
   * @brief   : Method to fetch first element from Keytree container
   *            during audit
   * @retval  : ConfigNode*
   */
  ConfigNode* AuditFirstItem() {
    ODC_FUNC_TRACE;
    currentIndex_ = 0;
    if (aggregate_ != NULL) {
      aggregate_->get_nodelist_keytree();
      return (*aggregate_)[currentIndex_];
    }
    return NULL;
  }

  /**
   * @brief   : Method to fetch first element from Keytree container
   *            during physical audit
   * @retval  : ConfigNode*
   */
  ConfigNode* PhysicalNodeFirstItem() {
    ODC_FUNC_TRACE;
    currentIndex_ = 0;
    if (aggregate_ != NULL) {
      aggregate_->get_nodelist_keytree();
      return (*aggregate_)[currentIndex_];
    }
    return NULL;
  }

  /**
   * @brief      : Method to check the end of Keytree container
   * @param [in] : None
   * @retval     : PFC_TRUE/PFC_FALSE
   */
  pfc_bool_t IsDone() {
    ODC_FUNC_TRACE;
    if (aggregate_ != NULL) {
      if (currentIndex_ < aggregate_->cfg_list_count()) {
        return PFC_FALSE;
      }
    }
    return PFC_TRUE;
  }

 private:
  /* keytree pointer for commoniterator class */
  KeyTree *aggregate_;

  /* Index to iterate container up to end */
  uint32_t currentIndex_;

  /**
   * @brief      : Method to fetch the current element from Keytree container
   * @param [in] : None
   * @retval     : current element in container
   */
  ConfigNode* CurrentItem() {
    if (aggregate_ != NULL)
    return (*aggregate_)[currentIndex_];
  }
};
}  // namespace vtndrvcache
}  // namespace unc
#endif
