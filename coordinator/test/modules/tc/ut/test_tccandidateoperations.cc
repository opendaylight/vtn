/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tccandidateoperations.hh"
#include <uncxx/tc/libtc_common.hh>
#include "tcmsg.hh"

using namespace std;
using namespace unc;
using namespace tc;

//Stub Flags
extern int stub_srv_int32;
extern int stub_srv_uint32;
extern int stub_create_session;
extern int stub_session_invoke;
extern int stub_response;
extern int stub_set_arg;
extern int stub_same_driverid;
extern int stub_set_string;
extern uint32_t resp_count;
extern int stub_clnt_forward;
extern int stub_create_session;
extern int stub_srv_uint8;

TEST(TcCandidateOperations, Dispatch_INVALID_INPUT) {
  SET_CANDIDATE_OPER_PARAMS();
  TcOperStatus status;
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  //Testing Dispatch Function when Invalid Arg Type
  status = tc_candidateoperations.Dispatch();
  EXPECT_EQ(TC_OPER_INVALID_INPUT, status);

  delete tc_lock_;
}

TEST(TcCandidateOperations, Dispatch_HandleArgs_Failure){
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  //Testing Dispatch Function when Handle Args return Invalid Type
  TcOperStatus status = tc_candidateoperations.Dispatch();
  EXPECT_EQ(TC_OPER_INVALID_INPUT, status);

  delete tc_lock_;
}

TEST(TcCandidateOperations, Dispatch_TC_SYSTEM_BUSY){
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           NULL,
                                           db_handler,
                                           unc_map_);
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  stub_srv_uint32 = 1;
  tc_lock_->TcUpdateUncState(TC_ACT);
  tc_candidateoperations.session_id_ = 100;
  std::string tc_config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          tc_config_name);
  TcOperStatus status = tc_candidateoperations.Dispatch();
  EXPECT_EQ(TC_SYSTEM_BUSY, status);
  delete tc_lock_;

  stub_srv_uint32 = 0;
}

TEST(TcCandidateOperations, Dispatch){
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);

  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           NULL,
                                           db_handler,
                                           map);
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  stub_srv_uint32 = 1;
  tc_lock_->TcUpdateUncState(TC_ACT);
  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 100;
  std::string tc_config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          tc_config_name);
  tc_candidateoperations.tclock_->tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  stub_session_invoke = 0;
  stub_response = 0;
  resp_count = 1;
  TcOperStatus status = tc_candidateoperations.Dispatch();
  EXPECT_EQ(TC_OPER_SUCCESS, status);
  delete tc_lock_;

  resp_count = 0;
  stub_srv_uint32 = 0;
}

TEST(TcCandidateOperations, HandleCandidateTimedRequest_AuditNotStarted){
  SET_CANDIDATE_OPER_PARAMS();

  pfc_ipcsrv_t *s = NULL;                 
  pfc::core::ipc::ServerSession sessi_(s);
  
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sessi_,
                                           db_handler,
                                           unc_map_);
  tc_candidateoperations.cancel_audit_ = PFC_TRUE;
  TcOperations::SetAuditPhase(unc::tc::AUDIT_NOT_STARTED);
  tc_lock_->TcUpdateUncState(TC_ACT);
  // Testing HandleCandidateTimedRequest when audit phase is set to 
  // Audit_Not_Started and cancel_audit_ set to TRUE
  TcOperStatus status = tc_candidateoperations.HandleCandidateTimedRequest();
  EXPECT_EQ(TC_OPER_FAILURE, status);

  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCandidateTimedRequest_AuditCancelInprogress){
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);

  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);
  tc_candidateoperations.cancel_audit_ = PFC_TRUE;
  TcOperations::SetAuditPhase(unc::tc::AUDIT_START);
  TcOperations::SetAuditCancelStatus(unc::tc::AUDIT_CANCEL_INPROGRESS);
  tc_lock_->TcUpdateUncState(TC_ACT);
  // Testing HandleCandidateTimedRequest when audit cancel status set to 
  // Audit_Cancel_Inprogress and cancel_audit_ set to TRUE
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.HandleCandidateTimedRequest());

  delete tc_lock_;
  TcMsg::SetAuditCancelFlag(PFC_FALSE); 
}

