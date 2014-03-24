/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef KT_HANDLER_HH_
#define KT_HANDLER_HH_

#include <handler.hh>
#include <controller_fw.hh>
#include <confignode.hh>
#include <vtn_conf_data_element_op.hh>
#include <pfcxx/ipc_server.hh>

namespace unc {
namespace driver {
class KtHandler {
 public:
  virtual UncRespCode handle_request(pfc::core::ipc::ServerSession &sess,
                                    odl_drv_request_header_t &request_header,
                                         ControllerFramework* crtl_fw)=0;

  virtual UncRespCode  execute_cmd(unc::vtndrvcache::ConfigNode *cfgptr,
                                       unc::driver::controller* ctl_ptr,
                                       unc::driver::driver* drv_ptr)=0;

  virtual void* get_key_struct(unc::vtndrvcache::ConfigNode *cfgptr)=0;


  virtual void* get_val_struct(unc::vtndrvcache::ConfigNode *cfgptr)=0;

  virtual ~KtHandler() {}
};
}  // namespace driver
}  // namespace unc
#endif
