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
#include <keytree.hh>

namespace unc{
namespace driver{

class controller {
 public:
  controller () :keytree_ptr(NULL) {}
  virtual ~controller() {}
  // Invoked to know the type of controller
  virtual unc_keytype_ctrtype_t get_controller_type()=0;
  // Invoked to learn if CDF needs to ping the controller
  // to check if alive
  virtual pfc_bool_t is_ping_needed ()=0;
  // Ping Interval
  virtual uint32_t get_ping_interval()=0;
  // Ping Fail Retry Count
  virtual uint32_t get_ping_fail_retry_count()=0;
  // Get the controller ID
  virtual std::string get_controller_id ()=0;
  // PING function for the controller
  virtual pfc_bool_t  ping_controller ()=0;
  // Invoked on global commit failure
  virtual pfc_bool_t  reset_connect ()=0;
  // Invoked cache create element
  unc::vtndrvcache::KeyTree *keytree_ptr;
};

}  // driver
}  // unc

#endif
