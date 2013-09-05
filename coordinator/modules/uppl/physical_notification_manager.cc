/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * @brief    Notification Manager
 * @file     physical_notification_manager.cc
 * @Desc     This source file contains the function definitions of Notification
 * Manager class
 *
 */
#include "physicallayer.hh"
#include "physical_notification_manager.hh"

using unc::uppl::NotificationManager;
using unc::uppl::PhysicalLayer;

NotificationManager* NotificationManager::pfc_notification_manager_ = NULL;
NotificationManager* NotificationManager::vnp_notification_manager_ = NULL;

/**
 * @Description : This function is used for handling different 
 *                notification events
 * @param[in]   : event - object of IPC event 
 * @return      : void
 * */
void NotificationManager::eventHandler(const IpcEvent &event) {
  // Get Physical layer instance
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  pfc_log_info("NotificationManager - An event is received");
  pfc_log_info(
      "Event:[Serial:%d , Type:%d , ChannelName:%s, "
      "ServiceName:%s, isStateChangeEvent:%d]",
      event.getSerial(), event.getType(),
      event.getChannelName(),
      event.getServiceName(),
      event.isStateChangeEvent());
  PHY_FINI_EVENT_LOCK();
  // Ignore notifications, if system state is standby
  if (physical_layer->get_physical_core()->get_system_state() == \
      UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_warn("Event is ignored due to standby system state\n");
    return;
  }
  // Call itc to process the events
  physical_layer->get_physical_core()->
      get_internal_transaction_coordinator()->ProcessEvent(event);
}

const char* NotificationManager::getName(void) {
  return UPPL_IPC_NOTIFICATION_HANDLER_NAME;
}

/**
 * @Description : Returns NotificationManager instance pointer for PFC and VNP
 *                controller type. For other types it returns NULL. This 
 *                function make sure only one instance is created for each 
 *                type(PFC/VNP) of controller.
 * @param[in]   : ctr_type - controller type
 * @return      : NotificationManager instance pointer for PFC and VNP
 *                controller type. For other types it returns NULL
 * */
NotificationManager* NotificationManager::get_notification_manager(
    unc_keytype_ctrtype_t ctr_type) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  physical_layer->notification_manager_mutex_.lock();
  if (ctr_type == UNC_CT_PFC) {
    if (pfc_notification_manager_ == NULL) {
      pfc_notification_manager_ = new NotificationManager();
    }
    physical_layer->notification_manager_mutex_.unlock();
    return pfc_notification_manager_;
  } else if (ctr_type == UNC_CT_VNP) {
    if (vnp_notification_manager_ == NULL) {
      vnp_notification_manager_ = new NotificationManager();
    }
    physical_layer->notification_manager_mutex_.unlock();
    return vnp_notification_manager_;
  } else {
    physical_layer->notification_manager_mutex_.unlock();
  }
  return NULL;
}

/**
 * @Description : This function is used for release notification
 *                manager instances
 * @param[in]   : None
 * @return      : void
 * */
void NotificationManager::release_notification_manager() {
  pfc_notification_manager_ = NULL;
  vnp_notification_manager_ = NULL;
  pfc_log_debug("Released notification manager instances");
}
