/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <tclib_module.hh>
#include <unc/keytype.h>
#include <stub/tclib_module/tclib_interface_stub.hh>
#include <stub/tclib_module/libtc_common.hh>
#include <gtest/gtest.h>
#include <pfcxx/ipc_server.hh>
#include <pfcxx/ipc_client.hh>
#include <stdio.h>
#include <iostream>

using namespace unc::tc;

extern uint32_t arg_count;
extern int ipcclnt_err;
extern int ipcclnt_conn;
extern int ipcclnt_invoke_err;
extern int ipcclnt_invoke_resp;
extern int ipcclnt_sess_create_err;

namespace unc {
namespace tclib {

TEST(test_24, test_TcLibRegisterHandler) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  //  pTcLibInterface_ NULL
  ret = tclib_obj.TcLibRegisterHandler(NULL);
  EXPECT_EQ(TC_INVALID_PARAM, ret);
  EXPECT_EQ(NULL, tclib_obj.pTcLibInterface_);

  TcLibInterfaceStub if_stub_obj;
  ret = tclib_obj.TcLibRegisterHandler(&if_stub_obj);
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);
  EXPECT_EQ(&if_stub_obj, tclib_obj.pTcLibInterface_);

  //  handler already active
  ret = tclib_obj.TcLibRegisterHandler(&if_stub_obj);
  EXPECT_EQ(TC_HANDLER_ALREADY_ACTIVE, ret);

  tclib_obj.fini();
}

TEST(test_25, test_UpdateControllerKeyList) {
  TcCommonRet ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  tclib_obj.sess_ = NULL;
  ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_FAILURE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;
  arg_count = 0;
  ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_FAILURE, ret);

  arg_count = 7;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_numof_errors(0);
  ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_SUCCESS, ret);

  arg_count = 10;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_numof_errors(1);
  ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_SUCCESS, ret);

  // two controller id's 1 with no error and another with error
  arg_count = 13;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_numof_errors(0);  // sets only controller 1 with errors=0
  ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_SUCCESS, ret);
  /* pop the elements from commit phase result and see*/
  /*std::vector<TcControllerResult>::iterator it;

    std::cout << "myvector contains:";
    for (it=tclib_obj.commit_phase_result_.begin(); it<tclib_obj.commit_phase_result_.end(); it++){
    std::cout << ' ' << (*it).controller_id;
    std::cout << ' ' << (*it).resp_code;
    std::cout << ' ' << (*it).num_of_errors;
    }
    std::cout << '\n';*/

  arg_count = 13;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_numof_errors(2);
  ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_SUCCESS, ret);

  // string failure
  arg_count = 7;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_FAILURE);
  sessutil.set_numof_errors(0);
  ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_FAILURE, ret);

  // uint32 failure1
  arg_count = 7;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_FAILURE_1);
  sessutil.set_numof_errors(0);
  ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_FAILURE, ret);

  // uint32 failure2
  arg_count = 7;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_FAILURE_2);
  sessutil.set_numof_errors(0);
  ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_FAILURE, ret);


  tclib_obj.fini();
}

