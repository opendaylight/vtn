/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vlanmap_momgr.hh"
#include <stdio.h>
#include "momgr_impl.hh"
#include "ipct_st.hh"
#include "unc/keytype.h"
#include "dal/dal_odbc_mgr.hh"
#include "dal/dal_dml_intf.hh"
#include "tclib_module.hh"
#include "dal_defines.hh"
#include "config_mgr.hh"

using namespace unc::upll::kt_momgr;

enum function {
  ReadConfigDB1,
};

class VlanmapStub : public VlanMapMoMgr {
public: 
  static std::map<function, upll_rc_t> stub_result;
  upll_rc_t ReadConfigDB(ConfigKeyVal *ikey,
                                  upll_keytype_datatype_t dt_type,
                                  unc_keytype_operation_t op,
                                  DbSubOp dbop,
                                  DalDmlIntf *dmi,
                                  MoMgrTables tbl) {
    printf("\nInside stub ReadConfigDb1");
    return stub_result[ReadConfigDB1];
}
MoManager *GetMoManager(unc_key_type_t kt) {

  std::cout<<"GetMoManger Stub being called\n";
  return new VlanmapStub();
}
};
std::map<function, upll_rc_t> VlanmapStub::stub_result;

