/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcautosaveoperations.hh"
#include <uncxx/tc/libtc_common.hh>

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_string;
extern int stub_opertype;
TEST(TcAutoSaveOperations, TcGetMinArgCount) {
  SET_AUDIT_OPER_PARAMS();

  TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  int argcount = tc_autosaveoperations.TestTcGetMinArgCount();
  EXPECT_EQ(2, argcount);
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, TcCheckOperArgCount) {
  SET_AUDIT_OPER_PARAMS();
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);

  uint32_t avail_count = 5;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
            tc_autosaveoperations.TcCheckOperArgCount(avail_count));

  avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_autosaveoperations.TcCheckOperArgCount(avail_count));
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, GetSessionId) {
  SET_AUDIT_OPER_PARAMS();
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);

  tc_autosaveoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  stub_srv_uint32 = 100;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
            tc_autosaveoperations.GetSessionId());
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, GetSessionId_UserAudit) {
  SET_AUDIT_OPER_PARAMS();
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  tc_autosaveoperations.tc_oper_ = TC_OP_USER_AUDIT;
  stub_srv_uint32 = 20;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
            tc_autosaveoperations.GetSessionId());
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, GetSessionId_Invalid) {
  SET_AUDIT_OPER_PARAMS();
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  tc_autosaveoperations.tc_oper_ = TC_OP_USER_AUDIT;
  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
            tc_autosaveoperations.GetSessionId());
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, TcValidateOperType) {
  SET_AUDIT_OPER_PARAMS();
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);

  tc_autosaveoperations.tc_oper_ =  TC_OP_CONFIG_ACQUIRE;
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE,
            tc_autosaveoperations.TcValidateOperType());

  tc_autosaveoperations.tc_oper_ = TC_OP_AUTOSAVE_ENABLE;
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_autosaveoperations.TcValidateOperType());
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, TcValidateOperParams) {
  SET_AUDIT_OPER_PARAMS();
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  tc_autosaveoperations.tc_oper_= TC_OP_USER_AUDIT;
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_autosaveoperations.TcValidateOperParams());

  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_autosaveoperations.TcValidateOperParams());

  tc_autosaveoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  stub_srv_string = 1;
  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_autosaveoperations.TcValidateOperParams());
  stub_srv_string = 0;

  EXPECT_EQ(TC_OPER_SUCCESS,
            tc_autosaveoperations.TcValidateOperParams());

  tc_autosaveoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  stub_opertype = TC_OP_DRIVER_AUDIT;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_autosaveoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, TcGetExclusion) {
  TestTcLock* tc_lock_ = new TestTcLock();
  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name = "UNC_DB_DSN";
  TcDbHandler* db_handler = new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;

  TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  uint session_id_ = 10;
  tc_lock_->GetLock(session_id_, TC_AUTO_SAVE_ENABLE, TC_WRITE_NONE);
  //oper is invalid
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.TcGetExclusion());
  // DEL_AUDIT_PARAMS();
}


TEST(TcAutoSaveOperations, TcReleaseExclusion) {
  TestTcLock*  tc_lock_ = new TestTcLock();
  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name = "UNC_DB_DSN";
  TcDbHandler* db_handler = new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;

    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  tc_autosaveoperations.tc_oper_ = TC_OP_USER_AUDIT;
  tc_autosaveoperations.tclock_ = tc_lock_;
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.TcReleaseExclusion());
  tc_autosaveoperations.tc_oper_ = TC_OP_DRIVER_AUDIT;
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.TcReleaseExclusion());
  // DEL_AUDIT_PARAMS();
}


