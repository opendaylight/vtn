/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_TEST_TC_TCOPERATIONS_H_
#define UNC_TEST_TC_TCOPERATIONS_H_


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

int stub_get_arg = CLEAR;
int stub_get_arg1 = CLEAR;
int stub_get_arg_fail = CLEAR;
//int stub_create_session = CLEAR;
//int stub_set_arg = CLEAR;
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
    test_map.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
    test_map.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
    test_map.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OPENFLOW,  "drvpfcd")));
    test_map.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OVERLAY,  "drvoverlay")));
  }else{
    test_map.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "")));
  }
  return test_map;
}*/

/*class to test TcOperations*/
class TestTcOperations : public TcOperations {
  public:
  TestTcOperations(TcLock* tc_lock_, 
                     pfc::core::ipc::ServerSession* sess_, 
                     TcDbHandler* db_handler, 
                     TcChannelNameMap& unc_map_)
                     :TcOperations(tc_lock_, sess_, db_handler, unc_map_){}

  //TestTcOperations(){};


  TcOperStatus TestGetOperType() {
	return GetOperType();
  }

  TcOperStatus TestGetSessionId() {
        return GetSessionId();
  }

  TcOperStatus TestSetOperType() {
  	return SetOperType();
  }
  
  TcOperStatus TestSetSessionId() {
  	return SetSessionId();
  }

  TcOperStatus TestSetOperStatus(TcOperStatus resp_status) {
  	return SetOperStatus(resp_status);
  }

  TcOperStatus TestHandleMsgRet(TcOperRet MsgRet) {
    return HandleMsgRet(MsgRet);
  }

  TcOperStatus TestHandleArgs() {
        return HandleArgs();
  }

  TcOperStatus TestSendResponse(TcOperStatus send_oper_status) {
	return SendResponse(send_oper_status);
  } 

  TcOperStatus TestRevokeOperation(TcOperStatus oper_status) {
	return RevokeOperation(oper_status);
  }

  TcOperStatus TestDispatch() {
  	return Dispatch();
  }

  TcOperStatus TestExecute() {
    return Execute();
  }
  
  uint32_t TcGetMinArgCount() {
    return TcGetMinArgCount();
  
  } 
  
   TcOperStatus TcValidateOperType() {
    return TcValidateOperType();
  }
  
  TcOperStatus TcValidateOperParams() {
    return TcValidateOperParams();
  }

  TcOperStatus TcCheckOperArgCount(uint32_t) {

     return TC_OPER_SUCCESS;
    //return TcCheckOperArgCount(uint32_t);
  }

  TcOperStatus HandleLockRet(TcLockRet) {
     return TC_OPER_SUCCESS;
	//return HandleLockRet(TcLockRet);
  }

  TcOperStatus TcGetExclusion() {
   	return TcGetExclusion();
  }

  TcOperStatus TcReleaseExclusion() {
	return TcReleaseExclusion();
  }

  TcOperStatus TcCreateMsgList() {
   	return TcCreateMsgList();
  }
 
  TcOperStatus FillTcMsgData(TcMsg*,  unc::tclib::TcMsgOperType) {
      //	return FillTcMsgData(TcMsg*,  TcMsgOperType);
      return TC_OPER_SUCCESS;
  }

  TcOperStatus SendAdditionalResponse(TcOperStatus) {
        //return SendAdditionalResponse(TcOperStatus);
     return TC_OPER_SUCCESS;
  }
};

#if 0
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
#endif
class TestTcLock : public TcLock { 
  public:
  TestTcLock():TcLock() { }
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

#endif
