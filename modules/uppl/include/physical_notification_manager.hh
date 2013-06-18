/*
 * Copyright (c) 2012-2013 NEC Corporation
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
#include <vector>
#include <string>
#include <map>
#include "physical_common_def.hh"

using pfc::core::ipc::IpcEventHandler;
using pfc::core::ipc::IpcEvent;
using std::vector;
using std::string;
using std::map;
using pfc::core::timer_func_t;
using pfc::core::TaskQueue;
using pfc::core::Timer;

namespace unc {
namespace uppl {
/**
 * * * @Description: Notification Manager class is responsible for all
 * notification event handling procedure
 * **/
class NotificationManager:public IpcEventHandler {
  public:
  virtual void eventHandler(const IpcEvent &event);
  virtual const char *getName(void);
  static NotificationManager* get_notification_manager(unc_keytype_ctrtype_t);
  static void release_notification_manager();
  private:
  NotificationManager() {}
  ~NotificationManager() {}
  static NotificationManager* pfc_notification_manager_;
  static NotificationManager* vnp_notification_manager_;
};

}  // namespace uppl
}  // namespace unc

#endif  // _NOTIFICATION_MANAGER_HH_
