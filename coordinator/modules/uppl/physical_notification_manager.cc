/*
 * Copyright (c) 2012-2014 NEC Corporation
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
NotificationManager* NotificationManager::polc_notification_manager_ = NULL;
NotificationManager* NotificationManager::odc_notification_manager_ = NULL;
PhyEventTaskqUtil* NotificationManager::taskq_util_ = NULL;

/**
 * @Description : This function is used for handling different 
 *                notification events
 * @param[in]   : event - object of IPC event 
 * @return      : void
 * */
void NotificationManager::eventHandler(const IpcEvent &event) {
  // Get Physical layer instance
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  pfc_log_info(
      "Event:[Srl:%d, Type:%d, ChName:%s, "
      "SvcName:%s, isStChEvt:%d]",
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
  } else if (ctr_type == UNC_CT_POLC) {
    if (polc_notification_manager_ == NULL) {
      polc_notification_manager_ = new NotificationManager();
    }
    physical_layer->notification_manager_mutex_.unlock();
    return polc_notification_manager_;
  } else if (ctr_type == UNC_CT_ODC) {
    if (odc_notification_manager_ == NULL) {
      odc_notification_manager_ = new NotificationManager();
    }
    physical_layer->notification_manager_mutex_.unlock();
    return odc_notification_manager_;
  } else {
    physical_layer->notification_manager_mutex_.unlock();
  }
  return NULL;
}

/**
 * @Description : Returns PhyEventTaskqUtil instance pointer. This
 *                function make sure only one instance is created.
 * @param[in]   : None
 * @return      : PhyEventTaskqUtil instance pointer
 * */
PhyEventTaskqUtil* NotificationManager::get_taskq_util() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  physical_layer->notification_manager_mutex_.lock();
  if (taskq_util_ == NULL) {
    //  number of threads to be span by taskq
    uint32_t concurrency = 1;
    taskq_util_ = new PhyEventTaskqUtil(concurrency);
  }
  physical_layer->notification_manager_mutex_.unlock();
  return taskq_util_;
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
  polc_notification_manager_ = NULL;
  odc_notification_manager_ = NULL;
  pfc_log_debug("Released notification manager instances");
}

void NotificationManager::delete_taskq_util() {
  pfc_log_debug("delete_taskq_util called");
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  physical_layer->notification_manager_mutex_.lock();
  if (taskq_util_ != NULL) {
    delete taskq_util_;
    taskq_util_ = NULL;
  }
  physical_layer->notification_manager_mutex_.unlock();
}
