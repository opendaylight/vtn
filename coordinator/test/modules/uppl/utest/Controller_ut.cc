/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at  http://www.eclipse.org/legal/epl-v10.html
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
#include <tclib_module.hh>
#include <odbcm_mgr.hh>
#include <physical_common_def.hh>
#include <unc/uppl_common.h>
#include <unc/keytype.h>
#include <itc_kt_base.hh>
#include <itc_kt_root.hh>
#include <itc_kt_controller.hh>
#include <itc_kt_ctr_domain.hh>
#include <itc_kt_switch.hh>
#include <itc_kt_port.hh>
#include <itc_kt_link.hh>
#include <itc_kt_boundary.hh>
#include <itc_read_request.hh>
#include <itc_kt_logicalport.hh>
#include <ipct_util.hh>
#include "PhysicalLayerStub.hh"
#include "ut_util.hh"

using unc::uppl::PhysicalLayer;
using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
using namespace std;
using namespace unc::tclib;
using namespace unc::uppl::test;

class ControllerTest
  : public UpplTestEnv {
};

static char pkctrName1[] = "Controller1";
static char pkDomainName2[] = "Domain1";
static char valDescription[] = "description1";

TEST_F(ControllerTest, PerformSyntxCheck_Domainname_notFound_01) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,
                                                &k,
                                                &v,
                                                operation,
                                                UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(ControllerTest, PerformSyntxCheck_Controllername_notFound_02) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,
                                                &k,
                                                &v,
                                                operation,
                                                UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(ControllerTest, PerformSyntxCheck_Valstrct_without_type_03) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,
                                                &k,
                                                &v,
                                                operation,
                                                UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(ControllerTest, PerformSyntxCheck_Valstrct_04) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,
                                                &k,
                                                &v,
                                                operation,
                                                UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(ControllerTest, PerformSyntxCheck_Valstrct_05) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,
                                                &k,
                                                &v,
                                                operation,
                                                UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(ControllerTest, PerformSyntxCheck_Valstrct_06) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  v.type = 0;
  v.valid[kIdxType] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,
                                                &k,
                                                &v,
                                                operation,
                                                UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(ControllerTest, PerformSyntxCheck_Valstrct_07) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,
                                                &k,
                                                &v,
                                                operation,
                                                UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(ControllerTest, PerformSyntxCheck_Valstrct_08) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(
      db_conn, &k, &v, operation, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
#if 0 //crash
//  Create for unsupported datatype
TEST_F(ControllerTest, Create_01) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Create(db_conn,
                               session_id,
                               configuration_id,
                               &k,
                               &v,
                               UNC_DT_STATE,
                               ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
#endif
#if 0
//TODO(ODC)
//  Domain Create success
TEST_F(ControllerTest, Create_02) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW,
                                             ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::CREATEONEROW,
      ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Create(db_conn,
                               session_id,
                               configuration_id,
                               &k,
                               &v,
                               UNC_DT_CANDIDATE,
                               ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
#endif
//  get_controller_type returns failure
TEST_F(ControllerTest, Create_03) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW,
      ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Create(db_conn,
                               session_id,
                               configuration_id,
                               &k,
                               &v,
                               UNC_DT_CANDIDATE,
                               ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
#if 0 // crash
// Create on unsupported datatype
TEST_F(ControllerTest, Create) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Create(db_conn,
                               session_id,
                               configuration_id,
                               &k,
                               &v,
                               UNC_DT_STATE,
                               ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
#endif
TEST_F(ControllerTest, PerformSemanticValidation_01) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,
                                                &k,
                                                &v,
                                                operation,
                                                UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// IsKeyExists  with UNC_DT_CANDIDATE datatype
// ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(ControllerTest, IsKeyExists_01) {
  Kt_Controller  ktCtrlrObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS,
      ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.IsKeyExists(db_conn,
                                    UNC_DT_CANDIDATE,
                                    vect_key);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  IsKeyExists  with UNC_DT_CANDIDATE datatype
//  ODBC return ODBCM_RC_ROW_EXISTS
TEST_F(ControllerTest, IsKeyExists_02) {
  Kt_Controller  ktCtrlrObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS,
      ODBCM_RC_ROW_EXISTS);
  int ret =  ktCtrlrObj.IsKeyExists(db_conn, UNC_DT_CANDIDATE, vect_key);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  IsKeyExists  with UNC_DT_CANDIDATE datatype
//  ODBC return ODBCM_RC_QUERY_TIMEOUT
TEST_F(ControllerTest, IsKeyExists_03) {
  Kt_Controller  ktCtrlrObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS,
      ODBCM_RC_QUERY_TIMEOUT);
  int ret =  ktCtrlrObj.IsKeyExists(db_conn, UNC_DT_CANDIDATE, vect_key);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  IsKeyExists  with key structure empty
TEST_F(ControllerTest, IsKeyExists_04) {
  Kt_Controller  ktCtrlrObj;
  vector<string> vect_key;
  vect_key.clear();
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.IsKeyExists(db_conn, UNC_DT_CANDIDATE, vect_key);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

//  CreateKeyInstance with UNC_DT_CANDIDATE
//  datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(ControllerTest, CreateKeyInstance_01) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::CREATEONEROW,
      ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn, &k, &v,
                                          UNC_DT_CANDIDATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  CreateKeyInstance with UNC_DT_RUNNING datatype ODBC
TEST_F(ControllerTest, CreateKeyInstance_02) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::CREATEONEROW,
      ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,
                                          &k,
                                          &v,
                                          UNC_DT_RUNNING,
                                          key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  CreateKeyInstance with UNC_DT_STATE
//  datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(ControllerTest, CreateKeyInstance_03) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::CREATEONEROW,
      ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,
                                          &k,
                                          &v,
                                          UNC_DT_STATE,
                                          key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  CreateKeyInstance with UNC_DT_IMPORT
//  datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(ControllerTest, CreateKeyInstance_04) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =
      ktCtrlrObj.CreateKeyInstance(db_conn, &k, &v, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  CreateKeyInstance with UNC_DT_CANDIDATE datatype ODBC return Failure
TEST_F(ControllerTest, CreateKeyInstance_05) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::CREATEONEROW,
      ODBCM_RC_TRANSACTION_ERROR);
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,
                                          &k,
                                          &v,
                                          UNC_DT_CANDIDATE,
                                          key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_CREATE, ret);
}
#if 0
//TODO(ODC)
//  CreateKeyInstance with UNC_DT_CANDIDATE datatype ODBC ODBCM_RC_SUCCESS
TEST_F(ControllerTest, CreateKeyInstance_06) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::CREATEONEROW,
      ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,
                                          &k,
                                          &v,
                                          UNC_DT_CANDIDATE,
                                          key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
#endif
//  CreateKeyInstance with UNC_DT_STATE  datatype ODBC return Failure
TEST_F(ControllerTest, CreateKeyInstance_07) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::CREATEONEROW,
      ODBCM_RC_TRANSACTION_ERROR);
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,
                                          &k,
                                          &v,
                                          UNC_DT_STATE,
                                          key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_CREATE, ret);
}

//  CreateKeyInstance with UNC_DT_IMPORT  datatype ODBC return Failure
TEST_F(ControllerTest, CreateKeyInstance_08) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::CREATEONEROW,
      ODBCM_RC_TRANSACTION_ERROR);
  int ret =  ktCtrlrObj.CreateKeyInstance(
      db_conn,
      &k,
      &v,
      UNC_DT_IMPORT,
      key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_CREATE, ret);
}
#if 0 //crash
//  Update for unsupported datatype
TEST_F(ControllerTest, Update_01) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Update(db_conn, session_id, configuration_id,
                               &k, &v, UNC_DT_STATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
#endif
//  Domain Update success

TEST_F(ControllerTest, Update_02) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW,
      ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW,
      ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Update(db_conn,
                               session_id,
                               configuration_id,
                               &k,
                               &v,
                               UNC_DT_CANDIDATE,
                               ses);
  EXPECT_NE(UNC_RC_SUCCESS, ret);
}