TEST(TcCandidateOperations, HandleCandidateTimedRequest_AUDIT_GLOBAL_COMMIT) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);
  tc_candidateoperations.cancel_audit_ = PFC_TRUE;
  TcOperations::SetAuditCancelStatus(unc::tc::AUDIT_CANCEL_DONE);
  TcOperations::SetAuditPhase(unc::tc::AUDIT_GLOBAL_COMMIT);
  tc_lock_->TcUpdateUncState(TC_ACT);
  // Testing HandleCandidateTimedRequest when audit phase set to
  // AUDIT_GLOBAL_COMMIT and cancel_audit_ set to TRUE  
  // HandleCancelAudit should not called
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.HandleCandidateTimedRequest());
  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCandidateTimedRequest_AUDIT_START) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);
  tc_candidateoperations.cancel_audit_ = PFC_TRUE;
  TcOperations::SetAuditCancelStatus(unc::tc::AUDIT_CANCEL_DONE);
  TcOperations::SetAuditPhase(unc::tc::AUDIT_START);
  TcMsg::SetNotifyDriverId(UNC_CT_VNP);

  tc_lock_->TcUpdateUncState(TC_ACT);
  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 3;
  std::string tc_config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          tc_config_name);
  tc_candidateoperations.tclock_->tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;

  // Testing HandleCandidateTimedRequest when audit phase set to 
  // Audit_Start and cancel_audit_ set to TRUE : Audit needs to be cancelled
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.HandleCandidateTimedRequest());

  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCandidateTimedRequest_timeout_zero){
  //Commit with no cancelAudit and queue is empty
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  tc_lock_->TcUpdateUncState(TC_ACT);

  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 4;
  std::string config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          config_name);
  tc_candidateoperations.tclock_->tc_config_name_map_[config_name].is_notify_pending = PFC_FALSE;

  // timeout is zero 
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.HandleCandidateTimedRequest());

  tc_candidateoperations.TcReleaseExclusion(); //Release Lock if any
  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCandidateTimedRequest_timeout_zero_QueueNotEmpty){
  //Commit with no cancelAudit and queue is not empty
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  tc_lock_->TcUpdateUncState(TC_ACT);

  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 4;
  std::string config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          config_name);
  tc_candidateoperations.tclock_->tc_config_name_map_[config_name].is_notify_pending = PFC_FALSE;

  // Queue is not empty
  tc_candidateoperations.InsertCandidateRequest();

  EXPECT_EQ(TC_SYSTEM_BUSY, tc_candidateoperations.HandleCandidateTimedRequest());

  tc_candidateoperations.ClearCandidateQueue();//Clear the candidate queue
  tc_candidateoperations.TcReleaseExclusion(); //Release Lock if any
  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCandidateTimedRequest_Timeout_possitive_Success){
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  tc_candidateoperations.cancel_audit_ = PFC_TRUE;
  tc_candidateoperations.timeout_ = 30;
  TcOperations::SetAuditPhase(unc::tc::AUDIT_NOT_STARTED);
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  tc_lock_->TcUpdateUncState(TC_ACT);

  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 4;
  std::string config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          config_name);
  tc_candidateoperations.tclock_->tc_config_name_map_[config_name].is_notify_pending = PFC_FALSE;

  // timeout is possitive and No Request are waiting in the queue
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.HandleCandidateTimedRequest());

  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCandidateTimedRequest_Timeout_possitive){
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  tc_candidateoperations.cancel_audit_ = PFC_TRUE;
  tc_candidateoperations.timeout_ = 30;
  TcOperations::SetAuditPhase(unc::tc::AUDIT_NOT_STARTED);
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  tc_lock_->TcUpdateUncState(TC_ACT);

  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 4;
  std::string config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          config_name);
 
  tc_candidateoperations.InsertCandidateRequest();
  //timeout_ is +ve and queue is not empty
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_candidateoperations.HandleCandidateTimedRequest());

  tc_candidateoperations.ClearCandidateQueue();
  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCandidateTimedRequest_timeout_Negative)  {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  tc_candidateoperations.timeout_ = -1;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  tc_lock_->TcUpdateUncState(TC_ACT);
  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 4;
  std::string config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          config_name);
  tc_candidateoperations.tclock_->tc_config_name_map_[config_name].is_notify_pending = PFC_FALSE;

  //Timeout_ is negative and queue is empty : it should get the write lock
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.HandleCandidateTimedRequest());

}

//HandleCancleAudit
TEST(TcCandidateOperations, HandleCancelAudit) { 
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  TcOperations::audit_cancel_notify_.push_back(TC_UPPL);
  TcOperations::audit_cancel_notify_.push_back(TC_UPLL);
  EXPECT_EQ(TCOPER_RET_SUCCESS, tc_candidateoperations.HandleCancelAudit());
  
  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCancelAudit_Fail) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  TcOperations::audit_cancel_notify_.push_back(TC_UPPL);
  TcOperations::audit_cancel_notify_.push_back(TC_UPLL);
  //channel_name.empty
  EXPECT_EQ(TCOPER_RET_FAILURE, tc_candidateoperations.HandleCancelAudit());

  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCancelAudit_SessNULL) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  TcOperations::audit_cancel_notify_.push_back(TC_UPPL);
  TcOperations::audit_cancel_notify_.push_back(TC_UPLL);
  stub_create_session = 1;
  //Client Session NULL
  EXPECT_EQ(TCOPER_RET_FAILURE, tc_candidateoperations.HandleCancelAudit());

  stub_create_session = 0;
  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCancelAudit_operType_Fail) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  TcOperations::audit_cancel_notify_.push_back(TC_UPPL);
  TcOperations::audit_cancel_notify_.push_back(TC_UPLL);
  stub_set_arg = 1;
  //set OperType Fail
  EXPECT_EQ(TCOPER_RET_FAILURE, tc_candidateoperations.HandleCancelAudit());

  stub_set_arg = 0;
  delete tc_lock_;
}


