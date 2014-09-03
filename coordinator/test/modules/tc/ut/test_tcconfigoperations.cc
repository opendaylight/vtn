/*
 * Copyright (c) 2012-2014 NEC Corporation
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
extern int arg_count;

TEST(TcConfigOperations, RevokeOperation_CONFIG_ACQUIRE_test){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE;
tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_OP_CONFIG_ACQUIRE);
//pfc::core::TaskQueue *taskq = pfc::core::TaskQueue::create(1);
pfc::core::timer_func_t timer_func;
//pfc_timeout_t time_out_id;
/*pfc_timespec_t  timeout;
timeout.tv_sec = 1200;
timeout.tv_nsec = 0;
tc_configoperations.timer_ = pfc::core::Timer::create(taskq->getId());
tc_configoperations.timer_->post(&timeout, timer_func, &time_out_id);*/
tc_configoperations.InsertConfigRequest();

EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.RevokeOperation(TC_SYSTEM_BUSY));
}


TEST(TcConfigOperations, TcGetMinArgCount) {
  SET_AUDIT_OPER_PARAMS();

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  int argcount  =  tc_configoperations.TestTcGetMinArgCount();
  EXPECT_EQ(2, argcount);
  // DEL_AUDIT_PARAMS();
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
  // DEL_AUDIT_PARAMS();
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
  // DEL_AUDIT_PARAMS();
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
  // DEL_AUDIT_PARAMS();
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
  // DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, HandleMsgRet_Abort) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_ABORT;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_ABORT, tc_configoperations.HandleMsgRet(msgret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, HandleMsgRet_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.HandleMsgRet(msgret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, TcValidateOperParams) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, TcValidateOperParams_Failure) {
  SET_AUDIT_OPER_PARAMS();
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
  EXPECT_EQ(TC_INVALID_STATE, tc_configoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}


TEST(TcConfigOperations, TcGetExclusion) {
  TestTcLock* tc_lock_  =  new TestTcLock();
  pfc_ipcsrv_t *srv  =  NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name  =  "UNC_DB_DSN";
  TcDbHandler* db_handler  =  new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  tc_configoperations.tclock_  =  tc_lock_;
  EXPECT_EQ(TC_CONFIG_NOT_PRESENT, tc_configoperations.TcGetExclusion());

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE;
  tc_configoperations.tclock_  =  tc_lock_;
  EXPECT_EQ(TC_INVALID_STATE, tc_configoperations.TcGetExclusion());

  tc_configoperations.tc_oper_  =  TC_OP_USER_AUDIT;
  tc_configoperations.tclock_  =  tc_lock_;
  EXPECT_EQ(TC_INVALID_STATE, tc_configoperations.TcGetExclusion());
  // DEL_AUDIT_PARAMS();
}


TEST(TcConfigOperations, TcReleaseExclusion) {
  TestTcLock*  tc_lock_  =  new TestTcLock();
  pfc_ipcsrv_t *srv  =  NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name  =  "UNC_DB_DSN";
  TcDbHandler* db_handler  =  new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;

  TestTcConfigOperations tc_configoperations(tc_lock_,
                         &sess_,
                         db_handler,
                         unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_USER_AUDIT;
  tc_configoperations.tclock_  =  tc_lock_;
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.TcReleaseExclusion());

  tc_configoperations.tc_oper_  =  TC_OP_DRIVER_AUDIT;
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.TcReleaseExclusion());
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
  EXPECT_EQ(105, tc_configoperations.HandleLockRet(ret));

  ret  =  TC_LOCK_INVALID_CONFIG_ID;
  EXPECT_EQ(103, tc_configoperations.HandleLockRet(ret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, TcCreateMsgList) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcCreateMsgList());
  // DEL_AUDIT_PARAMS();
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
  // DEL_AUDIT_PARAMS();
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
  // DEL_AUDIT_PARAMS();
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
  // DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, Execute) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.Execute());
  // DEL_AUDIT_PARAMS();
}

//U14
//Config-mode available
TEST(TcConfigOperations, HandleTimedConfigAcquisition_ConfMode){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.timeout_= 0;
tc_configoperations.SetConfigModeAvailability(PFC_TRUE);
EXPECT_EQ(106,
  tc_configoperations.HandleTimedConfigAcquisition());
}

//Config mode not available
TEST(TcConfigOperations, HandleTimedConfigAcquisition_NotConfMode){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.timeout_= 0;
tc_configoperations.SetConfigModeAvailability(PFC_FALSE);
EXPECT_EQ(TC_SYSTEM_BUSY,
  tc_configoperations.HandleTimedConfigAcquisition());
}

//tc_oper_ is TC_OP_INVALID
TEST(TcConfigOperations, RevokeOperation_Busy){
SET_AUDIT_OPER_PARAMS();
TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_SYSTEM_BUSY);
EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.RevokeOperation(TC_SYSTEM_BUSY));
}

//Crash in 518         tcop->timer_->cancel(tcop->time_out_id_);
TEST(TcConfigOperations, RevokeOperation_CONFIG_ACQUIRE){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_OP_CONFIG_ACQUIRE);
tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE;