TEST(test_27, test_TcLibReadKeyValueDataInfo) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;
  TcCommonRet tmp_ret = TC_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  std::string controller_id;
  uint32_t err_pos = 0;
  uint32_t key_type = 10;
  pfc_ipcstdef_t key_def;
  pfc_ipcstdef_t value_def;
  void* key_data = NULL;
  void* value_data = NULL;

  // invalid oper state for read
  tclib_obj.oper_state_ = MSG_COMMIT_TRANS_START;
  ret = tclib_obj.TcLibReadKeyValueDataInfo(controller_id, err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);
  EXPECT_EQ(TC_INVALID_OPER_STATE, ret);

  // invalid controller id/ controller key map empty
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.TcLibReadKeyValueDataInfo(controller_id, err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);
  EXPECT_EQ(TC_INVALID_CONTROLLER_ID, ret);


  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;

  arg_count = 7;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_numof_errors(0);
  tmp_ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_SUCCESS, tmp_ret);

  // controller key map empty
  controller_id = "openflow1";
  err_pos = 1;
  key_type = 20;
  ret = tclib_obj.TcLibReadKeyValueDataInfo(controller_id, err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);

  EXPECT_EQ(TC_INVALID_CONTROLLER_ID, ret);

  // key type map null
  arg_count = 13;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_numof_errors(2);
  tmp_ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_SUCCESS, tmp_ret);

  TcKeyTypeIndexMap key_map_;
  std::map<std::string, TcKeyTypeIndexMap>::iterator it;
  tclib_obj.controller_key_map_.insert(std::pair<std::string, TcKeyTypeIndexMap>
                                       (controller_id, key_map_));
  err_pos = 1;
  key_type = 20;
  ret = tclib_obj.TcLibReadKeyValueDataInfo(controller_id, err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);
  EXPECT_EQ(TC_INVALID_KEY_TYPE, ret);

  arg_count = 13;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_numof_errors(2);
  tmp_ret = tclib_obj.UpdateControllerKeyList();
  EXPECT_EQ(TC_SUCCESS, tmp_ret);
  /*insert err count and position in key_map_*/
  key_map_.insert(std::pair<uint32_t, uint32_t>(0, 7));
  key_map_.insert(std::pair<uint32_t, uint32_t>(1, 10));
  tclib_obj.controller_key_map_.insert(std::pair<std::string, TcKeyTypeIndexMap>
                                       (controller_id, key_map_));

  controller_id = "openflow1";
  err_pos = 1;
  key_type = 21;
  ret = tclib_obj.TcLibReadKeyValueDataInfo(controller_id, err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);
 // EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  ret = tclib_obj.TcLibReadKeyValueDataInfo("wrong_string", err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);
  EXPECT_EQ(TC_INVALID_PARAM,  ret);

  // invalid err_pos
  err_pos = 5;
  key_type = 20;
  ret = tclib_obj.TcLibReadKeyValueDataInfo(controller_id, err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);
  EXPECT_EQ(TC_INVALID_PARAM, ret);

  // invalid key type
  controller_id = "openflow1";
  err_pos = 1;
  key_type = 30;
  ret = tclib_obj.TcLibReadKeyValueDataInfo(controller_id, err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);
  EXPECT_EQ(TC_INVALID_KEY_TYPE, ret);

  // uint32 failure
  sessutil.set_return_type(RETURN_FAILURE_3);
  controller_id = "openflow1";
  err_pos = 1;
  key_type = 20;
  key_map_.insert(std::pair<uint32_t, uint32_t>(0, 7));
  key_map_.insert(std::pair<uint32_t, uint32_t>(1, 10));
  tclib_obj.controller_key_map_.insert(std::pair<std::string, TcKeyTypeIndexMap>
                                       (controller_id, key_map_));
  ret = tclib_obj.TcLibReadKeyValueDataInfo(controller_id, err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);
//  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  arg_count = 13;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_numof_errors(2);
  tmp_ret = tclib_obj.UpdateControllerKeyList();
  key_map_.insert(std::pair<uint32_t, uint32_t>(0, 7));
  key_map_.insert(std::pair<uint32_t, uint32_t>(1, 10));
  tclib_obj.controller_key_map_.insert(std::pair<std::string, TcKeyTypeIndexMap>
                                       (controller_id, key_map_));
  EXPECT_EQ(TC_SUCCESS, tmp_ret);

  // struct 1 failure
  sessutil.set_return_type_1(RETURN_STRUCT_FAILURE_1);
  controller_id = "openflow1";
  err_pos = 1;
  key_type = 21;
  ret = tclib_obj.TcLibReadKeyValueDataInfo(controller_id, err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);
  //EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  arg_count = 13;
  sessutil.set_read_type(LIB_UPDATE_KEY_LIST);
  sessutil.set_return_type(RETURN_SUCCESS);
  sessutil.set_numof_errors(2);
  tmp_ret = tclib_obj.UpdateControllerKeyList();
  key_map_.insert(std::pair<uint32_t, uint32_t>(0, 7));
  key_map_.insert(std::pair<uint32_t, uint32_t>(1, 10));
  tclib_obj.controller_key_map_.insert(std::pair<std::string, TcKeyTypeIndexMap>
                                       (controller_id, key_map_));
  EXPECT_EQ(TC_SUCCESS, tmp_ret);

  // struct 2 failure
  sessutil.set_return_type_1(RETURN_STRUCT_FAILURE_2);
  controller_id = "openflow1";
  err_pos = 1;
  key_type = 21;
  value_data = malloc(2);
  ret = tclib_obj.TcLibReadKeyValueDataInfo(controller_id, err_pos,
                                            key_type, key_def, value_def,
                                            key_data, value_data);
  //EXPECT_EQ(TC_API_COMMON_FAILURE, ret);
  free(value_data);
  value_data = NULL;

  tclib_obj.fini();
}

