/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_TEST_TC_TCMSG_COMMIT_H_
#define UNC_TEST_TC_TCMSG_COMMIT_H_

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

/*Frames TcChannelNameMap*/
TcChannelNameMap GetChannelNameMap_commit(int SetChannelName) {

  TcChannelNameMap test_map;
  if(SetChannelName) {
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_UPLL, "lgcnwd")));
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_UPPL, "phynwd")));
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_DRV_OPENFLOW, "")));
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_DRV_OVERLAY, "drvoverlay")));
  }else{
    test_map.insert((std::pair<TcDaemonName,std::string>(TC_UPLL, "")));
  }
  return test_map;
}

class Test2phaseCommit : public TwoPhaseCommit {
 public:

  /*Test2phaseCommit(uint32_t, unc::tclib::TcMsgOperType)
      :TwoPhaseCommit(uint32_t, unc::tclib::TcMsgOperType) {}*/

 //Test2phaseCommit();
 
 Test2phaseCommit(uint32_t sess_id,
                                unc::tclib::TcMsgOperType oper)
    :TwoPhaseCommit(sess_id, oper) {}

  TcOperRet TestSendRequestToDriver(){
    return SendRequestToDriver();
  }  
  TcOperRet TestHandleDriverNotPresent(unc_keytype_ctrtype_t driver){
    return HandleDriverNotPresent(driver);
  }
  TcOperRet TestGetControllerInfo(pfc::core::ipc::ClientSession* sess) {
    return GetControllerInfo(sess);
  }
};

class TestTwophaseAudit : public TwoPhaseAudit {

 public:
  TestTwophaseAudit(uint32_t sess_id, unc::tclib::TcMsgOperType oper)
      :TwoPhaseAudit(sess_id, oper){}

  TcOperRet TestSendRequest(std::string channel_name){
    return SendRequest(channel_name);
  }

  TcOperRet TestSendRequestToDriver() {
    return SendRequestToDriver();
  }

  //TcDriverInfoMap driverinfo_map_;

  TcOperRet TestGetControllerInfo(pfc::core::ipc::ClientSession* sess) {
    return GetControllerInfo(sess);
  }
};

/*class Test2phase: public TwoPhaseAudit {

Test2phase(uint32_t sess_id,
                                unc::tclib::TcMsgOperType oper)
    :TwoPhaseAudit(sess_id, oper) {}

}*/

#endif
