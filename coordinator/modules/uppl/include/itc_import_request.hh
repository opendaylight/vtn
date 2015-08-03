/*
 * Copyright (c) 2012-2015 NEC Corporation
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

#ifndef _IMPORT_REQUEST_H_
#define _IMPORT_REQUEST_H_

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
  UncRespCode ProcessRequest(OdbcmConnectionHandler *db_conn,
                                uint32_t operation,
                            key_ctr_t obj_key_ctr, uint32_t session_id,
                            uint32_t config_id);
  UncRespCode StartImport(OdbcmConnectionHandler *db_conn,
                             key_ctr_t obj_key_ctr, uint32_t session_id,
                             uint32_t config_id);
  UncRespCode MergeConfiguration(uint32_t session_id, uint32_t config_id);
  UncRespCode ClearImportConfig(uint32_t session_id, uint32_t config_id);
};
}/*namespace uppl*/
}/*namespace unc*/

#endif