TEST(test_28, test_TcLibWriteControllerInfo) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  std::string controller_id = "openflow1";
  uint32_t resp_code = 0;
  uint32_t num_of_errors = 0;

  // invalid oper state for read
  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.TcLibWriteControllerInfo(controller_id, resp_code,
                                           num_of_errors);
  EXPECT_EQ(TC_INVALID_OPER_STATE, ret);

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;

  // success
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  sessutil.set_read_type(LIB_WRITE_API);
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.TcLibWriteControllerInfo(controller_id, resp_code,
                                           num_of_errors);
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);
  EXPECT_EQ(tclib_obj.controllerid_, controller_id);

  // string set failure
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.TcLibWriteControllerInfo(controller_id,
                                           resp_code, num_of_errors);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  // uint32 resp code failure
  resp_code = 0;
  num_of_errors = 1;
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.TcLibWriteControllerInfo(controller_id,
                                           resp_code, num_of_errors);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  // uint32 num of errors failure
  resp_code = 2;
  num_of_errors = 1;
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.TcLibWriteControllerInfo(controller_id,
                                           resp_code, num_of_errors);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  tclib_obj.fini();
}

TEST(test_29, test_TcLibWriteKeyValueDataInfo) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  std::string controller_id = "openflow1";
  uint32_t key_type = 10;
  pfc_ipcstdef_t key_def;
  pfc_ipcstdef_t value_def;
  void* key_data = NULL;
  void* value_data = NULL;

  // invalid oper state for read
  tclib_obj.oper_state_ = MSG_NONE;
  ret = tclib_obj.TcLibWriteKeyValueDataInfo(controller_id,
                                             key_type, key_def, value_def,
                                             key_data, value_data);
  EXPECT_EQ(TC_INVALID_OPER_STATE, ret);

  // invalid controller id
  tclib_obj.controllerid_ = "openflow5";
  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  ret = tclib_obj.TcLibWriteKeyValueDataInfo(controller_id,
                                             key_type, key_def, value_def,
                                             key_data, value_data);
  EXPECT_EQ(TC_INVALID_CONTROLLER_ID, ret);
  tclib_obj.controllerid_ = "openflow1";

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  TcServerSessionUtils sessutil;

  tclib_obj.sess_ = &sess;

  // success
  key_def.ist_name = "key1";
  value_def.ist_name = "val1";

  tclib_obj.oper_state_ = MSG_COMMIT_VOTE_DRIVER_RESULT;
  sessutil.set_read_type(LIB_WRITE_API);
  sessutil.set_return_type(RETURN_SUCCESS);
  ret = tclib_obj.TcLibWriteKeyValueDataInfo(controller_id,
                                             key_type, key_def, value_def,
                                             key_data, value_data);
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  // uint32 resp code failure
  key_type = 0;
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.TcLibWriteKeyValueDataInfo(controller_id,
                                             key_type, key_def, value_def,
                                             key_data, value_data);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  // set key struct failure
  key_type = 10;
  key_def.ist_name = "key4";
  value_def.ist_name = "val1";
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.TcLibWriteKeyValueDataInfo(controller_id,
                                             key_type, key_def, value_def,
                                             key_data, value_data);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  // set value struct failure
  key_def.ist_name = "key1";
  value_def.ist_name = "val1";
  value_data = malloc(2);
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.TcLibWriteKeyValueDataInfo(controller_id,
                                             key_type, key_def, value_def,
                                             key_data, value_data);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);
  free(value_data);
  value_data = NULL;

  tclib_obj.fini();
}

