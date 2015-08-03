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
    TcChannelNameMap  unc_map_;               \
    TcTaskqUtil* audit_ = new TcTaskqUtil(TC_AUDIT_CONCURRENCY);

#define DEL_AUDIT_PARAMS() \
        delete tc_lock_; \
        tc_lock_ =NULL ; \
        delete db_handler; \
        db_handler = NULL; \
        delete audit_; \
        audit_ = NULL;


TcChannelNameMap GetTcChannelNameMap(int SetChannelName) {

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
}

/*class to test TcOperations*/
class TestTcOperations : public TcOperations {
  public:
  TestTcOperations(TcLock* tc_lock_,
                   pfc::core::ipc::ServerSession* sess_,
                   TcDbHandler* db_handler,
                   TcChannelNameMap& unc_map_)
                   :TcOperations(tc_lock_, sess_, db_handler, unc_map_) {}

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

/*class to test TcStartupOperations*/
class TestTcStartUpOperations : public TcStartUpOperations {

 public:
 TestTcStartUpOperations(TcLock* tclock,
                         pfc::core::ipc::ServerSession* sess,
                         TcDbHandler* tc_db_,
                         TcChannelNameMap& unc_map_,
                         pfc_bool_t is_switch)
 			 :TcStartUpOperations(tclock, sess, tc_db_, unc_map_, is_switch){}

  uint32_t TestTcGetMinArgCount() {
  	return TestTcGetMinArgCount();
  }

  TcOperStatus TestHandleArgs() {
  	return HandleArgs();
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

  TcOperStatus TestFillTcMsgData(TcMsg* tc_msg, unc::tclib::TcMsgOperType oper_type) {
  	return FillTcMsgData(tc_msg, oper_type);
  }


};

/*class to test TcReadOperations*/
class TestTcReadOperations : public TcReadOperations {

 public:
 TestTcReadOperations(TcLock* tclock, 
                         pfc::core::ipc::ServerSession* sess, 
                         TcDbHandler* tc_db_, 
                         TcChannelNameMap& unc_map_, 
                         TcTaskqUtil* readq_)
                         :TcReadOperations(tclock, sess, tc_db_, unc_map_, readq_){}

  uint32_t TestTcGetMinArgCount() {
        return TestTcGetMinArgCount();
  }

  TcOperStatus TestHandleArgs() {
        return HandleArgs();
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

  TcOperStatus TestFillTcMsgData(TcMsg* tc_msg, unc::tclib::TcMsgOperType oper_type) {
        return FillTcMsgData(tc_msg, oper_type);
  }

  TcOperStatus TestSendAdditionalResponse(TcOperStatus oper_stat) {
        return SendAdditionalResponse(oper_stat);
  }

  TcOperStatus TestExecute() {
        return Execute();
  }

};


/*class to test TcSaveOperations*/
class TestTcAutoSaveOperations : public TcAutoSaveOperations {

 public:
 TestTcAutoSaveOperations(TcLock* tc_lock_, 
                          pfc::core::ipc::ServerSession* sess_, 
                          TcDbHandler* db_handler, 
                          TcChannelNameMap& unc_map_)
                         :TcAutoSaveOperations(tc_lock_, sess_, db_handler, unc_map_){}

  uint32_t TestTcGetMinArgCount() {
        return TcGetMinArgCount();
  }

  TcOperStatus TestHandleArgs() {
        return HandleArgs();
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

  TcOperStatus TestFillTcMsgData(TcMsg* tc_msg, unc::tclib::TcMsgOperType oper_type) {
        return FillTcMsgData(tc_msg, oper_type);
  }

  TcOperStatus TestSendAdditionalResponse(TcOperStatus oper_stat) {
        return SendAdditionalResponse(oper_stat);
  }

  TcOperStatus TestExecute() {
        return Execute();
  }

  TcOperStatus TestSetAutoSave() {
        return SetAutoSave();
  }

 
};


/*class to test TcCandidateOperations*/
class TestTcCandidateOperations : public TcCandidateOperations {

 public:
 TestTcCandidateOperations(TcLock* tc_lock_, 
                          pfc::core::ipc::ServerSession* sess_, 
                          TcDbHandler* db_handler, 
                          TcChannelNameMap& unc_map_)
                         :TcCandidateOperations(tc_lock_, sess_, db_handler, unc_map_){}


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

  TcOperStatus TestFillTcMsgData(TcMsg* tc_msg, unc::tclib::TcMsgOperType oper_type) {
        return FillTcMsgData(tc_msg, oper_type);
  }

  TcOperStatus TestExecute() {
        return Execute();
  }
 

  pfc_bool_t TestTransStartMsg() {
        return TransStartMsg();
  }

  pfc_bool_t TestTransVoteMsg() {
        return TransVoteMsg();
  }

  pfc_bool_t TestTransGlobalCommitMsg() {
        return TransGlobalCommitMsg();
  }

  pfc_bool_t TestTransEndMsg() {
        return TransEndMsg();
  }

