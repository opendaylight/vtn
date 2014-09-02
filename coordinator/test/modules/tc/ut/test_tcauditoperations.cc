/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcauditoperations.hh"
#include <uncxx/tc/libtc_common.hh>

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_string;
extern int stub_opertype;

int OperArray[]= {
  MSG_NOTIFY_CONFIGID,
  MSG_SETUP,
  MSG_SETUP_COMPLETE,
  MSG_COMMIT_TRANS_START,
  MSG_COMMIT_VOTE,
  MSG_COMMIT_GLOBAL,
  MSG_COMMIT_TRANS_END,
  MSG_AUDIT_START,
  MSG_AUDIT_TRANS_START,
  MSG_AUDIT_VOTE,
  MSG_AUDIT_GLOBAL,
  MSG_AUDIT_TRANS_END,
  MSG_AUDIT_END,
  MSG_SAVE_CONFIG,
  MSG_CLEAR_CONFIG,
  MSG_ABORT_CANDIDATE,
  MSG_AUDITDB,
  MSG_GET_DRIVERID};

TEST(TcAuditOperations, TcGetMinArgCount) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  int argcount = tc_auditoperations.TestTcGetMinArgCount();
  EXPECT_EQ(4, argcount);
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, TcCheckOperArgCount) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  uint32_t avail_count = 5;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
            tc_auditoperations.TcCheckOperArgCount(avail_count));

  avail_count = 4;
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_auditoperations.TcCheckOperArgCount(avail_count));
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, GetSessionId_Success) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  stub_srv_uint32 = 100;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.GetSessionId());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, GetSessionId_Fatal) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  stub_srv_uint32 = -1;
  EXPECT_EQ(TC_OPER_FAILURE, tc_auditoperations.GetSessionId());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, GetSessionId_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_auditoperations.GetSessionId());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, GetSessionId_Session_Invalid) {
    SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  stub_srv_uint32 = 10;
  tc_auditoperations.session_id_ =0;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_auditoperations.GetSessionId());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, GetSessionId_Oper_Success) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  stub_srv_uint32 = 10;
  tc_auditoperations.session_id_ =10;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.GetSessionId());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, TcValidateOperType) {
    SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE;
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE,
            tc_auditoperations.TcValidateOperType());

  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.TcValidateOperType());

  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_FAILURE, tc_auditoperations.TcValidateOperType());
  DEL_AUDIT_PARAMS();
}
TEST(TcAuditOperations, TcValidateOperParams_UserAudit) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_= TC_OP_USER_AUDIT;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_auditoperations.TcValidateOperParams());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, TcValidateOperParams_Success) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.controller_id_ = "C1";
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.TcValidateOperParams());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, TcValidateOperParams_Invalid) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);

  tc_auditoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  stub_srv_string = 1;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_auditoperations.TcValidateOperParams());
  stub_srv_string = 0;
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, TcValidateOperParams_Unknown) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.driver_id_ = UNC_CT_UNKNOWN;
  tc_auditoperations.controller_id_ = "C1";
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.TcValidateOperParams());
  DEL_AUDIT_PARAMS();
}
/*
TEST(TcAuditOperations, TcGetExclusion) {
  TestTcLock* tc_lock_ = new TestTcLock();
  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name = "UNC_DB_DSN";
  TcDbHandler* db_handler = new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;
  //int32_t alarm_id = 1;
  TcTaskqUtil* audit_ = new TcTaskqUtil(TC_AUDIT_CONCURRENCY);

  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  tc_auditoperations.tclock_ = tc_lock_;
  tc_lock_->TcUpdateUncState(TC_ACT);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.TcGetExclusion());
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.TcReleaseExclusion());


  tc_auditoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.TcGetExclusion());
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.TcReleaseExclusion());

  tc_auditoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  tc_auditoperations.api_audit_ = PFC_TRUE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.TcGetExclusion());
  DEL_AUDIT_PARAMS();
}
*/
TEST(TcAuditOperations, TcReleaseExclusion) {
  TestTcLock*  tc_lock_ = new TestTcLock();
  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name = "UNC_DB_DSN";
  TcDbHandler* db_handler = new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;
  // int32_t alarm_id = 1;
  TcTaskqUtil* audit_ = new TcTaskqUtil(TC_AUDIT_CONCURRENCY);

  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  tc_auditoperations.tclock_ = tc_lock_;
  EXPECT_EQ(TC_OPER_FAILURE, tc_auditoperations.TcReleaseExclusion());

  tc_auditoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  EXPECT_EQ(TC_OPER_FAILURE, tc_auditoperations.TcReleaseExclusion());

  tc_auditoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  tc_auditoperations.api_audit_ = PFC_TRUE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.TcReleaseExclusion());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, HandleLockRet) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret = TC_LOCK_SUCCESS;
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  ret = TC_LOCK_INVALID_UNC_STATE;
  EXPECT_EQ(TC_INVALID_STATE, tc_auditoperations.HandleLockRet(ret));

  ret = TC_LOCK_OPERATION_NOT_ALLOWED;
  EXPECT_EQ(TC_INVALID_STATE, tc_auditoperations.HandleLockRet(ret));

  ret = TC_LOCK_BUSY;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_auditoperations.HandleLockRet(ret));

  ret = TC_LOCK_NO_CONFIG_SESSION_EXIST;
  EXPECT_EQ(TC_CONFIG_NOT_PRESENT, tc_auditoperations.HandleLockRet(ret));

  ret = TC_LOCK_INVALID_PARAMS;
  EXPECT_EQ(TC_OPER_FAILURE, tc_auditoperations.HandleLockRet(ret));
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, TcCreateMsgList) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.TcCreateMsgList());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, FillTcMsgData) {
    SET_AUDIT_OPER_PARAMS();
  TcMsg* tc_msg = NULL;
  TcChannelNameMap daemon_names = GetTcChannelNameMap(SET);

  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  EXPECT_EQ(TC_OPER_FAILURE,
            tc_auditoperations.FillTcMsgData(tc_msg, MSG_GET_DRIVERID));

  tc_msg =  TcMsg::CreateInstance(1, MSG_GET_DRIVERID,  daemon_names);
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_auditoperations.FillTcMsgData(tc_msg,  MSG_GET_DRIVERID));
  if ( tc_msg != 0 ) {
    delete tc_msg;
    tc_msg = NULL;
  }

  tc_auditoperations.driver_id_ = UNC_CT_UNKNOWN;
  EXPECT_EQ(TC_OPER_FAILURE,
            tc_auditoperations.FillTcMsgData(tc_msg, MSG_AUDIT_START));

  tc_auditoperations.driver_id_ = UNC_CT_PFC;
  EXPECT_EQ(TC_OPER_FAILURE,
            tc_auditoperations.FillTcMsgData(tc_msg,  MSG_AUDIT_START));

  EXPECT_EQ(TC_OPER_FAILURE,
            tc_auditoperations.FillTcMsgData(tc_msg,  MSG_AUDIT_TRANS_END));
  EXPECT_EQ(TC_OPER_FAILURE,
            tc_auditoperations.FillTcMsgData(tc_msg, MSG_AUDIT_END));
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, SendAdditionalResponse_Success) {
    SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  EXPECT_EQ(TC_OPER_FAILURE,
            tc_auditoperations.SendAdditionalResponse(oper_stat));
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, SendAdditionalResponse_UserAudit) {
    SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;;

  TcChannelNameMap daemon_names = GetTcChannelNameMap(1);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;

  EXPECT_EQ(TC_OPER_FAILURE,
            tc_auditoperations.SendAdditionalResponse(oper_stat));
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, SendAdditionalResponse_API_audit_true) {
    SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;;
  TcChannelNameMap daemon_names = GetTcChannelNameMap(1);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  tc_auditoperations.api_audit_ = PFC_TRUE;
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_auditoperations.SendAdditionalResponse(oper_stat));
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, SendAdditionalResponse_SystemFailure) {
    SET_AUDIT_OPER_PARAMS();
  TcMsg* tc_msg = NULL;
  TcOperStatus oper_stat = TC_OPER_SUCCESS;;
  TcChannelNameMap daemon_names = GetTcChannelNameMap(1);

  tc_msg =  TcMsg::CreateInstance(1, MSG_GET_DRIVERID,  daemon_names);

  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  stub_srv_uint32 = 1;
  tc_auditoperations.resp_tc_msg_ =  tc_msg;
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_auditoperations.SendAdditionalResponse(oper_stat));
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, SetAuditOperationStatus) {
    SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap(1);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  stub_srv_uint32 = 1;
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_auditoperations.SetAuditOperationStatus());
  DEL_AUDIT_PARAMS();
}
/*
TEST(TcAuditOperations, SendAuditStatusNotification) {
    SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap(1);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  stub_srv_uint32 = 1;
  tc_auditoperations.audit_result_ = unc::tclib::TC_AUDIT_SUCCESS;
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_auditoperations.SendAuditStatusNotification(alarm_id));
  DEL_AUDIT_PARAMS();
}
*/

