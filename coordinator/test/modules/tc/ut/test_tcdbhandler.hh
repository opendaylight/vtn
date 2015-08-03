/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_TEST_TC_TCHANDLER_H_
#define UNC_TEST_TC_TCHANDLER_H_


#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include "tc_operations.hh"
#include "tc_db_handler.hh"
#include <gtest/gtest.h>

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
        tc_lock_ =NULL ; \
        delete db_handler; \
        db_handler = NULL;


/*int stub_session_invoke_oper = CLEAR;
int stub_create_session_oper = CLEAR;
int stub_response_oper = CLEAR;
int stub_set_arg_oper = CLEAR;
int stub_clnt_forward_oper = CLEAR;
int stub_same_driverid_oper = CLEAR;
*/

/*TcChannelNameMap GetTcChannelNameMap(int SetChannelName) {

  TcChannelNameMap test_map;
  if(SetChannelName) {
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_UPLL, "lgcnwd")));
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_UPPL, "phynwd")));
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_DRV_OPENFLOW, "drvpfcd")));
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_DRV_OVERLAY, "drvoverlay")));
  }else{
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_UPLL, "")));
  }
  return test_map;
}*/

/*class to test TcDbOperations*/
class TestTcDbHandler : public TcDbHandler {

 public:
 TestTcDbHandler(std::string dsn_name)
                :TcDbHandler(dsn_name) {}

 TcOperRet TestUpdateRecoveryTableGlobalModeDirty(pfc_bool_t global_mode_dirty) {
      return UpdateRecoveryTableGlobalModeDirty(global_mode_dirty);
 }

 TcOperRet TestGetRecoveryTableGlobalModeDirty(pfc_bool_t &global_mode_dirty) {
     return GetRecoveryTableGlobalModeDirty (global_mode_dirty);
 }

};

class TestTcMsg : public TcMsg {
  
  TcDaemonName TestMapTcDriverId(unc_keytype_ctrtype_t driver_id) {
    return MapTcDriverId(driver_id);
  }

  TcOperRet TestRespondToTc(pfc_ipcresp_t resp){
    return RespondToTc(resp);
  }

  TcOperRet TestReturnUtilResp(TcUtilRet ret){
    return ReturnUtilResp(ret);
  }
  
  void ClearAbortOnFailVector(){
    abort_on_fail_.clear();
  }
  unc_keytype_ctrtype_t  TestGetResult() {
    return GetResult();
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
