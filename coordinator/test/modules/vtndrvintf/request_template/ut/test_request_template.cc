/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <pfc/ipc_struct.h>
#include <gtest/gtest.h>
#include <unc/keytype.h>
#include <unc/unc_base.h>
#include <unc/upll_ipc_enum.h>
#include <pfcxx/ipc_server.hh>
#include <driver/driver_interface.hh>
#include <confignode.hh>
#include <vtn_conf_utility.hh>
#include <vtn_conf_data_element_op.hh>
#include <controller_utils.hh>
#include "../../../../../modules/vtndrvintf/include/request_template.hh"

TEST(KT_VTN, Validate_Request1) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;
  sess.clearStubData();
  sess.stub_setArgument_(1);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;
  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}


TEST(KT_VTN, Validate_Request12) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;
  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_VTN, Validate_Request13) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}


TEST(KT_VTN, Validate_Request14) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;
  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;
  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_VTN, Validate_Request15) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_VTN, Validate_Request16) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_VTN, Validate_Request17) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}


TEST(KT_VTN, Validate_Request18) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}



TEST(KT_VTN, Validate_Request19) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_VTN, Validate_Request191) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_VTN, Validate_Request192) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_VTN, Validate_Request193) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_VTN, Validate_Request194) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}



TEST(KT_VTN, Validate_Request2) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_DELETE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}
/*
TEST(KT_VTN, Validate_Request3) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_UPDATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}
*/
TEST(KT_VBR, Validate_Request1) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vbr_req =
      new unc::driver::KtRequestHandler<key_vbr_t, val_vbr_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VBRIDGE;

  uint32_t ret_val = vbr_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vbr_req;
  ctrl_int = NULL;
  vbr_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(KT_VBR, Validate_Request2) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vbr_req =
      new unc::driver::KtRequestHandler<key_vbr_t, val_vbr_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_DELETE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VBRIDGE;

  uint32_t ret_val = vbr_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vbr_req;
  ctrl_int = NULL;
  vbr_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}
/*
TEST(KT_VBR, Validate_Request3) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vbr_req =
      new unc::driver::KtRequestHandler<key_vbr_t, val_vbr_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_UPDATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VBRIDGE;

  uint32_t ret_val = vbr_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vbr_req;
  ctrl_int = NULL;
  vbr_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}
*/
TEST(KT_VBR_IF, Validate_Request1) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vbrif_req =
      new unc::driver::KtRequestHandler<key_vbr_if_t, pfcdrv_val_vbr_if_t>
      (&kt_map);

  pfc::core::ipc::ServerSession sess;


  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VBR_IF;

  uint32_t ret_val = vbrif_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vbrif_req;
  ctrl_int = NULL;
  vbrif_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(KT_VBR_IF, Validate_Request2) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vbrif_req =
      new unc::driver::KtRequestHandler<key_vbr_if_t, pfcdrv_val_vbr_if_t>
      (&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_DELETE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VBR_IF;

  uint32_t ret_val = vbrif_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vbrif_req;
  ctrl_int = NULL;
  vbrif_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(KT_VBR_IF, Validate_Request3) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* vbrif_req =
      new unc::driver::KtRequestHandler<key_vbr_if_t, pfcdrv_val_vbr_if_t>
      (&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_UPDATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VBR_IF;

  uint32_t ret_val = vbrif_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vbrif_req;
  ctrl_int = NULL;
  vbrif_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}


TEST(KT_CTR, Validate_Request1) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* ctr_req =
      new unc::driver::KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;
  sess.clearStubData();
  sess.stub_setArgument_(1);

  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));
  unc::driver::driver::set_ctrl_instance(0);

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_CONTROLLER;

  uint32_t ret_val = ctr_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete ctr_req;
  ctrl_int = NULL;
  ctr_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_CTR, Validate_Request11) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* ctr_req =
      new unc::driver::KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;
  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);

  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  unc::driver::driver::set_ctrl_instance(1);

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_CONTROLLER;

  uint32_t ret_val = ctr_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete ctr_req;
  ctrl_int = NULL;
  ctr_req = NULL;

  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_CTR, Validate_Request12) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* ctr_req =
      new unc::driver::KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;
  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);

  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  unc::driver::driver::set_ctrl_instance(1);

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_CONTROLLER;

  uint32_t ret_val = ctr_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete ctr_req;
  ctrl_int = NULL;
  ctr_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(KT_CTR, Validate_Request13) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* ctr_req =
      new unc::driver::KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;
  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);

  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  unc::driver::driver::set_ctrl_instance(0);

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_CONTROLLER;

  uint32_t ret_val = ctr_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete ctr_req;
  ctrl_int = NULL;
  ctr_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}
