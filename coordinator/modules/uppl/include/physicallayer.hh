/*
 * Copyright (c) 2012-2014 NEC Corporation
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
    return (return_code); \
  } \
  ScopedReadWriteLock ipcFiniLock(PhysicalLayer::get_phy_fini_phycore_lock_(), \
      PFC_FALSE); \
  if (PhysicalLayer::phyFiniFlag == 1) { \
    pfc_log_info("PhysicalLayer:: Fini is invoked already ..!!"); \
    return (return_code); \
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

#define PHY_DB_SB_CXN_LOCK() \
  if (PhysicalLayer::phyFiniFlag == 1) { \
    pfc_log_info("PhysicalLayer Fini is invoked already ..!!"); \
    return; \
  } \
  ScopedReadWriteLock dbSbCxnLock(PhysicalLayer::get_phy_dbsbcxn_lock_(), \
      PFC_TRUE); \
  if (PhysicalLayer::phyFiniFlag == 1) { \
    pfc_log_info("PhysicalLayer:: Fini is invoked already ..!!"); \
    return; \
  }

#define PHY_TIMER_LOCK() \
  ScopedReadWriteLock timerLock(PhysicalLayer::get_timer_lock_(), \
        PFC_TRUE);

#define PHY_SQLEXEC_LOCK() \
  ScopedReadWriteLock sqlexecLock(PhysicalLayer::get_phy_sqlexec_lock_(), \
        PFC_TRUE);

#define OPEN_DB_CONNECTION_TC_REQUEST(conn_type) \
  UncRespCode db_ret = UNC_RC_SUCCESS; \
  /* Create a new odbcm connection */ \
  OdbcmConnectionHandler db_conn((conn_type), db_ret, \
      PhysicalLayer::get_instance()->get_odbc_manager()); \
  if (db_ret != UNC_RC_SUCCESS) { \
    /* Error in opening db connection */ \
    pfc_log_error("Error in opening DB connection"); \
    TcLibModule* tclib_ptr = static_cast<TcLibModule*> \
                (TcLibModule::getInstance(TCLIB_MODULE_NAME)); \
    tclib_ptr->TcLibWriteControllerInfo("", UNC_RC_INTERNAL_ERR, 0); \
    return unc::tclib::TC_FAILURE; \
  }

/* Create a new odbcm connection */
#define OPEN_DB_CONNECTION_WITH_POOL(conn_type, \
                   db_ret, db_conn, session_id, config_id) \
  ScopedDBConnection  scope_dbconn((conn_type), (db_ret), \
                     (db_conn), (session_id), (config_id), \
                     physical_layer->get_odbc_manager()); \
  if ((db_ret) != UNC_RC_SUCCESS) { \
    physical_layer->get_odbc_manager()->\
                 FreeingConnections(false/*Unused conn free*/); \
    PhysicalCore* physical_core = physical_layer->get_physical_core();\
    if (physical_core->system_transit_state_ == true) { \
      pfc_log_warn("odbc connection assignation is failed !!"); \
    } else { \
      pfc_log_error("odbc connection assignation is failed !!"); \
    } \
  }

#define OPEN_DB_CONNECTION(conn_type, db_ret) \
  OdbcmConnectionHandler db_conn((conn_type), (db_ret), \
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
  UncRespCode InitializePhysicalSubModules();
  UncRespCode FinalizePhysicalSubModules();
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
  static ReadWriteLock* get_phy_dbsbcxn_lock_() {
    return &phy_dbsbcxn_lock_;
  }
  static ReadWriteLock* get_events_done_lock_() {
    return &events_done_lock_;
  }
  static ReadWriteLock* get_timer_lock_() {
    return &timer_lock_;
  }
  static ReadWriteLock* get_phy_sqlexec_lock_() {
    return &phy_sqlexec_lock_;
  }

  static Mutex physical_layer_mutex_;
  static Mutex physical_core_mutex_;
  static Mutex ipc_client_config_hdlr_mutex_;
  static Mutex ipc_client_logical_hdlr_mutex_;
  static Mutex ipc_server_hdlr_mutex_;
  static Mutex notification_manager_mutex_;
  static Mutex ODBCManager_mutex_;
  static Mutex db_conpool_mutex_;
  static ReadWriteLock phy_fini_db_lock_;
  static ReadWriteLock phy_fini_phycore_lock_;
  static ReadWriteLock phy_fini_event_lock_;
  static ReadWriteLock phy_dbsbcxn_lock_;
  static ReadWriteLock events_done_lock_;
  static ReadWriteLock timer_lock_;
  static ReadWriteLock phy_sqlexec_lock_;
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
