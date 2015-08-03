/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcmsg.hh"

int OpArray[]=  {
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
    MSG_GET_DRIVERID
};

extern uint32_t resp_count;

TEST(TcMsg, CreateInstance) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_NONE;
  TcChannelNameMap daemon_names;
  /*invalid Opertype*/
  TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,  opertype,  daemon_names);
  EXPECT_EQ(NULL, tcmsg);

  daemon_names =  GetChannelNameMap(SET);
  int array_len =  (sizeof(OpArray)/sizeof(int));
  for (int i= CLEAR; i<= array_len-SET; i++)  {
    opertype = (TcMsgOperType) OpArray[i];
    tcmsg =  TcMsg::CreateInstance(sess_id,  opertype,  daemon_names);
    EXPECT_TRUE(NULL !=  tcmsg);
    if ( tcmsg !=  0 ) {
      delete tcmsg;
      tcmsg =  NULL;
    }
  }
}

TEST(MapTcDriverId, Execute) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_START;
  TestTcMsg tcmsg(sess_id,  opertype);

  TcDaemonName unc_ctrtype =  tcmsg.TestMapTcDriverId(UNC_CT_PFC);
  EXPECT_EQ(TC_DRV_OPENFLOW, unc_ctrtype);

  unc_ctrtype =  tcmsg.TestMapTcDriverId(UNC_CT_VNP);
  EXPECT_EQ(TC_DRV_OVERLAY, unc_ctrtype);

  unc_ctrtype =  tcmsg.TestMapTcDriverId(UNC_CT_UNKNOWN);
  EXPECT_EQ(TC_NONE, unc_ctrtype);
}

TEST(TcMsg, ForwardResponseToVTN) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_SETUP;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  pfc_ipcsrv_t *srv =  NULL;
  pfc::core::ipc::ServerSession s_sess(srv);
  pfc_ipcconn_t conn= 0;

  TestTcMsg tcmsg(sess_id,  opertype);
  pfc::core::ipc::ClientSession* c_sess
  =  TcClientSessionUtils::create_tc_client_session
  ("lgcnwd",
  unc::tclib::TCLIB_SETUP,
  conn);

  tcmsg.sess_ =  c_sess;
  retval =  tcmsg.ForwardResponseToVTN(s_sess);
  EXPECT_EQ(TCOPER_RET_SUCCESS, retval);

  tcmsg.sess_ =  NULL;
  tcmsg.upll_sess_ =  c_sess;
  resp_count =  3;
  retval =  tcmsg.ForwardResponseToVTN(s_sess);
  EXPECT_EQ(TCOPER_RET_SUCCESS, retval);

  tcmsg.sess_ =  NULL;
  tcmsg.upll_sess_ =  NULL;
  tcmsg.uppl_sess_ =  c_sess;
  resp_count =  3;
  retval =  tcmsg.ForwardResponseToVTN(s_sess);
  EXPECT_EQ(TCOPER_RET_SUCCESS, retval);
  /*if (c_sess !=  0) {
    delete c_sess;
    c_sess =  NULL;
  }*/
}

TEST(GetResult, Execute) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_SETUP;
  TestTcMsg tcmsg(sess_id,  opertype);
  unc_keytype_ctrtype_t driverid =  tcmsg.TestGetResult();
  EXPECT_EQ(UNC_CT_UNKNOWN, driverid);
}

TEST(RespondToTc, Execute) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_SETUP;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  pfc_ipcresp_t resp =  unc::tclib::TC_SUCCESS;
  int defaultval =  3;

  TestTcMsg tcmsg(sess_id,  opertype);

  retval =  tcmsg.TestRespondToTc(resp);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);

  resp =  unc::tclib::TC_FAILURE;
  retval =  tcmsg.TestRespondToTc(resp);
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);

  resp =  (pfc_ipcresp_t) defaultval;
  retval =  tcmsg.TestRespondToTc(resp);
  EXPECT_EQ(TCOPER_RET_UNKNOWN,  retval);
}

TEST(ReturnUtilResp, Execute) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_SETUP;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  TcUtilRet ret =  TCUTIL_RET_SUCCESS;
  int defaultval =  5;

  TestTcMsg tcmsg(sess_id,  opertype);

  retval =  tcmsg.TestReturnUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);

  ret =  TCUTIL_RET_FAILURE;
  retval =  tcmsg.TestReturnUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);

  ret =  TCUTIL_RET_FATAL;
  retval =  tcmsg.TestReturnUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);

  ret =  (TcUtilRet) defaultval;
  retval =  tcmsg.TestReturnUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_UNKNOWN,  retval);
}

TEST(AuditResult, Access) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  TestTcMsg tcmsg(sess_id,  opertype);

  TcAuditResult a_result =  TC_AUDIT_SUCCESS;
  retval =  tcmsg.SetAuditResult(a_result);
  EXPECT_EQ(TCOPER_RET_SUCCESS, retval);
  EXPECT_EQ(tcmsg.audit_result_, a_result);

  TcAuditResult out_result =  tcmsg.GetAuditResult();
  EXPECT_EQ(a_result, out_result);

  a_result =  (TcAuditResult)5;
  retval =  tcmsg.SetAuditResult(a_result);
  EXPECT_EQ(TCOPER_RET_FAILURE, retval);
}

TEST(TransResult, Access) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  TestTcMsg tcmsg(sess_id,  opertype);

  TcTransEndResult a_result =  TRANS_END_SUCCESS;
  tcmsg.SetTransResult(a_result);
  EXPECT_EQ(tcmsg.trans_result_, a_result);

  TcTransEndResult out_result =  tcmsg.GetTransResult();
  EXPECT_EQ(a_result, out_result);

  a_result =  (TcTransEndResult)5;
  retval =  tcmsg.SetTransResult(a_result);
  EXPECT_EQ(TCOPER_RET_FAILURE, retval);
}

/*GetControllerType cases*/
TEST(GetControllerType, Execute) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  unc_keytype_ctrtype_t ctrtype;
  string channel_name =  "drvpfcd";

  stub_response =  SET;
  retval =  TcMsg::GetControllerType(channel_name,  &ctrtype);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  EXPECT_EQ(UNC_CT_PFC,  ctrtype);
  stub_response =  CLEAR;

  stub_create_session =  SET;
  retval =  TcMsg::GetControllerType(channel_name,  &ctrtype);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;

  stub_session_invoke =  SET;
  retval =  TcMsg::GetControllerType(channel_name,  &ctrtype);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;
}

