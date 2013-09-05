/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Audit Request handling
 * @file     itc_audit_request.hh
 *
 */

#ifndef _ITC_AUDIT_REQUEST_HH_
#define _ITC_AUDIT_REQUEST_HH_

#include <unc/keytype.h>
#include <uncxx/tclib/tclib_defs.hh>
#include <vector>
#include <string>
#include <list>
#include <functional>
#include "physical_itc_req.hh"

using unc::tclib::TcDriverInfoMap;
using unc::tclib::TcCommitPhaseType;
using unc::tclib::TcCommitPhaseResult;
using unc::tclib::TcCommitOpAbortPhase;
using unc::tclib::TcAuditResult;
using unc::tclib::TcAuditOpAbortPhase;

#define STATE_OBJECTS 6
typedef enum {
  Notfn_Ctr_Domain = 0,
      Notfn_Logical_Port,
      Notfn_Logical_Member_Port,
      Notfn_Switch,
      Notfn_Port,
      Notfn_Link,
}AuditStateObjects;

namespace unc {
namespace uppl {

class AuditRequest:public ITCReq  {
 public:
  AuditRequest();
  ~AuditRequest();
  UpplReturnCode StartAudit(OdbcmConnectionHandler *db_conn,
                            unc_keytype_ctrtype_t driver_id,
                            string controller_id);
  UpplReturnCode StartAuditTransaction(uint32_t session_id,
                                       unc_keytype_ctrtype_t driver_id,
                                       string controller_id);
  UpplReturnCode HandleAuditGlobalCommit(uint32_t session_id,
                                         uint32_t driver_id,
                                         string controller_id,
                                         TcDriverInfoMap& driver_info,
                                         TcAuditResult& audit_result);
  UpplReturnCode HandleAuditVoteRequest(OdbcmConnectionHandler *db_conn,
                                        uint32_t session_id,
                                        uint32_t driver_id,
                                        string controller_id,
                                        TcDriverInfoMap &driver_info);
  UpplReturnCode HandleAuditDriverResult(uint32_t session_id,
                                         string controller_id,
                                         TcCommitPhaseType commitphase,
                                         TcCommitPhaseResult driver_result,
                                         TcAuditResult& audit_result) {
    audit_result = unc::tclib::TC_AUDIT_SUCCESS;
    return UPPL_RC_SUCCESS;}
  UpplReturnCode HandleAuditAbort(uint32_t session_id,
                                  unc_keytype_ctrtype_t driver_id,
                                  string controller_id,
                                  TcAuditOpAbortPhase operation_phase);
  UpplReturnCode EndAuditTransaction(uint32_t session_id,
                                     unc_keytype_ctrtype_t& driver_id,
                                     string controller_id);
  UpplReturnCode EndAudit(OdbcmConnectionHandler *db_conn,
                          unc_keytype_ctrtype_t driver_id,
                          string controller_id,
                          TcAuditResult audit_result);
  UpplReturnCode MergeAuditDbToRunning(OdbcmConnectionHandler *db_conn,
                                       string controller_name);

 private:
  Kt_Base* GetClassPointerAndKey(AuditStateObjects audit_key_type,
                                 string controller_name, void* &,
                                 uint32_t &key_type);
  void FreeKeyAndValueStruct(void* key_struct,
                             void* value_struct,
                             unsigned int key_type);
  void AddToRunningDbFromImportDb(const vector<void *> &key_import,
                                  const vector<void *> &val_import,
                                  const vector<void *> &key_running,
                                  const vector<void *> &val_running,
                                  unsigned int index,
                                  Kt_Base *class_pointer,
                                  OdbcmConnectionHandler *db_conn,
                                  uint32_t key_type);
  void UpdateRunningDbWithImportDb(const vector<void *> &key_import,
                                   const vector<void *> &val_import,
                                   const vector<void *> &key_running,
                                   const vector<void *> &val_running,
                                   unsigned int index,
                                   Kt_Base *class_pointer,
                                   OdbcmConnectionHandler *db_conn,
                                   uint32_t key_type);
  void FreeKeyAndValueStruct(const vector<void *> &key_import,
                             const vector<void *> &val_import,
                             const vector<void *> &key_running,
                             const vector<void *> &val_running,
                             unsigned int index);
};

class AuditNotification : public std::unary_function < void, void > {
 public:
  string controller_name_;
  explicit AuditNotification() {
  }
  void operator() ();
};

}  // namespace uppl
}  // namespace unc

#endif  //  _ITC_AUDIT_REQUEST_HH_
