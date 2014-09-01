/* Copyright (c) 2012-2013 NEC Corporation                 */
/* NEC CONFIDENTIAL AND PROPRIETARY                        */
/* All rights reserved by NEC Corporation.                 */
/* This program must be used solely for the purpose for    */
/* which it was furnished by NEC Corporation. No part      */
/* of this program may be reproduced or disclosed to       */
/* others,  in any form,  without the prior written          */
/* permission of NEC Corporation. Use of copyright         */
/* notice does not evidence publication of the program.    */

#include "test_tcmsg_audit.hh"

TEST(AuditTransaction, ExcutePOLC_UNC_CT_MAX) {
  TcMsgOperType oper =  MSG_AUDIT_TRANS_START;
  TcOperRet retval =  TCOPER_RET_SUCCESS;

  AuditTransaction Audit(SET,  oper);
  Audit.channel_names_ =  GetChannelNameMapAudit(SET);
  Audit.driver_id_ =  (unc_keytype_ctrtype_t)(UNC_CT_POLC + 1);
  retval =  Audit.Execute();
  EXPECT_EQ(TCOPER_RET_FAILURE,  retval);
}

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

