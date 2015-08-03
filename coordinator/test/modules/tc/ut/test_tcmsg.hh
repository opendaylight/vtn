/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_TEST_TC_TCMSG_H_
#define UNC_TEST_TC_TCMSG_H_

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include "tcmsg.hh"
#include "tcmsg_commit.hh"
#include "tcmsg_audit.hh"
#include <gtest/gtest.h>

using namespace std;
using namespace unc::tc;
using namespace unc::tclib;

#define SET 1
#define CLEAR 0

int stub_session_invoke = CLEAR;
int stub_create_session = CLEAR;
int stub_response = CLEAR;
int stub_set_arg = CLEAR;
int stub_clnt_forward = CLEAR;
int stub_same_driverid = CLEAR;
int stub_set_string = CLEAR;

/*Frames TcChannelNameMap*/
TcChannelNameMap GetChannelNameMap(int SetChannelName) {
  
  TcChannelNameMap test_map;
  if(SetChannelName) {
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_UPLL, "lgcnwd")));
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_UPPL, "phynwd")));
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_DRV_OPENFLOW, "drvpfcd")));
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_DRV_OVERLAY, "drvoverlay")));
  }else{
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_DRV_OPENFLOW, "lgcnwd")));
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_DRV_OPENFLOW, "")));
  }
  return test_map;
}

AbortOnFailVector PopulateAbortVector() {
  AbortOnFailVector abort_on_fail;
  abort_on_fail.push_back(TC_DRV_OPENFLOW);
  abort_on_fail.push_back(TC_DRV_OVERLAY);
  abort_on_fail.push_back(TC_UPLL);
  abort_on_fail.push_back(TC_UPPL);
  return abort_on_fail;
}

/*class to test TcMsg*/
class TestTcMsg : public TcMsg {
  public:
  TestTcMsg(uint32_t sess_id, unc::tclib::TcMsgOperType oper)
      :TcMsg(sess_id, oper) {}

  TcOperRet Execute(){
    return TCOPER_RET_SUCCESS;
  }
  
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

class TestCommit : public TcMsgCommit{
 public:
  TestCommit(uint32_t sess_id, unc::tclib::TcMsgOperType oper)
      :TcMsgCommit(sess_id, oper) {}

  TcOperRet TestSendAbortRequest(AbortOnFailVector abort_on_fail){
    return SendAbortRequest(abort_on_fail);
  }

  TcOperRet TestSendTransEndRequest(AbortOnFailVector abort_on_fail){
    return SendTransEndRequest(abort_on_fail);
  }
  TcOperRet Execute(){
    return TCOPER_RET_SUCCESS;
  }
};

class TestCommitTrans : public CommitTransaction {

 public:
  TestCommitTrans(uint32_t sess_id, unc::tclib::TcMsgOperType oper)
            :CommitTransaction(sess_id, oper) {}

  TcOperRet TestSendRequest(std::string channelname) {
    return SendRequest(channelname);
  }
};

class Test2phase : public TwoPhaseCommit {

 public:
  Test2phase(uint32_t sess_id, unc::tclib::TcMsgOperType oper)
      :TwoPhaseCommit(sess_id, oper){}
  
  TcOperRet TestSendRequest(std::string channel_name){
    return SendRequest(channel_name);
  }
  
  TcOperRet TestSendRequestToDriver() {
    return SendRequestToDriver();
  }
  
  TcOperRet TestGetControllerInfo(pfc::core::ipc::ClientSession* sess) {
    return GetControllerInfo(sess);
  }
  
  TcOperRet TestHandleDriverNotPresent(unc_keytype_ctrtype_t driver){
    return HandleDriverNotPresent(driver);
  }
};


class TestAudit : public TcMsgAudit{
 public:
  TestAudit(uint32_t sess_id, unc::tclib::TcMsgOperType oper)
      :TcMsgAudit(sess_id, oper) {}

  TcOperRet TestSendAbortRequest(AbortOnFailVector abort_on_fail){
    return SendAbortRequest(abort_on_fail);
  }

  TcOperRet TestSendAuditTransEndRequest(AbortOnFailVector abort_on_fail,
                                         unc::tclib::TcMsgOperType oper){
    return SendAuditTransEndRequest(abort_on_fail, oper);
  }
  TcOperRet Execute(){
    return TCOPER_RET_SUCCESS;
  }
};

class TestAuditTrans : public AuditTransaction {

 public:
  TestAuditTrans(uint32_t sess_id, unc::tclib::TcMsgOperType oper)
            :AuditTransaction(sess_id, oper) {}

  TcOperRet TestSendRequest(std::string channelname) {
    return SendRequest(channelname);
  }
};

class Test2phaseAudit : public TwoPhaseAudit {

 public:
  Test2phaseAudit(uint32_t sess_id, unc::tclib::TcMsgOperType oper)
      :TwoPhaseAudit(sess_id, oper){}
  
  TcOperRet TestSendRequest(std::string channel_name){
    return SendRequest(channel_name);
  }
  
  TcOperRet TestSendRequestToDriver() {
    return SendRequestToDriver();
  }
  
  TcOperRet TestGetControllerInfo(pfc::core::ipc::ClientSession* sess) {
    return GetControllerInfo(sess);
  }
};
#endif

