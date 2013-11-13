/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the  terms of the Eclipse Public License v1.0 which
 * accompanies this  distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __CONTROLLER_INTERFACE_HH__
#define __CONTROLLER_INTERFACE_HH__

#include <unc/keytype.h>
#include <pfcxx/timer.hh>
#include <keytree.hh>
#include <string>

namespace unc {
namespace driver {

typedef enum {
  CONNECTION_UP = 0,
  CONNECTION_DOWN
}ConnectionStatus;


class controller {
 public:
  /**
   * @brief - Constructor of controller class
   */
  controller() :controller_cache(NULL),
                timed_(NULL),
                connection_status_(CONNECTION_DOWN) {}
  /**
   * @brief - Destructor of controller class
   */
  virtual ~controller() {}

  /**
   * @brief  - Method to get the type of controller
   * @retval - unc_keytype_ctrtype_t Controller type
   */
  virtual unc_keytype_ctrtype_t get_controller_type() = 0;

  /**
   * @brief  - Method to get the controller ID
   * @retval - string - Controller ID
   */
  virtual std::string get_controller_id() = 0;

  /**
   * @brief  - Method Invoked on global commit failure
   * @retval - PFC_TRUE/PFC_FALSE
   */
  virtual pfc_bool_t  reset_connect() = 0;

  virtual pfc_bool_t update_ctr(const key_ctr_t& key_ctr,
                                const val_ctr_t& val_ctr) = 0;


  /**
   * @brief  - Method to get the host address
   * @retval - string - hostaddress
   */
  virtual std::string get_host_address() = 0;

  /**
   * @brief  - Method to get the audit status
   * @retval - PFC_TRUE/PFC_FALSE
   */
  virtual pfc_bool_t get_audit_status() = 0;

  /**
   * @brief  - Method to get the  user name
   * @retval - string - username configured
   */
  virtual std::string  get_user_name() = 0;

  /**
   * @brief  - Method to get the password
   * @retval - string - password configured
   */
  virtual std::string get_pass_word() = 0;

  /**
   * @brief  - Method to return connection status of controller
   * @retval - CONNECTION_UP/CONNECTION_DOWN
   */
  ConnectionStatus get_connection_status() {
    return connection_status_;
  }
  /**
   * @brief     - Method to set connection status of controller
   * @param[in] - ConnectionStatus - CONNECTION_UP/CONNECTION_DOWN
   */
  void set_connection_status(ConnectionStatus conn_status) {
    connection_status_ = conn_status;
  }

  /**
   * @brief  - Keytree pointer to access cache manager
   */
  unc::vtndrvcache::KeyTree *controller_cache;

  /**
   * @brief  - Timer Instance
   */
  pfc::core::Timer* timed_;

 private:
  ConnectionStatus connection_status_;
};
}  // namespace driver
}  // namespace unc
#endif
