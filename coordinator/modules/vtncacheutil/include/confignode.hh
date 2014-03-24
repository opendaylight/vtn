/*
 * Copyright (c) 2013-2014 NEC Corporation
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
#include <unc/unc_base.h>
#include <string>
#include <map>
#include <vector>
#include "unc/keytype.h"
#include "uncxx/odc_log.hh"

unsigned const NUM_SPACES = 4;
unsigned const CACHEMGR_CONFIGARR_SIZE = 518;


namespace unc {
namespace vtndrvcache {

/**
 * @brief     : Structure key_information contain key and key_type, that help
 *            : for remove entries from hash after delete node operation from
 *            : cache
 * param[in]  : NONE
 * @retval    : NONE
 */
struct key_information {
  key_information():key(), key_type() {}
  std::string key;
  unc_key_type_t key_type;
};

/**
 * @brief     : Returns the type string corresponding the the keytype. This is used
 *            : to print the keytree structure
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
   * @brief    : Method to retrieve node from the Keytree and populate in
   *             the vector value_list
   * param[in] : value_list(vector of ConfigNode*)
   * @retval   : UncRespCode(UNC_RC_SUCCESS)
   */
  UncRespCode get_node_list(std::vector<ConfigNode*>& value_list);

  /**
   * @brief   : This virtual method returns the Keytype of a node
   * @retval  : unc_key_type_t
   */
  virtual unc_key_type_t get_type_name() {
    return (unc_key_type_t) -1;
  }

  /**
   * @brief   : This virtual method returns the Search Key of the node
   * @retval  : string(return combined of child plus parent name and so on)
   */
  virtual std::string get_key_generate() {
    return "";
  }

  /**
   * @brief   : This virtual method returns the name of the key
   * @retval  : string (return name of vtn and vbridge)
   */
  virtual std::string get_key_name() {
    return "";
  }

  /**
   * @brief  : This virtual method returns the parent Key of the node
   * @retval : string(return combined of parent and parent name and so on)
   */
  virtual std::string get_parent_key_name() {
    return "";
  }

  /**
   * @brief    : This method Adds the node under the given parent in cache
   * param[in] : confignode *
   * @retval   : UncRespCode(UNC_RC_SUCCESS)
   */
  UncRespCode add_child_to_list(ConfigNode *node_ptr);

  /**
   * @brief    : This method delete the node from cache
   * param[in] : confignode *, vector<key_information>
   * @retval   : UncRespCode(UNC_RC_SUCCESS)
   */
  UncRespCode delete_child_node(ConfigNode *node_ptr,
                            std::vector<key_information>&);

  /**
   * @brief    : This method deletes the nodes under the given parent from cache
   * param[in] : vector<key_information>
   * @retval   : UncRespCode(UNC_RC_SUCCESS)
   */
  UncRespCode clear_child_list(std::vector<key_information>&);

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
   * @retval : uint32_t(create/update/delete)
   */
  virtual uint32_t get_operation() {
    return operation_;
  }

  /**
   * @brief     : This method gets the confignode count
   * @param[in] : none
   * @retval    : node count
   */

 protected:
  /**
   * @brief : child_list_ is a map to contain the configuration in tree manner
   */
  std::map<unc_key_type_t, std::vector<ConfigNode*> > child_list_;
  /**
   * @brief : Operation contain whether create/update/delete
   */
  uint32_t operation_;
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
  unc_key_type_t get_type_name() {
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
