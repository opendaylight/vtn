/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_TEST_TC_TCOPERATIONS_H_
#define UNC_TEST_TC_TCOPERATIONS_H_

#include <stdio.h>
#include <stdarg.h>
#include <gtest/gtest.h>
#include <string>
#include <utility>
#include "tc_operations.hh"
#include "tc_db_handler.hh"

using namespace std;
using namespace unc::tc;
using namespace unc::tclib;

#define SET 1
#define CLEAR 0

#define SET_AUDIT_OPER_PARAMS()               \
    TcLock* tc_lock_ = new TcLock();          \
pfc_ipcsrv_t *srv = NULL;                 \
pfc::core::ipc::ServerSession sess_(srv); \
std::string dsn_name = "UNC_DB_DSN";      \
TcDbHandler* db_handler = new TcDbHandler(dsn_name);\
TcChannelNameMap  unc_map_;               \
TcTaskqUtil* audit_ = new TcTaskqUtil(TC_AUDIT_CONCURRENCY);

#define DEL_AUDIT_PARAMS() \
    delete tc_lock_; \
tc_lock_ =NULL ; \
delete audit_; \
audit_ = NULL;

int stub_session_invoke_oper = CLEAR;
int stub_create_session_oper = CLEAR;
int stub_response_oper = CLEAR;
int stub_set_arg_oper = CLEAR;
int stub_clnt_forward_oper = CLEAR;
int stub_same_driverid_oper = CLEAR;

TcChannelNameMap GetTcChannelNameMap(int SetChannelName) {
  TcChannelNameMap test_map;
  if (SetChannelName) {
    test_map.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
    test_map.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
    test_map.insert((std::pair<TcDaemonName,
                     std::string>(TC_DRV_OPENFLOW, "drvpfcd")));
    test_map.insert((std::pair<TcDaemonName,
                     std::string>(TC_DRV_OVERLAY, "drvoverlay")));
  } else {
    test_map.insert((std::pair<TcDaemonName, std::string>(TC_UPLL, "")));
  }
  return test_map;
}


class TestTcAuditOperations : public TcAuditOperations {
 public:
  TestTcAuditOperations(TcLock* tclock,
                        pfc::core::ipc::ServerSession* sess,
                        TcDbHandler* tc_db_,
                        TcChannelNameMap& unc_map_,
                        TcTaskqUtil* audit_)
      :TcAuditOperations(tclock, sess, tc_db_, unc_map_, audit_) {}

  uint32_t TestTcGetMinArgCount() {
    return TcGetMinArgCount();
  }

  TcOperStatus TestHandleArgs() {
    return HandleArgs();
  }

  TcOperStatus TestTcCheckOperArgCount(uint32_t avail_count) {
    return TcCheckOperArgCount(avail_count);
  }
  TcOperStatus TestGetSessionId() {
    return GetSessionId();
  }
  TcOperStatus TestTcValidateOperType() {
    return TcValidateOperType();
  }

  TcOperStatus TestTcValidateOperParams() {
    return TcValidateOperParams();
  }

  TcOperStatus TestTcGetExclusion() {
    return TcGetExclusion();
  }

  TcOperStatus TestTcReleaseExclusion() {
    return TcReleaseExclusion();
  }

  TcOperStatus TestHandleLockRet(TcLockRet ret) {
    return HandleLockRet(ret);
  }

  TcOperStatus TestTcCreateMsgList() {
    return TcCreateMsgList();
  }

  TcOperStatus TestFillTcMsgData(TcMsg* tc_msg,
                                 unc::tclib::TcMsgOperType oper_type) {
    return FillTcMsgData(tc_msg, oper_type);
  }
  pfc_bool_t TestGetDriverType() {
    return GetDriverType();
  }
};

class TestTcLock : public TcLock {
 public:
  TestTcLock():TcLock() {}
  void ResetTcGlobalDataOnStateTransition(void);
  TcLockRet GetLock(uint32_t session_id,  TcOperation operation,
                    TcWriteOperation write_operation) {
    return TC_LOCK_SUCCESS;
  }
  TcLockRet ReleaseLock(uint32_t session_id,  uint32_t config_id,
                        TcOperation operation,
                        TcWriteOperation write_operation) {
    return TC_LOCK_SUCCESS;
  }
};


class TestTcMsg : public TcMsg {
  TcDaemonName TestMapTcDriverId(unc_keytype_ctrtype_t driver_id) {
    return MapTcDriverId(driver_id);
  }

  TcOperRet TestRespondToTc(pfc_ipcresp_t resp) {
    return RespondToTc(resp);
  }

  TcOperRet TestReturnUtilResp(TcUtilRet ret) {
    return ReturnUtilResp(ret);
  }

  void ClearAbortOnFailVector() {
    abort_on_fail_.clear();
  }
  unc_keytype_ctrtype_t  TestGetResult() {
    return GetResult();
  }
};
#endif
