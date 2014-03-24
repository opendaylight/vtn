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
#include <stdio.h>
#include <odc_drv.hh>
#include <odc_controller.hh>
#include <controller_fw.hh>
#include <arpa/inet.h>

namespace unc {
namespace driver {

class ControllerFrameworkTest: public testing::Test {
};

TEST_F(ControllerFrameworkTest, GetControllerInst_success) {
  std::string ctr_name = "ctr_demo";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  controller *ctr(NULL);
  driver *drv(NULL);
  UncRespCode ret_code(CtrObj->GetControllerInstance(ctr_name, &ctr,
                                                         &drv));
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetDriver_success) {
  std::string ctr_name = "ctr_demo";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  controller *ctr(NULL);
  driver *drv(NULL);
  EXPECT_EQ(UNC_RC_SUCCESS,
            CtrObj->GetControllerInstance(ctr_name, &ctr, &drv));
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);

  ctr = NULL;
  drv = NULL;
  UncRespCode ret_code(CtrObj->GetDriverByControllerName(ctr_name, &ctr,
                                                             &drv));
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetDriver_failure) {
  std::string ctr_name = "ctr_demo";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  controller *ctr(NULL);
  driver *drv(NULL);
  EXPECT_EQ(UNC_RC_SUCCESS,
            CtrObj->GetControllerInstance(ctr_name, &ctr, &drv));
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);

  ctr = NULL;
  drv = NULL;
  UncRespCode ret_code(CtrObj->GetDriverByControllerName(ctr_name, &ctr,
                                                             &drv));
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
}

TEST_F(ControllerFrameworkTest, PingController_success) {
  std::string ctr_name = "ctr_post";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  pfc::core::ipc::ServerSession::set_rest(1);
  ReadParams fun_obj(ctr_name, CtrObj);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  driver *drv(NULL);
  controller *ctr(NULL);
  UncRespCode ret_code(CtrObj->GetDriverByControllerName(ctr_name, &ctr,
                                                             &drv));
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);
  fun_obj.PingController();
  EXPECT_EQ(CONNECTION_UP, ctr_instance->get_connection_status());
  delete CtrObj;
  delete drv_instance;
  delete taskq_;
}

