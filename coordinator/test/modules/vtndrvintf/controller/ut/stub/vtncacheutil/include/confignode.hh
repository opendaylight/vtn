/*
 * Copyright (c) 2013 NEC Corporation
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

namespace unc {
namespace vtndrvcache {

class ConfigNode {
 public:
  /**
   * @brief : default constructor
   */
  ConfigNode() {}
  /**
   * @brief : default virtual destructor
   */

  virtual ~ConfigNode() {}


  virtual uint32_t get_operation() {
    return 1;
  }

  virtual unc_key_type_t get_type() {
    return (unc_key_type_t) -1;
  }
};

class RootNode : public ConfigNode {
 public:
  /**
   * @brief : default constructor
   */
  RootNode() { }

  /**
   * @brief : default desstructor
   */
  ~RootNode() { }
};
}  // namespace vtndrvcache
}  // namespace unc
#endif


