/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *@brief   IPCConnectionManager
 *@file    ipc_connection_manager.cc
 *Desc:This header file contains the definition of IPCConnectionManager class
 *     which is the submodule class of PhysicalLayer class
 */

#include "physicallayer.hh"
#include "ipc_connection_manager.hh"

using unc::uppl::IPCConnectionManager;
using unc::uppl::IPCServerHandler;
using unc::uppl::NotificationManager;
using unc::uppl::IPCClientLogicalHandler;
using unc::uppl::PhysicalLayer;
using pfc::core::ipc::IpcEventAttr;
using pfc::core::ipc::IpcEventMask;

/**
 *@Description : Called on creation of object
 *@param[in]   : None
 *@return      : None
 **/
IPCConnectionManager::IPCConnectionManager() {
  ipc_server_handler_ = NULL;
  ipc_client_logical_handler_= NULL;
  notfn_timer_ = NULL;
  taskq_ = NULL;
}

/**
 *@Description : Frees up allocated memory
 *@param[in]   : None
 *@return      : None
 **/
IPCConnectionManager::~IPCConnectionManager() {
  if (notfn_timer_ != NULL) {
    delete notfn_timer_;
    notfn_timer_ = NULL;
  }
  if (taskq_ != NULL) {
    delete taskq_;
    taskq_ = NULL;
  }
}


/**
 *@Description : Initializes the IPC client and server class objects
 *@param[in]   : None
 *@return      : None
 **/
void IPCConnectionManager::InitializeIpcServerClient() {
  ipc_server_handler_ = IPCServerHandler::get_ipc_server_handler();
  ipc_client_logical_handler_= IPCClientLogicalHandler::
      get_ipc_client_logical_handler();
}


/**
 *@Description : Initializes the data members of IPCConnectionManager
 *@param[in]   : channel name ,service name
 *@return      : None
 **/
void IPCConnectionManager::InitIpcConnectionManager(string channel_name,
                                                    string service_name) {
  uppl_ipc_channel_name_ = channel_name;
  uppl_ipc_service_name_ = service_name;
}



/**
 *@Description : Returns the object of IPCServerHandler class
 *@param[in]   : None
 *@return      : Pointer to IPCServerHandler class
 **/
IPCServerHandler * IPCConnectionManager::get_ipc_server_handler() {
  return ipc_server_handler_;
}

/**
 *@Description : Returns the object of NotificationManager class
 *@param[in]   : None
 *@return      : Pointer to NotificationManager class
 **/
NotificationManager* IPCConnectionManager::get_notification_manager(
    unc_keytype_ctrtype_t ctr_type) {
  if (ctr_type == UNC_CT_PFC) {
    return NotificationManager::get_notification_manager(
        UNC_CT_PFC);
  }
  if (ctr_type == UNC_CT_VNP) {
    return NotificationManager::get_notification_manager(
          UNC_CT_VNP);
  }
  return NULL;
}



/**
 *@Description : Returns the object of IPCClientLogicalHandler class
 *@param[in]   : None
 *@return      : Pointer to IPCClientLogicalHandler class
 **/
IPCClientLogicalHandler* IPCConnectionManager::
get_ipc_client_logical_handler() {
  return ipc_client_logical_handler_;
}



/**
 *@Description : Returns the uppl_ipc_channel_name_ of string type
 *@param[in]   : None
 *@return      : string
 **/
string IPCConnectionManager::get_uppl_channel_name() {
  return uppl_ipc_channel_name_;
}



/**
 *@Description : Returns the uppl_ipc_service_name_ of string type
 *@param[in]   : None
 *@return      : string
 **/
string IPCConnectionManager::get_uppl_service_name() {
  return uppl_ipc_service_name_;
}

/**
 *@Description : Posts the event to the client
 *@param[in]   : Server event
 *@return      : UPPL_RC_SUCCESS
 **/
uint32_t IPCConnectionManager::SendEvent(ServerEvent *evt) {
  uint32_t ret = ipc_server_handler_->SendEvent(evt);
  return ret;
}


/**
 *@Description : Frees up the allocated memory
 *@param[in]   : None
 *@return      : UPPL_RC_SUCCESS
 **/
UpplReturnCode IPCConnectionManager::Finalize() {
  if (ipc_server_handler_ != NULL)
    IPCServerHandler::release_ipc_server_handler();
  if (ipc_client_logical_handler_ != NULL)
    IPCClientLogicalHandler::release_ipc_client_logical_handler();
  ipc_server_handler_ = NULL;
  ipc_client_logical_handler_ = NULL;
  return UPPL_RC_SUCCESS;
}

/**
 * * @Description: This function is used for notification manager subscription
 * to IPC event handler
 * * * @param[in]: no argument
 * * * @return   : pfc_ipcresp_t
 * */
