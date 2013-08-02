/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_TC_STATE_HANDLER_HH_
#define UNC_TC_STATE_HANDLER_HH_

#include <clstat_api.h>
#include <pfcxx/event.hh>
#include <pfcxx/synch.hh>
#include <string>
#include <map>

/*
 * UncState enum values
 */
typedef enum {
  INIT = 1,
  ACT,
  SBY,
  STOP,
  FAILURE
}UncState;

namespace unc {
class UncStateHandler {
 public:
  /*
   * @brief constructor method
   */
  UncStateHandler();
  virtual ~UncStateHandler() {}

  /*
   * Methods to be defined by inheriting class
   * to handle state notifications
   */
  virtual pfc_bool_t HandleStart()=0;
  virtual pfc_bool_t HandleAct(pfc_bool_t is_switch)=0;
  virtual pfc_bool_t HandleStop()=0;

  /*  Register Method for state notifications */
  pfc_bool_t RegisterStateHandlers();
  /*  UnRegister Method for state notifications */
  pfc_bool_t UnRegisterStateHandlers();

  /*
   * Event Handler methods for state transitions
   */
  void unc_start_event_handler_fn(pfc::core::Event* Event);
  void unc_stop_event_handler_fn(pfc::core::Event* Event);
  void unc_act_event_handler_fn(pfc::core::Event* Event);
  /*
   * Method to get state
   */
  UncState unc_get_state();

 private:
  pfc::core::Mutex lock_;
  UncState state_;
  pfc_evhandler_t  unc_start_ev_id_;
  pfc_evhandler_t  unc_stop_ev_id_;
  pfc_evhandler_t  unc_act_ev_id_;
  pfc_evhandler_t  unc_sby_ev_id_;
  pfc::core::event_handler_t unc_start_ev_handler_;
  pfc::core::event_handler_t unc_stop_ev_handler_;
  pfc::core::event_handler_t unc_act_ev_handler_;
};
}  // namespace unc
#endif