TEST(TcCandidateOperations, HandleCancelAudit_driverId_Fail) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  TcOperations::audit_cancel_notify_.push_back(TC_UPPL);
  TcOperations::audit_cancel_notify_.push_back(TC_UPLL);
  TcMsg::SetNotifyDriverId(UNC_CT_UNKNOWN);
  //driver_id Fail
  EXPECT_EQ(TCOPER_RET_FAILURE, tc_candidateoperations.HandleCancelAudit());

  delete tc_lock_;
  TcMsg::SetNotifyDriverId(UNC_CT_PFC);
}

TEST(TcCandidateOperations, HandleCancelAudit_ctrlId_Fail) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  TcOperations::audit_cancel_notify_.push_back(TC_UPPL);
  TcOperations::audit_cancel_notify_.push_back(TC_UPLL);
  TcMsg::SetNotifyControllerId("");
  //controller_id Fail
  EXPECT_EQ(TCOPER_RET_FAILURE, tc_candidateoperations.HandleCancelAudit());

  TcMsg::SetNotifyControllerId("CTR1");
  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCancelAudit_SessInvoke_Fail) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  TcOperations::audit_cancel_notify_.push_back(TC_UPPL);
  TcOperations::audit_cancel_notify_.push_back(TC_UPLL);
  //SessionInvoke Fail
  stub_session_invoke = 1;
  EXPECT_EQ(TCOPER_RET_FATAL, tc_candidateoperations.HandleCancelAudit());
  
  stub_session_invoke = 0;
  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCancelAudit_SessResp_Fail) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  TcOperations::audit_cancel_notify_.push_back(TC_UPPL);
  TcOperations::audit_cancel_notify_.push_back(TC_UPLL);
  stub_response = 1;
  //Session Invoke Response Fail
  EXPECT_EQ(TCOPER_RET_ABORT, tc_candidateoperations.HandleCancelAudit());

  stub_response = 0;
  delete tc_lock_;
}

TEST(TcCandidateOperations, HandleCancelAudit_SessResp_success) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  TcOperations::audit_cancel_notify_.push_back(TC_UPPL);
  TcOperations::audit_cancel_notify_.push_back(TC_UPLL);
  //Session Invoke Response Success
  stub_response = 0;
  EXPECT_EQ(TCOPER_RET_SUCCESS, tc_candidateoperations.HandleCancelAudit());

  delete tc_lock_;
}


TEST(TcCandidateOperations, HandleCandidateRelease_QueEmpty) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  // Candidate queue is empty
  tc_candidateoperations.HandleCandidateRelease();
  EXPECT_TRUE(0 == tc_candidateoperations.candidate_req_queue_.size());

  delete tc_lock_;
}

TEST(TcCandidateOperations, ClearCandidateQueue_QueEmpty) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  // Candidate queue is empty
  tc_candidateoperations.ClearCandidateQueue();
  EXPECT_TRUE(0 == tc_candidateoperations.candidate_req_queue_.size());

  delete tc_lock_;
}

TEST(TcCandidateOperations, ClearCandidateQueue_QueNotEmpty) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  // Candidate queue is Not empty
  tc_candidateoperations.InsertCandidateRequest();
  tc_candidateoperations.ClearCandidateQueue();
  EXPECT_TRUE(0 == tc_candidateoperations.candidate_req_queue_.size());

  delete tc_lock_;
}

TEST(TcCandidateOperations, RemoveCandidateRequest_SessValid) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  // Candidate queue is Not empty
  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.InsertCandidateRequest();
  uint32_t sess_id = 100;
  tc_candidateoperations.RemoveCandidateRequest(sess_id);
  EXPECT_TRUE(0 == tc_candidateoperations.candidate_req_queue_.size());
}

TEST(TcCandidateOperations, RemoveCandidateRequest_SessInValid) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  // Candidate queue is Not empty
  tc_candidateoperations.InsertCandidateRequest();
  uint32_t sess_id = 100;
  //Session id matching Falied
  tc_candidateoperations.RemoveCandidateRequest(sess_id);
  EXPECT_TRUE(1 == tc_candidateoperations.candidate_req_queue_.size());
}

