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

#include <pfc/ipc_struct.h>
#include <arpa/inet.h>
#include <driver/controller_interface.hh>
#include <string>

namespace unc {
namespace odcdriver {

class ODCController: public unc::driver::controller {
 public:
  /**
   * @brief     - Parametrised Constructor
   * @param[in] - key_ctr (key structure for controller)
   * @param[in] - val_ctr (value structure for controller)
   */
  ODCController(const key_ctr_t& key_ctr, const val_ctr_t& val_ctr);

  /**
   * @brief - Destructor
   */
  ~ODCController();

  /**
   * @brief   - Gets the controller type
   * @return  - unc_keytype_ctrtype_t (controller type)
   */
  unc_keytype_ctrtype_t get_controller_type();

  /**
   *  @brief   - Gets the contoller id
   *  @return  - std::strinig(controller id)
   */
  std::string get_controller_id();

  /**
   * @brief   - Gets the audit status
   * @return  - pfc_bool_t(audit status)
   */
  pfc_bool_t get_audit_status();

  /**
   * @brief    - reset connect set or not
   * @return   - PFC_TRUE on reset connection, PFC_FALSE on failure
   */
  pfc_bool_t reset_connect();

  /**
   * @brief  - get the host address
   * @return - std::string(host address)
   */
  std::string get_host_address();

  /**
   * @brief  - gets the user name
   * @return - std::string (username)
   */
  std::string get_user_name();

  /**
   * @brief  - Gets the password
   * @return - std::string(password)
   */
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