/*Class TcMsgSetUp cases*/
TEST(TcMsgSetUp, ExecuteSetup) {
  uint32_t sess_id =  CLEAR;
  TcMsgOperType oper =  MSG_SETUP;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  stub_set_arg = 0; 
  TcChannelNameMap daemon_names =  GetChannelNameMap(SET);

  TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,  oper,  daemon_names);
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }

  tcmsg =  TcMsg::CreateInstance(sess_id,
                                 MSG_SETUP_COMPLETE,
                                 GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }

  tcmsg =  TcMsg::CreateInstance(sess_id,  oper, GetChannelNameMap(CLEAR));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

/*Class TcMsgSetUp cases*/
TEST(TcMsgSetUp, ExecuteSetup_Fatal) {
  uint32_t sess_id =  CLEAR;
  TcMsgOperType oper =  MSG_SETUP;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  TcChannelNameMap daemon_names =  GetChannelNameMap(SET);
  stub_set_arg = 0;
  TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,  oper,  daemon_names);
  stub_session_invoke =  SET;
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

/*Class TcMsgNotifyConfigId cases*/
TEST(NotifyConfigId, Execute) {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_NOTIFY_CONFIGID;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  stub_set_string = 1;
  stub_set_arg = 0;
  stub_session_invoke = 0;
  stub_create_session =0;
  TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,
                                        oper,
                                        GetChannelNameMap(SET));
  tcmsg->SetData(SET, "", UNC_CT_UNKNOWN);
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }

  tcmsg =  TcMsg::CreateInstance(SET,
                                 MSG_NOTIFY_CONFIGID,
                                 GetChannelNameMap(CLEAR));
  tcmsg->SetData(SET, "", UNC_CT_UNKNOWN);
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }

  tcmsg =  TcMsg::CreateInstance(SET,
                                 MSG_NOTIFY_CONFIGID,
                                 GetChannelNameMap(SET));
  tcmsg->SetData(SET, "", UNC_CT_UNKNOWN);

  stub_create_session =  SET;
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;

  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }

  /*session-id =  0*/
    tcmsg =  TcMsg::CreateInstance(SET,
                                 MSG_NOTIFY_CONFIGID,
                                 GetChannelNameMap(SET));
  tcmsg->SetData(SET, "", UNC_CT_UNKNOWN);
  stub_set_arg = 0;
  stub_session_invoke = 0;
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);

  if ( tcmsg != 0 ) {
      delete tcmsg;
      tcmsg =  NULL;
  }

  /*config-id =  0*/
    tcmsg =  TcMsg::CreateInstance(SET,
                                 MSG_NOTIFY_CONFIGID,
                                 GetChannelNameMap(SET));
  tcmsg->SetData(CLEAR, "", UNC_CT_UNKNOWN);
  stub_set_arg = 0;
  stub_session_invoke = 0;
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);

  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

/*Class TcMsgNotifyConfigId cases*/
TEST(NotifyConfigId, Execute_Failure) {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_NOTIFY_CONFIGID;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  TcMsg* tcmsg = TcMsg::CreateInstance(sess_id, oper, GetChannelNameMap(SET));
  tcmsg->SetData(SET, "", UNC_CT_UNKNOWN);
  stub_set_string = 1;
  stub_session_invoke =  SET;
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

/*Class TcMsgNotifyConfigId cases*/
TEST(NotifyConfigId, Execute_Fatal) {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_NOTIFY_CONFIGID;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,
                                        oper,
                                        GetChannelNameMap(SET));
  tcmsg->SetData(SET, "", UNC_CT_UNKNOWN);
  stub_set_arg =  SET;
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

/*Class TcMsgToStartupDB cases*/
TEST(TcMsgToStartupDB, ExecuteSave) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  TcMsg* tcmsg =  TcMsg::CreateInstance(SET,
                                        MSG_SAVE_CONFIG,
                                        GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  if ( tcmsg != 0 ) {
      delete tcmsg;
        tcmsg =  NULL;
  }

  tcmsg =  TcMsg::CreateInstance(SET,
                                 MSG_CLEAR_CONFIG,
                                 GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  if ( tcmsg != 0 ) {
      delete tcmsg;
        tcmsg =  NULL;
  }

  /*empty channel name*/
  tcmsg =  TcMsg::CreateInstance(SET,
  MSG_CLEAR_CONFIG,
  GetChannelNameMap(CLEAR));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  if ( tcmsg != 0 ) {
      delete tcmsg;
        tcmsg =  NULL;
  }


  tcmsg =  TcMsg::CreateInstance(SET,
  MSG_CLEAR_CONFIG,
  GetChannelNameMap(SET));
  stub_create_session =  SET;
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }

  /*session_id =  0*/
  tcmsg =  TcMsg::CreateInstance(CLEAR,
  MSG_SAVE_CONFIG,
  GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  if ( tcmsg != 0 ) {
      delete tcmsg;
        tcmsg =  NULL;
  }
}

/*Class TcMsgToStartupDB cases*/
TEST(TcMsgToStartupDB, ExecuteSave_Failure) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  TcMsg* tcmsg =  TcMsg::CreateInstance(SET,
  MSG_SAVE_CONFIG,
  GetChannelNameMap(SET));
  stub_set_arg =  SET;
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;
  if ( tcmsg != 0 ) {
      delete tcmsg;
        tcmsg =  NULL;
  }
}

/*Class TcMsgToStartupDB cases*/
TEST(TcMsgToStartupDB, ExecuteSave_Fatal) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  TcMsg* tcmsg =  TcMsg::CreateInstance(SET,
  MSG_SAVE_CONFIG,
  GetChannelNameMap(SET));
  stub_session_invoke =  SET;
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;
  if ( tcmsg != 0 ) {
      delete tcmsg;
        tcmsg =  NULL;
  }
}

