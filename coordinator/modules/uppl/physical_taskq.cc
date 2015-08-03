/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <pfcxx/task_queue.hh>
#include "physical_taskq.hh"
#include "physicallayer.hh"
#include "physical_common_def.hh"
#include "itc_notification_request.hh"
#include "itc_kt_base.hh"
#include "itc_kt_state_base.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_port.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_logical_member_port.hh"

using pfc::core::TaskQueue;
using unc::uppl::PhyEventTaskqUtil;
using unc::uppl::NotificationEventParams;
using unc::uppl::NotificationRequest;
using unc::uppl::EventAlarmDetail;

ReadWriteLock PhyEventTaskqUtil::taskqmap_mutex_;
/**
 *@brief    PhyEventTaskqUtil constructor,create a new task queue.
 *@param[in]  concurrency  Number of simultanious tasks.
 */
PhyEventTaskqUtil::PhyEventTaskqUtil(uint32_t concurrency)
                                :concurrency_(concurrency) {
  pfc_log_debug("Taskqueues concurrency is %d", concurrency_);
}

/**
 *@brief    PhyEventTaskqUtil destructor,delete all task queue.
 *@param[in] none 
 */
PhyEventTaskqUtil::~PhyEventTaskqUtil() {
  std::map<string, pfc::core::TaskQueue*>::iterator tq_map_iter;
  bool to_be_continued = false;
  do {
  to_be_continued = false;
  tq_map_iter = taskq_map_.begin();
  for (; tq_map_iter != taskq_map_.end(); tq_map_iter++) {
    string ctr_name = (tq_map_iter)->first;
    del_task_queue(ctr_name);
    to_be_continued = true;
  }
  } while (to_be_continued);
  taskq_map_.clear();
  pfc_log_debug("Taskqueues are cleared ..");
}

/**
 *@brief    delete a particular controller's task queue
 *@param[in]  string controller_name.
 */
void PhyEventTaskqUtil::del_task_queue(string ctr_name) {
  std::map<string, pfc::core::TaskQueue*>::iterator tq_map_iter;
  pfc::core::TaskQueue *phy_event_taskq_ = NULL;
  pfc_log_debug("taskq delete for controller %s", ctr_name.c_str());
  ScopedReadWriteLock taskqmaplock(
        PhyEventTaskqUtil::get_taskqmap_mutex_(), PFC_TRUE);  // write lock
  tq_map_iter = taskq_map_.find(ctr_name);
  if (tq_map_iter != taskq_map_.end()) {
    pfc_log_debug("taskq found to delete - controller %s", ctr_name.c_str());
    phy_event_taskq_ = (tq_map_iter)->second;
    int ret = phy_event_taskq_->clear(NULL);
    if (ret != 0)
      pfc_log_info("Error in clearing taskq ctr_name = %s", ctr_name.c_str());
    delete phy_event_taskq_;
    taskq_map_.erase(tq_map_iter);
    phy_event_taskq_ = NULL;
  } else {
    pfc_log_info("taskq not found for controller %s", ctr_name.c_str());
  }
}

/**
 *@brief    clear a particular controller's task queue
 *@param[in]  string controller_name.
 */
void PhyEventTaskqUtil::clear_task_queue(string ctr_name) {
  std::map<string, pfc::core::TaskQueue*>::iterator tq_map_iter;
  pfc::core::TaskQueue *phy_event_taskq_ = NULL;
  ScopedReadWriteLock taskqmaplock(
        PhyEventTaskqUtil::get_taskqmap_mutex_(), PFC_TRUE);  // write lock
  tq_map_iter = taskq_map_.find(ctr_name);
  if (tq_map_iter != taskq_map_.end()) {
    pfc_log_debug("Clear taskqueue of controller:%s", ctr_name.c_str());
    phy_event_taskq_ = (tq_map_iter)->second;
    int ret = phy_event_taskq_->clear(NULL);
    if (ret != 0)
      pfc_log_info("Error in clearing taskq ctr_name = %s", ctr_name.c_str());
  } else {
    pfc_log_debug("taskq not found for controller %s", ctr_name.c_str());
  }
}
/**
 *@brief    create_task_queue - return status of creation.
 *          it creates a new task queue.
 *@param[in] string controller_name
 */
