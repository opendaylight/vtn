/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <unc/upll_ipc_enum.h>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <dal/dal_cursor.hh>
#include <ctrlr_mgr.hh>
#include <capa_module/capa_intf.hh>
#include <capa_module/capa_module_stub.hh>
#include <tclib_module.hh>
#include <config_mgr.hh>
#include <momgr_intf.hh>
#include <momgr_impl.hh>
#include <config_mgr.hh>
#include <unc/upll_ipc_enum.h>
#include <policingprofile_entry_momgr.hh>
#include <flowlist_momgr.hh>
#include "../ut_util.hh"
#include <pfcxx/synch.hh>

#include <sql.h>

bool fatal_done;
pfc::core::Mutex  fatal_mutex_lock;

using namespace unc::upll::test;
using namespace unc::tclib;
using namespace std;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::ipc_util;
using namespace unc::upll::dal;
using namespace unc::capa;
using namespace unc::upll::config_momgr;

enum function {
  DiffConfigDB1 = 0,
};

static void GetKeyVal(ConfigKeyVal *&ikey) {
  key_policingprofile_entry_t *key = ZALLOC_TYPE(
      key_policingprofile_entry_t); 
  strncpy((char *)key->policingprofile_key.policingprofile_name,
          "PolicingProfile1", 33);
  val_policingprofile_entry_ctrl_t *policingprofile_val
      = ZALLOC_TYPE(val_policingprofile_entry_ctrl_t);
  policingprofile_val->cs_row_status = UNC_CS_APPLIED;
  policingprofile_val->valid[0] = UNC_VF_VALID;
  ConfigVal *cv = new ConfigVal(IpctSt::kIpcInvalidStNum,
            policingprofile_val);
  ikey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                            IpctSt::kIpcStKeyPolicingprofileEntry, key, cv);
}

class PolicingProfileEntryTest :
    public PolicingProfileEntryMoMgr, public ::testing::Test {
 public:
  static std::map<function, upll_rc_t> stub_result;

  upll_rc_t DiffConfigDB(upll_keytype_datatype_t dt_cfg1,
                         upll_keytype_datatype_t dt_cfg2,
                         unc_keytype_operation_t op,
                         ConfigKeyVal *&req,
                         ConfigKeyVal *&nreq,
                         DalCursor **cfg1_cursor,
                         DalDmlIntf *dmi,
                         uint8_t *cntrlr_id,
                         MoMgrTables tbl = MAINTBL,
                         bool read_withcs = false,
                         bool audit_diff_with_flag = false) {
    *cfg1_cursor = new DalCursor();
    GetKeyVal(req);
    return stub_result[DiffConfigDB1];
  }

  static void ClearStubData() {
    stub_result.clear();
  }
   virtual void SetUp() {}

   virtual void TearDown() {}

   virtual void TestBody() {}

};

std::map<function, upll_rc_t> PolicingProfileEntryTest::stub_result;
