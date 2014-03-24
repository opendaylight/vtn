/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "PhysicalLayerStub.hh"

using namespace unc::uppl;
using namespace pfc::core;

static PhysicalLayer  theInstance(NULL);
static bool initialized = false;

void
PhysicalLayerStub::loadphysicallayer() {
  Module::physical = &theInstance;
  if (!initialized) {
    initialized = true;
    ASSERT_TRUE(theInstance.init());
  }
}

void
PhysicalLayerStub::unloadphysicallayer() {
  Module::physical = NULL;
}
