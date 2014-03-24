/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <limits.h>
#include <gtest/gtest.h>

#include <pfcxx/ipc_client.hh>
#include <pfc/ipc_struct.h>

#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#include <time.h>
#include <pfc/base.h>
#include <pfc/ipc.h>
#include <pfc/config.h>

#include <pfc/conf.h>
#include <pfc/log.h>
#include <pfc/util.h>
#include <pfc/event.h>
#include <pfc/path.h>
#include <pfc/hash.h>
#include <pfc/thread.h>
#include <pfc/refptr.h>
#include <pfc/listmodel.h>
#include <pfc/iostream.h>
#include <pfc/ipc_client.h>
#include <pfc/ipc_pfcd.h>
#include <physical_common_def.hh>
#include <itc_read_request.hh>
#include <unc/uppl_common.h>
#include <unc/keytype.h>
#include <odbcm_mgr.hh>
#include <itc_kt_base.hh>
#include <itc_kt_root.hh>
#include <itc_kt_controller.hh>
#include <itc_kt_ctr_domain.hh>
#include <itc_kt_switch.hh>
#include <itc_kt_port.hh>
#include <itc_kt_link.hh>
#include <itc_kt_boundary.hh>
#include <ipct_util.hh>
#include <physicallayer.hh>
#include "PhysicalLayerStub.hh"
#include "ut_util.hh"

using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
using namespace unc::uppl;
using namespace unc::uppl::test;
using namespace std;

class SwitchTest
  : public UpplTestEnv {
};

static char pkName1[] = "{0x10, 0xbc}";
static char pkName2[] = "controller1";

static void getKeyForKtSwitch1(key_switch_t& k) {
  strncpy(reinterpret_cast<char *>(k.switch_id), pkName1, sizeof(k.switch_id));
  strncpy(reinterpret_cast<char *>(k.ctr_key.controller_name), pkName2,
          sizeof(k.ctr_key.controller_name));
}

static void getValForKtSwitch1(val_switch_t& v, bool zero) {
  if (zero) {
    memset(&v, 0, sizeof(v));
  }

  //  uint8_t description[128]
  pfc_strlcpy(reinterpret_cast<char *>(v.description), "switch description",
              sizeof(v.description));

  //  uint8_t model[16]
  pfc_strlcpy(reinterpret_cast<char *>(v.model), "switch model",
              sizeof(v.model));

  string ip_add = "10.100.12.34";
  inet_pton(AF_INET,
            (const char *)ip_add.c_str(),
            &v.ip_address.s_addr);

  string ipv6_add = "";
  inet_pton(AF_INET6, (const char *)ipv6_add.c_str(),
            &v.ipv6_address.s6_addr);

  //  uint8_t admin_status
  v.admin_status = 0;

  //  uint8_t domain_name[32]
  pfc_strlcpy(reinterpret_cast<char *>(v.domain_name), "domain_name",
              sizeof(v.domain_name));
}

static inline void getValForKtSwitch1(val_switch_t& v) {
  getValForKtSwitch1(v, true);
}

static void getValForKtSwitch1(val_switch_st_t& v) {
  memset(&v, 0, sizeof(v));
  getValForKtSwitch1(v.switch_val, false);

  //  uint8_t manufacturer
  pfc_strlcpy(reinterpret_cast<char *>(v.manufacturer), "NEC CORP",
              sizeof(v.manufacturer));

  //  uint8_t hardware
  pfc_strlcpy(reinterpret_cast<char *>(v.hardware), "HW", sizeof(v.hardware));

  //  uint8_t software
  pfc_strlcpy(reinterpret_cast<char *>(v.software), "SW", sizeof(v.software));

  //  uint64_t alarms_status
  v.alarms_status = 0x01;
}

static void getReqHeader(physical_request_header& rh,
                         unc_keytype_operation_t opern,
                         unc_keytype_datatype_t dtype) {
  rh.client_sess_id = 1;
  rh.config_id = 1;
  rh.operation = opern;
  rh.max_rep_count = 0;
  rh.option1 = 0;
  rh.option2 = 0;
  rh.data_type = dtype;
  rh.key_type = UNC_KT_SWITCH;
}

