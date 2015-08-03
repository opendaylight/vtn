/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <policingprofile_momgr.hh>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <ctrlr_mgr.hh>
#include <capa_module/capa_intf.hh>
#include <capa_module/capa_module_stub.hh>
#include <config_mgr.hh>
#include "../ut_util.hh"
#include <pfcxx/synch.hh>


using namespace unc::upll::dal;
using namespace unc::capa;
using namespace unc::upll::config_momgr;
using namespace unc::upll::test;
using namespace unc::upll::kt_momgr;

enum function {
  ReadConfigDB1,
  UpdateConfigDB1
};


//std::map<function, upll_rc_t> PolicingProfileMoMgrStub::stub_result;
class PolicingProfileMoMgrStub 
   : public PolicingProfileMoMgr, public ::testing::Test {
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
  upll_rc_t ReadConfigDB(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 unc_keytype_operation_t op,
                                 DbSubOp dbop ,
                                 uint32_t &sibling_count,
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

  static void ClearStubData() {
    stub_result.clear();
  }
   virtual void SetUp() {}

  virtual void TearDown() {}

  virtual void TestBody() {}
};
