/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tclock.hh"
#include <uncxx/tc/libtc_common.hh>

TEST(TcLock, GetConfigName_Global)  {
  TestTcLock tclock;
  std::string config_name;
  TcConfigMode tc_mode = TC_CONFIG_GLOBAL;
  std::string vtn_name = "";
  config_name = tclock.GetConfigName(tc_mode, vtn_name);
  std::string expected_str = "global-mode";
  EXPECT_EQ(expected_str, config_name); 
}

TEST(TcLock, GetConfigName_Real)  {
  TestTcLock tclock;
  std::string config_name;
  TcConfigMode tc_mode = TC_CONFIG_REAL;
  std::string vtn_name = "";
  config_name = tclock.GetConfigName(tc_mode, vtn_name);
  std::string expected_str = "real-mode";
  EXPECT_EQ(expected_str, config_name); 
}

TEST(TcLock, GetConfigId) {

  TestTcLock tclock;
  tclock.tc_config_name_map_.clear();
  int prev_config_id  =  tclock.GetConfigId();

  EXPECT_EQ(2, prev_config_id);
}


TEST(TcLock, GetConfigName_Virtual)  {
  TestTcLock tclock;
  std::string config_name;
  TcConfigMode tc_mode = TC_CONFIG_VIRTUAL;
  std::string vtn_name = "";
  config_name = tclock.GetConfigName(tc_mode, vtn_name);
  std::string expected_str = "virtual-mode";
  EXPECT_EQ(expected_str, config_name); 
}

TEST(TcLock, GetConfigName_VTN)  {
  TestTcLock tclock;
  std::string config_name;
  TcConfigMode tc_mode = TC_CONFIG_VTN;
  std::string vtn_name = "vtn1";
  config_name = tclock.GetConfigName(tc_mode, vtn_name);
  std::string expected_str = "vtn1";
  EXPECT_EQ(expected_str, config_name); 
}

TEST(TcLock, FindConfigName_SUCCESS)  {
  TestTcLock tclock;
  std::string config_name = "vtn1";
  uint32_t session_id = 555;
  tclock.UpdateConfigData(session_id, config_name);  
  std::string find_config_name;
  EXPECT_EQ(TC_LOCK_SUCCESS, tclock.FindConfigName(session_id, 
                                              find_config_name));
  tclock.tc_config_name_map_.clear();
}

TEST(TcLock, FindConfigName_FAILURE)  {
  TestTcLock tclock;
  std::string config_name;
  uint32_t session_id = 555;
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, 
           tclock.FindConfigName(session_id, config_name));
}

TEST(TcLock, GetLock_INVALID_OPERATION)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT; 
  std::string config_name(tclock.GetConfigName(TC_CONFIG_GLOBAL, ""));
  tclock.UpdateConfigData(234, config_name);
  TcLockRet ret = tclock.GetLock(1, TC_OPERATION_NONE, TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_INVALID_OPERATION, ret);
  tclock.tc_config_name_map_.clear();
}

TEST(TcLock, GetLock_TcOperationIsAllowed_INVALID_UNC_STATE)  {
  TestTcLock tclock;
  TcLockRet ret = tclock.GetLock(1, TC_OPERATION_NONE, TC_AUDIT_DRIVER);
  EXPECT_EQ( TC_LOCK_INVALID_UNC_STATE, ret);
}

TEST(TcLock, GetLock_ACQUIRE_CONFIG_OPERATION_NOT_ALLOWED)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT; 
  std::string config_name(tclock.GetConfigName(TC_CONFIG_GLOBAL, ""));
  tclock.UpdateConfigData(540, config_name);
  tclock.tc_state_lock_.state_transition_in_progress = PFC_TRUE;
  TcLockRet ret = tclock.GetLock(501, 
                                 TC_ACQUIRE_CONFIG_SESSION,
                                 TC_WRITE_NONE,
                                 TC_CONFIG_REAL, "");
  EXPECT_EQ(TC_LOCK_OPERATION_NOT_ALLOWED, ret);
  tclock.tc_config_name_map_.clear();
}

