/*
 * Copyright (c) 2013-2014 NEC Corporation
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
  DEL_AUDIT_PARAMS();
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
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, TcValidateOperType) {
  SET_AUDIT_OPER_PARAMS();
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  stub_srv_uint32 = 1;
  tc_dboperations.tc_oper_  =  TC_OP_RUNNING_SAVE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_dboperations.TcValidateOperType());

  tc_dboperations.tc_oper_  =  TC_OP_CLEAR_STARTUP;
  // change -11 to define
  EXPECT_EQ(TC_OPER_SUCCESS, tc_dboperations.TcValidateOperType());
  DEL_AUDIT_PARAMS();
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
  DEL_AUDIT_PARAMS();
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
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, HandleMsgRet_Abort) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_ABORT;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_ABORT, tc_dboperations.HandleMsgRet(msgret));
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, HandleMsgRet_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_SUCCESS;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_dboperations.HandleMsgRet(msgret));
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, TcValidateOperParams) {
  SET_AUDIT_OPER_PARAMS();
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_dboperations.TcValidateOperParams());
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, TcGetExclusion) {
  SET_AUDIT_OPER_PARAMS();
  tc_lock_->TcUpdateUncState(TC_ACT);
  TestTcDbOperations tc_dboperations(tc_lock_, &sess_, db_handler, unc_map_);
  tc_dboperations.tc_oper_  =  TC_OP_RUNNING_SAVE;
  tc_dboperations.tclock_  =  tc_lock_;
  // Check error Code and change
  EXPECT_EQ(TC_OPER_SUCCESS, tc_dboperations.TcGetExclusion());
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, TcReleaseExclusion) {
  SET_AUDIT_OPER_PARAMS();

  TestTcDbOperations tc_dboperations(tc_lock_,  &sess_, db_handler, unc_map_);

  tc_dboperations.tc_oper_  =  TC_OP_USER_AUDIT;
  tc_dboperations.tclock_  =  tc_lock_;
  EXPECT_EQ(TC_OPER_FAILURE, tc_dboperations.TcReleaseExclusion());
  tc_dboperations.tc_oper_  =  TC_OP_DRIVER_AUDIT;
  EXPECT_EQ(TC_OPER_FAILURE, tc_dboperations.TcReleaseExclusion());
  DEL_AUDIT_PARAMS();
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
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, TcCreateMsgList) {
  SET_AUDIT_OPER_PARAMS();
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_dboperations.TcCreateMsgList());
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, FillTcMsgData) {
  SET_AUDIT_OPER_PARAMS();
  TcMsg* tc_msg  =  NULL;
  stub_srv_uint32  =  1;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_dboperations.FillTcMsgData(tc_msg, MSG_SAVE_CONFIG));
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, SendAdditionalResponse_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_dboperations.SendAdditionalResponse(oper_stat));
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, SendAdditionalResponse_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  stub_srv_uint32  =  0;
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_dboperations.SendAdditionalResponse(oper_stat));
  DEL_AUDIT_PARAMS();
}

TEST(TcDbOperations, Execute) {
  SET_AUDIT_OPER_PARAMS();
  TestTcDbOperations tc_dboperations(tc_lock_,
                                     &sess_,
                                     db_handler,
                                     unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE, tc_dboperations.Execute());
  DEL_AUDIT_PARAMS();
}
