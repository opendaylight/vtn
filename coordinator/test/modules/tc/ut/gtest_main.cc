/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef _UNC_TCLOCK_GTEST_MAIN_HH_
#define _UNC_TCLOCK_GTEST_MAIN_HH_

#include <iostream>
#include <gtest/gtest.h>
#include <tc_module_data.hh>
#include <unc/tc/external/tc_services.h>
#include <tc_lock.hh>
#include <pfc/log.h>

using namespace std;
using namespace unc;
using namespace tc;

namespace unc {
namespace tc {

extern "C" {
  extern void libpfc_init();
  extern void libpfc_fini();
}

TcLock lock_obj;
TcLockRet  ret;
TcState state;
TcSessionOperationProgress op;
pfc_bool_t bol_ret;
uint32_t session_id = 10000;
uint32_t config_id = 10000;
TcConfigMode config_mode;
std::string vtn_name;

void *thread_function_4(void *arg) {
  sleep(5);
  TcState state;
  lock_obj.TcUpdateUncState(TC_ACT);
  state =lock_obj.GetUncCurrentState();
  EXPECT_EQ(TC_ACT,state);
 
  lock_obj.TcSetSetupComplete(PFC_TRUE); 
  cout<<"Executing Thread 4"<<endl; 
  ret = lock_obj.GetLock(23, TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  cout<<" Thread 4 Got Read Lock "<<endl;  
  
  sleep(13);
  ret = lock_obj.ReleaseLock(23, config_id,TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  sleep(3);
  // audit driver
  ret = lock_obj.GetLock(23, TC_ACQUIRE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  cout<<"thread 4  Got Write Lock "<<endl;
  sleep(10);
  ret = lock_obj.ReleaseLock(23, config_id,TC_RELEASE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  cout<<" Thread 4 Done "<<endl; 
  
  lock_obj.TcUpdateUncState(TC_STOP);
  ret = lock_obj.GetLock(24, TC_ACQUIRE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE, ret);
  
  return 0;
}

void *thread_function_3(void *arg) {
  
  sleep(3);
  //TcLockRet  ret;
  TcState state;
  lock_obj.TcUpdateUncState(TC_ACT);
  state =lock_obj.GetUncCurrentState();
  EXPECT_EQ(TC_ACT,state);
  sleep(8); 
  cout<<"Executing Thread 3"<<endl; 

  lock_obj.TcSetSetupComplete(PFC_TRUE); 
  ret = lock_obj.GetLock(33, TC_ACQUIRE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_BUSY , ret);
  
  cout<<" thread 3 Got Write Lock "<<endl;  
  ret = lock_obj.ReleaseLock(33, config_id,TC_RELEASE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_NOT_ACQUIRED, ret);

  sleep(6);
  ret = lock_obj.GetLock(33, TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  cout<<"Thread 3 Got read Lock "<<endl;

  ret = lock_obj.ReleaseLock(33, config_id,TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  
  cout<<" Thread 3 Done"<<endl; 
  // current state is STOP then we cant update state 
  state = lock_obj.GetUncCurrentState();
  EXPECT_EQ(TC_ACT,state);
  lock_obj.TcUpdateUncState(TC_ACT);
  state = lock_obj.GetUncCurrentState();
  EXPECT_EQ(TC_ACT,state);

  return 0;
}

void *thread_function_1(void *arg) {
  cout<<"Executing Thread 1"<<endl; 
  state =lock_obj.GetUncCurrentState();
  EXPECT_EQ(TC_INIT,state);
  
  lock_obj.TcUpdateUncState(TC_INIT);
  uint32_t sess_id = 123 ,conf_id;

  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE, ret);
  ret = lock_obj.GetConfigData(sess_id, conf_id, config_mode, vtn_name);
  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE, ret);

  ret = lock_obj.GetLock(11, TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  ret = lock_obj.ReleaseLock(11, config_id,TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetLock(12, TC_ACQUIRE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE, ret);


  lock_obj.TcUpdateUncState(TC_ACT);
  state = lock_obj.GetUncCurrentState();
  EXPECT_EQ(TC_ACT,state);

  ret = lock_obj.GetLock(session_id, TC_OPERATION_NONE,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_INVALID_OPERATION, ret);

  ret = lock_obj.ReleaseLock(session_id,config_id,TC_OPERATION_NONE,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_INVALID_OPERATION, ret);

  ret = lock_obj.GetLock(11, TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_OPERATION_NOT_ALLOWED, ret);
  ret = lock_obj.ReleaseLock(11, config_id,TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.ReleaseLock(session_id, config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,99,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, ret);


  ret = lock_obj.NotifyConfigIdSessionIdDone(111,session_id,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_INVALID_CONFIG_ID, ret);
  /* both session and confi ID is invalid */
  ret = lock_obj.NotifyConfigIdSessionIdDone(111,99,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, ret);

  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);


  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE,TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.ReleaseLock(session_id, 55,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_INVALID_CONFIG_ID, ret);

  ret = lock_obj.ReleaseLock(55, config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(unc::tc::TC_LOCK_NO_CONFIG_SESSION_EXIST, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.ReleaseLock(session_id, config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  /* Notify pending */
  ret = lock_obj.GetLock(session_id,TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  ret = lock_obj.GetLock(session_id,TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  /* Notify release config and session id*/
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_RELEASE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  //EXPECT_EQ(lock_obj.GetMarkedSessionId(session_id), 0);
  lock_obj.GetLock(session_id,TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  ret = lock_obj.ReleaseLock(session_id+1, config_id, TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_NO_CONFIG_SESSION_EXIST, ret);
  // TC_LOCK_BUSY

  lock_obj.tc_config_name_map_.clear();
  session_id =100;
  ret = lock_obj.GetLock(session_id,TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_VTN, "VTN_324");
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  lock_obj.GetLock(234,TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_VTN, "VTN_1");
  ret = lock_obj.GetConfigData(234, config_id, config_mode, vtn_name);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,234,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  lock_obj.tc_config_name_map_.clear();
  session_id =1340;
  lock_obj.GetLock(session_id,TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_VTN, "VTN_324");
  lock_obj.NotifyConfigIdSessionIdDone(17, 1340, TC_NOTIFY_ACQUIRE);
  ret = lock_obj.GetLock(session_id+1, TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  
  lock_obj.tc_config_name_map_.clear();
  session_id =175;
  lock_obj.GetLock(session_id,TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_VTN, "VTN_300");
  ret = lock_obj.GetLock(839,TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  /* commit flag is set */
  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.ReleaseLock(session_id, config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.GetLock(session_id+3,TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  ret = lock_obj.ReleaseLock(session_id, config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);


  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.ReleaseLock(session_id, config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_RELEASE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);


  /* config commit force acquire */
  ret = lock_obj.GetLock(session_id,TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  /* Config notify pending */ 
  ret = lock_obj.GetLock(session_id,TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetLock(session_id,TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  /* commint in progress for force acquire config  */
  ret = lock_obj.GetLock(session_id, TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.GetLock(session_id+1, TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  /* commit in progrss for release config */ 
  ret = lock_obj.ReleaseLock(session_id, config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.ReleaseLock(session_id, config_id,TC_RELEASE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.ReleaseLock(session_id, config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_RELEASE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  /*  max config id */
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.GetLock(session_id, TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetLock(1, TC_ACQUIRE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  ret = lock_obj.GetLock(1, TC_ACQUIRE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_ALREADY_ACQUIRED, ret);

  ret = lock_obj.GetLock(51, TC_ACQUIRE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  /* Write lock acquire commit */
  
  lock_obj.tc_config_name_map_.clear();
  lock_obj.GetLock(51, TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  ret = lock_obj.GetLock(51, TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  /* Write lock acquire audit user */
  ret = lock_obj.GetLock(51, TC_ACQUIRE_WRITE_SESSION,TC_AUDIT_USER);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  /* Write lock acquire save startup config */
  ret = lock_obj.GetLock(51, TC_ACQUIRE_WRITE_SESSION,TC_SAVE_STARTUP_CONFIG);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  /* Write lock acquire clear startup config */
  ret = lock_obj.GetLock(51, TC_ACQUIRE_WRITE_SESSION,TC_CLEAR_STARTUP_CONFIG);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  /* Write lock acquire  abort candidate config */
  ret = lock_obj.GetLock(51, TC_ACQUIRE_WRITE_SESSION,TC_ABORT_CANDIDATE_CONFIG);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  /* Release read lock */

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.ReleaseLock(99999, config_id,TC_RELEASE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_NOT_ACQUIRED, ret);

  lock_obj.tc_config_name_map_.clear();
  lock_obj.GetLock(1, TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  ret = lock_obj.ReleaseLock(1, config_id,TC_RELEASE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.ReleaseLock(51, config_id,TC_RELEASE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.ReleaseLock(1, config_id,TC_RELEASE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_NOT_ACQUIRED, ret);

  ret = lock_obj.GetLock(555, TC_ACQUIRE_WRITE_SESSION,TC_AUDIT_USER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  ret = lock_obj.GetLock(51, TC_ACQUIRE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.ReleaseLock(555, config_id,TC_RELEASE_WRITE_SESSION,TC_AUDIT_USER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  cout<<"Thread 1 Done"<<endl; 
  return 0;
}

void *thread_function_2(void *arg)
{
  sleep(2);
  cout<<"Executing  Thread 2"<<endl; 
  lock_obj.TcUpdateUncState(TC_ACT);
  state =lock_obj.GetUncCurrentState();
  EXPECT_EQ(TC_ACT,state);

  op = lock_obj.GetSessionOperation(0);
  EXPECT_EQ(TC_NO_OPERATION_PROGRESS,op);

  session_id = 10000; 
  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_NOT_ACQUIRED, ret);

  /* commit/abort candidate  before config acquire */
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, ret);
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_ABORT_CANDIDATE_CONFIG);
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, ret);

  lock_obj.tc_config_name_map_.clear();
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  /* commit/abort candidate  before config notify */
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_ABORT_CANDIDATE_CONFIG);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  /* Notify done */
  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  /* commit/candidate config  for session id not match with config session id*/
  ret = lock_obj.GetLock(session_id+1, TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, ret);
  ret = lock_obj.GetLock(session_id+1, TC_ACQUIRE_WRITE_SESSION,TC_ABORT_CANDIDATE_CONFIG);
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, ret);

  /* Write lock acquire commit */
  lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_ABORT_CANDIDATE_CONFIG);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  /* session ID other than config session id*/
  ret = lock_obj.ReleaseLock(777,config_id,TC_RELEASE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_NOT_ACQUIRED, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  /* Release write for other operation */
  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_NOT_ACQUIRED, ret);
  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
 
  /* Notify done - is success or failure */
  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  /* Write lock acquire  abort candidate config */
  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_ABORT_CANDIDATE_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_ABORT_CANDIDATE_CONFIG);
  EXPECT_EQ(TC_LOCK_BUSY, ret);


  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_INVALID_OPERATION, ret);

  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_WRITE_SESSION,TC_ABORT_CANDIDATE_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  /* Release config lock */
  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_RELEASE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  /* Write lock acquire audit user */
  session_id =999;
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_AUDIT_USER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_WRITE_SESSION,TC_AUDIT_USER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  /* Write lock acquire save startup config */
  session_id = 777;
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_SAVE_STARTUP_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  //session_id = write_lock.session_id;
  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_WRITE_SESSION,TC_SAVE_STARTUP_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  /* Write lock acquire clear startup config */
  session_id = 888;
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_CLEAR_STARTUP_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_WRITE_SESSION,TC_CLEAR_STARTUP_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  // auto save
  ret = lock_obj.GetLock(session_id, TC_AUTO_SAVE_ENABLE,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetLock(session_id, TC_AUTO_SAVE_ENABLE,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);
  
  ret = lock_obj.ReleaseLock(session_id,config_id, TC_AUTO_SAVE_ENABLE,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.ReleaseLock(session_id,config_id, TC_AUTO_SAVE_DISABLE,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_FAILURE, ret);

  /* state transition read progrss before move to SBY */
  bol_ret=lock_obj.IsStateTransitionInProgress();  
  EXPECT_EQ(PFC_FALSE,bol_ret);


  ret = lock_obj.ReleaseLock(23, config_id,TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_NOT_ACQUIRED, ret);

  lock_obj.TcSetSetupComplete(PFC_TRUE); 
  ret = lock_obj.GetLock(23, TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  lock_obj.ResetTcGlobalDataOnStateTransition();

  ret = lock_obj.GetLock(16, TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_OPERATION_NOT_ALLOWED, ret);

  ret = lock_obj.GetLock(16, TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_OPERATION_NOT_ALLOWED, ret);

  ret = lock_obj.ReleaseLock(23, config_id,TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  lock_obj.TcUpdateUncState(TC_SBY);
  ret = lock_obj.GetLock(6, TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_INVALID_UNC_STATE, ret);

  ret = lock_obj.ReleaseLock(1, config_id,TC_RELEASE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_NOT_ACQUIRED, ret);

  ret = lock_obj.GetLock(2, TC_ACQUIRE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.ReleaseLock(2, config_id,TC_RELEASE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  lock_obj.TcUpdateUncState(TC_ACT);

  session_id =100;
  lock_obj.TcSetSetupComplete(PFC_TRUE);
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  ret = lock_obj.ReleaseLock(session_id,999,TC_RELEASE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);


  ret = lock_obj.GetConfigData(1110, config_id, config_mode, vtn_name);
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID,ret);

  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE, TC_CONFIG_GLOBAL, "");
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  EXPECT_EQ(TC_LOCK_SUCCESS,ret);

  /* config notify pending */
  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_BUSY, ret);

  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_NONE);
  EXPECT_EQ(TC_LOCK_INVALID_OPERATION, ret);

  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_RELEASE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  config_id=session_id =100;

  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_AUDIT_USER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_WRITE_AUDIT_USER_PROGRESS,op);
  ret = lock_obj.ReleaseLock(session_id,999,TC_RELEASE_WRITE_SESSION,TC_AUDIT_USER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  session_id =1091;
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_WRITE_AUDIT_DRIVER_PROGRESS,op);
  ret = lock_obj.ReleaseLock(session_id,999,TC_RELEASE_WRITE_SESSION,TC_AUDIT_DRIVER);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  session_id =100021;
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_CLEAR_STARTUP_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_WRITE_CLEAR_STARTUP_CONFIG_PROGRESS,op);
  ret = lock_obj.ReleaseLock(session_id,999,TC_RELEASE_WRITE_SESSION,TC_CLEAR_STARTUP_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  session_id =100091;
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_SAVE_STARTUP_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_WRITE_SAVE_STARTUP_CONFIG_PROGRESS,op);
  ret = lock_obj.ReleaseLock(session_id,999,TC_RELEASE_WRITE_SESSION,TC_SAVE_STARTUP_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  session_id = 1111;
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_INVALID_SESSION_ID, ret);
  /* Acquire config session */
  session_id =100092;
  ret = lock_obj.GetLock(session_id, TC_FORCE_ACQUIRE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_CONFIG_NOTIFY_ACQUIRE_PROGRESS,op);
  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_ACQUIRE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);


  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);


  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_CONFIG_COMMIT_PROGRESS,op);

  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_WRITE_SESSION,TC_COMMIT);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);


  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_WRITE_SESSION,TC_ABORT_CANDIDATE_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_WRITE_ABORT_CANDIDATE_CONFIG_PROGRESS,op);
  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_WRITE_SESSION,TC_ABORT_CANDIDATE_CONFIG);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  ret = lock_obj.GetConfigData(session_id, config_id, config_mode, vtn_name);
  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_CONFIG_NO_NOTIFY_PROGRESS,op);

  ret = lock_obj.ReleaseLock(session_id,config_id,TC_RELEASE_CONFIG_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_CONFIG_NOTIFY_RELEASE_PROGRESS,op);

  ret = lock_obj.NotifyConfigIdSessionIdDone(config_id,session_id,TC_NOTIFY_RELEASE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);
  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_NO_OPERATION_PROGRESS,op);

  session_id =999;
  ret = lock_obj.GetLock(session_id, TC_ACQUIRE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  op = lock_obj.GetSessionOperation(session_id);
  EXPECT_EQ(TC_READ_PROGRESS,op);
  ret = lock_obj.ReleaseLock(session_id, 0,TC_RELEASE_READ_SESSION,TC_WRITE_NONE);
  EXPECT_EQ(TC_LOCK_SUCCESS, ret);

  cout<<" Thread 2 Done"<<endl; 
  return 0;
}

TEST (Test_TcLock, tc_lock_class)
{
#if 0
  pthread_t mythread_2, mythread_3, mythread_4, mythread_1;
  if ( pthread_create( &mythread_1, NULL, thread_function_1, NULL) ) {
    cout<<"error creating thread 1";
  }
  if ( pthread_create( &mythread_2, NULL, thread_function_2, NULL) ) {
    cout<<"error creating thread  2";
  }
  if ( pthread_create( &mythread_3, NULL, thread_function_3, NULL) ) {
    cout<<"error creating thread  3";
  }
  if ( pthread_create( &mythread_4, NULL, thread_function_4, NULL) ) {
    cout<<"error creating thread  4";
  }

  if ( pthread_join ( mythread_1, NULL ) ) {
    cout <<"error joining thread ";
  }

  if ( pthread_join ( mythread_2, NULL ) ) {
    cout <<"error joining thread ";
  }
  if ( pthread_join ( mythread_3, NULL ) ) {
    cout <<"error joining thread ";
  }
  if ( pthread_join ( mythread_4, NULL ) ) {
    cout <<"error joining thread ";
  }
#endif
}

} //   tc
} //   unc

class TestEnvironment : public ::testing::Environment {
 protected:
  virtual void SetUp() {
    pfc_log_init("gtest", stdout, PFC_LOGLVL_VERBOSE, NULL);
    libpfc_init();
  }
  virtual void TearDown() {
    pfc_log_fini();
    libpfc_fini();
  }
};


int main(int argc, char **argv) {
  std::cout << "Running main() from gtest_main.cc\n";
  testing::InitGoogleTest(&argc, argv);
  AddGlobalTestEnvironment(new TestEnvironment());
  return RUN_ALL_TESTS();
}


#endif //_UNC_TCLOCK_GTEST_MAIN_HH_