TEST(TcLock, GetLock_ACQUIRE_CONFIG_INVALID_OPERATION)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT; 
  std::string config_name(tclock.GetConfigName(TC_CONFIG_GLOBAL, ""));
  tclock.UpdateConfigData(544, config_name);
  TcLockRet ret = tclock.GetLock(501, 
                                 TC_ACQUIRE_CONFIG_SESSION,
                                 TC_WRITE_NONE,
                                 TC_CONFIG_VTN, "");
  EXPECT_EQ(TC_LOCK_INVALID_OPERATION, ret);
  tclock.tc_config_name_map_.clear();
}

TEST(TcLock, GetLock_ACQUIRE_CONFIG_LOCK_BUSY)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT; 
  std::string config_name(tclock.GetConfigName(TC_CONFIG_GLOBAL, ""));
  tclock.UpdateConfigData(888, config_name);
  TcLockRet ret = tclock.GetLock(888, 
                                 TC_ACQUIRE_CONFIG_SESSION,
                                 TC_WRITE_NONE,
                                 TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  tclock.tc_config_name_map_.clear();
}


TEST(TcLock, GetLock_ACQUIRE_CONFIG_LOCK_SUCCESS)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT; 
  TcLockRet ret = tclock.GetLock(521, 
                                 TC_ACQUIRE_CONFIG_SESSION,
                                 TC_WRITE_NONE,
                                 TC_CONFIG_VTN, "VTN132");
  EXPECT_EQ(TC_LOCK_SUCCESS , ret);
  tclock.tc_config_name_map_.clear();
}

TEST(TcLock, GetLock_FORCE_ACQUIRE_CONFIG_OPERATION_NOT_ALLOWED)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT; 
  std::string config_name(tclock.GetConfigName(TC_CONFIG_GLOBAL, ""));
  tclock.UpdateConfigData(444, config_name);
  tclock.tc_state_lock_.state_transition_in_progress = PFC_TRUE;
  TcLockRet ret = tclock.GetLock(444, 
                                 TC_FORCE_ACQUIRE_CONFIG_SESSION,
                                 TC_WRITE_NONE,
                                 TC_CONFIG_VTN, "");
 
  EXPECT_EQ(TC_LOCK_OPERATION_NOT_ALLOWED, ret);
  tclock.tc_config_name_map_.clear();
}

TEST(TcLock, GetLock_FORCE_ACQUIRE_CONFIG_LOCK_BUSY_1)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT; 
  std::string config_name(tclock.GetConfigName(TC_CONFIG_GLOBAL, ""));
  tclock.UpdateConfigData(444, config_name);
  TcLockRet ret = tclock.GetLock(444, 
                                 TC_FORCE_ACQUIRE_CONFIG_SESSION,
                                 TC_WRITE_NONE,
                                 TC_CONFIG_VTN, "");
 
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  tclock.tc_config_name_map_.clear();
}

TEST(TcLock, GetLock_FORCE_ACQUIRE_CONFIG_SUCCESS)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT;
  std::string config_name(tclock.GetConfigName(TC_CONFIG_GLOBAL, ""));
  tclock.UpdateConfigData(11,config_name);
  tclock.tc_config_name_map_[config_name].is_notify_pending = PFC_FALSE;
  TcLockRet ret = tclock.GetLock(104,
                                 TC_FORCE_ACQUIRE_CONFIG_SESSION,
                                 TC_WRITE_NONE);

  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  tclock.tc_config_name_map_.clear();
}

TEST(TcLock, ReleaseLock_INVALID_OPERATION)  {
  TestTcLock tclock;
  TcLockRet ret = tclock.ReleaseLock(1, 1, TC_OPERATION_NONE, TC_WRITE_NONE);

  EXPECT_EQ(TC_LOCK_INVALID_OPERATION, ret);
}


