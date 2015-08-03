/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_DBCONN_MGR_HH_
#define UPLL_DBCONN_MGR_HH_

#include <list>

#include "cxx/pfcxx/synch.hh"

#include "unc/upll_errno.h"
#include "uncxx/upll_log.hh"

#include "dal/dal_odbc_mgr.hh"

namespace unc {
namespace upll {
namespace config_momgr {

using unc::upll::dal::DalOdbcMgr;
namespace uudal = unc::upll::dal;

class UpllDbConnMgr {
 public:
  explicit UpllDbConnMgr(size_t max_ro_conns) : ro_conn_sem_(max_ro_conns) {
    config_rw_conn_ = alarm_rw_conn_ = audit_rw_conn_ = NULL;
    max_ro_conns_ = max_ro_conns;
    active_ro_conns_cnt_= 0;
  }
  void TerminateAndInitializeDbConns(bool active);
  // GetConfigRwConn() should be called after InitializeDbConnections()
  // It cannot be called after terminating all connections.
  DalOdbcMgr *GetConfigRwConn();
  // GetAlarmRwConn() should be called after InitializeDbConnections()
  // It cannot be called after terminating all connections.
  DalOdbcMgr *GetAlarmRwConn();
  // GetAuditRwConn() should be called after InitializeDbConnections()
  // It cannot be called after terminating all connections.
  DalOdbcMgr *GetAuditRwConn();
  void ReleaseRwConn(DalOdbcMgr *dom);

  inline size_t get_ro_conn_limit() const { return max_ro_conns_; }
  upll_rc_t AcquireRoConn(DalOdbcMgr **dom);
  upll_rc_t ReleaseRoConn(DalOdbcMgr *dom);
  // void DestroyRoConns();
  upll_rc_t DalOpen(DalOdbcMgr *dom, bool read_write_conn);
  upll_rc_t DalTxClose(DalOdbcMgr *dom, bool commit);

  static upll_rc_t ConvertDalResultCode(uudal::DalResultCode drc);
  inline void ConvertConnInfoToStr() const;

 private:
  enum  DbConnName {
    kConfigRwConn = 0,
    kAlarmRwConn,
    kAuditRwConn,
    kRoConn
  };
  class DbConn {
   public:
    explicit DbConn(DbConnName conn_name) {
      in_use_cnt = 0;
      close_on_finish = false;
      conn_name_ = conn_name;
    }
    // DAL object instance which manages the ODBC connection
    DalOdbcMgr dom;
    uint32_t in_use_cnt;    // If >0, connection is allocated
    DbConnName conn_name_;
    // If true, the connection is closed after the connection is returned.
    bool close_on_finish;
  };
  DbConn* config_rw_conn_;  // shared connection
  DbConn* alarm_rw_conn_;   // shared connection
  DbConn* audit_rw_conn_;

  // alarm connection might be used by multiple threads. To allow one connection
  // at a time alarm_rw_conn_mutex_ is used.
  pfc::core::Mutex alarm_rw_conn_mutex_;

  size_t max_ro_conns_;
  size_t active_ro_conns_cnt_;
  std::list<DbConn*> ro_conn_pool_;  // not shared connection
  // stale_rw_conn_pool_: rw connections that need to be closed and destroyed
  std::list<DbConn*> stale_rw_conn_pool_;
  pfc::core::Mutex conn_mutex_;
  pfc::core::Semaphore ro_conn_sem_;

  upll_rc_t InitializeDbConnectionsNoLock();
  upll_rc_t TerminateAllDbConnsNoLock();
  upll_rc_t TerminateDbConn(DbConn *dbc);
  upll_rc_t TerminateAllRoConns_NoLock();
};

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc


#endif  // UPLL_DBCONN_MGR_HH_