TEST(TcMsgAuditDB, Execute) {
  unc_keytype_datatype_t db =  UNC_DT_RUNNING;
  TcServiceType fail_oper =  TC_OP_USER_AUDIT;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  TcMsg* tcmsg =  TcMsg::CreateInstance(CLEAR,
  MSG_AUDITDB,
  GetChannelNameMap(SET));
  tcmsg->SetData(db, fail_oper);
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  if ( tcmsg != 0 ) {
      delete tcmsg;
        tcmsg =  NULL;
  }

  /*empty channel name*/
  tcmsg =  TcMsg::CreateInstance(CLEAR,
  MSG_AUDITDB,  GetChannelNameMap(CLEAR));
  tcmsg->SetData(UNC_DT_RUNNING,  TC_OP_USER_AUDIT);
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  if ( tcmsg != 0 ) {
      delete tcmsg;
        tcmsg =  NULL;
  }


  stub_create_session =  SET;
  tcmsg =  TcMsg::CreateInstance(CLEAR,
  MSG_AUDITDB,
  GetChannelNameMap(SET));
  tcmsg->SetData(UNC_DT_RUNNING,  TC_OP_USER_AUDIT);
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }

  stub_session_invoke =  SET;
  tcmsg =  TcMsg::CreateInstance(CLEAR,
  MSG_AUDITDB,
  GetChannelNameMap(SET));
  tcmsg->SetData(UNC_DT_RUNNING,  TC_OP_USER_AUDIT);
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;
  if ( tcmsg != 0 ) {
        delete tcmsg;
            tcmsg =  NULL;
  }

  stub_set_arg =  SET;
  tcmsg =  TcMsg::CreateInstance(CLEAR,
  MSG_AUDITDB,
  GetChannelNameMap(SET));
  tcmsg->SetData(UNC_DT_RUNNING,  TC_OP_USER_AUDIT);
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;
  if ( tcmsg != 0 ) {
        delete tcmsg;
            tcmsg =  NULL;
  }
}

TEST(CommitSendAbortRequest, Execute)  {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_COMMIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  TestCommit Ctest(sess_id,  opertype);
  Ctest.channel_names_ =  GetChannelNameMap(SET);

  Ctest.config_id_ =  SET; Ctest.session_id_ =  1;
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);


  stub_create_session = SET;
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;

  stub_set_arg =  SET;
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;

  stub_session_invoke =  SET;
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;

  stub_response = SET;
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_response = CLEAR;
}

TEST(CommitSendTransEndRequest, Execute)  {
#if 1
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_COMMIT_TRANS_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  TestCommit Ctest(sess_id,  opertype);
  Ctest.channel_names_ =  (GetChannelNameMap(SET));
  /*config_id/session_id validation*/
  Ctest.config_id_ =  CLEAR;
  retval =  Ctest.TestSendTransEndRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
#endif

  Ctest.session_id_ =  CLEAR;
  retval =  Ctest.TestSendTransEndRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);

  Ctest.config_id_ =  SET; Ctest.session_id_ =  1;
  retval =  Ctest.TestSendTransEndRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);

  stub_create_session = SET;
  retval =  Ctest.TestSendTransEndRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;

  stub_set_arg =  SET;
  retval =  Ctest.TestSendTransEndRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;

  stub_session_invoke =  SET;
  retval =  Ctest.TestSendTransEndRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;

  stub_response = SET;
  retval =  Ctest.TestSendTransEndRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_response = CLEAR;
  
}

/*Class AbortCandidateDB cases*/
TEST(AbortCandidateDB, Execute) {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_ABORT_CANDIDATE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

//  AbortCandidateDB *CAbort = new TcMsgCommit(sess_id, oper);

  AbortCandidateDB CAbort(sess_id,  oper);
  CAbort.channel_names_ =  GetChannelNameMap(SET);
  CAbort.TcMsgCommit::SetData(SET, "", UNC_CT_UNKNOWN);

  retval =  CAbort.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);

  CAbort.channel_names_ =  GetChannelNameMap(CLEAR);
  retval =  CAbort.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);

  CAbort.channel_names_ =  GetChannelNameMap(SET);
  stub_create_session =  SET;
  retval =  CAbort.Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;

  AbortCandidateDB CAbort1(CLEAR,  MSG_ABORT_CANDIDATE);
  CAbort1.channel_names_ =  GetChannelNameMap(SET);
  CAbort1.TcMsgCommit::SetData(SET, "", UNC_CT_UNKNOWN);
  retval =  CAbort1.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