TEST(TcLock, ReleaseLock_LOCK_NO_CONFIG_SESSION_EXIST)  {
  TestTcLock tclock;
  
  TcLockRet ret = tclock.ReleaseLock(1, 1, 
                        TC_RELEASE_CONFIG_SESSION, TC_WRITE_NONE);

  EXPECT_EQ(TC_LOCK_NO_CONFIG_SESSION_EXIST, ret);
}

TEST(TcLock, ReleaseLock_LOCK_NOT_ACQUIRED)  {
  TestTcLock tclock;
  
  tclock.UpdateConfigData(129, "vtn2");
  uint32_t config_id = tclock.prev_config_id_;
  tclock.tc_config_name_map_["vtn2"].is_taken = PFC_FALSE;
  TcLockRet ret = tclock.ReleaseLock(129, config_id, 
                        TC_RELEASE_CONFIG_SESSION, TC_WRITE_NONE);
  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_NOT_ACQUIRED, ret);
}

TEST(TcLock, ReleaseLock_LOCK_INVALID_CONFIG_ID)  {
  TestTcLock tclock;
  
  tclock.UpdateConfigData(129, "vtn2");
  TcLockRet ret = tclock.ReleaseLock(129, 11, 
                        TC_RELEASE_CONFIG_SESSION, TC_WRITE_NONE);
  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_INVALID_CONFIG_ID, ret);
}

TEST(TcLock, ReleaseLock_LOCK_BUSY)  {
  TestTcLock tclock;
  
  tclock.UpdateConfigData(129, "vtn2");
  uint32_t config_id = tclock.prev_config_id_;
  TcLockRet ret = tclock.ReleaseLock(129, config_id, 
                        TC_RELEASE_CONFIG_SESSION, TC_WRITE_NONE);

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_BUSY, ret);
}

TEST(TcLock, ReleaseLock_LOCK_SUCCESS)  {
  TestTcLock tclock;

  tclock.UpdateConfigData(129, "vtn2");
  uint32_t config_id = tclock.prev_config_id_;
  tclock.tc_config_name_map_["vtn2"].is_notify_pending = PFC_FALSE;
  TcLockRet ret = tclock.ReleaseLock(129, config_id,
                        TC_RELEASE_CONFIG_SESSION, TC_WRITE_NONE);

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
}

TEST(TcLock, AutoSaveEnable_LOCK_BUSY)  {
  TestTcLock tclock;
  tclock.tc_auto_save_.is_enable = PFC_TRUE;
  TcLockRet ret = tclock.AutoSaveEnable();

  EXPECT_EQ(TC_LOCK_BUSY, ret);
}


TEST(TcLock, AutoSaveEnable_LOCK_SUCCESS)  {
  TestTcLock tclock;
  tclock.tc_auto_save_.is_enable = PFC_FALSE;
  TcLockRet ret = tclock.AutoSaveEnable();

  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
}

TEST(TcLock, AutoSaveDisable_LOCK_SUCCESS)  {
  TestTcLock tclock;
  tclock.tc_auto_save_.is_enable = PFC_TRUE;
  TcLockRet ret = tclock.AutoSaveDisable();

  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
}


TEST(TcLock, AutoSaveDisable_LOCK_FAILURE)  {
  TestTcLock tclock;
  tclock.tc_auto_save_.is_enable = PFC_FALSE;
  TcLockRet ret = tclock.AutoSaveDisable();

  EXPECT_EQ(TC_LOCK_FAILURE, ret);
}

TEST(TcLock, GetConfigData_LOCK_INVALID_UNC_STATE)  {
  TestTcLock tclock;

  tclock.tc_state_lock_.current_state = TC_ACT_FAIL;

  uint32_t config_id;
  TcConfigMode tc_mode;
  std::string vtn_name;

  TcLockRet ret = tclock.GetConfigData(333,
                                        config_id,
                                        tc_mode,
                                        vtn_name);

  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE, ret);
}

