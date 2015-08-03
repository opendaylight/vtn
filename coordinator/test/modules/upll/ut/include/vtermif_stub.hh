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
#include "vtn_momgr.hh"
#include "vterm_if_momgr.hh"
#include "unc/keytype.h"
#include "config_mgr.hh"
#include "dal_odbc_mgr.hh"
#include "dal_dml_intf.hh"
#include "capa_intf.hh"
#include "capa_module_stub.hh"
#include "tclib_module.hh"
#include "ctrlr_mgr.hh"
#include "momgr_intf_stub.hh"
#include "vbr_if_momgr.hh"

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
  UpdateConfigDB1
};

enum testcase_num {
GetPortmapValid = 1,
VtermLogicalPortIdSame,
VtermLogicalPortIdSameKey,
LogicalPortIdSamekey,
};

class VtermIfMoMgrTest: public VtermIfMoMgr, public ::testing::Test {
 public:
  static std::map<function, upll_rc_t> stub_result;
  upll_rc_t ReadConfigDB(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 unc_keytype_operation_t op,
                                 DbSubOp dbop ,
                                 DalDmlIntf *dmi,
                                 MoMgrTables tbl) {
    if (ikey->get_cfg_val() == NULL) {
      val_vterm_if *vtermif_val = reinterpret_cast<val_vterm_if*>(
          ConfigKeyVal::Malloc(
              sizeof(val_vterm_if_t)));

      if (testcase_id == GetPortmapValid) {
        vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
        vtermif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]  = UNC_VF_VALID;
      }
      ConfigVal *ck_val = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
      ikey->AppendCfgVal(ck_val);
    }

    if (testcase_id == VtermLogicalPortIdSame) {
      val_vterm_if_t *vtermif_val =  reinterpret_cast<val_vterm_if*>(
                                            GetVal(ikey));
      vtermif_val->portmap.vlan_id = 10;
      vtermif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
      vtermif_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      vtermif_val->portmap.tagged = 1;
    }
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
 protected:
  virtual void SetUp() {}

  virtual void TearDown() {}
 
  virtual void TestBody() {}
};
class VtnMoMgrTest: public VtnMoMgr, public ::testing::Test {
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

 protected:
  virtual void SetUp() {}

  virtual void TearDown() {}

  virtual void TestBody() {}
};