/*Class AbortCandidateDB cases*/
TEST(AbortCandidateDB, Execute_Noargs) {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_ABORT_CANDIDATE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  AbortCandidateDB CAbort(sess_id,  oper);
  CAbort.channel_names_ =  GetChannelNameMap(SET);

  CAbort.TcMsgCommit::SetData(CLEAR, "", UNC_CT_UNKNOWN);
  retval =  CAbort.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

/*Class AbortCandidateDB cases*/
TEST(AbortCandidateDB, Execute_Fatal) {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_ABORT_CANDIDATE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  AbortCandidateDB CAbort(sess_id,  oper);
  CAbort.channel_names_ =  GetChannelNameMap(SET);

  stub_session_invoke =  SET;
  retval =  CAbort.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_session_invoke =  CLEAR;
}

/*Class AbortCandidateDB cases*/
TEST(AbortCandidateDB, Execute_Failure) {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_ABORT_CANDIDATE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  AbortCandidateDB CAbort(sess_id,  oper);
  CAbort.channel_names_ =  GetChannelNameMap(SET);

  CAbort.TcMsgCommit::SetData(SET, "", UNC_CT_UNKNOWN);
  stub_set_arg =  SET;
  retval =  CAbort.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;
}

TEST(CommitTransaction, Execute) {
  TcMsgOperType oper =  MSG_COMMIT_TRANS_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  /*sess id =  0*/
  CommitTransaction Commit(0,  oper);
  Commit.channel_names_ =  GetChannelNameMap(SET);
  retval =  Commit.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

TEST(CommitTransaction, Execute_Failure) {
  TcMsgOperType oper =  MSG_COMMIT_TRANS_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  /*config id =  0 */
  CommitTransaction Commit(0,  oper);
  Commit.session_id_ =  SET;
  Commit.channel_names_ =  GetChannelNameMap(SET);
  Commit.SetData(0, "", UNC_CT_UNKNOWN);
  retval =  Commit.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

TEST(CommitTransaction, Execute_Failure_transstart) {
  // TcMsgOperType oper =  MSG_COMMIT_TRANS_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  /*invalid opertype*/
  CommitTransaction Commit1(SET,  MSG_NONE);
  Commit1.SetData(1, "", UNC_CT_UNKNOWN);
  Commit1.channel_names_ =  GetChannelNameMap(SET);
  retval =  Commit1.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

TEST(CommitTransaction, Execute_TransStart) {
  // TcMsgOperType oper =  MSG_COMMIT_TRANS_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  /*proper request*/
  CommitTransaction Commit2(SET,  MSG_COMMIT_TRANS_START);
  Commit2.SetData(SET, "", UNC_CT_UNKNOWN);
  Commit2.channel_names_ =  GetChannelNameMap(SET);
  retval =  Commit2.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(CommitTransaction, Execute_TransEnd) {
  // TcMsgOperType oper =  MSG_COMMIT_TRANS_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  CommitTransaction Commit3(SET,  MSG_COMMIT_TRANS_END);
  Commit3.SetData(SET, "", UNC_CT_UNKNOWN);
  Commit3.channel_names_ =  GetChannelNameMap(SET);
  retval =  Commit3.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(CommitTransaction, SendRequest)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestCommitTrans Commit(SET,  MSG_COMMIT_TRANS_START);
  Commit.SetData(SET, "", UNC_CT_UNKNOWN);

  stub_create_session =  SET;
  retval =  Commit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;
}

TEST(CommitTransaction, SendRequest_Fatal)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestCommitTrans Commit(SET,  MSG_COMMIT_TRANS_START);
  Commit.SetData(SET, "", UNC_CT_UNKNOWN);

  stub_create_session =  SET;
  retval =  Commit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;
}

TEST(CommitTransaction, SendRequest_Failure)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestCommitTrans Commit(SET,  MSG_COMMIT_TRANS_START);
  Commit.SetData(SET, "", UNC_CT_UNKNOWN);

  stub_set_arg =  SET;
  retval =  Commit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;
}

TEST(CommitTransaction, SendRequest_Success)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestCommitTrans Commit(SET,  MSG_COMMIT_TRANS_START);
  Commit.SetData(SET, "", UNC_CT_UNKNOWN);

  Commit.opertype_ =  MSG_COMMIT_TRANS_END;
  retval =  Commit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(CommitTransaction, SendRequest_EndFailure)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestCommitTrans Commit(SET,  MSG_COMMIT_TRANS_START);
  Commit.SetData(SET, "", UNC_CT_UNKNOWN);

  stub_response =  SET;
  Commit.opertype_ =  MSG_COMMIT_TRANS_START;
  retval =  Commit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_ABORT,  retval);
  EXPECT_EQ(TRANS_END_FAILURE, Commit.trans_result_);
}

TEST(CommitTransaction, SendRequest_RetAbort)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestCommitTrans Commit(SET,  MSG_COMMIT_TRANS_START);
  Commit.SetData(SET, "", UNC_CT_UNKNOWN);

  retval =  Commit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_ABORT,  retval);
  EXPECT_EQ(TRANS_END_FAILURE, Commit.trans_result_);
}


TEST(CommitTransaction, SendRequest_RetFatal)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestCommitTrans Commit(SET,  MSG_COMMIT_TRANS_START);
  Commit.SetData(SET, "", UNC_CT_UNKNOWN);

  Commit.opertype_ =  MSG_COMMIT_TRANS_END;
  retval =  Commit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_response =  CLEAR;
}

TEST(CommitTransaction, SendRequest_RetSuccess)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestCommitTrans Commit(SET,  MSG_COMMIT_TRANS_START);
  Commit.SetData(SET, "", UNC_CT_UNKNOWN);

  Commit.opertype_ =  MSG_COMMIT_TRANS_START;
  Commit.channel_names_ =  GetChannelNameMap(SET);
  retval =  Commit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(TwoPhaseCommit,  Execute)  {
  uint32_t sess_id =  2;
  TcMsgOperType oper =  MSG_COMMIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  Test2phase C2phase(sess_id,  oper);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  /*driverinfo map is empty*/
  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  retval =  C2phase.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS, retval);
}

TEST(TwoPhaseCommit,  Execute_Failure)  {
  uint32_t sess_id =  2;
  TcMsgOperType oper =  MSG_COMMIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  Test2phase C2phase(sess_id,  oper);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  /*config-id is 0*/
  C2phase.SetData(CLEAR, "", UNC_CT_UNKNOWN);
  retval =  C2phase.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE, retval);
}

TEST(TwoPhaseCommit,  Execute_SessionForward)  {
  uint32_t sess_id =  2;
  TcMsgOperType oper =  MSG_COMMIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  Test2phase C2phase(sess_id,  oper);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  /*stub - getrespcount and sess->forward*/
  resp_count =  3;
  stub_clnt_forward =  SET;
  retval =  C2phase.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE, retval);
  stub_clnt_forward =  CLEAR;
}

TEST(TwoPhaseCommit,  Execute_FailureCommitVote)  {
  uint32_t sess_id =  2;
  TcMsgOperType oper =  MSG_COMMIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  Test2phase C2phase(sess_id,  oper);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  /*config-id is 0*/
  C2phase.SetData(CLEAR, "", UNC_CT_UNKNOWN);
  retval =  C2phase.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE, retval);
}

TEST(TwoPhaseCommit,  Execute_Commit)  {
  uint32_t sess_id =  2;
  stub_set_string = 1;
  TcMsgOperType oper =  MSG_COMMIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  Test2phase C2phase(sess_id,  oper);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  oper =  MSG_COMMIT_GLOBAL;
  Test2phase C2phase2(sess_id,  oper);
  C2phase2.channel_names_ =  GetChannelNameMap(SET);
  C2phase2.SetData(SET, "", UNC_CT_UNKNOWN);
  resp_count =  3;
  stub_clnt_forward =  SET;
  retval =  C2phase2.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS, retval);
  stub_clnt_forward =  CLEAR;
}

