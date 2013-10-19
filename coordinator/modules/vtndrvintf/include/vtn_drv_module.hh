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
#define PFC_IPCINT_EVSESS_OK 0

namespace unc {
namespace driver {

class VtnDrvIntf :public pfc::core::Module {
 public:
  /**
   * @brief     : Constructor
   * @param[in] :  pfc_modattr_t*
   **/
  explicit VtnDrvIntf(const pfc_modattr_t* attr);

  /**
   * @brief : TaskQueue*
   */
  pfc::core::TaskQueue* taskq_;

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
   * @brief     : This Function is called to register the driver handler
   *              with respect to the controller type
   * @param[in] : driver *
   * @return    : VTN_DRV_RET_FAILURE /VTN_DRV_RET_SUCCESS
   **/
  VtnDrvRetEnum register_driver(driver *drvobj);

  /**
   * @brief     : This Function recevies the ipc request and process the same
   * @param[in] : sess, service
   * @retval    : PFC_IPCRESP_FATAL/PFC_IPCINT_EVSESS_OK
   */
  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession& sess,
                            pfc_ipcid_t service);

  /**
   * @brief     : This function parse the session and fills
   *              keyif_drv_request_header_t
   * @param[in] : sess, request_hdr
   * @retval    : VTN_DRV_RET_FAILURE /VTN_DRV_RET_SUCCESS
   */
  VtnDrvRetEnum get_request_header(pfc::core::ipc::ServerSession*sess,
                            keyif_drv_request_header_t &request_hdr);

  /**
   * @brief     : This Function  returns the  kt_handler for
   *              the appropriate key types
   * @param[in] : key type
   * @retval    : KtHandler*
   */
  KtHandler*  get_kt_handler(unc_key_type_t kt);

  /**
   * @brief     : This Function  intialize ctrl_inst
   * @param[in] : ctrl_inst
   * @retval    : None
   */
  void  set_controller_instance(ControllerFramework* ctrl_inst) {
    ctrl_inst_ =  ctrl_inst;
  }

 private:
  std::map <unc_key_type_t, unc::driver::KtHandler*> map_kt_;
  pfc::core::ReadWriteLock kt_installer_wrlock_;
  pfc::core::ReadWriteLock drvcmds_installer_wrlock_;
  ControllerFramework* ctrl_inst_;
};
}  // namespace driver
}  // namespace unc
#endif
