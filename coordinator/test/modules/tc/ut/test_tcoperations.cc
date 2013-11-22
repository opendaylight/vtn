/*
 * Copyright (c) 2013 NEC Corporation
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

TEST(TcOperations, TcGetOperType) {
  SET_AUDIT_OPER_PARAMS();
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  EXPECT_EQ(TC_OPER_INVALID_INPUT, tcoperations.GetOperType());

  stub_srv_uint32 = -1;
  EXPECT_EQ(TC_OPER_FAILURE, tcoperations.GetOperType());

  stub_srv_uint32 = 10;
  EXPECT_EQ(TC_OPER_SUCCESS, tcoperations.GetOperType());
  DEL_AUDIT_PARAMS();
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
  DEL_AUDIT_PARAMS();
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
  DEL_AUDIT_PARAMS();
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
  DEL_AUDIT_PARAMS();
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
  DEL_AUDIT_PARAMS();
}


TEST(TcOperations, SendResponse) {
  SET_AUDIT_OPER_PARAMS();
  TestTcOperations tcoperations(tc_lock_,
                                &sess_,
                                db_handler,
                                unc_map_);
  TcOperStatus send_oper_status = TC_OPER_SUCCESS;

  EXPECT_EQ(0, tcoperations.SendResponse(send_oper_status));
  DEL_AUDIT_PARAMS();
}
