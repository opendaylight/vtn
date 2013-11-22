/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcstartupoperations.hh"
#include <uncxx/tc/libtc_common.hh>

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_string;
extern int stub_opertype;

TEST(TcStartUpOperations , TcGetMinArgCount) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  int argcount = tc_startupoperations.TestTcGetMinArgCount();
  EXPECT_EQ(0, argcount);
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcCheckOperArgCount) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  int avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcCheckOperArgCount(avail_count));
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcValidateOperType) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcValidateOperType());
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , HandleArgs) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_startupoperations.HandleArgs());
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , HandleArgs_IsSwitch) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.is_switch_ = PFC_FALSE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_startupoperations.HandleArgs());
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcValidateOperParams) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcValidateOperParams());
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcGetExclusion) {
  TestTcLock* tc_lock_ = new TestTcLock();
  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name = "UNC_DB_DSN";
  TcDbHandler* db_handler = new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;
  pfc_bool_t is_switch = true;

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcGetExclusion());
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcReleaseExclusion) {
  TestTcLock*  tc_lock_ = new TestTcLock();
  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name = "UNC_DB_DSN";
  TcDbHandler* db_handler = new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;
  pfc_bool_t is_switch = true;

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcReleaseExclusion());
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , HandleLockRet) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret = TC_LOCK_SUCCESS;
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.HandleLockRet(ret));
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcCreateMsgList) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.is_switch_ = PFC_FALSE;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcCreateMsgList());
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcCreateMsgList_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.database_type_ = UNC_DT_RUNNING;
  tc_startupoperations.fail_oper_ = TC_OP_RUNNING_SAVE;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcCreateMsgList());
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , FillTcMsgData_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TcMsg* tc_msg = NULL;
  stub_srv_uint32 = 1;
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_FAILURE,
  tc_startupoperations.FillTcMsgData(tc_msg, MSG_SAVE_CONFIG));
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , SendResponse) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.SendResponse(oper_stat));
  DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , SendAdditionalResponse) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.SendAdditionalResponse(oper_stat));
  DEL_AUDIT_PARAMS();
}