TEST_F(ControllerFrameworkTest, PingController_Failure) {
  std::string ctr_name = "ctr_post";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  pfc::core::ipc::ServerSession::set_rest(1);
  ReadParams fun_obj(ctr_name, CtrObj);
  OdcDriver *odcdrv(new OdcDriver());
  driver *drv_instance = static_cast <driver*>(odcdrv);
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  driver *drv(NULL);
  controller *ctr(NULL);
  UncRespCode ret_code(CtrObj->GetDriverByControllerName(ctr_name, &ctr,
                                                             &drv));
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);

  odcdrv->set_ping_result(PFC_FALSE);

  fun_obj.PingController();
  EXPECT_EQ(CONNECTION_DOWN, ctr_instance->get_connection_status());
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
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name_1, ctr_instance, drv_instance);

  UncRespCode ret_code;
  ret_code = CtrObj->UpdateControllerConfiguration(ctr_name_1, ctr_instance,
                                                   drv_instance, *key_ctr,
                                                   *val_ctr);
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);

  driver *drv(NULL);
  controller *ctr(NULL);
  ret_code = CtrObj->GetDriverByControllerName(ctr_name_1, &ctr, &drv);
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);
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
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);
  int ret(CtrObj->PostTimer(ctr_name, drv_instance, ctr_instance,
                            ping_interval));
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
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  OdcDriver *drv_instance = new OdcDriver();
  OdcController *ctr_instance = new OdcController();

  driver *drv(NULL);
  driver *compare_drv_ptr(NULL);
  controller *ctr(NULL);
  controller *compare_ctr_ptr(NULL);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            CtrObj->GetDriverByControllerName(ctr_name, &ctr, &drv));
  EXPECT_EQ(compare_ctr_ptr, ctr);
  EXPECT_EQ(compare_drv_ptr, drv);

  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);

  drv = NULL;
  ctr = NULL;
  EXPECT_EQ(UNC_RC_SUCCESS,
            CtrObj->GetDriverByControllerName(ctr_name, &ctr, &drv));
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);

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
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  driver *drv_instance1 = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  controller *ctr_instance1 = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name_1, ctr_instance, drv_instance);
  CtrObj->AddController(ctr_name_2, ctr_instance1, drv_instance1);

  controller *ctr(NULL);
  driver *drv(NULL);
  EXPECT_EQ(UNC_RC_SUCCESS,
            CtrObj->GetControllerInstance(ctr_name_1, &ctr, &drv));
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);

  ctr = NULL;
  drv = NULL;
  EXPECT_EQ(UNC_RC_SUCCESS,
            CtrObj->GetControllerInstance(ctr_name_2, &ctr, &drv));
  EXPECT_EQ(ctr_instance1, ctr);
  EXPECT_EQ(drv_instance1, drv);

  delete CtrObj;
  delete drv_instance;
  delete drv_instance1;
  delete taskq_;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, UpdateCtr_success) {
  std::string ctr_name_1 = "ctr_up";
  pfc_taskq_t id = 2;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  const key_ctr_t *key_ctr = new key_ctr_t;
  const val_ctr_t *val_ctr = new val_ctr_t;
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name_1, ctr_instance, drv_instance);

  controller *ctr(NULL);
  driver* drv(NULL);
  EXPECT_EQ(UNC_RC_SUCCESS,
            CtrObj->GetControllerInstance(ctr_name_1, &ctr, &drv));
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);

  UncRespCode ret_code;
  ret_code = CtrObj->UpdateControllerConfiguration(ctr_name_1, ctr, drv,
                                                   *key_ctr, *val_ctr);
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);
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
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  const key_ctr_t *key_ctr = new key_ctr_t;
  const val_ctr_t *val_ctr = new val_ctr_t;
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);

  controller *compare_ctr_ptr(NULL);
  controller *ctr(NULL);
  driver *compare_drv_ptr(NULL);
  driver *drv(NULL);
  EXPECT_EQ(UNC_RC_SUCCESS,
            CtrObj->GetControllerInstance(ctr_name, &ctr, &drv));
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);

  UncRespCode ret_code;
  ret_code = CtrObj->UpdateControllerConfiguration(ctr_name, ctr_instance,
                                                   drv_instance, *key_ctr,
                                                   *val_ctr);
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);

  ret_code = CtrObj->RemoveControllerConfiguration(ctr_name, ctr_instance,
                                                   drv_instance);
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);

  ctr = NULL;
  drv = NULL;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            CtrObj->GetControllerInstance(ctr_name, &ctr, &drv));
  EXPECT_EQ(compare_ctr_ptr, ctr);
  EXPECT_EQ(compare_drv_ptr, drv);

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
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  const key_ctr_t *key_ctr = new key_ctr_t;
  const val_ctr_t *val_ctr = new val_ctr_t;
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  CtrObj->AddController(ctr_name, ctr_instance, drv_instance);

  controller *ctr(NULL);
  driver *drv(NULL);
  EXPECT_EQ(UNC_RC_SUCCESS,
            CtrObj->GetControllerInstance(ctr_name, &ctr, &drv));
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);

  UncRespCode ret_code;
  ret_code = CtrObj->UpdateControllerConfiguration(ctr_name, ctr_instance,
                                                   drv_instance, *key_ctr,
                                                   *val_ctr);
  EXPECT_EQ(UNC_RC_SUCCESS, ret_code);

  ret_code = CtrObj->RemoveControllerConfiguration(ctr_unknown, ctr_instance,
                                                   drv_instance);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, ret_code);

  ctr = NULL;
  drv = NULL;
  EXPECT_EQ(UNC_RC_SUCCESS,
            CtrObj->GetControllerInstance(ctr_name, &ctr, &drv));
  EXPECT_EQ(ctr_instance, ctr);
  EXPECT_EQ(drv_instance, drv);

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
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  OdcDriver *drv_instance = new OdcDriver();
  OdcController *ctr_instance = new OdcController();
  UncRespCode ret_code;
  ret_code = CtrObj->RemoveControllerConfiguration(ctr_name, ctr_instance,
                                                   drv_instance);
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC, ret_code);
  delete taskq_;
  delete drv_instance;
  delete ctr_instance;
  delete CtrObj;
  CtrObj = NULL;
}

