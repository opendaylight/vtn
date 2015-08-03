/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcconfigoperations.hh"
#include <uncxx/tc/libtc_common.hh>

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_int32;
extern int stub_srv_string;
extern int stub_opertype;
extern int arg_count;
extern int stub_get_arg;
extern int stub_get_arg1;

TEST(TcConfigOperations, TcGetMinArgCount) {
  SET_AUDIT_OPER_PARAMS();

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE;
  int argcount  =  tc_configoperations.TestTcGetMinArgCount();
  EXPECT_EQ(2, argcount);
}

TEST(TcConfigOperations, TcCheckOperArgCount_INVALID) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_INVALID;
  uint32_t avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_configoperations.TcCheckOperArgCount(avail_count));
}

TEST(TcConfigOperations, TcCheckOperArgCount_ACQUIRE_ERR) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE;
  int avail_count = 5;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
  tc_configoperations.TcCheckOperArgCount(avail_count));
}
TEST(TcConfigOperations, TcCheckOperArgCount_ACQUIRE) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE;
  int avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_configoperations.TcCheckOperArgCount(avail_count));
}

TEST(TcConfigOperations, TcCheckOperArgCount_RELEASE_ERR) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  int avail_count = 2;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, 
  tc_configoperations.TcCheckOperArgCount(avail_count));

}

TEST(TcConfigOperations, TcCheckOperArgCount_RELEASE) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  int avail_count = 3;
  EXPECT_EQ(TC_OPER_SUCCESS, 
  tc_configoperations.TcCheckOperArgCount(avail_count));

}

TEST(TcConfigOperations, TcCheckOperArgCount_TIMED_ERR) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_TIMED;
  int avail_count = 5;
  EXPECT_EQ(TC_OPER_INVALID_INPUT,
  tc_configoperations.TcCheckOperArgCount(avail_count));

}

TEST(TcConfigOperations, TcCheckOperArgCount_TIMED) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_TIMED;
  int avail_count = 3;
  EXPECT_EQ(TC_OPER_SUCCESS, 
  tc_configoperations.TcCheckOperArgCount(avail_count));

}

TEST(TcConfigOperations, TcCheckOperArgCount_PARTIAL_ERR) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_PARTIAL;
  int avail_count = 2;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, 
  tc_configoperations.TcCheckOperArgCount(avail_count));

}

TEST(TcConfigOperations, TcCheckOperArgCount_PARTIAL) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_PARTIAL;
  int avail_count = 4;
  EXPECT_EQ(TC_OPER_SUCCESS, 
  tc_configoperations.TcCheckOperArgCount(avail_count));

}

TEST(TcConfigOperations, TcCheckOperArgCount_FORCE_ERR) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);


  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_FORCE;
  int avail_count = 3;
  EXPECT_EQ(TC_OPER_INVALID_INPUT, 
  tc_configoperations.TcCheckOperArgCount(avail_count));

}


TEST(TcConfigOperations, TcCheckOperArgCount_FORCE) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);


  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_FORCE;
  int avail_count = 2;
  EXPECT_EQ(TC_OPER_SUCCESS, 
  tc_configoperations.TcCheckOperArgCount(avail_count));

}

TEST(TcConfigOperations, TcValidateOperType_ERR_1) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_RUNNING_SAVE;
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE, tc_configoperations.TcValidateOperType());
  
}
TEST(TcConfigOperations, TcValidateOperType_ERR_2) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CLEAR_STARTUP;
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE, tc_configoperations.TcValidateOperType());
  
}

TEST(TcConfigOperations, TcValidateOperType_SUCCESS) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcValidateOperType());
  
}

TEST(TcConfigOperations, HandleMsgRet_Fatal) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_FATAL;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  // check code change define value
  EXPECT_EQ(TC_SYSTEM_FAILURE, tc_configoperations.HandleMsgRet(msgret));
}

TEST(TcConfigOperations, HandleMsgRet_Abort) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_ABORT;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_ABORT, tc_configoperations.HandleMsgRet(msgret));
}

TEST(TcConfigOperations, HandleMsgRet_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperRet msgret  =  TCOPER_RET_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.HandleMsgRet(msgret));
}

