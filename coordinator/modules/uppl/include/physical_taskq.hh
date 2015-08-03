/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_PHY_TASKQ_H_
#define UNC_PHY_TASKQ_H_

#include <pfcxx/task_queue.hh>
#include <unc/unc_base.h>
#include <unc/keytype.h>
#include <pfcxx/module.hh>
#include <pfcxx/synch.hh>
#include <map>
#include <string>
#include <functional>
#include "physical_common_def.hh"
#include "odbcm_connection.hh"
using std::string;
using std::map;
using pfc::core::TaskQueue;
using pfc::core::ReadWriteLock;
namespace unc {
namespace uppl {

typedef enum {
  TQ_EVENT = 0,
  TQ_ALARM
}EventType;

struct EventAlarmDetail {
  EventAlarmDetail():operation(0),
                data_type(0),
                key_type(0),
                key_struct(NULL),
                key_size(0),
                old_val_struct(NULL),
                new_val_struct(NULL),
                val_size(0),
                alarm_type(0),
                alarm_flag(TQ_EVENT),
                eventid(0),
                controller_name("") {
  }
  EventAlarmDetail(EventType flag):operation(0),
                data_type(0),
                key_type(0),
                key_struct(NULL),
                key_size(0),
                old_val_struct(NULL),
                new_val_struct(NULL),
                val_size(0),
                alarm_type(0),
                alarm_flag(flag),
                eventid(0),
                controller_name("") {
  }
  EventAlarmDetail(const EventAlarmDetail& event_detail) {
    operation = event_detail.operation;
    data_type = event_detail.data_type;
    key_type = event_detail.key_type;
    key_size = event_detail.key_size;
    if (event_detail.key_struct != NULL) {
      key_struct = (void*)malloc(event_detail.key_size);
      if (key_struct != NULL)
        memcpy(key_struct, event_detail.key_struct, event_detail.key_size);
      else
        pfc_log_error("key_struct malloc is failed ");
    } else {
      key_struct = NULL;
    }
    val_size = event_detail.val_size;
    if (event_detail.old_val_struct != NULL) {
      old_val_struct = (void*)malloc(event_detail.val_size);
      if (old_val_struct != NULL)
        memcpy(old_val_struct, event_detail.old_val_struct,
                        event_detail.val_size);
      else
        pfc_log_error("old_val_struct malloc is failed ");
    } else {
      old_val_struct = NULL;
    }
    if (event_detail.new_val_struct != NULL) {
      new_val_struct = (void*)malloc(event_detail.val_size);
      if (new_val_struct != NULL)
        memcpy(new_val_struct, event_detail.new_val_struct,
                event_detail.val_size);
      else
        pfc_log_error("new_val_struct malloc is failed ");
    } else {
      new_val_struct = NULL;
    }
    alarm_type = event_detail.alarm_type;
    alarm_flag = event_detail.alarm_flag;
    eventid = event_detail.eventid;
    controller_name = event_detail.controller_name;
  }
  EventAlarmDetail& operator= (const EventAlarmDetail& event_detail) {
    operation = event_detail.operation;
    data_type = event_detail.data_type;
    key_type = event_detail.key_type;
    key_size = event_detail.key_size;
    if (event_detail.key_struct != NULL) {
      key_struct = (void*)malloc(event_detail.key_size);
      memcpy(key_struct, event_detail.key_struct, event_detail.key_size);
    } else {
      key_struct = NULL;
    }
    val_size = event_detail.val_size;
    if (event_detail.old_val_struct != NULL) {
      old_val_struct = (void*)malloc(event_detail.val_size);
      memcpy(old_val_struct, event_detail.old_val_struct,
                 event_detail.val_size);
    } else {
      old_val_struct = NULL;
    }
    if (event_detail.new_val_struct != NULL) {
      new_val_struct = (void*)malloc(event_detail.val_size);
      memcpy(new_val_struct, event_detail.new_val_struct,
               event_detail.val_size);
    } else {
      new_val_struct = NULL;
    }
    alarm_type = event_detail.alarm_type;
    alarm_flag = event_detail.alarm_flag;
    eventid = event_detail.eventid;
    controller_name = event_detail.controller_name;
    return *this;
  }
  ~EventAlarmDetail() {
    if (key_struct != NULL) {
      free(key_struct);
      key_struct = NULL;
    }
    if (old_val_struct != NULL) {
      free(old_val_struct);
      old_val_struct = NULL;
    }
    if (new_val_struct) {
      free(new_val_struct);
      new_val_struct = NULL;
    }
  }
  uint32_t operation;
  uint32_t data_type;
  uint32_t key_type;
  void* key_struct;
  size_t key_size;
  void* old_val_struct;
  void* new_val_struct;
  size_t val_size;
  uint32_t alarm_type;
  uint8_t alarm_flag;
  uint64_t eventid;
  string controller_name;
};

class PhyEventTaskqUtil {
  private:
  uint32_t concurrency_;
  map<string, TaskQueue*> taskq_map_;

  public:
  explicit PhyEventTaskqUtil(uint32_t concurrency);
  ~PhyEventTaskqUtil();
  UncRespCode create_task_queue(string ctr_name);
  pfc::core::TaskQueue* get_task_queue(string ctr_name);
  void del_task_queue(string ctr_name);
  void clear_task_queue(string ctr_name);

  int DispatchNotificationEvent(
         EventAlarmDetail& event_detail, string ctr_name);
  static ReadWriteLock taskqmap_mutex_;
  static ReadWriteLock* get_taskqmap_mutex_() {
    return &taskqmap_mutex_;
  }
};

class NotificationEventParams: public std::unary_function < void, void > {
  public:
  EventAlarmDetail event_detail_;
  NotificationEventParams(EventAlarmDetail& event_detail);
  ~NotificationEventParams();
  void operator() ()  {
    InvokeNotificationEventProcess();
  }
  void InvokeNotificationEventProcess(void);
  void InvokeProcessEvent(OdbcmConnectionHandler& db_conn);
  void InvokeProcessAlarm(OdbcmConnectionHandler& db_conn);
};
}
}
#endif