TEST(KT_CTR, Validate_Request2) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* ctr_req =
      new unc::driver::KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);

  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_DELETE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_CONTROLLER;

  uint32_t ret_val = ctr_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete ctr_req;
  ctrl_int = NULL;
  ctr_req = NULL;
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(KT_CTR, Validate_Request21) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* ctr_req =
      new unc::driver::KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);

  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_DELETE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_CONTROLLER;

  unc::driver::ControllerFramework::set_result(1);
  uint32_t ret_val = ctr_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete ctr_req;
  ctrl_int = NULL;
  ctr_req = NULL;
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}


TEST(KT_CTR, Validate_Request3) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* ctr_req =
      new unc::driver::KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);

  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_UPDATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_CONTROLLER;

  uint32_t ret_val = ctr_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete ctr_req;
  ctrl_int = NULL;
  ctr_req = NULL;
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(KT_CTR, Validate_Request31) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* ctr_req =
      new unc::driver::KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);

  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_UPDATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_CONTROLLER;
  unc::driver::ControllerFramework::set_result(0);
  unc::driver::driver::set_ctrl_instance(1);
  uint32_t ret_val = ctr_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete ctr_req;
  ctrl_int = NULL;
  ctr_req = NULL;
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(KT_CTR, Validate_Request32) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* ctr_req =
      new unc::driver::KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);

  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_UPDATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_CONTROLLER;
  unc::driver::ControllerFramework::set_result(0);
  unc::driver::driver::set_ctrl_instance(0);
  uint32_t ret_val = ctr_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete ctr_req;
  ctrl_int = NULL;
  ctr_req = NULL;
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(KT_CTR, Validate_Request4) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;

  unc::driver::KtHandler* ctr_req =
      new unc::driver::KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);

  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_READ;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_CONTROLLER;
  unc::driver::ControllerFramework::set_result(0);
  unc::driver::driver::set_ctrl_instance(0);
  uint32_t ret_val = ctr_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete ctr_req;
  ctrl_int = NULL;
  ctr_req = NULL;
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(GLOBAL_CREATE, execute_cmd1) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  uint32_t operation = UNC_OP_CREATE;

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  unc::driver::controller *ctr = NULL;
  unc::driver::driver *drv = NULL;
  std::string ctr_name = "odc1";
  ctrl_int->GetControllerInstance(ctr_name, &ctr, &drv);
  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  uint32_t ret_val = vtn_req->execute_cmd(cfgptr, ctr, drv);

  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;
  if (cfgptr != NULL) {
    delete cfgptr;
    cfgptr = NULL;
  }
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(GLOBAL_DELETE, execute_cmd2) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  uint32_t operation = UNC_OP_DELETE;

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  unc::driver::controller *ctr = NULL;
  unc::driver::driver *drv = NULL;
  std::string ctr_name = "odc1";
  ctrl_int->GetControllerInstance(ctr_name, &ctr, &drv);
  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  uint32_t ret_val = vtn_req->execute_cmd(cfgptr, ctr, drv);

  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;
  if (cfgptr != NULL) {
    delete cfgptr;
    cfgptr = NULL;
  }
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(GLOBAL_UPDATE, execute_cmd3) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  uint32_t operation = UNC_OP_UPDATE;

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  unc::driver::controller *ctr = NULL;
  unc::driver::driver *drv = NULL;
  std::string ctr_name = "odc1";
  ctrl_int->GetControllerInstance(ctr_name, &ctr, &drv);
  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  uint32_t ret_val = vtn_req->execute_cmd(cfgptr, ctr, drv);

  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;
  if (cfgptr != NULL) {
    delete cfgptr;
    cfgptr = NULL;
  }
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(GLOBAL_DEFAULT, execute_cmd4) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  uint32_t operation = UNC_OP_READ;

  key_vtn key_obj;
  val_vtn val_obj;
  memcpy(key_obj.vtn_name, "vtn1", sizeof(key_obj.vtn_name));
  memcpy(val_obj.description, "vtn1_des", sizeof(val_obj.description));

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vtn, val_vtn, val_vtn, uint32_t>
      (&key_obj, &val_obj, &val_obj, operation);
  unc::driver::controller *ctr = NULL;
  unc::driver::driver *drv = NULL;
  std::string ctr_name = "odc1";
  ctrl_int->GetControllerInstance(ctr_name, &ctr, &drv);
  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t, val_vtn_t>(&kt_map);

  uint32_t ret_val = vtn_req->execute_cmd(cfgptr, ctr, drv);

  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;
  if (cfgptr != NULL) {
    delete cfgptr;
    cfgptr = NULL;
  }
  EXPECT_EQ(ret_val, UNC_DRV_RC_INVALID_OPERATION);
}