TEST(TcConfigOperations, TcValidateOperParams_FORCE_CONFIG_SUCCESS) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_FORCE;
  stub_srv_uint32  =  0;   //get_uint32 returns TCUTIL_RET_FAILURE
  EXPECT_EQ(TC_OPER_SUCCESS,tc_configoperations.TcValidateOperParams());
}

TEST(TcConfigOperations, TcValidateOperParams_CONFIG_RELEASE_Failure) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  stub_srv_uint32  =  0;   //get_uint32 returns TCUTIL_RET_FAILURE
  EXPECT_EQ(TC_OPER_INVALID_INPUT,tc_configoperations.TcValidateOperParams());
}


TEST(TcConfigOperations, TcValidateOperParams_CONFIG_RELEASE_Fatal) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;

  stub_srv_uint32  =  -1;  //get_uint32 returns TCUTIL_RET_FATAL
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.TcValidateOperParams());
}

TEST(TcConfigOperations, TcValidateOperParams_CONFIG_RELEASE_Invalid) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;

  stub_srv_uint32  =  2;
  EXPECT_EQ(TC_INVALID_STATE, tc_configoperations.TcValidateOperParams());
}

TEST(TcConfigOperations, TcValidateOperParams_TIMED_Failure) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_TIMED;
  stub_srv_int32  =  0;   //get_int32 returns TCUTIL_RET_FAILURE
  EXPECT_EQ(TC_OPER_INVALID_INPUT,tc_configoperations.TcValidateOperParams());

}

TEST(TcConfigOperations, TcValidateOperParams_TIMED_Fatal) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_TIMED;
  
  stub_srv_int32  =  -1;  //get_int32 returns TCUTIL_RET_FATAL
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.TcValidateOperParams());

}


TEST(TcConfigOperations, TcValidateOperParams_TIMED_SUCCESS) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_TIMED;

  stub_srv_int32  =  2; 
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcValidateOperParams());
}

TEST(TcConfigOperations, TcValidateOperParams_PARTIAL_Failure) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_PARTIAL;

  stub_srv_uint8  =  0;   //get_uint8 returns TCUTIL_RET_FAILURE
  EXPECT_EQ(TC_OPER_INVALID_INPUT,tc_configoperations.TcValidateOperParams());

}

TEST(TcConfigOperations, TcValidateOperParams_PARTIAL_Fatal) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE_PARTIAL;


  stub_srv_uint32 =  -1;  //get_uint8 returns TCUTIL_RET_FATAL
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.TcValidateOperParams());
}

TEST(TcConfigOperations, TcValidateOperParams_PARTIAL_SUCCESS) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_INVALID;

  stub_srv_uint8  =  2;
  stub_srv_string =  2;

  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcValidateOperParams());
}

TEST(TcConfigOperations, TcGetExclusion_RELEASE_1) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  EXPECT_EQ(TC_CONFIG_NOT_PRESENT, tc_configoperations.TcGetExclusion());
}

TEST(TcConfigOperations, TcGetExclusion_RELEASE_2) {
  
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  tc_configoperations.session_id_ = 1000;
  std::string tc_config_name      = "global-mode";
  
  (tc_configoperations).tclock_->UpdateConfigData(tc_configoperations.session_id_,
                                                      tc_config_name);
  tc_configoperations.config_id_ = (tc_configoperations).tclock_->prev_config_id_;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_configoperations.TcGetExclusion());
  tc_configoperations.tclock_-> tc_config_name_map_.clear();
}

TEST(TcConfigOperations, TcGetExclusion_RELEASE_3) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  tc_configoperations.session_id_ = 1000;
  std::string tc_config_name      = "global-mode";
  
  (tc_configoperations).tclock_->
              UpdateConfigData(tc_configoperations.session_id_, tc_config_name);
  tc_configoperations.config_id_ = 
              (tc_configoperations).tclock_->prev_config_id_; 
  tc_configoperations.tclock_->
              tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcGetExclusion());
  tc_configoperations.tclock_-> tc_config_name_map_.clear();
}

TEST(TcConfigOperations, TcGetExclusion_RELEASE_4) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  tc_configoperations.session_id_ = 1000;
  std::string tc_config_name      = "global-mode";

  (tc_configoperations).tclock_->
                               UpdateConfigData(tc_configoperations.session_id_,
                                                      tc_config_name);
  tc_configoperations.config_id_ =
                               (tc_configoperations).tclock_->prev_config_id_;

  tc_configoperations.tclock_->
                       tc_config_name_map_[tc_config_name].is_taken = PFC_FALSE;

  EXPECT_EQ(TC_CONFIG_NOT_PRESENT, tc_configoperations.TcGetExclusion());
  tc_configoperations.tclock_-> tc_config_name_map_.clear();
}