TEST(TcLock, GetConfigData_LOCK_INVALID_SESSION_ID)  {
  TestTcLock tclock;

  uint32_t config_id;
  TcConfigMode tc_mode;
  std::string vtn_name;


  tclock.tc_state_lock_.current_state = TC_ACT;

  TcLockRet ret = tclock.GetConfigData(333,
                                        config_id,
                                        tc_mode,
                                        vtn_name);

  EXPECT_EQ( TC_LOCK_INVALID_SESSION_ID, ret);
}

TEST(TcLock, GetConfigData_LOCK_NO_CONFIG_SESSION_EXIST)  {
  TestTcLock tclock;
  uint32_t config_id;
  TcConfigMode tc_mode;
  std::string vtn_name;


  tclock.tc_state_lock_.current_state = TC_ACT;
  tclock.UpdateConfigData(221, "vtn31");
  tclock.tc_config_name_map_["vtn31"].is_taken = PFC_FALSE;
  TcLockRet ret = tclock.GetConfigData(221,
                                        config_id,
                                        tc_mode,
                                        vtn_name);
  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_NO_CONFIG_SESSION_EXIST, ret);
}

TEST(TcLock, GetConfigData_LOCK_SUCCESS)  {
  TestTcLock tclock;
  uint32_t config_id;
  TcConfigMode tc_mode;
  std::string vtn_name;

  tclock.tc_state_lock_.current_state = TC_ACT;
  tclock.UpdateConfigData(231, "vtn31");
  TcLockRet ret = tclock.GetConfigData(231,
                                        config_id,
                                        tc_mode,
                                        vtn_name);
  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
}

TEST(TcLock, NotifyConfigIdSessionIdDone_LOCK_INVALID_SESSION_ID)  {
  TestTcLock tclock;

  TcLockRet ret = tclock.NotifyConfigIdSessionIdDone(231,12, TC_NOTIFY_NONE);
  
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, ret);
}

TEST(TcLock, NotifyConfigIdSessionIdDone_LOCK_INVALID_OPERATION)  {
  TestTcLock tclock;
    
  tclock.UpdateConfigData(231, "vtn231");

  TcLockRet ret = tclock.NotifyConfigIdSessionIdDone(12, 231, TC_NOTIFY_NONE);
 
  tclock.tc_config_name_map_.clear(); 
  EXPECT_EQ(TC_LOCK_INVALID_OPERATION, ret);
}

TEST(TcLock, NotifyConfigIdSessionIdDone_LOCK_INVALID_CONFIG_ID)  {
  TestTcLock tclock;

  tclock.UpdateConfigData(232, "vtn231");

  TcLockRet ret = tclock.NotifyConfigIdSessionIdDone(12,232,TC_NOTIFY_ACQUIRE);
  
  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_INVALID_CONFIG_ID, ret);
}

TEST(TcLock, NotifyConfigIdSessionIdDone_LOCK_SUCCESS)  {
  TestTcLock tclock;

  tclock.UpdateConfigData(234, "vtn231");
  uint32_t config_id = tclock.prev_config_id_;

  TcLockRet ret = tclock.NotifyConfigIdSessionIdDone(config_id, 234, 
                                                     TC_NOTIFY_ACQUIRE);
  
  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
}

TEST(TcLock, TcMarkSessionId_LOCK_INVALID_SESSION_ID)  {
  TestTcLock tclock;

  TcLockRet ret = tclock.TcMarkSessionId( 234 );

  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, ret);
}

TEST(TcLock, TcMarkSessionId_LOCK_FAILURE)  {
  TestTcLock tclock;
  
  tclock.UpdateConfigData(234, "vtn143");
  tclock.tc_config_name_map_["vtn143"].marked_session_id = 234;
  TcLockRet ret = tclock.TcMarkSessionId( 234 );

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_FAILURE, ret);
}

TEST(TcLock, TcMarkSessionId_LOCK_SUCCESS)  {
  TestTcLock tclock;

  tclock.UpdateConfigData(234, "vtn143");
  TcLockRet ret = tclock.TcMarkSessionId( 234 );
  
  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
}

