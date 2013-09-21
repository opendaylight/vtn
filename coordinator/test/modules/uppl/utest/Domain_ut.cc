 /*
  * Copyright (c) 2012-2013 NEC Corporation
  * All rights reserved
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
#include "stub/ODBC/include/odbcm_mgr.hh"
#include "physical_common_def.hh"
#include "unc/uppl_common.h"
#include "unc/keytype.h"
#include "itc_kt_base.hh"
#include "itc_kt_root.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_port.hh"
#include "itc_kt_link.hh"
#include "itc_kt_boundary.hh"
#include "itc_kt_logicalport.hh"
#include "ipct_util.hh"
#include "PhysicalLayerStub.hh"
#include "itc_read_request.hh"
#include "tclib_module.hh"

using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
using namespace std;
using namespace unc::tclib;

ClientSession *cli_sess = NULL;
pfc_ipcid_t service = UPPL_SVC_CONFIGREQ;
class KtClassTest : public testing::Test {
protected:
  virtual void SetUp() {
    unc::tclib::TcLibModule::stub_loadtcLibModule();
    if (cli_sess == NULL) {
      pfc_ipcconn_t connp = 0;
      int err = pfc_ipcclnt_altopen(UPPL_IPC_CHN_NAME, &connp);
      ASSERT_EQ(0, err);
      //ASSERT_TRUE(connp != 0);
      cli_sess = new ClientSession(connp, UPPL_IPC_SVC_NAME, service, err);
      //ASSERT_EQ(0, err);
    } else {
      cli_sess->reset(UPPL_IPC_SVC_NAME, service);
    }
  }
  virtual void TearDown() {
  }
};

// Can be changed based on testing need
#if 0
char pkName1[] = "Controller1_domain_name";
char pkName2[] = "Domain7";
char pkName3[] = "Domain15";
char pkName4[] = "";
char pkName5[] = "NotExisting";
char pkName11[] = "Domain20";

char pkName6[] = "Controller1";
char pkName7[] = "Controller7";
char pkName8[] = "Controller15";
char pkName9[] = "";
char pkName10[] = "NotExisting";
char pkName12[] = "Controller20";



#endif
char pkctrName1[] = "Controller1";
char pkDomainName2[] = "Domain1";
char pkDomainName3[] = "(DEFAULT)";
char valDescription[] = "description1";

TEST_F(KtClassTest, PerformSyntxCheck_Domainname_notFound_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  PhysicalLayerStub::loadphysicallayer();
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtClassTest, PerformSyntxCheck_Controllername_notFound_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtClassTest, PerformSyntxCheck_Valstrct_without_type_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtClassTest, PerformSyntxCheck_Valstrct_04) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, PerformSyntxCheck_Valstrct_05) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtClassTest, PerformSyntxCheck_Valstrct_06) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  v.type = 0;
  v.valid[kIdxDomainType] = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtClassTest, PerformSyntxCheck_Valstrct_07) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName3, strlen(pkDomainName3));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtClassTest, PerformSyntxCheck_Valstrct_08) {
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
  int ret =  KtdomianObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, PerformSyntxCheck_val_struct_empty_09) {
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
  int ret =  KtdomianObj.PerformSyntaxValidation(db_conn,&k,v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
// Create for unsupported datatype
TEST_F(KtClassTest, Create_01) {
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
  int ret =  KtdomianObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_STATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}
// Domain Create success 
TEST_F(KtClassTest, Create_02) {
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
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// get_controller_type returns failure 
TEST_F(KtClassTest, Create_03) {
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
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
/*
//Create on unsupported datatype 
TEST_F(KtClassTest, Create) {
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
  int ret =  KtdomianObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_STATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}
 
TEST_F(KtClassTest, PerformSemanticValidation_01) {
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
  int ret =  KtdomianObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}*/

// IsKeyExists  with UNC_DT_CANDIDATE datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, IsKeyExists_01) {
  Kt_Ctr_Domain  KtdomianObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,vect_key);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// IsKeyExists  with UNC_DT_CANDIDATE datatype ODBC return ODBCM_RC_ROW_EXISTS
TEST_F(KtClassTest, IsKeyExists_02) {
  Kt_Ctr_Domain  KtdomianObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  KtdomianObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,vect_key);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// IsKeyExists  with UNC_DT_CANDIDATE datatype ODBC return ODBCM_RC_QUERY_TIMEOUT
