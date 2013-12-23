/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "capa_module_stub.hh"

namespace unc {
namespace capa {

std::map<CapaModuleStub::CapaMethod, bool>CapaModuleStub::method_capa_map;
uint32_t CapaModuleStub::max_instance_count_local;
uint32_t *CapaModuleStub::num_attrs_local;
uint8_t *CapaModuleStub::attrs_local;

static CapaModuleStub  theInstance(NULL);

void
    CapaModuleStub::stub_loadCapaModule(void) {
      pfc::core::Module::capaModule = &theInstance;
    }

void
    CapaModuleStub::stub_unloadCapaModule(void) {
      pfc::core::Module::capaModule = NULL;
    }

void
    CapaModuleStub::stub_setResultcode(CapaModuleStub::CapaMethod methodType,
                                       bool res_code) {
      method_capa_map.insert(std::make_pair(methodType, res_code));
    }

bool
    CapaModuleStub::stub_getMappedResultCode
                                   (CapaModuleStub::CapaMethod methodType) {
      if (method_capa_map.count(methodType) != 0) {
        return method_capa_map[methodType];
      }
      return false;
    }

void
    CapaModuleStub::stub_clearStubData() {
      method_capa_map.clear();
    }
}  //  namespace capa
}  //  namespace unc
