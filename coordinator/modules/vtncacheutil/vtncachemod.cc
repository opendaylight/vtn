/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include <pfcxx/module.hh>

namespace unc {
namespace vtndrvcache {

class vtndrvcachemod: public pfc::core::Module {
 public:
  explicit vtndrvcachemod(const pfc_modattr_t *mattr):
                  pfc::core::Module(mattr) {
  }

  pfc_bool_t  init() {
    return PFC_TRUE;
  }

  pfc_bool_t  fini() {
    return PFC_TRUE;
  }
};
}
}

PFC_MODULE_IPC_DECL(unc::vtndrvcache::vtndrvcachemod, 0);