TEST_F(KtClassTest, IsKeyExists_03) {
  Kt_Ctr_Domain  KtdomianObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_QUERY_TIMEOUT);
  int ret =  KtdomianObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,vect_key);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// IsKeyExists  with key structure empty
TEST_F(KtClassTest, IsKeyExists_04) {
  Kt_Ctr_Domain  KtdomianObj;
  vector<string> vect_key;
  vect_key.clear();
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,vect_key);
  EXPECT_EQ(ret,UPPL_RC_ERR_BAD_REQUEST);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateKeyInstance with UNC_DT_CANDIDATE datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, CreateKeyInstance_01) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_CANDIDATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// CreateKeyInstance with UNC_DT_RUNNING datatype ODBC 
TEST_F(KtClassTest, CreateKeyInstance_02) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_RUNNING,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  unc::uppl::ODBCManager::clearStubData();
}
// CreateKeyInstance with UNC_DT_STATE  datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, CreateKeyInstance_03) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_STATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateKeyInstance with UNC_DT_IMPORT  datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, CreateKeyInstance_04) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_IMPORT,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateKeyInstance with UNC_DT_CANDIDATE datatype ODBC return Failure
TEST_F(KtClassTest, CreateKeyInstance_05) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_CANDIDATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_CREATE);
  unc::uppl::ODBCManager::clearStubData();
}
// CreateKeyInstance with UNC_DT_CANDIDATE datatype ODBC ODBCM_RC_SUCCESS
TEST_F(KtClassTest, CreateKeyInstance_06) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_CANDIDATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// CreateKeyInstance with UNC_DT_STATE  datatype ODBC return Failure
TEST_F(KtClassTest, CreateKeyInstance_07) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_STATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_CREATE);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateKeyInstance with UNC_DT_IMPORT  datatype ODBC return Failure
TEST_F(KtClassTest, CreateKeyInstance_08) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_IMPORT,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_CREATE);
  unc::uppl::ODBCManager::clearStubData();
}

// Update for unsupported datatype
TEST_F(KtClassTest, Update_01) {
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
  int ret =  KtdomianObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_STATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}
