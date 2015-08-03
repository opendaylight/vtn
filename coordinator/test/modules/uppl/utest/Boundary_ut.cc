/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include <odbcm_mgr.hh>
#include <itc_read_request.hh>
#include <tclib_module.hh>
#include <physical_common_def.hh>
#include <unc/uppl_common.h>
#include <unc/keytype.h>
#include <unc/upll_svc.h>
#include <itc_kt_boundary.hh>
#include <itc_kt_controller.hh>
#include <itc_kt_ctr_domain.hh>
#include <itc_kt_switch.hh>
#include <itc_kt_port.hh>
#include <itc_kt_link.hh>
#include <ipct_util.hh>
#include "PhysicalLayerStub.hh"
#include "ut_util.hh"

using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
using namespace std;
using namespace unc::tclib;
using namespace unc::uppl::test;

class BoundaryTest
  : public UpplTestEnv {
};

static char pkName1[]  =  "Boundary1";

static void getKeyForKtBoundary0(key_boundary_t& k) {
  memset(k.boundary_id, '\0', 32);
}

static void getKeyForKtBoundary1(key_boundary_t& k) {
  memset(k.boundary_id, '\0', 32);
  memcpy(k.boundary_id, pkName1, strlen(pkName1));
}

static void getValForKtBoundary1(val_boundary_t& v) {
  memset(v.description, '\0', 128);
  memcpy(v.description, "boundary description", strlen("boundary description"));

  memset(v.controller_name1, '\0', 32);
  memcpy(v.controller_name1, "Controller5", strlen("Controller5"));

  memset(v.domain_name1, '\0', 32);
  memcpy(v.domain_name1, "domain1", strlen("domain1"));

  memset(v.logical_port_id1, '\0', 320);
  memcpy(v.logical_port_id1, "{0xab, 0xc}", strlen("{0xab, 0xc}"));

  memset(v.controller_name2, '\0', 32);
  memcpy(v.controller_name2, "Controller7", strlen("Controller7"));

  memset(v.domain_name2, '\0', 32);
  memcpy(v.domain_name2, "domain2", strlen("domain2"));

  memset(v.logical_port_id2, '\0', 320);
  memcpy(v.logical_port_id2, "{0xcd, 0xe}", strlen("{0xcd, 0xe}"));

  memset(v.valid, 1, 7);  //  uint8_t valid[7];
  v.cs_row_status  =  0;  //  uint8_t cs_row_status;
  memset(v.cs_attr, 1, 7);  //  uint8_t cs_attr[7]
  v.cs_attr[6] = '\0';
}

static void getValForKtBoundary5(val_boundary_t& v) {
  memset(v.description, '\0', 128);
  memcpy(v.description, "boundary description", strlen("boundary description"));

  memset(v.controller_name1, '\0', 32);
  memcpy(v.controller_name1, "Controller5", strlen("Controller5"));

  memset(v.domain_name1, '\0', 32);
  memcpy(v.domain_name1, "domain1", strlen("domain1"));

  memset(v.logical_port_id1, '\0', 320);
  memcpy(v.logical_port_id1, "{0xab, 0xc}", strlen("{0xab, 0xc}"));

  memset(v.controller_name2, '\0', 32);
  memcpy(v.controller_name2, "Controller5", strlen("Controller5"));

  memset(v.domain_name2, '\0', 32);
  memcpy(v.domain_name2, "domain1", strlen("domain1"));

  memset(v.logical_port_id2, '\0', 320);
  memcpy(v.logical_port_id2, "{0xcd, 0xe}", strlen("{0xcd, 0xe}"));

  memset(v.valid, 1, 7);  //  uint8_t valid[7];
  v.cs_row_status  =  0;  //  uint8_t cs_row_status;
  memset(v.cs_attr, 1, 7);  //  uint8_t cs_attr[7]
  v.cs_attr[6] = '\0';
}

//  Create for unsupported datatype
TEST_F(BoundaryTest, Create_01) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn  = NULL;
  pfc::core::ipc::ServerSession::set_rest(2);
  int ret = KtboundaryObj.Create(
    db_conn, session_id, configuration_id, &k, &v, UNC_DT_STATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  Boundary  Create success
TEST_F(BoundaryTest, Create_02) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Create(
    db_conn, session_id, configuration_id, &k, &v, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  Boundary  Create Server session addOutput failed
TEST_F(BoundaryTest, Create_03) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Create(
    db_conn, session_id, configuration_id, &k, &v, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  CreateOneRow returns ODBCM_RC_CONNECTION_ERROR
TEST_F(BoundaryTest, CreateKeyInstance_01) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t key_type  =  1;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.CreateKeyInstance(
    db_conn, &k, &v, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  CreateOneRow returns ODBCM_RC_PKEY_VIOLATION
TEST_F(BoundaryTest, CreateKeyInstance_02) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t key_type  =  1;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_PKEY_VIOLATION);
  int ret = KtboundaryObj.CreateKeyInstance(
    db_conn, &k, &v, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_INSTANCE_EXISTS, ret);
}

//  CreateOneRow returns ODBCM_RC_QUERY_FAILED
TEST_F(BoundaryTest, CreateKeyInstance_03) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t key_type  =  1;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_QUERY_FAILED);
  int ret = KtboundaryObj.CreateKeyInstance(
    db_conn, &k, &v, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_CREATE, ret);
}

