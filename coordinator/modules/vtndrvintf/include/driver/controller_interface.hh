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
  controller() :keytree_ptr(NULL),
                timed_(NULL),
                connection_status_(CONNECTION_DOWN) {}
  virtual ~controller() {}
  // Invoked to know the type of controller
  virtual unc_keytype_ctrtype_t get_controller_type() = 0;
  // Get the controller ID
  virtual std::string get_controller_id() = 0;
  // Invoked on global commit failure
  virtual pfc_bool_t  reset_connect() = 0;
  // Gets the host address
  virtual std::string get_host_address() = 0;
  //  Gets the audit status
  virtual pfc_bool_t get_audit_status() = 0;
  // Gets the  user name
  virtual std::string  get_user_name() = 0;
  // Gets the pass word
  virtual std::string get_pass_word() = 0;

  // Invoked cache create element
  unc::vtndrvcache::KeyTree *keytree_ptr;

  //Timer Instance
  pfc::core::Timer* timed_;

  // Method to return connection status of controller
  ConnectionStatus get_connection_status() {
    return connection_status_;
  }
  // Method to set connection status of controller
  void set_connection_status(ConnectionStatus conn_status) {
    connection_status_ = conn_status;
  }

 private:
  ConnectionStatus connection_status_;
};
}  // namespace driver
}  // namespace unc
#endif