TEST(TcCandidateOperations, RetrieveCandidateRequestQueNotEmpty) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  // Candidate queue is Not empty
  tc_candidateoperations.InsertCandidateRequest();
  TcCandidateOperations* ptr = tc_candidateoperations.RetrieveCandidateRequest();
  EXPECT_TRUE(NULL != ptr);
}

TEST(TcCandidateOperations, RetrieveCandidateRequestQueEmpty) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap daemon_names = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           daemon_names);
  // Candidate queue is empty
  tc_candidateoperations.ClearCandidateQueue();
  TcCandidateOperations* ptr = tc_candidateoperations.RetrieveCandidateRequest();
  EXPECT_TRUE(NULL == ptr);
}


TEST(TcCandidateOperations, GetDaemonName_Valid) {
  //Testing the GetDaemonName Function Possitive Case with valid TcChannelMap
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);
  std::string retVal = tc_candidateoperations.GetDaemonName(TC_UPLL);
  EXPECT_EQ("lgcnwd",retVal);  

  delete tc_lock_;
}

TEST(TcCandidateOperations, GetDaemonName_Invalid) {
  //Testing the GetDaemonName Function Negative Case with invalid TcChannelMap
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  std::string retVal = tc_candidateoperations.GetDaemonName(TC_UPLL);
  EXPECT_EQ("",retVal);  

  delete tc_lock_;
}

TEST(TcCandidateOperations, GetUtilResp_Success) {
  //Testing the GetUtilResp Function when TCUTIL_RET_SUCCESS
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  TcUtilRet ret = TCUTIL_RET_SUCCESS; 
  TcOperRet retVal = tc_candidateoperations.GetUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_SUCCESS,retVal);  

  delete tc_lock_;
}

TEST(TcCandidateOperations, GetUtilResp_Faliure) {
  //Testing the GetUtilResp Function when TCUTIL_RET_FAILURE
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  TcUtilRet ret = TCUTIL_RET_FAILURE; 
  TcOperRet retVal = tc_candidateoperations.GetUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_FAILURE,retVal);  

  delete tc_lock_;
}

TEST(TcCandidateOperations, GetUtilResp_Fatal) {
  //Testing the GetUtilResp Function when TCUTIL_RET_FATAL
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  TcUtilRet ret = TCUTIL_RET_FATAL; 
  TcOperRet retVal = tc_candidateoperations.GetUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_FATAL,retVal);  

  delete tc_lock_;
}

TEST(TcCandidateOperations, GetUtilResp_Unknown) {
  //Testing the GetUtilResp Function when TCUTIL_RET_UNKNOWN
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  TcUtilRet ret = (TcUtilRet)3; 
  TcOperRet retVal = tc_candidateoperations.GetUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_UNKNOWN,retVal);  

  delete tc_lock_;
}

TEST(TcCandidateOperations, InsertCandidateRequest) {
  //Testing the InsertCandidateRequest
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  /* Before adding any request into the queue, size is zero */
  int val = TcCandidateOperations::candidate_req_queue_.size();
  EXPECT_EQ( 0, val);
  /* After adding request into the queue, size is one */
  tc_candidateoperations.InsertCandidateRequest();
  val = TcCandidateOperations::candidate_req_queue_.size();
  EXPECT_EQ( 1, val);

  //Clear the queue
  tc_candidateoperations.ClearCandidateQueue();

  delete tc_lock_;
}

TEST(TcCandidateOperations, RetrieveCandidateRequest) {
  //Testing the RetrieveCandidateRequest
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  /* Before adding any request into the queue, size is zero */
  TcCandidateOperations* ptr = tc_candidateoperations.RetrieveCandidateRequest();
  EXPECT_EQ( 0, ptr);

  //Clear the queue
  tc_candidateoperations.ClearCandidateQueue();
  delete tc_lock_;
}

TEST(TcCandidateOperations, TcCheckOperArgCount) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);

  uint32_t avail_count;

  //When tc_oper_ is set to TC_OP_CANDIDATE_COMMIT
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  avail_count = 3;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcCheckOperArgCount(avail_count));

  //When tc_oper_ is set to TC_OP_CANDIDATE_ABORT
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT;
  avail_count = 3;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcCheckOperArgCount(avail_count));

  //When tc_oper_ is set to TC_OP_CANDIDATE_COMMIT_TIMED
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  avail_count = 5;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcCheckOperArgCount(avail_count));

  //When tc_oper_ is set to TC_OP_CANDIDATE_ABORT_TIMED
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT_TIMED;
  avail_count = 5;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcCheckOperArgCount(avail_count));

  /* Test the Failure Conditions */

  //When tc_oper_ is set to TC_OP_CANDIDATE_COMMIT
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  avail_count = 2; //Invalid Input
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_candidateoperations.TcCheckOperArgCount(avail_count));

  //When tc_oper_ is set to TC_OP_CANDIDATE_ABORT
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT;
  avail_count = 1; //Invalid Input
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_candidateoperations.TcCheckOperArgCount(avail_count));

  //When tc_oper_ is set to TC_OP_CANDIDATE_COMMIT_TIMED
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  avail_count = 4; //Invalid Input
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_candidateoperations.TcCheckOperArgCount(avail_count));

  //When tc_oper_ is set to TC_OP_CANDIDATE_ABORT_TIMED
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT_TIMED;
  avail_count = 0; //Invalid Input
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_candidateoperations.TcCheckOperArgCount(avail_count));

  delete tc_lock_;
}