TEST(TcLock, GetMarkedSessionId_LOCK_INVALID_SESSION_ID) {
  TestTcLock tclock;

  TcLockRet ret = tclock.TcMarkSessionId( 234 );

  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, ret);
}

TEST(TcLock, GetSessionOperation_TC_NO_OPERATION_PROGRESS) {
  TestTcLock tclock;

  TcSessionOperationProgress ret = tclock.GetSessionOperation( 234 );

  EXPECT_EQ(TC_NO_OPERATION_PROGRESS, ret);
}

TEST(TcLock, GetSessionOperation_TC_CONFIG_NO_NOTIFY_PROGRESS)  {
  TestTcLock tclock;
  
  tclock.UpdateConfigData(234, "vtn125");
  tclock.tc_config_name_map_["vtn125"].notify_operation = TC_NOTIFY_NONE;
  TcSessionOperationProgress ret = tclock.GetSessionOperation( 234 );

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_CONFIG_NO_NOTIFY_PROGRESS, ret);
}

TEST(TcLock, GetSessionOperation_TC_CONFIG_NOTIFY_ACQUIRE_PROGRESS)  {
  TestTcLock tclock;

  tclock.UpdateConfigData(234, "vtn125");
  tclock.tc_config_name_map_["vtn125"].notify_operation = 
                                                        TC_NOTIFY_ACQUIRE;
  TcSessionOperationProgress ret = tclock.GetSessionOperation( 234 );

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_CONFIG_NOTIFY_ACQUIRE_PROGRESS, ret);
}


TEST(TcLock, GetSessionOperation_TC_CONFIG_NOTIFY_RELEASE_PROGRESS)  {
  TestTcLock tclock;

  tclock.UpdateConfigData(234, "vtn125");
  tclock.tc_config_name_map_["vtn125"].notify_operation = 
                                                        TC_NOTIFY_RELEASE;
  TcSessionOperationProgress ret = tclock.GetSessionOperation( 234 );

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_CONFIG_NOTIFY_RELEASE_PROGRESS, ret);
}

TEST(TcLock, AcquireConfigLock_TC_LOCK_OPERATION_NOT_ALLOWED)  {
  TestTcLock tclock;

  tclock.tc_state_lock_.state_transition_in_progress = PFC_TRUE;
  TcLockRet ret(tclock.AcquireConfigLock(123, TC_CONFIG_GLOBAL, ""));
  
  EXPECT_EQ(TC_LOCK_OPERATION_NOT_ALLOWED, ret);
}

TEST(TcLock, AcquireConfigLock_TC_LOCK_INVALID_OPERATION)  {
  TestTcLock tclock;

  tclock.tc_state_lock_.state_transition_in_progress = PFC_FALSE;
  TcLockRet ret(tclock.AcquireConfigLock(123, (TcConfigMode)5, ""));

  EXPECT_EQ(TC_LOCK_INVALID_OPERATION, ret);
}

TEST(TcLock, AcquireConfigLock_TC_LOCK_BUSY_1)  {
  TestTcLock tclock;

  tclock.tc_state_lock_.state_transition_in_progress = PFC_FALSE;
  tclock.tc_config_name_map_["vtn1"].is_notify_pending = PFC_TRUE;
  tclock.tc_config_name_map_["vtn1"].is_taken = PFC_TRUE;
  TcLockRet ret(tclock.AcquireConfigLock(123, TC_CONFIG_VTN, "vtn1"));
  
  EXPECT_EQ(ret, TC_LOCK_BUSY);
}

