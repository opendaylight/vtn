/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_IPC_EVENT_QUEUE_HH_
#define UPLL_IPC_EVENT_QUEUE_HH_

#include <string>

#include "pfcxx/task_queue.hh"

namespace unc {
namespace upll {
namespace ctrlr_events {

static const uint32_t kUpllTaskConcurrency = 1;
static const uint32_t kUpllTaskDetatched = 0;

struct EventArgument {
  enum IpcEventType {
    UNKNOWN_EVENT = 0,
    UPPL_CTRLR_STATUS_EVENT,
    UPPL_LOGICAL_PORT_STATUS_EVENT,
    UPPL_BOUNDARY_STATUS_EVENT,
    UPPL_PATH_FAULT_ALARM,
    PFCDRV_POLICER_FULL_ALARM,
    PFCDRV_POLICER_FAIL_ALARM,
    PFCDRV_NWMON_FAULT_ALARM,
    PFCDRV_VTNID_EXHAUSTION_ALARM
  };

  IpcEventType event_type_;

  EventArgument():event_type_(UNKNOWN_EVENT) {}
  virtual ~EventArgument() {}
};

struct CtrlrStatusArg : public EventArgument {
  std::string ctrlr_name_;
  uint8_t operstatus_;
};

struct LogicalPortArg : public EventArgument {
  std::string ctrlr_name_;
  std::string domain_name_;
  std::string logical_port_id_;
  uint8_t operstatus_;
};

struct BoundaryStatusArg : public EventArgument {
  std::string boundary_id_;
  bool operstatus_;
};

struct PathFaultArg : public EventArgument {
  std::string ctrlr_name_;
  std::string domain_name_;
  bool alarm_raised_;
};

struct PolicierAlarmArg : public EventArgument {
  std::string ctrlr_name_;
  std::string domain_name_;
  bool alarm_raised_;
  key_vtn key_vtn_;
  pfcdrv_policier_alarm_data_t policier_data_;
};

struct NwmonFaultArg : public EventArgument {
  std::string ctrlr_name_;
  std::string domain_name_;
  bool alarm_raised_;
  key_vtn key_vtn_;
  pfcdrv_network_mon_alarm_data_t network_mon_data_;
};
struct VtnIdExhaustionArg: public EventArgument {
  std::string ctrlr_name_;
  std::string domain_name_;
  bool alarm_raised_;
  key_vtn key_vtn_;
};


class UpllIpcEventQueue {
 public:
  UpllIpcEventQueue():ipc_event_taskq_(PFC_TASKQ_INVALID_ID)
                      , events_queued(0) { }
  ~UpllIpcEventQueue();
  void Create();  // create the queue
  void AddEvent(EventArgument *event_ptr);
  void Clear(const pfc_timespec_t *ts = NULL);  // delete all w/o processing

 private:
  static void EventHandler(void *event_ptr);
  inline static void DtorEventArgument(void *event_ptr) {
    if (event_ptr) {
      delete reinterpret_cast<EventArgument *>(event_ptr);
    }
    ++events_processed;
  }
  pfc_taskq_t ipc_event_taskq_;
  // Assumption: UpllIpcEventQueue is instantiated only once. So, counter
  // events_processed is declared as static. It needs improvement.
  // Rollovers are not designed.
  uint32_t events_queued;
  static uint32_t events_processed;
};

}  // namespace ctrlr_events
}  // namespace upll
}  // namespace unc

#endif  // UPLL_IPC_EVENT_QUEUE_HH_
