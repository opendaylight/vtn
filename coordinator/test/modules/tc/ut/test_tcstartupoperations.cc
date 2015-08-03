/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcstartupoperations.hh"
#include <uncxx/tc/libtc_common.hh>

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_string;
extern int stub_opertype;
extern int stub_set_string;
TEST(TcStartUpOperations , TcGetMinArgCount) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  int argcount = tc_startupoperations.TestTcGetMinArgCount();
  EXPECT_EQ(0, argcount);
  //// DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcCheckOperArgCount) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  int avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcCheckOperArgCount(avail_count));
  // DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcValidateOperType) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcValidateOperType());
  // DEL_AUDIT_PARAMS();
}


TEST(TcStartUpOperations , TcValidateOperParams) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcValidateOperParams());
  // DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcGetExclusion) {
  TestTcLock* tc_lock_ = new TestTcLock();
  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name = "UNC_DB_DSN";
  TcDbHandler* db_handler = new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;
  pfc_bool_t is_switch = true;

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcGetExclusion());
  // DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcReleaseExclusion) {
  TestTcLock*  tc_lock_ = new TestTcLock();
  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  std::string dsn_name = "UNC_DB_DSN";
  TcDbHandler* db_handler = new TcDbHandler(dsn_name);
  TcChannelNameMap  unc_map_;
  pfc_bool_t is_switch = true;

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcReleaseExclusion());
  // DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , HandleLockRet) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret = TC_LOCK_SUCCESS;
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.HandleLockRet(ret));
  // DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcCreateMsgList) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.is_switch_ = PFC_FALSE;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcCreateMsgList());
  // DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , TcCreateMsgList_Success) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.database_type_ = UNC_DT_RUNNING;
  tc_startupoperations.fail_oper_ = TC_OP_RUNNING_SAVE;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.TcCreateMsgList());
  // DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , FillTcMsgData_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TcMsg* tc_msg = NULL;
  stub_srv_uint32 = 1;
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_FAILURE,
  tc_startupoperations.FillTcMsgData(tc_msg, MSG_SAVE_CONFIG));
  // DEL_AUDIT_PARAMS();
}

TEST(TcStartUpOperations , FillTcMsgData_MSG_SETUP_COMPLETE) {
  SET_AUDIT_OPER_PARAMS();
  uint32_t sess_id =  CLEAR;
  unc::tclib::TcMsgOperType oper =  unc::tclib::MSG_SETUP;
  TcMsg* tc_msg =  TcMsg::CreateInstance(sess_id,  oper, GetTcChannelNameMap1(SET));
  stub_srv_uint32 = 1;
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.FillTcMsgData(tc_msg, MSG_SETUP_COMPLETE));
 }

/*TEST(TcStartUpOperations , FillTcMsgData_Success) {
  SET_AUDIT_OPER_PARAMS();
  //TcMsg* tc_msg = NULL;
  uint32_t sess_id =10;
  unc::tclib::TcMsgOperType oper = unc::tclib::MSG_AUDIT_GLOBAL;

  TcMsg* tc_msg = new TcMsg(sess_id,  oper);
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.FillTcMsgData(tc_msg, MSG_SETUP_COMPLETE));
}*/


TEST(TcStartUpOperations , SendAdditionalResponse) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_startupoperations.SendAdditionalResponse(oper_stat));
  // DEL_AUDIT_PARAMS();
}


/* When TcOperMessageList.size() == 0 */
TEST(TcStartUpOperations , Execute_FAILURE) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_FAILURE, tc_startupoperations.Execute());
}


TEST(TcStartUpOperations , HandleArgs_Failure) {
  TcLock* tc_lock_ = new TcLock();
  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess_(srv);
  TcChannelNameMap  unc_map_;
  pfc_bool_t is_switch = true ;

  std::string dsn = "UNC";
  TcDbHandler* db = new TcDbHandler(dsn);
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db,
                                           unc_map_,
                                           is_switch);
  EXPECT_EQ(TC_OPER_FAILURE , tc_startupoperations.HandleArgs());
}

/* COLD START: autosave_enabled = FALSE && fail_oper_ = TC_OP_RUNNING_SAVE */
TEST(TcStartUpOperations , Execute_COLDSTART_RUNNING_SAVE) {
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OPENFLOW,  "drvpfcd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OVERLAY,  "drvoverlay")));

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.HandleArgs();
  tc_startupoperations.fail_oper_ = TC_OP_RUNNING_SAVE;
  tc_startupoperations.database_type_ = UNC_DT_STARTUP;
  tc_startupoperations.is_switch_ = PFC_FALSE;
  tc_startupoperations.TcCreateMsgList();
  stub_set_string = 1;
  //tc_startupoperations.Execute();
  EXPECT_EQ(TC_OPER_SUCCESS, tc_startupoperations.Execute());
  stub_set_string = 0;
}