TEST(GetControllerInfo,  Execute)  {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_COMMIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "tclib";
  pfc_ipcconn_t conn =  0;

  Test2phase C2phase(sess_id, oper);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);

  retval =  C2phase.TestGetControllerInfo(c_sess);
  EXPECT_EQ(TCOPER_RET_SUCCESS, retval);
  if (c_sess !=  0) {
    delete c_sess;
    c_sess =  NULL;
  }
}

TEST(TwoPhaseCommit, SendRequest) {
 /* uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_COMMIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "lgcnwd";

  Test2phase C2phase(sess_id, oper);

  stub_create_session =  SET;
  retval =  C2phase.TestSendRequest(channel_name);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;

  stub_set_arg =  SET;
  retval =  C2phase.TestSendRequest(channel_name);
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;

  stub_session_invoke =  SET;
  retval =  C2phase.TestSendRequest(channel_name);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;

  stub_response =  SET;
  EXPECT_EQ(TRANS_END_SUCCESS, C2phase.trans_result_);
  retval =  C2phase.TestSendRequest(channel_name);
  EXPECT_EQ(TCOPER_RET_ABORT,  retval);
  EXPECT_EQ(TRANS_END_FAILURE, C2phase.trans_result_);

  retval =  C2phase.TestSendRequest(channel_name);
  EXPECT_EQ(TCOPER_RET_ABORT,  retval);
  EXPECT_EQ(TRANS_END_FAILURE, C2phase.trans_result_);

  Test2phase C2phase2(sess_id, MSG_COMMIT_GLOBAL);
  retval =  C2phase2.TestSendRequest(channel_name);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  EXPECT_EQ(TRANS_END_SUCCESS, C2phase2.trans_result_);
  stub_response =  CLEAR;
  */
}

TEST(TwoPhaseCommit, SendRequestToDriver) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "drvpfcd";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;
  stub_set_string = 1;
  Test2phase C2phase(SET,  MSG_COMMIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);
  retval =  C2phase.TestGetControllerInfo(c_sess);

  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap(SET);
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);

  /*stub_create_session =  SET;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_create_session =  CLEAR;

  stub_set_arg =  SET;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;

  stub_session_invoke =  SET;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;

  stub_response =  SET;
  EXPECT_EQ(TRANS_END_SUCCESS,  C2phase.trans_result_);
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  EXPECT_EQ(TRANS_END_FAILURE,  C2phase.trans_result_);
  stub_response =  CLEAR;*/
  if (c_sess !=  0) {
    delete c_sess;
    c_sess =  NULL;
  }
}

TEST(TwoPhaseCommit, SendRequestToDriver2) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "drvpfcd";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;

  Test2phase C2phase(SET,  MSG_COMMIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);
  retval =  C2phase.TestGetControllerInfo(c_sess);
  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  stub_create_session =  SET;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_create_session =  CLEAR;
  if (c_sess !=  0) {
    delete c_sess;
    c_sess =  NULL;
  }
}

TEST(TwoPhaseCommit, SendRequestToDriver3) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "drvpfcd";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;

  Test2phase C2phase(SET,  MSG_COMMIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);
  stub_set_string = 1;
  retval =  C2phase.TestGetControllerInfo(c_sess);

  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(0,  retval);
  stub_set_arg =  CLEAR;

  if (c_sess !=  0) {
    delete c_sess;
    c_sess =  NULL;
  }
}

TEST(TwoPhaseCommit, SendRequestToDriver4) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "drvpfcd";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;
  stub_set_string = 1;
  Test2phase C2phase(SET,  MSG_COMMIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                             unc::tclib::TCLIB_COMMIT_TRANSACTION,
                             conn);
  
  retval =  C2phase.TestGetControllerInfo(c_sess);
  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  stub_session_invoke =  SET;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;

  if (c_sess !=  0) {
    delete c_sess;
    c_sess =  NULL;
  }
}

TEST(AuditSendAbortRequest, Execute)  {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  TestAudit Ctest(sess_id,  opertype);
  Ctest.channel_names_ =  GetChannelNameMap(SET);
  Ctest.driver_id_ =  UNC_CT_PFC; Ctest.controller_id_ =  "C1";
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  stub_set_string = 1;
  stub_create_session = SET;
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;

  stub_set_arg =  SET;
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;

  stub_session_invoke =  SET;
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;

  stub_response = SET;
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_response = CLEAR;
}

TEST(AuditSendAuditTransEndRequest, Execute)  {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_TRANS_END;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  stub_set_string = 1;
  TestAudit Ctest(sess_id,  opertype);
  Ctest.channel_names_ =  (GetChannelNameMap(SET));
  Ctest.controller_id_ =  "C1";
  Ctest.driver_id_ =  UNC_CT_PFC;
  retval =  Ctest.TestSendAuditTransEndRequest(PopulateAbortVector(),
  MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);

  stub_create_session = SET;
  retval =  Ctest.TestSendAuditTransEndRequest(PopulateAbortVector(),
  MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;

  stub_set_arg =  SET;
  retval =  Ctest.TestSendAuditTransEndRequest(PopulateAbortVector(),
  MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;

  stub_session_invoke =  SET;
  retval =  Ctest.TestSendAuditTransEndRequest(PopulateAbortVector(),
  MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;

  stub_response = SET;
  retval =  Ctest.TestSendAuditTransEndRequest(PopulateAbortVector(),
  MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_response = CLEAR;
}

TEST(GetDriverID, Execute)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  stub_set_string = 1;
  GetDriverId getDrv(SET, MSG_GET_DRIVERID);
  getDrv.channel_names_ =  GetChannelNameMap(SET);
  getDrv.controller_id_ =  "C1";
  retval =  getDrv.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(AuditTransaction, Execute) {
  TcMsgOperType oper =  MSG_AUDIT_TRANS_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  stub_set_string = 1;
  AuditTransaction Audit(SET,  oper);
  Audit.channel_names_ =  GetChannelNameMap(SET);
  Audit.driver_id_ =  UNC_CT_UNKNOWN;
  retval =  Audit.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);

  Audit.channel_names_ =  GetChannelNameMap(SET);
  Audit.SetData(SET, "", UNC_CT_PFC);
  retval =  Audit.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);

  /*invalid opertype*/
  AuditTransaction Audit1(SET,  MSG_NONE);
  Audit1.SetData(1, "C1", UNC_CT_PFC);
  Audit1.channel_names_ =  GetChannelNameMap(SET);
  retval =  Audit1.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);

  /*proper request*/
  AuditTransaction Audit2(SET,  MSG_AUDIT_START);
  Audit2.SetData(SET, "C1", UNC_CT_PFC);
  Audit2.channel_names_ =  GetChannelNameMap(SET);
  retval =  Audit2.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(AuditTransaction, Execute_RetSuccess) {
  // TcMsgOperType oper =  MSG_AUDIT_TRANS_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  stub_set_string = 1;
  AuditTransaction Audit3(SET,  MSG_AUDIT_TRANS_START);
  Audit3.SetData(SET, "C1", UNC_CT_PFC);
  Audit3.channel_names_ =  GetChannelNameMap(SET);
  retval =  Audit3.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);

  stub_create_session =  SET;
  retval =  Audit3.Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session=  CLEAR;
}

TEST(AuditTransaction, SendRequest)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestAuditTrans Audit(SET,  MSG_AUDIT_TRANS_START);
  Audit.SetData(SET, "C1", UNC_CT_PFC);
  stub_create_session =  SET;
  retval =  Audit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;
}

