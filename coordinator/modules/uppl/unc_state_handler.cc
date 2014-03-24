/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    UNC State handler
 * @file     unc_state_handler.cc
 *
 */

#include <unc/config.h>
#include <unc/component.h>
#include <alarm.hh>
#include <clstat_api.h>
#include "physical_core.hh"
#include "physicallayer.hh"

namespace unc {
namespace uppl {

/* @Description : Constructor function used to initialize the system state
 *                to StandBy and other event handler ID's.
 * @param[in]   : None 
 * @return      : None 
 **/
UncStateHandler::UncStateHandler()
: lock_(),
  system_state_(UPPL_SYSTEM_ST_STANDBY),
  unc_act_ev_id_(EVHANDLER_ID_INVALID),
  unc_act_ev_handler_(std::bind1st(
      std::mem_fun(&UncStateHandler::unc_act_event_handler_fn), this)) {
}

/* @Description : This function is used for active state event handling from
 *                StandBy mode and updates the event subscription to driver.
 * @param[in]   : Event pointer to pfc core event 
 * @return      : None 
 **/
void UncStateHandler::unc_act_event_handler_fn(pfc::core::Event* Event) {
  lock_.lock();
  pfc_log_info("State change to Active received from Daemon");
  if (system_state_ == UPPL_SYSTEM_ST_STANDBY) {
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    PhysicalCore* physical_core = physical_layer->get_physical_core();
    physical_core->system_transit_state_ = true;

    // Close the existing rw connections if any after switch over
    ODBCM_RC_STATUS close_rw_handle =
        PhysicalLayer::get_instance()->get_odbc_manager()->CloseRwConnection();
    pfc_log_debug("Rw Connection Handle Free status %d", close_rw_handle);
    //  All existing Read only connections will be freed when transiting to ACT
    ODBCM_RC_STATUS close_ro_handle =
          PhysicalLayer::get_instance()->
                       get_odbc_manager()->FreeingConnections(false);
    pfc_log_debug("RO Connection Handle(s) Free status %d",
                                                     close_ro_handle);
    physical_core->system_transit_state_ = false;
    set_system_state(UPPL_SYSTEM_ST_ACTIVE);
    // Send event subscription to driver
    UncRespCode ret = PhysicalCore::get_physical_core()->
        SendEventSubscripToDriver();
    pfc_log_debug("Event subscription return %d", ret);
    // clear all alarms
    pfc::alarm::pfc_alarm_clear(UNCCID_PHYSICAL);
  } else {
    pfc_log_info("System is already in Active");
  }
  lock_.unlock();
  return;
}

/* @Description : This function is used for registering event handler for
 *                active and standby states
 * @param[in]   : None
 * @return      : PFC_TRUE  - if active event and standby event 
 *                registering response is 0 else PFC_FALSE will be returned 
 **/
pfc_bool_t UncStateHandler::RegisterStateHandlers()  {
  lock_.lock();
  // Register event handler for active
  pfc::core::EventMask mask(CLSTAT_EVTYPE_ACT);
  pfc_log_debug("clstat_event_getsource(): %s CLSTAT_EVTYPE_ACT %d ",
                clstat_event_getsource(), CLSTAT_EVTYPE_ACT);
  int ret=
      pfc::core::Event::addHandler(unc_act_ev_id_,
                                   clstat_event_getsource(),
                                   unc_act_ev_handler_,
                                   mask,
                                   0);
  pfc_log_info("Active Event registering response %d", ret);
  lock_.unlock();

  return (ret == 0) ? PFC_TRUE : PFC_FALSE;
}

/* @Description : This function is used for removing event handler status for
 *                active and standby event
 * @param[in]   : None
 * @return      : PFC_TRUE  - if active event and standby event
 *                handler status after removing is 0 else PFC_FALSE
 *                will be returned 
 **/
pfc_bool_t UncStateHandler::UnRegisterStateHandlers() {
  int ret = 0;
  if (EVHANDLER_ID_INVALID != unc_act_ev_id_) {
    ret = pfc::core::Event::removeHandler(unc_act_ev_id_);
    pfc_log_debug("Active Event Remove Handler Status %d", ret);
    if (ret != 0) {
      return PFC_FALSE;
    }
  }

  return PFC_TRUE;
}
}  // namespace uppl
}  // namespace unc
