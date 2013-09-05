#ifndef _IMPORT_REQUEST_H_
#define _IMPORT_REQUEST_H_

/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Import Request
 * @file     itc_import_request.cc
 *
 */

#include <string>
#include "physical_common_def.hh"
#include "unc/uppl_common.h"
#include "physicallayer.hh"
#include "physical_itc_req.hh"
#include "itc_kt_controller.hh"
#include "ipc_connection_manager.hh"

namespace unc {
namespace uppl {

class ImportRequest:public ITCReq {
 public:
  ImportRequest();
  ~ImportRequest();
  UpplReturnCode ProcessRequest(OdbcmConnectionHandler *db_conn,
                                uint32_t operation,
                            key_ctr_t obj_key_ctr);
  UpplReturnCode StartImport(OdbcmConnectionHandler *db_conn,
                             key_ctr_t obj_key_ctr);
  UpplReturnCode MergeConfiguration(key_ctr_t obj_key_ctr);
  UpplReturnCode ClearImportConfig(key_ctr_t obj_key_ctr);
};
}/*namespace uppl*/
}/*namespace unc*/

#endif


