/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcconfigoperations.hh"
#include <uncxx/tc/libtc_common.hh>

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_string;
extern int stub_opertype;

TEST(TcConfigOperations, TcGetMinArgCount) {
  SET_AUDIT_OPER_PARAMS();

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  int argcount  =  tc_configoperations.TestTcGetMinArgCount();
  EXPECT_EQ(2, argcount);
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, TcCheckOperArgCount) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  uint32_t avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_configoperations.TcCheckOperArgCount(avail_count));


  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE;
  avail_count = 5;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
  tc_configoperations.TcCheckOperArgCount(avail_count));

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  avail_count = 2;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
  tc_configoperations.TcCheckOperArgCount(avail_count));
  DEL_AUDIT_PARAMS();
}



TEST(TcConfigOperations, TcValidateOperType) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_RUNNING_SAVE;
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE, tc_configoperations.TcValidateOperType());

  tc_configoperations.tc_oper_  =  TC_OP_CLEAR_STARTUP;
  // change -11 to define
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE, tc_configoperations.TcValidateOperType());
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, TcValidateOperType_Failure) {
  SET_AUDIT_OPER_PARAMS();
  stub_srv_uint32  =  0;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  // Check return value and change to define
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE, tc_configoperations.TcValidateOperType());
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, HandleMsgRet_Fatal) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_FATAL;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  // check code change define value
  EXPECT_EQ(TC_SYSTEM_FAILURE, tc_configoperations.HandleMsgRet(msgret));
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, HandleMsgRet_Abort) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_ABORT;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_ABORT, tc_configoperations.HandleMsgRet(msgret));
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, HandleMsgRet_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.HandleMsgRet(msgret));
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, TcValidateOperParams) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcValidateOperParams());
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, TcValidateOperParams_Failure) {
  SET_AUDIT_OPER_PARAMS();
  tc_lock_->TcUpdateUncState(TC_ACT);
  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  stub_srv_uint32  =  0;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
  tc_configoperations.TcValidateOperParams());

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  stub_srv_uint32  =  -1;
  EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.TcValidateOperParams());

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  stub_srv_uint32  =  2;
  EXPECT_EQ(TC_CONFIG_NOT_PRESENT, tc_configoperations.TcValidateOperParams());
  DEL_AUDIT_PARAMS();
}


TEST(TcConfigOperations, TcGetExclusion) {
  TestTcLock* tc_lock_  =  new TestTcLock();
  pfc_ipcsrv_t *srv  =  NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name  =  "UNC_DB_DSN";
  TcDbHandler* db_handler  =  new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;

  tc_lock_->TcUpdateUncState(TC_ACT);
  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  tc_configoperations.tclock_  =  tc_lock_;
  EXPECT_EQ(TC_CONFIG_NOT_PRESENT, tc_configoperations.TcGetExclusion());

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE;
  tc_configoperations.tclock_  =  tc_lock_;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcGetExclusion());

  DEL_AUDIT_PARAMS();
}


TEST(TcConfigOperations, TcReleaseExclusion) {
  SET_AUDIT_OPER_PARAMS();
  TestTcConfigOperations tc_configoperations(tc_lock_,
                         &sess_,
                         db_handler,
                         unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_USER_AUDIT;
  tc_configoperations.tclock_  =  tc_lock_;
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.TcReleaseExclusion());

  tc_configoperations.tc_oper_  =  TC_OP_DRIVER_AUDIT;
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.TcReleaseExclusion());
  DEL_AUDIT_PARAMS();
}


TEST(TcConfigOperations, HandleLockRet) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret  =  TC_LOCK_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  ret  =  TC_LOCK_INVALID_UNC_STATE;
  EXPECT_EQ(TC_INVALID_STATE, tc_configoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_OPERATION_NOT_ALLOWED;
  EXPECT_EQ(TC_INVALID_STATE, tc_configoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_BUSY;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_configoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_NO_CONFIG_SESSION_EXIST;
  EXPECT_EQ(TC_CONFIG_NOT_PRESENT, tc_configoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_INVALID_PARAMS;
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_INVALID_SESSION_ID;
  EXPECT_EQ(5, tc_configoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_INVALID_CONFIG_ID;
  EXPECT_EQ(3, tc_configoperations.HandleLockRet(ret));
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, TcCreateMsgList) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcCreateMsgList());
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, FillTcMsgData) {
  SET_AUDIT_OPER_PARAMS();
  TcMsg* tc_msg  =  NULL;
  stub_srv_uint32  =  1;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.FillTcMsgData(tc_msg, MSG_SAVE_CONFIG));
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, SendAdditionalResponse_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_configoperations.SendAdditionalResponse(oper_stat));
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, SendAdditionalResponse_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  stub_srv_uint32  =  0;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.SendAdditionalResponse(oper_stat));
  DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, Execute) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.Execute());
  DEL_AUDIT_PARAMS();
}
