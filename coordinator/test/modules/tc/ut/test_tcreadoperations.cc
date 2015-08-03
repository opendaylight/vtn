/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcreadoperations.hh"
#include <uncxx/tc/libtc_common.hh>

/* Minimum number of aguments in Read request */
#define UNC_READ_OPER_ARG_COUNT_MIN  2
/* Maximum number of aguments in Read request */
#define UNC_READ_OPER_ARG_COUNT_MAX  3
/* Number of aguments in Acquire Read request */
#define UNC_READ_ACQUIRE_ARG_COUNT 3
/* Number of aguments in Release Read request */
#define UNC_READ_RELEASE_ARG_COUNT 2

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_string;
extern int stub_opertype;

TEST(TcReadOperations, TcGetMinArgCount) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  int argcount  =  tc_readoperations.TestTcGetMinArgCount();
  EXPECT_EQ(2, argcount);
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcCheckOperArgCount) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);

  uint32_t avail_count = 5;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.TcCheckOperArgCount(avail_count));

  avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.TcCheckOperArgCount(avail_count));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcCheckOperArgCount_ReadAcquire) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  uint32_t avail_count = 5;
  tc_readoperations.tc_oper_  =  TC_OP_READ_ACQUIRE;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
  tc_readoperations.TcCheckOperArgCount(avail_count));
  avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.TcCheckOperArgCount(avail_count));

  avail_count  =  UNC_READ_OPER_ARG_COUNT_MAX;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.TcCheckOperArgCount(avail_count));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcCheckOperArgCount_ReadRelease) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  uint32_t avail_count = 5;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.TcCheckOperArgCount(avail_count));
  avail_count = 2;
  tc_readoperations.tc_oper_  =  TC_OP_READ_RELEASE;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.TcCheckOperArgCount(avail_count));

  tc_readoperations.tc_oper_  =  TC_OP_READ_RELEASE;
  avail_count  =  UNC_READ_RELEASE_ARG_COUNT+1;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
  tc_readoperations.TcCheckOperArgCount(avail_count));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcCheckOperArgCount_Success) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  uint32_t avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.TcCheckOperArgCount(avail_count));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcValidateOperType) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);

  tc_readoperations.tc_oper_  =  TC_OP_READ_ACQUIRE;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.TcValidateOperType());
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcValidateOperType_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE,
  tc_readoperations.TcValidateOperType());
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, HandleMsgRet_Fatal) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_FATAL;
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_SYSTEM_FAILURE, tc_readoperations.HandleMsgRet(msgret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, HandleMsgRet_Abort) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_ABORT;
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_OPER_ABORT, tc_readoperations.HandleMsgRet(msgret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, HandleMsgRet_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_SUCCESS;
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_readoperations.HandleMsgRet(msgret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcValidateOperParams) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_readoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcValidateOperParams_ReadRelease) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  stub_srv_uint32  =  10;
  tc_readoperations.tc_oper_  =  TC_OP_READ_RELEASE;
  tc_readoperations.arg_timeout_  =  PFC_TRUE;

  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcValidateOperParams_RetFailure) {
  SET_AUDIT_OPER_PARAMS();
  stub_srv_uint32  =  0;
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_readoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcValidateOperParams_OperFailure) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);

  tc_readoperations.tc_oper_  =  TC_OP_READ_ACQUIRE;
  tc_readoperations.arg_timeout_  =  PFC_TRUE;
  stub_srv_uint32  =  0;
  tc_readoperations.timeout_  =  10;
  tc_readoperations.read_handle_  =  NULL;

  EXPECT_EQ(TC_OPER_INVALID_INPUT,
  tc_readoperations.TcValidateOperParams());

  // TO check fatal condition

  tc_readoperations.tc_oper_  =  TC_OP_READ_ACQUIRE;
  tc_readoperations.arg_timeout_  =  PFC_TRUE;
  stub_srv_uint32  =  -1;
  EXPECT_EQ(TC_OPER_FAILURE, tc_readoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcValidateOperParams_OperFailureWithTimeout) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);

  tc_readoperations.tc_oper_  =  TC_OP_READ_ACQUIRE;
  tc_readoperations.arg_timeout_  =  PFC_TRUE;
  stub_srv_uint32  =   10;
  tc_readoperations.timeout_  =  10;
  tc_readoperations.read_handle_  =  NULL;
  EXPECT_EQ(TC_OPER_FAILURE, tc_readoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}


