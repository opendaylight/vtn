/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


/*
 *@brief   Physical Internal Transaction Coordinator header
 *@file    physical_itc_req.hh
 *
 *Desc:This header file contains the declaration of
 *InternalTransactionCoordinator class
 */


#ifndef _PFC_PHYSICALITCREQ_H_
#define _PFC_PHYSICALITCREQ_H_

#include <pfcxx/ipc_server.hh>
#include "physical_common_def.hh"
#include "ipc_connection_manager.hh"

using pfc::core::ipc::ServerSession;

namespace unc {
namespace uppl {
/* *
* * It is a base class for all the request classes.
* */

class ITCReq {
  public:
    ITCReq() {}
    virtual ~ITCReq() {}
    virtual UncRespCode ProcessReq(ServerSession &session) {
      return UNC_RC_SUCCESS;
    }
    virtual uint16_t ProcessEvent(uint16_t service_id, ServerSession &session) {
      return 0;
    }
    virtual pfc_bool_t ProcessEvent(const IpcEvent &event) {
      return 0;
    }
};
}  // namespace uppl
}  // namespace unc
#endif