TEST(TcConfigOperations, TcGetExclusion_ACQUIRE_INVALID_STATE_1) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE;
  EXPECT_EQ(TC_INVALID_STATE, tc_configoperations.TcGetExclusion());
}

TEST(TcConfigOperations, TcGetExclusion_ACQUIRE_INVALID_OPERATION_TYPE) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_USER_AUDIT;
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE, tc_configoperations.TcGetExclusion());
}

TEST(TcConfigOperations, TcGetExclusion_ACQUIRE_INVALID_STATE) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.session_id_ = 1000;
  std::string tc_config_name      = "global-mode";

  (tc_configoperations).tclock_->
                    UpdateConfigData(tc_configoperations.session_id_,
                                                      tc_config_name);

  tc_configoperations.config_id_ =
                       (tc_configoperations).tclock_->prev_config_id_;

  tc_configoperations.tclock_->
              tc_config_name_map_[tc_config_name].is_taken = PFC_FALSE;


  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE;
  tc_configoperations.tc_mode_  =  TC_CONFIG_VIRTUAL;
  tc_configoperations.tclock_->tc_state_lock_.current_state = TC_ACT;
  tc_configoperations.tclock_->tc_state_lock_.state_transition_in_progress =  
                                                                     PFC_TRUE;
  EXPECT_EQ(TC_INVALID_STATE, tc_configoperations.TcGetExclusion());
  tc_configoperations.tclock_-> tc_config_name_map_.clear();
}

TEST(TcConfigOperations, TcGetExclusion_ACQUIRE_SUCCESS) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.session_id_ = 1000;
  std::string tc_config_name      = "global-mode";

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE;
  tc_configoperations.tc_mode_  =  TC_CONFIG_VIRTUAL;  
  tc_configoperations.tclock_->tc_state_lock_.current_state = TC_ACT;
  tc_configoperations.tclock_->tc_state_lock_.state_transition_in_progress =
                                                                      PFC_FALSE;
  tc_configoperations.tclock_->prev_config_id_ = 0;
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcGetExclusion());
}

TEST(TcConfigOperations, TcGetExclusion_ACQUIRE_4) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  tc_configoperations.session_id_ = 888;
  std::string tc_config_name      = "global-mode";

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_ACQUIRE;
  tc_configoperations.tc_mode_  =  TC_CONFIG_GLOBAL;
  tc_configoperations.tclock_->tc_state_lock_.current_state = TC_ACT;
  tc_configoperations.tclock_->tc_state_lock_.state_transition_in_progress =
                                                                      PFC_FALSE;
  tc_configoperations.tclock_->prev_config_id_ = 0;
  tc_configoperations.tclock_->
                    UpdateConfigData(tc_configoperations.session_id_, 
                                                    tc_config_name);
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_configoperations.TcGetExclusion());
  tc_configoperations.tclock_->tc_config_name_map_.clear();
}


TEST(TcConfigOperations, TcReleaseExclusion_1) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                         &sess_,
                         db_handler,
                         unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_USER_AUDIT;

  EXPECT_EQ(::TC_INVALID_SESSION_ID, tc_configoperations.TcReleaseExclusion());

}

TEST(TcConfigOperations, TcReleaseExclusion_2) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                         &sess_,
                         db_handler,
                         unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_USER_AUDIT;
  tc_configoperations.session_id_ = 777;
  std::string tc_config_name      = "global-mode";

  (tc_configoperations).tclock_->
                    UpdateConfigData(tc_configoperations.session_id_,
                                                      tc_config_name);
  tc_configoperations.config_id_ = 11;  

  EXPECT_EQ(::TC_INVALID_CONFIG_ID, tc_configoperations.TcReleaseExclusion());

}

