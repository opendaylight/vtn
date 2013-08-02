/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *@brief   IPCServerHandler header
 *@file    ipc_server_handler.hh
 *@ Desc:  This header file contains the declaration of IPCServerHandler class
 **
 **/

#ifndef _IPC_SERVER_H_
#define _IPC_SERVER_H_

#include <pfcxx/ipc_server.hh>
#include <pfcxx/ipc_client.hh>
#include <pfcxx/module.hh>
#include <pfc/hostaddr.h>
#include <pfc/log.h>
#include <pfc/debug.h>

using pfc::core::ipc::ServerEvent;
using pfc::core::ipc::ServerSession;

namespace unc {
namespace uppl {


/*****************************************************************
 * This singleton class contains the IPC API calls to interact with
 * north bound (UPLL and VTN Service).
 *******************************************************************/

class IPCServerHandler {
 public:
  pfc_ipcresp_t IpcService(ServerSession &session,
     pfc_ipcid_t service_id);
  uint32_t SendEvent(ServerEvent *serv_event);
  static IPCServerHandler* get_ipc_server_handler();
  static void release_ipc_server_handler();
 private:
  IPCServerHandler() {}
  ~IPCServerHandler() {}
  static IPCServerHandler *ipc_server_handler_;
};
}  // namespace uppl
}  // namespace unc

#endif
