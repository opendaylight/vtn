/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcoperations.hh"
#include <uncxx/tc/libtc_common.hh>

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_string;
extern int stub_opertype;
extern int stub_create_session;
extern int stub_set_arg;
extern int stub_session_invoke;
extern int stub_response;

TEST(TcOperations, TcGetOperType) {
  SET_AUDIT_OPER_PARAMS();
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  //EXPECT_EQ(TC_OPER_INVALID_INPUT, tcoperations.GetOperType());

  stub_srv_uint32 = -1;
  EXPECT_EQ(TC_OPER_FAILURE, tcoperations.GetOperType());

  stub_srv_uint32 = 10;
  EXPECT_EQ(TC_OPER_SUCCESS, tcoperations.GetOperType());
  // DEL_AUDIT_PARAMS();
}


TEST(TcOperations, GetSessionId) {
  SET_AUDIT_OPER_PARAMS();
  TestTcOperations tcoperations(tc_lock_,
                                  &sess_,
                                  db_handler,
                                  unc_map_);

  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tcoperations.GetSessionId());

  stub_srv_uint32 = -1;
  EXPECT_EQ(TC_OPER_FAILURE, tcoperations.GetSessionId());

  stub_srv_uint32 = 10;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tcoperations.GetSessionId());

  stub_srv_uint32 = 10;
  tcoperations.session_id_ = 50;
  EXPECT_EQ(TC_OPER_SUCCESS, tcoperations.GetSessionId());
  // DEL_AUDIT_PARAMS();
}

TEST(TcOperations, SetOperType) {
  SET_AUDIT_OPER_PARAMS();
    TestTcOperations tcoperations(tc_lock_,
                                  &sess_,
                                  db_handler,
                                  unc_map_);

  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tcoperations.SetOperType());

  stub_srv_uint32 = -1;
  EXPECT_EQ(TC_OPER_FAILURE, tcoperations.SetOperType());

  stub_srv_uint32 = 10;
  EXPECT_EQ(TC_OPER_SUCCESS, tcoperations.SetOperType());
  // DEL_AUDIT_PARAMS();
}

TEST(TcOperations, SetSessionId) {
  SET_AUDIT_OPER_PARAMS();
    TestTcOperations tcoperations(tc_lock_,
                                  &sess_,
                                  db_handler,
                                  unc_map_);

  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tcoperations.SetSessionId());

  stub_srv_uint32 = -1;
  EXPECT_EQ(TC_OPER_FAILURE, tcoperations.SetSessionId());

  stub_srv_uint32 = 10;
  EXPECT_EQ(TC_OPER_SUCCESS, tcoperations.SetSessionId());
  // DEL_AUDIT_PARAMS();
}

TEST(TcOperations, SetOperStatus) {
  SET_AUDIT_OPER_PARAMS();
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  TcOperStatus resp_status = TC_OPER_SUCCESS;

  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tcoperations.SetOperStatus(resp_status));

  stub_srv_uint32 = -1;
  EXPECT_EQ(TC_OPER_FAILURE, tcoperations.SetOperStatus(resp_status));

  stub_srv_uint32 = 10;
  EXPECT_EQ(TC_OPER_SUCCESS, tcoperations.SetOperStatus(resp_status));
  // DEL_AUDIT_PARAMS();
}


/*TEST(TcOperations, HandleArgs) {
  SET_AUDIT_OPER_PARAMS();
    TestTcOperations tcoperations(tc_lock_,
                                &sess_,
								db_handler,
								unc_map_);

  sess_.getArgCount();

  EXPECT_EQ(TC_OPER_SUCCESS, tcoperations.HandleArgs());
}
*/

TEST(TcOperations, SendResponse) {
  SET_AUDIT_OPER_PARAMS();
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  TcOperStatus send_oper_status = TC_OPER_SUCCESS;

  EXPECT_EQ(0, tcoperations.SendResponse(send_oper_status));
  // DEL_AUDIT_PARAMS();
}