TEST(TcLock, AcquireConfigLock_TC_LOCK_BUSY_2)  {
  TestTcLock tclock;

  tclock.tc_state_lock_.state_transition_in_progress = PFC_FALSE;
  tclock.UpdateConfigData(234, "vtn125");
  TcLockRet ret(tclock.AcquireConfigLock(234, TC_CONFIG_VTN, "vtn125"));

  EXPECT_EQ(TC_LOCK_BUSY, ret);
}
TEST(TcLock, AcquireConfigLock_TC_LOCK_SUCCESS)  {
  TestTcLock tclock;

  tclock.tc_state_lock_.state_transition_in_progress = PFC_FALSE;
  TcLockRet ret(tclock.AcquireConfigLock(123, (TcConfigMode)3, "vtn1"));

  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
}
TEST(TcLock, ReleaseConfigLock_TC_LOCK_NO_CONFIG_SESSION_EXIST)  {
  TestTcLock tclock;

  TcLockRet ret(tclock.ReleaseConfigLock(1,2));

  EXPECT_EQ(TC_LOCK_NO_CONFIG_SESSION_EXIST, ret);
}

TEST(TcLock, ReleaseConfigLock_TC_LOCK_NOT_ACQUIRED)  {
  TestTcLock tclock;
  
  tclock.tc_config_name_map_.clear();
  tclock.UpdateConfigData(11, "vtn11");
  tclock.tc_config_name_map_["vtn11"].is_taken = PFC_FALSE;

  TcLockRet ret(tclock.ReleaseConfigLock(11,1));
   
  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_NOT_ACQUIRED, ret);
}

TEST(TcLock, ReleaseConfigLock_TC_LOCK_INVALID_CONFIG_ID)  {
  TestTcLock tclock;

  tclock.tc_config_name_map_.clear();
  tclock.UpdateConfigData(11, "vtn11");

  TcLockRet ret(tclock.ReleaseConfigLock(11,3));

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_INVALID_CONFIG_ID, ret);
}

TEST(TcLock, ReleaseConfigLock_TC_LOCK_BUSY)  {
  TestTcLock tclock;

  tclock.tc_config_name_map_.clear();
  tclock.UpdateConfigData(11, "vtn11");
  uint32_t config_id(tclock.prev_config_id_);

  TcLockRet ret(tclock.ReleaseConfigLock(11,config_id));

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_BUSY, ret);
}

TEST(TcLock, ReleaseConfigLock_TC_LOCK_SUCCESS)  {
  TestTcLock tclock;

  tclock.tc_config_name_map_.clear();
  tclock.UpdateConfigData(11, "vtn11");
  uint32_t config_id(tclock.prev_config_id_);
  tclock.tc_config_name_map_["vtn11"].is_notify_pending = PFC_FALSE;

  TcLockRet ret(tclock.ReleaseConfigLock(11,config_id));

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
}

TEST(TcLock, ForceAcquireConfigLock_TC_LOCK_OPERATION_NOT_ALLOWED)  {
  TestTcLock tclock;
  
  tclock.tc_state_lock_.state_transition_in_progress = PFC_TRUE;
  TcLockRet ret(tclock.ForceAcquireConfigLock(11));

  EXPECT_EQ(TC_LOCK_OPERATION_NOT_ALLOWED, ret);
}

TEST(TcLock, ForceAcquireConfigLock_TC_LOCK_BUSY)  {
  TestTcLock tclock;

  tclock.tc_state_lock_.state_transition_in_progress = PFC_FALSE;
  tclock.UpdateConfigData(11, "vtn11");

  TcLockRet ret(tclock.ForceAcquireConfigLock(11));

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_BUSY, ret);
}

TEST(TcLock, ForceAcquireConfigLock_TC_LOCK_SUCCESS)  {
  TestTcLock tclock;

  tclock.tc_state_lock_.state_transition_in_progress = PFC_FALSE;
  tclock.tc_config_name_map_.clear();

  TcLockRet ret(tclock.ForceAcquireConfigLock(11));

  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
}

TEST(TcLock, TcOperationIsAllowed_TC_ACT_FAIL_1)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT_FAIL;
  TcLockRet ret(tclock.TcOperationIsAllowed(11, TC_OPERATION_NONE));

  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE, ret);
}

