/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_CONFIG_SVC_HH_
#define UPLL_CONFIG_SVC_HH_

#include <string>
#include <map>

#include "pfcxx/module.hh"

#include "cxx/pfcxx/synch.hh"
#include "cxx/pfcxx/ipc_server.hh"
#include "cxx/pfcxx/ipc_client.hh"

#include "ipc_event_queue.hh"
#include "config_mgr.hh"

namespace unc {
namespace upll {
namespace config_svc {

class UpllIpcEventHandler;

class UpllConfigSvc : public pfc::core::Module {
 public:
  static unsigned int kNumCfgServices;
  explicit UpllConfigSvc(const pfc_modattr_t *mattr);
  ~UpllConfigSvc(void);
  pfc_bool_t init(void);
  pfc_bool_t fini(void);
  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession &sess,        // NOLINT
                           pfc_ipcid_t service);

  void PhysicalEventHandler(const pfc::core::ipc::IpcEvent &event);
  void PfcDriverAlarmHandler(const pfc::core::ipc::IpcEvent &event);

  bool IsShuttingDown() {
    bool state;
    sys_state_rwlock_.rdlock();
    state = shutting_down_;
    sys_state_rwlock_.unlock();
    return state;
  }
  bool IsActiveNode() {
    bool state;
    sys_state_rwlock_.rdlock();
    state = node_active_;
    sys_state_rwlock_.unlock();
    return state;
  }

 private:
  pfc_ipcresp_t KtService(pfc::core::ipc::ServerSession *sess,
                          upll_service_ids_t service);
  pfc_ipcresp_t GlobalConfigService(pfc::core::ipc::ServerSession *sess,
                                    upll_service_ids_t service);
  pfc_ipcresp_t HandleIsKeyInUse(pfc::core::ipc::ServerSession *sess,
                                 int index);
  pfc_ipcresp_t HandleUpplUpdate(pfc::core::ipc::ServerSession *sess,
                                 int index);
  // System and Cluster Event
  bool RegisterForModuleEvents();
  static void HandleSystemEventStatic(pfc_event_t event, pfc_ptr_t arg);
  void HandleSystemEvent(pfc_event_t event);
  static void HandleClusterEventStatic(pfc_event_t event, pfc_ptr_t arg);
  void HandleClusterEvent(pfc_event_t event);

  // IPC Event
  bool RegisterForIpcEvents();

  void UnregisterModuleEventHandlers();
  void UnregisterIpcEventHandlers();

  // UpllConfigSvc  *tclib_module_;
  unc::upll::config_momgr::UpllConfigMgr *config_mgr_;
  bool node_active_;  // If true, this node is active node
  bool shutting_down_;  // If true, upll is shutting down
  pfc::core::ReadWriteLock sys_state_rwlock_;

  static const uint32_t kEventPriority = 1000;
  static const uint32_t kIpcEventPriority = 50;

  pfc_evhandler_t sys_evhid_;   // system event handler
  pfc_evhandler_t cls_evhid_;   // cluster event handler

  UpllIpcEventHandler *physical_evhdlr_;
  UpllIpcEventHandler *pfcdriver_evhdlr_;

  pfc::core::Mutex ipc_event_mutex_;

  unc::upll::ctrlr_events::UpllIpcEventQueue event_queue_;
};

class UpllIpcEventHandler : public pfc::core::ipc::IpcEventHandler {
 public:
  enum IpcEventSource {
    IPC_EVENT_SOURCE_PHYSICAL,
    IPC_EVENT_SOURCE_PFCDRIVER
  };
  UpllIpcEventHandler(UpllConfigSvc *ucs, IpcEventSource src) {
    PFC_ASSERT(ucs != NULL);
    ucs_ = ucs;
    source_ = src;
  }

  void eventHandler(const pfc::core::ipc::IpcEvent &event) {
    if (source_ == IPC_EVENT_SOURCE_PHYSICAL) {
      ucs_->PhysicalEventHandler(event);
    } else if (source_ == IPC_EVENT_SOURCE_PFCDRIVER) {
      ucs_->PfcDriverAlarmHandler(event);
    }
  }

 private:
  UpllConfigSvc *ucs_;
  IpcEventSource source_;
};

}  // namespace config_svc
}  // namespace upll
}  // namespace unc

#endif  // UPLL_CONFIG_SVC_HH_
