/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VTNDRVMOD_HH__
#define __VTNDRVMOD_HH__

#include <unc/keytype.h>
#include <pfcxx/ipc_server.hh>
#include <pfc/log.h>
#include <pfcxx/synch.hh>
#include <kt_handler.hh>
#include <controller_fw.hh>
#include <driver/driver_command.hh>
#include <driver/driver_interface.hh>
#include <vtndrvintf_defs.h>
#include <map>

namespace unc {
namespace driver {

class VtnDrvIntf :public pfc::core::Module {
 public:
  explicit VtnDrvIntf(const pfc_modattr_t* attr);

  ~VtnDrvIntf() {
  }

  pfc_bool_t init(void) {
    return PFC_TRUE;
  }

  pfc_bool_t fini() {
    return PFC_TRUE;
  }
  static VtnDrvIntf theInstance;

  static void stub_loadVtnDrvModule(void);
  static void stub_unloadVtnDrvModule(void);

  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession& sess,
                           pfc_ipcid_t service) {
    //  return PFC_IPCINT_EVSESS_OK;

    return pfc_ipcresp_t();
  }

  VtnDrvRetEnum get_request_header(pfc::core::ipc::ServerSession*sess,
                                   odl_drv_request_header_t &request_hdr) {
    return VTN_DRV_RET_SUCCESS;
  }

  KtHandler*  get_kt_handler(unc_key_type_t kt) {
    return NULL;
  }

  VtnDrvRetEnum register_driver(driver *drv_obj) {
    return VTN_DRV_RET_SUCCESS;
  }
  void logicalport_event(oper_type operation, key_logical_port_t key_struct,
                         val_logical_port_st val_struct) {
  }

  void logicalport_event(oper_type operation, key_logical_port_t key_struct,
                         val_logical_port_st new_val_struct, val_logical_port_st
                         old_val_struct) {
  }

  void port_event(oper_type operation, key_port_t
                  key_struct, val_port_st val_struct) {
  }

  void port_event(oper_type operation, key_port_t
                  key_struct, val_port_st new_val_struct,
                  val_port_st old_val_struct) {
  }

  void switch_event(oper_type operation, key_switch
                    key_struct, val_switch_st val_struct) {
  }

  void switch_event(oper_type operation, key_switch
                    key_struct, val_switch_st new_val_struct,
                    val_switch_st old_val_struct) {
  }
};
}  // namespace driver
}  // namespace unc
#endif

