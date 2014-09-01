/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Transaction Request
 * @file     itc_transaction_request.hh
 *
 **/

#ifndef _ITC_TRANSACTION_REQUEST_HH_
#define _ITC_TRANSACTION_REQUEST_HH_

#include <unc/keytype.h>
#include <uncxx/tclib/tclib_defs.hh>
#include <stdlib.h>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <string>
#include "phy_util.hh"
#include "itc_kt_base.hh"
#include "physical_itc_req.hh"
#include "odbcm_connection.hh"

using std::vector;
using std::set;
using std::map;
using unc::tclib::TcDriverInfoMap;
using unc::tclib::TcCommitPhaseType;
using unc::tclib::TcCommitPhaseResult;
using unc::tclib::TcCommitOpAbortPhase;
using unc::tclib::TcTransEndResult;

namespace unc {
namespace uppl {


class TransactionRequest : public ITCReq {
  private:
    vector<key_ctr> controller_created;
    vector<key_ctr> controller_updated;
    vector<key_ctr> controller_deleted;
    map<string, uint32_t> controller_type_map;
    map<string, val_ctr_commit_ver_t> controller_old_upd_val;
    map<string, val_ctr_commit_ver_t> controller_old_del_val;
    vector<key_boundary> boundary_created;
    vector<key_boundary> boundary_updated;
    vector<key_boundary> boundary_deleted;
    vector<key_ctr_domain> domain_created;
    vector<key_ctr_domain> domain_updated;
    vector<key_ctr_domain> domain_deleted;
    UncRespCode SendControllerNotification(OdbcmConnectionHandler *db_conn,
                                              vector<void *> vec_old_val_ctr);
    UncRespCode SendDomainNotification(
        OdbcmConnectionHandler *db_conn,
        vector<void *> vec_old_val_ctr_domain);
    UncRespCode SendBoundaryNotification(
        OdbcmConnectionHandler *db_conn,
        vector<void *> vec_old_val_boundary);
    void SendControllerInfo(OdbcmConnectionHandler *db_conn,
                            uint32_t operation_type,
                            uint32_t session_id,
                            uint32_t config_id,
                            bool audit_flag);
    UncRespCode GetModifiedConfiguration(OdbcmConnectionHandler *db_conn,
                                            CsRowStatus row_status);
    void ClearMaps();
    UncRespCode GetModifiedController(OdbcmConnectionHandler *db_conn,
                                         CsRowStatus row_status);
    UncRespCode GetModifiedDomain(OdbcmConnectionHandler *db_conn,
                                     CsRowStatus row_status);
    UncRespCode GetModifiedBoundary(OdbcmConnectionHandler *db_conn,
                                       CsRowStatus row_status);

  public:
    TransactionRequest();
    ~TransactionRequest();
    UncRespCode StartTransaction(OdbcmConnectionHandler *db_conn,
                                    uint32_t session_id, uint32_t config_id);
    UncRespCode HandleVoteRequest(uint32_t session_id, uint32_t config_id,
                                 TcDriverInfoMap &driver_info);
    UncRespCode HandleDriverResult(OdbcmConnectionHandler *db_conn,
                                      uint32_t session_id,
                                      uint32_t config_id,
                                      TcCommitPhaseType phase,
                                      TcCommitPhaseResult
                                      driver_result);
    UncRespCode HandleGlobalCommitRequest(uint32_t session_id,
                                   uint32_t config_id,
                                   TcDriverInfoMap &driver_info);
    UncRespCode HandleDriverGlobalCommitResult(uint32_t session_id,
                                      uint32_t config_id,
                                      TcCommitPhaseType
                                      commitphase,
                                      TcCommitPhaseResult driver_result);

    UncRespCode AbortTransaction(uint32_t session_id,
                                uint32_t config_id,
                                TcCommitOpAbortPhase operation_phase);
    UncRespCode EndTransaction(OdbcmConnectionHandler *db_conn,
                                  uint32_t session_id,
                                  uint32_t config_id,
                                  TcTransEndResult trans_res,
                                  bool audit_flag = true);
};
}   //   namespace uppl
}   //   namespace unc

#endif   //   _ITC_TRANSACTION_REQUEST_HH_
