/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <pfcxx/module.hh>

namespace unc {
namespace restjson {

class RestJsonMod: public pfc::core::Module {
 public:
  /*
   * @brief - Parametrised Constructor
   */
  explicit RestJsonMod(const pfc_modattr_t *mattr) :
      pfc::core::Module(mattr) {
      }

  /*
   * @brief - init method
   * @retval- pfc_bool_t
   */
  pfc_bool_t init() {
    return PFC_TRUE;
  }

  /*
   * @brief - fini method
   * @retval- pfc_bool_t
   */
  pfc_bool_t fini() {
    return PFC_TRUE;
  }
};
}  // namespace restjson
}  // namespace unc
PFC_MODULE_IPC_DECL(unc::restjson::RestJsonMod, 0);
