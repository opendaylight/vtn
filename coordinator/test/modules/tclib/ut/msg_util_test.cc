/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <pfcxx/ipc_server.hh>
#include <tclib_module.hh>
#include <unc/keytype.h>
#include <stub/tclib_module/tclib_interface_stub.hh>
#include <stub/tclib_module/libtc_common.hh>
#include <tclib_msg_util.hh>
#include <stdio.h>

using namespace unc::tc;

extern uint32_t arg_count;

namespace unc {
namespace tclib {

TEST(test_40, test_GetCommitTransactionMsg) {
  TcCommonRet ret = TC_SUCCESS;
  TcCommitTransactionMsg commit_trans_msg;

  ret = TcLibMsgUtil::GetCommitTransactionMsg(NULL, commit_trans_msg);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;
  pfc::core::ipc::ServerSession *sess_;

  sess_ = &sess;
  arg_count = 0;
  ret = TcLibMsgUtil::GetCommitTransactionMsg(sess_, commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 4;
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_TRANS_END);
  ret = TcLibMsgUtil::GetCommitTransactionMsg(sess_, commit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_COMMIT_TRANS_END, commit_trans_msg.oper_type);
  EXPECT_EQ(SESSION_ID, commit_trans_msg.session_id);
  EXPECT_EQ(CONFIG_ID, commit_trans_msg.config_id);
  EXPECT_EQ(0, commit_trans_msg.end_result);

  // Failure 1 uint8_t
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_FAILURE);
  ret = TcLibMsgUtil::GetCommitTransactionMsg(sess_, commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1 uint32_t
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = TcLibMsgUtil::GetCommitTransactionMsg(sess_, commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 3 uint32_t
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = TcLibMsgUtil::GetCommitTransactionMsg(sess_, commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 4 uint8_t
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_FAILURE_3);
  ret = TcLibMsgUtil::GetCommitTransactionMsg(sess_, commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);
}

TEST(test_41, test_GetCommitDrvVoteGlobalMsg) {
  TcCommonRet ret = TC_SUCCESS;
  TcCommitDrvVoteGlobalMsg drv_vote_global_msg;

  ret = TcLibMsgUtil::GetCommitDrvVoteGlobalMsg(NULL, drv_vote_global_msg);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;
  pfc::core::ipc::ServerSession *sess_;

  sess_ = &sess;
  arg_count = 0;
  ret = TcLibMsgUtil::GetCommitDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 5;
  sessutil.set_read_type(LIB_COMMIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_3);
  sessutil.set_oper_type(MSG_COMMIT_DRIVER_VOTE);
  ret = TcLibMsgUtil::GetCommitDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 4 string
  sessutil.set_read_type(LIB_COMMIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_4);
  ret = TcLibMsgUtil::GetCommitDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  EXPECT_EQ(TC_FAILURE, ret);
}

TEST(test_42, test_GetCommitDrvResultMsg) {
  TcCommonRet ret = TC_SUCCESS;
  TcCommitDrvResultMsg drvresult_msg;

  ret = TcLibMsgUtil::GetCommitDrvResultMsg(NULL, drvresult_msg);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;
  pfc::core::ipc::ServerSession *sess_;

  sess_ = &sess;
  arg_count = 0;
  ret = TcLibMsgUtil::GetCommitDrvResultMsg(sess_, drvresult_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 4;
  sessutil.set_read_type(LIB_COMMIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_FAILURE_4);
  sessutil.set_oper_type(MSG_COMMIT_VOTE_DRIVER_RESULT);
  ret = TcLibMsgUtil::GetCommitDrvResultMsg(sess_, drvresult_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // uint32_t session failure
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = TcLibMsgUtil::GetCommitDrvResultMsg(sess_, drvresult_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // uint32_t configid failure
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = TcLibMsgUtil::GetCommitDrvResultMsg(sess_, drvresult_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // uint8 phase failure
  sessutil.set_return_type(RETURN_FAILURE_3);
  ret = TcLibMsgUtil::GetCommitDrvResultMsg(sess_, drvresult_msg);
  EXPECT_EQ(TC_FAILURE, ret);
}

TEST(test_43, test_GetAuditTransactionMsg) {
  TcCommonRet ret = TC_SUCCESS;
  TcAuditTransactionMsg audit_trans_msg;

  ret = TcLibMsgUtil::GetAuditTransactionMsg(NULL, audit_trans_msg);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;
  pfc::core::ipc::ServerSession *sess_;

  sess_ = &sess;
  arg_count = 0;
  ret = TcLibMsgUtil::GetAuditTransactionMsg(sess_, audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 6;
  sessutil.set_read_type(LIB_AUDIT_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_END);
  ret = TcLibMsgUtil::GetAuditTransactionMsg(sess_, audit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  // Failure 1 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = TcLibMsgUtil::GetAuditTransactionMsg(sess_, audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 2 uint32_t
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = TcLibMsgUtil::GetAuditTransactionMsg(sess_, audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 3 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_3);
  ret = TcLibMsgUtil::GetAuditTransactionMsg(sess_, audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 4 string
  sessutil.set_return_type(RETURN_FAILURE);
  ret = TcLibMsgUtil::GetAuditTransactionMsg(sess_, audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 5 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_4);
  ret = TcLibMsgUtil::GetAuditTransactionMsg(sess_, audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // end result success case
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_TRANS_END);
  ret = TcLibMsgUtil::GetAuditTransactionMsg(sess_, audit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  // Failure 6 uint8_t
  sessutil.set_oper_type(MSG_AUDIT_TRANS_END);
  sessutil.set_return_type(RETURN_FAILURE_5);
  ret = TcLibMsgUtil::GetAuditTransactionMsg(sess_, audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);
}

TEST(test_44, test_GetAuditDrvVoteGlobalMsg) {
  TcCommonRet ret = TC_SUCCESS;
  TcAuditDrvVoteGlobalMsg drv_vote_global_msg;

  ret = TcLibMsgUtil::GetAuditDrvVoteGlobalMsg(NULL, drv_vote_global_msg);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;
  pfc::core::ipc::ServerSession *sess_;

  sess_ = &sess;
  arg_count = 0;
  ret = TcLibMsgUtil::GetAuditDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 5;
  sessutil.set_read_type(LIB_AUDIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_DRIVER_VOTE);
  ret = TcLibMsgUtil::GetAuditDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  // Failure 1 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = TcLibMsgUtil::GetAuditDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 2 uint32_t
  sessutil.set_return_type(RETURN_FAILURE_3);
  ret = TcLibMsgUtil::GetAuditDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 3 string
  sessutil.set_return_type(RETURN_FAILURE_4);
  ret = TcLibMsgUtil::GetAuditDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  EXPECT_EQ(TC_FAILURE, ret);


  // Failure 4 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = TcLibMsgUtil::GetAuditDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 5 string
  sessutil.set_return_type(RETURN_FAILURE_5);
  ret = TcLibMsgUtil::GetAuditDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  EXPECT_EQ(TC_FAILURE, ret);
}

TEST(test_45, test_GetAuditDrvResultMsg) {
  TcCommonRet ret = TC_SUCCESS;
  TcAuditDrvResultMsg drv_result_msg;

  ret = TcLibMsgUtil::GetAuditDrvResultMsg(NULL, drv_result_msg);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;
  pfc::core::ipc::ServerSession *sess_;

  sess_ = &sess;
  arg_count = 0;
  ret = TcLibMsgUtil::GetAuditDrvResultMsg(sess_, drv_result_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 5;
  sessutil.set_read_type(LIB_AUDIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_VOTE_DRIVER_RESULT);
  ret = TcLibMsgUtil::GetAuditDrvResultMsg(sess_, drv_result_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  // Failure 1 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_4);
  ret = TcLibMsgUtil::GetAuditDrvResultMsg(sess_, drv_result_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 2 uint32_t
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = TcLibMsgUtil::GetAuditDrvResultMsg(sess_, drv_result_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 3 string
  sessutil.set_return_type(RETURN_FAILURE);
  ret = TcLibMsgUtil::GetAuditDrvResultMsg(sess_, drv_result_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 4 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_3);
  ret = TcLibMsgUtil::GetAuditDrvResultMsg(sess_, drv_result_msg);
  EXPECT_EQ(TC_FAILURE, ret);
}

TEST(test_46, test_GetAuditGlobalAbortMsg) {
  TcCommonRet ret = TC_SUCCESS;
  TcAuditGlobalAbortMsg global_abort_msg;
  ret = TcLibMsgUtil::GetAuditGlobalAbortMsg(NULL, global_abort_msg);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;
  pfc::core::ipc::ServerSession *sess_;

  sess_ = &sess;
  arg_count = 0;
  ret = TcLibMsgUtil::GetAuditGlobalAbortMsg(sess_, global_abort_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 5;
  sessutil.set_read_type(LIB_AUDIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_ABORT);
  ret = TcLibMsgUtil::GetAuditGlobalAbortMsg(sess_, global_abort_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  // Failure 1 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = TcLibMsgUtil::GetAuditGlobalAbortMsg(sess_, global_abort_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 2 uint32_t
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = TcLibMsgUtil::GetAuditGlobalAbortMsg(sess_, global_abort_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 3 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_3);
  ret = TcLibMsgUtil::GetAuditGlobalAbortMsg(sess_, global_abort_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 4 string
  sessutil.set_return_type(RETURN_FAILURE);
  ret = TcLibMsgUtil::GetAuditGlobalAbortMsg(sess_, global_abort_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 5 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_4);
  ret = TcLibMsgUtil::GetAuditGlobalAbortMsg(sess_, global_abort_msg);
  EXPECT_EQ(TC_FAILURE, ret);
}

TEST(test_47, test_GetAuditConfigMsg) {
  TcCommonRet ret = TC_SUCCESS;
  TcAuditConfigMsg audit_config_msg;

  ret = TcLibMsgUtil::GetAuditConfigMsg(NULL, audit_config_msg);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;
  pfc::core::ipc::ServerSession *sess_;

  sess_ = &sess;
  arg_count = 0;
  ret = TcLibMsgUtil::GetAuditConfigMsg(sess_, audit_config_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 5;
  sessutil.set_read_type(LIB_AUDIT_CONFIG);
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = TcLibMsgUtil::GetAuditConfigMsg(sess_, audit_config_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  // Failure 1 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = TcLibMsgUtil::GetAuditConfigMsg(sess_, audit_config_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 2 uint8_t
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = TcLibMsgUtil::GetAuditConfigMsg(sess_, audit_config_msg);
  EXPECT_EQ(TC_FAILURE, ret);
}


}  // namespace tclib
}  // namespace unc

