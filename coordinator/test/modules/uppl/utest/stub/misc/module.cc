/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <string.h>
#include <pfcxx/module.hh>

namespace pfc {
namespace core {

Module* Module::physical = NULL;
Module* Module::tcLib = NULL;

Module *
Module::getModule(const char* moduleName) {
  if (!strcmp(moduleName, "uppl")) {
    return physical;
  }
  if (!strcmp(moduleName, "tclib")) {
    return tcLib;
  }

  return NULL;
}

}
}
