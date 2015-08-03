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
TcChannelNameMap GetChannelNameMapAudit(int SetChannelName) {

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
}

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
#endif