//  CreateOneRow returns ODBCM_RC_QUERY_FAILED
TEST_F(BoundaryTest, CreateKeyInstance_04) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t key_type  =  1;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.CreateKeyInstance(
    db_conn, &k, &v, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  Update for unsupported datatype
TEST_F(BoundaryTest, Update_01) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Update(
    db_conn, session_id, configuration_id, &k, &v, UNC_DT_STATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  Boundary  Update success
TEST_F(BoundaryTest, Update_02) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Update(
    db_conn, session_id, configuration_id, &k, &v, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  Boundary  Update Server session addOutput failed
TEST_F(BoundaryTest, Update_03) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Update(
    db_conn, session_id, configuration_id, &k, &v, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  UpdateOneRow returns ODBCM_RC_CONNECTION_ERROR
TEST_F(BoundaryTest, Update_04) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_DB_ACCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.Update(
    db_conn, session_id, configuration_id, &k, &v, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  UpdateOneRow returns ODBCM_RC_PKEY_VIOLATION
TEST_F(BoundaryTest, Update_05) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_DB_UPDATE);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_PKEY_VIOLATION);
  int ret = KtboundaryObj.Update(
    db_conn, session_id, configuration_id, &k, &v, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  ReadBulkInternal GetBulkRows returns ODBCM_RC_RECORD_NOT_FOUND
TEST_F(BoundaryTest, ReadBulkInternal_01) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtboundaryObj.ReadBulkInternal(
    db_conn, &k, &v, UNC_DT_STATE, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  ReadBulkInternal GetBulkRows returns ODBCM_RC_CONNECTION_ERROR
TEST_F(BoundaryTest, ReadBulkInternal_02) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.ReadBulkInternal(
    db_conn, &k, &v, UNC_DT_STATE, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  ReadBulkInternal GetBulkRows returns ODBCM_RC_QUERY_FAILED
TEST_F(BoundaryTest, ReadBulkInternal_03) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_QUERY_FAILED);
  int ret = KtboundaryObj.ReadBulkInternal(
    db_conn, &k, &v, UNC_DT_STATE, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  ReadBulkInternal GetBulkRows returns ODBCM_RC_SUCCESS
TEST_F(BoundaryTest, ReadBulkInternal_04) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.ReadBulkInternal(
    db_conn, &k, &v, UNC_DT_STATE, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  Boundary  Delete on unsupported data type server sessio failed
TEST_F(BoundaryTest, Delete_01) {
  key_boundary_t k;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_RUNNING);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Delete(
    db_conn, session_id,
                                 configuration_id, &k, UNC_DT_RUNNING, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  Boundary  Delete on unsupported data type server session pass
TEST_F(BoundaryTest, Delete_02) {
  key_boundary_t k;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  ses.stub_setAddOutput((uint32_t)UNC_DT_RUNNING);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Delete(
    db_conn, session_id,
                                 configuration_id, &k, UNC_DT_RUNNING, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

#if 0
//  DeleteOneRow returns ODBCM_RC_SUCCESS
TEST_F(BoundaryTest, Delete_03) {
  key_boundary_t k;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UPLL_IS_KEY_TYPE_IN_USE_OP);
  ses.stub_setAddOutput((uint32_t)UNC_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Delete(
    db_conn, session_id, configuration_id, &k, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  DeleteOneRow returns ODBCM_RC_CONNECTION_ERROR
TEST_F(BoundaryTest, Delete_04) {
  key_boundary_t k;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_DB_ACCESS);
  ses.stub_setAddOutput((uint32_t)UPLL_IS_KEY_TYPE_IN_USE_OP);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Delete(
    db_conn, session_id, configuration_id, &k, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  DeleteOneRow returns ODBCM_RC_ROW_NOT_EXISTS
TEST_F(BoundaryTest, Delete_05) {
  key_boundary_t k;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Delete(
    db_conn, session_id, configuration_id, &k, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  DeleteOneRow returns ODBCM_RC_FAILED
TEST_F(BoundaryTest, Delete_06) {
  key_boundary_t k;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_DB_DELETE);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_FAILED);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Delete(
    db_conn, session_id, configuration_id, &k, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

#endif

//  DeleteOneRow returns ODBCM_RC_FAILED
TEST_F(BoundaryTest, Delete_07) {
  key_boundary_t k;
  uint32_t session_id  =  1;
  uint32_t configuration_id  =  2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UNC_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_FAILED);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.Delete(
    db_conn, session_id, configuration_id, &k, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  ReadInternal success
TEST_F(BoundaryTest, ReadInternal_01) {
  key_boundary_t k;
  val_boundary_st_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v.boundary);
  vector<void *> boundary_key;
  vector<void *> boundary_val;
  boundary_key.push_back(&k);
  boundary_val.push_back(&v);
  uint32_t operation_type  =  UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.ReadInternal(
    db_conn, boundary_key, boundary_val, UNC_DT_STATE, operation_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  ReadInternal operation_type ! =  UNC_OP_READ
TEST_F(BoundaryTest, ReadInternal_02) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  vector<void *> boundary_key;
  vector<void *> boundary_val;
  boundary_key.push_back(&k);
  boundary_val.push_back(&v);
  uint32_t operation_type  =  UNC_OP_CREATE;
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.ReadInternal(
    db_conn, boundary_key, boundary_val, UNC_DT_STATE, operation_type);
  EXPECT_EQ(2008, ret);
}

//  ReadBoundaryValFromDB returns failure
TEST_F(BoundaryTest, ReadInternal_03) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  vector<void *> boundary_key;
  vector<void *> boundary_val;
  boundary_key.push_back(&k);
  boundary_val.push_back(&v);
  uint32_t operation_type  =  UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.ReadInternal(
    db_conn, boundary_key, boundary_val, UNC_DT_STATE, operation_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  ReadBulk
TEST_F(BoundaryTest, ReadBulk_01) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t max_rep_ct  =  2;
  int child_index  =  1;
  ReadRequest *read_req   =  NULL;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.ReadBulk(
    db_conn, &k, UNC_DT_IMPORT, max_rep_ct,
    child_index,
    (pfc_bool_t)false, (pfc_bool_t)false, read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

//  ReadBulk
TEST_F(BoundaryTest, ReadBulk_02) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t max_rep_ct  =  0;
  int child_index  =  1;
  ReadRequest *read_req   =  NULL;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.ReadBulk(
    db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
    child_index,
    (pfc_bool_t)false, (pfc_bool_t)false, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  ReadBulk
TEST_F(BoundaryTest, ReadBulk_03) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t max_rep_ct  =  1;
  int child_index  =  1;
  ReadRequest read_req;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.ReadBulk(
    db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct, child_index,
    (pfc_bool_t)false, (pfc_bool_t)false, &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  ReadBulk
TEST_F(BoundaryTest, ReadBulk_04) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t max_rep_ct  =  2;
  int child_index  =  1;
  ReadRequest read_req;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.ReadBulk(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index,
    (pfc_bool_t)false, (pfc_bool_t)false, &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  ReadBulk
TEST_F(BoundaryTest, ReadBulk_05) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t max_rep_ct  =  1;
  int child_index  =  1;
  ReadRequest read_req;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.ReadBulk(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index,
    (pfc_bool_t)false, (pfc_bool_t)false, &read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  ReadBulk
TEST_F(BoundaryTest, ReadBulk_06) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t max_rep_ct  =  1;
  int child_index  =  1;
  ReadRequest read_req;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret = KtboundaryObj.ReadBulk(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index,
    (pfc_bool_t)false, (pfc_bool_t)false, &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformSyntaxValidation returns success
TEST_F(BoundaryTest, PerformSyntaxValidation_01) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  uint32_t operation_type  =  UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation_type, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformSyntaxValidation without boundary name
TEST_F(BoundaryTest, PerformSyntaxValidation_02) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary0(k);
  uint32_t operation_type  =  UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation_type, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

//  PerformSyntaxValidation without controller name1
TEST_F(BoundaryTest, PerformSyntaxValidation_03) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.controller_name1, '\0', 32);
  uint32_t operation_type  =  UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation_type, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
//  PerformSyntaxValidation without controller name2
TEST_F(BoundaryTest, PerformSyntaxValidation_04) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.controller_name2, '\0', 32);
  uint32_t operation_type  =  UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation_type, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

//  PerformSyntaxValidation without domain name1
TEST_F(BoundaryTest, PerformSyntaxValidation_05) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.domain_name1, '\0', 32);
  uint32_t operation_type  =  UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation_type, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
//  PerformSyntaxValidation without domain name2
TEST_F(BoundaryTest, PerformSyntaxValidation_06) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.domain_name2, '\0', 32);
  uint32_t operation_type  =  UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation_type, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
//  PerformSyntaxValidation returns success
TEST_F(BoundaryTest, PerformSyntaxValidation_07) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  uint32_t operation_type  =  UNC_OP_UPDATE;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation_type, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

//  PerformSyntaxValidation returns success
TEST_F(BoundaryTest, PerformSyntaxValidation_08) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  uint32_t operation_type  =  UNC_OP_READ_SIBLING_BEGIN;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation_type, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  GETONEROW returns ODBCM_RC_RECORD_NOT_FOUND
TEST_F(BoundaryTest, ReadBoundaryValFromDB_01) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtboundaryObj.ReadBoundaryValFromDB(
    db_conn, &k, &v, UNC_DT_STATE, UNC_OP_READ, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  GETONEROW returns ODBCM_RC_CONNECTION_ERROR
TEST_F(BoundaryTest, ReadBoundaryValFromDB_02) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.ReadBoundaryValFromDB(
    db_conn, &k, &v, UNC_DT_STATE, UNC_OP_READ, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  GETONEROW returns ODBCM_RC_FAILED
TEST_F(BoundaryTest, ReadBoundaryValFromDB_03) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = KtboundaryObj.ReadBoundaryValFromDB(
    db_conn, &k, &v, UNC_DT_STATE, UNC_OP_READ, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  GETBULKROWS returns ODBCM_RC_RECORD_NOT_FOUND
TEST_F(BoundaryTest, ReadBoundaryValFromDB_04) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtboundaryObj.ReadBoundaryValFromDB(
    db_conn, &k, &v, UNC_DT_STATE, UNC_OP_READ_NEXT, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  GETBULKROWS returns ODBCM_RC_CONNECTION_ERROR
TEST_F(BoundaryTest, ReadBoundaryValFromDB_05) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.ReadBoundaryValFromDB(
    db_conn, &k, &v, UNC_DT_STATE, UNC_OP_READ_NEXT, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  GETBULKROWS returns ODBCM_RC_FAILED
TEST_F(BoundaryTest, ReadBoundaryValFromDB_06) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret = KtboundaryObj.ReadBoundaryValFromDB(
    db_conn, &k, &v, UNC_DT_STATE, UNC_OP_READ_NEXT, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//   operation_type < UNC_OP_READ
TEST_F(BoundaryTest, ReadBoundaryValFromDB_07) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct  =  2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.ReadBoundaryValFromDB(
    db_conn, &k, &v, UNC_DT_STATE, UNC_OP_CREATE, max_rep_ct,
    vect_key_boundary, vect_val_boundary);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  GetModifiedRows returns ODBCM_RC_RECORD_NOT_FOUND
TEST_F(BoundaryTest, GetModifiedRows_01) {
  vector<void *> obj_key_struct;
  obj_key_struct.clear();
  CsRowStatus row_status  =  UPDATED;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtboundaryObj.GetModifiedRows(db_conn, obj_key_struct, row_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  GetModifiedRows returns ODBCM_RC_CONNECTION_ERROR
TEST_F(BoundaryTest, GetModifiedRows_02) {
  vector<void *> obj_key_struct;
  obj_key_struct.clear();
  CsRowStatus row_status  =  UPDATED;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.GetModifiedRows(db_conn, obj_key_struct, row_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  GetModifiedRows returns ODBCM_RC_SUCCESS
TEST_F(BoundaryTest, GetModifiedRows_03) {
  vector<void *> obj_key_struct;
  obj_key_struct.clear();
  CsRowStatus row_status  =  UPDATED;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.GetModifiedRows(db_conn, obj_key_struct, row_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  GetModifiedRows returns ODBCM_RC_FAILED
TEST_F(BoundaryTest, GetModifiedRows_04) {
  vector<void *> obj_key_struct;
  obj_key_struct.clear();
  CsRowStatus row_status  =  UPDATED;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_FAILED);
  int ret = KtboundaryObj.GetModifiedRows(db_conn, obj_key_struct, row_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  IsKeyExists with empty vector
TEST_F(BoundaryTest, IsKeyExists_01) {
  vector<string> obj_key_struct;
  obj_key_struct.clear();
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.IsKeyExists(
    db_conn, UNC_DT_CANDIDATE, obj_key_struct);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

//  IsRowExists returns  ODBCM_RC_CONNECTION_ERROR
TEST_F(BoundaryTest, IsKeyExists_02) {
  vector<string> obj_key_struct;
  obj_key_struct.clear();
  string BD1  =  "boundary1";
  obj_key_struct.push_back(BD1);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.IsKeyExists(
    db_conn, UNC_DT_CANDIDATE, obj_key_struct);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  IsRowExists returns  ODBCM_RC_ROW_EXISTS
TEST_F(BoundaryTest, IsKeyExists_03) {
  vector<string> obj_key_struct;
  obj_key_struct.clear();
  string BD1  =  "boundary1";
  obj_key_struct.push_back(BD1);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtboundaryObj.IsKeyExists(
    db_conn, UNC_DT_CANDIDATE, obj_key_struct);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  IsRowExists returns  ODBCM_RC_FAILED
TEST_F(BoundaryTest, IsKeyExists_04) {
  vector<string> obj_key_struct;
  obj_key_struct.clear();
  string BD1  =  "boundary1";
  obj_key_struct.push_back(BD1);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_FAILED);
  int ret = KtboundaryObj.IsKeyExists(
    db_conn, UNC_DT_CANDIDATE, obj_key_struct);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  PerformSemanticValidation with UNC_OP_CREATE
TEST_F(BoundaryTest, PerformSemanticValidation_01) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_CREATE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_INSTANCE_EXISTS, ret);
}

//  IsKeyExists returns ODBCM_RC_CONNECTION_ERROR
TEST_F(BoundaryTest, PerformSemanticValidation_02) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_CREATE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  PerformSemanticValidation with UNC_OP_UPDATE
TEST_F(BoundaryTest, PerformSemanticValidation_03) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_UPDATE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  PerformSemanticValidation with UNC_OP_DELETE
TEST_F(BoundaryTest, PerformSemanticValidation_04) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_DELETE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  PerformSemanticValidation with UNC_OP_READ
TEST_F(BoundaryTest, PerformSemanticValidation_05) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_READ, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  PerformSemanticValidation with UNC_OP_UPDATE and
//  IsKeyExists returns ODBCM_RC_ROW_NOT_EXISTS
TEST_F(BoundaryTest, PerformSemanticValidation_06) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_UPDATE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  PerformSemanticValidation with UNC_OP_DELETE and
//  IsKeyExists returns ODBCM_RC_ROW_NOT_EXISTS
TEST_F(BoundaryTest, PerformSemanticValidation_07) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_DELETE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  PerformSemanticValidation with UNC_OP_READ and
//  IsKeyExists returns ODBCM_RC_ROW_NOT_EXISTS
TEST_F(BoundaryTest, PerformSemanticValidation_08) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_READ, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  PerformSemanticValidation with UNC_OP_UPDATE and
//  IsKeyExists returns ODBCM_RC_ROW_EXISTS
TEST_F(BoundaryTest, PerformSemanticValidation_09) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_UPDATE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  controller1  =  =  controller2 && domain1  =  =  domain2
TEST_F(BoundaryTest, PerformSemanticValidation_10) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary5(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_UPDATE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SEMANTIC, ret);
}

//  Val struct NULL
TEST_F(BoundaryTest, PerformSemanticValidation_11) {
  key_boundary_t k;
  val_boundary_t *v  =  NULL;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, v, UNC_OP_UPDATE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformSemanticValidation with UNC_OP_DELETE
//  and IsKeyExists returns ODBCM_RC_ROW_EXISTS
TEST_F(BoundaryTest, PerformSemanticValidation_12) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_DELETE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  Getctr type failure
TEST_F(BoundaryTest, PerformSemanticValidation_13) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_UPDATE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SEMANTIC, ret);
}

//  Domain1 name Default
TEST_F(BoundaryTest, PerformSemanticValidation_14) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.domain_name1, '\0', 32);
  memcpy(v.domain_name1, "(DEFAULT)", strlen("(DEFAULT)"));
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_UPDATE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SEMANTIC, ret);
}

//  Domain2 name Default
TEST_F(BoundaryTest, PerformSemanticValidation_15) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.domain_name2, '\0', 32);
  memcpy(v.domain_name2, "(DEFAULT)", strlen("(DEFAULT)"));
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.PerformSemanticValidation(
    db_conn, &k, &v, UNC_OP_UPDATE, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SEMANTIC, ret);
}

//  ValidateSiblingFiltering
TEST_F(BoundaryTest, ValidateSiblingFiltering_01) {
  Kt_Boundary  KtboundaryObj;
  int ret = KtboundaryObj.ValidateSiblingFiltering(0, 0, 1, 1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

//  ValidateSiblingFiltering
TEST_F(BoundaryTest, ValidateSiblingFiltering_02) {
  Kt_Boundary  KtboundaryObj;
  int ret = KtboundaryObj.ValidateSiblingFiltering(1, 0, 0, 1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

//  ValidateSiblingFiltering
TEST_F(BoundaryTest, ValidateSiblingFiltering_03) {
  Kt_Boundary  KtboundaryObj;
  int ret = KtboundaryObj.ValidateSiblingFiltering(1, 1, 1, 1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
#if 0
//TODO(ODC)
//  SendOperStatusNotification Success
TEST_F(BoundaryTest, SendOperStatusNotification_01) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint8_t old_oper_st  =  1;
  uint8_t new_oper_st  =  0;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  Kt_Boundary  KtboundaryObj;
  int ret = KtboundaryObj.SendOperStatusNotification(
    k, old_oper_st, new_oper_st);
  EXPECT_EQ(UNC_UPPL_RC_FAILURE, ret);
}
//  SendOperStatusNotification Failure
TEST_F(BoundaryTest, SendOperStatusNotification_02) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint8_t old_oper_st  =  1;
  uint8_t new_oper_st  =  0;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  Kt_Boundary  KtboundaryObj;
  int ret = KtboundaryObj.SendOperStatusNotification(
    k, old_oper_st, new_oper_st);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
#endif
//  GetAllBoundaryOperStatus success
TEST_F(BoundaryTest, GetAllBoundaryOperStatus_01) {
  string ctr_name  =  "controller1";
  string domain  =  "domain1";
  string lp_name  =  "logical_port1";
  map<string, uint8_t> bdry_notfn;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.GetAllBoundaryOperStatus(
    db_conn, ctr_name, domain, lp_name, bdry_notfn, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  GetAllBoundaryOperStatus Failure
TEST_F(BoundaryTest, GetAllBoundaryOperStatus_02) {
  string ctr_name  =  "controller1";
  string domain  =  "domain1";
  string lp_name  =  "logical_port1";
  map<string, uint8_t> bdry_notfn;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret = KtboundaryObj.GetAllBoundaryOperStatus(
    db_conn, ctr_name, domain, lp_name, bdry_notfn, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  CheckBoundaryExistence
TEST_F(BoundaryTest, CheckBoundaryExistence_01) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.CheckBoundaryExistence(db_conn, &k, &v, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SEMANTIC, ret);
}

//  GetOperStatus success
TEST_F(BoundaryTest, GetOperStatus_01) {
  key_boundary_t k;
  uint8_t oper_status  =  0;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.GetOperStatus(db_conn, UNC_DT_STATE, &k, oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  GetOperStatus Failure
TEST_F(BoundaryTest, GetOperStatus_02) {
  key_boundary_t k;
  uint8_t oper_status  =  0;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.GetOperStatus(db_conn, UNC_DT_STATE, &k, oper_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  GetBoundaryValidFlag
TEST_F(BoundaryTest, GetBoundaryValidFlag_01) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.GetBoundaryValidFlag(db_conn, &k, v, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  IsBoundaryReferred with key type UNC_KT_CONTROLLER
TEST_F(BoundaryTest, IsBoundaryReferred_01) {
  key_ctr_t k;
  pfc_strlcpy(reinterpret_cast<char *>(k.controller_name), "Controller1",
              sizeof(k.controller_name));
  unc_key_type_t keytype  =  UNC_KT_CONTROLLER;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  OdbcmConnectionHandler *db_conn  = NULL;
  ASSERT_EQ(PFC_TRUE,
            KtboundaryObj.IsBoundaryReferred(db_conn, keytype, &k,
                                             UNC_DT_STATE));
}

//  IsBoundaryReferred with key type UNC_KT_CONTROLLER and row not exists
TEST_F(BoundaryTest, IsBoundaryReferred_02) {
  key_ctr_t k;
  pfc_strlcpy(reinterpret_cast<char *>(k.controller_name), "Controller1",
              sizeof(k.controller_name));
  unc_key_type_t keytype  =  UNC_KT_CONTROLLER;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  OdbcmConnectionHandler *db_conn  = NULL;
  ASSERT_EQ(PFC_FALSE,
            KtboundaryObj.IsBoundaryReferred(db_conn, keytype, &k,
                                             UNC_DT_STATE));
}

//  IsBoundaryReferred with key type UNC_KT_CTR_DOMAIN
TEST_F(BoundaryTest, IsBoundaryReferred_03) {
  key_ctr_domain_t k;
  pfc_strlcpy(reinterpret_cast<char *>(k.ctr_key.controller_name),
              "Controller1", sizeof(k.ctr_key.controller_name));
  pfc_strlcpy(reinterpret_cast<char *>(k.domain_name), "Domain1",
              sizeof(k.domain_name));
  unc_key_type_t keytype  =  UNC_KT_CTR_DOMAIN;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  OdbcmConnectionHandler *db_conn  = NULL;
  ASSERT_EQ(PFC_TRUE,
            KtboundaryObj.IsBoundaryReferred(db_conn, keytype, &k,
                                             UNC_DT_STATE));
}

//  IsBoundaryReferred with key type UNC_KT_CTR_DOMAIN and row not exists
TEST_F(BoundaryTest, IsBoundaryReferred_04) {
  key_ctr_domain_t k;
  pfc_strlcpy(reinterpret_cast<char *>(k.ctr_key.controller_name),
              "Controller1", sizeof(k.ctr_key.controller_name));
  pfc_strlcpy(reinterpret_cast<char *>(k.domain_name), "Domain1",
              sizeof(k.domain_name));
  unc_key_type_t keytype  =  UNC_KT_CTR_DOMAIN;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  OdbcmConnectionHandler *db_conn  = NULL;
  ASSERT_EQ(PFC_FALSE,
            KtboundaryObj.IsBoundaryReferred(db_conn, keytype, &k,
                                             UNC_DT_STATE));
}

//  IsBoundaryReferred with unknown key type
TEST_F(BoundaryTest, IsBoundaryReferred_05) {
  unc_key_type_t keytype  =  UNC_KT_LINK;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  OdbcmConnectionHandler *db_conn  = NULL;
  ASSERT_EQ(PFC_FALSE,
            KtboundaryObj.IsBoundaryReferred(db_conn, keytype, NULL,
                                             UNC_DT_STATE));
}

// PerformRead with negative option1
TEST_F(BoundaryTest, PerformRead_Neg_option1_01) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret = KtboundaryObj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_DETAIL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

// PerformRead with negative option1
TEST_F(BoundaryTest, PerformRead_Neg_option1_02) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION1);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret = KtboundaryObj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_DETAIL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformRead with unsupported datatype
TEST_F(BoundaryTest, PerformRead_Neg_datatype_03) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_AUDIT);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret = KtboundaryObj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v, (uint32_t)UNC_DT_AUDIT,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

// PerformRead with unsupported datatype
TEST_F(BoundaryTest, PerformRead_Neg_datatype_04) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_AUDIT);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret = KtboundaryObj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v, (uint32_t)UNC_DT_AUDIT,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformRead with negative dB
TEST_F(BoundaryTest, PerformRead_Neg_DB_05) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret = KtboundaryObj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v, (uint32_t)UNC_DT_CANDIDATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

// PerformRead with positive dB
TEST_F(BoundaryTest, PerformRead_pos_DB_06) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret = KtboundaryObj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v, (uint32_t)UNC_DT_CANDIDATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformRead with max_rep_cnt NULL
TEST_F(BoundaryTest, PerformRead_max_rep_count_07) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret = KtboundaryObj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v, (uint32_t)UNC_DT_CANDIDATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)0);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

// PerformRead with Datatype UNC_DT_STATE
TEST_F(BoundaryTest, PerformRead_STATE_08) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
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

  int ret = KtboundaryObj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// PerformRead with max_rep_cnt NULL
TEST_F(BoundaryTest, PerformRead_max_rep_count_09) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
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
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret = KtboundaryObj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v, (uint32_t)UNC_DT_CANDIDATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)0);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

// HandleOperStatus with Val_Struct NULL
TEST_F(BoundaryTest, HandleOperStatus_Val_Struct_NULL_01) {
  key_boundary_t k;
  val_boundary_t *v = NULL;
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = KtboundaryObj.HandleOperStatus(
    db_conn, UNC_DT_STATE, &k, v, ref_oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// HandleOperStatus
TEST_F(BoundaryTest, HandleOperStatus_02) {
  key_boundary_t k;
  val_boundary_t v;
  memset(&v, 0, sizeof(v));
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  v.valid[kIdxBoundaryControllerName1] = 0;
  v.valid[kIdxBoundaryControllerName2] = 0;

  int ret = KtboundaryObj.HandleOperStatus(
    db_conn, UNC_DT_CANDIDATE, &k, &v, ref_oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// HandleOperStatus
TEST_F(BoundaryTest, HandleOperStatus_03) {
  key_boundary_t k;
  val_boundary_t v;
  memset(&v, 0, sizeof(v));
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  v.valid[kIdxBoundaryControllerName1] = 1;
  v.valid[kIdxBoundaryControllerName2] = 0;

  int ret = KtboundaryObj.HandleOperStatus(
    db_conn, UNC_DT_CANDIDATE, &k, &v, ref_oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// HandleOperStatus
TEST_F(BoundaryTest, HandleOperStatus_04) {
  key_boundary_t k;
  val_boundary_t v;
  memset(&v, 0, sizeof(v));
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  v.valid[kIdxBoundaryControllerName2] = 1;
  v.valid[kIdxBoundaryControllerName1] = 0;

  int ret = KtboundaryObj.HandleOperStatus(
    db_conn, UNC_DT_CANDIDATE, &k, &v, ref_oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// HandleOperStatus
TEST_F(BoundaryTest, HandleOperStatus_05) {
  key_boundary_t k;
  val_boundary_t v;
  memset(&v, 0, sizeof(v));
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  v.valid[kIdxBoundaryControllerName2] = 1;
  v.valid[kIdxBoundaryControllerName1] = 1;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);

  int ret = KtboundaryObj.HandleOperStatus(
    db_conn, UNC_DT_RUNNING, &k, &v, ref_oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// HandleOperStatus
TEST_F(BoundaryTest, HandleOperStatus_06) {
  key_boundary_t k;
  val_boundary_t v;
  memset(&v, 0, sizeof(v));
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn  = NULL;
  v.valid[kIdxBoundaryControllerName2] = 1;
  v.valid[kIdxBoundaryControllerName1] = 1;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);

  int ret = KtboundaryObj.HandleOperStatus(
    db_conn, UNC_DT_RUNNING, &k, &v, ref_oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
//  FrameValidValue
TEST_F(BoundaryTest, FrameValidValue_01) {
  val_boundary_st v_st;
  val_boundary_t v;
  memset(&v, 0xff, sizeof(v));
  memset(&v_st, 0xff, sizeof(v_st));

  string at_value("01234567");
  Kt_Boundary  KtboundaryObj;
  KtboundaryObj.FrameValidValue(at_value, v_st, v);

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
//  FrameCsAttrValue
TEST_F(BoundaryTest, FrameCsAttrValue_01) {
  val_boundary_t v;
  string at_value  =  "boundary";
  Kt_Boundary  KtboundaryObj;
  int ret = UNC_RC_SUCCESS;
  KtboundaryObj.FrameCsAttrValue(at_value, v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/********TAMIL TC's*********/
/*****getBoundaryInputOperStatus*****/
// Send controller name as empty
TEST_F(BoundaryTest, getBoundaryInputOperStatus_BadRequest) {
  OdbcmConnectionHandler *db_conn  =  NULL;
  uint32_t data_type(UNC_DT_STATE);
  string controller_name;
  string domain_name;
  string logical_port_id;
  vector<OperStatusHolder> ref_oper_status;
  Kt_Boundary  KtboundaryObj;
  int ret = KtboundaryObj.getBoundaryInputOperStatus(db_conn, data_type,
    controller_name, domain_name, logical_port_id, ref_oper_status);
  EXPECT_EQ(UPPL_BOUNDARY_OPER_UNKNOWN, ret);
}

// Fill the controler name and send
TEST_F(BoundaryTest, getBoundaryInputOperStatus_ctrlName_fill) {
  OdbcmConnectionHandler *db_conn  =  NULL;
  uint32_t data_type  =  UNC_DT_STATE;
  string controller_name  =  "ctr1";
  string domain_name;
  string logical_port_id;
  vector<OperStatusHolder> ref_oper_status;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.getBoundaryInputOperStatus(db_conn, data_type,
    controller_name, domain_name, logical_port_id, ref_oper_status);
  EXPECT_EQ(UPPL_BOUNDARY_OPER_UNKNOWN, ret);
}

TEST_F(BoundaryTest, getBoundaryInputOperStatus) {
  OdbcmConnectionHandler *db_conn  =  NULL;
  uint32_t data_type  =  UNC_DT_IMPORT;
  string controller_name  =  "ctr1";
  string domain_name  =  "";
  string logical_port_id  =  "";

  unc_key_type_t key_type  =  UNC_KT_CONTROLLER;
  key_ctr_t key_struct;
  pfc_strlcpy(reinterpret_cast<char *>(key_struct.controller_name),
              "controller1", sizeof(key_struct.controller_name));
  uint8_t oper_status  =  1;
  OperStatusHolder obj(key_type, &key_struct, oper_status);
  vector<OperStatusHolder> ref_oper_status;
  ref_oper_status.push_back(obj);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.getBoundaryInputOperStatus(db_conn, data_type,
    controller_name, domain_name, logical_port_id, ref_oper_status);
  EXPECT_EQ(UPPL_BOUNDARY_OPER_UNKNOWN, ret);
}

/********SetOperStatus********/
// key_struct and val_struct are NULL
// return success
TEST_F(BoundaryTest, SetOperStatus_NULL_check) {
  OdbcmConnectionHandler *db_conn  =  NULL;
  uint32_t data_type(UNC_DT_STATE);
  key_boundary_t *k  =  NULL;
  val_boundary_st_t *v  =  NULL;
  UpplBoundaryOperStatus oper_status(UPPL_BOUNDARY_OPER_UP);
  Kt_Boundary  KtboundaryObj;
  int ret = KtboundaryObj.SetOperStatus(
    db_conn, data_type, k, v, oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// controller_name1/2 flag is not valid
TEST_F(BoundaryTest, SetOperStatus_InValid_CtrName) {
  OdbcmConnectionHandler *db_conn  =  NULL;
  uint32_t data_type(UNC_DT_STATE);
  key_boundary_t *k  =  NULL;
  val_boundary_st_t v;
  memset(&v, 0, sizeof(v));
  UpplBoundaryOperStatus oper_status(UPPL_BOUNDARY_OPER_UP);
  Kt_Boundary  KtboundaryObj;
  int ret = KtboundaryObj.SetOperStatus(
    db_conn, data_type, k, &v, oper_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

// SetOperStatus Success_check
TEST_F(BoundaryTest, SetOperStatus_Success_check) {
  OdbcmConnectionHandler *db_conn  =  NULL;
  uint32_t data_type(UNC_DT_STATE);
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  v.valid[kIdxBoundaryControllerName1]  =  UNC_VF_VALID;
  UpplBoundaryOperStatus oper_status(UPPL_BOUNDARY_OPER_UP);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.SetOperStatus(
    db_conn, data_type, &k, &v, oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// oper_status update operation not success
TEST_F(BoundaryTest, SetOperStatus_UPD_NotSucc_Ctr1_Valid) {
  OdbcmConnectionHandler *db_conn  =  NULL;
  uint32_t data_type(UNC_DT_STATE);
  key_boundary_t *k  =  NULL;
  val_boundary_t v;
  // getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  v.valid[kIdxBoundaryControllerName1]  =  UNC_VF_VALID;
  UpplBoundaryOperStatus oper_status  =  UPPL_BOUNDARY_OPER_UNKNOWN;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.SetOperStatus(
    db_conn, data_type, k, &v, oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// oper_status update operation not success
// ctr2 name is set as Invaild
TEST_F(BoundaryTest, SetOperStatus_Ctr2_Valid) {
  OdbcmConnectionHandler *db_conn  =  NULL;
  uint32_t data_type(UNC_DT_STATE);
  key_boundary_t *k  =  NULL;
  val_boundary_t v;
  // getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  v.valid[kIdxBoundaryControllerName1]  =  0;
  UpplBoundaryOperStatus oper_status  =  UPPL_BOUNDARY_OPER_UNKNOWN;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtboundaryObj.SetOperStatus(
    db_conn, data_type, k, &v, oper_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