TEST(TcCandidateOperations, TcValidateOperType) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  
  //When tc_oper_ is set to TC_OP_CANDIDATE_COMMIT 
  stub_srv_uint32 = 1;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcValidateOperType());

  //When tc_oper_ is set to TC_OP_CANDIDATE_ABORT 
  stub_srv_uint32 = 1;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcValidateOperType());

  //When tc_oper_ is set to TC_OP_CANDIDATE_COMMIT_TIMED 
  stub_srv_uint32 = 1;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcValidateOperType());

  //When tc_oper_ is set to TC_OP_CANDIDATE_ABORT_TIMED 
  stub_srv_uint32 = 1;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT_TIMED;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcValidateOperType());
  
  /* Testing the Failure Cases */ 

  //When tc_oper_ is set to TC_OP_INVALID 
  tc_candidateoperations.tc_oper_ = TC_OP_INVALID;
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE, tc_candidateoperations.TcValidateOperType());
  
  //When tc_oper_ is set to TC_OP_CANDIDATE_COMMIT 
  stub_srv_uint32 = 0;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcValidateOperType());

  //When tc_oper_ is set to TC_OP_CANDIDATE_ABORT 
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcValidateOperType());

  //When tc_oper_ is set to TC_OP_CANDIDATE_COMMIT_TIMED 
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcValidateOperType());

  //When tc_oper_ is set to TC_OP_CANDIDATE_ABORT_TIMED 
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT_TIMED;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcValidateOperType());

  delete tc_lock_;
}

TEST(TcCandidateOperations, TcValidateOperParams_Success) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateobj(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  tc_lock_->TcUpdateUncState(TC_ACT);
  uint32_t session_id = 100;
  uint32_t config_id = 4;
  std::string tc_config_name;
  tc_candidateobj.tclock_->tc_config_name_map_[tc_config_name].session_id = session_id;
  tc_candidateobj.config_id_ = 4;
  tc_candidateobj.tclock_->tc_config_name_map_[tc_config_name].config_id = config_id;
  tc_candidateobj.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  tc_candidateobj.tclock_->tc_config_name_map_[tc_config_name].is_taken = PFC_TRUE;
  tc_candidateobj.session_id_ = 100;
  stub_srv_uint32 = 1;
  //Testing the TcValidateOperParams with Valid Data
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateobj.TcValidateOperParams());
}

TEST(TcCandidateOperations, TcValidateOperParams) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);

  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcValidateOperParams());

  //TC_INVALID_SESSION_ID
  stub_srv_uint32 = 1;
  tc_lock_->TcUpdateUncState(TC_ACT);
  EXPECT_EQ(105, tc_candidateoperations.TcValidateOperParams());

  std::string tc_config_name;
  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 4;
  uint32_t session_id = 100;
  tc_candidateoperations.tclock_->tc_config_name_map_[tc_config_name].session_id = session_id;
  uint32_t config_id = 4;
  tc_candidateoperations.tclock_->tc_config_name_map_[tc_config_name].config_id = config_id;
  tc_candidateoperations.tclock_->tc_config_name_map_[tc_config_name].is_taken = PFC_TRUE;

  /* Failure Case when tc_oper_ is TC_OP_CANDIDATE_COMMIT_TIMED */
  stub_srv_uint32 = 0;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcValidateOperParams());

  /* Failure Case when tc_oper_ is TC_OP_CANDIDATE_ABORT_TIMED */
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT_TIMED;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcValidateOperParams());

  /* Success Case when tc_oper_ is TC_OP_CANDIDATE_COMMIT_TIMED */
  stub_srv_int32 = 1;
  stub_srv_uint32 = 1;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcValidateOperParams());

  /* Success Case when tc_oper_ is TC_OP_CANDIDATE_ABORT_TIMED */
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT_TIMED;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcValidateOperParams());

  /* Timeout missing for candidate timed operation */
  stub_srv_int32 = 0;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcValidateOperParams());
  stub_srv_int32 = 1;

  /* CancelAudit_ missing for candidate timed operation */
  stub_srv_uint8 = 0;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcValidateOperParams());
  stub_srv_uint8 = 1;
  delete tc_lock_;
}

