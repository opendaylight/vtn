/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 @brief   IPCClientLogicalHandler header
 @file    ipc_client_logical_handler.hh
 @ Desc:  This header file contains the declaration of
          IPCClientLogicalHandler class
 *
*/

#ifndef _IPC_CLIENT_LOGICAL_HANDLER_H_
#define _IPC_CLIENT_LOGICAL_HANDLER_H_

#include <unc/keytype.h>
#include <pfcxx/module.hh>
#include <pfcxx/ipc_client.hh>
#include "unc/uppl_common.h"
#include "unc/unc_base.h"

namespace unc {
namespace uppl {


/**************************************************************************
 * It is class that includes the SendMessageToLogicalToFindInUse function.
 * For further info,see the comments in .cc file
 * ***************************************************************************/

class IPCClientLogicalHandler {
 public:
  UncRespCode CheckInUseInLogical(unc_key_type_t k_type,
                                     void *key_str,
                                     uint32_t data_type);
  static IPCClientLogicalHandler* get_ipc_client_logical_handler();
  static void release_ipc_client_logical_handler();

 private:
  static IPCClientLogicalHandler* ipc_client_logical_handler_;
  IPCClientLogicalHandler() {}
  ~IPCClientLogicalHandler() {}
};
}  // namespace uppl
}  // namespace unc

#endif  /* !_PFC_MODULE_IPC_CLIENT_LOGICAL_HANDLER_H_ */

