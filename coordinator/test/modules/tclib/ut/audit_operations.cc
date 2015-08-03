/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_TCLIB_AUDIT_OPER_HH_
#define _UNC_TCLIB_AUDIT_OPER_HH_

#include <gtest/gtest.h>
#include <pfcxx/ipc_server.hh>
#include <tclib_module.hh>
#include <unc/keytype.h>
#include <stub/tclib_module/tclib_interface_stub.hh>
#include <stub/tclib_module/libtc_common.hh>
#include <stdio.h>

using namespace unc::tc;

extern uint32_t arg_count;
namespace unc {
namespace tclib {

TEST(test_A, test_AuditTransaction) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = CONFIG_ID;

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  tclib_obj.sess_ = NULL;
  ret = tclib_obj.AuditTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;
  arg_count = 0;
  ret = tclib_obj.AuditTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 3;
  sessutil.set_read_type(LIB_AUDIT_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_START);
  ret = tclib_obj.AuditTransaction();
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_AUDIT_START, tclib_obj.oper_state_);

  arg_count = 3;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;

  sessutil.set_read_type(LIB_AUDIT_VOTE);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_VOTE);
  ret = tclib_obj.AuditTransaction();
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_AUDIT_VOTE, tclib_obj.oper_state_);

  // Invalid oper_state
  sessutil.set_read_type(LIB_AUDIT_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_TRANS_START);
  ret = tclib_obj.AuditTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_AUDIT_START;

  // tclib_obj.audit_in_progress_
  if_stub_obj.ctr_type = UNC_CT_PFC;
  sessutil.set_read_type(LIB_AUDIT_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_TRANS_START);
  sessutil.set_ctr_type(UNC_CT_PFC);
  ret = tclib_obj.AuditTransaction();
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_AUDIT_TRANS_START, tclib_obj.oper_state_);
  EXPECT_EQ(PFC_TRUE, tclib_obj.audit_in_progress_);

  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_NONE;

  // tclib_obj.audit_in_progress_
  if_stub_obj.ctr_type = UNC_CT_PFC;
  sessutil.set_read_type(LIB_AUDIT_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_TRANS_START);
  sessutil.set_ctr_type(UNC_CT_PFC);
  ret = tclib_obj.AuditTransaction();
  EXPECT_EQ(TC_FAILURE, ret);
  EXPECT_EQ(MSG_AUDIT_TRANS_START, tclib_obj.oper_state_);
  EXPECT_EQ(PFC_TRUE, tclib_obj.audit_in_progress_);

  tclib_obj.fini();
}

