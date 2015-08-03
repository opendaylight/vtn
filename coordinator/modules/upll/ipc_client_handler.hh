/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_IPC_CLIENT_HANDLER_HH_
#define UPLL_IPC_CLIENT_HANDLER_HH_

#include "ipct_st.hh"
#include "unc/upll_svc.h"

#include "unc/pfcdriver_include.h"
#include "unc/vnpdriver_include.h"
#include "momgr_impl.hh"

using unc::upll::ipc_util::IpcResponse;

namespace unc {
namespace upll {
namespace ipc_util {

class IpcClientHandler {
  public:
    uint32_t arg;
    IpcResponse ipc_resp;
    pfc::core::ipc::ClientSession *cl_sess;

    IpcClientHandler() {
      cl_sess = NULL;
      memset(&ipc_resp, 0, sizeof(IpcResponse));
      arg = 0;
      connid = 0;
    }
    ~IpcClientHandler() {
       if (cl_sess) {
         int err = pfc_ipcclnt_altclose(connid);
         if (err != 0) {
           pfc_log_info("Unable to close the connection");
         }
         cl_sess->cancel(PFC_TRUE);
         delete cl_sess;
       }
       DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    }
    bool SendReqToDriver(const char *ctrlr_name, char *domain_id,
                                IpcRequest *req);
    // ctrlr_name and domain_id are valid only when sending to driver. When
    // sending to physical they are ignored.
    bool SendReqToServer(const char *channel_name,
                         const char *service_name, pfc_ipcid_t service_id,
                         bool driver_msg,
                         const char *ctrlr_name, char *domain_id,
                         IpcRequest *req);

  private:
    pfc_ipcconn_t connid;
    bool ReadKtResponse(pfc::core::ipc::ClientSession *sess,
                        pfc_ipcid_t service,
                        bool driver_msg, char *domain_id);
};

}  // namespace ipc_util
}  // namespace upll
}  // namespace unc

#endif  // UPLL_IPC_CLIENT_HANDLER_HH_
