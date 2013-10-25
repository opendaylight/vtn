/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * misc.cc - Miscellaneous utilities.
 */

#include <capa_module_stub.hh>
#include <dal_odbc_mgr.hh>
#include <tclib_module.hh>
#include <ctrlr_mgr.hh>
#include <config_mgr.hh>
#include "ut_util.hh"

using namespace unc::upll::dal;
using namespace unc::upll::test;
using namespace unc::upll::config_momgr;
using namespace unc::capa;
using namespace unc::tclib;

/*
 * Set up test environment.
 */
void
UpllTestEnv::SetUp()
{
  // Load stub modules.
  TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER,
                                          unc::tclib::TC_API_COMMON_SUCCESS);
  TcLibModule::stub_loadtcLibModule();

  // Initialize UpllConfigMgr.
  UpllConfigMgr::GetUpllConfigMgr();
}

/*
 * Clean up test environment.
 */
void
UpllTestEnv::TearDown()
{
  CtrlrMgr::GetInstance()->CleanUp();

  TcLibModule::stub_clearTcLibStubData();
  TcLibModule::stub_unloadtcLibModule();

  CapaModuleStub::stub_clearStubData();
  CapaModuleStub::stub_unloadCapaModule();

  DalOdbcMgr::clearStubData();
}
