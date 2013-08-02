/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    UNC State Handler
 * @file     unc_state_handler.hh
 */

#ifndef _UNC_STATE_HANDLER_HH_
#define _UNC_STATE_HANDLER_HH_

#include <unc/keytype.h>
#include <uncxx/tclib/tclib_interface.hh>
#include <pfcxx/module.hh>
#include <pfcxx/synch.hh>
#include <string>
#include "physical_common_def.hh"

using pfc::core::Event;
using pfc::core::Mutex;
using pfc::core::event_handler_t;

namespace unc {
namespace uppl {

/*
 * UncStateHandler class holds the state of the system
 */

class UncStateHandler {
 public:
  UncStateHandler();

  pfc_bool_t RegisterStateHandlers();
  pfc_bool_t UnRegisterStateHandlers();
  void unc_act_event_handler_fn(Event* Event);
  /**
   * @Description : This inline function sets the system state
   * @param[in] : system_state - Current state of the system
   */

  inline void set_system_state(UpplSystemState system_state) {
    system_state_ = system_state;
  }

  /**
   * @Description : This inline function gets the system state
   * @param[in] :
   */

  inline UpplSystemState get_system_state() {
    return system_state_;
  }

 private:
  Mutex lock_;
  UpplSystemState system_state_;
/*
 * Act Event handler ID.
 */
  pfc_evhandler_t  unc_act_ev_id_;

  event_handler_t unc_act_ev_handler_;
};
}  // namespace uppl
}  // namespace unc

#endif