/********TEST CASES***************/
// Validating Key:ControllerId Not found, Operation:UNC_OP_CREATE
TEST_F(SwitchTest, PerformSyntxCheck_controllerId_notFound_01) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10, 0xbc}";
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.PerformSyntaxValidation(db_conn, &k, &v,
                                                operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Validating Key:ControllerId Not found, Operation:UNC_OP_UPDATE
TEST_F(SwitchTest, PerformSyntxCheck_controllerId_notFound_02) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10, 0xbc}";
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.PerformSyntaxValidation(db_conn, &k, &v,
                                                operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Validating Key:ControllerId Not found, Operation:UNC_OP_DELETE
TEST_F(SwitchTest, PerformSyntxCheck_controllerId_notFound_03) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10, 0xbc}";
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  uint32_t operation = UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.PerformSyntaxValidation(db_conn, &k, &v,
                                                operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Validating Key:ControllerId Not found, Operation:UNC_OP_READ
TEST_F(SwitchTest, PerformSyntxCheck_controllerId_notFound_04) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10, 0xbc}";
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  uint32_t operation = UNC_OP_READ;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.PerformSyntaxValidation(db_conn, &k, &v,
                                                operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Validating Key:SwitchId_notFound, Operation:UNC_OP_CREATE
TEST_F(SwitchTest, PerformSyntxCheck_SwitchId_notFound_01) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.PerformSyntaxValidation(db_conn, &k, &v,
                                                operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Validating Key:SwitchId_notFound, Operation:UNC_OP_UPDATE
TEST_F(SwitchTest, PerformSyntxCheck_SwitchId_notFound_02) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.PerformSyntaxValidation(db_conn, &k, &v,
                                                operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Validating Key:SwitchId_notFound, Operation:UNC_OP_DELETE
TEST_F(SwitchTest, PerformSyntxCheck_SwitchId_notFound_03) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  uint32_t operation = UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.PerformSyntaxValidation(
      db_conn,
      &k,
      &v,
      operation,
      UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Validating Key:SwitchId_notFound, Operation:UNC_OP_READ
TEST_F(SwitchTest, PerformSyntxCheck_SwitchId_notFound_04) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  uint32_t operation = UNC_OP_READ;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.PerformSyntaxValidation(db_conn, &k, &v,
                                                operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Validation of key and value struct-Success
TEST_F(SwitchTest, PerformSyntxCheck_ValStruct_success) {
  key_switch_t k;
  val_switch_t v;
  Kt_Switch KtSwitchObj;
  getKeyForKtSwitch1(k);
  getValForKtSwitch1(v);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.PerformSyntaxValidation(db_conn,
                                                &k,
                                                &v,
                                                operation,
                                                UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*******IsKeyExists*************/
// No key given..Returning error
TEST_F(SwitchTest, IskeyExists_NoKey) {
  key_switch_t k;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  vector<string> sw_vect_key_value;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

// DB connection not available, Returns Error
TEST_F(SwitchTest, IskeyExists_Db_Connxn_Error) {
  key_switch_t k;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// DB returned success for Row exists
TEST_F(SwitchTest, IskeyExists_Success) {
  key_switch_t k;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// DB Returned failure for IsRowExists
TEST_F(SwitchTest, IskeyExists_Error) {
  key_switch_t k;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS, ODBCM_RC_FAILED);
  int ret = KtSwitchObj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/****DeleteKeyInstance****/

// Delete operation on unsupported DB: STARTUP
TEST_F(SwitchTest, DeleteKeyInstance_UnsupportedDB_01) {
  key_switch_t k;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn,
                                          &k,
                                          UNC_DT_STARTUP,
                                          UNC_KT_SWITCH);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

// Delete for unsupported DB UNC_DT_CANDIDATE
TEST_F(SwitchTest, DeleteKeyInstance_UnsupportedDB_02) {
  key_switch_t k;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn,
                                          &k,
                                          UNC_DT_CANDIDATE,
                                          UNC_KT_SWITCH);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

// Delete for unsupported DB UNC_DT_RUNNING
TEST_F(SwitchTest, DeleteKeyInstance_UnsupportedDB_03) {
  key_switch_t k;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn,
                                          &k,
                                          UNC_DT_RUNNING,
                                          UNC_KT_SWITCH);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

// Delete operation failed in switch common table
TEST_F(SwitchTest, DeleteKeyInstance_Fail_In_Db) {
  key_switch_t k;
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn,
                                          &k,
                                          UNC_DT_STATE,
                                          UNC_KT_SWITCH);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_DELETE, ret);
}

// Delete operation failed due to DB connection not available
TEST_F(SwitchTest, DeleteKeyInstance_Db_Conxn_Error) {
  key_switch_t k;
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::DELETEONEROW,
                                  ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn,
                                          &k,
                                          UNC_DT_STATE,
                                          UNC_KT_SWITCH);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// Row not Exixts to Delete
TEST_F(SwitchTest, DeleteKeyInstance_RowNotExists) {
  key_switch_t k;
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::DELETEONEROW,
                                  ODBCM_RC_ROW_NOT_EXISTS);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn,
                                          &k,
                                          UNC_DT_STATE,
                                          UNC_KT_SWITCH);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_DELETE, ret);
}

// DElete operation Success
TEST_F(SwitchTest, DeleteKeyInstance_Success) {
  key_switch_t k;
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::DELETEONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn,
                                          &k,
                                          UNC_DT_STATE,
                                          UNC_KT_SWITCH);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// Delete Child classess Success
TEST_F(SwitchTest, DeleteKeyInstance_Child_Del) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn,
                                          &k,
                                          UNC_DT_STATE,
                                          UNC_KT_SWITCH);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// Error Deleting port in DB
TEST_F(SwitchTest, DeleteKeyInstance_Del_port) {
  key_port_t k;
  memset(&k, 0, sizeof(key_port_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::DELETEONEROW, ODBCM_RC_FAILED);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn,
                                          &k,
                                          UNC_DT_STATE,
                                          UNC_KT_SWITCH);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_DELETE, ret);
}

// No Child Instance exist to delete
TEST_F(SwitchTest, DeleteKeyInstance_NoChildInstance) {
  key_switch_t k;
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::DELETEONEROW,
                                  ODBCM_RC_ROW_NOT_EXISTS);
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn,
                                          &k,
                                          UNC_DT_STATE,
                                          UNC_KT_SWITCH);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_DELETE, ret);
}