TEST(TcLock, TcOperationIsAllowed_TC_ACT_FAIL_2)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT_FAIL;
  TcLockRet ret(tclock.TcOperationIsAllowed(11, TC_OPERATION_NONE));

  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE, ret);
}

TEST(TcLock, TcOperationIsAllowed_TC_INIT)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_INIT;
  TcLockRet ret(tclock.TcOperationIsAllowed(11, TC_OPERATION_NONE));

  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE, ret);
}

TEST(TcLock, TcOperationIsAllowed_TC_STOP)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_STOP;
  TcLockRet ret(tclock.TcOperationIsAllowed(11, TC_OPERATION_NONE));

  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE, ret);
}

TEST(TcLock, TcOperationIsAllowed_TC_ACT)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT;
  tclock.UpdateConfigData(11, "vtn123");
  tclock.tc_config_name_map_["vtn123"].marked_session_id = 11;

  TcLockRet ret(tclock.TcOperationIsAllowed(11,
                              TC_ACQUIRE_CONFIG_SESSION));

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_OPERATION_NOT_ALLOWED, ret);
}

TEST(TcLock, TcOperationIsAllowed_TC_ACT_SUCCESS)  {
  TestTcLock tclock;
  tclock.tc_state_lock_.current_state = TC_ACT;
  tclock.UpdateConfigData(11, "vtn123");
  tclock.tc_config_name_map_["vtn123"].marked_session_id = 0;

  TcLockRet ret(tclock.TcOperationIsAllowed(11,
                              TC_ACQUIRE_CONFIG_SESSION));

  tclock.tc_config_name_map_.clear();
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
}

TEST(TcLock, GetLock_WRITE_SESSION) {

  TcLock* tc_lock_ = new TcLock();
  uint32_t session_id = 100;
  //tc_lock_->session_id = 100;
  TcOperation tc_operation = TC_ACQUIRE_WRITE_SESSION;
  TcWriteOperation write_operation = TC_AUDIT_USER;
  TcConfigMode tc_mode = TC_CONFIG_GLOBAL;
  std::string vtn_name = "";

 //Invalid State
  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE,tc_lock_->GetLock(session_id,tc_operation,write_operation,tc_mode,vtn_name));

  tc_lock_->tc_state_lock_.current_state = TC_ACT;
//write_operation = TC_AUDIT_USER;
  EXPECT_EQ(TC_LOCK_SUCCESS,tc_lock_->GetLock(session_id,tc_operation,write_operation,tc_mode,vtn_name));

//Lock busy
  EXPECT_EQ(TC_LOCK_BUSY,tc_lock_->GetLock(session_id,tc_operation,write_operation,tc_mode,vtn_name));

  tc_lock_->tc_state_lock_.current_state = TC_ACT_FAIL;
 EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE,tc_lock_->GetLock(session_id,tc_operation,write_operation,tc_mode,vtn_name));

}

TEST(TcLock, AcquireWriteLock_AuditUser) {

  TcLock* tc_lock_ = new TcLock();
  uint32_t session_id = 100;

  TcWriteOperation write_operation = TC_WRITE_NONE;
  EXPECT_EQ(TC_LOCK_INVALID_OPERATION,tc_lock_->AcquireWriteLock(session_id,write_operation));
  write_operation = TC_AUDIT_USER;
  EXPECT_EQ(TC_LOCK_SUCCESS,tc_lock_->AcquireWriteLock(session_id,write_operation));
}


TEST(TcLock, AcquireWriteLock_AuditDriver) {
  TcLock* tc_lock_ = new TcLock();
  uint32_t session_id = 100;
  tc_lock_->TcSetSetupComplete(PFC_TRUE);

  TcWriteOperation write_operation = TC_AUDIT_DRIVER;
  EXPECT_EQ(TC_LOCK_SUCCESS,tc_lock_->AcquireWriteLock(session_id,write_operation));

  write_operation = TC_WRITE_NONE;
  EXPECT_EQ(TC_LOCK_INVALID_OPERATION,tc_lock_->AcquireWriteLock(session_id,write_operation));
}