/* COLD START: autosave_enabled = FALSE && fail_oper_ = TC_OP_CLEAT_STARTUP */
TEST(TcStartUpOperations , Execute_COLDSTART_CLEAR_STARTUP) {
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OPENFLOW,  "drvpfcd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OVERLAY,  "drvoverlay")));

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.HandleArgs();
  tc_startupoperations.fail_oper_ = TC_OP_CLEAR_STARTUP;
  tc_startupoperations.database_type_ = UNC_DT_STARTUP;
  tc_startupoperations.is_switch_ = PFC_FALSE;
  tc_startupoperations.TcCreateMsgList();
  stub_set_string = 1;
  //tc_startupoperations.Execute();
  EXPECT_EQ(TC_OPER_SUCCESS, tc_startupoperations.Execute());
  stub_set_string = 0;
}


/* COLD START: autosave_enabled = TRUE && fail_oper_ = TC_OP_CANDIDATE_COMMIT */
TEST(TcStartUpOperations , Execute_COLDSTART_Candidate) {
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OPENFLOW,  "drvpfcd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OVERLAY,  "drvoverlay")));

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.HandleArgs();
  tc_startupoperations.fail_oper_ = TC_OP_CANDIDATE_COMMIT;
  tc_startupoperations.database_type_ = UNC_DT_RUNNING;
  tc_startupoperations.is_switch_ = PFC_FALSE;
  tc_startupoperations.TcCreateMsgList();
  stub_set_string = 1;
  //tc_startupoperations.Execute();
  EXPECT_EQ(TC_OPER_SUCCESS, tc_startupoperations.Execute());
  stub_set_string = 0;
}

/* TcOperMessageList.empty() == FALSE ->  Not Empty */
TEST(TcStartUpOperations , TcCreateMsgList_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.TcOperMessageList.push_back(unc::tclib::MSG_AUDITDB);
  EXPECT_EQ(TC_OPER_FAILURE , tc_startupoperations.TcCreateMsgList());
}

/* TcOperMessageList.empty() == TRUE && autosave = TRUE & fail_oper_ = TC_OP_CANDIDATE_COMMIT --> COLD START */
TEST(TcStartUpOperations , TcCreateMsgList_CANDIDATE) {
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OPENFLOW,  "drvpfcd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OVERLAY,  "drvoverlay")));

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.HandleArgs();
  tc_startupoperations.fail_oper_ = TC_OP_CANDIDATE_COMMIT;
  tc_startupoperations.database_type_ = UNC_DT_RUNNING;
  tc_startupoperations.is_switch_ = PFC_FALSE;
  EXPECT_EQ(TC_OPER_SUCCESS , tc_startupoperations.TcCreateMsgList());
}

/* TcOperMessageList.empty() == TRUE && autosave = FALSE & fail_oper_ = TC_OP_CLEAT_STARTUP --> COLD START */
TEST(TcStartUpOperations , TcCreateMsgList_TC_OP_CLEAR_STARTUP) {
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OPENFLOW,  "drvpfcd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OVERLAY,  "drvoverlay")));

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.HandleArgs();
  tc_startupoperations.fail_oper_ = TC_OP_CLEAR_STARTUP;
  tc_startupoperations.database_type_ = UNC_DT_STARTUP;
  tc_startupoperations.is_switch_ = PFC_FALSE;
  EXPECT_EQ(TC_OPER_SUCCESS , tc_startupoperations.TcCreateMsgList());
}

/* COLD START: autosave_enabled = FALSE && fail_oper_ = TC_OP_RUNNING_SAVE */
TEST(TcStartUpOperations , TcCreateMsgList_TC_OP_RUNNING_SAVE) {
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OPENFLOW,  "drvpfcd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OVERLAY,  "drvoverlay")));

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.HandleArgs();
  tc_startupoperations.fail_oper_ = TC_OP_RUNNING_SAVE;
  tc_startupoperations.database_type_ = UNC_DT_STARTUP;
  tc_startupoperations.is_switch_ = PFC_FALSE;
  EXPECT_EQ(TC_OPER_SUCCESS , tc_startupoperations.TcCreateMsgList());
}

/* switch/fail over */
TEST(TcStartUpOperations , TcCreateMsgList_switch_over) {
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OPENFLOW,  "drvpfcd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_DRV_OVERLAY,  "drvoverlay")));

  TestTcStartUpOperations  tc_startupoperations(tc_lock_, &sess_,
                                           db_handler,
                                           unc_map_,
                                           is_switch);
  tc_startupoperations.fail_oper_ = TC_OP_RUNNING_SAVE;
  tc_startupoperations.database_type_ = UNC_DT_STARTUP;

  EXPECT_EQ(TC_OPER_SUCCESS , tc_startupoperations.TcCreateMsgList());
}

/*-------------------------- U17-CR --------------------------*/
