/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_TCLIB_TCLIB_COMMON_HH_
#define _UNC_TCLIB_TCLIB_COMMON_HH_

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

TEST(test_13, test_ReleaseTransactionResources) {
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();


  tclib_obj.ReleaseTransactionResources();
  EXPECT_EQ(MSG_NONE, tclib_obj.oper_state_);
  EXPECT_EQ(0, tclib_obj.key_map_.size());
  EXPECT_EQ(0, tclib_obj.controller_key_map_.size());
  EXPECT_EQ(0, tclib_obj.commit_phase_result_.size());

  TcControllerResult ctrl_res;

  ctrl_res.controller_id = "ctr_1";
  ctrl_res.resp_code = 0;
  ctrl_res.num_of_errors = 0;
  tclib_obj.oper_state_ = MSG_AUDIT_DRIVER_GLOBAL;

  tclib_obj.key_map_.insert(std::pair<uint32_t, uint32_t>(1, 10));
  tclib_obj.controller_key_map_.insert(
      std::pair<std::string,
      TcKeyTypeIndexMap>(ctrl_res.controller_id, tclib_obj.key_map_));
  tclib_obj.commit_phase_result_.push_back(ctrl_res);

  EXPECT_EQ(MSG_AUDIT_DRIVER_GLOBAL, tclib_obj.oper_state_);
  EXPECT_EQ(1, tclib_obj.key_map_.size());
  EXPECT_EQ(1, tclib_obj.controller_key_map_.size());
  EXPECT_EQ(1, tclib_obj.commit_phase_result_.size());

  tclib_obj.ReleaseTransactionResources();
  EXPECT_EQ(MSG_NONE, tclib_obj.oper_state_);
  EXPECT_EQ(0, tclib_obj.key_map_.size());
  EXPECT_EQ(0, tclib_obj.controller_key_map_.size());
  EXPECT_EQ(0, tclib_obj.commit_phase_result_.size());

  tclib_obj.fini();
}

TEST(test_14, test_test_SaveConfiguration) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  // pTcLibInterface_ NULL
  ret = tclib_obj.SaveConfiguration(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  tclib_obj.sess_ = NULL;
  ret = tclib_obj.SaveConfiguration(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;
  arg_count = 0;
  ret = tclib_obj.SaveConfiguration(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 1;
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.SaveConfiguration(tclib_obj.sess_);
  EXPECT_EQ(TC_SUCCESS, ret);

  // sess failure
  arg_count = 1;
  sessutil.set_read_type(LIB_COMMON);
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.SaveConfiguration(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  // handle interface failure
  arg_count = 1;
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  sessutil.set_read_type(LIB_COMMON);
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.SaveConfiguration(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_15, test_ClearStartup) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  // pTcLibInterface_ NULL
  ret = tclib_obj.ClearStartup(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  tclib_obj.sess_ = NULL;
  ret = tclib_obj.ClearStartup(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;
  arg_count = 0;
  ret = tclib_obj.ClearStartup(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 1;
  sessutil.set_read_type(LIB_COMMON);
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.ClearStartup(tclib_obj.sess_);
  EXPECT_EQ(TC_SUCCESS, ret);

  // sess failure1
  arg_count = 1;
  sessutil.set_read_type(LIB_COMMON);
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.ClearStartup(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  // handle interface failure
  arg_count = 1;
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  sessutil.set_read_type(LIB_COMMON);
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.ClearStartup(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_16, test_AbortCandidate) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  // pTcLibInterface_ NULL
  ret = tclib_obj.AbortCandidate(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  tclib_obj.sess_ = NULL;
  ret = tclib_obj.AbortCandidate(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;
  arg_count = 0;
  ret = tclib_obj.AbortCandidate(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 2;
  sessutil.set_read_type(LIB_ABORT_CANDIDATE);
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.AbortCandidate(tclib_obj.sess_);
  EXPECT_EQ(TC_SUCCESS, ret);

  // sess failure1
  arg_count = 2;
  sessutil.set_read_type(LIB_ABORT_CANDIDATE);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.AbortCandidate(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  // sess failure2
  arg_count = 2;
  sessutil.set_read_type(LIB_ABORT_CANDIDATE);
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = tclib_obj.AbortCandidate(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  // handle interface failure
  arg_count = 2;
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  sessutil.set_read_type(LIB_ABORT_CANDIDATE);
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.AbortCandidate(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}


TEST(test_17, test_Setup) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();


  // pTcLibInterface_ NULL
  ret = tclib_obj.Setup(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // handle interface failure
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  ret = tclib_obj.Setup(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_17, test_SetupComplete) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  // pTcLibInterface_ NULL
  ret = tclib_obj.SetupComplete(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // handle interface failure
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  ret = tclib_obj.SetupComplete(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_18, test_GetDriverId) {
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();
  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;

  // pTcLibInterface_ NULL
  ctr_type = tclib_obj.GetDriverId();
  EXPECT_EQ(UNC_CT_UNKNOWN, ctr_type);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  tclib_obj.sess_ = NULL;
  ctr_type = tclib_obj.GetDriverId();
  EXPECT_EQ(UNC_CT_UNKNOWN, ctr_type);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;
  arg_count = 0;
  ctr_type = tclib_obj.GetDriverId();
  EXPECT_EQ(UNC_CT_UNKNOWN, ctr_type);

  arg_count = 1;
  if_stub_obj.ctr_type = UNC_CT_PFC;
  sessutil.set_return_type(RETURN_SUCCESS);
  ctr_type = tclib_obj.GetDriverId();
  EXPECT_EQ(UNC_CT_PFC, ctr_type);

  // Failure
  arg_count = 1;
  if_stub_obj.ctr_type = UNC_CT_PFC;
  sessutil.set_return_type(RETURN_FAILURE);
  ctr_type = tclib_obj.GetDriverId();
  EXPECT_EQ(UNC_CT_UNKNOWN, ctr_type);

  tclib_obj.fini();
}

TEST(test_19, test_AuditConfig) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  // pTcLibInterface_ NULL
  ret = tclib_obj.AuditConfig(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  tclib_obj.sess_ = NULL;
  ret = tclib_obj.AuditConfig(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;
  arg_count = 0;
  ret = tclib_obj.AuditConfig(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 2;
  sessutil.set_read_type(LIB_AUDIT_CONFIG);
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.AuditConfig(tclib_obj.sess_);
  EXPECT_EQ(TC_SUCCESS, ret);

  // handle interface failure
  arg_count = 2;
  if_stub_obj.tclib_stub_failure_ = PFC_TRUE;
  sessutil.set_read_type(LIB_AUDIT_CONFIG);
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.AuditConfig(tclib_obj.sess_);
  EXPECT_EQ(TC_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_31, test_fini) {
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  EXPECT_EQ(NULL, tclib_obj.pTcLibInterface_);
  EXPECT_EQ(NULL, tclib_obj.sess_);
  EXPECT_EQ(MSG_NONE, tclib_obj.oper_state_);
  EXPECT_EQ(0, tclib_obj.key_map_.size());
  EXPECT_EQ(0, tclib_obj.controller_key_map_.size());
  EXPECT_EQ(0, tclib_obj.commit_phase_result_.size());
  EXPECT_EQ(PFC_FALSE, tclib_obj.audit_in_progress_);
}

}  // namespace tclib
}  // namespace unc

#endif
