/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vrt_if_momgr.hh"
#include <stdio.h>
#include "momgr_impl.hh"
#include "ipct_st.hh"
#include "unc/keytype.h"
#include "config_mgr.hh"
#include "dal/dal_odbc_mgr.hh"
#include "dal/dal_dml_intf.hh"
#include "tclib_module.hh"
#include "config_mgr.hh"
#include "dal_defines.hh"

using namespace unc::upll::kt_momgr;

  enum Method
  {Read1, Read2};

class VrtIfstub : public VrtIfMoMgr {
public:
VrtIfstub() {std::cout<<"In VrtIfConstructor \n";}
  static std::map<Method, upll_rc_t> stub_result_map;

upll_rc_t ReadConfigDB(ConfigKeyVal *ikey,
                                  upll_keytype_datatype_t dt_type,
                                  unc_keytype_operation_t op,
                                  DbSubOp dbop,
                                  DalDmlIntf *dmi,
                                  MoMgrTables tbl){
std::cout << "hellllloooooooooooooo\n";
return stub_result_map[Read1];
}

upll_rc_t ReadConfigDB(ConfigKeyVal *ikey,
                                  upll_keytype_datatype_t dt_type,
                                  unc_keytype_operation_t op,
                                  DbSubOp dbop,
                                  uint32_t &sibling_count,
                                  DalDmlIntf *dmi,
                                  MoMgrTables tbl) {
std::cout << "hellllloooooooooooooo\n";
return stub_result_map[Read2];
}

const MoManager *GetMoManager(unc_key_type_t kt) {
 std::cout<<" Get MoManager called \n";
 return new VrtIfstub();
}

						 
};
std::map<Method, upll_rc_t> VrtIfstub::stub_result_map;
