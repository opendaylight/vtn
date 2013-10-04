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

#ifndef _ODC_CONTROLLER_HH_
#define _ODC_CONTROLLER_HH_

#include <arpa/inet.h>
#include <driver/controller_interface.hh>
#include <string>

namespace odc {
namespace driver {

class ODCController: public unc::driver::controller {
 public:
  /*
   * @brief - Parametrised Constructor
   * @param[in] - key_ctr - key structure for controller
   * @param[in] - val_ctr - value structure for controller
   */
  ODCController(const key_ctr_t& key_ctr, const val_ctr_t& val_ctr);

  /*
   * Destructor
   */
  ~ODCController();

  /**
   * Gets the controller type
   * @retval - unc_keytype_ctrtype_t - enum for controller type
   */
  unc_keytype_ctrtype_t get_controller_type();

  /**
   * Ping needed for the ODC Conttoller or not
   * @reval - pfc_bool_t - PFC_TRUE./ PFC_FALSE
   */
  pfc_bool_t is_ping_needed();

  /*
   *  Gets the ping interval
   *  @retval - uint32_t - ping interval
   */
  uint32_t get_ping_interval();

  /*
   *  Gets the ping interval count
   *  @retval - uint32_t - ping interval count
   */
  uint32_t get_ping_fail_retry_count();

  /*
   *  Gets the contoller id
   *  @retval - std::strin - controller id
   */
  std::string get_controller_id();

  /*
   *  ping conttoller available or nor
   *  @retval - pfc_bool_t - PFC_TRUE/ PFC_FALSE
   */
  pfc_bool_t ping_controller();

  /*
   *  rest connect set or not
   *  @retval - pfc_bool_t - PFC_TRUE/ PFC_FALSE
   */
  pfc_bool_t reset_connect();

  /**
   * get the host address
   * @retval - std::string - host address
   */
  std::string get_host_address();

 private:
  std::string ip_addr_;
  std::string controller_name_;
  std::string version_;
  std::string description_;
  pfc_bool_t audit_;
};
}
}

#endif