/******PerformSemanticValidatin***********/
// Key for parent not exists
TEST_F(SwitchTest, PerformSemanticValid_NoParentKey) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  vector<string> sw_vect_key_value;

  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn,
                                                  &k,
                                                  &v,
                                                  operation,
                                                  UNC_DT_STATE);

  EXPECT_EQ(UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST, ret);
}

// Update operation not allowed
TEST_F(SwitchTest, PerformSemanticValid_Upd_NotAllowed) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_UPDATE;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);

  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn,
                                                  &k,
                                                  &v,
                                                  operation,
                                                  UNC_DT_STATE);

  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

// Delete operation not allowed
TEST_F(SwitchTest, PerformSemanticValid_DEl_NotAllowed) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_DELETE;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);

  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn,
                                                  &k,
                                                  &v,
                                                  operation,
                                                  UNC_DT_STATE);

  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

// Read operation not allowed
TEST_F(SwitchTest, PerformSemanticValid_Read_NotAllowed) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_READ;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);

  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn,
                                                  &k,
                                                  &v,
                                                  operation,
                                                  UNC_DT_STATE);

  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

// create operation not allowed for Already
// existing key
TEST_F(SwitchTest, PerformSemanticValid_CREATE_NotAllowed) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);

  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn,
                                                  &k,
                                                  &v,
                                                  operation,
                                                  UNC_DT_STATE);

  EXPECT_EQ(UNC_UPPL_RC_ERR_INSTANCE_EXISTS, ret);
}

// PerformSemanticValid Success for update
TEST_F(SwitchTest, PerformSemanticValid_Update_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_UPDATE;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);

  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn,
                                                  &k,
                                                  &v,
                                                  operation,
                                                  UNC_DT_STATE);

  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformSemanticValid Success for DELETE
TEST_F(SwitchTest, PerformSemanticValid_Delete_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_DELETE;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);

  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn,
                                                  &k,
                                                  &v,
                                                  operation,
                                                  UNC_DT_STATE);

  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformSemanticValid Success for READ
TEST_F(SwitchTest, PerformSemanticValid_Read_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_READ;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);

  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn,
                                                  &k,
                                                  &v,
                                                  operation,
                                                  UNC_DT_STATE);

  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// Parent Key Not exists, DB:UNC_DT_IMPORT
TEST_F(SwitchTest, PerformSemanticValid_Create_In_IMPORT_Fail) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10, 0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  vector<string> sw_vect_key_value;

  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn,
                                                  &k,
                                                  &v,
                                                  operation,
                                                  UNC_DT_IMPORT);

  EXPECT_EQ(UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST, ret);
}

/******ReadSwtchValFromDb******/
// Operation Other Than Read -Success
TEST_F(SwitchTest, ReadSwtchValFromDb_NoREAD) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_CREATE;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn,
                                            &k,
                                            &v,
                                            UNC_DT_STATE,
                                            operation_type,
                                            max_rep_ct,
                                            vect_val_switch_st,
                                            vect_switch_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// No Record Found to read
TEST_F(SwitchTest, ReadSwtchValFromDb_NoRecordFound) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn,
                                            &k,
                                            &v,
                                            UNC_DT_STATE,
                                            operation_type,
                                            max_rep_ct,
                                            vect_val_switch_st,
                                            vect_switch_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

// Db connxn error in Read
TEST_F(SwitchTest, ReadSwtchValFromDb_DbConnxnError) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn,
                                            &k,
                                            &v,
                                            UNC_DT_STATE,
                                            operation_type,
                                            max_rep_ct,
                                            vect_val_switch_st,
                                            vect_switch_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// Read operation failed with DB