  TcOperStatus TestSetConfigId() {
        return SetConfigId();
  }


  TcOperStatus TestSendResponse(TcOperStatus oper_stat) {
        return SendResponse(oper_stat);
  }

  TcOperStatus TestSendAdditionalResponse(TcOperStatus oper_stat) {
        return SendAdditionalResponse(oper_stat);
  }

};


/*class to test TcConfigOperations*/
class TestTcConfigOperations : public TcConfigOperations {

 public:
 TestTcConfigOperations(TcLock* tc_lock_, 
                          pfc::core::ipc::ServerSession* sess_, 
                          TcDbHandler* db_handler, 
                          TcChannelNameMap& unc_map_)
                         :TcConfigOperations(tc_lock_, sess_, db_handler, unc_map_){}
  
  TcOperStatus TestSetConfigId(){
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

  TcOperStatus TestFillTcMsgData(TcMsg* tc_msg, unc::tclib::TcMsgOperType oper_type) {
        return FillTcMsgData(tc_msg, oper_type);
  }

  TcOperStatus TestSendAdditionalResponse(TcOperStatus oper_stat) {
        return SendAdditionalResponse(oper_stat);
  }



};


/*class to test TcConfigOperations*/
class TestTcDbOperations : public TcDbOperations {

 public:
 TestTcDbOperations(TcLock* tc_lock_, 
                          pfc::core::ipc::ServerSession* sess_, 
                          TcDbHandler* db_handler, 
                          TcChannelNameMap& unc_map_)
                         :TcDbOperations(tc_lock_, sess_, db_handler, unc_map_){}


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

  TcOperStatus TestFillTcMsgData(TcMsg* tc_msg, unc::tclib::TcMsgOperType oper_type) {
        return FillTcMsgData(tc_msg, oper_type);
  }

  TcOperStatus TestSendAdditionalResponse(TcOperStatus oper_stat) {
        return SendAdditionalResponse(oper_stat);
  }

};

class TestTcAuditOperations : public TcAuditOperations {

 public:

  TestTcAuditOperations(TcLock* tclock, 
                        pfc::core::ipc::ServerSession* sess, 
                        TcDbHandler* tc_db_, 
                        TcChannelNameMap& unc_map_, 
                        TcTaskqUtil* audit_)
 			 :TcAuditOperations(tclock, sess, tc_db_, unc_map_, audit_){}

  uint32_t TestTcGetMinArgCount() {
  	return TcGetMinArgCount();
  }

  TcOperStatus TestHandleArgs() {
  	return HandleArgs();
  }
  
  TcOperStatus TestTcCheckOperArgCount(uint32_t avail_count) {
  	return TcCheckOperArgCount(avail_count);
  }
  TcOperStatus TestGetSessionId(){
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

  TcOperStatus TestFillTcMsgData(TcMsg* tc_msg, unc::tclib::TcMsgOperType oper_type) {
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
                    TcWriteOperation write_operation, TcConfigMode config_mode, std::string vtn_name) {
    return TC_LOCK_SUCCESS;
  }
  TcLockRet ReleaseLock(uint32_t session_id,  uint32_t config_id, 
                        TcOperation operation, 
                        TcWriteOperation write_operation) {
    return TC_LOCK_SUCCESS;
  }
#if 0
  TcLockRet AutoSaveEnable();
  TcLockRet AutoSaveDisable();
  TcLockRet GetConfigData(uint32_t session_id,  uint32_t &config_id, 
                          TcConfigMode& config_mode, std::string& vtn_name);
  TcLockRet NotifyConfigIdSessionIdDone(uint32_t config_id, 
            uint32_t session_id,  TcNotifyOperation config_notify_operation);
  TcSessionOperationProgress  GetSessionOperation(uint32_t session_id);
  TcLockRet  TcAcquireReadLockForStateTransition(void);
  TcLockRet  TcReleaseReadLockForStateTransition(void);
  void TcUpdateUncState(TcState state);
  TcState  GetUncCurrentState(void);
  pfc_bool_t IsStateTransitionInProgress(void);
  TcLockRet TcMarkSessionId(uint32_t session_id);
  uint32_t  GetMarkedSessionId(uint32_t session_id);

  private:
  /* Prohibit copy constuction and copy assignment.*/
  TcLock(const TcLock&);
  TcLock& operator=(const TcLock&);

  /* Config data map */
  TcConfigNameMap tc_config_name_map_;
  /* Read data */
  TcReadLock tc_read_lock_;
  /* Write data */
  TcReadWriteLock tc_rwlock_;
  /* save data */
  TcAutoSave tc_auto_save_;
  /* State data */
  TcStateLock tc_state_lock_;
  inline pfc::core::Mutex &
  getGlobalLock(void)  {
    return tc_rwlock_.rw_mutex;
  }

  /* Config lock methods */
  TcLockRet AcquireConfigLock(uint32_t session_id,
                              TcConfigMode tc_mode,
                              std::string vtn_name);
#endif                              
};


#endif


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
