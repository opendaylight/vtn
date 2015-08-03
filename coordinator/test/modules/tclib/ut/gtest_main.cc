/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_TCLIB_GTEST_MAIN_HH_
#define _UNC_TCLIB_GTEST_MAIN_HH_

#include <gtest/gtest.h>
#include <pfcxx/ipc_server.hh>
#include <tclib_module.hh>
#include <unc/keytype.h>
#include <stub/tclib_module/tclib_interface_stub.hh>
#include <stub/tclib_module/libtc_common.hh>
#include <stdio.h>

using namespace unc::tc;

uint32_t arg_count = 0;
int ipcclnt_err = 0;
int ipcclnt_conn = 1;
int ipcclnt_invoke_err = 0;
int ipcclnt_invoke_resp = TC_OPER_SUCCESS;
int ipcclnt_sess_create_err = 0;
int stub_getargtype = 0;

uint32_t
pfc_ipcsrv_getargcount(pfc_ipcsrv_t *srv) {
#ifdef TCLIB_UT_DEBUG
  std::cout << "stub file return" << std::endl;
#endif
  return arg_count;
}

int
pfc_ipcclnt_altopen(const char *PFC_RESTRICT name,
                    pfc_ipcconn_t *PFC_RESTRICT connp) {
#ifdef TCLIB_UT_DEBUG
  std::cout << "stub file ipcclnt altopen return" << std::endl;
#endif
  *connp = ipcclnt_conn;
  return ipcclnt_err;
}

int
pfc_ipcclnt_sess_invoke(pfc_ipcsess_t *PFC_RESTRICT sess,
                        pfc_ipcresp_t *PFC_RESTRICT respp) {
#ifdef TCLIB_UT_DEBUG
  std::cout << "stub file ipcclnt invoke return" << std::endl;
#endif
  *respp = ipcclnt_invoke_resp;
  return ipcclnt_invoke_err;
}

int
pfc_ipcclnt_sess_altcreate(pfc_ipcsess_t **PFC_RESTRICT sessp,
                           pfc_ipcconn_t conn,
                           const char *PFC_RESTRICT name,
                           pfc_ipcid_t service) {
#ifdef TCLIB_UT_DEBUG
  std::cout << "stub file ipcclnt sess create return" << std::endl;
#endif
  return ipcclnt_sess_create_err;
}

int pfc_ipcclnt_altclose(pfc_ipcconn_t conn) {
#ifdef TCLIB_UT_DEBUG
  std::cout << "stub file ipcclnt ipcclnt_altclose return" << std::endl;
#endif
  return 0;
}

void __pfc_ipcclnt_sess_destroy(pfc_ipcsess_t *sess) {
#ifdef TCLIB_UT_DEBUG
  std::cout << "stub file ipcclnt sess_destroy" << std::endl;
#endif
//        PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(sess), 0);
}

int getArgType(uint32_t index, pfc_ipctype_t &type) {
  static int i = 0;
  if (i == 0) {
    type = PFC_IPCTYPE_STRING;
    i++;
  } else {
    type = PFC_IPCTYPE_UINT32;
    i = 0;
  }
  return 1;
}

class ServiceTestEnv : public ::testing::Environment {
  friend class ServiceTest;
 public:
  ServiceTestEnv() {}
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

namespace {
ServiceTestEnv* g_env = 0;
}

class ServiceTest : public ::testing::Test {
 public:
  virtual ~ServiceTest() {
  }
};

namespace unc {
namespace tclib {

TEST(test_1, test_sample) {
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
  tclib_obj.init();

  ctr_type = tclib_obj.GetControllerType();
  EXPECT_EQ(UNC_CT_UNKNOWN, ctr_type);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  if_stub_obj.ctr_type = UNC_CT_PFC;
  ctr_type = tclib_obj.GetControllerType();
  EXPECT_EQ(UNC_CT_PFC, ctr_type);

  if_stub_obj.ctr_type = UNC_CT_VNP;
  ctr_type = tclib_obj.GetControllerType();
  EXPECT_EQ(UNC_CT_VNP, ctr_type);

  tclib_obj.fini();
}

TEST(test_2, test_notify_session) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  tclib_obj.sess_ = NULL;
  ret = tclib_obj.NotifySessionConfig(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;
  arg_count = 0;
  ret = tclib_obj.NotifySessionConfig(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1
  arg_count = 2;
  sessutil.set_read_type(LIB_NOTIFY_SESSION);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.NotifySessionConfig(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 2
  arg_count = 2;
  sessutil.set_read_type(LIB_NOTIFY_SESSION);
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = tclib_obj.NotifySessionConfig(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_3, test_commit_transaction) {
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
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;
  arg_count = 0;
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 3;
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_TRANS_START);
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(MSG_COMMIT_TRANS_START, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);
  EXPECT_EQ(CONFIG_ID, tclib_obj.config_id_);

  // Failure 1 uint8_t
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 1 uint32_t
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure 3 uint32_t
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  // Invalid oper_state
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_NONE);
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  // Invalid session
  tclib_obj.oper_state_ = MSG_NONE;
  tclib_obj.session_id_ = 0;
  tclib_obj.config_id_ = CONFIG_ID;
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_TRANS_START);
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  // Invalid config id
  tclib_obj.oper_state_ = MSG_NONE;
  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = 0;
  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_TRANS_START);
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(TC_FAILURE, ret);

