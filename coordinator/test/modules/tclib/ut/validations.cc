/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_TCLIB_VALIDATIONS_HH_
#define _UNC_TCLIB_VALIDATIONS_HH_

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

TEST(test_20, test_IsReadKeyValueAllowed) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.IsReadKeyValueAllowed();
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.IsReadKeyValueAllowed();
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  tclib_obj.oper_state_ = MSG_COMMIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.IsReadKeyValueAllowed();
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  tclib_obj.oper_state_ = MSG_AUDIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.IsReadKeyValueAllowed();
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  tclib_obj.oper_state_ = MSG_AUDIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.IsReadKeyValueAllowed();
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  tclib_obj.fini();
}


TEST(test_21, test_IsWriteKeyValueAllowed) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.IsWriteKeyValueAllowed();
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.IsWriteKeyValueAllowed();
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  tclib_obj.oper_state_ = MSG_COMMIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.IsWriteKeyValueAllowed();
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  tclib_obj.oper_state_ = MSG_AUDIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.IsWriteKeyValueAllowed();
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  tclib_obj.oper_state_ = MSG_AUDIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.IsWriteKeyValueAllowed();
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  tclib_obj.fini();
}

TEST(test_22, test_ValidateUpllUpplCommitSequence) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // success
  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_TRANS_START);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_TRANS_START);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_VOTE);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_VOTE);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_VOTE_DRIVER_RESULT);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_VOTE_DRIVER_RESULT);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_GLOBAL);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_GLOBAL);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_GLOBAL);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_GLOBAL;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(
      MSG_COMMIT_GLOBAL_DRIVER_RESULT);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(
      MSG_COMMIT_GLOBAL_DRIVER_RESULT);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_COMMIT_GLOBAL;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_COMMIT_ABORT;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_TRANS_END);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_ABORT);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_ABORT);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_COMMIT_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);

  // default case
  tclib_obj.oper_state_ = MSG_COMMIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplCommitSequence(MSG_AUDIT_START);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_23, test_ValidateDriverCommitSequence) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // success
  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_TRANS_START);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_TRANS_START);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_DRIVER_VOTE);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_DRIVER_VOTE);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_DRIVER_VOTE;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_DRIVER_GLOBAL);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_DRIVER_GLOBAL);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_DRIVER_GLOBAL;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_COMMIT_ABORT;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_DRIVER_VOTE;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_ABORT);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_COMMIT_DRIVER_GLOBAL;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_COMMIT_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);

  // default case
  tclib_obj.oper_state_ = MSG_COMMIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.ValidateDriverCommitSequence(MSG_AUDIT_START);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_24, test_ValidateUpllUpplAuditSequence) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // success
  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_START);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_START);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_START;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_TRANS_START);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_TRANS_START);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_VOTE);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_VOTE);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_VOTE_DRIVER_RESULT);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_VOTE_DRIVER_RESULT);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_GLOBAL);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_GLOBAL);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_GLOBAL;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_GLOBAL_DRIVER_RESULT);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_GLOBAL_DRIVER_RESULT);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_AUDIT_ABORT;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_END;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_END);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_AUDIT_START;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_END);
  EXPECT_EQ(TC_SUCCESS, ret);
  // success
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_ABORT);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_AUDIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_ABORT);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);
  tclib_obj.oper_state_ = MSG_AUDIT_START;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_AUDIT_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);
  // default case
  tclib_obj.oper_state_ = MSG_AUDIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.ValidateUpllUpplAuditSequence(MSG_COMMIT_TRANS_START);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_23, test_ValidateDriverAuditSequence) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // success
  tclib_obj.audit_in_progress_ = PFC_TRUE;
  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_START);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.audit_in_progress_ = PFC_TRUE;
  tclib_obj.oper_state_ = MSG_AUDIT_START;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_START);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.audit_in_progress_ = PFC_FALSE;
  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_START);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.audit_in_progress_ = PFC_TRUE;
  tclib_obj.oper_state_ = MSG_AUDIT_START;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_TRANS_START);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.audit_in_progress_ = PFC_FALSE;
  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_TRANS_START);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.audit_in_progress_ = PFC_TRUE;
  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_TRANS_START);
  EXPECT_EQ(TC_FAILURE, ret);
  tclib_obj.audit_in_progress_ = PFC_FALSE;
  tclib_obj.oper_state_ = MSG_AUDIT_START;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_TRANS_START);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_DRIVER_VOTE);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_START;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_DRIVER_VOTE);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_DRIVER_VOTE;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_DRIVER_GLOBAL);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_DRIVER_GLOBAL);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_DRIVER_GLOBAL;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);
  tclib_obj.oper_state_ = MSG_AUDIT_ABORT;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TC_SUCCESS, ret);
  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_START;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_TRANS_END);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.audit_in_progress_ = PFC_TRUE;
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_END;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_END);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.audit_in_progress_ = PFC_TRUE;
  tclib_obj.oper_state_ = MSG_AUDIT_DRIVER_GLOBAL;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_END);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.audit_in_progress_ = PFC_FALSE;
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_END;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_END);
  EXPECT_EQ(TC_FAILURE, ret);

  // success
  tclib_obj.oper_state_ = MSG_AUDIT_DRIVER_VOTE;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_ABORT);
  EXPECT_EQ(TC_SUCCESS, ret);

  // failure
  tclib_obj.oper_state_ = MSG_AUDIT_DRIVER_GLOBAL;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);
  tclib_obj.oper_state_ = MSG_AUDIT_START;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);
  tclib_obj.oper_state_ = MSG_AUDIT_TRANS_START;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_AUDIT_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);

  // default case
  tclib_obj.oper_state_ = MSG_AUDIT_GLOBAL_DRIVER_RESULT;
  ret = tclib_obj.ValidateDriverAuditSequence(MSG_COMMIT_TRANS_START);
  EXPECT_EQ(TC_FAILURE, ret);


  tclib_obj.fini();
}

}  // namespace tclib
}  // namespace unc

#endif
