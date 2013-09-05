/*
 * Copyright (c) 2012-2013 NEC Corporation
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
                                    UpplReturnCode &conn_status,
                                    ODBCManager *odbc_manager) {
      conn_type_ = conn_type;
      odbc_manager_ = odbc_manager;
      conn_handle_ = NULL;
      conn_status = UPPL_RC_SUCCESS;
      ODBCM_RC_STATUS db_ret = odbc_manager_->OpenDBConnection(this);
      if (db_ret != ODBCM_RC_SUCCESS) {
        conn_status = UPPL_RC_ERR_DB_ACCESS;
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
};
}  // namespace uppl
}  // namespace unc
#endif /*_ODBCM_DB_CONNECTION_H_*/
