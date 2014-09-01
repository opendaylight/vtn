/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * *@brief   Notification Manager header file
 * *@file    physical_notification_manager.hh
 *
 * *Desc:This header file contains the declaration of Notification Manager class
 * */

#ifndef _NOTIFICATION_MANAGER_HH_
#define _NOTIFICATION_MANAGER_HH_

#include <pfcxx/timer.hh>
#include <pfcxx/task_queue.hh>
#include <string>
#include "physical_common_def.hh"
#include "physical_taskq.hh"

using pfc::core::ipc::IpcEventHandler;
using pfc::core::ipc::IpcEvent;
using std::string;
using pfc::core::timer_func_t;
using pfc::core::TaskQueue;
using pfc::core::Timer;
using unc::uppl::PhyEventTaskqUtil;

namespace unc {
namespace uppl {
/**
 * * * @Description: Notification Manager class is responsible for all
 * events and alarms notification received from south bound
 * **/
class NotificationManager:public IpcEventHandler {
  public:
  virtual void eventHandler(const IpcEvent &event);
  virtual const char *getName(void);
  static NotificationManager* get_notification_manager(unc_keytype_ctrtype_t);
  static void release_notification_manager();
  static void delete_taskq_util();
  static PhyEventTaskqUtil* taskq_util_;
  static PhyEventTaskqUtil* get_taskq_util();

  private:
  NotificationManager() {}
  ~NotificationManager() {}
  static NotificationManager* pfc_notification_manager_;
  static NotificationManager* vnp_notification_manager_;
  static NotificationManager* polc_notification_manager_;
  static NotificationManager* odc_notification_manager_;
};

}  // namespace uppl
}  // namespace unc

#endif  // _NOTIFICATION_MANAGER_HH_
