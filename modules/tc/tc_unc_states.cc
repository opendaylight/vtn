/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <unc_state_handler.hh>

namespace unc  {

/**
 *@brief UncStateHandler class constructor.
 **/

UncStateHandler::UncStateHandler()
    :lock_(),
    state_(INIT),
    unc_start_ev_id_(EVHANDLER_ID_INVALID),
    unc_stop_ev_id_(EVHANDLER_ID_INVALID),
    unc_act_ev_id_(EVHANDLER_ID_INVALID),
    unc_start_ev_handler_(std::bind1st(
             std::mem_fun(&UncStateHandler::unc_start_event_handler_fn), this)),
    unc_stop_ev_handler_(std::bind1st(
             std::mem_fun(&UncStateHandler::unc_stop_event_handler_fn), this)),
    unc_act_ev_handler_(std::bind1st(
             std::mem_fun(&UncStateHandler::unc_act_event_handler_fn), this)) {}


/**
 *@brief  Event Registration functio.
 *@param[in]  none.
 *@return     PFC_TRUE .
 *@return     PFC_FALSE.
 **/
pfc_bool_t UncStateHandler::RegisterStateHandlers()  {
  pfc::core::ScopedMutex m(lock_);
  pfc::core::EventMask mask(PFC_EVTYPE_SYS_START);
  int ret=
    pfc::core::Event::addHandler(unc_start_ev_id_,
                                 pfc_event_global_source(),
                                 unc_start_ev_handler_,
                                 mask,
                                 0);
  if (ret != 0) {
    return PFC_FALSE;
  }
  mask.empty();
  mask.add(PFC_EVTYPE_SYS_STOP);
  ret=
    pfc::core::Event::addHandler(unc_stop_ev_id_,
                                 pfc_event_global_source(),
                                 unc_stop_ev_handler_,
                                 mask,
                                 0);
  if (ret != 0) {
    return PFC_FALSE;
  }
  mask.empty();
  mask.add(CLSTAT_EVTYPE_ACT);
  ret=
    pfc::core::Event::addHandler(unc_act_ev_id_,
                                 clstat_event_getsource(),
                                 unc_act_ev_handler_,
                                 mask,
                                 0);
  if (ret != 0) {
    return PFC_FALSE;
  }

  return PFC_TRUE;
}

/**
 *@brief   Event Handler un registration.
 *@param[in]  none .
 *@return     PFC_TRUE .
 *@return     PFC_FALSE.
 **/
pfc_bool_t UncStateHandler::UnRegisterStateHandlers() {
  int ret;
  if ( EVHANDLER_ID_INVALID != unc_start_ev_id_ ) {
    ret = pfc::core::Event::removeHandler(unc_start_ev_id_);
    if (ret != 0)
      return PFC_FALSE;
  }
  if ( EVHANDLER_ID_INVALID != unc_stop_ev_id_ ) {
    ret = pfc::core::Event::removeHandler(unc_stop_ev_id_);
    if (ret != 0)
      return PFC_FALSE;
  }
  if ( EVHANDLER_ID_INVALID != unc_act_ev_id_ ) {
    ret = pfc::core::Event::removeHandler(unc_act_ev_id_);
    if (ret != 0)
      return PFC_FALSE;
  }

  return PFC_TRUE;
}


/**
 *@brief  Start event handling method.
 *@param[in]  Event start event notification.
 *@return     none .
 **/
void UncStateHandler::unc_start_event_handler_fn(pfc::core::Event* Event) {
  pfc_bool_t ret;
  pfc::core::ScopedMutex m(lock_);
  switch (Event->getType())  {
  case PFC_EVTYPE_SYS_START:
    if (state_ != INIT)  {
      pfc_log_fatal("Invalid state Transition");
    } else {
        ret = HandleStart();
        if ( ret == PFC_TRUE ) {
            state_ = SBY;
          } else {
            pfc_log_fatal("Failed State Transition");
            state_ = FAILURE;
          }
        }
    break;

  default:
    pfc_log_fatal("Unexpected Event");
    break;
  }
}

/**
 *@brief  Stop event handling method.
 *@param[in]  Event stop event notification.
 *@return     none .
 **/
void UncStateHandler::unc_stop_event_handler_fn(pfc::core::Event* Event) {
  pfc_bool_t ret;
  pfc::core::ScopedMutex m(lock_);
  switch (Event->getType()) {
  case PFC_EVTYPE_SYS_STOP:
    if (state_ == STOP) {
      pfc_log_fatal("Invalid state Transition");
    } else {
      ret = HandleStop();
      if (ret == PFC_TRUE) {
        state_ = STOP;
      } else {
        pfc_log_fatal("state Transition Failed");
        state_ = FAILURE;
      }
    }
    break;
  default:
    pfc_log_fatal("Unexpected Event");
    break;
  }
}

/**
 *@brief  Act event handling method.
 *@param[in]  Event Act event notification.
 *@return     none .
 **/
void UncStateHandler::unc_act_event_handler_fn(pfc::core::Event* Event) {
  pfc_bool_t ret;
  pfc::core::ScopedMutex m(lock_);
  switch (Event->getType()) {
  case CLSTAT_EVTYPE_ACT:
    if (state_ != SBY) {
          pfc_log_fatal("Invalid state Transition");
    } else {
      ret =  HandleAct(clstat_event_isactive(Event->getEvent()));
      if ( ret == PFC_TRUE ) {
        state_ = ACT;
      } else {
        state_ = FAILURE;
      }
    }
    break;
  default:
    pfc_log_fatal("Unexpected Event");
    break;
  }
}

/**
 *@brief  Get UNC State
 *@return     Current UncState value.
 **/

UncState UncStateHandler::unc_get_state() {
  pfc::core::ScopedMutex m(lock_);
  return state_;
}
}  //  namespace unc