/*
 * Test case for RegisterDriver() and GetDriverInstance()
 *   - Successful test using one driver.
 */
TEST_F(ControllerFrameworkTest, RegisterDriver_success) {
  std::string ctr_name_1 = "ctr_add";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  unc_keytype_ctrtype_t controller_type = UNC_CT_ODC;
  OdcDriver *drv_instance = new OdcDriver();
  driver *compare_drv_ptr = NULL;
  EXPECT_EQ(compare_drv_ptr, CtrObj->GetDriverInstance(controller_type));
  EXPECT_EQ(UNC_RC_SUCCESS,
            CtrObj->RegisterDriver(controller_type, drv_instance));
  EXPECT_EQ(drv_instance, CtrObj->GetDriverInstance(controller_type));

  delete taskq_;
  delete drv_instance;
  delete CtrObj;
  CtrObj = NULL;
}

/*
 * Test case for RegisterDriver() and GetDriverInstance()
 *   - Successful test using multiple drivers.
 */

TEST_F(ControllerFrameworkTest, RegisterDriver_MultipleEntry_success) {
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  driver *compare_drv_ptr(NULL);
  struct testdrv {
    unc_keytype_ctrtype_t  type;
    OdcDriver *driver;
  } *tdp, testdrv[] = {
    {UNC_CT_ODC, NULL},
    {UNC_CT_VNP, NULL},
    {UNC_CT_PFC, NULL},
  };

  for (tdp = testdrv; tdp < PFC_ARRAY_LIMIT(testdrv); tdp++) {
    EXPECT_EQ(compare_drv_ptr, CtrObj->GetDriverInstance(tdp->type));
  }
  for (tdp = testdrv; tdp < PFC_ARRAY_LIMIT(testdrv); tdp++) {
    tdp->driver = new OdcDriver();
    EXPECT_EQ(UNC_RC_SUCCESS,
              CtrObj->RegisterDriver(tdp->type, tdp->driver));
  }
  for (tdp = testdrv; tdp < PFC_ARRAY_LIMIT(testdrv); tdp++) {
    EXPECT_EQ(tdp->driver, CtrObj->GetDriverInstance(tdp->type));
  }

  for (tdp = testdrv; tdp < PFC_ARRAY_LIMIT(testdrv); tdp++) {
    delete tdp->driver;
  }

  delete taskq_;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, RegisterDriver_DrvInst_NULL) {
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  unc_keytype_ctrtype_t controller_type = UNC_CT_ODC;
  OdcDriver *drv_instance = NULL;
  driver *compare_drv_ptr = NULL;
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            CtrObj->RegisterDriver(controller_type, drv_instance));
  EXPECT_EQ(compare_drv_ptr, CtrObj->GetDriverInstance(controller_type));
  delete taskq_;
  delete CtrObj;
  CtrObj = NULL;
}

TEST_F(ControllerFrameworkTest, GetDriver_CtrName_NotFound) {
  std::string ctr_name = "ctr_demo";
  pfc_taskq_t id = 1;
  pfc::core::TaskQueue* taskq_ = new pfc::core::TaskQueue(id);
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            CtrObj->GetDriverByControllerName(ctr_name, &ctr_instance,
                                              &drv_instance));
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
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  driver *drv_instance = static_cast <driver*>(new OdcDriver());
  controller *ctr_instance = static_cast <controller*>(new OdcController());
  EXPECT_EQ(UNC_DRV_RC_ERR_GENERIC,
            CtrObj->GetControllerInstance(ctr_name, &ctr_instance,
                                          &drv_instance));
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
  ControllerFramework *CtrObj = new ControllerFramework(taskq_, 1);
  pfc::core::ipc::ServerSession::set_rest(1);
  CtrObj->SendNotificationToPhysical(ctr_name, type);
  delete CtrObj;
  delete taskq_;
  CtrObj = NULL;
}
}  // namespace driver
}  // namespace unc
