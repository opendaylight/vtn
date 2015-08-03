/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcdboperations.hh"
#include <uncxx/tc/libtc_common.hh>

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_string;
extern int stub_opertype;

TEST(TcDbOperations, TcGetMinArgCount) {
  SET_AUDIT_OPER_PARAMS();
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  int argcount  =  tc_dboperations.TcGetMinArgCount();
  EXPECT_EQ(2, argcount);
  // DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, TcCheckOperArgCount) {
  SET_AUDIT_OPER_PARAMS();
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);

  uint32_t avail_count = 5;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
  tc_dboperations.TcCheckOperArgCount(avail_count));

  avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_dboperations.TcCheckOperArgCount(avail_count));
  // DEL_AUDIT_PARAMS();
}


TEST(TcDbOperations, TcValidateOperType_Failure) {
  SET_AUDIT_OPER_PARAMS();
  stub_srv_uint32  =  0;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  // Check return value and change to define
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE, tc_dboperations.TcValidateOperType());
  // DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, HandleMsgRet_Fatal) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_FATAL;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  // check code change define value
  EXPECT_EQ(TC_SYSTEM_FAILURE, tc_dboperations.HandleMsgRet(msgret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, HandleMsgRet_Abort) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_ABORT;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_ABORT, tc_dboperations.HandleMsgRet(msgret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, HandleMsgRet_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_SUCCESS;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_dboperations.HandleMsgRet(msgret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, TcValidateOperParams) {
  SET_AUDIT_OPER_PARAMS();
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_dboperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, TcGetExclusion) {
  TestTcLock* tc_lock  =  new TestTcLock();
  pfc_ipcsrv_t *srv  =  NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name  =  "UNC_DB_DSN";
  TcDbHandler* db_handler  =  new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;

  TestTcDbOperations tc_dboperations(tc_lock, &sess_, db_handler, unc_map_);
  tc_dboperations.tc_oper_  =  TC_OP_RUNNING_SAVE;
  tc_dboperations.tclock_  =  tc_lock;
  // Check error Code and change
  EXPECT_EQ(TC_INVALID_STATE, tc_dboperations.TcGetExclusion());
}

TEST(TcDbOperations, TcReleaseExclusion) {
  TestTcLock*  tc_lock  =  new TestTcLock();
  pfc_ipcsrv_t *srv  =  NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name  =  "UNC_DB_DSN";
  TcDbHandler* db_handler  =  new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;

  TestTcDbOperations tc_dboperations(tc_lock,  &sess_, db_handler, unc_map_);

  tc_dboperations.tc_oper_  =  TC_OP_USER_AUDIT;
  tc_dboperations.tclock_  =  tc_lock;
  EXPECT_EQ(TC_OPER_FAILURE, tc_dboperations.TcReleaseExclusion());
  tc_dboperations.tc_oper_  =  TC_OP_DRIVER_AUDIT;
  EXPECT_EQ(TC_OPER_FAILURE, tc_dboperations.TcReleaseExclusion());
}


TEST(TcDbOperations, HandleLockRet) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret  =  TC_LOCK_SUCCESS;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);

  ret  =  TC_LOCK_INVALID_UNC_STATE;
  EXPECT_EQ(TC_INVALID_STATE, tc_dboperations.HandleLockRet(ret));

  ret  =  TC_LOCK_OPERATION_NOT_ALLOWED;
  EXPECT_EQ(TC_INVALID_STATE, tc_dboperations.HandleLockRet(ret));

  ret  =  TC_LOCK_BUSY;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_dboperations.HandleLockRet(ret));

  ret  =  TC_LOCK_NO_CONFIG_SESSION_EXIST;
  EXPECT_EQ(TC_OPER_FAILURE, tc_dboperations.HandleLockRet(ret));

  ret  =  TC_LOCK_INVALID_PARAMS;
  EXPECT_EQ(TC_OPER_FAILURE, tc_dboperations.HandleLockRet(ret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, FillTcMsgData) {
  SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap test_map;
  
  test_map.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  test_map.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  test_map.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OPENFLOW,  "drvpfcd")));
  test_map.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OVERLAY,  "drvoverlay")));

  TcMsg* tc_msg  =  TcMsg::CreateInstance(1211,
                                   MSG_SETUP,
                                   test_map);
  stub_srv_uint32  =  1;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_dboperations.FillTcMsgData(tc_msg, MSG_SAVE_CONFIG));
  // DEL_AUDIT_PARAMS();
}


TEST(TcDbOperations, Execute) {
  SET_AUDIT_OPER_PARAMS();
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE, tc_dboperations.Execute());
  // DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, Dispatch_Invalid_Input) {

  SET_AUDIT_OPER_PARAMS();
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_dboperations.Dispatch());
}

TEST(TcDbOperations, Dispatch_Failure) {

  SET_AUDIT_OPER_PARAMS();
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  sess_.getArgCount();
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_dboperations.Dispatch());
}
