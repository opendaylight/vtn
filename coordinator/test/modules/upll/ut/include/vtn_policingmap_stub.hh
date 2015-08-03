/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <upll/vtn_policingmap_momgr.hh>
#include <upll/config_mgr.hh>
#include <iostream>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <capa_module/capa_intf.hh>
#include <capa_module/capa_module_stub.hh>
#include <config_mgr.hh>
#include <ctrlr_mgr.hh>
#include <tclib_module/tclib_module.hh>
#include "../ut_util.hh"

using namespace std;
using namespace unc::upll::dal;
using namespace unc::capa;
using namespace unc::upll::config_momgr;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::test;

//bool fatal_done;
//pfc::core::Mutex  fatal_mutex_lock;

enum function {
  DiffConfigDB1 = 0,
  UpdateConfigDB1,
};

static void GetKeyVal(ConfigKeyVal *&ikey) {
  key_vtn_t *vtn_key = ZALLOC_TYPE(key_vtn_t);
  strcpy((char*)vtn_key->vtn_name,"vtn1");
  ConfigVal *val = NULL; 
  val_policingmap_t *policingmap_val = reinterpret_cast<val_policingmap_t *>(
      ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  strcpy((char*)policingmap_val->policer_name,"policer_name");
  val = new ConfigVal(IpctSt::kIpcStValPolicingmap, policingmap_val);
  ikey = new ConfigKeyVal(UNC_KT_VTN_POLICINGMAP,
                          IpctSt::kIpcStKeyVtn, vtn_key, val);
  }

class VtnPolicingMapTest :
    public VtnPolicingMapMoMgr, public ::testing::Test {
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

  upll_rc_t UpdateConfigDB(ConfigKeyVal *ikey,
                           upll_keytype_datatype_t dt_type,
                           unc_keytype_operation_t op,
                           DalDmlIntf *dmi,
                           DbSubOp *pdbop,
                           MoMgrTables tbl = MAINTBL) {
   return stub_result[UpdateConfigDB1];
 }

  static void ClearStubData() {
    stub_result.clear();
  }
   virtual void SetUp() {}

   virtual void TearDown() {}

   virtual void TestBody() {}

};

std::map<function, upll_rc_t> VtnPolicingMapTest::stub_result;
