/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * misc.cc - Miscellaneous utilities.
 */

#include <pfcxx/ipc_server.hh>
#include <tclib_module.hh>
#include "PhysicalLayerStub.hh"
#include "ut_util.hh"

using namespace unc::uppl::test;
using namespace unc::tclib;
using namespace pfc::core::ipc;

/*
 * Set up test environment.
 */
void
UpplTestEnv::SetUp() {
  TcLibModule::stub_loadtcLibModule();
  TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER,
                                          TC_API_COMMON_SUCCESS);

  PhysicalLayerStub::loadphysicallayer();
}

/*
 * Clean up test environment.
 */
void
UpplTestEnv::TearDown() {
  ServerSession::clearStubData();
  ODBCManager::clearStubData();
  PhysicalLayerStub::unloadphysicallayer();
  TcLibModule::stub_unloadtcLibModule();
}
