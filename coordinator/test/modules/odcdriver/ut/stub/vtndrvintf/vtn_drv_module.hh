/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __VTNDRVMOD_STUB_HH__
#define __VTNDRVMOD_STUB_HH__
#include <driver/driver_command.hh>
#include <driver/driver_interface.hh>
#include <pfcxx/module.hh>

typedef enum {
  VTN_DRV_RET_SUCCESS = 0,
  VTN_DRV_RET_FAILURE
}VtnDrvRetEnum;

using namespace pfc::core;

namespace unc {
namespace driver {

class VtnDrvIntf :public pfc::core::Module {
 public:
  /**
   * @brief     : Constructor
   * @param[in] :  pfc_modattr_t*
   **/
  explicit VtnDrvIntf(const pfc_modattr_t* attr): Module(attr)  {}

  /**
   * @brief : Destructor
   **/
  ~VtnDrvIntf() {}

  /**
   * @brief  : This Function is called to load the
   *           vtndrvintf module
   * @return : PFC_TRUE/PFC_FALSE
   **/
  inline pfc_bool_t init(void) {
    return PFC_FALSE;
  }

  /**
   * @brief  : This Function is called to unload the
   *           vtndrvintf module
   * @return : PFC_TRUE/PFC_FALSE
   **/
  inline pfc_bool_t fini(void) {
    return PFC_FALSE;
  }

  /**
   * @brief     : This Function is called to register the driver handler
   *              with respect to the controller type
   * @param[in] : driver *
   * @return    : VTN_DRV_RET_FAILURE /VTN_DRV_RET_SUCCESS
   **/
  inline VtnDrvRetEnum register_driver(driver *drv_obj) {
    return VTN_DRV_RET_SUCCESS;
  }
};
}  // namespace driver
}  // namespace unc
#endif
