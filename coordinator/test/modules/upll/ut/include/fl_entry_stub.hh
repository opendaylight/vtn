/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <limits.h>
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <pfc/ipc.h>
#include <pfc/ipc_struct.h>
#include <unc/keytype.h>
#include <pfcxx/synch.hh>
#include "flowlist_entry_momgr.hh"
#include "momgr_impl.hh"
#include "unc/keytype.h"
#include "config_mgr.hh"
#include "dal_odbc_mgr.hh"
#include "dal_dml_intf.hh"
#include "capa_intf.hh"
#include "capa_module_stub.hh"
#include "tclib_module.hh"
#include "ctrlr_mgr.hh"
#include "momgr_intf_stub.hh"

using ::testing::Test;
using ::testing::TestWithParam;
using ::testing::Values;
using namespace std;
using namespace unc::upll;
using namespace unc::tclib;
using namespace unc::upll::dal;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::config_momgr;
using namespace unc::capa;
using namespace pfc::core;
using namespace unc::upll::dal::schema::table;

extern int testcase_id;
enum function {
  ReadConfigDB1,
  UpdateConfigDB1,
  ValidateImportWithRunning1
};

class FlowlistEntryTest: public FlowListEntryMoMgr, public ::testing::Test {
 public:
  static std::map<function, upll_rc_t> stub_result;
   upll_rc_t ReadConfigDB(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 unc_keytype_operation_t op,
                                 DbSubOp dbop ,
                                 DalDmlIntf *dmi,
                                 MoMgrTables tbl) {
    return stub_result[ReadConfigDB1];
  }

  upll_rc_t UpdateConfigDB(ConfigKeyVal *ikey,
                           upll_keytype_datatype_t dt_type,
                           unc_keytype_operation_t op,
                           DalDmlIntf *dmi,
                           DbSubOp *pdbop,
                           MoMgrTables tbl) {
    return stub_result[UpdateConfigDB1];
  }

upll_rc_t ValidateImportWithRunning(unc_key_type_t keytype,
                               const char *ctrlr_id, ConfigKeyVal *ikey,
                               unc_keytype_operation_t op[],
                               int nop, DalDmlIntf *dmi) {
 return stub_result[ValidateImportWithRunning1];
}
 protected:
  virtual void SetUp() {}

  virtual void TearDown() {}
 
  virtual void TestBody() {}
};

