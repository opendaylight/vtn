/*
 * Copyright (c) 2012-2013 NEC Corporation
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
  return PFC_TRUE;
}

pfc_bool_t DalModule::fini(void) {
  pfc_log_notice("Fini");
  DalErrorHandler::ClearErrorMap();

  return PFC_TRUE;
}

}  // namespace dal
}  // namespace upll
}  // namespace unc

PFC_MODULE_DECL(unc::upll::dal::DalModule);
