/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <limits.h>
#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <vtn_drv_transaction_handle.hh>
#include <controller_utils.hh>
#include <vtn_cache_mod.hh>
#include <stdio.h>
#include <map>
#include <unc/upll_ipc_enum.h>
#include "../../../stub/tclib_module/tclib_interface.hh"
#include "../../../stub/tclib_module/tclib_module.hh"
#include "../../../stub/ContrllerFrameworkStub/driver/driver_interface.hh"
#include "../../../stub/ContrllerFrameworkStub/vtn_drv_module.hh"
#include "../../../../../modules/vtndrvintf/include/request_template.hh"


namespace unc {
namespace driver {
class DriverTxnInterfaceTest: public testing::Test {
};


TEST_F(DriverTxnInterfaceTest, HandleCommitGlobalCommitSuccess) {
  const pfc_modattr_t* attr = NULL;
  VtnDrvIntf obj(attr);
  obj.init();
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  std::string ctr_name = "ctr_demo";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  typedef std::list<std::string> ::iterator ctr_iter;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitGlobalCommit
            (session_id, config_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitGlobalCommitFailure) {
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  std::string ctr_name = "ctr_demo";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  unc::driver::driver::set_ctrl_instance(1);
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  typedef std::list<std::string> ::iterator ctr_iter;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  ret_code = TxnObj->HandleCommitGlobalCommit
      (session_id, config_id, controller);
  EXPECT_EQ(ret_code, unc::tclib::TC_SUCCESS);
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitGlobalCommitConDown) {
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  std::string ctr_name = "ctr_demo";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  typedef std::list<std::string> ::iterator ctr_iter;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller::set_controller_status(1);
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitGlobalCommit
            (session_id, config_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitGlobalCommitFailure2) {
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  std::string ctr_name = "ctr_demo";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  typedef std::list<std::string> ::iterator ctr_iter;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::driver::driver::set_ctrl_instance(0);
  unc::driver::driver::set_ret_code(1);
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitGlobalCommit
            (session_id, config_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitGlobalCommitDrvFailure) {
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  std::string ctr_name = "ctr_demo";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  typedef std::list<std::string> ::iterator ctr_iter;
  kt_handler_map map_kt_;
  unc::driver::driver::set_result = 1;
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitGlobalCommit
            (session_id, config_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleAuditVoteRequestSuccess) {
  uint32_t session_id = 1;
  std::string ctr_id = "ctr_name";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleAuditVoteRequest
            (session_id, ctr_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleAuditVoteRequestFailure) {
  uint32_t session_id = 1;
  std::string ctr_id = "ctr_name";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller::set_controller_status(0);
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_FAILURE;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleAuditVoteRequest
            (session_id, ctr_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}


TEST_F(DriverTxnInterfaceTest, HandleAuditGlobalCommitSuccess) {
  uint32_t session_id = 1;
  std::string ctr_id = "ctr_name";
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_FAILURE;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleAuditGlobalCommit
            (session_id, ctr_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleAuditGlobalCommitFailure) {
  uint32_t session_id = 1;
  std::string ctr_id = "ctr_name";
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::driver::driver::set_ctrl_instance(0);
  unc::driver::driver::set_ret_code(0);
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleAuditGlobalCommit
            (session_id, ctr_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitVoteRequestSuccess) {
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  std::string ctr_name = "ctr_name";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitVoteRequest
            (session_id, config_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitVoteRequest2PhTrue) {
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  std::string ctr_name = "ctr_name";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  typedef std::list<std::string> ::iterator ctr_iter;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::driver::driver::set_ctrl_instance(1);
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitVoteRequest
            (session_id, config_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitVoteRequestDown) {
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  std::string ctr_name = "ctr_name";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  controller::set_controller_status(1);
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  typedef std::list<std::string> ::iterator ctr_iter;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  controller::set_controller_status(1);
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitVoteRequest
            (session_id, config_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}


TEST_F(DriverTxnInterfaceTest, HandleCommitVoteRequestFailure) {
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  controller *ctrl_ptr = NULL;
  driver *drv =   NULL;
  std::string ctr_name = "ctr_name";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  CtrObj->GetDriverByControllerName(ctr_name, &ctrl_ptr, &drv);
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  ctrl_ptr->controller_cache = unc::vtndrvcache::KeyTree::create_cache();
  unc::driver::driver::set_ctrl_instance(1);
  controller::set_controller_status(0);
  controller.push_back("ctr_demo");
  // Recent Import Fix will make it return success
  // even if the controller is down.
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitVoteRequest
            (session_id, config_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitVoteCacheSuccess) {
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  std::string ctr_name = "ctr_name";
  controller *ctrl_ptr = NULL;
  driver *drv =   NULL;
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  CtrObj->GetDriverByControllerName(ctr_name, &ctrl_ptr, &drv);

  uint32_t operation = 1;
  ctrl_ptr->controller_cache = unc::vtndrvcache::KeyTree::create_cache();
  key_vtn key_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  val_vtn val_obj;
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));
  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vtn, val_vtn,
            val_vtn, uint32_t>(&key_obj, &val_obj, &val_obj, operation);
  uint32_t ret =  ctrl_ptr->controller_cache->append_commit_node(cfgptr);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
  typedef std::map <unc_key_type_t, unc::driver::KtHandler*> kt_handler_map;

  kt_handler_map map_kt_;
  KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(NULL);

  map_kt_.insert(std::pair<unc_key_type_t,
                 unc::driver::KtHandler*>(UNC_KT_VTN, vtn_req));
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitVoteRequest
                                    (session_id, config_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete vtn_req;
  delete ctrl_ptr->controller_cache;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitCacheFailure) {
  std::string ctr_name = "ctr_name";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  driver *drv = driver::create_driver();
  controller *ctr = controller::create_controll();
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_FAILURE;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitCache(ctr_name, ctr, drv));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitCacheSuccess) {
  std::string ctr_name = "ctr_name";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  controller *ctrl_ptr = NULL;
  driver *drv =   NULL;
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  CtrObj->GetDriverByControllerName(ctr_name, &ctrl_ptr, &drv);

  uint32_t operation = 1;
  ctrl_ptr->controller_cache = unc::vtndrvcache::KeyTree::create_cache();
  key_vtn key_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  val_vtn val_obj;
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));
  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vtn, val_vtn, val_vtn,
        uint32_t>(&key_obj, &val_obj, &val_obj, operation);
  uint32_t ret =  ctrl_ptr->controller_cache->append_commit_node(cfgptr);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
  typedef std::map <unc_key_type_t, unc::driver::KtHandler*> kt_handler_map;

  kt_handler_map map_kt_;
  KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(NULL);

  map_kt_.insert(std::pair<unc_key_type_t,
                 unc::driver::KtHandler*>(UNC_KT_VTN, vtn_req));
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitCache(ctr_name, ctrl_ptr, drv));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete vtn_req;
  delete ctrl_ptr->controller_cache;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleCommitCacheCmdFailure) {
  std::string ctr_name = "ctr_name";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  controller *ctrl_ptr = NULL;
  driver *drv =   NULL;
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  CtrObj->GetDriverByControllerName(ctr_name, &ctrl_ptr, &drv);

  uint32_t operation = 4;
  ctrl_ptr->controller_cache = unc::vtndrvcache::KeyTree::create_cache();
  key_vtn key_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  val_vtn val_obj;
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));
  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vtn, val_vtn, val_vtn,
              uint32_t>(&key_obj, &val_obj, &val_obj, operation);
  uint32_t ret =  ctrl_ptr->controller_cache->append_commit_node(cfgptr);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
  typedef std::map <unc_key_type_t, unc::driver::KtHandler*> kt_handler_map;

  kt_handler_map map_kt_;
  KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(NULL);

  map_kt_.insert(std::pair<unc_key_type_t,
                 unc::driver::KtHandler*>(UNC_KT_VTN, vtn_req));
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_FAILURE;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleCommitCache(ctr_name, ctrl_ptr, drv));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete vtn_req;
  delete ctrl_ptr->controller_cache;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleAuditVoteRequest_ControllerNameNotExist) {
  uint32_t session_id = 1;
  std::string ctr_id = "ctr_name";
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  ControllerFramework::res_code = 1;
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  kt_handler_map map_kt_;
  unc::tclib::TcControllerList controller;
  controller::set_controller_status(0);
  controller.push_back("ctrl_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleAuditVoteRequest
            (session_id, ctr_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleAuditGlobalCommit_ControllerNameNotExist) {
  uint32_t session_id = 1;
  std::string ctr_id = "ctr_name";
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  kt_handler_map map_kt_;
  ControllerFramework::res_code = 1;
  unc::tclib::TcControllerList controller;
  controller.push_back("ctr_demo");
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleAuditGlobalCommit
            (session_id, ctr_id, controller));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}

TEST_F(DriverTxnInterfaceTest, HandleAuditEND_ControllerNameNotExist) {
  uint32_t session_id = 1;
  std::string ctr_id = "ctr_name";
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::driver::ControllerFramework* CtrObj =
      new unc::driver::ControllerFramework;
  kt_handler_map map_kt_;
  unc_keytype_ctrtype_t ctr_type = UNC_CT_ODC;
  ControllerFramework::res_code = 1;
  unc::tclib::TcAuditResult audit_result = unc::tclib::TC_AUDIT_SUCCESS;
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  DriverTxnInterface *TxnObj = new DriverTxnInterface(CtrObj, map_kt_);
  EXPECT_EQ(ret_code, TxnObj->HandleAuditEnd
            (session_id, ctr_type, ctr_id, audit_result));
  unc::tclib::TcLibModule::stub_unloadtcLibModule();
  delete TxnObj;
  delete CtrObj;
  CtrObj = NULL;
  TxnObj = NULL;
}
}  // namespace driver
}  // namespace unc