TEST(test_26, test_ipcService) {
  pfc_ipcresp_t ret = 0;

  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  pfc_ipcsrv_t *srv = NULL;
  pfc::core::ipc::ServerSession sess(srv);
  // tclib_obj.pTcLibInterface_ NULL
  ret = tclib_obj.ipcService(sess, TCLIB_NOTIFY_SESSION_CONFIG);
  EXPECT_EQ(PFC_IPCRESP_FATAL, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  arg_count = 0;
  ret = tclib_obj.ipcService(sess, TCLIB_NOTIFY_SESSION_CONFIG);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_COMMIT_TRANSACTION);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_COMMIT_DRV_VOTE_GLOBAL);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_COMMIT_DRV_RESULT);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_COMMIT_GLOBAL_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_AUDIT_TRANSACTION);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_AUDIT_DRV_VOTE_GLOBAL);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_AUDIT_DRV_RESULT);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_AUDIT_GLOBAL_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_SAVE_CONFIG);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_CLEAR_STARTUP);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_USER_ABORT);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_GET_DRIVERID);
  EXPECT_EQ(UNC_CT_UNKNOWN, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_CONTROLLER_TYPE);
  EXPECT_EQ(UNC_CT_UNKNOWN, ret);

  ret = tclib_obj.ipcService(sess, TCLIB_AUDIT_CONFIG);
  EXPECT_EQ(TC_FAILURE, ret);

  ret = tclib_obj.ipcService(sess, 20);
  EXPECT_EQ(PFC_IPCRESP_FATAL, ret);

  tclib_obj.fini();
}

TEST(test_30, test_TcLibAuditControllerRequest) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;
  pfc_modattr_t mattr;
  mattr.pma_name = "tclib";
  TcLibModule tclib_obj(&mattr);
  tclib_obj.init();

  std::string controller_id = "openflow1";
  // tclib_obj.pTcLibInterface_ NULL
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  TcLibInterfaceStub if_stub_obj;
  tclib_obj.pTcLibInterface_ = &if_stub_obj;

  // ctr type unknown
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);

  if_stub_obj.ctr_type = UNC_CT_PFC;
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_COMMON_SUCCESS, ret);

  // alt open fail casess
  ipcclnt_err = 1;
  ipcclnt_conn = 1;
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_FATAL, ret);

  ipcclnt_err = 0;
  ipcclnt_conn = 0;
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_FATAL, ret);
  ipcclnt_conn = 1;

  // sess create fail
  ipcclnt_sess_create_err = 1;
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_FATAL, ret);
  ipcclnt_sess_create_err = 0;

  // invoke fail
  ipcclnt_invoke_err = 1;
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_FATAL, ret);
  ipcclnt_invoke_err = 0;

  // resp fail case TC_OPER_ABORT
  ipcclnt_invoke_err = 0;
  ipcclnt_invoke_resp = TC_OPER_ABORT;
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);
  ipcclnt_invoke_resp = TC_OPER_SUCCESS;

  // set uint32 failure
  TcClientSessionUtils sessutil;
  /*opertype failure*/
  sessutil.set_return_type(RETURN_FAILURE_1);
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);
  /*controller_id failure*/
  sessutil.set_return_type(RETURN_FAILURE_2);
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);
  /*ctr_type failure*/
  sessutil.set_return_type(RETURN_FAILURE_3);
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);
  /*force_reconnect failure*/
  sessutil.set_return_type(RETURN_FAILURE);
  ret = tclib_obj.TcLibAuditControllerRequest(controller_id);
  EXPECT_EQ(TC_API_COMMON_FAILURE, ret);
  tclib_obj.fini();
}

}  // namespace tclib
}  // namespace unc