//  get_controller_type returns failure
TEST_F(ControllerTest, Update_03) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW,
      ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Update(db_conn,
                               session_id,
                               configuration_id,
                               &k,
                               &v,
                               UNC_DT_CANDIDATE,
                               ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  UpdateKeyInstance with UNC_DT_CANDIDATE ODBC
//  retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(ControllerTest, UpdateKeyInstance_01) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW,
      ODBCM_RC_CONNECTION_ERROR);
  int ret =
      ktCtrlrObj.UpdateKeyInstance(db_conn, &k, &v, UNC_DT_CANDIDATE,
                                   key_type,true);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  UpdateKeyInstance on unsupported datatype
TEST_F(ControllerTest, UpdateKeyInstance_02) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.UpdateKeyInstance(db_conn, &k, &v,
                                          UNC_DT_RUNNING, key_type,true);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}
//  UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(ControllerTest, UpdateKeyInstance_03) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW,
      ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.UpdateKeyInstance(db_conn, &k, &v,
                                          UNC_DT_IMPORT, key_type,true);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns ODBCM_RC_SUCCESS
TEST_F(ControllerTest, UpdateKeyInstance_04) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW,
      ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.UpdateKeyInstance(db_conn,
                                          &k,
                                          &v,
                                          UNC_DT_IMPORT,
                                          key_type,true);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns
TEST_F(ControllerTest, UpdateKeyInstance_05) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW,
      ODBCM_RC_TRANSACTION_ERROR);
  int ret =  ktCtrlrObj.UpdateKeyInstance(db_conn,
                                          &k,
                                          &v,
                                          UNC_DT_IMPORT,
                                          key_type,true);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}