TEST(TcAuditOperations, GetDriverType) {
  SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap(SET);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  tc_auditoperations.controller_id_ = "C1";
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.GetDriverType());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, AuditStart) {
    SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap(SET);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.AuditStart());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, AuditTransStart) {
    SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap(SET);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.AuditTransStart());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, AuditVote) {
    SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap(SET);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.AuditVote());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, AuditGlobalCommit) {
    SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap(SET);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.AuditGlobalCommit());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, AuditTransEnd) {
    SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap(SET);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.AuditTransEnd());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, AuditEnd) {
    SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap(SET);
  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.tc_oper_ = TC_OP_USER_AUDIT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.AuditEnd());
  DEL_AUDIT_PARAMS();
}

TEST(TcAuditOperations, Execute) {
    SET_AUDIT_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap(SET);

  TestTcAuditOperations tc_auditoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_,
                                           audit_);
  tc_auditoperations.api_audit_ = PFC_TRUE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.Execute());

  tc_auditoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  tc_auditoperations.controller_id_ = "C1";
  EXPECT_EQ(TC_OPER_SUCCESS, tc_auditoperations.Execute());

  tc_auditoperations.api_audit_ = PFC_FALSE;
  tc_auditoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  tc_auditoperations.controller_id_ = "C1";
  EXPECT_EQ(TC_SYSTEM_FAILURE, tc_auditoperations.Execute());
  DEL_AUDIT_PARAMS();
}