TEST(TcLock, AcquireWriteLock_Audit) {
  TcLock* tc_lock_ = new TcLock();
  TcWriteOperation write_operation = TC_AUDIT_DRIVER;
  std::string config_name = "";
  uint32_t session_id = 10;
  tc_lock_->TcSetSetupComplete(PFC_TRUE);
  tc_lock_->tc_config_name_map_[config_name].is_taken = PFC_TRUE;
  tc_lock_->tc_config_name_map_[config_name].session_id = session_id;
  tc_lock_->tc_config_name_map_[config_name].config_id = TcLock::GetConfigId();
  //testing secondary wait

  write_operation = TC_COMMIT;
  EXPECT_EQ(TC_LOCK_SUCCESS,tc_lock_->AcquireWriteLock(session_id,write_operation));
}


TEST(TcLock, AcquireWriteLock_AuditWait) {
  TcLock* tc_lock_ = new TcLock();
  TcWriteOperation write_operation = TC_AUDIT_DRIVER;
  std::string config_name = "";
  uint32_t session_id = 11;
  tc_lock_->TcSetSetupComplete(PFC_TRUE);
  tc_lock_->tc_config_name_map_[config_name].is_taken = PFC_TRUE;
  tc_lock_->tc_config_name_map_[config_name].session_id = session_id;
  tc_lock_->tc_config_name_map_[config_name].config_id = TcLock::GetConfigId();
  //testing secondary wait

  write_operation = TC_ABORT_CANDIDATE_CONFIG;
  EXPECT_EQ(TC_LOCK_SUCCESS,tc_lock_->AcquireWriteLock(session_id,write_operation));
}

TEST(TcLock, GetLock_Auto_save_enable) {
  TcLock* tc_lock_ = new TcLock();
  tc_lock_->AutoSaveDisable();
  EXPECT_EQ(TC_LOCK_SUCCESS,tc_lock_->AutoSaveEnable());

}

TEST(TcLock, GetLock_Auto_Save_Get) {

  TcLock* tc_lock_ = new TcLock();
  EXPECT_EQ(TC_LOCK_SUCCESS,tc_lock_->AutoSaveEnable());

  tc_lock_->tc_auto_save_.is_enable = PFC_TRUE;
  EXPECT_EQ(TC_LOCK_BUSY,tc_lock_->AutoSaveEnable());
}


TEST(TcLock, GetLock_Auto_Save_Disable) {

  TcLock* tc_lock_ = new TcLock();
  tc_lock_->tc_auto_save_.is_enable = PFC_TRUE;
  EXPECT_EQ(TC_LOCK_SUCCESS,tc_lock_->AutoSaveDisable());

  tc_lock_->tc_auto_save_.is_enable = PFC_FALSE;
  EXPECT_EQ(TC_LOCK_FAILURE,tc_lock_->AutoSaveDisable());
}

TEST(TcLock, GetLock_states) {
  TcLock* tc_lock_ = new TcLock();
  uint32_t session_id = 100;
  TcOperation tc_operation = TC_ACQUIRE_READ_SESSION;
  TcWriteOperation write_operation = TC_AUDIT_USER;
  TcConfigMode tc_mode = TC_CONFIG_GLOBAL;
  std::string vtn_name = "";
  //Invalid State
  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE,tc_lock_->GetLock(session_id,tc_operation,write_operation,tc_mode,vtn_name));
  tc_lock_->tc_state_lock_.current_state = TC_ACT;
  EXPECT_EQ(TC_LOCK_SUCCESS,tc_lock_->GetLock(session_id,tc_operation,write_operation,tc_mode,vtn_name));
  tc_lock_->tc_state_lock_.current_state = TC_ACT;
   session_id = 100;
  EXPECT_EQ(TC_LOCK_ALREADY_ACQUIRED,tc_lock_->GetLock(session_id,tc_operation,write_operation,tc_mode,vtn_name));

}
