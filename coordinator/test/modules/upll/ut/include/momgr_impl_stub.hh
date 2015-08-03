/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <limits.h>
#include <gtest/gtest.h>
#include <pfc/event.h>
#include <cxx/pfcxx/synch.hh>
#include <unc/keytype.h>

#include <ipc_util.hh>
#include <cxx/pfcxx/module.hh>
#include "upll_validation.hh"
#include "capa_module_stub.hh"
#include "dal/dal_dml_intf.hh"
#include "dal/dal_odbc_mgr.hh"
#include <momgr_intf.hh>
#include <momgr_impl.hh>
#include <ctrlr_mgr.hh>
#include <pfcxx/synch.hh>
#include <vtn_momgr.hh>
#include <vbr_momgr.hh>

bool fatal_done;
pfc::core::Mutex  fatal_mutex_lock;

using namespace unc::upll::kt_momgr;
using namespace unc::upll::ipc_util;
using namespace unc::upll::dal;
using namespace unc::upll::config_momgr;
using namespace unc::upll::test;

using std::list;
using std::multimap;
using std::string;
using std::set;
using std::map;

using ::testing::TestWithParam;
using ::testing::Values;

namespace unc {
namespace upll {
namespace kt_momgr {

enum function {
  ReadConfigDB1,
  UpdateConfigDB1,
  BindAttr1
};

class VtnMoMgrTest : public VtnMoMgr {
 public:
  static std::map<function, upll_rc_t> stub_result;
  upll_rc_t ReadConfigDB(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 unc_keytype_operation_t op,
                                 DbSubOp dbop ,
                                 DalDmlIntf *dmi,
                                 MoMgrTables tbl) {
    //SET_USER_DATA_FLAGS(okey, 0x00);
    reinterpret_cast<key_user_data_t *>(ikey->get_user_data())->flags = 0x00;
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

  upll_rc_t BindAttr(DalBindInfo *db_info,
                     ConfigKeyVal *&req,
                     unc_keytype_operation_t op,
                     upll_keytype_datatype_t dt_type,
                     DbSubOp dbop, MoMgrTables tbl = MAINTBL) {
    return stub_result[BindAttr1];
  }

  static void ClearStubData() {
    stub_result.clear();
  }

};

std::map<function, upll_rc_t> VtnMoMgrTest::stub_result;

   }
  }
 }