UncRespCode PhyEventTaskqUtil::create_task_queue(string ctr_name) {
  pfc::core::TaskQueue *phy_event_taskq_ = NULL;
  UncRespCode status = UNC_RC_SUCCESS;
  pfc_log_debug("taskq create for controller %s", ctr_name.c_str());
  ScopedReadWriteLock taskqmaplock(
        PhyEventTaskqUtil::get_taskqmap_mutex_(), PFC_TRUE);  // write lock
  std::map<string, pfc::core::TaskQueue*>::iterator tq_map_iter;
  tq_map_iter = taskq_map_.find(ctr_name);
  if (tq_map_iter != taskq_map_.end()) {
    pfc_log_info("TaskQueue recreation flow SHOULDNOTHAPPEN");
  } else {
  phy_event_taskq_ = pfc::core::TaskQueue::create(concurrency_);
  if (phy_event_taskq_ == NULL) {
    pfc_log_info("TaskQueue creation failed");
    status = UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  } else {
    taskq_map_.insert(std::pair<string, pfc::core::TaskQueue *>
                                        (ctr_name, phy_event_taskq_));
    pfc_log_debug("taskqueue is created for controller %s", ctr_name.c_str());
  }
  }
  return status;
}

/**
 *@brief    get_task_queue - return a particular controller's
 *          taskqueue from taskq_map if exists. If queue is not available for a
 *          controller, it shall return NULL.
 *@param[in] string controller_name 
 */
pfc::core::TaskQueue* PhyEventTaskqUtil::get_task_queue(string ctr_name) {
  pfc_log_debug("taskq get for controller %s", ctr_name.c_str());
  std::map<string, pfc::core::TaskQueue*>::iterator tq_map_iter;
  pfc::core::TaskQueue *phy_event_taskq_ = NULL;
  ScopedReadWriteLock taskqmaplock(
        PhyEventTaskqUtil::get_taskqmap_mutex_(), PFC_FALSE);  // read lock
  tq_map_iter = taskq_map_.find(ctr_name);
  if (tq_map_iter != taskq_map_.end()) {
    phy_event_taskq_ = (tq_map_iter)->second;
    pfc_log_debug("taskq found for controller %s", ctr_name.c_str());
  } else {
    pfc_log_info("taskq NOT found for controller %s", ctr_name.c_str());
  }
  return phy_event_taskq_;
}

/**
 *@brief    DispatchNotificationEvent - event processing function shall be
            dispatched to the task queue for further process as separate task.
 *@param[in] EventDetail - consist the necessary information to process event further. 
             string ctr_name
 */
int PhyEventTaskqUtil::DispatchNotificationEvent(
                   EventAlarmDetail& event_detail,
                   string ctr_name) {
  int ret = 0;
  pfc::core::TaskQueue *event_taskq = get_task_queue(ctr_name);
  if (event_taskq == NULL) {
    pfc_log_info("TaskQueue is NULL, controller is not available");
    return -1;
  }
  pfc_timespec_t msectm;
  pfc_clock_gettime(&msectm);
  event_detail.eventid = pfc_clock_time2msec(&msectm);
  event_detail.controller_name = ctr_name;
  NotificationEventParams func_obj(event_detail);
  pfc::core::taskq_func_t  task_func(func_obj);
  ret = event_taskq->dispatch(task_func);
  if (ret != 0) {
    pfc_log_info("failed to dispatch() for invoke event process");
  } else {
  pfc_log_info("TASKQ ADD cname:%s kt:%d evid:%"PFC_PFMT_u64,
      event_detail.controller_name.c_str(), event_detail.key_type,
      event_detail.eventid);
  }
  return ret;
}

/**
 *@brief   NotificationEventParams - constructor, it receives
 *         EventAlarmDetail object from its caller
 *@param[in]  EventAlarmDetail
 */
NotificationEventParams::NotificationEventParams(
                EventAlarmDetail& event_detail):event_detail_(event_detail) {
}

/**
 *@brief  NotificationEventParams - destructor, it clears
 *         memory if any
 *@param[in] None 
 */
NotificationEventParams::~NotificationEventParams() {
  // memeoy need to be deleted if any
}

