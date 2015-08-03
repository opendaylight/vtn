/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

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


#define SET_CANDIDATE_OPER_PARAMS()               \
    TcLock* tc_lock_ = new TcLock();          \
    pfc_ipcsrv_t *srv = NULL;                 \
    pfc::core::ipc::ServerSession sess_(srv); \
    std::string dsn_name = "UNC_DB_DSN";      \
    TcDbHandler* db_handler = new TcDbHandler(dsn_name);\
    TcChannelNameMap  unc_map_;               

TcChannelNameMap GetTcChannelNameMap_(int SetChannelName) {
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

/*class to test TcCandidateOperations*/
class TestClassTcCandidateOperations : public TcCandidateOperations {

 public:
 TestClassTcCandidateOperations(TcLock* tc_lock_,
                          pfc::core::ipc::ServerSession* sess_,
                          TcDbHandler* db_handler,
                          TcChannelNameMap& unc_map_)
                         :TcCandidateOperations(tc_lock_, sess_, db_handler, unc_map_){}
}; 

class TestClassTcLock : public TcLock {
  public:
  TestClassTcLock():TcLock() {}
  TcLockRet GetConfigData(uint32_t session_id,
                       uint32_t& config_id,
                       TcConfigMode& tc_mode,
                       std::string& vtn_name) {
          return TC_LOCK_SUCCESS; 
  }
};