TEST(TcCandidateOperations,TcGetExclusion) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  //Failure Case with Invalid Session Id
  tc_lock_->TcUpdateUncState(TC_ACT);
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcGetExclusion());

  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 3;
  std::string tc_config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          tc_config_name);
  //Lock is not available
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_candidateoperations.TcGetExclusion());
  
  //Failure case when tc_oper_ is TC_OP_CANDIDATE_COMMIT 
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_candidateoperations.TcGetExclusion());

  //Failure case when tc_oper_ is TC_OP_CANDIDATE_COMMIT_TIMED 
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_candidateoperations.TcGetExclusion());

  //Failure case when tc_oper_ is TC_OP_CANDIDATE_ABORT 
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_candidateoperations.TcGetExclusion());

  //Failure case when tc_oper_ is TC_OP_CANDIDATE_ABORT_TIMED 
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT_TIMED;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_candidateoperations.TcGetExclusion());

  tc_candidateoperations.TcReleaseExclusion(); //Release the acquired lock
  //Acquire lock for TC_OP_CANDIDATE_COMMIT
  tc_candidateoperations.tclock_->tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcGetExclusion());
  tc_candidateoperations.TcReleaseExclusion(); //Release the acquired lock
 
  //Acquire Lock For TC_OP_CANDIDATE_COMMIT_TIMED
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcGetExclusion());
  tc_candidateoperations.TcReleaseExclusion(); //Release the acquired lock

  //Acquire lock for TC_OP_CANDIDATE_ABORT
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcGetExclusion());
  tc_candidateoperations.TcReleaseExclusion(); //Release the acquired lock

  //Acquire lock for TC_OP_CANDIDATE_ABORT_TIMED
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT_TIMED;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcGetExclusion());
  tc_candidateoperations.TcReleaseExclusion(); //Release the acquired lock

  delete tc_lock_;
}

TEST(TcCandidateOperations, TcReleaseExclusion) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  
  //trying to release when no lock is acquired  
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.TcReleaseExclusion());

  //Acquire the write lock for TC_OP_CANDIDATE_COMMIT
  tc_lock_->TcUpdateUncState(TC_ACT);
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 3;
  std::string tc_config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          tc_config_name);
  tc_candidateoperations.tclock_->tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  tc_candidateoperations.TcGetExclusion();
  EXPECT_EQ(TC_OPER_SUCCESS,tc_candidateoperations.TcReleaseExclusion());

  //Acquire the write lock for TC_OP_CANDIDATE_COMMIT_TIMED
  tc_lock_->TcUpdateUncState(TC_ACT);
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT_TIMED;
  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 3;
  std::string config_name = "global-config";
  tc_candidateoperations.tclock_->UpdateConfigData(tc_candidateoperations.session_id_,
                                                          config_name);
  tc_candidateoperations.tclock_->tc_config_name_map_[config_name].is_notify_pending = PFC_FALSE;
  tc_candidateoperations.TcGetExclusion();
  EXPECT_EQ(TC_OPER_SUCCESS,tc_candidateoperations.TcReleaseExclusion());

  delete tc_lock_;
}

TEST(TcCandidateOperations, Execute) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);

  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  EXPECT_EQ(TC_SYSTEM_FAILURE, tc_candidateoperations.Execute()); //Invalid Session/Config ID

  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 3;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.Execute());
}

TEST(TcCandidateOperations, Execute_TransStartMsg) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  //Test the TransStartMsg Fn False case due to invalid session Id
  EXPECT_EQ(PFC_FALSE, tc_candidateoperations.TransStartMsg()); 

  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 300;
  //Test the TransStartMsg Fn True case
  EXPECT_EQ(PFC_TRUE, tc_candidateoperations.TransStartMsg()); 
}

TEST(TcCandidateOperations, Execute_TransVoteMsg) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);

  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_COMMIT;
  //Test the TransVoteMsg Fn False case due to invalid session Id
  EXPECT_EQ(PFC_FALSE, tc_candidateoperations.TransVoteMsg()); 

  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 100;
  //Test the TransVoteMsg Fn True case
  EXPECT_EQ(PFC_TRUE, tc_candidateoperations.TransVoteMsg()); 
}

TEST(TcCandidateOperations, Execute_TransGlobalCommitMsg) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);

  //Test TransGlobalCommitMsg() Fn with Invalid Session/Config ID
  EXPECT_EQ(PFC_FALSE, tc_candidateoperations.TransGlobalCommitMsg());

  //Test TransGlobalCommitMsg() Fn 
  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 100;
  EXPECT_EQ(PFC_TRUE, tc_candidateoperations.TransGlobalCommitMsg());

}


TEST(TcCandidateOperations, Execute_TransEndMsg) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);
  //Test TransEndMsg() Fn with Invalid Session/Config ID
  EXPECT_EQ(PFC_FALSE, tc_candidateoperations.TransEndMsg());

  //Test TransEndMsg() Fn
  tc_candidateoperations.session_id_ = 100;
  tc_candidateoperations.config_id_ = 100;
  EXPECT_EQ(PFC_TRUE, tc_candidateoperations.TransEndMsg());

}