/**
 *@brief   InvokeNotificationEventProces - dispatch thread shall
 *         continue the queued task from here.
 *@param[in] none 
 */
void NotificationEventParams::InvokeNotificationEventProcess(void) {
  PHY_FINI_EVENT_LOCK();
  UncRespCode db_ret = UNC_RC_SUCCESS;
  PHY_DB_SB_CXN_LOCK();
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
    return;
  }
  pfc_log_info("TASKQ PRC cname:%s kt:%d evid:%"
      PFC_PFMT_u64" almflag:%d op:%d dt:%d at:%d",
      event_detail_.controller_name.c_str(), event_detail_.key_type,
      event_detail_.eventid, event_detail_.alarm_flag,
      event_detail_.operation, event_detail_.data_type,
      event_detail_.alarm_type);

  if (event_detail_.alarm_flag == TQ_EVENT) {
    InvokeProcessEvent(db_conn);
  } else if (event_detail_.alarm_flag == TQ_ALARM) {
    InvokeProcessAlarm(db_conn);
  }
}

/**
 *@brief  InvokeProcessEvent - processing notification events
 *@param[in] None
 */
void NotificationEventParams::InvokeProcessEvent(
    OdbcmConnectionHandler& db_conn) {
  UncRespCode status = UNC_RC_SUCCESS;
  NotificationRequest notifyrequest;
  notifyrequest.GetNotificationDT(&db_conn, event_detail_.controller_name,
             event_detail_.data_type);
  // process the event_detail and call a respective method
  switch (event_detail_.key_type) {
    case UNC_KT_PORT:
    case UNC_KT_PORT_NEIGHBOR:
    case UNC_KT_SWITCH:
    case UNC_KT_LINK:
    case UNC_KT_CTR_DOMAIN:
    case UNC_KT_LOGICAL_PORT:
    {
       PhysicalLayer::ctr_oprn_mutex_.lock();
       map<string, CtrOprnStatus> :: iterator it =
            PhysicalLayer::ctr_oprn_status_.find(event_detail_.controller_name);
       if (it != PhysicalLayer::ctr_oprn_status_.end()) {
         pfc_log_debug(
            "Controller %s IsIpChanged %d EvtStRecvd %d",
             event_detail_.controller_name.c_str(), it->second.IsIPChanged
             , it->second.EventsStartReceived);
         if (it->second.IsIPChanged == true ||
                  it->second.EventsStartReceived == false) {
           PhysicalLayer::ctr_oprn_mutex_.unlock();
           return;
         }
       }
       PhysicalLayer::ctr_oprn_mutex_.unlock();
       status = notifyrequest.InvokeKtDriverEvent(&db_conn,
                event_detail_.operation,
                event_detail_.data_type,
                event_detail_.key_struct,
                event_detail_.new_val_struct,
                event_detail_.old_val_struct,
                event_detail_.key_type);
      break;
    }
    case UNC_KT_CONTROLLER:
    {
      Kt_Controller NotifyController;


      val_ctr_st_t *new_val_ctr = (reinterpret_cast<val_ctr_st_t*>
                                 (event_detail_.new_val_struct));
      uint8_t driver_oper_status = new_val_ctr->oper_status;
      pfc_bool_t is_events_done = PFC_FALSE;
      key_ctr_t *key_ctr = (reinterpret_cast<key_ctr_t*>
                                 (event_detail_.key_struct));
      string controller_name = reinterpret_cast<char *>
        (key_ctr->controller_name);
      if (driver_oper_status == CONTROLLER_EVENTS_START) {
        uint8_t oper_status_db = UPPL_CONTROLLER_OPER_UP,
                new_oper_status = UPPL_CONTROLLER_OPER_EVENTS_START;
        UncRespCode read_status;
        ReadWriteLock *rwlock = NULL;
        PhysicalLayer::ctr_oprn_mutex_.lock();
        PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, true);
        if (read_status != UNC_RC_SUCCESS) {
          pfc_log_error("PHY_OPERSTATUS_LOCK:controller not avail");
          PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, false);
          PhysicalLayer::ctr_oprn_mutex_.unlock();
          return;
        }
        // Set the oper status as UP
        new_val_ctr->oper_status = UPPL_CONTROLLER_OPER_UP;
        new_val_ctr->valid[kIdxOperStatus] = UNC_VF_VALID;
        UncRespCode handle_oper_status = NotifyController.HandleOperStatus(
          &db_conn, UNC_DT_RUNNING,
          key_ctr,
          new_val_ctr,
          true);
        pfc_log_debug("EventStart: handle_oper_status ret = %d",
                                           handle_oper_status);
        pfc_log_debug(
            "Received events start notification for controller %s",
            controller_name.c_str());
        UncRespCode return_code =
            NotifyController.SendOperStatusNotification(*key_ctr,
                                             oper_status_db, new_oper_status);
        if (return_code == UNC_RC_SUCCESS) {
          pfc_log_debug("EVENTS_START notification is sent to UPLL");
        } else {
          pfc_log_error("Err in sending EVENTS_START notification to UPLL");
        }
        // Set the flag related to EVENT_START as true
        // It will be used to allow the events after receving EVENT_START
        map<string, CtrOprnStatus> :: iterator it =
            PhysicalLayer::ctr_oprn_status_.find(controller_name);
        if (it != PhysicalLayer::ctr_oprn_status_.end()) {
          CtrOprnStatus ctr_oprn =  it->second;
          ctr_oprn.EventsStartReceived =  true;
          ctr_oprn.ActualOperStatus = UPPL_CONTROLLER_OPER_UP; 
          PhysicalLayer::ctr_oprn_status_[controller_name] = ctr_oprn;
        }
        PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, false);
        PhysicalLayer::ctr_oprn_mutex_.unlock();
        return;
      }
      //  Check for events_done and do required steps
      if (driver_oper_status == CONTROLLER_EVENTS_DONE) {
        // CONTROLLER_OPER_UP can be set
        // Its same as enum UPPL_CONTROLLER_OPER_UP
        new_val_ctr->oper_status = CONTROLLER_OPER_UP;
        pfc_log_debug(
            "Received end of events notification for controller %s",
            controller_name.c_str());
        IPCConnectionManager *ipc_mgr = PhysicalLayer::get_instance()->
            get_ipc_connection_manager();
        pfc_bool_t is_controller_in_audit = ipc_mgr->
            IsControllerInAudit(controller_name);
        if (is_controller_in_audit == PFC_TRUE) {
          pfc_log_trace("Calling CancelTimer");
          PhysicalLayer::get_phy_dbsbcxn_lock_()->unlock();
          {
          // To cancel the already running timer in Audit
          PHY_TIMER_LOCK();
          UncRespCode cancel_ret = ipc_mgr->CancelTimer(controller_name);
          if (cancel_ret != UNC_RC_SUCCESS) {
            pfc_log_info("Failure in cancelling timer for controller %s",
                         controller_name.c_str());
          }
          }
          PhysicalLayer::get_phy_dbsbcxn_lock_()->wrlock();
          AuditRequest audit_req;
          pfc_log_debug("Calling MergeAuditDbToRunning");
          UncRespCode merge_auditdb =
              audit_req.MergeAuditDbToRunning(&db_conn, reinterpret_cast<char *>
          (key_ctr->controller_name));
          if (merge_auditdb != UNC_RC_SUCCESS) {
            pfc_log_info("Merge of audit and running db failed");
          }
        } else {
          pfc_log_info("End of events received non-audit controller %s",
                       controller_name.c_str());
          return;
        }
        is_events_done = PFC_TRUE;
      }
      status = NotifyController.HandleDriverEvents(
              &db_conn, event_detail_.key_struct,
              event_detail_.operation,
              event_detail_.data_type,
              event_detail_.old_val_struct,
              event_detail_.new_val_struct,
              is_events_done);
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT:
    {
       PhysicalLayer::ctr_oprn_mutex_.lock();
       map<string, CtrOprnStatus> :: iterator it =
            PhysicalLayer::ctr_oprn_status_.find(event_detail_.controller_name);
       if (it != PhysicalLayer::ctr_oprn_status_.end()) {
         if (it->second.IsIPChanged == true ||
                         it->second.EventsStartReceived == false) {
           PhysicalLayer::ctr_oprn_mutex_.unlock();
           return;
         }
       }
       PhysicalLayer::ctr_oprn_mutex_.unlock();
      Kt_LogicalMemberPort NotifyLogicalMemberPort;
      status = NotifyLogicalMemberPort.HandleDriverEvents(
               &db_conn, event_detail_.key_struct,
               event_detail_.operation,
               event_detail_.data_type, event_detail_.key_type, NULL, NULL);
      break;
    }
    default:
    {
      pfc_log_error("Invalid key type received at InvokeProcessEvent");
      return;
    }
  }
  pfc_log_debug("Return from InvokeProcessEvent: %d", status);
}

