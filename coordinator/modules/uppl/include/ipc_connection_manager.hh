/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *@brief   IPCConnectionManager header
 *@file    ipc_connection_manager.hh
 *Desc:This header file contains the declaration of IPCConnectionManager class
*/

#ifndef _IPC_CONNECTION_MANAGER_H_
#define _IPC_CONNECTION_MANAGER_H_

#include <pfcxx/ipc_client.hh>
#include <string>
#include <vector>
#include <map>
#include "unc/uppl_common.h"
#include "ipc_server_handler.hh"
#include "ipc_client_logical_handler.hh"
#include "physical_notification_manager.hh"
#include "odbcm_connection.hh"

using std::string;
using std::vector;
using std::map;
namespace unc {
namespace uppl {

/****************************************************************************
 IPCConnectionManager Class is responsible for holding IPC Server and Client
objects which are used to communicate with other UNC modules.
The various IPC classes that are held are IPCServerHandler,
IPCClientConfigurationHandler ,IPCClientLogicalHandler and NotificationManager.
******************************************************************************/
class IPCConnectionManager {
 public:
  IPCConnectionManager();
  ~IPCConnectionManager();
  void InitializeIpcServerClient();
  void InitIpcConnectionManager(string channel_name, string service_name);
  IPCServerHandler * get_ipc_server_handler();
  NotificationManager* get_notification_manager(unc_keytype_ctrtype_t);
  IPCClientLogicalHandler* get_ipc_client_logical_handler();
  string get_uppl_channel_name();
  string get_uppl_service_name();
  UncRespCode Finalize();
  uint32_t SendEvent(ServerEvent *evt, std::string controller_name,
                                                pfc_ipcevtype_t event_type);
  UncRespCode SendEventSubscription();
  UncRespCode CancelEventSubscription();
  void addControllerToAuditList(string);
  UncRespCode removeControllerFromAuditList(string);
  pfc_bool_t IsControllerInAudit(string controller_name);
  UncRespCode GetDriverPresence(uint32_t ctr_type);
  uint32_t StartNotificationTimer(OdbcmConnectionHandler *db_conn,
                                  string controller_name);
  void setTimeOutId(string controller_name, uint32_t notfn_timer_id) {
    notfn_timer_id_[controller_name] = notfn_timer_id;
  }
  uint32_t getTimeOutId(string controller_name) {
    uint32_t timer_id = 0;
    map<string, uint32_t> :: iterator iter =
        notfn_timer_id_.find(controller_name);
    if (iter != notfn_timer_id_.end()) {
      timer_id = (*iter).second;
    }
    return timer_id;
  }
  UncRespCode CancelTimer(string);

 private:
  IPCServerHandler* ipc_server_handler_;
  IPCClientLogicalHandler* ipc_client_logical_handler_;
  string uppl_ipc_channel_name_;
  string uppl_ipc_service_name_;
  // vector to denote a controller is being audited
  vector<string> controller_in_audit_;
  map<string, uint32_t> notfn_timer_id_;
  map<string, Timer *> timer_obj_;
  map<string, TaskQueue *> queue_obj_;
};
}  // namespace uppl
}  // namespace unc

#endif
