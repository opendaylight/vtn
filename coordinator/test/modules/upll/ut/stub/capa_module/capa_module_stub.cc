/*
/*      Copyright (c) 2012 NEC Corporation                   */
/*      NEC CONFIDENTIAL AND PROPRIETARY                          */
/*      All rights reserved by NEC Corporation.                   */
/*      This program must be used solely for the purpose for      */
/*      which it was furnished by NEC Corporation.   No part      */
/*      of this program may be reproduced  or  disclosed  to      */
/*      others,  in  any form,  without  the  prior  written      */
/*      permission of NEC Corporation.    Use  of  copyright      */
/*      notice does not evidence publication of the program.      */

#include "capa_module_stub.hh"

namespace unc {
namespace capa {

 std::map<CapaModuleStub::CapaMethod,bool>CapaModuleStub::method_capa_map;
   uint32_t CapaModuleStub::max_instance_count_local;
   uint32_t *CapaModuleStub::num_attrs_local;
   uint8_t *CapaModuleStub::attrs_local;

  void CapaModuleStub::stub_setResultcode(CapaModuleStub::CapaMethod methodType ,bool res_code) {
	 method_capa_map.insert(std::make_pair(methodType,res_code));
    }

  bool CapaModuleStub::stub_getMappedResultCode(CapaModuleStub::CapaMethod methodType)
    {
	  if (0!= method_capa_map.count(methodType))
	  {
		  return method_capa_map[methodType];
	  }
	  return false;
    }

  void CapaModuleStub::stub_clearStubData()
  {
	  method_capa_map.clear();
  }

}

}