TEST(TcAutoSaveOperations, HandleLockRet) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret = TC_LOCK_SUCCESS;
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);

  ret = TC_LOCK_INVALID_UNC_STATE;
  EXPECT_EQ(TC_INVALID_STATE, tc_autosaveoperations.HandleLockRet(ret));

  ret = TC_LOCK_OPERATION_NOT_ALLOWED;
  EXPECT_EQ(TC_INVALID_STATE, tc_autosaveoperations.HandleLockRet(ret));

  ret = TC_LOCK_BUSY;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_autosaveoperations.HandleLockRet(ret));

  ret = TC_LOCK_NO_CONFIG_SESSION_EXIST;
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.HandleLockRet(ret));

  ret = TC_LOCK_INVALID_PARAMS;
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.HandleLockRet(ret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, TcCreateMsgList) {
  SET_AUDIT_OPER_PARAMS();
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_autosaveoperations.TcCreateMsgList());
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, SetAutoSave_enabled) {
  SET_AUDIT_OPER_PARAMS();
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  tc_autosaveoperations.autosave_ = PFC_TRUE;
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.SetAutoSave());
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, SetAutoSave_disabled) {
  SET_AUDIT_OPER_PARAMS();
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.SetAutoSave());
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, SetAutoSavefailure) {
  SET_AUDIT_OPER_PARAMS();
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  stub_srv_uint32 = 0;
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.SetAutoSave());
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, FillTcMsgData) {
  SET_AUDIT_OPER_PARAMS();
  TcMsg* tc_msg = NULL;
  stub_srv_uint32 = 1;
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_autosaveoperations.FillTcMsgData(tc_msg, MSG_SAVE_CONFIG));
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, SendAdditionalResponse_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_autosaveoperations.SendAdditionalResponse(oper_stat));
  // DEL_AUDIT_PARAMS();
}

TEST(TcAutoSaveOperations, SendAdditionalResponse_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  stub_srv_uint32 = 0;
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE,
  tc_autosaveoperations.SendAdditionalResponse(oper_stat));
  // DEL_AUDIT_PARAMS();
}
#if 0
TEST(TcAutoSaveOperations, Execute) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.Execute());

  // Case1
  tc_autosaveoperations.autosave_ = PFC_TRUE;
  tc_autosaveoperations.tc_oper_ = TC_OP_AUTOSAVE_GET;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_autosaveoperations.Execute());

  // Case 2
  tc_autosaveoperations.autosave_ = PFC_TRUE;
  tc_autosaveoperations.tc_oper_ = TC_OP_AUTOSAVE_ENABLE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_autosaveoperations.Execute());

  tc_autosaveoperations.autosave_ = PFC_FALSE;
  tc_autosaveoperations.tc_oper_ = TC_OP_AUTOSAVE_ENABLE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_autosaveoperations.Execute());

  // case 3
  tc_autosaveoperations.autosave_ = PFC_TRUE;
  tc_autosaveoperations.tc_oper_ = TC_OP_AUTOSAVE_DISABLE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_autosaveoperations.Execute());

  // Case 4
  tc_autosaveoperations.autosave_ = PFC_TRUE;
  tc_autosaveoperations.tc_oper_ = TC_OP_AUTOSAVE_DISABLE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_autosaveoperations.Execute());
  // DEL_AUDIT_PARAMS();

  // Case 5
  tc_autosaveoperations.autosave_ = PFC_TRUE;
  tc_autosaveoperations.tc_oper_ = TC_OP_INVALID;
  EXPECT_EQ(TC_SYSTEM_FAILURE, tc_autosaveoperations.Execute());

  // Case 6
  tc_autosaveoperations.autosave_ = PFC_FALSE;
  tc_autosaveoperations.tc_oper_ = TC_OP_AUTOSAVE_DISABLE;
  EXPECT_EQ(TC_SYSTEM_FAILURE, tc_autosaveoperations.Execute());

}
#endif
TEST(TcAutoSaveOperations, Execute_TC_OP_INVALID) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  // Case 5
  tc_autosaveoperations.autosave_ = PFC_TRUE;
  tc_autosaveoperations.tc_oper_ = TC_OP_INVALID;
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.Execute());
}
#if 0
TEST(TcAutoSaveOperations, Execute_TC_OP_AUTOSAVE_DISABLE) {
  SET_AUDIT_OPER_PARAMS();
  TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  // Case 6
  tc_autosaveoperations.autosave_ = PFC_FALSE;
  tc_autosaveoperations.tc_oper_ = TC_OP_AUTOSAVE_ENABLE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_autosaveoperations.Execute());//Need to verify
}
#endif

/*TEST(TcAutoSaveOperations, Execute_Autosave) {
  SET_AUDIT_OPER_PARAMS();
  pfc_bool_t* auto_save = NULL;
    TestTcAutoSaveOperations tc_autosaveoperations(tc_lock_,
                                                 &sess_,
                                                 db_handler,
                                                 unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE, tc_autosaveoperations.Execute());
  tc_autosaveoperations.autosave_ = PFC_TRUE;
  db_handler->GetConfTable(auto_save);

  EXPECT_EQ(TC_OPER_SUCCESS, tc_autosaveoperations.Execute());
}*/
