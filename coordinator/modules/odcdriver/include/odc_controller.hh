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

namespace unc {
namespace odcdriver {

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

  /*
   *  Gets the contoller id
   *  @retval - std::strin - controller id
   */
  std::string get_controller_id();

 /*
  * Gets the audit status
  * @retval - pfc_bool_t - returns audit status 
  */
  pfc_bool_t get_audit_status();

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

  /*
   *    * gets the user name
   *       * @param[out] - string - user
   *          */
  std::string get_user_name();

  /*
   *    * Gets the pass word
   *       * @param[out] - std::string - pass word
   *          */
  std::string get_pass_word();


 private:
  std::string ip_addr_;
  std::string controller_name_;
  std::string version_;
  std::string description_;
  std::string user_name_;
  std::string pass_word_;
  pfc_bool_t audit_;
};
}  // namespace odcdriver
}  // namespace unc

#endif