TEST(TcCandidateOperations, TcCreateMsgList_Empty) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);

  //MessageList is Empty
  tc_candidateoperations.tc_oper_ = TC_OP_CANDIDATE_ABORT;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcCreateMsgList());
}

TEST(TcCandidateOperations, TcCreateMsgList) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,                 
                                           map);
  tc_candidateoperations.TcOperMessageList.push_back(unc::tclib::MSG_COMMIT_GLOBAL);
  //MSG_COMMIT_GLOBAL
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.TcCreateMsgList());
}

TEST(TcCandidateOperations, FillTcMsgData_Fail) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);
  TcMsg* tc_msg = NULL;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.FillTcMsgData(tc_msg,unc::tclib::MSG_COMMIT_GLOBAL));
}

TEST(TcCandidateOpeations, FillTcMsgData_Success) {
  SET_CANDIDATE_OPER_PARAMS();
  TcChannelNameMap map = GetTcChannelNameMap_(SET);
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           map);

  uint32_t sess = 100;
  TcMsg* tc_msg = TcMsg::CreateInstance(sess, unc::tclib::MSG_COMMIT_TRANS_END,map);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.FillTcMsgData(tc_msg,unc::tclib::MSG_COMMIT_GLOBAL));
}

TEST(TcCandidateOperations, HandleLockRet) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);

  TcLockRet lock_ret = TC_LOCK_INVALID_UNC_STATE;
  EXPECT_EQ(TC_INVALID_STATE, tc_candidateoperations.HandleLockRet(lock_ret));

  lock_ret = TC_LOCK_OPERATION_NOT_ALLOWED;
  EXPECT_EQ(TC_INVALID_STATE, tc_candidateoperations.HandleLockRet(lock_ret));

  lock_ret = TC_LOCK_BUSY;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_candidateoperations.HandleLockRet(lock_ret));

  lock_ret = TC_LOCK_NO_CONFIG_SESSION_EXIST;
  EXPECT_EQ(TC_CONFIG_NOT_PRESENT, tc_candidateoperations.HandleLockRet(lock_ret));

  lock_ret = (TcLockRet)0; //Invalid type
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.HandleLockRet(lock_ret));

}

TEST(TcCandidateOperations, SetConfigId) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.SetConfigId());
  
  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.SetConfigId());

}

TEST(TcCandidateOpeations, SendResponse_SessNULL) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           NULL,
                                           db_handler,
                                           unc_map_);

  //Session Null
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.SendResponse(oper_stat));
}

TEST(TcCandidateOpeations, SendResponse) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);

  TcOperStatus oper_stat = TC_OPER_INVALID_INPUT;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tc_candidateoperations.SendResponse(oper_stat));

  //SetOperType failed
  oper_stat = TC_OPER_SUCCESS;
  EXPECT_EQ(TC_OPER_FAILURE, tc_candidateoperations.SendResponse(oper_stat));
}

TEST(TcCandidateOperations, SendAdditionalResponse) {
  SET_CANDIDATE_OPER_PARAMS();
  TestClassTcCandidateOperations tc_candidateoperations(tc_lock_,
                                           &sess_,
                                           db_handler,
                                           unc_map_);
  
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_candidateoperations.SendAdditionalResponse(oper_stat));
}


TEST(TcLock, AcquireWriteLock_sessInvalid) {
  TcLock tclock;
  uint32_t session_id = 100;
  TcWriteOperation write_operation = TC_COMMIT;
  /* Testing AcquireWriteLock for Commit when session is invalid */
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, tclock.AcquireWriteLock(session_id, write_operation));

  write_operation = TC_ABORT_CANDIDATE_CONFIG;
  /* Testing AcquireWriteLock for Abort  when session is invalid */
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, tclock.AcquireWriteLock(session_id, write_operation));
}

TEST(TcLock, AcquireWriteLock_COMMIT) {
  TcLock tclock;
  uint32_t session_id = 100;
  std::string tc_config_name;
  tclock.UpdateConfigData(session_id, tc_config_name);
  tclock.tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  TcWriteOperation write_operation = TC_COMMIT;
  /* Testing AcquireWriteLock for COMMIT operation*/
  EXPECT_EQ(TC_LOCK_SUCCESS, tclock.AcquireWriteLock(session_id, write_operation));
}

TEST(TcLock, AcquireWriteLock_ABORT) {
  TcLock tclock;
  uint32_t session_id = 100;
  std::string tc_config_name;
  tclock.UpdateConfigData(session_id, tc_config_name);
  tclock.tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  TcWriteOperation write_operation = TC_ABORT_CANDIDATE_CONFIG;
  /* Testing AcquireWriteLock for ABORT */
  EXPECT_EQ(TC_LOCK_SUCCESS, tclock.AcquireWriteLock(session_id, write_operation));
}