TEST(TcReadOperations, TcGetExclusion_ReadRelease) {
  TestTcLock* tc_lock_  =  new TestTcLock();
  pfc_ipcsrv_t *srv  =  NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name  =  "UNC_DB_DSN";
  TcDbHandler* db_handler  =  new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;
  TcTaskqUtil* readq  =  NULL;

  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq);
  tc_readoperations.tc_oper_  =  TC_OP_READ_RELEASE;
  tc_readoperations.tclock_  =  tc_lock_;
  EXPECT_EQ(TC_SESSION_NOT_ACTIVE, tc_readoperations.TcGetExclusion());

  /*
  if( audit_ ! =  0)
   {
    delete audit_;
    audit_  =  NULL;
  }
  if( tc_lock ! =  0)
   {
    delete tc_lock;
    tc_lock  =  NULL;
  }
  if( db_handler ! =  0)
   {
    delete db_handler;
    db_handler  =  NULL;
  }
*/
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcGetExclusion_ReadAcquire) {
  TestTcLock* tc_lock_  =  new TestTcLock();
  pfc_ipcsrv_t *srv  =  NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name  =  "UNC_DB_DSN";
  TcDbHandler* db_handler  =  new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;
  TcTaskqUtil* readq  =  NULL;
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq);
  tc_readoperations.tc_oper_  =  TC_OP_READ_ACQUIRE;
  tc_readoperations.tclock_  =  tc_lock_;
  // Check error Code and change
  EXPECT_EQ(TC_INVALID_STATE, tc_readoperations.TcGetExclusion());
  // DEL_AUDIT_PARAMS();
}


TEST(TcReadOperations, TcGetExclusion_ReadAcquire_Success) {
  TestTcLock* tc_lock_  =  new TestTcLock();
  pfc_ipcsrv_t *srv  =  NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name  =  "UNC_DB_DSN";
  TcDbHandler* db_handler  =  new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;
  TcTaskqUtil* readq  =  NULL;

  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq);
  tc_readoperations.tc_oper_  =  TC_OP_USER_AUDIT;
  tc_readoperations.tclock_  =  tc_lock_;
  // Check error Code and change
  EXPECT_EQ(TC_OPER_FAILURE, tc_readoperations.TcGetExclusion());
  // DEL_AUDIT_PARAMS();
}



TEST(TcReadOperations, TcReleaseExclusion) {
  TestTcLock*  tc_lock_  =  new TestTcLock();
  pfc_ipcsrv_t *srv  =  NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name  =  "UNC_DB_DSN";
  TcDbHandler* db_handler  =  new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;
  TcTaskqUtil* readq  =  NULL;

  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_readoperations.TcReleaseExclusion());
  // DEL_AUDIT_PARAMS();
#if 0
  delete audit_;
  audit_  =  NULL;
  delete tc_lock;
  tc_lock  =  NULL;
  delete db_handler;
  db_handler  =  NULL;
#endif
}

TEST(TcReadOperations, HandleLockRet) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret  =  TC_LOCK_SUCCESS;
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);

  ret  =  TC_LOCK_INVALID_UNC_STATE;
  EXPECT_EQ(TC_INVALID_STATE, tc_readoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_OPERATION_NOT_ALLOWED;
  EXPECT_EQ(TC_INVALID_STATE, tc_readoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_BUSY;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_readoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_INVALID_SESSION_ID;
  // CHange  to define
  EXPECT_EQ(105, tc_readoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_ALREADY_ACQUIRED;
  EXPECT_EQ(TC_SESSION_ALREADY_ACTIVE, tc_readoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_NOT_ACQUIRED;
  EXPECT_EQ(TC_SESSION_NOT_ACTIVE, tc_readoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_ALREADY_ACQUIRED;
  EXPECT_EQ(TC_SESSION_ALREADY_ACTIVE, tc_readoperations.HandleLockRet(ret));

  // ret  =  TC_LOCK_ALREADY_ACQUIRED;
  // EXPECT_EQ(TC_OPER_FAILURE, tc_readoperations.HandleLockRet(ret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, TcCreateMsgList) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_readoperations.TcCreateMsgList());
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, FillTcMsgData) {
  SET_AUDIT_OPER_PARAMS();
  TcMsg* tc_msg  =  NULL;
  stub_srv_uint32  =  1;
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.FillTcMsgData(tc_msg, MSG_SAVE_CONFIG));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, SendAdditionalResponse_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.SendAdditionalResponse(oper_stat));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, SendAdditionalResponse_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  stub_srv_uint32  =  0;
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_readoperations.SendAdditionalResponse(oper_stat));
  // DEL_AUDIT_PARAMS();
}

TEST(TcReadOperations, Execute) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_readoperations.Execute());
  // DEL_AUDIT_PARAMS();
}

/*
TEST(TcReadOperations, Execute_PostTimer) {
  SET_AUDIT_OPER_PARAMS();
  TestTcReadOperations tc_readoperations(tc_lock_,
                                         &sess_,
                                         db_handler,
                                         unc_map_, readq_);
  tc_readoperations.timeout_  =  10;
  tc_readoperations.tc_oper_  =  TC_OP_READ_ACQUIRE;
  EXPECT_EQ(TC_OPER_FAILURE, tc_readoperations.Execute());
}*/
