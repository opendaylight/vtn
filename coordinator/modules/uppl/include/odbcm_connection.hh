/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * @brief   ODBC Manager
 * @file    odbcm_connection.hh
 */

#ifndef _ODBCM_DB_CONNECTION_H_
#define _ODBCM_DB_CONNECTION_H_

#include "odbcm_common.hh"
#include "odbcm_mgr.hh"
#include "pfcxx/thread.hh"
using unc::uppl::ODBCManager;
namespace unc {
namespace uppl {

enum OdbcmConnType {
  kOdbcmConnReadOnly = 0,  // Read Only Connection
  kOdbcmConnReadWriteNb,   // Read Write Connection For Nb requests
  kOdbcmConnReadWriteSb    // Read Write Connection For Sb events
};


class OdbcmConnectionHandler {
  public:
    /*
    * Constructor for OdbcmConnectionHandler
    */
    explicit OdbcmConnectionHandler(const OdbcmConnType conn_type,
                                    UncRespCode &conn_status,
                                    ODBCManager *odbc_manager):
                                    conn_type_(conn_type),
                                    conn_handle_(NULL),
                                    odbc_manager_(odbc_manager),
                                    using_session_id_(0) {
      conn_status = UNC_RC_SUCCESS;
      ODBCM_RC_STATUS db_ret = odbc_manager_->OpenDBConnection(this);
      if (db_ret != ODBCM_RC_SUCCESS) {
        conn_status = UNC_UPPL_RC_ERR_DB_ACCESS;
      }
    }
    /*
    * Destructor for OdbcmConnectionHandler
    */
    ~OdbcmConnectionHandler(void) {
      ODBCM_RC_STATUS conn_ret = odbc_manager_->CloseDBConnection(this);
      if (conn_ret != ODBCM_RC_SUCCESS) {
        pfc_log_error(
            "ODBCM:OdbcmConnectionHandler:Error in closing DB connection");
      }
    }

    void set_conn_handle(SQLHDBC conn_handle) {
      conn_handle_ = conn_handle;
    }

    void set_using_session_id(uint32_t session_id, uint32_t thread_id) {
      using_session_id_ = session_id;
      using_session_id_ = (using_session_id_ << 32) + thread_id;
      pfc_log_info("using_session_id_ (session_id + thread_id) = %"
                                   PFC_PFMT_u64, using_session_id_);
    }

    uint64_t get_using_session_id() {
      return using_session_id_;
    }

    OdbcmConnType get_conn_type() {
      return conn_type_;
    }

    SQLHDBC get_conn_handle() {
      return conn_handle_;
    }

  private:
    OdbcmConnType conn_type_;
    SQLHDBC conn_handle_;  // Connection handler to create ODBC Connection
    ODBCManager *odbc_manager_;
    uint64_t using_session_id_;
};

class ScopedDBConnection {
  public:
    explicit ScopedDBConnection(const OdbcmConnType conn_type,
                         UncRespCode &resp_code,
                         OdbcmConnectionHandler *&db_conn,
                         uint32_t session_id,
                         uint32_t config_id,
                         ODBCManager *odbc_manager): conn_type_(conn_type),
                                              db_conn_(NULL),
                                              session_id_(session_id),
                                              config_id_(config_id),
                                              odbc_manager_(odbc_manager) {
      resp_code = UNC_RC_SUCCESS;
      ODBCM_RC_STATUS db_status = odbc_manager_->AssignDBConnection(
                                         db_conn_, session_id_, config_id_);
      db_conn = db_conn_;
      if (db_conn == NULL) pfc_log_info("conn is null after assign");
      if (db_status != ODBCM_RC_SUCCESS) {
        resp_code = UNC_UPPL_RC_ERR_DB_ACCESS;
        pfc_log_error("odbc connection assignation is failed!!");
      }
    }
    ~ScopedDBConnection() {
      odbc_manager_->PoolDBConnection(db_conn_, session_id_, config_id_);
    }

  private:
    OdbcmConnType conn_type_;
    OdbcmConnectionHandler *db_conn_;
    uint32_t session_id_;
    uint32_t config_id_;
    ODBCManager *odbc_manager_;
};

}  // namespace uppl
}  // namespace unc
#endif /*_ODBCM_DB_CONNECTION_H_*/
