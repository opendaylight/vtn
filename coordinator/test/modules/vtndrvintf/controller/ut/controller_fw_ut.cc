/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <limits.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <odc_drv.hh>
#include <odc_controller.hh>
#include <controller_fw.hh>
#include <arpa/inet.h>

namespace unc {
namespace driver {

class ControllerFrameworkTest: public testing::Test {
 protected:
  virtual void SetUp() { }

  virtual void TearDown() {}
};


TEST_F(ControllerFrameworkTest, GetControllerInst_success) {
  std::string ctr_name = "ctr_demo";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  uint32_t ret_code = DRVAPI_RESPONSE_SUCCESS;
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  ret_code = CtrObj->GetControllerInstance(ctr_name, &ctr_instance,
                                           &drv_instance);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, ret_code);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetDriver_success) {
  std::string ctr_name = "ctr_demo";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  uint32_t ret_code = DRVAPI_RESPONSE_SUCCESS;
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  CtrObj->GetControllerInstance(ctr_name, &ctr_instance,
                                &drv_instance);
  ret_code = CtrObj->GetDriverByControllerName(ctr_name, &ctr_instance,
                                               &drv_instance);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, ret_code);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetDriver_failure) {
  std::string ctr_name = "ctr_demo";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  uint32_t ret_code = DRVAPI_RESPONSE_SUCCESS;
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  CtrObj->GetControllerInstance(ctr_name, &ctr_instance,
                                &drv_instance);
  driver *drv_instance_new = NULL;
  controller *ctr_instance_new = NULL;
  ret_code = CtrObj->GetDriverByControllerName(ctr_name, &ctr_instance_new,
                                               &drv_instance_new);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, ret_code);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
}

TEST_F(ControllerFrameworkTest, PingController_success) {
  std::string ctr_name = "ctr_post";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  ReadParams fun_obj(ctr_name, CtrObj);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  uint32_t ret_code = DRVAPI_RESPONSE_SUCCESS;
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  ret_code = CtrObj->GetDriverByControllerName(ctr_name, &ctr_instance,
                                               &drv_instance);
  fun_obj.PingController();
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, ret_code);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
}

TEST_F(ControllerFrameworkTest, PingController_Failure) {
  std::string ctr_name = "ctr_post";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  ReadParams fun_obj(ctr_name, CtrObj);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  uint32_t ret_code = DRVAPI_RESPONSE_SUCCESS;
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  driver *drv_instance1 = NULL;
  controller *ctr_instance1 = NULL;
  ret_code = CtrObj->GetDriverByControllerName(ctr_name, &ctr_instance1,
                                               &drv_instance1);
  fun_obj.PingController();
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, ret_code);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
}

TEST_F(ControllerFrameworkTest, GetDriver_AfterUpdate) {
  std::string ctr_name_1 = "ctr_drv";
  const key_ctr_t* key_ctr = new key_ctr_t;
  const val_ctr_t* val_ctr = new val_ctr_t;
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  uint32_t ret_code = DRVAPI_RESPONSE_SUCCESS;
  CtrObj->AddController(ctr_name_1, ctr_instance, drv_instance);

  CtrObj->UpdateControllerConfiguration(ctr_name_1, ctr_instance,
                                        drv_instance, *key_ctr, *val_ctr);
  ret_code = CtrObj->GetDriverByControllerName(ctr_name_1, &ctr_instance,
                                               &drv_instance);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, ret_code);
  delete CtrObj;
  delete key_ctr;
  delete val_ctr;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, PostTimer) {
  std::string ctr_name = "ctr1";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  uint32_t ping_interval = 0;
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  int ret = 0;
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  ret = CtrObj->PostTimer(ctr_name, drv_instance, ctr_instance, ping_interval);
  EXPECT_EQ(0, ret);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, AddController_success) {
  std::string ctr_name = "ctr1";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  OdcDriver *drv_instance = new OdcDriver();
  OdcController *ctr_instance = new OdcController();
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  int ret = 0;
  EXPECT_EQ(ret, 0);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}


