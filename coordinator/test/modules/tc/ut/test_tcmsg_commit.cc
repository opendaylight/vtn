/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcmsg_commit.hh"

TEST(TwoPhaseCommit, TestHandleDriver) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_START;
  Test2phaseCommit TestphasecommitObj(sess_id,  opertype);
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  //Test2phaseCommit TestphasecommitObj;
  ret_val =  TestphasecommitObj.TestHandleDriverNotPresent(UNC_CT_POLC);
  EXPECT_EQ(TCOPER_RET_FAILURE, ret_val);
}

/*Cannot find driver in driverinfo_map_*/
TEST(TwoPhaseCommit, TestHandleDriver_Keytype_POLC_Failure) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_START;
  Test2phaseCommit TestphasecommitObj(sess_id,  opertype);
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  TestTwophaseAudit Test2PhaseAuditObj(sess_id,  opertype);
  unc_keytype_ctrtype_t driver_type = UNC_CT_POLC;
  typedef std::list<std::string> TestControllerList;
  TestControllerList controllers_test;
  controllers_test.push_back("sample1");
  controllers_test.push_back("sample2");
  controllers_test.push_back("sample3");
  ret_val =  TestphasecommitObj.TestHandleDriverNotPresent(driver_type);
  EXPECT_EQ(TCOPER_RET_FAILURE, ret_val);
}

TEST(TwoPhaseCommit, TestHandleDriver_Keytype_Invalid_Failure) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_START;
  Test2phaseCommit TestphasecommitObj(sess_id,  opertype);
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  TwoPhaseAudit Test2PhaseAuditObj(sess_id,  opertype);
  unc_keytype_ctrtype_t driver_type = (unc_keytype_ctrtype_t)5;
  typedef std::list<std::string> TestControllerList;
  TestControllerList controllers_test;
  controllers_test.push_back("sample1");
  controllers_test.push_back("sample2");
  controllers_test.push_back("sample3");
  //unc::tc::TcDriverInfoMap driverinfo_map_;
  /*Test2PhaseAuditObj.driverinfo_map_.insert(std::pair<unc_keytype_ctrtype_t,
                             TestControllerList>(driver_type, controllers_test));*/
  //driverinfo_map_.insert(std::pair<unc_keytype_ctrtype_t, std::vector<std::string>('UNC_CT_POLC',"sample") );
  ret_val =  TestphasecommitObj.TestHandleDriverNotPresent(driver_type);
  EXPECT_EQ(TCOPER_RET_FAILURE, ret_val);
}
#if 0
TEST(TwoPhaseCommit, TestHandleDriver_Keytype_POLC_Success) {
  uint32_t sess_id =  SET;
  TcMsgOperType opertype =  MSG_AUDIT_START;
  Test2phaseCommit TestphasecommitObj(sess_id,  opertype);
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  TestTwophaseAudit Test2PhaseAuditObj(sess_id,  opertype);
  unc_keytype_ctrtype_t driver_type = UNC_CT_POLC;
  typedef std::list<std::string> TestControllerList;
  TestControllerList controllers_test;
  controllers_test.push_back("sample1");
  controllers_test.push_back("sample2");
  controllers_test.push_back("sample3");
  //unc::tc::TcDriverInfoMap driverinfo_map_;
  Test2PhaseAuditObj.driverinfo_map_.insert(std::pair<unc_keytype_ctrtype_t,
                             TestControllerList>(driver_type, controllers_test));
  //driverinfo_map_.insert(std::pair<unc_keytype_ctrtype_t, std::vector<std::string>('UNC_CT_POLC',"sample") );
  ret_val =  TestphasecommitObj.TestHandleDriverNotPresent(driver_type);
  EXPECT_EQ(TCOPER_RET_SUCCESS, ret_val);
}

TEST(TwoPhaseCommit, SendRequestToDriver_DriverNotPresent) {
  TcOperRet retval =  TCOPER_RET_SUCCESS;
  //std::string channel_name =  "drvpfcd";
  std::string channel_name = "";
  ControllerList clist;
  pfc_ipcconn_t conn =  0;

  Test2phaseCommit C2phase(SET,  MSG_COMMIT_VOTE);
  pfc::core::ipc::ClientSession* c_sess
      =  TcClientSessionUtils::create_tc_client_session(channel_name,
                                unc::tclib::TCLIB_COMMIT_TRANSACTION,
                                conn);
  retval =  C2phase.TestGetControllerInfo(c_sess);

  C2phase.SetData(SET, "", UNC_CT_UNKNOWN);
  C2phase.channel_names_ =  GetChannelNameMap_commit(SET);
  retval =  C2phase.TestSendRequestToDriver();
  EXPECT_EQ(TCOPER_RET_SUCCESS,  retval);
}
#endif