UpplReturnCode IPCConnectionManager::SendEventSubscription() {
  // Get Physical layer instance
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  IpcEventAttr attr_pfc, attr_vnp;
  IpcEventMask mask(0);
  // Add events to mask
  if ((mask.add(UNC_PHYSICAL_EVENTS)) != 0) {
    pfc_log_error("add() failed for UNC_PHYSICAL_EVENTS\n");
    return UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  if ((mask.add(UNC_ALARMS)) != 0) {
    pfc_log_error("add() failed for UNC_ALARMS\n");
    return UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  if ((mask.add(UNC_CTLR_STATE_EVENTS)) != 0) {
    pfc_log_error("add() failed for UNC_CTLR_STATE_EVENTS\n");
    return UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  attr_pfc.setPriority(150);
  attr_vnp.setPriority(150);
  if ((attr_pfc.addTarget(PFCDRIVER_IPC_SVC_NAME, mask)) != 0) {
    pfc_log_error("addTarget() failed for PFC driver\n");
    return UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  // Notification manager event is registered
  physical_layer->Module::addIpcEventHandler(
      PFCDRIVER_IPC_CHN_NAME,
      get_notification_manager(UNC_CT_PFC) ,
      &attr_pfc);
  pfc_log_debug("Event Subscribed for PFC driver");
  if ((attr_vnp.addTarget(VNPDRIVER_IPC_SVC_NAME, mask)) != 0) {
    pfc_log_error("addTarget() failed for OVERLAY driver\n");
    return UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  physical_layer->Module::addIpcEventHandler(
      VNPDRIVER_IPC_CHN_NAME,
      get_notification_manager(UNC_CT_VNP),
      &attr_vnp);
  pfc_log_debug("Event Subscribed for Overlay driver");
  return UPPL_RC_SUCCESS;
}

/**
 * * @Description: This function is used for notification manager cancel
 * subscription from IPC event handler
 * * * @param[in]: no argument
 * * * @return   : pfc_ipcresp_t
 * */
UpplReturnCode  IPCConnectionManager::CancelEventSubscription() {
  // Get Physical layer instance
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Remove the Event Handler
  if ((physical_layer->Module::removeIpcEventHandler(
      get_notification_manager(UNC_CT_PFC)))
      != 0) {
    pfc_log_error("removeIpcEventHandler() failed\n");
    return UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  // Remove the Event Handler
  if ((physical_layer->Module::removeIpcEventHandler(
      get_notification_manager(UNC_CT_VNP)))
      != 0) {
    pfc_log_error("removeIpcEventHandler() failed\n");
    return UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  NotificationManager::release_notification_manager();
  return UPPL_RC_SUCCESS;
}

/**
 * * @Description: This function is used for add controller to controller_in_audit
 * vector
 * * * @param[in]: controller_name
 * * * @return   : None
 * */
void IPCConnectionManager::addControllerToAuditList(string controller_name) {
  pfc_log_debug("Executing addControllerToAuditList");
  vector<string> :: iterator iter = find(controller_in_audit_.begin(),
                                         controller_in_audit_.end(),
                                         controller_name);
  if (iter != controller_in_audit_.end()) {
    pfc_log_warn("Controller is already available in vector");
  } else {
    controller_in_audit_.push_back(controller_name);
    pfc_log_debug("Adding %s to list", controller_name.c_str());
  }
}

/**
 * * @Description: This function is used for remove controller from controller_in_audit
 * vector
 * * * @param[in]: controller_name
 * * * @return   : None
 * */
UpplReturnCode IPCConnectionManager::removeControllerFromAuditList(
    string controller_name) {
  pfc_log_debug("Executing removeControllerFromAuditList");
  vector<string> :: iterator iter = find(controller_in_audit_.begin(),
                                         controller_in_audit_.end(),
                                         controller_name);
  if (iter == controller_in_audit_.end()) {
    pfc_log_debug("Controller is not available in vector");
    return UPPL_RC_ERR_AUDIT_FAILURE;
  } else {
    controller_in_audit_.erase(iter);
    pfc_log_debug("Removed %s from list", controller_name.c_str());
  }
  map<string, uint32_t> :: iterator timer_iter =
      notfn_timer_id_.find(controller_name);
  if (timer_iter != notfn_timer_id_.end()) {
    notfn_timer_id_.erase(timer_iter);
  } else {
    return UPPL_RC_ERR_AUDIT_FAILURE;
  }
  return UPPL_RC_SUCCESS;
}

/**
 * * @Description: This function is used for check whether controller name is
 * available in controller_in_audit vector
 * * * @param[in]: controller_name
 * * * @return   : PFC_TRUE or PFC_FALSE
 * */
pfc_bool_t IPCConnectionManager::IsControllerInAudit(string controller_name) {
  pfc_log_debug("Executing IsControllerInAudit");
  vector<string> :: iterator iter = find(controller_in_audit_.begin(),
                                         controller_in_audit_.end(),
                                         controller_name);
  if (iter != controller_in_audit_.end()) {
    pfc_log_debug("Controller is in audit list");
    return PFC_TRUE;
  }
  return PFC_FALSE;
}

/**
 * * @Description: This function is used for start the timer to receive
 * notification after audit end
 * * * @param[in]: controller_name
 * * * @return   : Success or Failure
 * */
uint32_t IPCConnectionManager::StartNotificationTimer(string controller_name) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  uint32_t time_out = physical_core->getAuditNotfnTimeOut();
  uint32_t no_tasks = 1;
  pfc_log_debug("Starting timer with timeout %d for audit", time_out);
  taskq_ = TaskQueue::create(no_tasks);
  AuditNotification func_obj;
  func_obj.controller_name_ = controller_name;
  timer_func_t timer_func = func_obj;
  pfc_timeout_t  time_out_id;
  pfc_timespec_t  timeout;
  timeout.tv_sec = time_out;
  timeout.tv_nsec = 0;
  notfn_timer_ = pfc::core::Timer::create(taskq_->getId());
  uint32_t ret = notfn_timer_->post(&timeout, timer_func, &time_out_id);
  if (ret != 0) {
    pfc_log_error("Failure occurred in starting notification timer");
  } else {
    pfc_log_debug("Setting time_out_id %d, taskq id %d controller_name %s",
                  time_out_id, taskq_->getId(), controller_name.c_str());
    setTimeOutId(controller_name, time_out_id);
  }
  return ret;
}