TEST(test_9, test_AuditTransStartEnd) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcAuditTransactionMsg audit_trans_msg;
  audit_trans_msg.oper_type = MSG_AUDIT_START;
  audit_trans_msg.session_id = SESSION_ID;
  audit_trans_msg.driver_id = UNC_CT_PFC;
  audit_trans_msg.controller_id = "openflow1";
  audit_trans_msg.reconnect_controller = 1;

  ret = tclib_obj.AuditTransStartEnd(MSG_AUDIT_START,
                                     audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // audit start failure
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  ret = tclib_obj.AuditTransStartEnd(MSG_AUDIT_START,
                                     audit_trans_msg);
  EXPECT_EQ(MSG_NONE, tclib_obj.oper_state_);
  EXPECT_EQ(PFC_FALSE, tclib_obj.audit_in_progress_);

  if_stub_obj.tclib_stub_failure_ = PFC_FALSE;
  ret = tclib_obj.AuditTransStartEnd(MSG_AUDIT_START,
                                     audit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  ret = tclib_obj.AuditTransStartEnd(MSG_AUDIT_END,
                                     audit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_NONE, tclib_obj.oper_state_);
  EXPECT_EQ(PFC_FALSE, tclib_obj.audit_in_progress_);

  ret = tclib_obj.AuditTransStartEnd(MSG_AUDIT_TRANS_START,
                                     audit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  if_stub_obj.ctr_type = UNC_CT_PFC;
  tclib_obj.audit_in_progress_ = PFC_FALSE;
  ret = tclib_obj.AuditTransStartEnd(MSG_AUDIT_TRANS_END,
                                     audit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_NONE, tclib_obj.oper_state_);

  ret = tclib_obj.AuditTransStartEnd(MSG_AUDIT_VOTE,
                                     audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_10, test_AuditVoteGlobal) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcAuditTransactionMsg audit_trans_msg;
  audit_trans_msg.oper_type = MSG_AUDIT_VOTE;
  audit_trans_msg.session_id = SESSION_ID;
  audit_trans_msg.driver_id = UNC_CT_PFC;
  audit_trans_msg.controller_id = "openflow1";

  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_VOTE,
                                  audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  if_stub_obj.fill_driver_info_ = PFC_FALSE;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_VOTE,
                                  audit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);


  // vote failure
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_VOTE,
                                  audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.oper_state_ = MSG_AUDIT_VOTE;
  if_stub_obj.tclib_stub_failure_ = PFC_FALSE;
  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_GLOBAL,
                                  audit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  ret = tclib_obj.AuditVoteGlobal(MSG_COMMIT_TRANS_START,
                                  audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  TcServerSessionUtils sessutil;
  if_stub_obj.tclib_stub_failure_ = PFC_FALSE;
  // driver info checks
  if_stub_obj.fill_driver_info_ = PFC_TRUE;
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_GLOBAL,
                                  audit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  // driver empty and set uint8 failed
  if_stub_obj.fill_driver_info_ = PFC_FALSE;
  sessutil.set_read_type(LIB_COMMIT_TRANS_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_5);
  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_GLOBAL,
                                  audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);
  if_stub_obj.fill_driver_info_ = PFC_TRUE;

  // Failure set uint8 1
  sessutil.set_read_type(LIB_COMMIT_TRANS_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_GLOBAL,
                                  audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);
  // Failure set uint8 2
  sessutil.set_read_type(LIB_COMMIT_TRANS_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_GLOBAL,
                                  audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);
  // Failure set uint8 3
  sessutil.set_read_type(LIB_COMMIT_TRANS_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_3);
  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_GLOBAL,
                                  audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure set uint8 4
  sessutil.set_read_type(LIB_COMMIT_TRANS_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_4);
  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_GLOBAL,
                                  audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure set string
  sessutil.set_read_type(LIB_COMMIT_TRANS_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.AuditVoteGlobal(MSG_AUDIT_GLOBAL,
                                  audit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);
  tclib_obj.fini();
}

TEST(test_11, test_AuditDriverVoteGlobal) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = CONFIG_ID;

  // pTcLibInterface_ NULL
  ret = tclib_obj.AuditDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // sess_ NULL
  tclib_obj.sess_ = NULL;
  ret = tclib_obj.AuditDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;

  arg_count = 3;
  if_stub_obj.ctr_type = UNC_CT_PFC;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;
  sessutil.set_read_type(LIB_AUDIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_DRIVER_VOTE);
  ret = tclib_obj.AuditDriverVoteGlobal();
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_AUDIT_DRIVER_VOTE, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);

  // Invalid oper_state
  sessutil.set_read_type(LIB_AUDIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_TRANS_END);
  ret = tclib_obj.AuditDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1 uint8_t
  sessutil.set_read_type(LIB_AUDIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.AuditDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1 uint32_t
  sessutil.set_read_type(LIB_AUDIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.AuditDriverVoteGlobal();
  EXPECT_EQ(TC_FAILURE, ret);

  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_AUDIT_DRIVER_VOTE;

  sessutil.set_read_type(LIB_AUDIT_DRIVER_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_DRIVER_GLOBAL);
  ret = tclib_obj.AuditDriverVoteGlobal();
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_AUDIT_DRIVER_GLOBAL, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);

  tclib_obj.fini();
}

TEST(test_12, test_AuditGlobalAbort) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = CONFIG_ID;

  // pTcLibInterface_ NULL
  ret = tclib_obj.AuditGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // sess_ NULL
  tclib_obj.sess_ = NULL;
  ret = tclib_obj.AuditGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;

  arg_count = 3;
  if_stub_obj.ctr_type = UNC_CT_PFC;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_AUDIT_DRIVER_VOTE;
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_ABORT);
  ret = tclib_obj.AuditGlobalAbort();
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_AUDIT_ABORT, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);
  EXPECT_EQ(CONFIG_ID, tclib_obj.config_id_);

  // Invalid oper_state
  tclib_obj.oper_state_ = MSG_AUDIT_DRIVER_GLOBAL;
  sessutil.set_read_type(LIB_AUDIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_ABORT);
  ret = tclib_obj.AuditGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1 uint8_t
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.AuditGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1 uint32_t
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.AuditGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 3 uint32_t
  sessutil.set_read_type(LIB_COMMIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = tclib_obj.AuditGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  // stub failure
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  tclib_obj.oper_state_ = MSG_AUDIT_DRIVER_VOTE;
  sessutil.set_read_type(LIB_AUDIT_GLOBAL_ABORT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_ABORT);
  ret = tclib_obj.AuditGlobalAbort();
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}


TEST(test_13, test_AuditDriverResult) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  tclib_obj.session_id_ = SESSION_ID;

  // pTcLibInterface_ NULL
  ret = tclib_obj.AuditDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // sess_ NULL
  tclib_obj.sess_ = NULL;
  ret = tclib_obj.AuditDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;

  arg_count = 4;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE;
  sessutil.set_read_type(LIB_AUDIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_VOTE_DRIVER_RESULT);
  ret = tclib_obj.AuditDriverResult();
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_AUDIT_VOTE_DRIVER_RESULT, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);

  // Invalid oper_state
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE_DRIVER_RESULT;
  sessutil.set_read_type(LIB_AUDIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_GLOBAL_DRIVER_RESULT);
  ret = tclib_obj.AuditDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);

  // stub failure
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE;
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  sessutil.set_read_type(LIB_AUDIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_VOTE_DRIVER_RESULT);
  ret = tclib_obj.AuditDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);
  EXPECT_EQ(MSG_AUDIT_VOTE_DRIVER_RESULT, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);

  // update key list failure
  arg_count = 10;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE;
  if_stub_obj.tclib_stub_failure_ = PFC_FALSE;
  sessutil.set_read_type(LIB_AUDIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_VOTE_DRIVER_RESULT);
  sessutil.set_updatekey_failure(PFC_TRUE);
  ret = tclib_obj.AuditDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);
  EXPECT_EQ(MSG_AUDIT_VOTE, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);
  sessutil.set_updatekey_failure(PFC_FALSE);

  // util failure for set audit_result
  arg_count = 10;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE;
  sessutil.set_read_type(LIB_AUDIT_DRIVER_RESULT);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_return_type_1(RETURN_FAILURE);
  sessutil.set_oper_type(MSG_AUDIT_VOTE_DRIVER_RESULT);
  ret = tclib_obj.AuditDriverResult();
  EXPECT_EQ(TC_FAILURE, ret);
  EXPECT_EQ(MSG_AUDIT_VOTE_DRIVER_RESULT, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);

  tclib_obj.fini();
}

}  //  namespace tclib
}  //  namespace unc

#endif
