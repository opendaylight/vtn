#ifndef _ITC_TRANSACTION_REQUEST_HH_
#define _ITC_TRANSACTION_REQUEST_HH_

/*
 * Copyright (c) 2012-2013 NEC Corporation
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
    vector<key_boundary> boundary_created;
    vector<key_boundary> boundary_updated;
    vector<key_boundary> boundary_deleted;
    vector<key_ctr_domain> domain_created;
    vector<key_ctr_domain> domain_updated;
    vector<key_ctr_domain> domain_deleted;
    set<string>set_controller_oflow;
    set<string>set_controller_vnp;
    map<unc_keytype_ctrtype_t, vector<string> > driver_controller_info_map_;
    UpplReturnCode SendControllerNotification(OdbcmConnectionHandler *db_conn,
                                              vector<void *> vec_old_val_ctr);
    UpplReturnCode SendDomainNotification(
        OdbcmConnectionHandler *db_conn,
        vector<void *> vec_old_val_ctr_domain);
    UpplReturnCode SendBoundaryNotification(
        OdbcmConnectionHandler *db_conn,
        vector<void *> vec_old_val_boundary);
    void SendControllerInfo(OdbcmConnectionHandler *db_conn,
                            uint32_t operation_type,
                            uint32_t session_id,
                            uint32_t config_id);
    UpplReturnCode GetModifiedConfiguration(OdbcmConnectionHandler *db_conn,
                                            CsRowStatus row_status);
    void ClearMaps();
    UpplReturnCode GetModifiedController(OdbcmConnectionHandler *db_conn,
                                         CsRowStatus row_status);
    UpplReturnCode GetModifiedDomain(OdbcmConnectionHandler *db_conn,
                                     CsRowStatus row_status);
    UpplReturnCode GetModifiedBoundary(OdbcmConnectionHandler *db_conn,
                                       CsRowStatus row_status);

  public:
    TransactionRequest();
    ~TransactionRequest();
    UpplReturnCode StartTransaction(OdbcmConnectionHandler *db_conn,
                                    uint32_t session_id, uint32_t config_id);
    UpplReturnCode HandleVoteRequest(uint32_t session_id, uint32_t config_id,
                                 TcDriverInfoMap &driver_info);
    UpplReturnCode HandleDriverResult(OdbcmConnectionHandler *db_conn,
                                      uint32_t session_id,
                                      uint32_t config_id,
                                      TcCommitPhaseType phase,
                                      TcCommitPhaseResult
                                      driver_result);
    UpplReturnCode HandleGlobalCommitRequest(uint32_t session_id,
                                   uint32_t config_id,
                                   TcDriverInfoMap &driver_info);
    UpplReturnCode HandleDriverGlobalCommitResult(uint32_t session_id,
                                      uint32_t config_id,
                                      TcCommitPhaseType
                                      commitphase,
                                      TcCommitPhaseResult driver_result);

    UpplReturnCode AbortTransaction(uint32_t session_id,
                                uint32_t config_id,
                                TcCommitOpAbortPhase operation_phase);
    UpplReturnCode EndTransaction(OdbcmConnectionHandler *db_conn,
                                  uint32_t session_id,
                                  uint32_t config_id,
                                  TcTransEndResult trans_res);
};
}   //   namespace uppl
}   //   namespace unc

#endif   //   _ITC_TRANSACTION_REQUEST_HH_
