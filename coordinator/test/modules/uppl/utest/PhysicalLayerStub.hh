/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PHYSICAL_LAYER_STUB_HH_
#define _PHYSICAL_LAYER_STUB_HH_

#include <pfc/debug.h>
#include <pfc/log.h>
#include <pfc/hostaddr.h>
#include <pfcxx/module.hh>
#include <pfcxx/ipc_server.hh>
#include <pfcxx/ipc_client.hh>
#include <pfcxx/synch.hh>
#include <physicallayer.hh>

class PhysicalLayerStub : public pfc::core::Module {
 public:
  static void loadphysicallayer();
  static void unloadphysicallayer();
};

#endif
