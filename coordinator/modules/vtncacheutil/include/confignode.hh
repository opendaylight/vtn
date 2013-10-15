/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef _CONFIGNODE_HH_
#define _CONFIGNODE_HH_

#include <pfc/debug.h>
#include <pfc/log.h>
#include <string>
#include <map>
#include <vector>
#include "unc/keytype.h"

using namespace std;

unsigned const NUM_SPACES = 4;

namespace unc {
namespace vtndrvcache {

/**
 * @brief  Returns the type string corresponding the the keytype. This is used
 *   to print the keytree structure
 * param[in] search_type
 * param[out] Kettype converted to string
 */

string TypeToStrFun(unc_key_type_t search_type);

class ConfigNode {
 public:
  /**
   * @brief : default constructor
   */
  ConfigNode();
  /**
   * @brief : default virtual destructor
   */

  virtual ~ConfigNode() {}

  /**
   * @brief  Method to retrieve each node from the Keytree and populate in
   *         the vector
   * param[in] value_list
   * param[out] TREE_OK
   */
  uint32_t get_node_list(vector<ConfigNode*>&value_list);

  /**
   * @brief  This virtual method returns the Keytype of a node
   * param[in] none
   * param[out] unc_key_type_t
   */
  virtual unc_key_type_t get_type() {
    return (unc_key_type_t) -1;
  }

  /**
   * @brief  This virtual method returns the Search Key of the node
   * param[in] none
   * param[out] string
   */
  virtual string get_key() {
    return "";
  }

  /**
   * @brief  This virtual method returns the parent Key of the node
   * param[in] none
   * param[out]string
   */
  virtual string get_parent_key() {
    return "";
  }

  /**
   * @brief  This method inserts the node in the cache
   * param[in] confignode *
   * param[out]uint32_t
   */
  uint32_t add_child_to_list(ConfigNode *node_ptr);

  /**
   * @brief  This method prints each node information
   * param[in] level_index
   * param[out] none
   */
  void print(int level_index);

  /**
   * @brief  This method prints the Key of the
   *         particular node
   * param[in] level_index
   * param[out] none
   */
  virtual void print_key(int level_index) {
    return;
  }

  /**
   * @brief  This virtual method returns the operation
   * param[in] none
   * param[out]string
   */
  virtual uint32_t get_operation() {
    return operation_;
  }

  /**
   * @brief  This method clear the cache
   * param[in] none
   * param[out]none
   */
  uint32_t  clear_kt_map();

  uint32_t get_cfgnode_count() {
    return cfgnode_count_;
  }

 protected:
  map<unc_key_type_t, vector<ConfigNode*> > child_list_;
  uint32_t operation_;
  uint32_t cfgnode_count_;
};
class RootNode : public ConfigNode {
 public:
  /**
   * @brief : default constructor
   */
  RootNode();

  /**
   * @brief : default desstructor
   */
  ~RootNode();

  /**
   * @brief  This method return root
   * param[in] none
   * param[out]UNC_KT_ROOT
   */
  unc_key_type_t get_type() {
    return UNC_KT_ROOT;
  }

  /**
   * @brief  This method prints each node information
   * param[in] level_index
   * param[out] none
   */
  void print_key(int level_index);
};
}  // end of namespace vtndrvcache
}  // end of namespace unc
#endif
