/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcmsg_audit.hh"

TEST(AuditTransaction, Excute_UNC_CT_POLC) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  AuditTransaction AuditTest(SET,  MSG_AUDIT_START);
  AuditTest.SetData(SET, "C1", UNC_CT_POLC);
  AuditTest.channel_names_ =  GetChannelNameMapAudit(SET);
  retval =  AuditTest.Execute();
  EXPECT_EQ(TCOPER_RET_NO_DRIVER,  retval);
}

TEST(AuditTransaction, Excute_UNC_CT_POLC_NoChannelNames) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  AuditTransaction AuditTest(SET, MSG_COMMIT_TRANS_END);
  AuditTest.SetData(SET, "C1", UNC_CT_POLC);
  AuditTest.channel_names_ =  GetChannelNameMapAudit(SET);
  retval =  AuditTest.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

TEST(AuditTransaction, Excute_MSG_AUDIT_TRANS_END) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  AuditTransaction AuditTest(SET, MSG_AUDIT_TRANS_END);
  AuditTest.SetData(SET, "C1", UNC_CT_POLC);
  AuditTest.channel_names_ =  GetChannelNameMapAudit(SET);
  retval =  AuditTest.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}


TEST(AuditTransaction, Execute_Test) {
  TcMsgOperType oper =  MSG_AUDIT_TRANS_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  AuditTransaction Audit(SET,  oper);
  Audit.channel_names_ =  GetChannelNameMapAudit(SET);
  Audit.driver_id_ =  UNC_CT_UNKNOWN;
  retval =  Audit.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);

  Audit.channel_names_ =  GetChannelNameMapAudit(SET);
  Audit.SetData(SET, "", UNC_CT_PFC);
  retval =  Audit.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);

  /*invalid opertype*/
  AuditTransaction Audit1(SET,  MSG_NONE);
  Audit1.SetData(1, "C1", UNC_CT_PFC);
  Audit1.channel_names_ =  GetChannelNameMapAudit(SET);
  retval =  Audit1.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);

  /*proper request*/
  AuditTransaction Audit2(SET,  MSG_AUDIT_START);
  Audit2.SetData(SET, "C1", UNC_CT_PFC);
  Audit2.channel_names_ =  GetChannelNameMapAudit(SET);
  retval =  Audit2.Execute();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}