TEST(TcConfigOperations, TcReleaseExclusion_3) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                         &sess_,
                         db_handler,
                         unc_map_);
  tc_configoperations.tc_oper_  =  TC_OP_USER_AUDIT;
  tc_configoperations.session_id_ = 777;
  std::string tc_config_name      = "global-mode";
  (tc_configoperations).tclock_->prev_config_id_ = 0;

  (tc_configoperations).tclock_->
                    UpdateConfigData(tc_configoperations.session_id_,
                                                      tc_config_name);
  tc_configoperations.config_id_ = 
                    (tc_configoperations).tclock_->prev_config_id_;

  (tc_configoperations).tclock_->
  tc_config_name_map_[tc_config_name].notify_operation = TC_NOTIFY_NONE;

  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.TcReleaseExclusion());
}

TEST(TcConfigOperations, TcReleaseExclusion_4) {
  SET_AUDIT_OPER_PARAMS()

  TestTcConfigOperations tc_configoperations(tc_lock_,
                         &sess_,
                         db_handler,
                         unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_USER_AUDIT;
  tc_configoperations.session_id_ = 777;
  std::string tc_config_name      = "global-mode";
  (tc_configoperations).tclock_->prev_config_id_ = 0;

  (tc_configoperations).tclock_->
                    UpdateConfigData(tc_configoperations.session_id_,
                                                      tc_config_name);
  tc_configoperations.config_id_ =
                    (tc_configoperations).tclock_->prev_config_id_;

  (tc_configoperations).tclock_->
  tc_config_name_map_[tc_config_name].notify_operation = TC_NOTIFY_NONE;
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.TcReleaseExclusion());
}

TEST(TcConfigOperations, TcReleaseExclusion_5) {
  SET_AUDIT_OPER_PARAMS();
  TestTcConfigOperations tc_configoperations(tc_lock_,
                         &sess_,
                         db_handler,
                         unc_map_);

  tc_configoperations.tc_oper_  =  TC_OP_CONFIG_RELEASE;
  tc_configoperations.session_id_ = 777;
  std::string tc_config_name      = "global-mode";
  (tc_configoperations).tclock_->prev_config_id_ = 0;

  (tc_configoperations).tclock_->
                    UpdateConfigData(tc_configoperations.session_id_,
                                                      tc_config_name);
  tc_configoperations.config_id_ =
                    (tc_configoperations).tclock_->prev_config_id_;

  (tc_configoperations).tclock_->
  tc_config_name_map_[tc_config_name].notify_operation = TC_NOTIFY_RELEASE;

  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcReleaseExclusion());
}

TEST(TcConfigOperations, HandleLockRet_INVALID_UNC_STATE) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret  =  TC_LOCK_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  ret  =  TC_LOCK_INVALID_UNC_STATE;
  EXPECT_EQ(TC_INVALID_STATE, tc_configoperations.HandleLockRet(ret));

}

TEST(TcConfigOperations, HandleLockRet_OPERATION_NOT_ALLOWED) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret  =  TC_LOCK_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  ret  =  TC_LOCK_OPERATION_NOT_ALLOWED;
  EXPECT_EQ(TC_INVALID_STATE, tc_configoperations.HandleLockRet(ret));
}

TEST(TcConfigOperations, HandleLockRet_TC_LOCK_BUSY) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret  =  TC_LOCK_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  ret  =  TC_LOCK_BUSY;
  EXPECT_EQ(TC_SYSTEM_BUSY, tc_configoperations.HandleLockRet(ret));
}


TEST(TcConfigOperations, HandleLockRet_NO_CONFIG_SESSION_) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret  =  TC_LOCK_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  ret  =  TC_LOCK_NO_CONFIG_SESSION_EXIST;
  EXPECT_EQ(TC_CONFIG_NOT_PRESENT, tc_configoperations.HandleLockRet(ret));
}

TEST(TcConfigOperations, HandleLockRet_INVALID_PARAMS) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret  =  TC_LOCK_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  ret  =  TC_LOCK_INVALID_PARAMS;
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.HandleLockRet(ret));

}

TEST(TcConfigOperations, HandleLockRet_INVALID_SESSION_ID) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret  =  TC_LOCK_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  ret  =  TC_LOCK_INVALID_SESSION_ID;
  EXPECT_EQ(105, tc_configoperations.HandleLockRet(ret));
}

TEST(TcConfigOperations, HandleLockRet_INVALID_CONFIG_ID) {
  SET_AUDIT_OPER_PARAMS();
  TcLockRet ret  =  TC_LOCK_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);

  ret  =  TC_LOCK_INVALID_CONFIG_ID;
  EXPECT_EQ(103, tc_configoperations.HandleLockRet(ret));
}