  // MSG as vote
  tclib_obj.session_id_ = SESSION_ID;
  tclib_obj.config_id_ = CONFIG_ID;
  // to return success for oper state validation
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;

  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_COMMIT_VOTE);
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(MSG_COMMIT_VOTE, tclib_obj.oper_state_);
  EXPECT_EQ(SESSION_ID, tclib_obj.session_id_);
  EXPECT_EQ(CONFIG_ID, tclib_obj.config_id_);

  sessutil.set_read_type(LIB_COMMIT_TRANS_START);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_oper_type(MSG_AUDIT_VOTE);
  ret = tclib_obj.CommitTransaction();
  EXPECT_EQ(TC_FAILURE, ret);
  tclib_obj.fini();
}

TEST(test_4, test_commit_trans_start_end) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcCommitTransactionMsg commit_trans_msg;
  commit_trans_msg.oper_type = MSG_COMMIT_TRANS_START;
  commit_trans_msg.session_id = SESSION_ID;
  commit_trans_msg.config_id = CONFIG_ID;

  ret = tclib_obj.CommitTransStartEnd(MSG_COMMIT_TRANS_START,
                                           commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // stub failure
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  ret = tclib_obj.CommitTransStartEnd(MSG_COMMIT_TRANS_START,
                                           commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);
  EXPECT_EQ(MSG_NONE, tclib_obj.oper_state_);

  if_stub_obj.tclib_stub_failure_ = PFC_FALSE;
  ret = tclib_obj.CommitTransStartEnd(MSG_COMMIT_TRANS_START,
                                           commit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  ret = tclib_obj.CommitTransStartEnd(MSG_COMMIT_TRANS_END,
                                           commit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_NONE, tclib_obj.oper_state_);

  ret = tclib_obj.CommitTransStartEnd(MSG_COMMIT_VOTE,
                                      commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_5, test_commit_trans_vote_global) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcCommitTransactionMsg commit_trans_msg;
  commit_trans_msg.oper_type = MSG_COMMIT_VOTE;
  commit_trans_msg.session_id = SESSION_ID;
  commit_trans_msg.config_id = CONFIG_ID;

  ret = tclib_obj.CommitVoteGlobal(MSG_COMMIT_VOTE,
                                      commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  if_stub_obj.fill_driver_info_ = PFC_FALSE;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  ret = tclib_obj.CommitVoteGlobal(MSG_COMMIT_VOTE,
                                      commit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  // vote failure
  commit_trans_msg.session_id = 0;
  ret = tclib_obj.CommitVoteGlobal(MSG_COMMIT_VOTE,
                                      commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);
  commit_trans_msg.session_id = SESSION_ID;

  ret = tclib_obj.CommitVoteGlobal(MSG_COMMIT_GLOBAL,
                                      commit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);
  EXPECT_EQ(MSG_NONE, tclib_obj.oper_state_);

  ret = tclib_obj.CommitVoteGlobal(MSG_COMMIT_TRANS_START,
                                      commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  TcServerSessionUtils sessutil;
  // driver info checks
  if_stub_obj.fill_driver_info_ = PFC_TRUE;
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.CommitVoteGlobal(MSG_COMMIT_VOTE,
                                   commit_trans_msg);
  EXPECT_EQ(TC_SUCCESS, ret);

  // Failure set uint8 1
  sessutil.set_read_type(LIB_COMMIT_TRANS_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.CommitVoteGlobal(MSG_COMMIT_VOTE,
                                   commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure set uint8 2
  sessutil.set_read_type(LIB_COMMIT_TRANS_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = tclib_obj.CommitVoteGlobal(MSG_COMMIT_VOTE,
                                   commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure set uint8 3
  sessutil.set_read_type(LIB_COMMIT_TRANS_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE_3);
  ret = tclib_obj.CommitVoteGlobal(MSG_COMMIT_VOTE,
                                   commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  // Failure set string
  sessutil.set_read_type(LIB_COMMIT_TRANS_VOTE_GLOBAL);
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.CommitVoteGlobal(MSG_COMMIT_VOTE,
                                   commit_trans_msg);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

}  // namespace tclib
}  // namespace unc


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  g_env = static_cast<ServiceTestEnv*>
      (::testing::AddGlobalTestEnvironment(new ServiceTestEnv));
  return RUN_ALL_TESTS();
}

#endif
