/*
 * Copyright (c) 2013 NEC Corporation
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
#include <vtn_drv_transaction_handle.hh>
#include <driver/driver_interface.hh>
#include <request_template.hh>
#include <map>

namespace unc {
namespace driver {

class VtnDrvIntf :public pfc::core::Module {
 public:
  /**
   * @brief     : Constructor
   * @param[in] : pfc_modattr_t*
   **/
  explicit VtnDrvIntf(const pfc_modattr_t* attr);


  /**
   * @brief : Destructor
   **/
  ~VtnDrvIntf();

  /**
   * @brief  : This Function is called to load the
   *           vtndrvintf module
   * @return : PFC_TRUE/PFC_FALSE
   **/
  pfc_bool_t init(void);

  /**
   * @brief  : This Function is called to unload the
   *           vtndrvintf module
   * @return : PFC_TRUE/PFC_FALSE
   **/
  pfc_bool_t fini(void);

  /**
   * @brief     : Used register the driver handler with respect to the
   *              controller type
   * @param[in] : driver pointer
   * @return    : VTN_DRV_RET_FAILURE /VTN_DRV_RET_SUCCESS
   **/
  VtnDrvRetEnum register_driver(driver *drv_obj);

  /**
   * @brief     : This Function recevies the ipc request and process the same
   * @param[in] : sess, service
   * @retval    : PFC_IPCRESP_FATAL/PFC_IPCINT_EVSESS_OK
   */
  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession& sess,
                            pfc_ipcid_t service);

  /**
   * @brief     : This function parse the session and fills
   *              odl_drv_request_header_t
   * @param[in] : sess, request_hdr
   * @retval    : VTN_DRV_RET_FAILURE /VTN_DRV_RET_SUCCESS
   */
  VtnDrvRetEnum get_request_header(pfc::core::ipc::ServerSession*sess,
                            odl_drv_request_header_t &request_hdr);

  /**
   * @brief     : This Function  returns the key type handler pointer for
   *              the appropriate key types
   * @param[in] : key type
   * @retval    : KtHandler pointer
   */
  KtHandler*  get_kt_handler(unc_key_type_t kt);

  /**
   * @brief     : This Function  intialize controller instance
   * @param[in] : ControllerFramework pointer
   * @retval    : None
   */
  void  set_controller_instance(ControllerFramework* ctrl_inst) {
    ctrl_inst_ =  ctrl_inst;
  }

  // used for Controller ping
  pfc::core::TaskQueue* taskq_;

 private:
  // To store key type handler pointer for supported keytypes
  std::map <unc_key_type_t, unc::driver::KtHandler*> map_kt_;

  // To store ControllerFramework instance
  ControllerFramework* ctrl_inst_;
};
}  // namespace driver
}  // namespace unc
#endif