/*
TEST(KT_ROOT, Validate_Request1) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  ctrl_int->set_root_result(1);

  unc::driver::KtHandler* root_req =
      new unc::driver::KtRequestHandler<key_root_t, unc::driver::val_root_t>
      (&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();

  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_RUNNING;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_ROOT;

  uint32_t ret_val = root_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete root_req;
  ctrl_int = NULL;
  root_req = NULL;
  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}

TEST(KT_ROOT, Validate_Request2) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  ctrl_int->set_root_result(2);

  unc::driver::KtHandler* root_req =
      new unc::driver::KtRequestHandler<key_root_t, unc::driver::val_root_t>
      (&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();

  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_RUNNING;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_ROOT;

  uint32_t ret_val = root_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete root_req;
  ctrl_int = NULL;
  root_req = NULL;
  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_ROOT, Validate_Request3) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  ctrl_int->set_root_result(3);

  unc::driver::KtHandler* root_req =
      new unc::driver::KtRequestHandler<key_root_t, unc::driver::val_root_t>
      (&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();

  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_RUNNING;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_ROOT;

  uint32_t ret_val = root_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete root_req;
  ctrl_int = NULL;
  root_req = NULL;
  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_ROOT, Validate_Request4) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  ctrl_int->set_root_result(3);

  unc::driver::KtHandler* root_req =
      new unc::driver::KtRequestHandler<key_root_t, unc::driver::val_root_t>
      (&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(0);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_ROOT;

  uint32_t ret_val = root_req->handle_request(sess, driver_data, ctrl_int);

  delete ctrl_int;
  delete root_req;
  ctrl_int = NULL;
  root_req = NULL;
  EXPECT_EQ(ret_val, UNC_DRV_RC_ERR_GENERIC);
}

TEST(KT_VTN, ExecuteControllerStatusDown) {
  unc::driver::kt_handler_map kt_map;
  unc::driver::odl_drv_request_header_t driver_data;
  unc::driver::ControllerFramework* ctrl_int =
      new unc::driver::ControllerFramework;
  unc::driver::controller::set_controller_status(1);

  unc::driver::KtHandler* vtn_req =
      new unc::driver::KtRequestHandler<key_vtn_t,
          val_vtn_t,
          unc::driver::vtn_driver_command>(&kt_map);

  pfc::core::ipc::ServerSession sess;

  sess.clearStubData();
  sess.stub_setArgument_(0);
  sess.stub_setArgument_(1);
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(0));
  sess.stub_setAddOutput(uint32_t(1));

  driver_data.header.session_id     = 1;
  driver_data.header.config_id      = 0;
  driver_data.header.operation      = UNC_OP_CREATE;
  driver_data.header.max_rep_count  = 0;
  driver_data.header.option1        = 0;
  driver_data.header.option2        = 0;
  driver_data.header.data_type      = UNC_DT_CANDIDATE;

  snprintf(reinterpret_cast<char*>(driver_data.controller_name),
           32, "%s", "odcdriver");

  driver_data.key_type              = UNC_KT_VTN;

  uint32_t ret_val = vtn_req->handle_request(sess, driver_data, ctrl_int);
  delete ctrl_int;
  delete vtn_req;
  ctrl_int = NULL;
  vtn_req = NULL;

  EXPECT_EQ(ret_val, UNC_RC_SUCCESS);
}
*/