TEST(TcConfigOperations, TcCreateMsgList) {
  SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS, tc_configoperations.TcCreateMsgList());
}

TEST(TcConfigOperations, FillTcMsgData) {
  SET_AUDIT_OPER_PARAMS();
  TcMsg* tc_msg  =  NULL;
  stub_srv_uint32  =  1;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.FillTcMsgData(tc_msg, MSG_SAVE_CONFIG));
}

TEST(TcConfigOperations, SendAdditionalResponse_Success) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_SUCCESS,
  tc_configoperations.SendAdditionalResponse(oper_stat));
}

TEST(TcConfigOperations, SendAdditionalResponse_Failure) {
  SET_AUDIT_OPER_PARAMS();
  TcOperStatus oper_stat = TC_OPER_SUCCESS;
  stub_srv_uint32  =  0;
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.SendAdditionalResponse(oper_stat));
}

TEST(TcConfigOperations, Execute) {
  SET_AUDIT_OPER_PARAMS();
  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  EXPECT_EQ(TC_OPER_FAILURE, tc_configoperations.Execute());
}

TEST(TcConfigOperations, HandleTimedConfigAcquisition_1){
  SET_AUDIT_OPER_PARAMS();
  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE_TIMED; 
  tc_configoperations.timeout_= 0;
  EXPECT_EQ(TC_INVALID_STATE,
  tc_configoperations.HandleTimedConfigAcquisition());
}

TEST(TcConfigOperations, HandleTimedConfigAcquisition_2){
  SET_AUDIT_OPER_PARAMS();
  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.timeout_= 0;
  tc_configoperations.config_req_queue_.clear();
  EXPECT_EQ(TC_INVALID_OPERATION_TYPE,
  tc_configoperations.HandleTimedConfigAcquisition());
}

TEST(TcConfigOperations, HandleTimedConfigAcquisition_3){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.timeout_= 1;
tc_configoperations.config_req_queue_.clear();
EXPECT_EQ(TC_SYSTEM_BUSY,
  tc_configoperations.HandleTimedConfigAcquisition());
}

//tc_oper_ is TC_OP_INVALID
TEST(TcConfigOperations, RevokeOperation_Busy){
SET_AUDIT_OPER_PARAMS();
TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_SYSTEM_BUSY);
EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.RevokeOperation(TC_SYSTEM_BUSY));
}

TEST(TcConfigOperations, RevokeOperation_CONFIG_ACQUIRE_test){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE;
tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_OP_CONFIG_ACQUIRE);
pfc::core::timer_func_t timer_func;
//pfc_timespec_t  timeout;
//timeout.tv_sec = 1200;
//timeout.tv_nsec = 0;
tc_configoperations.InsertConfigRequest();

EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.RevokeOperation(TC_SYSTEM_BUSY));
}



TEST(TcConfigOperations, RevokeOperation_CONFIG_ACQUIRE){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_OP_CONFIG_ACQUIRE);
tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE;

pfc::core::timer_func_t timer_func;
//pfc_timespec_t  timeout;
//timeout.tv_sec = 120;
//timeout.tv_nsec = tc_configoperations.timeout_ * 1000000;    // 1ms = 1000000ns
//timeout.tv_nsec = 0;
tc_configoperations.InsertConfigRequest();

EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.RevokeOperation(TC_SYSTEM_BUSY));
}

TEST(TcConfigOperations, RevokeOperation_CONFIG_ACQUIRE_TIMED){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
pfc::core::timer_func_t timer_func;
//pfc_timespec_t  timeout;
//timeout.tv_sec = 120;
//timeout.tv_nsec = 0;
tc_configoperations.InsertConfigRequest();
tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE_TIMED;
tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_OP_CONFIG_ACQUIRE_TIMED);

EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.RevokeOperation(TC_SYSTEM_BUSY));
}

TEST(TcConfigOperations, RevokeOperation_CONFIG_ACQUIRE_FORCE){
SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE_FORCE;
pfc::core::timer_func_t timer_func;
//pfc_timespec_t  timeout;
//timeout.tv_sec = 120;
//timeout.tv_nsec = tc_configoperations.timeout_ * 1000000;    // 1ms = 1000000ns
//timeout.tv_nsec = 0;
tc_configoperations.InsertConfigRequest();
tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_OP_CONFIG_ACQUIRE_FORCE);

