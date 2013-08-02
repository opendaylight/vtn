/*
 * Copyright (c) 2012-2013 NEC Corporation
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

UncStateHandler::UncStateHandler()
: lock_(),
  system_state_(UPPL_SYSTEM_ST_STANDBY),
  unc_act_ev_id_(EVHANDLER_ID_INVALID),
  unc_act_ev_handler_(std::bind1st(
      std::mem_fun(&UncStateHandler::unc_act_event_handler_fn), this)) {}

void UncStateHandler::unc_act_event_handler_fn(pfc::core::Event* Event) {
  lock_.lock();
  pfc_log_info("State change to Active received from Daemon");
  if (system_state_ == UPPL_SYSTEM_ST_STANDBY) {
    set_system_state(UPPL_SYSTEM_ST_ACTIVE);
    // ReConnect the DB
    ODBCM_RC_STATUS odbc_ret = ODBCManager::get_ODBCManager()->ReconnectDB();
    if (odbc_ret != ODBCM_RC_SUCCESS) {
      pfc_log_error("Unable to reconnect to DB");
      return;
    }
    // Send event subscription to driver
    UpplReturnCode ret = PhysicalCore::get_physical_core()->
        SendEventSubscripToDriver();
    pfc_log_debug("Event subscription return %d", ret);
  } else {
    pfc_log_info("System is already in Active");
  }
  lock_.unlock();
  return;
}

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
