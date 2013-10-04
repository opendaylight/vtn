/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the
 * terms of the Eclipse Public License v1.0 which
 * accompanies this
 * distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
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
#include <vtn_drv_transaction_handle.hh>
#include <driver/driver_interface.hh>
#include <vtndrvintf_defs.h>
#include <request_template.hh>
#include <map>

namespace unc {
namespace driver {

class VtnDrvIntf :public pfc::core::Module {
 public:
  
  /**
   *@brief: Constructor
   **/

  explicit VtnDrvIntf(const pfc_modattr_t* attr);

  /**
   *@brief: Destructor
   **/
  ~VtnDrvIntf();
  /**
   *@brief: This Function is called to load the
   *        vtndrvintf module
   **/
  pfc_bool_t init(void);

  /**
   *@brief: This Function is called to unload the
   *        vtndrvintf module
   **/
  pfc_bool_t fini(void);

  /**
   *@brief: This Function is called to register the driver handler
   *        with respect to the controller type
   **/
  VtnDrvRetEnum register_driver(driver *drvobj);

  /**
   *@brief: This Function recevies the ipc request and process the same
   **/

  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession& sess,
                            pfc_ipcid_t service);

  /**
   *@brief: This function parse the sess and fills
   *        keyif_drv_request_header_t
   **/
  VtnDrvRetEnum get_request_header(pfc::core::ipc::ServerSession*sess,
                            keyif_drv_request_header_t &request_hdr);

  /**
   * @brief: This Function  returns the  kt_handler for
   *         the appropriate key types
   **/
  KtHandler*  get_kt_handler(unc_key_type_t kt);

 private:
  std::map <unc_key_type_t, unc::driver::KtHandler*> map_kt_;
  pfc::core::ReadWriteLock kt_installer_wrlock_;
  pfc::core::ReadWriteLock drvcmds_installer_wrlock_;
  ControllerFramework* ctrl_inst_;
};
}  // driver
}  // unc
#endif