/**
 *@brief  InvokeProcessAlarm - processing alarms
 *@param[in] None
 */
void NotificationEventParams::InvokeProcessAlarm(
    OdbcmConnectionHandler& db_conn) {
  UncRespCode status = UNC_RC_SUCCESS;
  NotificationRequest notifyrequest;
  notifyrequest.GetNotificationDT(&db_conn, event_detail_.controller_name,
            event_detail_.data_type);
  PhysicalLayer::ctr_oprn_mutex_.lock();
  map<string, CtrOprnStatus> :: iterator it =
          PhysicalLayer::ctr_oprn_status_.find(event_detail_.controller_name);
  if (it != PhysicalLayer::ctr_oprn_status_.end()) {
    if (it->second.IsIPChanged == true ||
                 it->second.EventsStartReceived == false) {
      PhysicalLayer::ctr_oprn_mutex_.unlock();
      return;
     }
  }
  PhysicalLayer::ctr_oprn_mutex_.unlock();
  switch (event_detail_.key_type) {
    case UNC_KT_CONTROLLER:
    {
      Kt_Controller NotifyController;
      //  IF data_type == IMPORT that means audit is going on,
      //  so buffer path fault
      if (event_detail_.data_type == UNC_DT_IMPORT) {
        pfc_log_trace("Path fault alarm-store in buffer");
        notifyrequest.FillAlarmDetails(event_detail_.alarm_type,
                        event_detail_.operation,
                        event_detail_.key_struct,
                        event_detail_.new_val_struct);
      } else {
         status = NotifyController.HandleDriverAlarms(
           &db_conn, event_detail_.data_type, event_detail_.alarm_type,
           event_detail_.operation, event_detail_.key_struct,
           event_detail_.new_val_struct);
         pfc_log_debug(
          "Return status of controller HandleDriverAlarms: %d", status);
      }
      break;
    }
    case UNC_KT_CTR_DOMAIN:
    {
      Kt_Ctr_Domain NotifyDomain;
      status = NotifyDomain.HandleDriverAlarms(
          &db_conn, event_detail_.data_type, event_detail_.alarm_type,
          event_detail_.operation, event_detail_.key_struct, NULL);
      pfc_log_debug(
         "Return status of domain HandleDriverAlarms: %d", status);
      break;
    }
    case UNC_KT_LOGICAL_PORT:
    {
      Kt_LogicalPort NotifyLogicalPort;
      status = NotifyLogicalPort.HandleDriverAlarms(
          &db_conn, event_detail_.data_type, event_detail_.alarm_type,
          event_detail_.operation, event_detail_.key_struct, NULL);
      pfc_log_debug(
         "Return status of sub_domain HandleDriverAlarms: %d", status);
      break;
    }
    case UNC_KT_PORT:
    {
      Kt_Port NotifyPort;
      status = NotifyPort.HandleDriverAlarms(
          &db_conn, event_detail_.data_type, event_detail_.alarm_type,
          event_detail_.operation, event_detail_.key_struct, NULL);
      pfc_log_debug(
         "Return status of port HandleDriverAlarms: %d", status);
      break;
    }
    case UNC_KT_SWITCH:
    {
      Kt_Switch NotifySwitch;
      status = NotifySwitch.HandleDriverAlarms(
          &db_conn, event_detail_.data_type, event_detail_.alarm_type,
          event_detail_.operation, event_detail_.key_struct, NULL);
      pfc_log_debug(
          "Return status of switch HandleDriverAlarms: %d", status);
      break;
    }
   default:
    {
      pfc_log_error("Invalid key type\n");
    }
  }
}