TEST_F(ControllerFrameworkTest, Add_MultiCtr_success) {
  std::string ctr_name_1 = "ctr1";
  std::string ctr_name_2 = "ctr2";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  controller *ctr_instance1 = static_cast <controller*>(new OdcController());
  uint32_t ret_code1, ret_code2 = DRVAPI_RESPONSE_SUCCESS;
  CtrObj->AddController(ctr_name_1, ctr_instance, drv_instance);
  ret_code1 = CtrObj->GetControllerInstance(ctr_name_1, &ctr_instance,
                                            &drv_instance);
  CtrObj->AddController(ctr_name_2, ctr_instance1, drv_instance);
  ret_code2 = CtrObj->GetControllerInstance(ctr_name_2, &ctr_instance1,
                                            &drv_instance);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, ret_code1);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, ret_code2);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, UpdateCtr_success) {
  std::string ctr_name_1 = "ctr_up";
  pfc_taskq_t id = 2;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  const key_ctr_t *key_ctr = new key_ctr_t;
  const val_ctr_t *val_ctr = new val_ctr_t;
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  uint32_t ret_code = DRVAPI_RESPONSE_SUCCESS;
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name_1, ctr_instance, drv_instance);

  controller *update_ctr = NULL;
  driver*  update_drv = NULL;
  CtrObj->GetControllerInstance(ctr_name_1, &update_ctr, &update_drv);
  ret_code = CtrObj->UpdateControllerConfiguration(ctr_name_1, update_ctr,
                                               update_drv, *key_ctr, *val_ctr);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, ret_code);
  delete CtrObj;
  delete key_ctr;
  delete val_ctr;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, RemoveCtr_success) {
  std::string ctr_name = "ctr_remove";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  const key_ctr_t *key_ctr = new key_ctr_t;
  const val_ctr_t *val_ctr = new val_ctr_t;
  uint32_t ret_code = VTN_DRV_RET_SUCCESS;
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  CtrObj->GetControllerInstance(ctr_name, &ctr_instance, &drv_instance);
  CtrObj->UpdateControllerConfiguration(ctr_name, ctr_instance,
                                        drv_instance, *key_ctr, *val_ctr);
  ret_code = CtrObj->RemoveControllerConfiguration(ctr_name, ctr_instance,
                                                   drv_instance);
  EXPECT_EQ(DRVAPI_RESPONSE_SUCCESS, ret_code);
  delete CtrObj;
  delete key_ctr;
  delete val_ctr;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, RemoveCtr_Failure) {
  std::string ctr_name = "ctr_remove";
  std::string ctr_unknown = "ctr_unkown";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  const key_ctr_t *key_ctr = new key_ctr_t;
  const val_ctr_t *val_ctr = new val_ctr_t;
  uint32_t ret_code = VTN_DRV_RET_SUCCESS;
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  CtrObj->GetControllerInstance(ctr_name, &ctr_instance, &drv_instance);
  CtrObj->UpdateControllerConfiguration(ctr_name, ctr_instance,
                                        drv_instance, *key_ctr, *val_ctr);
  ret_code = CtrObj->RemoveControllerConfiguration(ctr_unknown, ctr_instance,
                                                   drv_instance);
  EXPECT_EQ(DRVAPI_RESPONSE_FAILURE, ret_code);
  delete CtrObj;
  delete key_ctr;
  delete val_ctr;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, RemoveController_List_Empty) {
  std::string ctr_name = "ctr1";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  uint32_t ret_code = VTN_DRV_RET_SUCCESS;
  OdcDriver *drv_instance = new OdcDriver();
  OdcController *ctr_instance = new OdcController();
  ret_code = CtrObj->RemoveControllerConfiguration(ctr_name, ctr_instance,
                                                   drv_instance);
  EXPECT_EQ(VTN_DRV_RET_FAILURE, ret_code);
  delete taskq_;
  delete drv_instance;
  delete ctr_instance;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, RegisterDriver_success) {
  std::string ctr_name_1 = "ctr_add";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  uint32_t ret_code = VTN_DRV_RET_SUCCESS;
  unc_keytype_ctrtype_t controller_type = UNC_CT_ODC;
  OdcDriver *drv_instance = new OdcDriver();
  ret_code = CtrObj->RegisterDriver(controller_type, drv_instance);
  EXPECT_EQ(VTN_DRV_RET_SUCCESS, ret_code);
  delete taskq_;
  delete drv_instance;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, RegisterDriver_MultipleEntry_success) {
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  uint32_t ret_code = VTN_DRV_RET_SUCCESS;
  unc_keytype_ctrtype_t controller_type_1 = UNC_CT_ODC;
  unc_keytype_ctrtype_t controller_type_2 = UNC_CT_PFC;
  OdcDriver *drv_instance = new OdcDriver();
  ret_code = CtrObj->RegisterDriver(controller_type_1, drv_instance);
  ret_code = CtrObj->RegisterDriver(controller_type_2, drv_instance);
  EXPECT_EQ(VTN_DRV_RET_SUCCESS, ret_code);
  delete taskq_;
  delete drv_instance;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, RegisterDriver_DrvInst_NULL) {
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  uint32_t ret_code = VTN_DRV_RET_SUCCESS;
  unc_keytype_ctrtype_t controller_type_1 = UNC_CT_ODC;
  OdcDriver *drv_instance = NULL;
  ret_code = CtrObj->RegisterDriver(controller_type_1, drv_instance);
  EXPECT_EQ(VTN_DRV_RET_FAILURE, ret_code);
  delete taskq_;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetDriverInst_success) {
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  unc_keytype_ctrtype_t controller_type_1 = UNC_CT_ODC;
  OdcDriver *drv_instance = new OdcDriver();
  int ret = 0;
  CtrObj->RegisterDriver(controller_type_1, drv_instance);
  CtrObj->GetDriverInstance(controller_type_1);
  EXPECT_EQ(0, ret);
  delete taskq_;
  delete drv_instance;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetDriverInst1_success) {
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  unc_keytype_ctrtype_t controller_type_1 = UNC_CT_VNP;
  OdcDriver *drv_instance = new OdcDriver();
  int ret = 0;
  CtrObj->RegisterDriver(controller_type_1, drv_instance);
  CtrObj->GetDriverInstance(controller_type_1);
  EXPECT_EQ(0, ret);
  delete taskq_;
  delete drv_instance;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetMultipleEntrySuccess) {
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  unc_keytype_ctrtype_t controller_type_1 = UNC_CT_ODC;
  unc_keytype_ctrtype_t controller_type_2 = UNC_CT_ODC;
  OdcDriver *drv_instance = new OdcDriver();
  int ret1, ret2 = 0;
  CtrObj->RegisterDriver(controller_type_1, drv_instance);
  CtrObj->RegisterDriver(controller_type_2, drv_instance);
  CtrObj->GetDriverInstance(controller_type_1);
  EXPECT_EQ(0, ret1);
  CtrObj->GetDriverInstance(controller_type_2);
  EXPECT_EQ(0, ret2);
  delete taskq_;
  delete drv_instance;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetDriverInst_drv_Inst_NULL) {
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  unc_keytype_ctrtype_t controller_type_1 = UNC_CT_ODC;
  OdcDriver *drv_instance = NULL;
  OdcController *ctr_instance = new OdcController();
  int ret = 0;
  CtrObj->RegisterDriver(controller_type_1, drv_instance);
  CtrObj->GetDriverInstance(controller_type_1);
  EXPECT_EQ(0, ret);
  delete taskq_;
  delete ctr_instance;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetDriver_CtrName_NotFound) {
  std::string ctr_name = "ctr_demo";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  int ret = VTN_DRV_RET_SUCCESS;
  ret = CtrObj->GetDriverByControllerName(ctr_name, &ctr_instance,
                                          &drv_instance);
  EXPECT_EQ(VTN_DRV_RET_FAILURE, ret);
  delete taskq_;
  delete drv_instance;
  delete ctr_instance;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetControllerInst_CtrName_NotFound) {
  std::string ctr_name = "controller_unknown";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  int ret = VTN_DRV_RET_SUCCESS;
  ret = CtrObj->GetControllerInstance(ctr_name, &ctr_instance,
                                      &drv_instance);
  EXPECT_EQ(VTN_DRV_RET_FAILURE, ret);
  delete taskq_;
  delete drv_instance;
  delete ctr_instance;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, SendNotificationToPhysicalDown) {
  std::string ctr_name = "controller_down";
  ConnectionStatus type = CONNECTION_DOWN;
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_);
  CtrObj->SendNotificationToPhysical(ctr_name, type);
  delete CtrObj;
  delete taskq_;
  CtrObj = NULL;
}
}  // namespace driver
}  // namespace unc
