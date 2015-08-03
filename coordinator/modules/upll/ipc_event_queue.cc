/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <pthread.h>

#include "uncxx/upll_log.hh"

#include "ipc_event_queue.hh"
#include "config_mgr.hh"
#include "ctrlr_mgr.hh"


namespace unc {
namespace upll {
namespace ctrlr_events {

using unc::upll::config_momgr::UpllConfigMgr;
using unc::upll::config_momgr::CtrlrMgr;
uint32_t UpllIpcEventQueue::events_processed = 0;

UpllIpcEventQueue::~UpllIpcEventQueue() {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("IPC events queued: %d, processed: %d",
                events_queued, events_processed);
  /* for the same reasons, we cannot destroy
  if (ipc_event_taskq_ != PFC_TASKQ_INVALID_ID) {
    int err = pfc_taskq_destroy(ipc_event_taskq_);
    if (err != 0) {
      UPLL_LOG_INFO("Failed to destroy ipc_event_taskq_ err=%d", err);
    } else {
      UPLL_LOG_INFO("ipc_event_taskq_ destroyed successfully");
    }
  }
  ipc_event_taskq_ = PFC_TASKQ_INVALID_ID;
  */
}

void UpllIpcEventQueue::Create() {
  UPLL_FUNC_TRACE;
  if (ipc_event_taskq_ != PFC_TASKQ_INVALID_ID) {
    UPLL_LOG_WARN("ipc_event_taskq_ (%u) already created",
                  ipc_event_taskq_);
    return;
  }

  events_queued = events_processed = 0;

  // create task queue  for IPC event
  int err = pfc_taskq_create_named(&ipc_event_taskq_, NULL,
                                   kUpllTaskConcurrency,
                                   "upll_ipc_event_taskq");
  if (err != 0) {
    UPLL_LOG_FATAL("Failed to create ipc_event_taskq_ err=%d", err);
  } else {
    UPLL_LOG_INFO("ipc_event_taskq_ (%u) created successfully.",
                  ipc_event_taskq_);
  }
}

void UpllIpcEventQueue::Clear(const pfc_timespec_t *ts) {
  UPLL_FUNC_TRACE;
  if (ipc_event_taskq_ != PFC_TASKQ_INVALID_ID) {
    UPLL_LOG_INFO("IPC events queued: %d, processed: %d",
                  events_queued, events_processed);
    int err = pfc_taskq_clear(ipc_event_taskq_, ts);
    if (err != 0) {
      UPLL_LOG_FATAL("Failed to clear ipc_event_taskq_ err=%d", err);
    } else {
      UPLL_LOG_INFO("ipc_event_taskq_ cleared successfully");
    }
    events_queued = events_processed = 0;
  }
}

// event_ptr passed to this function will be owned by UpllIpcEventQueue
void UpllIpcEventQueue::AddEvent(EventArgument *event_ptr) {
  UPLL_FUNC_TRACE;
  if (event_ptr == NULL) {
    UPLL_LOG_ERROR("Null argument");
    return;
  }
  if (ipc_event_taskq_ != PFC_TASKQ_INVALID_ID) {
    pfc_taskfunc_t func = &UpllIpcEventQueue::EventHandler;
    pfc_taskdtor_t dtor_func = &UpllIpcEventQueue::DtorEventArgument;
    pfc_task_t tid = PFC_TASKQ_INVALID_TASKID;
    int err = pfc_taskq_dispatch_dtor(ipc_event_taskq_, func,
                                      reinterpret_cast<void*>(event_ptr),
                                      dtor_func, kUpllTaskDetatched, &tid);
    if (err != 0) {
      DtorEventArgument(event_ptr);
      UPLL_LOG_FATAL("Failed to dispatch ipc_event_taskq_. err=%d, tid=%u",
                     err, ipc_event_taskq_);
    } else {
      ++events_queued;
      UPLL_LOG_TRACE("ipc_event_taskq_ dispatched queued: %d, processed: %d",
                  events_queued, events_processed);
    }
  }
}

void UpllIpcEventQueue::EventHandler(void *event_ptr) {
  UPLL_FUNC_TRACE;
  EventArgument *arg_ptr = reinterpret_cast<EventArgument *>(event_ptr);
  if (event_ptr == NULL) {
    UPLL_LOG_ERROR("EventArgument pointer is  NULL");
    return;
  }
  UpllConfigMgr *config_mgr = UpllConfigMgr::GetUpllConfigMgr();
  switch (arg_ptr->event_type_) {
    case EventArgument::UPPL_CTRLR_STATUS_EVENT:
      {
        CtrlrStatusArg *ptr = reinterpret_cast<CtrlrStatusArg *>(arg_ptr);
        const char* ctrlr_id = ptr->ctrlr_name_.c_str();
        config_mgr->OnControllerStatusChange(ptr->ctrlr_name_.c_str(),
                                             ptr->operstatus_);
        if (ptr->operstatus_== UPPL_CONTROLLER_OPER_DOWN &&
            CtrlrMgr::GetInstance()->IsPathFaultOccured(ctrlr_id, "*"))
          CtrlrMgr::GetInstance()->ClearPathfault(ctrlr_id, "*");
        pthread_yield();
      }
      break;
    case EventArgument::UPPL_LOGICAL_PORT_STATUS_EVENT:
      {
        LogicalPortArg *ptr = reinterpret_cast<LogicalPortArg *>(arg_ptr);
        config_mgr->OnLogicalPortStatusChange(ptr->ctrlr_name_.c_str(),
                                              ptr->domain_name_.c_str(),
                                              ptr->logical_port_id_.c_str(),
                                              ptr->operstatus_);
        pthread_yield();
      }
      break;
    case EventArgument::UPPL_BOUNDARY_STATUS_EVENT:
      {
        BoundaryStatusArg *ptr = reinterpret_cast<BoundaryStatusArg *>(arg_ptr);
        config_mgr->OnBoundaryStatusChange(ptr->boundary_id_.c_str(),
                                           ptr->operstatus_);

        pthread_yield();
      }
      break;
    case EventArgument::UPPL_PATH_FAULT_ALARM:
      {
        PathFaultArg *ptr = reinterpret_cast<PathFaultArg *>(arg_ptr);
        config_mgr->OnPathFaultAlarm(ptr->ctrlr_name_.c_str(),
                                     ptr->domain_name_.c_str(),
                                     ptr->alarm_raised_);

        pthread_yield();
      }
      break;
    case EventArgument::PFCDRV_POLICER_FULL_ALARM:
      {
        PolicierAlarmArg *ptr = reinterpret_cast<PolicierAlarmArg *>(arg_ptr);
        config_mgr->OnPolicerFullAlarm(ptr->ctrlr_name_,
                                       ptr->domain_name_,
                                       ptr->key_vtn_,
                                       ptr->policier_data_,
                                       ptr->alarm_raised_);
        pthread_yield();
      }
      break;
    case EventArgument::PFCDRV_POLICER_FAIL_ALARM:
      {
        PolicierAlarmArg *ptr = reinterpret_cast<PolicierAlarmArg *>(arg_ptr);
        config_mgr->OnPolicerFailAlarm(ptr->ctrlr_name_,
                                       ptr->domain_name_,
                                       ptr->key_vtn_,
                                       ptr->policier_data_,
                                       ptr->alarm_raised_);
        pthread_yield();
      }
      break;
    case EventArgument::PFCDRV_NWMON_FAULT_ALARM:
      {
        NwmonFaultArg *ptr = reinterpret_cast<NwmonFaultArg *>(arg_ptr);
        config_mgr->OnNwmonFaultAlarm(ptr->ctrlr_name_,
                                      ptr->domain_name_,
                                      ptr->key_vtn_,
                                      ptr->network_mon_data_,
                                      ptr->alarm_raised_);
        pthread_yield();
      }
      break;
    case EventArgument::PFCDRV_VTNID_EXHAUSTION_ALARM:
      {
        VtnIdExhaustionArg *ptr =
                       reinterpret_cast<VtnIdExhaustionArg *>(arg_ptr);
        config_mgr->OnVtnIDExhaustionAlarm(ptr->ctrlr_name_.c_str(),
                                       ptr->domain_name_.c_str(),
                                       ptr->key_vtn_,
                                       ptr->alarm_raised_);
        pthread_yield();
      }
      break;
    case EventArgument::UNKNOWN_EVENT:
    default:
      UPLL_LOG_ERROR("UNKNOWN event dispatched in event queue");
  }
}
}  // namespace ctrlr_events
}  // namespace upll
}  // namespace unc
