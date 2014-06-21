/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <vtn_cache_mod.hh>
#include "vtn_drv_module.hh"

static bool  drvcache_initialized = false;
static unc::vtndrvcache::VtnDrvCacheMod drvcache(NULL);

namespace unc {
namespace driver {
std::map<unc_key_type_t, pfc_ipcstdef_t*>VtnDrvIntf::key_map;
std::map<unc_key_type_t, pfc_ipcstdef_t*>VtnDrvIntf::val_map;

VtnDrvIntf VtnDrvIntf::theInstance(NULL);

VtnDrvIntf::VtnDrvIntf(const pfc_modattr_t* attr)
: pfc::core::Module(attr) {
}

void
VtnDrvIntf::stub_loadVtnDrvModule(void) {
  if (!drvcache_initialized) {
    // Initialize vtncacheutil module.
    drvcache.init();
    drvcache_initialized = true;
  }
  pfc::core::Module::vtndrv = &theInstance;
}

void
VtnDrvIntf::stub_unloadVtnDrvModule(void) {
  pfc::core::Module::vtndrv = NULL;
}
}  // namespace driver
}  // namespace unc

