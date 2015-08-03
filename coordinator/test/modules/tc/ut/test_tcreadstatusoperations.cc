/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcreadstatusoperations.hh"
#include <uncxx/tc/libtc_common.hh>

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_string;
extern int stub_opertype;
extern int arg_count;

TcReadStatusType  rdbs;
uint64_t commit_number;

TEST(TcReadStatusOperations, TcGetMinArgCount) {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  int argcount  =  tc_readstatusoperations.TestTcGetMinArgCount();
  EXPECT_EQ(2, argcount);
}

TEST(TcReadStatusOperations, GetRunningStatus) {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
 
 EXPECT_EQ(TC_OPER_SUCCESS, 
      tc_readstatusoperations.GetRunningStatus(commit_number, rdbs));
}

TEST(TcReadStatusOperations, GetStartupStatus) {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
 EXPECT_EQ(TC_OPER_SUCCESS,
      tc_readstatusoperations.GetStartupStatus(commit_number, rdbs));
}

TEST(TcReadStatusOperations, SetRunningStatus) {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
 EXPECT_EQ(TC_OPER_SUCCESS,
      tc_readstatusoperations.SetRunningStatus());
}

TEST(TcReadStatusOperations, SetStartupStatus) {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
 EXPECT_EQ(TC_OPER_SUCCESS,
      tc_readstatusoperations.SetStartupStatus());
}

TEST(TcReadStatusOperations, SetRunningStatusIncr)  {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
 EXPECT_EQ(TC_OPER_SUCCESS,
      tc_readstatusoperations.SetRunningStatusIncr());
}

TEST(TcReadStatusOperations, SetStartupStatusIncr)  {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
 EXPECT_EQ(TC_OPER_SUCCESS,
      tc_readstatusoperations.SetStartupStatusIncr());
}

TEST(TcReadStatusOperations, TcValidateOperType_TC_INVALID_OPERATION_TYPE_1)  {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_readstatusoperations.tc_oper_ = TC_OP_READ_RELEASE;
 EXPECT_EQ(TC_INVALID_OPERATION_TYPE,
      tc_readstatusoperations.TcValidateOperType());
}

TEST(TcReadStatusOperations, TcValidateOperType_TC_INVALID_OPERATION_TYPE_2)  {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_readstatusoperations.tc_oper_ = TC_OP_INVALID;
 EXPECT_EQ(TC_INVALID_OPERATION_TYPE,
      tc_readstatusoperations.TcValidateOperType());
}

TEST(TcReadStatusOperations, TcCheckOperArgCount_TC_INVALID_OPERATION_TYPE)  {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_readstatusoperations.tc_oper_ = TC_OP_INVALID;
 EXPECT_EQ(TC_INVALID_OPERATION_TYPE,
      tc_readstatusoperations.TcCheckOperArgCount(3));
}

TEST(TcReadStatusOperations, TcCheckOperArgCount_TC_OPER_INVALID_INPUT)  {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_readstatusoperations.tc_oper_ = TC_OP_READ_RUNNING_STATUS;
 EXPECT_EQ(TC_OPER_INVALID_INPUT,
      tc_readstatusoperations.TcCheckOperArgCount(3));
}

TEST(TcReadStatusOperations, TcCheckOperArgCount_TC_OPER_SUCCESS)  {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_readstatusoperations.tc_oper_ = TC_OP_READ_RUNNING_STATUS;
 EXPECT_EQ(TC_OPER_SUCCESS,
      tc_readstatusoperations.TcCheckOperArgCount(2));
}

TEST(TcReadStatusOperations, SendAdditionalResponse_TC_OP_INVALID)  {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_readstatusoperations.tc_oper_ = TC_OP_INVALID;
 EXPECT_EQ(TC_INVALID_OPERATION_TYPE,
      tc_readstatusoperations.SendAdditionalResponse(TC_OPER_SUCCESS));
}

TEST(TcReadStatusOperations, SendAdditionalResponse_TC_OPER_FAILURE_1)  {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_readstatusoperations.tc_oper_ = TC_OP_READ_RUNNING_STATUS;
  EXPECT_EQ(TC_OPER_FAILURE,
      tc_readstatusoperations.SendAdditionalResponse(TC_OPER_SUCCESS));
}

TEST(TcReadStatusOperations, SendAdditionalResponse_TC_OPER_FAILURE_2)  {
  SET_AUDIT_OPER_PARAMS();

  TestTcReadStatusOperations tc_readstatusoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_readstatusoperations.tc_oper_ = TC_OP_READ_STARTUP_STATUS;
  EXPECT_EQ(TC_OPER_FAILURE,
      tc_readstatusoperations.SendAdditionalResponse(TC_OPER_SUCCESS));
}


