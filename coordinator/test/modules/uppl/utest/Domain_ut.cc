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
#include <itc_kt_logicalport.hh>
#include <ipct_util.hh>
#include <itc_read_request.hh>
#include <tclib_module.hh>
#include "PhysicalLayerStub.hh"
#include "ut_util.hh"

using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
using namespace std;
using namespace unc::tclib;
using namespace unc::uppl::test;

class DomainTest
  : public UpplTestEnv {
};

static char pkctrName1[] = "Controller1";
static char pkDomainName2[] = "Domain1";
static char pkDomainName3[] = "(DEFAULT)";
static char valDescription[] = "description1";

TEST_F(DomainTest, PerformSyntxCheck_Domainname_notFound_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(DomainTest, PerformSyntxCheck_Controllername_notFound_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(DomainTest, PerformSyntxCheck_Valstrct_without_type_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));

  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(DomainTest, PerformSyntxCheck_Valstrct_04) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;

  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(DomainTest, PerformSyntxCheck_Valstrct_05) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;

  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(DomainTest, PerformSyntxCheck_Valstrct_06) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 0;
  v.valid[kIdxDomainType] = 1;

  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(DomainTest, PerformSyntxCheck_Valstrct_07) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;

  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName3, strlen(pkDomainName3));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

TEST_F(DomainTest, PerformSyntxCheck_Valstrct_08) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  v.valid[kIdxDomainDescription] = 0;

  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(DomainTest, PerformSyntxCheck_val_struct_empty_09) {
  key_ctr_domain_t k;
  val_ctr_domain *v;
  v = NULL;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(
    db_conn, &k, v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
//  Create for unsupported datatype
TEST_F(DomainTest, Create_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  v.valid[kIdxDomainDescription] = 0;

  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Create(
      db_conn, session_id, configuration_id, &k, &v, UNC_DT_STATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
//  Domain Create success
TEST_F(DomainTest, Create_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  v.valid[kIdxDomainDescription] = 0;

  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Create(
      db_conn, session_id, configuration_id, &k, &v, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  get_controller_type returns failure
TEST_F(DomainTest, Create_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  v.valid[kIdxDomainDescription] = 0;

  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =
      KtdomianObj.Create(
          db_conn, session_id, configuration_id, &k, &v, UNC_DT_CANDIDATE,
          ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*
// Create on unsupported datatype
TEST_F(DomainTest, Create) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDomainDescription] = 0;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =
  KtdomianObj.Create(db_conn, session_id, configuration_id, &k, &v, UNC_DT_STATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(DomainTest, PerformSemanticValidation_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDomainDescription] = 0;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_CANDIDATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}*/

//  IsKeyExists  with UNC_DT_CANDIDATE datatype
//  ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(DomainTest, IsKeyExists_01) {
  Kt_Ctr_Domain  KtdomianObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.IsKeyExists(db_conn, UNC_DT_CANDIDATE, vect_key);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  IsKeyExists  with UNC_DT_CANDIDATE datatype
//  ODBC return ODBCM_RC_ROW_EXISTS
TEST_F(DomainTest, IsKeyExists_02) {
  Kt_Ctr_Domain  KtdomianObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  KtdomianObj.IsKeyExists(db_conn, UNC_DT_CANDIDATE, vect_key);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  IsKeyExists  with UNC_DT_CANDIDATE datatype ODBC
//  return ODBCM_RC_QUERY_TIMEOUT
TEST_F(DomainTest, IsKeyExists_03) {
  Kt_Ctr_Domain  KtdomianObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_QUERY_TIMEOUT);
  int ret =  KtdomianObj.IsKeyExists(db_conn, UNC_DT_CANDIDATE, vect_key);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  IsKeyExists  with key structure empty
TEST_F(DomainTest, IsKeyExists_04) {
  Kt_Ctr_Domain  KtdomianObj;
  vector<string> vect_key;
  vect_key.clear();
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.IsKeyExists(db_conn, UNC_DT_CANDIDATE, vect_key);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

//  CreateKeyInstance with UNC_DT_CANDIDATE datatype ODBC
//  return ODBCM_RC_CONNECTION_ERROR
TEST_F(DomainTest, CreateKeyInstance_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.CreateKeyInstance(
      db_conn, &k, &v, UNC_DT_CANDIDATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  CreateKeyInstance with UNC_DT_RUNNING datatype ODBC
TEST_F(DomainTest, CreateKeyInstance_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.CreateKeyInstance(
      db_conn, &k, &v, UNC_DT_RUNNING, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

//  CreateKeyInstance with UNC_DT_STATE  datatype
//  ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(DomainTest, CreateKeyInstance_03) {
  key_ctr_domain_t k;
  val_ctr_domain_st v;
  memset(&v, 0, sizeof(v));
  v.domain.type = 1;
  v.domain.valid[kIdxDomainType] = 1;
  memcpy(v.domain.description, valDescription, strlen(valDescription));
  v.domain.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.CreateKeyInstance(
      db_conn, &k, &v, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  CreateKeyInstance with UNC_DT_IMPORT
//  datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(DomainTest, CreateKeyInstance_04) {
  key_ctr_domain_t k;
  val_ctr_domain_st v;
  memset(&v, 0, sizeof(v));
  v.domain.type = 1;
  v.domain.valid[kIdxDomainType] = 1;
  memcpy(v.domain.description, valDescription, strlen(valDescription));
  v.domain.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.CreateKeyInstance(
      db_conn, &k, &v, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  CreateKeyInstance with UNC_DT_CANDIDATE
//  datatype ODBC return Failure
TEST_F(DomainTest, CreateKeyInstance_05) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtdomianObj.CreateKeyInstance(
      db_conn, &k, &v, UNC_DT_CANDIDATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_CREATE, ret);
}

//  CreateKeyInstance with UNC_DT_CANDIDATE
//  datatype ODBC ODBCM_RC_SUCCESS
TEST_F(DomainTest, CreateKeyInstance_06) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.CreateKeyInstance(
      db_conn, &k, &v, UNC_DT_CANDIDATE, key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  CreateKeyInstance with UNC_DT_STATE  datatype ODBC return Failure
TEST_F(DomainTest, CreateKeyInstance_07) {
  key_ctr_domain_t k;
  val_ctr_domain_st v;
  memset(&v, 0, sizeof(v));
  v.domain.type = 1;
  v.domain.valid[kIdxDomainType] = 1;
  memcpy(v.domain.description, valDescription, strlen(valDescription));
  v.domain.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtdomianObj.CreateKeyInstance(
      db_conn, &k, &v, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_CREATE, ret);
}

//  CreateKeyInstance with UNC_DT_IMPORT
//  datatype ODBC return Failure
TEST_F(DomainTest, CreateKeyInstance_08) {
  key_ctr_domain_t k;
  val_ctr_domain_st v;
  memset(&v, 0, sizeof(v));
  v.domain.type = 1;
  v.domain.valid[kIdxDomainType] = 1;
  memcpy(v.domain.description, valDescription, strlen(valDescription));
  v.domain.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtdomianObj.CreateKeyInstance(
      db_conn, &k, &v, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_CREATE, ret);
}

//  Update for unsupported datatype
TEST_F(DomainTest, Update_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  v.valid[kIdxDomainDescription] = 0;

  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Update(
      db_conn, session_id, configuration_id, &k, &v, UNC_DT_STATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
//  Domain Update success
TEST_F(DomainTest, Update_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  v.valid[kIdxDomainDescription] = 0;

  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Update(
      db_conn, session_id, configuration_id, &k, &v, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  get_controller_type returns failure
TEST_F(DomainTest, Update_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  v.valid[kIdxDomainDescription] = 0;

  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Update(
      db_conn, session_id, configuration_id, &k, &v, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  UpdateKeyInstance with UNC_DT_CANDIDATE ODBC
//  retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(DomainTest, UpdateKeyInstance_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.UpdateKeyInstance(
    db_conn, &k, &v, UNC_DT_CANDIDATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  UpdateKeyInstance on unsupported datatype
TEST_F(DomainTest, UpdateKeyInstance_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.UpdateKeyInstance(
    db_conn, &k, &v, UNC_DT_RUNNING, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

//  UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(DomainTest, UpdateKeyInstance_03) {
  key_ctr_domain_t k;
  val_ctr_domain_st v;
  memset(&v, 0, sizeof(v));
  v.domain.type = 1;
  v.domain.valid[kIdxDomainType] = 1;
  memcpy(v.domain.description, valDescription, strlen(valDescription));
  v.domain.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.UpdateKeyInstance(
    db_conn, &k, &v, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns ODBCM_RC_SUCCESS
TEST_F(DomainTest, UpdateKeyInstance_04) {
  key_ctr_domain_t k;
  val_ctr_domain_st v;
  memset(&v, 0, sizeof(v));
  v.domain.type = 1;
  v.domain.valid[kIdxDomainType] = 1;
  memcpy(v.domain.description, valDescription, strlen(valDescription));
  v.domain.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.UpdateKeyInstance(
    db_conn, &k, &v, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns
TEST_F(DomainTest, UpdateKeyInstance_05) {
  key_ctr_domain_t k;
  val_ctr_domain_st v;
  memset(&v, 0, sizeof(v));
  v.domain.type = 1;
  v.domain.valid[kIdxDomainType] = 1;
  memcpy(v.domain.description, valDescription, strlen(valDescription));
  v.domain.valid[kIdxDomainDescription] = 1;

  uint32_t key_type = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtdomianObj.UpdateKeyInstance(
    db_conn, &k, &v, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

//  Delete for unsupported datatype
TEST_F(DomainTest, Delete_01) {
  key_ctr_domain_t k;
  val_ctr_domain_st v;
  memset(&v, 0, sizeof(v));
  v.domain.type = 1;
  v.domain.valid[kIdxDomainType] = 1;
  v.domain.valid[kIdxDomainDescription] = 0;

  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Delete(
      db_conn, session_id, configuration_id, &k, UNC_DT_STATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
//  Domain Delete success
TEST_F(DomainTest, Delete_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  v.valid[kIdxDomainDescription] = 0;

  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Delete(
      db_conn, session_id, configuration_id, &k, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  get_controller_type returns failure
TEST_F(DomainTest, Delete_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  v.valid[kIdxDomainDescription] = 0;

  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Delete(
      db_conn, session_id, configuration_id, &k, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  Domain Delete With boundary referred
TEST_F(DomainTest, Delete_04) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  v.valid[kIdxDomainDescription] = 0;

  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Delete(db_conn,
                                session_id, configuration_id,
                                &k, UNC_DT_CANDIDATE, ses);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  UpdateKeyInstance with ODBC retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(DomainTest, GetModifiedRows_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;

  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.GetModifiedRows(db_conn, obj_key_struct, UPDATED);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  ODBC retuns ODBCM_RC_RECORD_NOT_FOUND
TEST_F(DomainTest, GetModifiedRows_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;

  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtdomianObj.GetModifiedRows(db_conn, obj_key_struct, UPDATED);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  ODBC retuns ODBCM_RC_SUCCESS
TEST_F(DomainTest, GetModifiedRows_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;

  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.GetModifiedRows(db_conn, obj_key_struct, UPDATED);
  EXPECT_EQ(ODBCM_RC_SUCCESS, ret);
}

//  ODBC retuns ODBCM_RC_FAILED
TEST_F(DomainTest, GetModifiedRows_04) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;

  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.GetModifiedRows(db_conn, obj_key_struct, UPDATED);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(DomainTest, SetOperStatus_001) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplDomainOperStatus)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

TEST_F(DomainTest, SetOperStatus_002) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtdomianObj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplDomainOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(DomainTest, SetOperStatus_003) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplDomainOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(DomainTest, SetOperStatus_004) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ser_evt;
  ser_evt.addOutput((uint32_t)UNC_OP_CREATE);
  ser_evt.addOutput((uint32_t)UNC_DT_STATE);
  ser_evt.addOutput((uint32_t)UNC_KT_PORT);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplDomainOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(DomainTest, SetOperStatus_005) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ser_evt;
  ser_evt.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ser_evt.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  ser_evt.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplDomainOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  GetOperStatus ODBC returns failure
TEST_F(DomainTest, GetOperStatus_001) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.GetOperStatus(db_conn, UNC_DT_STATE, &k, op_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  GetOperStatus ODBC returns SUCCESS
TEST_F(DomainTest, GetOperStatus_002) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.GetOperStatus(db_conn, UNC_DT_STATE, &k, op_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  HandleOperStatus with NULL keystruct
TEST_F(DomainTest, HandleOperStatus_01) {
  key_ctr_domain_t *k;
  val_ctr_domain v;
  k = NULL;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.HandleOperStatus(db_conn, UNC_DT_STATE, k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

//  HandleOperStatus
TEST_F(DomainTest, HandleOperStatus_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

//  HandleOperStatus Controller oper_status  retunrs failure
TEST_F(DomainTest, HandleOperStatus_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

//  HandleDriverAlarms with unsupported alarm type
TEST_F(DomainTest, HandleDriverAlarms_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  HandleDriverAlarms with UNC_COREDOMAIN_SPLIT alarm type
TEST_F(DomainTest, HandleDriverAlarms_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  HandleDriverAlarms with oper_type UNC_OP_CREATE
TEST_F(DomainTest, HandleDriverAlarms_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  HandleDriverAlarms with oper_type UNC_OP_DELETE
TEST_F(DomainTest, HandleDriverAlarms_04) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_DELETE;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_CREATE
TEST_F(DomainTest, PerformSemanticValidation_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  KtdomianObj.PerformSemanticValidation(
    db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_INSTANCE_EXISTS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_CREATE
TEST_F(DomainTest, PerformSemanticValidation_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.PerformSemanticValidation(
    db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_CREATE
TEST_F(DomainTest, PerformSemanticValidation_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtdomianObj.PerformSemanticValidation(
    db_conn, &k, &v, oper_type, UNC_DT_IMPORT);
  EXPECT_EQ(UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_READ
TEST_F(DomainTest, PerformSemanticValidation_04) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_READ;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  KtdomianObj.PerformSemanticValidation(
    db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_UPDATE
TEST_F(DomainTest, PerformSemanticValidation_05) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_UPDATE;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.PerformSemanticValidation(
    db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  PerformSemanticValidation with oper_type UNC_OP_DELETE
TEST_F(DomainTest, PerformSemanticValidation_06) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_DELETE;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_QUERY_TIMEOUT);
  int ret =  KtdomianObj.PerformSemanticValidation(
    db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  ReadBulkInternal with max_count zero
TEST_F(DomainTest, ReadBulkInternal_01) {
  key_ctr_domain_t k;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct,
    vect_val_ctr_domain_st, vect_domain_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_RECORD_NOT_FOUND
TEST_F(DomainTest, ReadBulkInternal_02) {
  key_ctr_domain_t k;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtdomianObj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct,
    vect_val_ctr_domain_st, vect_domain_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_CONNECTION_ERROR
TEST_F(DomainTest, ReadBulkInternal_03) {
  key_ctr_domain_t k;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct,
    vect_val_ctr_domain_st, vect_domain_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_TABLE_NOT_FOUND
TEST_F(DomainTest, ReadBulkInternal_04) {
  key_ctr_domain_t k;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_TABLE_NOT_FOUND);
  int ret =  KtdomianObj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct,
    vect_val_ctr_domain_st, vect_domain_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

//  ReadBulkInternal with read_db_status ODBCM_RC_SUCCESS
TEST_F(DomainTest, ReadBulkInternal_05) {
  key_ctr_domain_t k;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct,
    vect_val_ctr_domain_st, vect_domain_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//   ReadBulk with get_controller_type returns failure
//   MEM
TEST_F(DomainTest, ReadBulk_02) {
  key_ctr_domain_t k;
  int child_index = 2;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = false;
  ReadRequest read_req;
  uint32_t max_rep_ct = 4;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
        child_index, parent_call, is_read_next,
                                  &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//   ReadBulk with data type UNC_DT_IMPORT
TEST_F(DomainTest, ReadBulk_01) {
  key_ctr_domain_t k;
  int child_index = 0;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = false;
  ReadRequest read_req;
  uint32_t max_rep_ct = 4;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.ReadBulk(db_conn, &k, UNC_DT_IMPORT, max_rep_ct,
    child_index, parent_call, is_read_next, &read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

//  DeleteKeyInstance with data type UNC_DT_RUNNING
TEST_F(DomainTest, DeleteKeyInstance_01) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  // unc::uppl::ODBCManager::stub_setResultcode(
  // unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.DeleteKeyInstance(db_conn, &k, UNC_DT_RUNNING,
                                           key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

//  DeleteKeyInstance with out child
TEST_F(DomainTest, DeleteKeyInstance_02) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtdomianObj.DeleteKeyInstance(db_conn,
                                           &k, UNC_DT_CANDIDATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  DeleteKeyInstance with out child
TEST_F(DomainTest, DeleteKeyInstance_03) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtdomianObj.DeleteKeyInstance(
      db_conn, &k, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  DeleteKeyInstance with out child
TEST_F(DomainTest, DeleteKeyInstance_04) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtdomianObj.DeleteKeyInstance(
      db_conn, &k, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SEMANTIC, ret);
}

//  DeleteKeyInstance suceess
TEST_F(DomainTest, DeleteKeyInstance_05) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.DeleteKeyInstance(db_conn, &k, UNC_DT_IMPORT,
                                           key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}


//  FreeChildKeyStruct suceess
TEST_F(DomainTest, FreeChildKeyStruct_02) {
  void *key = new key_logical_port_t;
  int child_class = 0;
  Kt_Ctr_Domain  KtdomianObj;
  int ret =  UNC_RC_SUCCESS;
  KtdomianObj.FreeChildKeyStruct(child_class, key);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  InvokeBoundaryNotifyOperStatus  suceess
TEST_F(DomainTest, InvokeBoundaryNotifyOperStatus_01) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.InvokeBoundaryNotifyOperStatus(
      db_conn, UNC_DT_IMPORT, &k);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformRead invalid option and session failure
TEST_F(DomainTest, PerformRead_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  uint32_t option1 = UNC_OPT1_DETAIL;
  uint32_t option2 = 2;
  uint32_t max_rep_ct = 1;
  uint32_t operation_type  = UNC_OP_READ;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDomainDescription] = 0;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformRead(
    db_conn, session_id, configuration_id, &k, &v,
    UNC_DT_CANDIDATE, operation_type,
    ses, option1, option2, max_rep_ct);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  PerformRead invalid option
TEST_F(DomainTest, PerformRead_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  uint32_t option1 = UNC_OPT1_DETAIL;
  uint32_t option2 = 2;
  uint32_t max_rep_ct = 1;
  uint32_t operation_type  = UNC_OP_READ;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDomainDescription] = 0;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)operation_type);
  ses.stub_setAddOutput((uint32_t)max_rep_ct);
  ses.stub_setAddOutput((uint32_t)option1);
  ses.stub_setAddOutput((uint32_t)option2);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION1);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformRead(
    db_conn, session_id, configuration_id, &k, &v,
    UNC_DT_CANDIDATE, operation_type,
    ses, option1, option2, max_rep_ct);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformRead invalid data type and session failure
TEST_F(DomainTest, PerformRead_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  uint32_t option1 = UNC_OPT1_NORMAL;
  uint32_t option2 = 2;
  uint32_t max_rep_ct = 1;
  uint32_t operation_type  = UNC_OP_READ;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDomainDescription] = 0;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformRead(
    db_conn, session_id, configuration_id, &k, &v, UNC_DT_AUDIT, operation_type,
    ses, option1, option2, max_rep_ct);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  PerformRead invalid data type
TEST_F(DomainTest, PerformRead_04) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  uint32_t option1 = UNC_OPT1_NORMAL;
  uint32_t option2 = 2;
  uint32_t max_rep_ct = 1;
  uint32_t operation_type  = UNC_OP_READ;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDomainDescription] = 0;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)operation_type);
  ses.stub_setAddOutput((uint32_t)max_rep_ct);
  ses.stub_setAddOutput((uint32_t)option1);
  ses.stub_setAddOutput((uint32_t)option2);
  ses.stub_setAddOutput((uint32_t)UNC_DT_AUDIT);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  ses.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformRead(
    db_conn, session_id, configuration_id, &k, &v, UNC_DT_AUDIT, operation_type,
    ses, option1, option2, max_rep_ct);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformRead invalid data type
TEST_F(DomainTest, PerformRead_05) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  uint32_t option1 = UNC_OPT1_NORMAL;
  uint32_t option2 = 2;
  uint32_t max_rep_ct = 0;
  uint32_t operation_type  = UNC_OP_READ;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDomainDescription] = 0;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)operation_type);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  ses.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.PerformRead(
    db_conn, session_id, configuration_id, &k, &v,
    UNC_DT_CANDIDATE, operation_type,
    ses, option1, option2, max_rep_ct);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  PerformRead invalid data type
TEST_F(DomainTest, PerformRead_06) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  uint32_t option1 = UNC_OPT1_NORMAL;
  uint32_t option2 = 2;
  uint32_t max_rep_ct = 0;
  uint32_t operation_type  = UNC_OP_READ;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDomainDescription] = 0;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)operation_type);
  ses.stub_setAddOutput((uint32_t)max_rep_ct);
  ses.stub_setAddOutput((uint32_t)option1);
  ses.stub_setAddOutput((uint32_t)option2);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  ses.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.PerformRead(
    db_conn, session_id, configuration_id, &k, &v,
    UNC_DT_CANDIDATE, operation_type,
    ses, option1, option2, max_rep_ct);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

//  PerformRead invalid data type
TEST_F(DomainTest, PerformRead_07) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  uint32_t option1 = UNC_OPT1_NORMAL;
  uint32_t option2 = 2;
  uint32_t max_rep_ct = 1;
  uint32_t operation_type  = UNC_OP_READ;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDomainDescription] = 0;
  Kt_Ctr_Domain  KtdomianObj;
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)operation_type);
  ses.stub_setAddOutput((uint32_t)max_rep_ct);
  ses.stub_setAddOutput((uint32_t)option1);
  ses.stub_setAddOutput((uint32_t)option2);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  ses.stub_setAddOutput((uint32_t)0);
  // ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.PerformRead(
    db_conn, session_id, configuration_id, &k, &v,
    UNC_DT_CANDIDATE, operation_type,
    ses, option1, option2, max_rep_ct);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  ReadDomainValFromDB with operation type UNC_OP_INVALID
TEST_F(DomainTest, ReadDomainValFromDB_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  uint32_t max_rep_ct = 1;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> domain_id;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_INVALID;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.ReadDomainValFromDB(
    db_conn, &k, &v, UNC_DT_CANDIDATE, oper_type,
    max_rep_ct,
    vect_val_ctr_domain_st, domain_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  ReadDomainValFromDB with operation type UNC_OP_READ
TEST_F(DomainTest, ReadDomainValFromDB_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;

  uint32_t max_rep_ct = 1;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> domain_id;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_READ;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtdomianObj.ReadDomainValFromDB(
    db_conn, &k, &v, UNC_DT_CANDIDATE, oper_type,
    max_rep_ct,
    vect_val_ctr_domain_st, domain_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

//  ReadDomainValFromDB with operation type UNC_OP_READ_BULK
TEST_F(DomainTest, ReadDomainValFromDB_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;

  uint32_t max_rep_ct = 1;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> domain_id;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_READ_BULK;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.ReadDomainValFromDB(
    db_conn, &k, &v, UNC_DT_CANDIDATE, oper_type,
    max_rep_ct,
    vect_val_ctr_domain_st, domain_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(DomainTest, ReadDomainValFromDB_04) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;

  uint32_t max_rep_ct = 1;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> domain_id;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_READ_BULK;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_QUERY_TIMEOUT);
  int ret =  KtdomianObj.ReadDomainValFromDB(
    db_conn, &k, &v, UNC_DT_CANDIDATE, oper_type,
    max_rep_ct,
    vect_val_ctr_domain_st, domain_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(DomainTest, ReadDomainValFromDB_05) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  memset(&v, 0, sizeof(v));
  v.type = 1;
  v.valid[kIdxDomainType] = 1;

  uint32_t max_rep_ct = 1;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> domain_id;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t oper_type = UNC_OP_READ_BULK;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.ReadDomainValFromDB(
    db_conn, &k, &v, UNC_DT_CANDIDATE, oper_type,
    max_rep_ct,
    vect_val_ctr_domain_st, domain_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  GetChildClassPointer suceess
TEST_F(DomainTest, GetChildClassPointer_02) {
  int KIndex = 0;
  Kt_Ctr_Domain  KtdomianObj;
  Kt_Base *child(KtdomianObj.GetChildClassPointer((KtDomainChildClass)KIndex));
  ASSERT_TRUE(child != NULL);
}
