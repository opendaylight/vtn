/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfcxx/module.hh"
#include "uncxx/upll_log.hh"
#include "dal_error_handler.hh"
#include "dal_module.hh"
#include "dal_schema.hh"
#include "dal_odbc_mgr.hh"

namespace unc {
namespace upll {
namespace dal {

DalModule::DalModule(const pfc_modattr_t *attr)
    :pfc::core::Module(attr) {
      /*  */
    }

DalModule::~DalModule() {
}

pfc_bool_t DalModule::init(void) {
  pfc_log_notice("Init");
  DalErrorHandler::FillErrorMap();
  // Fill TableName-Index map
  if (!DalOdbcMgr::FillTableName2IndexMap()) {
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

pfc_bool_t DalModule::fini(void) {
  pfc_log_notice("Fini");
  // There could be transactions in progress. So not clearing error map.
  // DalErrorHandler::ClearErrorMap();

  return PFC_TRUE;
}

}  // namespace dal
}  // namespace upll
}  // namespace unc

PFC_MODULE_DECL(unc::upll::dal::DalModule);