TEST(AuditTransaction, SendRequest_RetFailure)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";
  
  TestAuditTrans Audit(SET,  MSG_AUDIT_TRANS_START);

  stub_set_arg =  SET;
  retval =  Audit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;
}

TEST(AuditTransaction, SendRequest_RetFatal)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";
  stub_set_string = 1;
  TestAuditTrans Audit(SET,  MSG_AUDIT_TRANS_START);

  stub_session_invoke =  SET;
  retval =  Audit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;
}

TEST(AuditTransaction, SendRequest_RetSuccess)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";
  stub_set_string = 1;
  TestAuditTrans Audit(SET,  MSG_AUDIT_TRANS_START);

  Audit.opertype_ =  MSG_AUDIT_TRANS_END;
  retval =  Audit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(AuditTransaction, SendRequest_Resp_EndFailure)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";
  stub_set_string = 1;
  TestAuditTrans Audit(SET,  MSG_AUDIT_TRANS_START);

  stub_response =  SET;
  Audit.opertype_ =  MSG_AUDIT_TRANS_START;
  retval =  Audit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_ABORT,  retval);
  EXPECT_EQ(TRANS_END_FAILURE, Audit.trans_result_);
}

TEST(AuditTransaction, SendRequest_RetAbort)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";
  stub_set_string = 1;
  TestAuditTrans Audit(SET,  MSG_AUDIT_TRANS_START);

  retval =  Audit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_ABORT,  retval);
  EXPECT_EQ(TRANS_END_FAILURE, Audit.trans_result_);
}

TEST(AuditTransaction, SendRequest_TransEndFatal)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestAuditTrans Audit(SET,  MSG_AUDIT_TRANS_START);
  stub_set_string = 1;
  Audit.opertype_ =  MSG_AUDIT_TRANS_END;
  retval =  Audit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_response =  CLEAR;
}

TEST(AuditTransaction, SendRequestTransStart_Success)  {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channelname =  "drvpfcd";

  TestAuditTrans Audit(SET,  MSG_AUDIT_TRANS_START);
  stub_set_string = 1;
  Audit.opertype_ =  MSG_AUDIT_TRANS_START;
  Audit.channel_names_ =  GetChannelNameMap(SET);
  retval =  Audit.TestSendRequest(channelname);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(TwoPhaseAudit,  Execute)  {
  uint32_t sess_id =  2;
  TcMsgOperType oper =  MSG_AUDIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  stub_set_string = 1;
  Test2phaseAudit C2phase(sess_id,  oper);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  /*invalid driver id*/
  C2phase.SetData(CLEAR, "", UNC_CT_UNKNOWN);
  retval =  C2phase.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE, retval);
  /*controller id is empty*/
  C2phase.SetData(SET, "", UNC_CT_PFC);
  retval =  C2phase.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE, retval);
  /*stub - getrespcount and sess->forward*/
  resp_count =  3;
  stub_clnt_forward =  SET;
  retval =  C2phase.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE, retval);
  stub_clnt_forward =  CLEAR;

  oper =  MSG_AUDIT_GLOBAL;
  Test2phaseAudit C2phase2(sess_id,  oper);
  C2phase2.channel_names_ =  GetChannelNameMap(SET);
  C2phase2.SetData(SET, "C1", UNC_CT_PFC);
  resp_count =  3;
  stub_clnt_forward =  SET;
  retval =  C2phase2.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS, retval);
  stub_clnt_forward =  CLEAR;
}

TEST(TwoPhaseAudit, SendRequest) {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_AUDIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "lgcnwd";

  Test2phaseAudit C2phase(sess_id, oper);
  stub_create_session =  SET;
  retval =  C2phase.TestSendRequest(channel_name);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_create_session =  CLEAR;
}

TEST(TwoPhaseAudit, SendRequest_ReqFatal_SessionInvoke) {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_AUDIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "lgcnwd";
  stub_set_string = 1;
  Test2phaseAudit C2phase(sess_id, oper);
  stub_session_invoke =  SET;
  retval =  C2phase.TestSendRequest(channel_name);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;
}

TEST(TwoPhaseAudit, SendRequest_ReqFailure) {
  uint32_t sess_id =  SET;
  TcMsgOperType oper =  MSG_AUDIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "lgcnwd";

  Test2phaseAudit C2phase(sess_id, oper);
  // stub_create_session =  SET;
  stub_set_arg =  SET;
  retval =  C2phase.TestSendRequest(channel_name);
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;
}

TEST(TwoPhaseAudit, SendRequestToDriver) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "drvpfcd";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;
  Test2phaseAudit C2phase(SET,  MSG_AUDIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                           unc::tclib::TCLIB_AUDIT_TRANSACTION,
                           conn);
  stub_set_string = 1;
  retval =  C2phase.TestGetControllerInfo(c_sess);
  C2phase.SetData(SET, "C1", UNC_CT_PFC);
  C2phase.channel_names_ =  GetChannelNameMap(SET);
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  EXPECT_EQ(TRANS_END_SUCCESS, C2phase.trans_result_);
  EXPECT_EQ(TC_AUDIT_SUCCESS, C2phase.audit_result_);

  if (c_sess !=  0) {
        delete c_sess;
        c_sess =  NULL;
  }
}

