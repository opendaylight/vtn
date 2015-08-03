/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "ipc_client_configuration_handler.hh"

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
IPCConnectionManager::IPCConnectionManager()
:
                      ipc_server_handler_(NULL),
                      ipc_client_logical_handler_(NULL) {
}

/**
 *@Description : Frees up allocated memory
 *@param[in]   : None
 *@return      : None
 **/
IPCConnectionManager::~IPCConnectionManager() {
  PHY_TIMER_LOCK();
  // Clear timer and taskq if present in case of timeout
  if (!timer_obj_.empty()) {
    map<string, Timer *> :: iterator timer_iter = timer_obj_.begin();
    for (; timer_iter != timer_obj_.end();) {
      Timer * timer = (*timer_iter).second;
      if (timer != NULL) {
        delete timer;
        timer = NULL;
      }
      timer_obj_.erase(timer_iter++);
    }
  } else {
    pfc_log_debug("timer_obj_ map is empty");
  }
  if (!queue_obj_.empty()) {
    map<string, TaskQueue *> :: iterator task_iter = queue_obj_.begin();
    for (; task_iter != queue_obj_.end(); ) {
      TaskQueue * task = (*task_iter).second;
      if (task != NULL) {
        delete task;
        task = NULL;
      }
      queue_obj_.erase(task_iter++);
    }
  } else {
    pfc_log_debug("queue_obj_ map is empty");
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
 * @Description : Initializes the data members of IPCConnectionManager
 * @param[in]   : channel name - it contains the ipc channel name provided
 *                               by UPPL 
 *                service name - it contains the ipc service name provided
 *                               by UPPL
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
 * @Description : Returns the object of NotificationManager class
 * @param[in]   : ctr_type - enum which indicates the controller type
 *                           i.e either UNKNOWN or PFC or VNP or POLC or ODC
 * @return      : Pointer to NotificationManager class
 **/
NotificationManager* IPCConnectionManager::get_notification_manager(
    unc_keytype_ctrtype_t ctr_type) {
  if ((ctr_type == UNC_CT_PFC)
      || (ctr_type == UNC_CT_VNP)
      || (ctr_type == UNC_CT_POLC)
      || (ctr_type == UNC_CT_ODC)) {
    return NotificationManager::get_notification_manager(
        ctr_type);
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
 * @Description : Posts the event to the client
 * @param[in]   : pointer to Serverevent
 * @return      : UNC_RC_SUCCESS
 **/
uint32_t IPCConnectionManager::SendEvent(ServerEvent *evt,
                                 std::string controller_name,
                                 pfc_ipcevtype_t event_type) {
  uint32_t ret = ipc_server_handler_->SendEvent(evt);
  //  IPC service name, and `type' is an IPC event type
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  if (ret != 0) {
    string event_detail = PhyUtil::getEventDetailsString(event_type);
    pfc_log_warn("posts event to client failed, so failure notice is issued");
    UncRespCode alarm_status = physical_layer->get_physical_core()->
          SendEventPostFailureNotice(controller_name, event_detail);
    if (alarm_status != UNC_RC_SUCCESS)
      pfc_log_error("posts event to client failed - notice issue is failed");
  }
  return ret;
}

/**
 * @Description : Frees up the allocated memory
 * @return      : UNC_RC_SUCCESS - if the allocated memory is freed
 *                successfully
 **/
UncRespCode IPCConnectionManager::Finalize() {
  if (ipc_server_handler_ != NULL)
    IPCServerHandler::release_ipc_server_handler();
  if (ipc_client_logical_handler_ != NULL)
    IPCClientLogicalHandler::release_ipc_client_logical_handler();
  ipc_server_handler_ = NULL;
  ipc_client_logical_handler_ = NULL;
  notfn_timer_id_.clear();
  controller_in_audit_.clear();
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function is used for notification manager subscription
 *                to IPC event handler
 * @return      : UNC_RC_SUCCESS  - if the IPC Event subscription notification
 *                is successful else UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED 
 *                is returned if event subscription notification failed
 **/
UncRespCode IPCConnectionManager::SendEventSubscription() {
  // Get Physical layer instance
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  IpcEventAttr attr_pfc, attr_vnp, attr_polc, attr_odc;
  IpcEventMask mask(0);
  // Add events to mask
  if ((mask.add(UNC_PHYSICAL_EVENTS)) != 0) {
    pfc_log_error("add() failed for UNC_PHYSICAL_EVENTS\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  if ((mask.add(UNC_UPPL_ALARMS)) != 0) {
    pfc_log_error("add() failed for UNC_UPPL_ALARMS\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  if ((mask.add(UNC_CTLR_STATE_EVENTS)) != 0) {
    pfc_log_error("add() failed for UNC_CTLR_STATE_EVENTS\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  attr_pfc.setPriority(150);
  attr_vnp.setPriority(150);
  attr_polc.setPriority(150);
  attr_odc.setPriority(150);

  if ((attr_pfc.addTarget(PFCDRIVER_IPC_SVC_NAME, mask)) != 0) {
    pfc_log_error("addTarget() failed for PFC driver\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  // Notification manager event is registered
  physical_layer->Module::addIpcEventHandler(
      PFCDRIVER_IPC_CHN_NAME,
      get_notification_manager(UNC_CT_PFC) ,
      &attr_pfc);
  pfc_log_debug("Event Subscribed for PFC driver");

  // VNP
  if ((attr_vnp.addTarget(VNPDRIVER_IPC_SVC_NAME, mask)) != 0) {
    pfc_log_error("addTarget() failed for OVERLAY driver\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  physical_layer->Module::addIpcEventHandler(
      VNPDRIVER_IPC_CHN_NAME,
      get_notification_manager(UNC_CT_VNP),
      &attr_vnp);
  pfc_log_debug("Event Subscribed for Overlay driver");

  // POLC
  if ((attr_polc.addTarget(POLCDRIVER_IPC_SVC_NAME, mask)) != 0) {
    pfc_log_error("addTarget() failed for POLC driver\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  physical_layer->Module::addIpcEventHandler(
      POLCDRIVER_IPC_CHN_NAME,
      get_notification_manager(UNC_CT_POLC),
      &attr_polc);
  pfc_log_debug("Event Subscribed for Polc driver");

  // ODC
  if ((attr_odc.addTarget(ODCDRIVER_IPC_SVC_NAME, mask)) != 0) {
    pfc_log_error("addTarget() failed for ODC driver\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  physical_layer->Module::addIpcEventHandler(
      ODCDRIVER_IPC_CHN_NAME,
      get_notification_manager(UNC_CT_ODC),
      &attr_odc);
  pfc_log_debug("Event Subscribed for ODC driver");
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function is used for notification manager cancel
 *                subscription from IPC event handler
 * @return      : UNC_RC_SUCCESS - if event subscription is cancelled
 *                successfully from IPC Event Handler 
 *                else UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED
 *                will be returned
 **/
UncRespCode  IPCConnectionManager::CancelEventSubscription() {
  // Get Physical layer instance
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Remove the Event Handler for PFC
  if ((physical_layer->Module::removeIpcEventHandler(
      get_notification_manager(UNC_CT_PFC)))
      != 0) {
    pfc_log_error("removeIpcEventHandler() pfc failed\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  // Remove the Event Handler for VNP
  if ((physical_layer->Module::removeIpcEventHandler(
      get_notification_manager(UNC_CT_VNP)))
      != 0) {
    pfc_log_error("removeIpcEventHandler() vnp failed\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  // Remove the Event Handler for POLC
  if ((physical_layer->Module::removeIpcEventHandler(
      get_notification_manager(UNC_CT_POLC))) != 0) {
    pfc_log_error("removeIpcEventHandler() polc failed\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  // Remove the Event Handler for ODC
  if ((physical_layer->Module::removeIpcEventHandler(
      get_notification_manager(UNC_CT_ODC))) != 0) {
    pfc_log_error("removeIpcEventHandler() ODC failed\n");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  NotificationManager::release_notification_manager();
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function is used for add controller to
 *                controller_in_audit vector
 * @param[in]   : controller_name of type string
 * @return      : void 
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
 * @Description : This function is used for remove controller from
 *                controller_in_audit vector
 * @param[in]   : controller_name of type string
 * @return      : UNC_RC_SUCCESS  - if controller is removed successfully
 *                from audit list else UNC_UPPL_RC_ERR_AUDIT_FAILURE will be
 *                returned if controller is not available in the vector
 **/
UncRespCode IPCConnectionManager::removeControllerFromAuditList(
    string controller_name) {
  pfc_log_debug("Executing removeControllerFromAuditList");
  vector<string> :: iterator iter = find(controller_in_audit_.begin(),
                                         controller_in_audit_.end(),
                                         controller_name);
  if (iter == controller_in_audit_.end()) {
    pfc_log_info("Controller is not available in vector");
    return UNC_UPPL_RC_ERR_AUDIT_FAILURE;
  } else {
    controller_in_audit_.erase(iter);
    pfc_log_debug("Removed %s from list", controller_name.c_str());
  }
  map<string, uint32_t> :: iterator timer_iter =
      notfn_timer_id_.find(controller_name);
  if (timer_iter != notfn_timer_id_.end()) {
    notfn_timer_id_.erase(timer_iter);
  } else {
    pfc_log_info("Not able to delete timer");
    return UNC_UPPL_RC_ERR_AUDIT_FAILURE;
  }
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function is used for check whether controller name is
 *                available in controller_in_audit vector
 * @param[in]   : controller_name of type string
 * @return      : PFC_TRUE - if controller is in the audit list else
 *                PFC_FALSE is returned
 **/
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
 * @Description : This function is used for start the timer to receive
 *                notification after audit end
 * @param[in]   : controller_name of type string
 * @return      : UNC_RC_SUCCESS if timer is started successfully for audit
 *                as well as for task queue else
 *                UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION if there is error while
 *                creating task queue or timer for task queue
 **/
uint32_t IPCConnectionManager::StartNotificationTimer(
    OdbcmConnectionHandler *db_conn,
    string controller_name) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  PHY_TIMER_LOCK();
  uint32_t time_out = physical_core->getAuditNotfnTimeOut();
  uint32_t no_tasks = 1;
  map<string, Timer *> :: iterator timer_iter =
      timer_obj_.find(controller_name);
  map<string, TaskQueue *> :: iterator task_iter
      = queue_obj_.find(controller_name);
  if ((timer_iter != timer_obj_.end()) || (task_iter != queue_obj_.end())) {
    pfc_log_warn("StartNotificationTimer::Timer object found "
                 "for controller %s", controller_name.c_str());
    UncRespCode cancel_ret = CancelTimer(controller_name);
    if (cancel_ret != UNC_RC_SUCCESS) {
      pfc_log_error("StartNotificationTimer::Failure in cancelling "
          "timer for controller %s", controller_name.c_str());
    }
  }
  std::pair<std::map<string, Timer*>::iterator, bool> tmap_rc;
  std::pair<std::map<string, TaskQueue*>::iterator, bool> tqmap_rc;

  pfc_log_debug("Starting timer with timeout %d for audit", time_out);
  TaskQueue *taskq = TaskQueue::create(no_tasks);
  if (taskq == NULL) {
    UPPL_LOG_FATAL("Error while creating task queue");
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  tqmap_rc = queue_obj_.insert(
          std::pair<std::string, TaskQueue*>(controller_name, taskq));
  if (tqmap_rc.second == false) {
    pfc_log_error("StartNotificationTimer - Error in "
                  "inserting TaskQueue object");
  }
  AuditNotification func_obj;
  func_obj.controller_name_ = controller_name;
  timer_func_t timer_func = func_obj;
  pfc_timeout_t  time_out_id;
  pfc_timespec_t  timeout;
  timeout.tv_sec = time_out;
  timeout.tv_nsec = 0;
  Timer *notfn_timer = pfc::core::Timer::create(taskq->getId());
  if (notfn_timer == NULL) {
    UPPL_LOG_FATAL("Error while creating timer for the task queue");
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  tmap_rc = timer_obj_.insert(
          std::pair<string, Timer*>(controller_name, notfn_timer));
  if (tmap_rc.second == false) {
    pfc_log_error("StartNotificationTimer - Error in inserting Timer object");
  }
  uint32_t ret = notfn_timer->post(&timeout, timer_func, &time_out_id);
  if (ret != 0) {
    pfc_log_error("StartNotificationTimer - Failure occurred in"
                  " starting timer");
  } else {
    pfc_log_debug("Setting time_out_id %d, taskq id %d controller_name %s",
                  time_out_id, taskq->getId(), controller_name.c_str());
    setTimeOutId(controller_name, time_out_id);
  }
  return ret;
}

/**
 * @Description : This function is used to get the driver presence during
 *                create/update operations 
 * @param[in]   : ctr_type of type uint32
 * @return      : UNC_RC_SUCCESS - if respective driver is present
 *                UNC_RC_ERR_DRIVER_NOT_PRESENT -if driver is not installed
 *                will be returned
 **/
UncRespCode IPCConnectionManager::GetDriverPresence(uint32_t ctr_type) {
  UncRespCode driver_response = UNC_RC_SUCCESS;
  if (ctr_type == UNC_CT_PFC || ctr_type == UNC_CT_VNP ||
      ctr_type == UNC_CT_POLC ||
      ctr_type == UNC_CT_ODC) {
    UncRespCode err_resp = UNC_RC_SUCCESS;
    pfc_ipcresp_t resp = 0;
    uint8_t err = 0;

    if (ctr_type == UNC_CT_PFC) {
      IPCClientDriverHandler pfc_drv_handler(UNC_CT_PFC, err_resp);
      if (err_resp == 0) {
        err = pfc_drv_handler.ResetAndGetSession()->invoke(resp);
        pfc_log_debug("GetDriverPresence invoke status = %d, resp = %d",
                                                            err, resp);
      }
    } else if (ctr_type == UNC_CT_VNP) {
      IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err_resp);
      if (err_resp == 0) {
        err = vnp_drv_handler.ResetAndGetSession()->invoke(resp);
      }
    } else if (ctr_type == UNC_CT_POLC) {
      IPCClientDriverHandler polc_drv_handler(UNC_CT_POLC, err_resp);
      if (err_resp == 0) {
        err = polc_drv_handler.ResetAndGetSession()->invoke(resp);
      }
    } else if (ctr_type == UNC_CT_ODC) {
      IPCClientDriverHandler odc_drv_handler(UNC_CT_ODC, err_resp);
      if (err_resp == 0) {
        err = odc_drv_handler.ResetAndGetSession()->invoke(resp);
      }
    }
    if (err != 0) {
      pfc_log_debug("DriverPresence status = %d", err);
      if (err == ECONNREFUSED) {
        pfc_log_debug("DriverPresence status = %d, driver not present", err);
        driver_response =  UNC_RC_ERR_DRIVER_NOT_PRESENT;
      } else {
        pfc_log_debug("DriverPresence status = %d, comm failure", err);
        driver_response = UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
      }
    } else {
      pfc_log_debug("DriverPresence status = %d, comm success", err);
      driver_response = UNC_RC_SUCCESS;
    }
  }
  return driver_response;
}

/**
 * @Description : This function is used to cancel the timer and 
 *                delete associated objects
 * @param[in]   : controller_name of type string
 * @return      : UNC_RC_SUCCESS - if timer object and task object 
 *                are deleted successfully else UNC_UPPL_RC_ERR_AUDIT_FAILURE
 *                will be returned
 **/
UncRespCode IPCConnectionManager::CancelTimer(string controller_name) {
  pfc_log_debug("Executing CancelTimer()");
  UncRespCode ret = UNC_RC_SUCCESS;
  uint32_t time_out_id = getTimeOutId(controller_name);
  map<string, Timer *> :: iterator timer_iter =
      timer_obj_.find(controller_name);
  if (timer_iter != timer_obj_.end()) {
    Timer * timer = (*timer_iter).second;
    if (timer != NULL) {
      timer->cancel(time_out_id);
      delete timer;
      timer = NULL;
      pfc_log_debug("Deleted Timer object successfully");
    }
    timer_obj_.erase(timer_iter);
  } else {
    pfc_log_debug("Timer object not found ");
  }
  map<string, TaskQueue *> :: iterator task_iter
  = queue_obj_.find(controller_name);
  if (task_iter != queue_obj_.end()) {
    TaskQueue * task = (*task_iter).second;
    if (task != NULL) {
      delete task;
      task = NULL;
      pfc_log_debug("Deleted Task object successfully");
    }
    queue_obj_.erase(task_iter);
  } else {
    pfc_log_debug("Task object not found ");
  }
  return ret;
}