//pfc::core::TaskQueue *taskq = pfc::core::TaskQueue::create(1);
pfc::core::timer_func_t timer_func;
//pfc_timeout_t time_out_id;
/*pfc_timespec_t  timeout;
timeout.tv_sec = 120;
timeout.tv_nsec = tc_configoperations.timeout_ * 1000000;    // 1ms = 1000000ns
timeout.tv_nsec = 0;
tc_configoperations.timer_ = pfc::core::Timer::create(taskq->getId());
tc_configoperations.timer_->post(&timeout, timer_func, &time_out_id);*/
tc_configoperations.InsertConfigRequest();

EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.RevokeOperation(TC_SYSTEM_BUSY));
}

TEST(TcConfigOperations, RevokeOperation_CONFIG_ACQUIRE_TIMED){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
//pfc::core::TaskQueue *taskq = pfc::core::TaskQueue::create(1);
pfc::core::timer_func_t timer_func;
//pfc_timeout_t time_out_id;
/*pfc_timespec_t  timeout;
timeout.tv_sec = 120;
timeout.tv_nsec = 0;
tc_configoperations.timer_ = pfc::core::Timer::create(taskq->getId());
tc_configoperations.timer_->post(&timeout, timer_func, &time_out_id);*/
tc_configoperations.InsertConfigRequest();
tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE_TIMED;
tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_OP_CONFIG_ACQUIRE_TIMED);

EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.RevokeOperation(TC_SYSTEM_BUSY));
}

TEST(TcConfigOperations, RevokeOperation_CONFIG_ACQUIRE_FORCE){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE_FORCE;
//pfc::core::TaskQueue *taskq = pfc::core::TaskQueue::create(1);
pfc::core::timer_func_t timer_func;
//pfc_timeout_t time_out_id;
/*pfc_timespec_t  timeout;
timeout.tv_sec = 120;
timeout.tv_nsec = tc_configoperations.timeout_ * 1000000;    // 1ms = 1000000ns
timeout.tv_nsec = 0;
tc_configoperations.timer_ = pfc::core::Timer::create(taskq->getId());
tc_configoperations.timer_->post(&timeout, timer_func, &time_out_id);*/
tc_configoperations.InsertConfigRequest();
tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_OP_CONFIG_ACQUIRE_FORCE);

EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.RevokeOperation(TC_SYSTEM_BUSY));
}

TEST(TcConfigOperations, RevokeOperation_CONFIG_CONFIG_RELEASE){
SET_AUDIT_OPER_PARAMS();
TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.tc_oper_ = TC_OP_CONFIG_RELEASE;
tc_configoperations.tc_oper_status_ = SEND_RESPONSE_PHASE;
//tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_OP_CONFIG_RELEASE);
//pfc::core::TaskQueue *taskq = pfc::core::TaskQueue::create(1);
pfc::core::timer_func_t timer_func;
//pfc_timeout_t time_out_id;
/*pfc_timespec_t  timeout;
timeout.tv_sec = 120;
timeout.tv_nsec = tc_configoperations.timeout_ * 1000000;    // 1ms = 1000000ns
timeout.tv_nsec = 0;
tc_configoperations.timer_ = pfc::core::Timer::create(taskq->getId());
tc_configoperations.timer_->post(&timeout, timer_func, &time_out_id);*/
tc_configoperations.InsertConfigRequest();
EXPECT_EQ(TC_OPER_FAILURE,
      tc_configoperations.RevokeOperation(TC_OPER_SUCCESS));
}

TEST(TcConfigOperations, Dispatch_InvalidInput){
    SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
    EXPECT_EQ(TC_OPER_INVALID_INPUT,tc_configoperations.Dispatch());

}

TEST(TcConfigOperations, Dispatch_HandleArgs_Failure){
    SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
    EXPECT_EQ(TC_OPER_INVALID_INPUT,tc_configoperations.Dispatch());
}
#if 0
TEST(TcConfigOperations, Dispatch_CONFIG_ACQUIRE){
    SET_AUDIT_OPER_PARAMS();
    //pfc_ipcsrv_t *srv1  =  NULL;
    //pfc::core::ipc::ServerSession sess_test(srv1);

    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
    tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE;

    pfc::core::TaskQueue *taskq = pfc::core::TaskQueue::create(1);
    pfc::core::timer_func_t timer_func;
    pfc_timeout_t time_out_id;
    pfc_timespec_t  timeout;
    timeout.tv_sec = 1200;
    timeout.tv_nsec = 0;
    tc_configoperations.timer_ = pfc::core::Timer::create(taskq->getId());
    tc_configoperations.timer_->post(&timeout, timer_func, &time_out_id);
    tc_configoperations.InsertConfigRequest();

    EXPECT_EQ(TC_OPER_FAILURE,tc_configoperations.Dispatch());
}
TEST(TcConfigOperations, Dispatch_CONFIG_RELEASE){
    SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
    tc_configoperations.tc_oper_ = TC_OP_CONFIG_RELEASE;
    EXPECT_EQ(TC_OPER_FAILURE,tc_configoperations.Dispatch());
}

TEST(TcConfigOperations, TcValidateOperParams_CONFIG_ACQUIRE_TIMED) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE_TIMED;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_configoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}

TEST(TcConfigOperations, TcValidateOperParams_TIMED_ARG_COUNT_MIN) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE_TIMED;
  //pfc_ipcsrv_t test_srv;
  //test_srv.isv_args= 3;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_configoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}
#endif