TEST(TcLock, AcquireWriteLockForCommit_InvalidSessId) {
  TcLock tclock;
  uint32_t session_id = 100;
  /* Testing AcquireWriteLock for COMMIT*/
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, tclock.AcquireWriteLockForCommit(session_id));

}

TEST(TcLock, AcquireWriteLockForCommit) {
  TcLock tclock;
  uint32_t session_id = 100;
  std::string tc_config_name;
  tclock.UpdateConfigData(session_id, tc_config_name);
  tclock.tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  /* Testing AcquireWriteLock for COMMIT*/
  EXPECT_EQ(TC_LOCK_SUCCESS, tclock.AcquireWriteLockForCommit(session_id));

} 

TEST(TcLock, AcquireWriteLockForCommit_LockBusy) {
  TcLock tclock;
  uint32_t session_id = 100;
  std::string tc_config_name;
  tclock.UpdateConfigData(session_id, tc_config_name);
  /* Testing AcquireWriteLock for COMMIT*/
  EXPECT_EQ(TC_LOCK_BUSY, tclock.AcquireWriteLockForCommit(session_id));
}

TEST(TcLock, AcquireWriteLockForAbort_InvalidSessId) {
  TcLock tclock;
  uint32_t session_id = 100;
  /* Testing AcquireWriteLockForAbortCandidateConfig for Abort*/
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, tclock.AcquireWriteLockForAbortCandidateConfig(session_id));
}

TEST(TcLock, AcquireWriteLockForAbort) {
  TcLock tclock;
  uint32_t session_id = 100;
  std::string tc_config_name;
  tclock.UpdateConfigData(session_id, tc_config_name);
  tclock.tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  /* Testing AcquireWriteLockForAbortCandidateConfig for Abort */
  EXPECT_EQ(TC_LOCK_SUCCESS, tclock.AcquireWriteLockForAbortCandidateConfig(session_id));

}

TEST(TcLock, AcquireWriteLockForAbort_LockBusy) {
  TcLock tclock;
  uint32_t session_id = 100;
  std::string tc_config_name;
  tclock.UpdateConfigData(session_id, tc_config_name);
  /* Testing AcquireWriteLockForAbortCandidateConfig for Abort*/
  EXPECT_EQ(TC_LOCK_BUSY, tclock.AcquireWriteLockForAbortCandidateConfig(session_id));
}

TEST(TcLock, GetLock) {
  TcLock tclock;
  uint32_t session_id = 100;
  TcOperation tc_operation = TC_ACQUIRE_WRITE_SESSION;
  TcWriteOperation write_operation = TC_COMMIT;
  TcConfigMode tc_mode = TC_CONFIG_GLOBAL;
  std::string vtn_name = "";
  tclock.TcUpdateUncState(TC_ACT);
  /* Testing GetLock Fn for Commit when sessionid is invalid */
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, tclock.GetLock(session_id,tc_operation,write_operation,tc_mode,vtn_name));

  /* Testing GetLock Fn for Abort when sessionid is invalid */
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, tclock.GetLock(session_id,tc_operation,TC_ABORT_CANDIDATE_CONFIG,tc_mode,vtn_name));
  
}
TEST(TcLock, GetLock_Commit) {
  TcLock tclock;
  uint32_t session_id = 100;
  TcOperation tc_operation = TC_ACQUIRE_WRITE_SESSION;
  TcWriteOperation write_operation = TC_COMMIT;
  TcConfigMode tc_mode = TC_CONFIG_GLOBAL;
  std::string vtn_name = "";
  tclock.TcUpdateUncState(TC_ACT);
  std::string tc_config_name;
  tclock.UpdateConfigData(session_id, tc_config_name);
  tclock.tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  /* Testing GetLock for Commit */
  EXPECT_EQ(TC_LOCK_SUCCESS, tclock.GetLock(session_id,tc_operation,write_operation,tc_mode,vtn_name));
}

TEST(TcLock, GetLock_Abort) {
  TcLock tclock;
  uint32_t session_id = 100;
  TcOperation tc_operation = TC_ACQUIRE_WRITE_SESSION;
  TcWriteOperation write_operation = TC_ABORT_CANDIDATE_CONFIG;
  TcConfigMode tc_mode = TC_CONFIG_GLOBAL;
  std::string vtn_name = "";
  tclock.TcUpdateUncState(TC_ACT);
  std::string tc_config_name;
  tclock.UpdateConfigData(session_id, tc_config_name);
  tclock.tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  /* Testing GetLock for Abort */
  EXPECT_EQ(TC_LOCK_SUCCESS, tclock.GetLock(session_id,tc_operation,write_operation,tc_mode,vtn_name));
}
