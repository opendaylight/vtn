/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_TCLIB_COMMIT_DRIVER_HH_
#define _UNC_TCLIB_COMMIT_DRIVER_HH_

#include <pfcxx/ipc_server.hh>
#include <gtest/gtest.h>
#include <tclib_module.hh>
#include <unc/keytype.h>
#include <stub/tclib_module/tclib_interface_stub.hh>
#include <stub/tclib_module/libtc_common.hh>
#include <stdio.h>

using namespace unc::tc;

extern uint32_t arg_count;
namespace unc {
namespace tclib {

TEST(test_6, test_CommitDriverVoteGlobal) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = CONFIG_ID;

  // pTcLibInterface_ NULL
  ret = tclib_obj.CommitDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // sess_ NULL
  tclib_obj.sess_ = NULL;
  ret = tclib_obj.CommitDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;

  arg_count = 3;
  if_stub_obj.ctr_type = UNC_CT_PFC;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  sessutil.set_read_type(LIB_COMMIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_DRIVER_VOTE);
  ret = tclib_obj.CommitDriverVoteGlobal();
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_COMMIT_DRIVER_VOTE, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);
  EXPECT_EQ(CONFIG_ID, tclib_obj.config_id_);

  // Invalid oper_state
  sessutil.set_read_type(LIB_COMMIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_TRANS_START);
  ret = tclib_obj.CommitDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1 uint8_t
  sessutil.set_read_type(LIB_COMMIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.CommitDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1 uint32_t
  sessutil.set_read_type(LIB_COMMIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.CommitDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 3 uint32_t
  sessutil.set_read_type(LIB_COMMIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = tclib_obj.CommitDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);
  // MSG as driver global
  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = CONFIG_ID;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_COMMIT_DRIVER_VOTE;

  sessutil.set_read_type(LIB_COMMIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_DRIVER_GLOBAL);
  ret = tclib_obj.CommitDriverVoteGlobal();
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_COMMIT_DRIVER_GLOBAL, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);
  EXPECT_EQ(CONFIG_ID, tclib_obj.config_id_);

  tclib_obj.fini();
}

TEST(test_7, test_CommitGlobalAbort) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = CONFIG_ID;

  // pTcLibInterface_ NULL
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // sess_ NULL
  tclib_obj.sess_ = NULL;
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;

  arg_count = 3;
  if_stub_obj.ctr_type = UNC_CT_PFC;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_COMMIT_DRIVER_VOTE;
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_ABORT);
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_COMMIT_ABORT, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);
  EXPECT_EQ(CONFIG_ID, tclib_obj.config_id_);

  // Invalid oper_state
  tclib_obj.oper_state_ = MSG_COMMIT_DRIVER_GLOBAL;
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_ABORT);
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1 uint8_t
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1 uint32_t
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 3 uint32_t
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 4 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_3);
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  if_stub_obj.ctr_type = UNC_CT_UNKNOWN;
  // Invalid session
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  tclib_obj.session_id_ = 0;
  tclib_obj.config_id_ = CONFIG_ID;
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_ABORT);
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  // Invalid config id
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = 0;
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_ABORT);
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = CONFIG_ID;
  // stub failure
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_ABORT);
  ret = tclib_obj.CommitGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_8, test_CommitDriverResult) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = CONFIG_ID;

  // pTcLibInterface_ NULL
  ret = tclib_obj.CommitDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // sess_ NULL
  tclib_obj.sess_ = NULL;
  ret = tclib_obj.CommitDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;

  arg_count = 4;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  sessutil.set_read_type(LIB_COMMIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_VOTE_DRIVER_RESULT);
  ret = tclib_obj.CommitDriverResult();
  EXPECT_EQ(MSG_COMMIT_VOTE_DRIVER_RESULT, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);
  EXPECT_EQ(CONFIG_ID, tclib_obj.config_id_);

  // Invalid oper_state
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  sessutil.set_read_type(LIB_COMMIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_GLOBAL_DRIVER_RESULT);
  ret = tclib_obj.CommitDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);

  // Invalid session
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  tclib_obj.session_id_ = 0;
  tclib_obj.config_id_ = CONFIG_ID;
  sessutil.set_read_type(LIB_COMMIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_VOTE_DRIVER_RESULT);
  ret = tclib_obj.CommitDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);

  // Invalid config id
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = 0;
  sessutil.set_read_type(LIB_COMMIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_VOTE_DRIVER_RESULT);
  ret = tclib_obj.CommitDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);


  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = CONFIG_ID;
  // stub failure
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  sessutil.set_read_type(LIB_COMMIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_VOTE_DRIVER_RESULT);
  ret = tclib_obj.CommitDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);
  EXPECT_EQ(MSG_COMMIT_VOTE_DRIVER_RESULT, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);
  EXPECT_EQ(CONFIG_ID, tclib_obj.config_id_);

  // update key list failure
  arg_count = 10;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  if_stub_obj.tclib_stub_failure_ = PFC_FALSE;
  sessutil.set_read_type(LIB_COMMIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_VOTE_DRIVER_RESULT);
  sessutil.set_updatekey_failure(PFC_TRUE);
  ret = tclib_obj.CommitDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);
  EXPECT_EQ(MSG_COMMIT_VOTE_DRIVER_RESULT, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);
  EXPECT_EQ(CONFIG_ID, tclib_obj.config_id_);

  tclib_obj.fini();
}


}  // namespace tclib
}  // namespace unc

#endif