// Domain Update success 
TEST_F(KtClassTest, Update_02) {
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
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// get_controller_type returns failure 
TEST_F(KtClassTest, Update_03) {
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
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// UpdateKeyInstance with UNC_DT_CANDIDATE ODBC retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, UpdateKeyInstance_01) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.UpdateKeyInstance(db_conn,&k,&v,UNC_DT_CANDIDATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// UpdateKeyInstance on unsupported datatype
TEST_F(KtClassTest, UpdateKeyInstance_02) {
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
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.UpdateKeyInstance(db_conn,&k,&v,UNC_DT_RUNNING,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}
// UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, UpdateKeyInstance_03) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.UpdateKeyInstance(db_conn,&k,&v,UNC_DT_IMPORT,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns ODBCM_RC_SUCCESS
TEST_F(KtClassTest, UpdateKeyInstance_04) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  KtdomianObj.UpdateKeyInstance(db_conn,&k,&v,UNC_DT_IMPORT,key_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns 
TEST_F(KtClassTest, UpdateKeyInstance_05) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtdomianObj.UpdateKeyInstance(db_conn,&k,&v,UNC_DT_IMPORT,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}
// Delete for unsupported datatype
TEST_F(KtClassTest, Delete_01) {
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
  int ret =  KtdomianObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_STATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}
// Domain Delete success 
TEST_F(KtClassTest, Delete_02) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// get_controller_type returns failure 
TEST_F(KtClassTest, Delete_03) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
// Domain Delete With boundary referred
TEST_F(KtClassTest, Delete_04) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
// UpdateKeyInstance with ODBC retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, GetModifiedRows_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  vector<void *> obj_key_struct;
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;
  PhysicalLayerStub::loadphysicallayer();
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.GetModifiedRows(db_conn,obj_key_struct,UPDATED);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ODBC retuns ODBCM_RC_RECORD_NOT_FOUND 
TEST_F(KtClassTest, GetModifiedRows_02) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  vector<void *> obj_key_struct;
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;
  PhysicalLayerStub::loadphysicallayer();
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtdomianObj.GetModifiedRows(db_conn,obj_key_struct,UPDATED);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// ODBC retuns ODBCM_RC_SUCCESS 
TEST_F(KtClassTest, GetModifiedRows_03) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  vector<void *> obj_key_struct;
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;
  PhysicalLayerStub::loadphysicallayer();
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.GetModifiedRows(db_conn,obj_key_struct,UPDATED);
  EXPECT_EQ(ret,ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ODBC retuns ODBCM_RC_FAILED
TEST_F(KtClassTest, GetModifiedRows_04) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  vector<void *> obj_key_struct;
  uint32_t key_type = 1;
  v.type = 1;
  v.valid[kIdxDomainType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDomainDescription] = 1;
  PhysicalLayerStub::loadphysicallayer();
  Kt_Ctr_Domain  KtdomianObj;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.GetModifiedRows(db_conn,obj_key_struct,UPDATED);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, SetOperStatus_001) {

  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplDomainOperStatus)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, SetOperStatus_002) {

  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtdomianObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplDomainOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, SetOperStatus_003) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplDomainOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, SetOperStatus_004) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ser_evt;
  ServerSession::clearStubData();
  int err;
  ser_evt.addOutput((uint32_t)UNC_OP_CREATE);
  ser_evt.addOutput((uint32_t)UNC_DT_STATE);
  ser_evt.addOutput((uint32_t)UNC_KT_PORT);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplDomainOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, SetOperStatus_005) {
  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER, TC_API_COMMON_SUCCESS);
  unc::uppl::PhysicalLayer *physical_layer = unc::uppl::PhysicalLayer::get_instance();
  physical_layer->init();
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ser_evt;
  ServerSession::clearStubData();
  int err;
  ser_evt.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ser_evt.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  ser_evt.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplDomainOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// GetOperStatus ODBC returns failure
TEST_F(KtClassTest, GetOperStatus_001) {

  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.GetOperStatus(db_conn,UNC_DT_STATE,&k,op_status);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
// GetOperStatus ODBC returns SUCCESS
TEST_F(KtClassTest, GetOperStatus_002) {

  key_ctr_domain_t k;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.GetOperStatus(db_conn,UNC_DT_STATE,&k,op_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// HandleOperStatus with NULL keystruct
TEST_F(KtClassTest, HandleOperStatus_01) {

  key_ctr_domain_t *k;
  val_ctr_domain v;
  k = NULL;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  int ret =  KtdomianObj.HandleOperStatus(db_conn,UNC_DT_STATE,k,&v);
  EXPECT_EQ(ret,UPPL_RC_ERR_BAD_REQUEST);
}

// HandleOperStatus 
TEST_F(KtClassTest, HandleOperStatus_02) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

// HandleOperStatus Controller oper_status  retunrs failure 
TEST_F(KtClassTest, HandleOperStatus_03) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

// HandleDriverAlarms with unsupported alarm type
TEST_F(KtClassTest, HandleDriverAlarms_01) {
  key_ctr_domain_t k;
  val_ctr_domain v;
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// HandleDriverAlarms with UNC_COREDOMAIN_SPLIT alarm type
TEST_F(KtClassTest, HandleDriverAlarms_02) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtdomianObj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// HandleDriverAlarms with oper_type UNC_OP_CREATE 
TEST_F(KtClassTest, HandleDriverAlarms_03) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// HandleDriverAlarms with oper_type UNC_OP_DELETE 
TEST_F(KtClassTest, HandleDriverAlarms_04) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_CREATE 
TEST_F(KtClassTest, PerformSemanticValidation_01) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  KtdomianObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_INSTANCE_EXISTS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_CREATE 
TEST_F(KtClassTest, PerformSemanticValidation_02) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_CREATE 
TEST_F(KtClassTest, PerformSemanticValidation_03) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtdomianObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_IMPORT);
  EXPECT_EQ(ret,UPPL_RC_ERR_PARENT_DOES_NOT_EXIST);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_READ 
TEST_F(KtClassTest, PerformSemanticValidation_04) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  KtdomianObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_UPDATE 
TEST_F(KtClassTest, PerformSemanticValidation_05) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_DELETE 
TEST_F(KtClassTest, PerformSemanticValidation_06) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_QUERY_TIMEOUT);
  int ret =  KtdomianObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal with max_count zero
TEST_F(KtClassTest, ReadBulkInternal_01) {
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
  int ret =  KtdomianObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_val_ctr_domain_st, vect_domain_id);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal with read_db_status ODBCM_RC_RECORD_NOT_FOUND
TEST_F(KtClassTest, ReadBulkInternal_02) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtdomianObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_val_ctr_domain_st, vect_domain_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal with read_db_status ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, ReadBulkInternal_03) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_val_ctr_domain_st, vect_domain_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal with read_db_status ODBCM_RC_TABLE_NOT_FOUND
TEST_F(KtClassTest, ReadBulkInternal_04) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_TABLE_NOT_FOUND);
  int ret =  KtdomianObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct, 
                                              vect_val_ctr_domain_st, vect_domain_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal with read_db_status ODBCM_RC_SUCCESS
TEST_F(KtClassTest, ReadBulkInternal_05) {
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct,
                                             vect_val_ctr_domain_st, vect_domain_id);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//  ReadBulk with get_controller_type returns failure
//  MEM
TEST_F(KtClassTest, ReadBulk_02) {
  key_ctr_domain_t k;
  int child_index = 2;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = false;
  ReadRequest *read_req = new ReadRequest;
  uint32_t max_rep_ct = 4;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//  ReadBulk with data type UNC_DT_IMPORT
TEST_F(KtClassTest, ReadBulk_01) {
  key_ctr_domain_t k;
  int child_index = 0;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = false;
  ReadRequest *read_req;
  uint32_t max_rep_ct = 4;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.ReadBulk(db_conn, &k, UNC_DT_IMPORT, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteKeyInstance with data type UNC_DT_RUNNING
TEST_F(KtClassTest, DeleteKeyInstance_01) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  //unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.DeleteKeyInstance(db_conn, &k, UNC_DT_RUNNING, key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteKeyInstance with out child
TEST_F(KtClassTest, DeleteKeyInstance_02) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtdomianObj.DeleteKeyInstance(db_conn, &k, UNC_DT_CANDIDATE, key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteKeyInstance with out child
TEST_F(KtClassTest, DeleteKeyInstance_03) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtdomianObj.DeleteKeyInstance(db_conn, &k, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteKeyInstance with out child
TEST_F(KtClassTest, DeleteKeyInstance_04) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtdomianObj.DeleteKeyInstance(db_conn, &k, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SEMANTIC);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteKeyInstance suceess
TEST_F(KtClassTest, DeleteKeyInstance_05) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.DeleteKeyInstance(db_conn, &k, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// FreeChildKeyStruct invalid index
TEST_F(KtClassTest, FreeChildKeyStruct_01) {
  key_ctr_domain_t k;
  int child_class = 1;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  UPPL_RC_SUCCESS;
  KtdomianObj.FreeChildKeyStruct(child_class, &k);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// FreeChildKeyStruct suceess
TEST_F(KtClassTest, FreeChildKeyStruct_02) {
  void *key = new key_logical_port_t;
  int child_class = 0;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  UPPL_RC_SUCCESS;
  KtdomianObj.FreeChildKeyStruct(child_class, key);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// InvokeBoundaryNotifyOperStatus  suceess
TEST_F(KtClassTest, InvokeBoundaryNotifyOperStatus_01) {
  key_ctr_domain_t k;
  uint32_t key_type = 0;
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.InvokeBoundaryNotifyOperStatus(db_conn, UNC_DT_IMPORT, &k);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformRead invalid option and session failure
TEST_F(KtClassTest, PerformRead_01) {
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
  ServerSession::clearStubData();
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformRead(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,operation_type,
                                      ses, option1, option2,max_rep_ct);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformRead invalid option 
TEST_F(KtClassTest, PerformRead_02) {
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
  ServerSession::clearStubData();
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)operation_type);
  ses.stub_setAddOutput((uint32_t)max_rep_ct);
  ses.stub_setAddOutput((uint32_t)option1);
  ses.stub_setAddOutput((uint32_t)option2);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_INVALID_OPTION1);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformRead(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,operation_type,
                                      ses, option1, option2,max_rep_ct);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformRead invalid data type and session failure
TEST_F(KtClassTest, PerformRead_03) {
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
  ServerSession::clearStubData();
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformRead(db_conn,session_id,configuration_id,&k,&v,UNC_DT_AUDIT,operation_type,
                                      ses, option1, option2,max_rep_ct);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformRead invalid data type 
TEST_F(KtClassTest, PerformRead_04) {
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
  ServerSession::clearStubData();
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)operation_type);
  ses.stub_setAddOutput((uint32_t)max_rep_ct);
  ses.stub_setAddOutput((uint32_t)option1);
  ses.stub_setAddOutput((uint32_t)option2);
  ses.stub_setAddOutput((uint32_t)UNC_DT_AUDIT);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.PerformRead(db_conn,session_id,configuration_id,&k,&v,UNC_DT_AUDIT,operation_type,
                                      ses, option1, option2,max_rep_ct);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformRead invalid data type 
TEST_F(KtClassTest, PerformRead_05) {
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
  ServerSession::clearStubData();
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)operation_type);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.PerformRead(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,operation_type,
                                      ses, option1, option2,max_rep_ct);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformRead invalid data type 
TEST_F(KtClassTest, PerformRead_06) {
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
  ServerSession::clearStubData();
  ServerSession ses;
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)operation_type);
  ses.stub_setAddOutput((uint32_t)max_rep_ct);
  ses.stub_setAddOutput((uint32_t)option1);
  ses.stub_setAddOutput((uint32_t)option2);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.PerformRead(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,operation_type,
                                      ses, option1, option2,max_rep_ct);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformRead invalid data type 
TEST_F(KtClassTest, PerformRead_07) {
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
  ServerSession::clearStubData();
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
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkctrName1, strlen(pkctrName1));
  memset(k.domain_name, '\0', sizeof(k.domain_name));
  memcpy(k.domain_name, pkDomainName2, strlen(pkDomainName2));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.PerformRead(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,operation_type,
                                      ses, option1, option2,max_rep_ct);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadDomainValFromDB with operation type UNC_OP_INVALID
TEST_F(KtClassTest, ReadDomainValFromDB_01) {
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
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_INVALID;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtdomianObj.ReadDomainValFromDB(db_conn,&k,&v,UNC_DT_CANDIDATE,oper_type,
                                          max_rep_ct,vect_val_ctr_domain_st,domain_id);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// ReadDomainValFromDB with operation type UNC_OP_READ
TEST_F(KtClassTest, ReadDomainValFromDB_02) {
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
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_READ;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtdomianObj.ReadDomainValFromDB(db_conn,&k,&v,UNC_DT_CANDIDATE,oper_type,
                                          max_rep_ct,vect_val_ctr_domain_st,domain_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadDomainValFromDB with operation type UNC_OP_READ_BULK
TEST_F(KtClassTest, ReadDomainValFromDB_03) {
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
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_READ_BULK;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtdomianObj.ReadDomainValFromDB(db_conn,&k,&v,UNC_DT_CANDIDATE,oper_type,
                                          max_rep_ct,vect_val_ctr_domain_st,domain_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadDomainValFromDB_04) {
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
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_READ_BULK;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_QUERY_TIMEOUT);
  int ret =  KtdomianObj.ReadDomainValFromDB(db_conn,&k,&v,UNC_DT_CANDIDATE,oper_type,
                                          max_rep_ct,vect_val_ctr_domain_st,domain_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadDomainValFromDB_05) {
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
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_READ_BULK;
  vector<void *> obj_key_struct;
  Kt_Ctr_Domain  KtdomianObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtdomianObj.ReadDomainValFromDB(db_conn,&k,&v,UNC_DT_CANDIDATE,oper_type,
                                          max_rep_ct,vect_val_ctr_domain_st,domain_id);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// GetChildClassPointer invalid index
TEST_F(KtClassTest, GetChildClassPointer_01) {
  int KIndex = 1;
  Kt_Ctr_Domain  KtdomianObj;
  Kt_Base *child[KT_CTR_DOMAIN_CHILD_COUNT];
  //Kt_Base *base_obj;
  int ret =  UPPL_RC_SUCCESS;
  child[KIndex]  = KtdomianObj.GetChildClassPointer((KtDomainChildClass)KIndex);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// GetChildClassPointer suceess
TEST_F(KtClassTest, GetChildClassPointer_02) {
  int KIndex = 0;
  Kt_Ctr_Domain  KtdomianObj;
 // Kt_Base *base_obj;
  Kt_Base *child[KT_CTR_DOMAIN_CHILD_COUNT];
  int ret =  UPPL_RC_SUCCESS;
  child[KIndex] = KtdomianObj.GetChildClassPointer((KtDomainChildClass)KIndex);
  //delete child[KIndex];
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