TEST_F(SwitchTest, ReadSwtchValFromDb_Error_Db_Get) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_FAILED);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn,
                                            &k,
                                            &v,
                                            UNC_DT_STATE,
                                            operation_type,
                                            max_rep_ct,
                                            vect_val_switch_st,
                                            vect_switch_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

// Read From DB Success
TEST_F(SwitchTest, ReadSwtchValFromDb_READ_Success) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn,
                                            &k,
                                            &v,
                                            UNC_DT_STATE,
                                            operation_type,
                                            max_rep_ct,
                                            vect_val_switch_st,
                                            vect_switch_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// ReadBulk operation failed
TEST_F(SwitchTest, ReadSwtchValFromDb_READBULK_NoInstance) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS,
                                  ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn,
                                            &k,
                                            &v,
                                            UNC_DT_STATE,
                                            operation_type,
                                            max_rep_ct,
                                            vect_val_switch_st,
                                            vect_switch_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

// ReadBulk operation failed:Db conxn error
TEST_F(SwitchTest, ReadSwtchValFromDb_READBULK_07) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS,
                                  ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn,
                                            &k,
                                            &v,
                                            UNC_DT_STATE,
                                            operation_type,
                                            max_rep_ct,
                                            vect_val_switch_st,
                                            vect_switch_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// ReadBulk operation failed:Db err
TEST_F(SwitchTest, ReadSwtchValFromDb_READBULK_Db_Get) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS,
                                  ODBCM_RC_FAILED);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn,
                                            &k,
                                            &v,
                                            UNC_DT_STATE,
                                            operation_type,
                                            max_rep_ct,
                                            vect_val_switch_st,
                                            vect_switch_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

// ReadBulk operation Success
TEST_F(SwitchTest, ReadSwtchValFromDb_READBULK_Success) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS,
                                  ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn,
                                            &k,
                                            &v,
                                            UNC_DT_STATE,
                                            operation_type,
                                            max_rep_ct,
                                            vect_val_switch_st,
                                            vect_switch_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/********ReadInternal****/