TEST(TwoPhaseAudit, SendRequestToDriver_Failure) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "drvpfcd";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;

  Test2phaseAudit C2phase(SET,  MSG_AUDIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                               unc::tclib::TCLIB_AUDIT_TRANSACTION,
                               conn);

  retval =  C2phase.TestGetControllerInfo(c_sess);
  C2phase.SetData(SET, "C1", UNC_CT_PFC);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  stub_create_session =  SET;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_create_session =  CLEAR;

  if (c_sess !=  0) {
        delete c_sess;
        c_sess =  NULL;
  }
}

TEST(TwoPhaseAudit, SendRequestToDriver_Failure2) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "drvpfcd";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;

  Test2phaseAudit C2phase(SET,  MSG_AUDIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                             unc::tclib::TCLIB_AUDIT_TRANSACTION,
                             conn);

  retval =  C2phase.TestGetControllerInfo(c_sess);
  C2phase.SetData(SET, "C1", UNC_CT_PFC);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  stub_set_arg =  SET;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_arg =  CLEAR;

  if (c_sess !=  0) {
        delete c_sess;
        c_sess =  NULL;
  }
}

TEST(TwoPhaseAudit, SendRequestToDriver_Fatal) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "drvpfcd";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;

  Test2phaseAudit C2phase(SET,  MSG_AUDIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_AUDIT_TRANSACTION,
                                conn);
  stub_set_string = 1;
  retval =  C2phase.TestGetControllerInfo(c_sess);
  C2phase.SetData(SET, "C1", UNC_CT_PFC);
  C2phase.channel_names_ =  GetChannelNameMap(SET);

  stub_session_invoke =  SET;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  stub_session_invoke =  CLEAR;

  if (c_sess !=  0) {
        delete c_sess;
        c_sess =  NULL;
  }
}

