/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef KEYTREE_CONFIG_HH_
#define KEYTREE_CONFIG_HH_

#include "confignode.hh"
#include <vector>
#include <utility>
#include <map>
#include <string>

#define TREE_OK                      1000
#define ERR_ADD_CHILD_TO_TREE_FAILD  1005
#define CONFIGARR_SIZE                50
#define PFCDRVAPI_RESPONSE_FAILURE 1
#define PFCDRVAPI_RESPONSE_SUCCESS 0
#define CACHEMGR_RESPONSE_SUCCESS 0

namespace unc {
namespace vtndrvcache {

struct key_tree {
  unc_key_type_t id; /* Key type */
  unc_key_type_t parent; /* Parent */
  int order; /* Sibling order */
};

class CommonIterator;
typedef std::map<std::string, ConfigNode* > configNodeHash;

class KeyTree {
 public:
  friend class CommonIterator;

  /**
   ** Method to create Cache and return the pointer to it
   ** @param [out] - KeyTree*
   **/
  static KeyTree* create_cache();

  /**
   ** Constructor to insert the root node in the array of map
   **/
  KeyTree() {
    configHashArr[0].insert(pair <std::string, ConfigNode*>
                            ("ROOT", &node_tree));
  }
  /**
   * Destructor
   **/
  ~KeyTree() {}
  void print();
  /**
   ** Method to traverse the tree and populate the nodes into the vector
   ** @param [in] - ctrlid 
   ** @param [in] - value_list
   ** @param [out] - TREE_OK
   **/
  uint32_t get_nodelist_keytree(
      std::vector<ConfigNode*>&value_list);
  /**
   ** Method to return the Keytype of Parent given the search Key
   ** @param [in] - search_type 
   ** @param [out] - unc_key_type_t
   **/
  unc_key_type_t get_parenttype(unc_key_type_t search_type);

  /**
   ** Method to validate if parent exists for this particular node
   ** @param [in] - value_list 
   ** @param [out] - true/false
   **/
  bool  validate_parentkey(ConfigNode* value_list);

  /**
   ** Method to add confignode to the cache for Audit
   ** @param [in] - value_list 
   ** @param [out] - TREE_OK
   **/
  uint32_t append_audit_node(const std::vector<ConfigNode*>&value_list);

  /**
   ** Method to add confignode to the cache for Audit
   ** @param [in] - value_list 
   ** @param [out] - TREE_OK
   **/
  uint32_t append_audit_node(ConfigNode* value_list);

  /**
   ** Method to add confignode to the cache for commit 
   ** @param [in] - value_list 
   ** @param [out] - TREE_OK
   **/
  uint32_t append_commit_node(ConfigNode* value_list);

  /**
   ** Method to add node to the cache and search map by validation 
   ** @param [in] - value_list 
   ** @param [out] - TREE_OK/PFCDRVAPI_RESPONSE_FAILURE
   **/
  uint32_t add_node_to_tree(ConfigNode* child_ptr);

  /**
   ** Method to search and retrieve the node from the search map
   ** @param [in] - key 
   ** @param [in] - key_Type 
   ** @param [out] - ConfigNode*
   **/
  ConfigNode* get_node_from_hash(std::string key,
                                 unc_key_type_t key_Type);

  /**
   ** Method to check if the particular key is already present in search map
   ** @param [in] - key 
   ** @param [in] - key_Type 
   ** @param [out] - true/false 
   **/
  bool is_already_present(std::string key, unc_key_type_t key_Type);

  /**
   ** Method to insert node to the search map
   ** @param [in] - child_ptr 
   ** @param [out] - TREE_OK
   **/
  uint32_t add_child_to_hash(ConfigNode* child_ptr);


  /**
   ** Method to clear the elements of commit cache 
   ** @param [in] - None
   ** @param [out] - None
   **/
  void clear_commit_cache();

  /**
   ** Method to clear the elements of audit cache 
   ** @param [in] - None
   ** @param [out] - None
   **/
  void clear_audit_cache();

  /**
   ** Method to clear the elements of vector 
   ** @param [in] - None
   ** @param [out] - None
   **/
  void clear_cache_vector();

  /**
   ** Method to clear the elements of search map 
   ** @param [in] - None
   ** @param [out] - None
   **/
  void clear_search_cache();

  /**
   ** Method to clear the elements of audit cache 
   ** @param [in] - None
   ** @param [out] - None
   **/
  CommonIterator* get_iterator();
  /**
   ** Overloaded [] operator that returns each element of the container 
   ** @param [in] - None
   ** @param [out] - None
   **/
  ConfigNode* operator[](int itemIndex) {
    return cfg_node_list[itemIndex];
  }

  /**
   ** Method to calculate the size of container cfg_node_list 
   ** @param [in] - None
   ** @param [out] - None
   **/
  int cfg_list_count() {
    return cfg_node_list.size();
  }
  uint32_t read_all_cfgnode(unc_key_type_t key,
                            std::vector<ConfigNode*>& vec_vtn);

  configNodeHash configHashArr[CONFIGARR_SIZE];
  typedef std::vector<ConfigNode*> Config_node_list;
  Config_node_list cfg_node_list;
  RootNode node_tree;
};

class CommonIterator {
  KeyTree *aggregate_;
  int currentIndex_;

 public:
  explicit CommonIterator(KeyTree *aggregate)
      : aggregate_(aggregate)
        , currentIndex_(0) {
        }
  ConfigNode* FirstItem() {
    currentIndex_ = 0;
    return (*aggregate_)[currentIndex_];
  }

  ConfigNode* NextItem() {
    currentIndex_ += 1;

    if (IsDone() == false) {
      return (*aggregate_)[currentIndex_];
    } else {
      return NULL;
    }
  }
  ConfigNode* AuditFirstItem() {
    currentIndex_ = 0;
    aggregate_->get_nodelist_keytree(aggregate_->cfg_node_list);
    return (*aggregate_)[currentIndex_];
  }

  ConfigNode* CurrentItem() {
    return (*aggregate_)[currentIndex_];
  }

  bool IsDone() {
    if (currentIndex_ < aggregate_->cfg_list_count()) {
      return false;
    }
    return true;
  }
};
}
}
#endif