TEST_F(SwitchTest, ReadInternal) {
  vector<void*> k;
  vector<void*> v;
  Kt_Switch KtSwitchObj;
  uint32_t operation_type = UNC_OP_READ;

  key_switch_t key;
  val_switch_st_t val;
  getKeyForKtSwitch1(key);
  getValForKtSwitch1(val);
  k.push_back(&key);
  v.push_back(&val);

  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadInternal(db_conn,
                                     k,
                                     v,
                                     UNC_DT_STATE,
                                     operation_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*****ReadBulkInternal****/
TEST_F(SwitchTest, ReadBulkInternal_MaxCt_01) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct = 0;
  vector<val_switch_st_t> vect_val_switch;
  vector<key_switch_t> vect_switch_id;
  OdbcmConnectionHandler *db_conn = NULL;
  int ret = KtSwitchObj.ReadBulkInternal(db_conn,
                                         &k,
                                         &v,
                                         UNC_DT_STATE,
                                         max_rep_ct,
                                         vect_val_switch,
                                         vect_switch_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, ReadBulkInternal_NoRecordFound_02) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct(1);
  vector<val_switch_st_t> vect_val_switch;
  vector<key_switch_t> vect_switch_id;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS,
                                  ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtSwitchObj.ReadBulkInternal(db_conn,
                                         &k,
                                         &v,
                                         UNC_DT_STATE,
                                         max_rep_ct,
                                         vect_val_switch,
                                         vect_switch_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(SwitchTest, ReadBulkInternal_ConnError_03) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct(1);
  vector<val_switch_st_t> vect_val_switch;
  vector<key_switch_t> vect_switch_id;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS,
                                  ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.ReadBulkInternal(db_conn,
                                         &k,
                                         &v,
                                         UNC_DT_STATE,
                                         max_rep_ct,
                                         vect_val_switch,
                                         vect_switch_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(SwitchTest, ReadBulkInternal_DbGetError_04) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct(1);
  vector<val_switch_st_t> vect_val_switch;
  vector<key_switch_t> vect_switch_id;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret = KtSwitchObj.ReadBulkInternal(db_conn,
                                         &k,
                                         &v,
                                         UNC_DT_STATE,
                                         max_rep_ct,
                                         vect_val_switch,
                                         vect_switch_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(SwitchTest, ReadBulkInternal_Success_04) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct(1);
  vector<val_switch_st_t> vect_val_switch;
  vector<key_switch_t> vect_switch_id;
  OdbcmConnectionHandler *db_conn = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS,
                                  ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadBulkInternal(db_conn,
                                         &k,
                                         &v,
                                         UNC_DT_STATE,
                                         max_rep_ct,
                                         vect_val_switch,
                                         vect_switch_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/********ReadBulk********/
TEST_F(SwitchTest, ReadBulk_NoStateDb_01) {
  key_switch_t k;
  memset(&k, 0, sizeof(k));

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct(1);
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = KtSwitchObj.ReadBulk(db_conn,
                                 &k,
                                 UNC_DT_CANDIDATE,
                                 max_rep_ct,
                                 child_index,
                                 parent_call,
                                 is_read_next,
                                 read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

TEST_F(SwitchTest, ReadBulk_Max_Rpct_01) {
  key_switch_t k;
  memset(&k, 0, sizeof(k));

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct = 0;
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = KtSwitchObj.ReadBulk(db_conn,
                                 &k,
                                 UNC_DT_STATE,
                                 max_rep_ct,
                                 child_index,
                                 parent_call,
                                 is_read_next,
                                 read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, ReadBulkInternal_Success) {
  key_switch_t k;
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct(1);
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest read_req;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct,
                                 child_index, parent_call, is_read_next,
                                 &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, ReadBulkInternal_NoRecordFound) {
  key_switch_t k;
  memset(&k, 0, sizeof(k));

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct = 1;
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest read_req;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS
                                  , ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtSwitchObj.ReadBulk(db_conn,
                                 &k,
                                 UNC_DT_STATE,
                                 max_rep_ct,
                                 child_index,
                                 parent_call,
                                 is_read_next,
                                 &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, ReadBulk_childkey) {
  key_switch_t k;
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct = 1;
  int child_index = -1;
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  ODBCManager::stub_setResultcode(ODBCManager::ISROWEXISTS,
                                  ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.ReadBulk(db_conn,
                                 &k,
                                 UNC_DT_STATE,
                                 max_rep_ct,
                                 child_index,
                                 parent_call,
                                 is_read_next,
                                 read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, ReadBulk_SwitchExists_FALSE) {
  key_switch_t k;
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct = 1;
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest read_req;
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS,
                                  ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadBulk(db_conn,
                                 &k,
                                 UNC_DT_STATE,
                                 max_rep_ct,
                                 child_index,
                                 parent_call,
                                 is_read_next,
                                 &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/********PerformRead******/
TEST_F(SwitchTest, PerformRead_01) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);

  int ret =  KtSwitchObj.PerformRead(db_conn,
                                     (uint32_t)0,
                                     (uint32_t)0,
                                     &k,
                                     &v,
                                     (uint32_t)UNC_DT_STATE,
                                     (uint32_t)UNC_OP_READ,
                                     sess,
                                     (uint32_t)UNC_OPT1_DETAIL,
                                     (uint32_t)UNC_OPT2_NONE,
                                     (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(SwitchTest, PerformRead_002_Invalid_option1) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION1);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  int ret =  KtSwitchObj.PerformRead(db_conn,
                                     (uint32_t)0,
                                     (uint32_t)0,
                                     &k,
                                     &v,
                                     (uint32_t)UNC_DT_STATE,
                                     (uint32_t)UNC_OP_READ,
                                     sess,
                                     (uint32_t)UNC_OPT1_NORMAL,
                                     (uint32_t)UNC_OPT2_L2DOMAIN,
                                     (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(SwitchTest, PerformRead_002_Invalid_option2) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  pfc::core::ipc::ServerSession::set_rest(0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION2);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  int ret =  KtSwitchObj.PerformRead(db_conn,
                                     (uint32_t)0,
                                     (uint32_t)0,
                                     &k,
                                     &v,
                                     (uint32_t)UNC_DT_STATE,
                                     (uint32_t)UNC_OP_READ,
                                     sess,
                                     (uint32_t)UNC_OPT1_NORMAL,
                                     (uint32_t)UNC_OPT2_L2DOMAIN,
                                     (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(SwitchTest, PerformRead_002) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  pfc::core::ipc::ServerSession::set_rest(2);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  int ret =  KtSwitchObj.PerformRead(db_conn,
                                     (uint32_t)0,
                                     (uint32_t)0,
                                     &k,
                                     &v,
                                     (uint32_t)UNC_DT_STATE,
                                     (uint32_t)UNC_OP_READ,
                                     sess,
                                     (uint32_t)UNC_OPT1_NORMAL,
                                     (uint32_t)UNC_OPT2_L2DOMAIN,
                                     (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(SwitchTest, PerformRead_ReadSwtVal_Succes_03) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  pfc::core::ipc::ServerSession::set_rest(2);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION2);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.PerformRead(db_conn,
                                     (uint32_t)0,
                                     (uint32_t)0,
                                     &k,
                                     &v,
                                     (uint32_t)UNC_DT_STATE,
                                     (uint32_t)UNC_OP_READ,
                                     sess,
                                     (uint32_t)UNC_OPT1_NORMAL,
                                     (uint32_t)UNC_OPT2_L2DOMAIN,
                                     (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, PerformRead_ReadSwtVal_OPT1_NoMatch) {
  val_switch_st_t v;
  key_switch_t k;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));

  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION2);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  int ret =  KtSwitchObj.PerformRead(db_conn,
                                     (uint32_t)0,
                                     (uint32_t)0,
                                     &k,
                                     &v,
                                     (uint32_t)UNC_DT_STATE,
                                     (uint32_t)UNC_OP_READ,
                                     sess,
                                     (uint32_t)UNC_OPT1_DETAIL,
                                     (uint32_t)UNC_OPT2_L2DOMAIN,
                                     (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, PerformRead_ReadSwtVal_DataType_NoMatch_Success) {
  val_switch_st_t v;
  key_switch_t k;
  memset(&v, 0, sizeof(val_switch_st_t));
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION2);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);

  int ret =  KtSwitchObj.PerformRead(db_conn,
                                     (uint32_t)0,
                                     (uint32_t)0,
                                     &k,
                                     &v,
                                     (uint32_t)UNC_DT_CANDIDATE,
                                     (uint32_t)UNC_OP_READ,
                                     sess,
                                     (uint32_t)UNC_OPT1_NORMAL,
                                     (uint32_t)UNC_OPT2_L2DOMAIN,
                                     (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, PerformRead_ReadSwtVal_addOutput_fail) {
  val_switch_st_t v;
  key_switch_t k;
  memset(&v, 0, sizeof(val_switch_st_t));
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);

  pfc::core::ipc::ServerSession::set_rest(2);
  int ret =  KtSwitchObj.PerformRead(db_conn,
                                     (uint32_t)0,
                                     (uint32_t)0,
                                     &k,
                                     &v,
                                     (uint32_t)UNC_DT_STATE,
                                     (uint32_t)UNC_OP_READ,
                                     sess,
                                     (uint32_t)UNC_OPT1_NORMAL,
                                     (uint32_t)UNC_OPT2_L2DOMAIN,
                                     (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(SwitchTest, PerformRead_005) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
int ret =  KtSwitchObj.PerformRead(db_conn,
                                   (uint32_t)0,
                                   (uint32_t)0,
                                   &k,
                                   &v,
                                   (uint32_t)UNC_DT_STATE,
                                   (uint32_t)UNC_OP_INVALID,
                                   sess,
                                   (uint32_t)UNC_OPT1_NORMAL,
                                   (uint32_t)UNC_OPT2_NONE,
                                   (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, PerformRead_UnsupportDB_STARTUP) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  int ret =  KtSwitchObj.PerformRead(db_conn,
                                     (uint32_t)0,
                                     (uint32_t)0,
                                     &k,
                                     &v,
                                     (uint32_t)UNC_DT_STARTUP,
                                     (uint32_t)UNC_OP_READ,
                                     sess,
                                     (uint32_t)UNC_OPT1_NORMAL,
                                     (uint32_t)UNC_OPT2_NONE,
                                     (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(SwitchTest, PerformRead_UnsupportDb_CANDIDDATE) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  int ret =  KtSwitchObj.PerformRead(db_conn,
                                    (uint32_t)0,
                                    (uint32_t)0,
                                    &k,
                                    &v,
                                    (uint32_t)UNC_DT_CANDIDATE,
                                    (uint32_t)UNC_OP_READ,
                                    sess,
                                    (uint32_t)UNC_OPT1_NORMAL,
                                    (uint32_t)UNC_OPT2_NONE,
                                    (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(SwitchTest, PerformRead_GetOneRow_006) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  int ret =  KtSwitchObj.PerformRead(db_conn,
                                     (uint32_t)0,
                                     (uint32_t)0,
                                     &k,
                                     &v,
                                     (uint32_t)UNC_DT_STATE,
                                     (uint32_t)UNC_OP_READ,
                                     sess,
                                     (uint32_t)UNC_OPT1_NORMAL,
                                     (uint32_t)UNC_OPT2_NONE,
                                     (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*******PopulateSchemaForValidFlag*********/
TEST_F(SwitchTest, PopulateSchemaForValidFlag_Success) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  string valid_new;
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.PopulateSchemaForValidFlag(db_conn,
                                                    &k,
                                                    &v,
                                                    valid_new,
                                                    UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, PopulateSchemaForValidFlag_Failure) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  string valid_new;
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::UPDATEONEROW,
                                  ODBCM_RC_GENERAL_ERROR);
  int ret =  KtSwitchObj.PopulateSchemaForValidFlag(db_conn,
                                                    &k,
                                                    &v,
                                                    valid_new,
                                                    UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

/********SetOperSatus******/
// Faliure of GetoneRow
TEST_F(SwitchTest, SetOperStatus_001) {
  key_switch_t k;
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;

  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtSwitchObj.SetOperStatus(db_conn,
                                       UNC_DT_STATE,
                                       &k,
                                       (UpplSwitchOperStatus)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(SwitchTest, SetOperStatus_002) {
  key_switch_t k;
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;

  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.SetOperStatus(db_conn,
                                       UNC_DT_STATE,
                                       &k,
                                       (UpplSwitchOperStatus)0);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

TEST_F(SwitchTest, SetOperStatus_004) {
  key_switch_t k;
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;

  ODBCManager::stub_setResultcode(ODBCManager::UPDATEONEROW,
                                  ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.SetOperStatus(db_conn,
                                       UNC_DT_STATE,
                                       &k,
                                       (UpplSwitchOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, SetOperStatus_005) {
  key_switch_t k;
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int err = 0;
  ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_PORT, err);
  ser_evt.addOutput((uint32_t)UNC_OP_UPDATE);
  ser_evt.addOutput((uint32_t)UNC_DT_STATE);
  ser_evt.addOutput((uint32_t)UNC_KT_SWITCH);

  ODBCManager::stub_setResultcode(ODBCManager::UPDATEONEROW,
                                  ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.SetOperStatus(db_conn,
                                       UNC_DT_STATE,
                                       &k,
                                       (UpplSwitchOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/******GetAlarmStatus******/
// GetOneRow Success
TEST_F(SwitchTest, GetAlarmStatus_sucess_01) {
  key_switch_t k;
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  uint64_t alarm_status = 1;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.GetAlarmStatus(db_conn,
                                        UNC_DT_STATE,
                                        &k,
                                        alarm_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// GetOneRow Failure
TEST_F(SwitchTest, GetAlarmStatus_Nosuccess_01) {
  key_switch_t k;
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  uint64_t alarm_status = 1;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_FAILED);
  int ret =  KtSwitchObj.GetAlarmStatus(db_conn,
                                        UNC_DT_STATE,
                                        &k,
                                        alarm_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/*****HandleDriverAlarms******/

// Reading alarm status from db failed
TEST_F(SwitchTest, HandleDriverAlarm_01) {
  key_switch_t k;
  val_switch_st_t v;
  getKeyForKtSwitch1(k);
  memset(&v, 0, sizeof(val_switch_st_t));

  Kt_Switch KtSwitchObj;
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtSwitchObj.HandleDriverAlarms(db_conn,
                                            UNC_DT_STATE,
                                            alarm_type,
                                            oper_type,
                                            &k,
                                            &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// Error creating ServerEvent object
// alarm_type:UNC_FLOW_ENT_FULL, oper_type:UNC_OP_CREATE
TEST_F(SwitchTest, HandleDriverAlarm_02) {
  key_switch_t k;
  val_switch_st_t v;
  getKeyForKtSwitch1(k);
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::UPDATEONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleDriverAlarms(db_conn,
                                            UNC_DT_STATE,
                                            alarm_type,
                                            oper_type,
                                            &k,
                                            &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(SwitchTest, HandleDriverAlarm_03) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&v, 0, sizeof(val_switch_st_t));
  getKeyForKtSwitch1(k);

  Kt_Switch KtSwitchObj;
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::UPDATEONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleDriverAlarms(db_conn,
                                            UNC_DT_STATE,
                                            alarm_type,
                                            oper_type,
                                            &k,
                                            &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, HandleDriverAlarm_04) {
  key_switch_t k;
  val_switch_st_t v;
  getKeyForKtSwitch1(k);
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  uint32_t alarm_type = UNC_OFS_LACK_FEATURES;
  uint32_t oper_type = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::UPDATEONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleDriverAlarms(db_conn,
                                            UNC_DT_STATE,
                                            alarm_type,
                                            oper_type,
                                            &k,
                                            &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(SwitchTest, HandleDriverAlarm_05) {
  key_switch_t k;
  val_switch_st_t v;
  getKeyForKtSwitch1(k);
  memset(&v, 0, sizeof(val_switch_st_t));

  Kt_Switch KtSwitchObj;
  uint32_t alarm_type = UNC_OFS_LACK_FEATURES;
  uint32_t oper_type = UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::UPDATEONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleDriverAlarms(db_conn,
                                            UNC_DT_STATE,
                                            alarm_type,
                                            oper_type,
                                            &k,
                                            &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*****HandleOperStaus******/
TEST_F(SwitchTest, Handleoperstatus_BadRequest) {
  key_switch_t *k = NULL;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&v, 0, sizeof(val_switch_st_t));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtSwitchObj.HandleOperStatus(db_conn,
                                          UNC_DT_STATE,
                                          k,
                                          &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

TEST_F(SwitchTest, Handleoperstatus_GetOneRow_success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

TEST_F(SwitchTest, Handleoperstatus_GetOneRow_Fail) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtSwitchObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(SwitchTest, Handleoperstatus_GetBulkRow_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(SwitchTest, Handleoperstatus_GetBulkRow_Failure) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  KtSwitchObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

TEST_F(SwitchTest, GetSwitchValStructure) {
  key_switch_t k;
  val_switch_st_t *obj_val_switch(NULL);
  vector<TableAttrSchema> vect_table_attr_schema;
  vector<string> vect_prim_keys;
  uint8_t operation_type(UNC_OP_READ);
  val_switch_st_t *val_switch_valid_st(NULL);
  stringstream valid;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = UNC_RC_SUCCESS;
  KtSwitchObj.GetSwitchValStructure(db_conn,
                                    obj_val_switch,
                                    vect_table_attr_schema,
                                    vect_prim_keys,
                                    operation_type,
                                    val_switch_valid_st,
                                    valid);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, GetSwitchValStructure_valid) {
  key_switch_t k;
  val_switch_st_t obj_val_switch;
  memset(&obj_val_switch, 0, sizeof(obj_val_switch));
  vector<TableAttrSchema> vect_table_attr_schema;
  vector<string> vect_prim_keys;
  uint8_t operation_type(UNC_OP_READ);
  val_switch_st_t *val_switch_valid_st(NULL);
  stringstream valid;
  Kt_Switch KtSwitchObj;
  obj_val_switch.valid[kIdxSwitch] = UNC_VF_VALID;
  memset(&k, 0, sizeof(key_switch_t));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = UNC_RC_SUCCESS;
  KtSwitchObj.GetSwitchValStructure(db_conn,
                                    &obj_val_switch,
                                    vect_table_attr_schema,
                                    vect_prim_keys,
                                    operation_type,
                                    val_switch_valid_st,
                                    valid);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*******UpdateSwitchValidFlag******/

TEST_F(SwitchTest, UpdateSwitchValidFlag) {
  key_switch_t k;
  val_switch_t v;
  val_switch_st_t v_st;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));
  memset(&v_st, 0, sizeof(v_st));

  Kt_Switch KtSwitchObj;
  unc_keytype_validflag_t new_valid_val = UNC_VF_VALID;
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW,
                                  ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtSwitchObj.UpdateSwitchValidFlag(db_conn,
                                               &k,
                                               &v,
                                               v_st,
                                               new_valid_val,
                                               UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(SwitchTest, UpdateSwitchValidFlag_Success) {
  key_switch_t k;
  val_switch_t v;
  val_switch_st_t v_st;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));
  memset(&v_st, 0, sizeof(v_st));

  Kt_Switch KtSwitchObj;
  unc_keytype_validflag_t new_valid_val = UNC_VF_VALID;
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.UpdateSwitchValidFlag(db_conn, &k, &v, v_st,
                                               new_valid_val, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(SwitchTest, UpdateSwitchValidFlag_NoFillVal) {
  key_switch_t k;
  val_switch_t v;
  val_switch_st_t v_st;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));
  memset(&v_st, 0, sizeof(v_st));

  Kt_Switch KtSwitchObj;
  unc_keytype_validflag_t new_valid_val(UNC_VF_INVALID);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.UpdateSwitchValidFlag(db_conn, &k, &v, v_st,
                                               new_valid_val, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

TEST_F(SwitchTest, UpdateSwitchValidFlag_ret_NULL) {
  key_switch_t k;
  val_switch_st_t v;
  val_switch_st_t v_st;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));
  memset(&v_st, 0, sizeof(v_st));

  Kt_Switch KtSwitchObj;
  unc_keytype_validflag_t new_valid_val(UNC_VF_INVALID);
  OdbcmConnectionHandler *db_conn =NULL;
  ODBCManager::stub_setResultcode(ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ODBCManager::stub_setResultcode(ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  pfc_log_set_level(PFC_LOGLVL_VERBOSE);
  int ret =  KtSwitchObj.UpdateSwitchValidFlag(db_conn, &k, &v, v_st,
                                               new_valid_val, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/********FrameValidValue*******/
TEST_F(SwitchTest, FrameValidValue) {
  string attr_value = "ctr";
  val_switch_st obj_val_switch;
  Kt_Switch KtSwitchObj;
  int ret = UNC_RC_SUCCESS;
  obj_val_switch.valid[kIdxSwitch] = UNC_VF_VALID;
  KtSwitchObj.FrameValidValue(attr_value, obj_val_switch);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