EXPECT_EQ(TC_OPER_FAILURE,
  tc_configoperations.RevokeOperation(TC_SYSTEM_BUSY));
}

TEST(TcConfigOperations, RevokeOperation_CONFIG_CONFIG_RELEASE){
SET_AUDIT_OPER_PARAMS();
TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
tc_configoperations.tc_oper_ = TC_OP_CONFIG_RELEASE;
tc_configoperations.tc_oper_status_ = SEND_RESPONSE_PHASE;
//tc_configoperations.tc_oper_status_ = unc::tc::TcOperEnum(TC_OP_CONFIG_RELEASE);
pfc::core::timer_func_t timer_func;
//pfc_timespec_t  timeout;
//timeout.tv_sec = 120;
//timeout.tv_nsec = tc_configoperations.timeout_ * 1000000;    // 1ms = 1000000ns
//timeout.tv_nsec = 0;
tc_configoperations.InsertConfigRequest();
EXPECT_EQ(TC_OPER_FAILURE,
      tc_configoperations.RevokeOperation(TC_OPER_SUCCESS));
}

TEST(TcConfigOperations, Dispatch_InvalidInput){
    SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
    EXPECT_EQ(TC_OPER_INVALID_INPUT,tc_configoperations.Dispatch());

}

TEST(TcConfigOperations, Dispatch_HandleArgs_Failure){
    SET_AUDIT_OPER_PARAMS();
    TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
    EXPECT_EQ(TC_OPER_INVALID_INPUT,tc_configoperations.Dispatch());
}

/*-----------------------------------U17 CR--------------------------*/
TEST(TcConfigOperations, ValidateGlobalModeDirty_release) {
  SET_AUDIT_OPER_PARAMS();
  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_ = TC_OP_CONFIG_RELEASE;
  tc_configoperations.tc_mode_ = TC_CONFIG_GLOBAL;
  EXPECT_EQ(TC_OPER_SUCCESS,tc_configoperations.ValidateGlobalModeDirty());
}

TEST(TcConfigOperations, ValidateGlobalModeDirty_Acquire_Timed) {
  SET_AUDIT_OPER_PARAMS();
  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_ = TC_OP_CONFIG_ACQUIRE_TIMED;
  tc_configoperations.tc_mode_ = TC_CONFIG_GLOBAL;
  EXPECT_EQ(TC_OPER_SUCCESS,tc_configoperations.ValidateGlobalModeDirty());
}

TEST(TcConfigOperations, CandidateDirty_false) {
  SET_AUDIT_OPER_PARAMS();
  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_ = TC_OP_CONFIG_RELEASE;
  tc_configoperations.tc_mode_ = TC_CONFIG_GLOBAL;
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPPL,  "phynwd")));
  EXPECT_EQ(TC_OPER_SUCCESS,tc_configoperations.ValidateGlobalModeDirty());
}


TEST(TcOperations, ValidateGlobalModeDirty_IsCandidateDirty_true)
{
  SET_AUDIT_OPER_PARAMS();
  unc_map_.insert((std::pair<TcDaemonName, std::string>(TC_UPLL,  "lgcnwd")));
  stub_get_arg = 2;
  stub_get_arg1 = 3;
  TestTcConfigOperations tc_configoperations(tc_lock_,
                                             &sess_,
                                             db_handler,
                                             unc_map_);
  tc_configoperations.tc_oper_ = TC_OP_CONFIG_RELEASE;
  tc_configoperations.tc_mode_ = TC_CONFIG_VTN;

  tc_configoperations.session_id_ = 1000;
  std::string tc_config_name      = "global-mode";

  (tc_configoperations).tclock_->UpdateConfigData(tc_configoperations.session_id_,
                                                      tc_config_name);
  tc_configoperations.tclock_->tc_state_lock_.current_state = TC_ACT;
  tc_configoperations.tclock_->tc_state_lock_.state_transition_in_progress =
                                                                     PFC_TRUE;
  //tc_configoperations.config_id_ = (tc_configoperations).tclock_->prev_config_id_;
  EXPECT_EQ(TC_OPER_SUCCESS,tc_configoperations.ValidateGlobalModeDirty());
  tc_configoperations.tclock_-> tc_config_name_map_.clear();
  stub_get_arg1 =0;
}

