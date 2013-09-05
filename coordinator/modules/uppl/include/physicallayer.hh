/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *@brief   Physical Layer Header
 *@file    physical_layer.hh
 * Desc:
 * This header file contains the declaration of PhysicalLayer class
 * which is the base class for all other classes
 */

#ifndef _PHYSICAL_LAYER_HH_
#define _PHYSICAL_LAYER_HH_

#include <pfc/debug.h>
#include <pfc/log.h>
#include <pfc/hostaddr.h>
#include <pfcxx/module.hh>
#include <pfcxx/ipc_server.hh>
#include <pfcxx/ipc_client.hh>
#include <pfcxx/synch.hh>
#include "odbcm_mgr.hh"
#include "ipc_connection_manager.hh"
#include "physical_core.hh"
#include "odbcm_connection.hh"

using unc::uppl::PhysicalCore;
using unc::uppl::ODBCManager;
using unc::uppl::IPCConnectionManager;
using pfc::core::ipc::ServerSession;
using pfc::core::ReadWriteLock;

/* This struct should be outside namespace */
struct build_stamp {
    const char  *b_host;        /* build host */
    const char  *b_date;        /* build date */
    const char  *b_uname;       /* uname -a */

    build_stamp(const char *host, const char *date, const char *uname)
    : b_host(host), b_date(date), b_uname(uname) {
    }
};

#define PHY_FINI_IPC_LOCK(return_code) \
  if (PhysicalLayer::phyFiniFlag == 1) { \
    pfc_log_info("PhysicalLayer Fini is invoked already ..!!"); \
    return return_code; \
  } \
  ScopedReadWriteLock ipcFiniLock(PhysicalLayer::get_phy_fini_phycore_lock_(), \
      PFC_FALSE); \
  if (PhysicalLayer::phyFiniFlag == 1) { \
    pfc_log_info("PhysicalLayer:: Fini is invoked already ..!!"); \
    return return_code; \
  }

#define PHY_FINI_EVENT_LOCK() \
  if (PhysicalLayer::phyFiniFlag == 1) { \
    pfc_log_info("PhysicalLayer Fini is invoked already ..!!"); \
    return; \
  } \
  ScopedReadWriteLock eventFiniLock(PhysicalLayer::get_phy_fini_event_lock_(), \
      PFC_FALSE); \
  if (PhysicalLayer::phyFiniFlag == 1) { \
    pfc_log_info("PhysicalLayer:: Fini is invoked already ..!!"); \
    return; \
  }

#define OPEN_DB_CONNECTION_TC_REQUEST(conn_type) \
  UpplReturnCode db_ret = UPPL_RC_SUCCESS; \
  /* Create a new odbcm connection */ \
  OdbcmConnectionHandler db_conn(conn_type, db_ret, \
      PhysicalLayer::get_instance()->get_odbc_manager()); \
  if (db_ret != UPPL_RC_SUCCESS) { \
    /* Error in opening db connection */ \
    pfc_log_error("Error in opening DB connection"); \
    return unc::tclib::TC_FAILURE; \
  }

#define OPEN_DB_CONNECTION(conn_type, db_ret) \
  /* Create a new odbcm connection */ \
  OdbcmConnectionHandler db_conn(conn_type, db_ret, \
      PhysicalLayer::get_instance()->get_odbc_manager());

namespace unc {

namespace uppl {
/**************************************************************************
It is a singleton class which will instantiate other UPPL's classes. 
This class will be inherited from PFC module in order to 
use the IPC service handlers.For further info,see the comments in .cc file
 ***************************************************************************/
class PhysicalLayer : public pfc::core::Module {
  public:
  explicit PhysicalLayer(const pfc_modattr_t *mattr):Module(mattr) {
    physical_core_ = NULL;
    ipc_connection_manager_ = NULL;
    odbc_manager_ = NULL;
    phyFiniFlag = 0;
  }
  pfc_bool_t init(void);
  pfc_bool_t fini(void);
  static PhysicalLayer* get_instance();
  UpplReturnCode InitializePhysicalSubModules();
  UpplReturnCode FinalizePhysicalSubModules();
  pfc_ipcresp_t ipcService(ServerSession &session,
                           pfc_ipcid_t service_id);
  PhysicalCore* get_physical_core();
  IPCConnectionManager* get_ipc_connection_manager();
  ODBCManager* get_odbc_manager();
  static ReadWriteLock* get_phy_fini_db_lock() {
    return &phy_fini_db_lock_;
  }
  static ReadWriteLock* get_phy_fini_phycore_lock_() {
    return &phy_fini_phycore_lock_;
  }
  static ReadWriteLock* get_phy_fini_event_lock_() {
    return &phy_fini_event_lock_;
  }
  static ReadWriteLock* get_events_done_lock_() {
    return &events_done_lock_;
  }
  static Mutex physical_layer_mutex_;
  static Mutex physical_core_mutex_;
  static Mutex ipc_client_config_hdlr_mutex_;
  static Mutex ipc_client_logical_hdlr_mutex_;
  static Mutex ipc_server_hdlr_mutex_;
  static Mutex notification_manager_mutex_;
  static Mutex ODBCManager_mutex_;
  static ReadWriteLock phy_fini_db_lock_;
  static ReadWriteLock phy_fini_phycore_lock_;
  static ReadWriteLock phy_fini_event_lock_;
  static ReadWriteLock events_done_lock_;
  static uint8_t phyFiniFlag;

  private:
  static PhysicalLayer* physical_layer_;
  PhysicalCore* physical_core_;
  IPCConnectionManager* ipc_connection_manager_;
  ODBCManager* odbc_manager_;
};

class ScopedReadWriteLock {
 public:
  ScopedReadWriteLock(ReadWriteLock* coreRwLock, pfc_bool_t isWriteLock)
  : rwLock(coreRwLock), isWrLock(isWriteLock) {
    if (isWrLock == PFC_TRUE) {
      pfc_log_debug("ScopedReadWriteLock write lock");
      rwLock->wrlock();
    } else {
      pfc_log_debug("ScopedReadWriteLock read lock");
      rwLock->rdlock();
    }
  }

  ~ScopedReadWriteLock() {
    if (isWrLock == PFC_TRUE) {
      pfc_log_debug("ScopedReadWriteLock write unlock");
    } else {
      pfc_log_debug("ScopedReadWriteLock read unlock");
    }
    rwLock->unlock();
  }

 private:
  ReadWriteLock *rwLock;
  pfc_bool_t isWrLock;
};

}  // namespace uppl
}  // namespace unc


#endif  /* _PHYSICAL_LAYER_HH_ */