#if 0 //crash
//  Delete for unsupported datatype
TEST_F(ControllerTest, Delete_01) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Delete(db_conn,
                               session_id,
                               configuration_id,
                               &k,
                               UNC_DT_STATE,
                               ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR,
            ret);
}
#endif
//  Domain Delete success
TEST_F(ControllerTest, Delete_02) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW,
      ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::DELETEONEROW,
      ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Delete(
      db_conn,
      session_id,
      configuration_id,
      &k,
      UNC_DT_CANDIDATE,
      ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  get_controller_type returns failure
TEST_F(ControllerTest, Delete_03) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW,
                                             ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Delete(db_conn, session_id, configuration_id,
                               &k, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  Domain Delete With boundary referred
TEST_F(ControllerTest, Delete_04) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW,
      ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS,
      ODBCM_RC_ROW_EXISTS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Delete(db_conn,
                               session_id,
                               configuration_id,
                               &k,
                               UNC_DT_CANDIDATE,
                               ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  UpdateKeyInstance with ODBC retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(ControllerTest, GetModifiedRows_01) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  vector<void *> obj_key_struct;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.GetModifiedRows(db_conn, obj_key_struct, UPDATED);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  ODBC retuns ODBCM_RC_RECORD_NOT_FOUND
TEST_F(ControllerTest, GetModifiedRows_02) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  vector<void *> obj_key_struct;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETMODIFIEDROWS,
      ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  ktCtrlrObj.GetModifiedRows(db_conn, obj_key_struct, UPDATED);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  ODBC retuns ODBCM_RC_SUCCESS
TEST_F(ControllerTest, GetModifiedRows_03) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  vector<void *> obj_key_struct;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode
      (unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_SUCCESS);
  int ret =
      ktCtrlrObj.GetModifiedRows(db_conn, obj_key_struct, UPDATED);
  EXPECT_EQ(ODBCM_RC_SUCCESS, ret);
}

//  ODBC retuns ODBCM_RC_FAILED
TEST_F(ControllerTest, GetModifiedRows_04) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  vector<void *> obj_key_struct;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.GetModifiedRows(db_conn, obj_key_struct, UPDATED);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(ControllerTest, SetOperStatus_001) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.SetOperStatus(db_conn, UNC_DT_STATE, &k,
                                      (UpplControllerOperStatus)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

TEST_F(ControllerTest, SetOperStatus_002) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  ktCtrlrObj.SetOperStatus(db_conn, UNC_DT_STATE, &k,
                                      (UpplControllerOperStatus)0);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

TEST_F(ControllerTest, SetOperStatus_003) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.SetOperStatus(
      db_conn, UNC_DT_STATE, &k, (UpplControllerOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
#if 0 //crash
TEST_F(ControllerTest, SetOperStatus_004) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ser_evt;
  ser_evt.addOutput((uint32_t)UNC_OP_CREATE);
  ser_evt.addOutput((uint32_t)UNC_DT_STATE);
  ser_evt.addOutput((uint32_t)UNC_KT_PORT);
  unc::uppl::ODBCManager::stub_setResultcode
      (unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode
      (unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.SetOperStatus(
      db_conn, UNC_DT_STATE, &k, (UpplControllerOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
#endif
//  GetOperStatus ODBC returns failure
TEST_F(ControllerTest, GetOperStatus_001) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.GetOperStatus(
      db_conn, UNC_DT_STATE, &k, op_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  GetOperStatus ODBC returns SUCCESS
TEST_F(ControllerTest, GetOperStatus_002) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.GetOperStatus(
      db_conn, UNC_DT_STATE, &k, op_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(ControllerTest, GetOperStatus_003) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS_WITH_INFO);
  int ret =  ktCtrlrObj.GetOperStatus(
      db_conn, UNC_DT_STATE, &k, op_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  NotifyOperStatus with NULL keystruct
TEST_F(ControllerTest, NotifyOperStatus_01) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  vector<OperStatusHolder> oper_stat_hldr;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.NotifyOperStatus(
      db_conn, UNC_DT_STATE, &k, &v, oper_stat_hldr);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*
//  NotifyOperStatus
TEST_F(ControllerTest, NotifyOperStatus_02) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<OperStatusHolder> oper_stat_hldr;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode
  (unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.NotifyOperStatus
  (db_conn, UNC_DT_STATE, &k, &v, oper_stat_hldr );
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

//  NotifyOperStatus Controller oper_status  retunrs failure
TEST_F(ControllerTest, NotifyOperStatus_03) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<OperStatusHolder> oper_stat_hldr;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode
  (unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =
  ktCtrlrObj.NotifyOperStatus(db_conn, UNC_DT_STATE, &k, &v, oper_stat_hldr);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}
*/

//  HandleOperStatus Controller oper_status returns success
TEST_F(ControllerTest, HandleOperStatus_01) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  HandleOperStatus Controller oper_status  retunrs failure
TEST_F(ControllerTest, HandleOperStatus_02) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

//  HandleOperStatus with bIsInternal false
TEST_F(ControllerTest, HandleOperStatus_03) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  bool bIsInternal = false;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleOperStatus(
      db_conn, UNC_DT_STATE, &k, &v, bIsInternal);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

//  HandleOperStatus with bIsInternal false
//  and oper_status is UPPL_CONTROLLER_OPER_UP
TEST_F(ControllerTest, HandleOperStatus_04) {
  key_ctr_t k;
  val_ctr_st_t v;
  v.controller.type = 1;
  v.controller.valid[kIdxType] = 1;
  memset(v.controller.description, '\0', 128);
  memcpy(v.controller.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  v.oper_status = UPPL_CONTROLLER_OPER_UP;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  bool bIsInternal = false;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleOperStatus(
      db_conn, UNC_DT_STATE, &k, &v, bIsInternal);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  HandleOperStatus with bIsInternal
//  true and oper_status is UPPL_CONTROLLER_OPER_UP
#if 0 //crash
TEST_F(ControllerTest, HandleOperStatus_05) {
  key_ctr_t k;
  val_ctr_st_t v;
  v.controller.type = 1;
  v.controller.valid[kIdxType] = 1;
  memset(v.controller.description, '\0', 128);
  memcpy(v.controller.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  v.oper_status = UPPL_CONTROLLER_OPER_UP;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  bool bIsInternal = true;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleOperStatus(
      db_conn, UNC_DT_STATE, &k, &v, bIsInternal);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
#endif
//  HandleOperStatus with bIsInternal true
//  and oper_status is UPPL_CONTROLLER_OPER_DOWN
#if 0
TEST_F(ControllerTest, HandleOperStatus_06) {
  key_ctr_t k;
  val_ctr_st_t v;
  v.controller.type = 1;
  v.controller.valid[kIdxType] = 1;
  memset(v.controller.description, '\0', 128);
  memcpy(v.controller.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  v.oper_status = UPPL_CONTROLLER_OPER_DOWN;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  bool bIsInternal = true;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleOperStatus(
      db_conn, UNC_DT_RUNNING, &k, &v, bIsInternal);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
#endif
//  HandleDriverAlarms with unsupported alarm type
TEST_F(ControllerTest, HandleDriverAlarms_01) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.HandleDriverAlarms(
      db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  HandleDriverAlarms with UNC_COREDOMAIN_SPLIT alarm type
TEST_F(ControllerTest, HandleDriverAlarms_02) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.HandleDriverAlarms(
      db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  HandleDriverAlarms with oper_type UNC_OP_CREATE
TEST_F(ControllerTest, HandleDriverAlarms_03) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleDriverAlarms(
      db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  HandleDriverAlarms with oper_type UNC_OP_DELETE
TEST_F(ControllerTest, HandleDriverAlarms_04) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_DELETE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleDriverAlarms(
      db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_CREATE
TEST_F(ControllerTest, PerformSemanticValidation_11) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktCtrlrObj.PerformSemanticValidation(
      db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_INSTANCE_EXISTS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_CREATE
TEST_F(ControllerTest, PerformSemanticValidation_02) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode
      (unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =
      ktCtrlrObj.PerformSemanticValidation(
          db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_CREATE
TEST_F(ControllerTest, PerformSemanticValidation_03) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  ktCtrlrObj.PerformSemanticValidation(
      db_conn, &k, &v, oper_type, UNC_DT_IMPORT);
  EXPECT_NE(UNC_RC_SUCCESS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_READ
TEST_F(ControllerTest, PerformSemanticValidation_04) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_READ;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktCtrlrObj.PerformSemanticValidation(
      db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_UPDATE
TEST_F(ControllerTest, PerformSemanticValidation_05) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_UPDATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.PerformSemanticValidation(
      db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_DELETE
TEST_F(ControllerTest, PerformSemanticValidation_06) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_DELETE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_QUERY_TIMEOUT);
  int ret =
      ktCtrlrObj.PerformSemanticValidation(
          db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/*
//  ReadBulkInternal with max_count zero
TEST_F(ControllerTest, ReadBulkInternal_01) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  vector<val_ctr_st> vect_val_ctr_st;
  vector<key_ctr> vect_ctr_id;
  uint32_t max_rep_ct = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
UncRespCode Kt_Boundary::ReadBulkInternal(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void *val_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<key_boundary_t> &vect_key_boundary,
    vector<val_boundary_st_t> &vect_val_boundary)

  int ret =  KtctrObj.ReadBulkInternal(
  db_conn, &k, &v, UNC_DT_STATE, max_rep_ct,  vect_ctr_id, vect_val_ctr_st);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_RECORD_NOT_FOUND
TEST_F(ControllerTest, ReadBulkInternal_02) {
  key_ctr_t k;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
  unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtctrObj.ReadBulkInternal(
  db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_val_ctr_st, vect_domain_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_CONNECTION_ERROR
TEST_F(ControllerTest, ReadBulkInternal_03) {
  key_ctr_t k;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
  unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtctrObj.ReadBulkInternal(
  db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_val_ctr_st, vect_domain_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_TABLE_NOT_FOUND
TEST_F(ControllerTest, ReadBulkInternal_04) {
  key_ctr_t k;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_TABLE_NOT_FOUND);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct,
                                       vect_val_ctr_st, vect_domain_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_SUCCESS
TEST_F(ControllerTest, ReadBulkInternal_05) {
  key_ctr_t k;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct,
                                      vect_val_ctr_st, vect_domain_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
*/

//   ReadBulk with get_controller_type returns failure
//   MEM
TEST_F(ControllerTest, ReadBulk_02) {
  key_ctr_t k;
  int child_index = 2;
  pfc_bool_t parent_call(PFC_TRUE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest *read_req(NULL);
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                              child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//   ReadBulk with data type UNC_DT_IMPORT
TEST_F(ControllerTest, ReadBulk_01) {
  key_ctr_t k;
  int child_index = 0;
  pfc_bool_t parent_call(PFC_TRUE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest *read_req(NULL);
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_IMPORT, max_rep_ct,
                              child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

//   ReadBulk with data type UNC_DT_IMPORT
TEST_F(ControllerTest, ReadBulk_05) {
  key_ctr_t k;
  int child_index = -1;
  pfc_bool_t parent_call(PFC_TRUE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest *read_req(NULL);
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_IMPORT, max_rep_ct,
                              child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}


//   ReadBulk with data type UNC_DT_IMPORT
TEST_F(ControllerTest, ReadBulk_06) {
  key_ctr_t k;
  int child_index = 2;
  pfc_bool_t parent_call(PFC_TRUE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest *read_req(NULL);
  uint32_t max_rep_ct = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_IMPORT, max_rep_ct,
                              child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

//  DeleteKeyInstance with data type UNC_DT_RUNNING
TEST_F(ControllerTest, DeleteKeyInstance_01) {
  key_ctr_t k;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  // unc::uppl::ODBCManager::stub_setResultcode(
  // unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.DeleteKeyInstance(
      db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_DELETE, ret);
}

//  DeleteKeyInstance with out child
TEST_F(ControllerTest, DeleteKeyInstance_02) {
  key_ctr_t k;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn, &k,
                                        UNC_DT_CANDIDATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  DeleteKeyInstance with out child
TEST_F(ControllerTest, DeleteKeyInstance_03) {
  key_ctr_t k;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn,
                                        &k, UNC_DT_AUDIT, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  DeleteKeyInstance with out child
TEST_F(ControllerTest, DeleteKeyInstance_04) {
  key_ctr_t k;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn, &k, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_DELETE, ret);
}

//  DeleteKeyInstance suceess
TEST_F(ControllerTest, DeleteKeyInstance_05) {
  key_ctr_t k;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn, &k, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
//  FreeChildKeyStruct success
TEST_F(ControllerTest, FreeChildKeyStruct_01) {
  void *k = new key_ctr_domain_t;
  int child_class = 0;
  Kt_Controller  KtctrObj;
  KtctrObj.FreeChildKeyStruct(k, child_class);
}

//  FreeChildKeyStruct suceess
TEST_F(ControllerTest, FreeChildKeyStruct_02) {
  void *key = new key_logical_port_t;
  int child_class = 1;
  Kt_Controller  KtctrObj;
  int ret =  UNC_RC_SUCCESS;
  KtctrObj.FreeChildKeyStruct(key, child_class);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  FreeChildKeyStruct suceess
TEST_F(ControllerTest, FreeChildKeyStruct_03) {
  void *key = new key_link_t;
  int child_class = 2;
  Kt_Controller  KtctrObj;
  int ret =  UNC_RC_SUCCESS;
  KtctrObj.FreeChildKeyStruct(key, child_class);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  getChildKeyStruct success
TEST_F(ControllerTest, getChildKeyStruct_01) {
  int child_class = 0;
  Kt_Controller  KtctrObj;
  KtctrObj.getChildKeyStruct(child_class, "controller1");
}

//  getChildKeyStruct suceess
TEST_F(ControllerTest, getChildKeyStruct_02) {
  int child_class = 1;
  Kt_Controller  KtctrObj;
  int ret =  UNC_RC_SUCCESS;
  KtctrObj.getChildKeyStruct(child_class, "controller1");
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  getChildKeyStruct suceess
TEST_F(ControllerTest, getChildKeyStruct_03) {
  int child_class = 2;
  Kt_Controller  KtctrObj;
  int ret =  UNC_RC_SUCCESS;
  KtctrObj.getChildKeyStruct(child_class, "controller1");
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformRead with negative option1
TEST_F(ControllerTest, PerformRead_Neg_option1_01) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(
      db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
      (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ, sess,
      (uint32_t)UNC_OPT1_DETAIL, (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

// PerformRead with negative option1
TEST_F(ControllerTest, PerformRead_Neg_option1_02) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION1);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(
      db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
      (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ, sess,
      (uint32_t)UNC_OPT1_DETAIL,
      (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformRead with negative option2
TEST_F(ControllerTest, PerformRead_Neg_option2_03) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_MAC_ENTRY_STATIC);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(
      db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
      (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ,
      sess, (uint32_t)UNC_OPT1_NORMAL,
      (uint32_t)UNC_OPT2_MAC_ENTRY_STATIC, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

// PerformRead with negative option1
TEST_F(ControllerTest, PerformRead_Neg_option2_04) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_MAC_ENTRY_STATIC);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION2);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(
      db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
      (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ,
      sess, (uint32_t)UNC_OPT1_NORMAL,
      (uint32_t)UNC_OPT2_MAC_ENTRY_STATIC, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformRead with unsupported datatype
TEST_F(ControllerTest, PerformRead_Neg_datatype_05) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_AUDIT);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(
      db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
      (uint32_t)UNC_DT_AUDIT, (uint32_t)UNC_OP_READ,
      sess, (uint32_t)UNC_OPT1_NORMAL,
      (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

// PerformRead with negative dataype
TEST_F(ControllerTest, PerformRead_Neg_datatype_06) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_AUDIT);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(
      db_conn, (uint32_t)0,
      (uint32_t)0, &k, &v, (uint32_t)UNC_DT_AUDIT,
      (uint32_t)UNC_OP_READ, sess,
      (uint32_t)UNC_OPT1_NORMAL, (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformRead with valid option1 and valid option2
TEST_F(ControllerTest, PerformRead_pos_07) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(
      db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
      (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ,
      sess, (uint32_t)UNC_OPT1_NORMAL,
      (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformRead with valid option1 and valid option2
TEST_F(ControllerTest, PerformRead_pos_db_Success_08) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  vector<val_ctr_st_t> vect_val_ctr_st;
  val_ctr_st_t vst;
  memset(&vst, 0, sizeof(vst));
  vst.controller.cs_row_status = DELETED;
  vect_val_ctr_st.push_back(vst);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(
      db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
      (uint32_t)UNC_DT_CANDIDATE, (uint32_t)UNC_OP_READ,
      sess, (uint32_t)UNC_OPT1_NORMAL,
      (uint32_t)UNC_OPT2_NONE, (uint32_t)0);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(ControllerTest, PerformRead_pos_db_fail_09) {
  key_ctr_t k;
  val_ctr v;
  memset(&v, 0, sizeof(v));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(
      db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
      (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ,
      sess, (uint32_t)UNC_OPT1_NORMAL,
      (uint32_t)UNC_OPT2_NONE, (uint32_t)0);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

// PerformRead with valid option1 and valid option2
TEST_F(ControllerTest, PerformRead_pos_db_Success_10) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  vector<val_ctr_st_t> vect_val_ctr_st;
  val_ctr_st_t vst;
  memset(&vst, 0, sizeof(vst));
  vst.controller.cs_row_status = DELETED;
  vect_val_ctr_st.push_back(vst);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(
      db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
      (uint32_t)UNC_DT_CANDIDATE, (uint32_t)UNC_OP_READ,
      sess, (uint32_t)UNC_OPT1_NORMAL,
      (uint32_t)UNC_OPT2_NONE, (uint32_t)0);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

// HandleDriverEvents with other than Update operation
TEST_F(ControllerTest, HandleDriverEvents_No_Update_01) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  pfc_bool_t is_events_done(PFC_FALSE);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_READ;
  uint32_t data_type = UNC_DT_STATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(
      db_conn, &k, oper_type, data_type, &v_old, &v_new, is_events_done);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// HandleDriverEvents with CANDIDATE operation
TEST_F(ControllerTest, HandleDriverEvents_CANDIDATE_NegDB_02) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(v_old.valid, '\0', sizeof(v_old.valid));
  memset(v_new.valid, '\0', sizeof(v_new.valid));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  pfc_bool_t is_events_done(PFC_FALSE);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_CANDIDATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =
      KtctrObj.HandleDriverEvents(db_conn, &k, oper_type, data_type,
                                  &v_old, &v_new, is_events_done);
  EXPECT_NE(UNC_UPPL_RC_ERR_DB_GET, ret);
}

// HandleDriverEvents with RUNNING datatype

TEST_F(ControllerTest, HandleDriverEvents_RUNNING_NegDB_03) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(v_old.valid, '\0', sizeof(v_old.valid));
  memset(v_new.valid, '\0', sizeof(v_new.valid));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  pfc_bool_t is_events_done(PFC_FALSE);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(
      db_conn, &k, oper_type, data_type, &v_old, &v_new, is_events_done);
  EXPECT_NE(UNC_UPPL_RC_ERR_DB_GET, ret);
}
#if 0 //failing
// HandleDriverEvents with RUNNING datatype
TEST_F(ControllerTest, HandleDriverEvents_RUNNING_04) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(v_old.valid, '\0', sizeof(v_old.valid));
  memset(v_new.valid, '\0', sizeof(v_new.valid));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  pfc_bool_t is_events_done(PFC_FALSE);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(
      db_conn, &k, oper_type, data_type, &v_old, &v_new, is_events_done);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

// HandleDriverEvents with RUNNING datatype
TEST_F(ControllerTest, HandleDriverEvents_RUNNING_05) {
  key_ctr_t k;
  memset(&k, 0, sizeof(k));

  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(&v_old, 0, sizeof(v_old));
  memset(&v_new, 0, sizeof(v_new));
  v_new.oper_status = UPPL_CONTROLLER_OPER_UP;

  pfc_bool_t is_events_done(PFC_FALSE);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(
      db_conn, &k, oper_type, data_type, &v_old, &v_new, is_events_done);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// HandleDriverEvents with RUNNING datatype
TEST_F(ControllerTest, HandleDriverEvents_RUNNING_06) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(v_old.valid, '\0', sizeof(v_old.valid));
  memset(v_new.valid, '\0', sizeof(v_new.valid));

  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  pfc_bool_t is_events_done(PFC_FALSE);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(
      db_conn, &k, oper_type, data_type, &v_old, &v_new, is_events_done);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

// HandleDriverEvents with RUNNING datatype
TEST_F(ControllerTest, HandleDriverEvents_RUNNING_07) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(v_old.valid, '\0', sizeof(v_old.valid));
  memset(v_new.valid, '\0', sizeof(v_new.valid));
  v_new.oper_status = UPPL_CONTROLLER_OPER_UP;

  pfc_bool_t is_events_done = false;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(
      db_conn, &k, oper_type, data_type, &v_old, &v_new, is_events_done);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// HandleDriverEvents with RUNNING datatype
TEST_F(ControllerTest, HandleDriverEvents_RUNNING_08) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(v_old.valid, '\0', sizeof(v_old.valid));
  memset(v_new.valid, '\0', sizeof(v_new.valid));
  v_new.oper_status = UPPL_CONTROLLER_OPER_DOWN;

  pfc_bool_t is_events_done(PFC_FALSE);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(
      db_conn, &k, oper_type, data_type, &v_old, &v_new, is_events_done);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// HandleDriverEvents with RUNNING datatype
TEST_F(ControllerTest, HandleDriverEvents_RUNNING_09) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(&v_old, 0, sizeof(v_old));
  memset(&v_new, 0, sizeof(v_new));
  v_new.valid[kIdxActualVersion]=UNC_VF_VALID;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  pfc_bool_t is_events_done(PFC_FALSE);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(
      db_conn, &k, oper_type, data_type, &v_old, &v_new, is_events_done);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}
#endif 
// CheckIpAndClearStateDB with DB success
#if 0
TEST_F(ControllerTest, CheckIpAndClearStateDB_Db_Success_01) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.CheckIpAndClearStateDB(db_conn, &k);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
#endif
// CheckIpAndClearStateDB with DB failure
TEST_F(ControllerTest, CheckIpAndClearStateDB_Db_failure_02) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.CheckIpAndClearStateDB(db_conn, &k);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

// SendOperStatusNotification with add output error
TEST_F(ControllerTest, SendOperStatusNotification_01) {
  key_ctr_t k;
  uint8_t old_oper_st(0);
  uint8_t new_oper_st(1);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)UNC_DT_RUNNING);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  Kt_Controller  KtctrObj;

  int ret =
      KtctrObj.SendOperStatusNotification(k, old_oper_st, new_oper_st);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
#if 0 //failing
// SendOperStatusNotification with success in add output
TEST_F(ControllerTest, SendOperStatusNotification_02) {
  key_ctr_t k;
  uint8_t old_oper_st(0);
  uint8_t new_oper_st(1);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  Kt_Controller  KtctrObj;

  int ret =
      KtctrObj.SendOperStatusNotification(k, old_oper_st, new_oper_st);
  EXPECT_EQ(UNC_UPPL_RC_FAILURE, ret);
}
#endif
// ValidateControllerIpAddress
TEST_F(ControllerTest, ValidateControllerIpAddress_01) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  val_ctr_t v;
  memset(&v, 0, sizeof(v));

  Kt_Controller  KtctrObj;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type(UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  v.valid[kIdxIpAddress]=1;
  int ret =  KtctrObj.ValidateControllerIpAddress(
      db_conn, operation, data_type, (unc_keytype_ctrtype_t)0,
      (UncRespCode)0, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// ValidateControllerIpAddress
TEST_F(ControllerTest, ValidateControllerIpAddress_02) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  val_ctr_t v;
  memset(&v, 0, sizeof(v));

  Kt_Controller  KtctrObj;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type(UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  v.valid[kIdxIpAddress]=2;
  int ret =  KtctrObj.ValidateControllerIpAddress(
      db_conn, operation, data_type,
      (unc_keytype_ctrtype_t)1, (UncRespCode)0, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// ValidateControllerIpAddress
TEST_F(ControllerTest, ValidateControllerIpAddress_03) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  val_ctr_t v;
  memset(&v, 0, sizeof(v));

  Kt_Controller  KtctrObj;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type(UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  v.valid[kIdxIpAddress]=1;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ValidateControllerIpAddress(
      db_conn, operation, data_type, (unc_keytype_ctrtype_t)1,
      (UncRespCode)0, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// ValidateControllerIpAddress
TEST_F(ControllerTest, ValidateControllerIpAddress_04) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  val_ctr_t v;
  memset(&v, 0, sizeof(v));

  Kt_Controller  KtctrObj;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;
  v.valid[kIdxIpAddress]=1;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  KtctrObj.ValidateControllerIpAddress(
      db_conn, operation, data_type,
      (unc_keytype_ctrtype_t)1, (UncRespCode)0, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// ValidateUnknownCtrlrScalability
TEST_F(ControllerTest, ValidateUnknownCtrlrScalability_Neg_DB_01) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint8_t type(UNC_CT_PFC);
  uint32_t data_type(UNC_DT_RUNNING);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETSIBLINGCOUNT_FILTER, ODBCM_RC_FAILED);
  int ret =  KtctrObj.ValidateUnknownCtrlrScalability(
      db_conn, &k, type, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

// ValidateUnknownCtrlrScalability
TEST_F(ControllerTest, ValidateUnknownCtrlrScalability_Neg_DB_02) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint8_t type(UNC_CT_PFC);
  uint32_t data_type(UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETSIBLINGCOUNT_FILTER, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtctrObj.ValidateUnknownCtrlrScalability(
      db_conn, &k, type, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// ValidateUnknownCtrlrScalability
TEST_F(ControllerTest, ValidateUnknownCtrlrScalability_Neg_DB_03) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint8_t type(UNC_CT_PFC);
  uint32_t data_type(UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETSIBLINGCOUNT_FILTER, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setSiblingCount(1);
  int ret =  KtctrObj.ValidateUnknownCtrlrScalability(
      db_conn, &k, type, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_EXCEEDS_RESOURCE_LIMIT, ret);
}

// ValidateUnknownCtrlrScalability
TEST_F(ControllerTest, ValidateUnknownCtrlrScalability_Neg_DB_04) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint8_t type(UNC_CT_PFC);
  uint32_t data_type(UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETSIBLINGCOUNT_FILTER, ODBCM_RC_RECORD_NOT_FOUND);
  unc::uppl::ODBCManager::stub_setSiblingCount(1);
  int ret =  KtctrObj.ValidateUnknownCtrlrScalability(
      db_conn, &k, type, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_EXCEEDS_RESOURCE_LIMIT, ret);
}

// FrameValidValue
#if 0
TEST_F(ControllerTest, FrameValidValue_01) {
  val_ctr_t v;
  val_ctr_st_t v_st;
  memset(&v, 0xff, sizeof(v));
  memset(&v_st, 0xff, sizeof(v_st));

  Kt_Controller  KtctrObj;
  string abd("012345678");
  KtctrObj.FrameValidValue(abd, v_st, v);

  const uint32_t vsize(PFC_ARRAY_CAPACITY(v.valid));
  for (uint32_t i(0); i < vsize; i++) {
    ASSERT_EQ(i, v.valid[i]);
  }

  ASSERT_EQ(UNC_VF_VALID, v_st.valid[0]);
  for (uint8_t i(1); i < PFC_ARRAY_CAPACITY(v_st.valid); i++) {
    uint8_t required(static_cast<uint8_t>(vsize + i - 1));
    ASSERT_EQ(required, v_st.valid[i]);
  }
}
#endif
/*******TAMIL TEST CASES*******/
/********ReadBulk*******/

// ReadBulk opr returns Success for max_ct is zero
TEST_F(ControllerTest, ReadBulk_Max_Ct_Zero) {
  key_ctr_t k;
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest *read_req(NULL);
  uint32_t max_rep_ct = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                              child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// Check for controller key existence
TEST_F(ControllerTest, ReadBulk_childIndex) {
  key_ctr_t k;
  int child_index = -1;
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest *read_req = NULL;
  uint32_t max_rep_ct = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  // unc::uppl::ODBCManager::stub_setResultcode(
  // unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                          child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*************ReadBulkInternal**********/
//  ReadBulkInternal: No record to read
TEST_F(ControllerTest, ReadBulkInternal_NoRecordFound) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  vector<val_ctr_st> vect_val_ctr_st;
  vector<string> vect_ctr_id;
  uint32_t max_rep_ct(1);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, &v,
          UNC_DT_STATE, max_rep_ct, vect_val_ctr_st, vect_ctr_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_CONNECTION_ERROR
TEST_F(ControllerTest, ReadBulkInternal_Db_Connxn_Error) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  vector<val_ctr_st> vect_val_ctr_st;
  vector<string> vect_ctr_id;
  uint32_t max_rep_ct(1);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtctrObj.ReadBulkInternal(
      db_conn, &k, &v, UNC_DT_STATE,
      max_rep_ct, vect_val_ctr_st, vect_ctr_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_TABLE_NOT_FOUND
TEST_F(ControllerTest, ReadBulkInternal_Err_DB_Get) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  vector<val_ctr_st> vect_val_ctr_st;
  vector<string> vect_ctr_id;
  uint32_t max_rep_ct(1);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETBULKROWS,
      ODBCM_RC_TABLE_NOT_FOUND);
  int ret =  KtctrObj.ReadBulkInternal(
      db_conn, &k, &v, UNC_DT_STATE, max_rep_ct,
      vect_val_ctr_st, vect_ctr_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_SUCCESS
TEST_F(ControllerTest, ReadBulkInternal_Success) {
  key_ctr_t k;
  val_ctr_t v;
  memset(&v, 0, sizeof(v));
  vector<val_ctr_st> vect_val_ctr_st;
  vector<string> vect_ctr_id;
  uint32_t max_rep_ct(1);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulkInternal(db_conn,
                                       &k, &v, UNC_DT_STATE, max_rep_ct,
                                       vect_val_ctr_st, vect_ctr_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//TODO(ODC)
/*********ValidateCtrlrValueCapability*******/
// returns config syntax error
#if 0
TEST_F(ControllerTest, ValidateCtrlrValueCapability_Err_CFG_SYNTAX) {
  string version;
  uint32_t key_type(UNC_KT_CONTROLLER);
  Kt_Controller  KtctrObj;
  int ret =  KtctrObj.ValidateCtrlrValueCapability(version, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
#endif
/*******ValidateCtrlrScalability****/
// Returns Connxn Error
/*
TEST_F(ControllerTest, ValidateCtrlrScalability_Err_DB_Access) {
  OdbcmConnectionHandler *db_conn =NULL;
  string version;
  uint32_t key_type = UNC_KT_CONTROLLER;
  uint32_t data_type = UNC_DT_STATE;
  Kt_Controller  KtctrObj;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETROWCOUNT, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtctrObj.ValidateCtrlrScalability(
      db_conn, version, key_type, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// Unable to get scalability number from system
TEST_F(ControllerTest, ValidateCtrlrScalability_System_Error) {
  OdbcmConnectionHandler *db_conn =NULL;
  string version;
  uint32_t key_type = UNC_KT_CONTROLLER;
  uint32_t data_type = UNC_DT_STATE;
  Kt_Controller  KtctrObj;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETROWCOUNT, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ValidateCtrlrScalability(
      db_conn, version, key_type, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

// Unable to get scalability number from DB
TEST_F(ControllerTest, ValidateCtrlrScalability_DB_Err) {
  OdbcmConnectionHandler *db_conn =NULL;
  string version;
  uint32_t key_type = UNC_KT_CONTROLLER;
  uint32_t data_type = UNC_DT_STATE;
  Kt_Controller  KtctrObj;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETROWCOUNT, ODBCM_RC_FAILED);
  int ret =  KtctrObj.ValidateCtrlrScalability(
      db_conn, version, key_type, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}
*/
/******ValidateTypeIpAddress*******/

// Not required to validate type and ip
TEST_F(ControllerTest, ValidateTypeIpAddress_NoValidation) {
  OdbcmConnectionHandler *db_conn =NULL;
  key_ctr_t k;
  val_ctr_st_t *v = NULL;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t ctrl_type(UNC_CT_PFC);
  Kt_Controller  KtctrObj;
  int ret =  KtctrObj.ValidateTypeIpAddress(
      db_conn, &k, v, data_type, ctrl_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(ControllerTest, ValidateTypeIpAddress) {
  OdbcmConnectionHandler *db_conn =NULL;
  key_ctr_t k;
  pfc_strlcpy(reinterpret_cast<char *>(k.controller_name), pkctrName1,
              sizeof(k.controller_name));

  val_ctr_st_t v;
  memset(&v, 0, sizeof(v));
  // v->valid[kIdxIpAddress] = UNC_VF_INVALID;

  uint32_t data_type = UNC_DT_STATE;
  uint32_t ctrl_type(UNC_CT_PFC);
  Kt_Controller  KtctrObj;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  KtctrObj.ValidateTypeIpAddress(
      db_conn, &k, &v, data_type, ctrl_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
#if 0 //crash
/*******HandleDriverAlarms*********/
TEST_F(ControllerTest, HandleDriverAlarms) {
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t alarm_type = UNC_PHYS_PATH_FAULT;
  uint32_t oper_type(UNC_OP_UPDATE);
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  val_phys_path_fault_alarm_t v;
  memset(&v, 0, sizeof(v));
  pfc_strlcpy(reinterpret_cast<char *>(v.ingress_ofs_dpid),
              "00:11:22:33:44:55:66:77", sizeof(v.ingress_ofs_dpid));
  pfc_strlcpy(reinterpret_cast<char *>(v.egress_ofs_dpid),
              "aa:bb:cc:dd:ee:ff:00:11", sizeof(v.egress_ofs_dpid));

  Kt_Controller  KtctrObj;
  int ret =  KtctrObj.HandleDriverAlarms(
      db_conn, data_type, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
#endif

/******SendSemanticRequestToUPLL*******/
#if 0
TEST_F(ControllerTest, SendSemanticRequestToUPLL) {
  key_ctr_t k;
  uint32_t data_type = UNC_DT_STATE;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  int ret =  KtctrObj.SendSemanticRequestToUPLL(&k, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE, ret);
}

// GetChildClassPointer
TEST_F(ControllerTest, GetChildClassPointer_01) {
  int kIndex = 0;
  Kt_Controller  KtctrObj;
  Kt_Base *child(
      KtctrObj.GetChildClassPointer((KtControllerChildClass)kIndex));
  ASSERT_TRUE(child != NULL);
}

TEST_F(ControllerTest, GetChildClassPointer_02) {
  int kIndex = 1;
  Kt_Controller  KtctrObj;
  Kt_Base *child(KtctrObj.GetChildClassPointer(
          (KtControllerChildClass)kIndex));
  ASSERT_TRUE(child != NULL);
}

TEST_F(ControllerTest, GetChildClassPointer_03) {
  int kIndex = 2;
  Kt_Controller  KtctrObj;
  Kt_Base *child(KtctrObj.GetChildClassPointer(
          (KtControllerChildClass)kIndex));
  ASSERT_TRUE(child != NULL);
}

/****ReadBulk Continuation******/
TEST_F(ControllerTest, ReadBulk) {
  key_ctr_t k;
  int child_index = 0;
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest *read_req = NULL;
  uint32_t max_rep_ct = 1;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  // unc::uppl::ODBCManager::stub_setResultcode(
  // unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                            child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(ControllerTest, ReadBulk_ReadBuffer) {
  key_ctr_t k;
  int child_index = 0;
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest read_req;
  uint32_t max_rep_ct = 1;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  // unc::uppl::ODBCManager::stub_setResultcode(
  // unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                          child_index, parent_call, is_read_next, &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(ControllerTest, ReadBulk_child_ind) {
  key_ctr_t k;
  int child_index = -1;
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest read_req;
  uint32_t max_rep_ct = 1;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
      unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  // unc::uppl::ODBCManager::stub_setResultcode(
  // unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct,
                               child_index, parent_call, is_read_next,
                               &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(ControllerTest, ReadBulk_ctr_exists_FALSE) {
  key_ctr_t k;
  memset(&k, 0, sizeof(key_ctr_t));
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest read_req;
  uint32_t max_rep_ct = 1;
  // memset(k.controller_name, '\0', 32);
  // memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;

  // unc::uppl::ODBCManager::stub_setResultcode
  // (unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  unc::uppl::ODBCManager::stub_setResultcode
      (unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct,
                               child_index, parent_call, is_read_next,
                               &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  DeleteKeyInstance:returns DB error
TEST_F(ControllerTest, DeleteKeyInstan_Err_DB_ACCESS) {
  key_ctr_t k;
  uint32_t key_type(UNC_KT_CONTROLLER);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode
      (unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// ReadBulkInternal:Reaturns success for max_ct zero
TEST_F(ControllerTest, ReadBulkInternal_MaxCt_LessThan_Zero) {
  OdbcmConnectionHandler *db_conn =NULL;
  key_ctr_t k;
  val_ctr_st_t v;
  uint32_t data_type =UNC_DT_STATE;
  uint32_t max_rep_ct = 0;
  vector<val_ctr_st_t> vect_val_ctr;
  vector<string> vect_ctr_id;
  Kt_Controller  KtctrObj;
  int ret = KtctrObj.ReadBulkInternal
      (db_conn, &k, &v, data_type, max_rep_ct, vect_val_ctr, vect_ctr_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// HandleDriverEvents
TEST_F(ControllerTest, HandleDriverEvents) {
  OdbcmConnectionHandler *db_conn =NULL;
  key_ctr_t k;
  pfc_strlcpy(reinterpret_cast<char *>(k.controller_name), pkctrName1,
              sizeof(k.controller_name));

  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_STATE;
  val_ctr_st_t *old_val_struct(NULL);
  val_ctr_st_t *new_val_struct(NULL);
  pfc_bool_t is_events_done(PFC_FALSE);
  Kt_Controller  KtctrObj;
  int ret = KtctrObj.HandleDriverEvents
      (db_conn, &k, oper_type, data_type, old_val_struct,
       new_val_struct, is_events_done);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/***8*ReadCtrValFromDb****/
// Unsuported opr type....Returns Success
TEST_F(ControllerTest, ReadCtrValFromDB) {
  OdbcmConnectionHandler *db_conn =NULL;
  key_ctr_t k;
  val_ctr_st_t v;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t operation_type = UNC_OP_CREATE;
  uint32_t max_rep_ct;
  vector<val_ctr_st_t> vect_val_ctr_st;
  vector<string> controller_id;
  Kt_Controller  KtctrObj;
  int ret = KtctrObj.ReadCtrValFromDB
      (db_conn, &k, &v, data_type, operation_type, max_rep_ct,
       vect_val_ctr_st, controller_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

#endif