TEST(TcMsgSetUp, Execute_MSG_SETUP) {
uint32_t sess_id =  SET;
unc::tclib::TcMsgOperType oper = MSG_SETUP;
stub_set_arg = 0;
TcOperRet retval =  TCOPER_RET_SUCCESS;
TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,
                                        oper,
                                        GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

TEST(TcMsgSetUp, Execute_MSG_SETUP_COMPLETE) {
uint32_t sess_id =  SET;
unc::tclib::TcMsgOperType oper = MSG_SETUP_COMPLETE;
stub_set_arg = 0;
TcOperRet retval =  TCOPER_RET_SUCCESS;
TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,
                                        oper,
                                        GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

/*session_invoke failed*/
TEST(TcMsgSetUp, Execute_Failure) {
uint32_t sess_id =  SET;
stub_session_invoke =1;
unc::tclib::TcMsgOperType oper = MSG_SETUP;
stub_set_arg = 0;
TcOperRet retval =  TCOPER_RET_SUCCESS;
TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,
                                        oper,
                                        GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

/*create client session is NULL*/
TEST(TcMsgSetUp, Execute_ClientSessionNull) {
uint32_t sess_id =  SET;
stub_create_session = 1;
unc::tclib::TcMsgOperType oper = MSG_SETUP;

TcOperRet retval =  TCOPER_RET_SUCCESS;
TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,
                                        oper,
                                        GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

/*set_uint failed*/
TEST(TcMsgSetUp, Execute_set_uint_Failure) {
uint32_t sess_id =  SET;
stub_set_arg = 1;
unc::tclib::TcMsgOperType oper = MSG_SETUP;

TcOperRet retval =  TCOPER_RET_SUCCESS;
TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,
                                        oper,
                                        GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}


TEST(TcMsgAutoSave, Execute_AutoSaveEnabled) {
uint32_t sess_id =  SET;
unc::tclib::TcMsgOperType oper = MSG_AUTOSAVE_ENABLE;
TcOperRet retval =  TCOPER_RET_SUCCESS;
TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,
                                        oper,
                                        GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

TEST(TcMsgAutoSave, Execute_AutoSaveDisabled) {
uint32_t sess_id =  SET;
unc::tclib::TcMsgOperType oper = MSG_AUTOSAVE_DISABLE;
TcOperRet retval =  TCOPER_RET_SUCCESS;
TcMsg* tcmsg =  TcMsg::CreateInstance(sess_id,
                                        oper,
                                        GetChannelNameMap(SET));
  retval =  tcmsg->Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  if ( tcmsg != 0 ) {
    delete tcmsg;
    tcmsg =  NULL;
  }
}

TEST(MapTcDriverId, Execute_POLC) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_START;
  TestTcMsg tcmsg(sess_id,  opertype);

  TcDaemonName unc_ctrtype =  tcmsg.TestMapTcDriverId(UNC_CT_POLC);
  EXPECT_EQ(TC_DRV_POLC, unc_ctrtype);
}

TEST(ReturnUtilResp, ReturnUtilResp_Success) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_SETUP;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  TcUtilRet ret =  TCUTIL_RET_SUCCESS;
  TestTcMsg tcmsg(sess_id,  opertype);
  retval =  tcmsg.TestReturnUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(ReturnUtilResp, ReturnUtilResp_Failure) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_SETUP;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  TcUtilRet ret =  TCUTIL_RET_FAILURE;
  TestTcMsg tcmsg(sess_id,  opertype);
  retval =  tcmsg.TestReturnUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

TEST(ReturnUtilResp, ReturnUtilResp_Fatal) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_SETUP;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  TcUtilRet ret =  TCUTIL_RET_FATAL;
  TestTcMsg tcmsg(sess_id,  opertype);
  retval =  tcmsg.TestReturnUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
}

TEST(ReturnUtilResp, ReturnUtilResp_Unknown) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_SETUP;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  int defaultval =  5;
  TcUtilRet ret = (TcUtilRet) defaultval;
  TestTcMsg tcmsg(sess_id,  opertype);
  retval =  tcmsg.TestReturnUtilResp(ret);
  EXPECT_EQ(TCOPER_RET_UNKNOWN,  retval);
}

TEST(TwoPhaseCommit, SendReqToDvr_NoDvr_set_uint32_failed) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  //std::string channel_name =  "drvpfcd";
  std::string channel_name = "";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;

  Test2phase C2phase(SET,  MSG_COMMIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);
  retval =  C2phase.TestGetControllerInfo(c_sess);

  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap(SET);
  stub_set_arg = 1;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

TEST(TwoPhaseCommit, SendReqToDvr_NoDvr_set_string_failed) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  //std::string channel_name =  "drvpfcd";
  std::string channel_name = "";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;

  Test2phase C2phase(SET,  MSG_COMMIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);
  retval =  C2phase.TestGetControllerInfo(c_sess);
  stub_set_string = 0;
  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap(SET);
  //stub_set_arg = 1;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
  stub_set_string = 1;
}

TEST(TwoPhaseCommit, SendReqToDvr_NoDvr_set_uint8_fail) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  //std::string channel_name =  "drvpfcd";
  std::string channel_name = "";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;

  Test2phase C2phase(SET,  MSG_COMMIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);
  retval =  C2phase.TestGetControllerInfo(c_sess);
  stub_set_string = 0;
  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap(SET);
  stub_set_arg = 1;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

TEST(TwoPhaseCommit, SendReqToDvr_Invalid_Session) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  //std::string channel_name =  "drvpfcd";
  std::string channel_name = "";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;

  Test2phase C2phase(SET,  MSG_COMMIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);
  retval =  C2phase.TestGetControllerInfo(c_sess);
  stub_set_string = 0;
  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap(SET);
  stub_set_arg = 0;
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

TEST(AuditSendAbortRequest,Execute_DrvID_POLC)  {
  uint32_t sess_id =  SET;
  stub_set_string = 1;
  TcMsgOperType opertype =  MSG_AUDIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  TestAudit Ctest(sess_id,  opertype);
  Ctest.channel_names_ =  GetChannelNameMap(SET);
  Ctest.driver_id_ =  UNC_CT_POLC; 
  Ctest.controller_id_ =  "C1";
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}
//Driver id Less than POLC
TEST(AuditSendAbortRequest,Execute_DrvID_VNP)  {
  uint32_t sess_id =  SET;
  stub_set_string =1;
  TcMsgOperType opertype =  MSG_AUDIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  TestAudit Ctest(sess_id,  opertype);
  Ctest.channel_names_ =  GetChannelNameMap(SET);
  Ctest.driver_id_ =  UNC_CT_VNP;
  Ctest.controller_id_ =  "C1";
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
  stub_set_string = 0;
}

//Driver id greater than POLC
TEST(AuditSendAbortRequest,Execute_DrvID_MAX)  {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_VOTE;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  TestAudit Ctest(sess_id,  opertype);
  Ctest.channel_names_ =  GetChannelNameMap(SET);
  //Ctest.driver_id_ =  UNC_CT_MAX;
  Ctest.controller_id_ =  "C1";
  retval =  Ctest.TestSendAbortRequest(PopulateAbortVector());
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

TEST(AuditSendAuditTransEndRequest, Execute_DrvID_POLC)  {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_TRANS_END;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  stub_set_string = 1;
  TestAudit Ctest(sess_id,  opertype);
  Ctest.channel_names_ =  (GetChannelNameMap(SET));
  Ctest.controller_id_ =  "C1";
  Ctest.driver_id_ =  UNC_CT_POLC;
  retval =  Ctest.TestSendAuditTransEndRequest(PopulateAbortVector(),
  MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(AuditSendAuditTransEndRequest, Execute_DrvID_VNP)  {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_TRANS_END;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  stub_set_string = 1;
  TestAudit Ctest(sess_id,  opertype);
  Ctest.channel_names_ =  (GetChannelNameMap(SET));
  Ctest.controller_id_ =  "C1";
  Ctest.driver_id_ =  UNC_CT_VNP;
  retval =  Ctest.TestSendAuditTransEndRequest(PopulateAbortVector(),
  MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

TEST(AuditSendAuditTransEndRequest, Execute_DrvID_MAX)  {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_TRANS_END;
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  stub_set_string = 1;
  TestAudit Ctest(sess_id,  opertype);
  Ctest.channel_names_ =  (GetChannelNameMap(SET));
  Ctest.controller_id_ =  "C1";
//  Ctest.driver_id_ =  UNC_CT_MAX;
  retval =  Ctest.TestSendAuditTransEndRequest(PopulateAbortVector(),
  MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

#if 0
TEST(TwoPhaseCommit, SendRequestToDriver_HandleDriverNotPresent_Failure) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  std::string channel_name =  "";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;
  stub_set_string = 1;
  Test2phase C2phase(SET,  MSG_COMMIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);
  retval =  C2phase.TestGetControllerInfo(c_sess);

  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap(SET);
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_FATAL,  retval);
}

TEST(TwoPhaseCommit, TestHandleDriver_Keytype_POLC_Success) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_START;
  Test2phase C2phase(sess_id,  opertype);
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  std::string channel_name = "";
  //TestTwophaseAudit Test2PhaseAuditObj(sess_id,  opertype);
  unc_keytype_ctrtype_t driver_type = UNC_CT_POLC;
  //typedef std::list<std::string> TestControllerList;
  //TestControllerList controllers_test;
  //controllers_test.push_back("sample1");
  //controllers_test.push_back("sample2");
  //controllers_test.push_back("sample3");
  //unc::tc::TcDriverInfoMap driverinfo_map_;
  //Test2PhaseAuditObj.driverinfo_map_.insert(std::pair<unc_keytype_ctrtype_t,
    //                         TestControllerList>(driver_type, controllers_test));
  //driverinfo_map_.insert(std::pair<unc_keytype_ctrtype_t, std::vector<std::string>('UNC_CT_POLC',"sample") );
  
  ControllerList clist;
  pfc_ipcconn_t conn =  0;
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);
  ret_val =  C2phase.TestGetControllerInfo(c_sess);

  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap(SET);
  C2phase.TestSendRequestToDriver();
  ret_val =  C2phase.TestHandleDriverNotPresent(driver_type);
  EXPECT_EQ(TCOPER_RET_SUCCESS, ret_val);
}
#endif
