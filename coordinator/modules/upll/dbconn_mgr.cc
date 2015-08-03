/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * config_mgr.cc - UPLL Config Manager
 */

// #include <iostream>
#include <sstream>
#include "upll_util.hh"
#include "dbconn_mgr.hh"

namespace unc {
namespace upll {
namespace config_momgr {

using unc::upll::dal::DalOdbcMgr;
namespace uudal = unc::upll::dal;
// using namespace unc::upll::upll_util;

// Error translation given below apply for Connect/Disconnect/Commit/Rollback
// Operations only.
upll_rc_t UpllDbConnMgr::ConvertDalResultCode(uudal::DalResultCode drc) {
  switch (drc) {
    case uudal::kDalRcSuccess:
      return UPLL_RC_SUCCESS;
    case uudal::kDalRcConnNotEstablished:
    case uudal::kDalRcConnNotAvailable:
    case uudal::kDalRcConnTimeOut:
    case uudal::kDalRcQueryTimeOut:
      return UPLL_RC_ERR_RESOURCE_DISCONNECTED;
    case uudal::kDalRcTxnError:
    case uudal::kDalRcAccessViolation:
      return UPLL_RC_ERR_DB_ACCESS;
    case uudal::kDalRcNotDisconnected:
    case uudal::kDalRcInvalidConnHandle:
    case uudal::kDalRcInvalidCursor:
    case uudal::kDalRcDataError:
    case uudal::kDalRcRecordAlreadyExists:
    case uudal::kDalRcParentNotFound:
    case uudal::kDalRcRecordNotFound:
    case uudal::kDalRcRecordNoMore:
    case uudal::kDalRcMemoryError:
    case uudal::kDalRcInternalError:
    case uudal::kDalRcGeneralError:
    // default:
      return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_ERR_GENERIC;
}


void UpllDbConnMgr::ConvertConnInfoToStr() const {
#ifdef PFC_VERBOSE_DEBUG
  std::stringstream ss;
  ss << " Max No. Of Connections:" << max_ro_conns_
     << " No. Of Connections In Use:" << active_ro_conns_cnt_
     << " No. Of Free Connections:" << (max_ro_conns_ - active_ro_conns_cnt_);
  UPLL_LOG_DEBUG("DbConn: %s", ss.str().c_str());
#endif
}

upll_rc_t UpllDbConnMgr::DalOpen(DalOdbcMgr *dom, bool read_write_conn) {
  UPLL_FUNC_TRACE;

  uudal::DalResultCode drc;

  uudal::DalConnType conn_type = ((read_write_conn) ? uudal::kDalConnReadWrite :
                                  uudal::kDalConnReadOnly);
  // Initialize
  drc = dom->Init();
  if (drc != uudal::kDalRcSuccess) {
    if (read_write_conn) {
      UPLL_LOG_FATAL("Failed to initialize DalOdbcMgr. Err=%d", drc);
    } else {
      UPLL_LOG_ERROR("Failed to initialize DalOdbcMgr. Err=%d", drc);
    }
    return ConvertDalResultCode(drc);
  } else {
    UPLL_LOG_TRACE("DalOdbcMgr init successful.");
  }

  // Connect to DB
  drc = dom->ConnectToDb(conn_type);
  if (drc != uudal::kDalRcSuccess) {
    if (read_write_conn) {
      UPLL_LOG_FATAL("Failed to connect to database. Err=%d", drc);
    } else {
      UPLL_LOG_ERROR("Failed to connect to database. Err=%d", drc);
    }
    return ConvertDalResultCode(drc);
  } else {
    UPLL_LOG_TRACE("Connected to database.");
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllDbConnMgr::DalTxClose(DalOdbcMgr *dom, bool commit) {
  UPLL_FUNC_TRACE;

  uudal::DalResultCode drc;
  upll_rc_t urc = UPLL_RC_SUCCESS;

  // Commit or Rollback is required only for read-write connections
  if ((dom->get_conn_type() == uudal::kDalConnReadWrite) && (commit)) {
    // Commit the transaction
    drc = dom->CommitTransaction();
    if (drc != uudal::kDalRcSuccess) {
      UPLL_LOG_ERROR("Failed to commit DB transaction. Err=%d", drc);
      urc = ConvertDalResultCode(drc);
    } else {
      UPLL_LOG_TRACE("Committed the DB transaction.");
      urc = UPLL_RC_SUCCESS;
    }
  } else {
    // Rollback the transaction
    drc = dom->RollbackTransaction();
    if (drc != uudal::kDalRcSuccess) {
      UPLL_LOG_ERROR("Failed to rollback DB transaction. Err=%d", drc);
      urc = ConvertDalResultCode(drc);
    } else {
      UPLL_LOG_TRACE("Rolledback the DB transaction.");
      urc = UPLL_RC_SUCCESS;
    }
  }

  return urc;
}

// GetConfigRwConn() should be called only after InitializeDbConnections()
// It cannot be called after terminating all connections
DalOdbcMgr *UpllDbConnMgr::GetConfigRwConn() {
  pfc::core::ScopedMutex lock(conn_mutex_);
  if (config_rw_conn_ == NULL) {
    return NULL;
  }
  config_rw_conn_->in_use_cnt++;
  if (config_rw_conn_->in_use_cnt > 1) {
    UPLL_LOG_INFO("Config connection shared %d times",
                  config_rw_conn_->in_use_cnt);
  }
  return &config_rw_conn_->dom;
}

// GetAlarmRwConn() should be called only after InitializeDbConnections()
// It cannot be called after terminating all connections
DalOdbcMgr *UpllDbConnMgr::GetAlarmRwConn() {
  // Allow only one thread to get alarm connection at a time
  alarm_rw_conn_mutex_.lock();
  UPLL_LOG_TRACE("Acquired alarm_rw_conn_mutex_ lock");
  pfc::core::ScopedMutex lock(conn_mutex_);
  if (alarm_rw_conn_ == NULL) {
    alarm_rw_conn_mutex_.unlock();
    return NULL;
  }
  alarm_rw_conn_->in_use_cnt++;
  if (alarm_rw_conn_->in_use_cnt > 1) {
    UPLL_LOG_TRACE("Alarm connection shared %d times",
                   alarm_rw_conn_->in_use_cnt);
  }
  return &alarm_rw_conn_->dom;
}

// GetAuditRwConn() should be called only after InitializeDbConnections()
// It cannot be called after terminating all connections
DalOdbcMgr *UpllDbConnMgr::GetAuditRwConn() {
  pfc::core::ScopedMutex lock(conn_mutex_);
  if (audit_rw_conn_ == NULL) {
    return NULL;
  }
  audit_rw_conn_->in_use_cnt++;
  if (audit_rw_conn_->in_use_cnt > 1) {
    UPLL_LOG_INFO("Audit connection shared %d times",
                  audit_rw_conn_->in_use_cnt);
  }
  return &audit_rw_conn_->dom;
}

void UpllDbConnMgr::ReleaseRwConn(DalOdbcMgr *dom) {
  UPLL_FUNC_TRACE;
  pfc::core::ScopedMutex lock(conn_mutex_);
  DbConn *dbc = NULL;
  if ((config_rw_conn_ != NULL) && (&config_rw_conn_->dom == dom)) {
    dbc = config_rw_conn_;
  } else if ((alarm_rw_conn_ != NULL) && (&alarm_rw_conn_->dom == dom)) {
    dbc = alarm_rw_conn_;
  } else if ((audit_rw_conn_ != NULL) && (&audit_rw_conn_->dom == dom)) {
    dbc = audit_rw_conn_;
  }
  if (dbc != NULL) {
    if (dbc->in_use_cnt == 0) {  // Should not happen
      UPLL_LOG_INFO("Error: RW connection is not acquired, but released");
      ConvertConnInfoToStr();
      return;
    }
    dbc->in_use_cnt--;
    if (&alarm_rw_conn_->dom == dom) {
      UPLL_LOG_TRACE("Release alarm_rw_conn_mutex_ lock");
      alarm_rw_conn_mutex_.unlock();
    }
    if (dom->get_conn_state() == uudal::kDalDbDisconnected) {
       UPLL_LOG_FATAL("RW connection Failure.");
    }
  } else {
    for (std::list<DbConn*>::iterator iter = stale_rw_conn_pool_.begin();
         iter != stale_rw_conn_pool_.end(); iter++) {
      dbc = *iter;
      if (dom == &dbc->dom) {
        if (dbc->in_use_cnt == 0) {  // Should not happen
          UPLL_LOG_INFO("Error: RW connection is not acquired, but released");
          return;
        }
        if (dom->get_conn_state() == uudal::kDalDbDisconnected) {
          UPLL_LOG_FATAL("Stale RW connection Failure.");
        }
        dbc->in_use_cnt--;
        if (dbc->conn_name_ == kAlarmRwConn) {
          UPLL_LOG_INFO("Release alarm_rw_conn_mutex_ lock");
          alarm_rw_conn_mutex_.unlock();
        }
        if (dbc->in_use_cnt == 0) {
          TerminateDbConn(dbc);
          delete dbc;
          stale_rw_conn_pool_.erase(iter);
        }
        ConvertConnInfoToStr();
        return;
      }
    }
    UPLL_LOG_INFO("RW connection not found");
  }
  ConvertConnInfoToStr();
}

upll_rc_t UpllDbConnMgr::InitializeDbConnectionsNoLock() {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;

  UPLL_LOG_INFO("Creating config db conn");
  config_rw_conn_ = new DbConn(kConfigRwConn);
  if (UPLL_RC_SUCCESS != (urc = DalOpen(&config_rw_conn_->dom, true))) {
    // return urc;  // Other dom object needs to be created.
  }
  UPLL_LOG_INFO("Creating alarm db conn");
  alarm_rw_conn_ = new DbConn(kAlarmRwConn);
  alarm_rw_conn_->dom.set_wr_exclusion_on_runn();
  if (UPLL_RC_SUCCESS != (urc = DalOpen(&alarm_rw_conn_->dom, true))) {
    // return urc;  // Other dom object needs to be created.
  }
  UPLL_LOG_INFO("Creating audit db conn");
  audit_rw_conn_ = new DbConn(kAuditRwConn);
  audit_rw_conn_->dom.set_wr_exclusion_on_runn();
  if (UPLL_RC_SUCCESS != (urc = DalOpen(&audit_rw_conn_->dom, true))) {
    return urc;
  }

  ConvertConnInfoToStr();
  return urc;
}

upll_rc_t UpllDbConnMgr::TerminateDbConn(DbConn *dbc) {
  UPLL_FUNC_TRACE
  if (dbc->in_use_cnt > 0) {
    dbc->close_on_finish = true;
    UPLL_LOG_DEBUG("Connection is in use (%d), setting close_on_finish",
                   dbc->in_use_cnt);
  } else {
    uudal::DalResultCode drc = dbc->dom.DisconnectFromDb();
    if (drc != uudal::kDalRcSuccess) {
      UPLL_LOG_ERROR("Failed to disconnect from database. Err=%d", drc);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllDbConnMgr::TerminateAllDbConnsNoLock() {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Any exising DB connections are being closed");
  upll_rc_t urc = UPLL_RC_SUCCESS;
  if (config_rw_conn_!= NULL) {
    TerminateDbConn(config_rw_conn_);
    if (config_rw_conn_->close_on_finish) {
      UPLL_LOG_INFO("config_rw_conn_ is in use, putting on the stale list");
      stale_rw_conn_pool_.push_back(config_rw_conn_);
    } else {
      delete config_rw_conn_;
      UPLL_LOG_INFO("config_rw_conn_ is deleted.");
    }
    config_rw_conn_ = NULL;
  }
  if (alarm_rw_conn_!= NULL) {
    TerminateDbConn(alarm_rw_conn_);
    if (alarm_rw_conn_->close_on_finish) {
      UPLL_LOG_INFO("alarm_rw_conn_ is in use, putting on the stale list");
      stale_rw_conn_pool_.push_back(alarm_rw_conn_);
    } else {
      delete alarm_rw_conn_;
      UPLL_LOG_INFO("alarm_rw_conn_ is deleted");
    }
    alarm_rw_conn_ = NULL;
  }
  if (audit_rw_conn_!= NULL) {
    TerminateDbConn(audit_rw_conn_);
    if (audit_rw_conn_->close_on_finish) {
      UPLL_LOG_INFO("audit_rw_conn_ is in use, putting on the stale list");
      stale_rw_conn_pool_.push_back(audit_rw_conn_);
    } else {
      delete audit_rw_conn_;
      UPLL_LOG_INFO("audit_rw_conn_ is deleted");
    }
    audit_rw_conn_ = NULL;
  }

  TerminateAllRoConns_NoLock();
  ConvertConnInfoToStr();
  return urc;
}

void UpllDbConnMgr::TerminateAndInitializeDbConns(bool active) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG(
      "All DB connections are being closed and initialized "
      "based on cluster state");
  pfc::core::ScopedMutex lock(conn_mutex_);
  TerminateAllDbConnsNoLock();
  if (active) {
    InitializeDbConnectionsNoLock();
  }
}

upll_rc_t UpllDbConnMgr::TerminateAllRoConns_NoLock() {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("All DB RO connections are being closed");
  std::list<DbConn*>::iterator iter = ro_conn_pool_.begin();
  while (iter != ro_conn_pool_.end()) {
    TerminateDbConn(*iter);
    if ((*iter)->close_on_finish == true) {
      iter++;
    } else {
      std::list<DbConn*>::iterator tmp_iter = iter;
      iter++;
      DbConn *dbc = *tmp_iter;
      delete dbc;
      ro_conn_pool_.erase(tmp_iter);
      active_ro_conns_cnt_--;
      ro_conn_sem_.post();
    }
  }
  ConvertConnInfoToStr();
  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllDbConnMgr::AcquireRoConn(DalOdbcMgr **dom) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Acquiring RO Connection");
  ro_conn_sem_.wait();
  pfc::core::ScopedMutex lock(conn_mutex_);

  // First check if there is a not-in-use connection available
  for (std::list<DbConn*>::iterator iter = ro_conn_pool_.begin();
       iter != ro_conn_pool_.end(); iter++) {
    if ((*iter)->in_use_cnt == 0) {
      (*iter)->in_use_cnt = 1;
      *dom = &(*iter)->dom;
      return UPLL_RC_SUCCESS;
    }
  }
  if (active_ro_conns_cnt_ >= max_ro_conns_) {
    UPLL_LOG_INFO("Error: Could not find a not-in-use connection");
    return UPLL_RC_ERR_GENERIC;
  }

  // Create a new connection
  upll_rc_t urc = UPLL_RC_SUCCESS;
  DbConn *ro_conn = new DbConn(kRoConn);
  if (UPLL_RC_SUCCESS != (urc = DalOpen(&ro_conn->dom, false))) {
    delete ro_conn;
    TerminateAllRoConns_NoLock();
    return urc;
  }

  ro_conn_pool_.push_back(ro_conn);
  active_ro_conns_cnt_++;

  ro_conn->in_use_cnt = 1;
  *dom = &ro_conn->dom;
  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllDbConnMgr::ReleaseRoConn(DalOdbcMgr *dom) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Releasing RO Connection");
  pfc::core::ScopedMutex lock(conn_mutex_);
  ConvertConnInfoToStr();
  for (std::list<DbConn*>::iterator iter = ro_conn_pool_.begin();
       iter != ro_conn_pool_.end(); iter++) {
    if (&(*iter)->dom == dom) {
      (*iter)->in_use_cnt = 0;
      if ((*iter)->close_on_finish == true) {
        TerminateDbConn(*iter);
        DbConn *dbc = *iter;
        delete dbc;
        ro_conn_pool_.erase(iter);
        active_ro_conns_cnt_--;
        ro_conn_sem_.post();
      } else {
        // If dom had encountered connection error, close all RO connections
        if (dom->get_conn_state() == uudal::kDalDbDisconnected) {
          UPLL_LOG_TRACE("DB RO connection error, closing all RO connections");
          TerminateAllRoConns_NoLock();
        } else {
          ro_conn_sem_.post();
        }
      }
      ConvertConnInfoToStr();
      return UPLL_RC_SUCCESS;
    }
  }
  ConvertConnInfoToStr();
  UPLL_LOG_INFO("Error: connection not found");
  return UPLL_RC_ERR_GENERIC;
}
                                                                       // NOLINT
}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