/*TEST(TcOperations, RevokeOperation) {
  SET_AUDIT_OPER_PARAMS();

    TestTcOperations tcoperations(tc_lock_,
                                &sess_,
								db_handler,
								unc_map_);
  TcOperStatus oper_status=TC_OPER_SUCCESS;

  tcoperations.tc_oper_status_ = INPUT_VALIDATION;
  EXPECT_EQ(0, tcoperations.RevokeOperation(oper_status));

  tcoperations.tc_oper_status_ = GET_EXCLUSION_PHASE;
  EXPECT_EQ(0, tcoperations.RevokeOperation(oper_status));

  tcoperations.tc_oper_status_ = EXECUTE_PHASE;
  EXPECT_EQ(0, tcoperations.RevokeOperation(oper_status));

  tcoperations.tc_oper_status_ = RELEASE_EXCLUSION_PHASE;
  EXPECT_EQ(0, tcoperations.RevokeOperation(oper_status));

//tcoperations.tc_oper_status_ = INPUT_VALIDATION;
//EXPECT_EQ(0, tcoperations.RevokeOperation(oper_status));
}

TEST(TcOperations, Dispatch) {

  SET_AUDIT_OPER_PARAMS();
    TestTcOperations tcoperations(tc_lock_,
                                &sess_,
								db_handler,
								unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tcoperations.Dispatch());
}
*/

TEST(TcOperations,TestHandleMsgRet_AddOnDriver)
{
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus ret = TC_OPER_SUCCESS;
  TcOperRet MsgRet = TCOPER_RET_NO_DRIVER;
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  ret = tcoperations.TestHandleMsgRet(MsgRet);
  EXPECT_EQ(TC_OPER_DRIVER_NOT_PRESENT,ret);
}

TEST(TcOperations,TestIsCandidateDirty_response_failure)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  pfc_bool_t result = false;
  stub_get_arg_fail = 1;
  stub_get_arg = 2;
  result = tcoperations.IsCandidateDirty(0,0);
  EXPECT_EQ(result,PFC_FALSE);
  stub_get_arg_fail = 0;
  stub_get_arg = 0;
}

TEST(TcOperations,TestIsCandidateDirty_fetch_op_type_failed_upll)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  pfc_bool_t result = false;
  stub_get_arg = 1;
  result = tcoperations.IsCandidateDirty(0,0);
  EXPECT_EQ(result,PFC_FALSE);
}


TEST(TcOperations,TestIsCandidateDirty_failure_response)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  pfc_bool_t result = false;
  stub_get_arg = 2;
  result = tcoperations.IsCandidateDirty(0,0);
  EXPECT_EQ(result,PFC_FALSE);
}

TEST(TcOperations,TestIsCandidateDirty_IsCandidateDirty)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  pfc_bool_t result = false;
  stub_get_arg = 2;
  stub_get_arg1 = 1;
  result = tcoperations.IsCandidateDirty(0,0);
  EXPECT_EQ(result,PFC_FALSE);
  stub_get_arg1 =0;
}


TEST(TcOperations,TestIsCandidateDirty_fetch_IsDirty_fail)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  pfc_bool_t result = false;
  stub_get_arg = 2;
  stub_get_arg1 = 2;
  result = tcoperations.IsCandidateDirty(0,0);
  EXPECT_EQ(result,PFC_FALSE);
  stub_get_arg1 =0;
}

TEST(TcOperations,TestIsCandidateDirty_IsCandidateDirty_true)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  pfc_bool_t result = false;
  stub_get_arg = 2;
  stub_get_arg1 = 3;
  result = tcoperations.IsCandidateDirty(0,0);
  EXPECT_EQ(result,PFC_TRUE);
  stub_get_arg1 =0;
}

TEST(TcOperations,TestIsCandidateDirty_ClientSession_Null)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  pfc_bool_t result = false;
  stub_create_session = 1;
  result = tcoperations.IsCandidateDirty(0,0);
  EXPECT_EQ(result,PFC_FALSE);
  stub_create_session = 0;
}

TEST(TcOperations,TestIsCandidateDirty_FailedToSetSessionId)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  pfc_bool_t result = false;
  stub_set_arg = 1;
  result = tcoperations.IsCandidateDirty(0,0);
  EXPECT_EQ(result,PFC_FALSE);
  stub_set_arg = 0;
}

TEST(TcOperations,TestIsCandidateDirty_InvokingSessionFailed)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  pfc_bool_t result = false;
  stub_session_invoke = 1;
  result = tcoperations.IsCandidateDirty(0,0);
  EXPECT_EQ(result,PFC_FALSE);
  stub_session_invoke = 0;
}

TEST(TcOperations,TestIsCandidateDirty_InvokingResponseFailure)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  pfc_bool_t result = false;
  stub_response = 1;
  result = tcoperations.IsCandidateDirty(0,0);
  EXPECT_EQ(result,PFC_FALSE);
  stub_response = 0;
}
