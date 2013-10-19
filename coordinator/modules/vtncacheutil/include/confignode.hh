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

unsigned const NUM_SPACES = 4;

namespace unc {
namespace vtndrvcache {

/**
 * @brief     : Returns the type string corresponding the the keytype. This is used
 to print the keytree structure
 * param[in]  : search_type
 * @retval    : string
 */

std::string TypeToStrFun(unc_key_type_t search_type);

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
   * @brief    : Method to retrieve each node from the Keytree and populate in
   *             the vector
   * param[in] : value_list
   * @retval   : iuint32_t
   */
  uint32_t get_node_list(std::vector<ConfigNode*>&value_list);

  /**
   * @brief   : This virtual method returns the Keytype of a node
   * @retval  : unc_key_type_t
   */
  virtual unc_key_type_t get_type() {
    return (unc_key_type_t) -1;
  }

  /**
   * @brief   : This virtual method returns the Search Key of the node
   * @retval  : string
   */
  virtual std::string get_key() {
    return "";
  }

  /**
   * @brief  : This virtual method returns the parent Key of the node
   * @retval : string
   */
  virtual std::string get_parent_key() {
    return "";
  }

  /**
   * @brief    : This method inserts the node in the cache
   * param[in] : confignode *
   * @retval   : uint32_t
   */
  uint32_t add_child_to_list(ConfigNode *node_ptr);

  /**
   * @brief    : This method prints each node information
   * param[in] : level_index
   */
  void print(int level_index);

  /**
   * @brief    : This method prints the Key of the
   *             particular node
   * param[in] : level_index
   */
  virtual void print_key(int level_index) {
    return;
  }

  /**
   * @brief  : This virtual method returns the operation
   * @retval : string
   */
  virtual uint32_t get_operation() {
    return operation_;
  }

  /**
   * @brief     : This method gets the confignode count
   * @param[in] : none
   * @retval    : node count
   */
  uint32_t get_cfgnode_count() {
    return cfgnode_count_;
  }

 protected:
  std::map<unc_key_type_t, std::vector<ConfigNode*> > child_list_;
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
   * @brief  : This method return root
   * @retval : UNC_KT_ROOT
   */
  unc_key_type_t get_type() {
    return UNC_KT_ROOT;
  }

  /**
   * @brief    : This method prints each node information
   * param[in] : level_index
   */
  void print_key(int level_index);
};
}  // end of namespace vtndrvcache
}  // end of namespace unc
#endif
