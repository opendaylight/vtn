/*
 * Copyright (c) 2013-2014 NEC Corporation
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
TcChannelNameMap  unc_map_;

#define DEL_AUDIT_PARAMS() \
    delete tc_lock_; \
tc_lock_ =NULL;


/*class to test TcConfigOperations*/
class TestTcConfigOperations : public TcConfigOperations {
 public:
  TestTcConfigOperations(TcLock* tc_lock_,
                         pfc::core::ipc::ServerSession* sess_,
                         TcDbHandler* db_handler,
                         TcChannelNameMap& unc_map_)
      :TcConfigOperations(tc_lock_, sess_, db_handler, unc_map_) {}

  TcOperStatus TestSetConfigId() {
    return SetConfigId();
  }

  uint32_t TestTcGetMinArgCount() {
    return TcGetMinArgCount();
  }

  TcOperStatus TestTcCheckOperArgCount(uint32_t avail_count) {
    return TcCheckOperArgCount(avail_count);
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

  TcOperStatus TestSendAdditionalResponse(TcOperStatus oper_stat) {
    return SendAdditionalResponse(oper_stat);
  }
};

class TestTcLock : public TcLock {
 public:
  TestTcLock():TcLock() {}
  void ResetTcGlobalDataOnStateTransition(void);
  TcLockRet GetLock(uint32_t session_id, TcOperation operation,
                    TcWriteOperation write_operation) {
    return TC_LOCK_SUCCESS;
  }
  TcLockRet ReleaseLock(uint32_t session_id, uint32_t config_id,
                        TcOperation operation,
                        TcWriteOperation write_operation) {
    return TC_LOCK_SUCCESS;
  }
};

#endif
