/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 @brief   IPCClientConfigurationHandler header
 @file    ipc_client_configuration_handler.hh
 @ Desc:  This header file contains the declaration of
          IPCClientConfigurationHandler class
 *
 */

#ifndef _IPC_CLIENT_CONFIGURATION_HANDLER_H_
#define _IPC_CLIENT_CONFIGURATION_HANDLER_H_


#include <pfcxx/ipc_client.hh>
#include <pfcxx/module.hh>
#include <string>
#include "unc/uppl_common.h"
#include "unc/keytype.h"
#include "phy_util.hh"

using pfc::core::ipc::ClientSession;

namespace unc {
namespace uppl {


/**************************************************************************
 * It is class that includes the SendMessageToDriver function.
 * For further info, see the comments in .cc file
 ***************************************************************************/

class IPCClientDriverHandler {
  public:
    IPCClientDriverHandler(unc_keytype_ctrtype_t type, UncRespCode &err);
    ~IPCClientDriverHandler();
    IPCClientDriverHandler();
    IPCClientDriverHandler(const IPCClientDriverHandler&);
    IPCClientDriverHandler& operator=(const IPCClientDriverHandler&);
    ClientSession* ResetAndGetSession();
    UncRespCode SendReqAndGetResp(driver_response_header &rsp);
    UncRespCode ConvertDriverErrorCode(uint32_t drv_err_code);
  private:
    ClientSession *cli_session;
    pfc_ipcconn_t connp;
    unc_keytype_ctrtype_t controller_type;
    string driver_name;
    string chn_name;
};
}  // namespace uppl
}  // namespace unc

#endif
