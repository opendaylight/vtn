/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VTNCACHEMOD_HH__
#define __VTNCACHEMOD_HH__

#include <pfcxx/module.hh>
#include <map>
#include <string>
#include "unc/keytype.h"

namespace unc {
namespace vtndrvcache {

class VtnDrvCacheMod: public pfc::core::Module {
 public:
  /**
   ** @brief : VtnDrvCacheMod Constructor
   **/
  explicit VtnDrvCacheMod(const pfc_modattr_t *mattr);

  /**
   ** @brief : Call init function to initialize module
   **/
  pfc_bool_t  init();

  /**
   ** @brief : Call fini function to release module
   **/
  pfc_bool_t  fini();

  /**
   * @brief : keyTypeStrMap is a map to contain the keytype names
   **/
  typedef std::map<unc_key_type_t, std::string> KeyStrMap;
  static KeyStrMap keyTypeStrMap;
  typedef struct {
    unc_key_type_t keytype;
    char name[50];
  }key_name_pair;
};
}  // namespace vtndrvcache
}  // namespace unc
#endif
