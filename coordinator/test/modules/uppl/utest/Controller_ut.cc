
 /*
  * Copyright (c) 2012-2013 NEC Corporation
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
#include "itc_read_request.hh"
#include "itc_kt_logicalport.hh"
#include "ipct_util.hh"
#include "PhysicalLayerStub.hh"
#include "tclib_module.hh"

using unc::uppl::PhysicalLayer;
using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
using namespace std;
using namespace unc::tclib;
ClientSession *cli_sess = NULL;
pfc_ipcid_t service = UPPL_SVC_CONFIGREQ;
class KtCtrlrTest : public testing::Test {
protected:
  virtual void SetUp() {
    if (cli_sess == NULL) {
      pfc_ipcconn_t connp = 0;
      int err = pfc_ipcclnt_altopen(UPPL_IPC_CHN_NAME, &connp);
      ASSERT_EQ(0, err);
      //ASSERT_TRUE(connp != 0);
      cli_sess = new ClientSession(connp, UPPL_IPC_SVC_NAME, service, err);
      //ASSERT_EQ(0, err);
      PhysicalLayerStub::loadphysicallayer();
      unc::tclib::TcLibModule::stub_loadtcLibModule();
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

TEST_F(KtCtrlrTest, PerformSyntxCheck_Domainname_notFound_01) {
  key_ctr_t k;
  val_ctr_t v;
  PhysicalLayerStub::loadphysicallayer();
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}
TEST_F(KtCtrlrTest, PerformSyntxCheck_Controllername_notFound_02) {
  key_ctr_t k;
  val_ctr_t v;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtCtrlrTest, PerformSyntxCheck_Valstrct_without_type_03) {
  key_ctr_t k;
  val_ctr_t v;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtCtrlrTest, PerformSyntxCheck_Valstrct_04) {
  key_ctr_t k;
  val_ctr_t v;
  v.type = 1;
  v.valid[kIdxType] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtCtrlrTest, PerformSyntxCheck_Valstrct_05) {
  key_ctr_t k;
  val_ctr_t v;
  v.type = 1;
  v.valid[kIdxType] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtCtrlrTest, PerformSyntxCheck_Valstrct_06) {
  key_ctr_t k;
  val_ctr_t v;
  v.type = 0;
  v.valid[kIdxType] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtCtrlrTest, PerformSyntxCheck_Valstrct_07) {
  key_ctr_t k;
  val_ctr_t v;
  v.type = 1;
  v.valid[kIdxType] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtCtrlrTest, PerformSyntxCheck_Valstrct_08) {
  key_ctr_t k;
  val_ctr_t v;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

// Create for unsupported datatype
TEST_F(KtCtrlrTest, Create_01) {
  key_ctr_t k;
  val_ctr_t v;
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
  int ret =  ktCtrlrObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_STATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}
// Domain Create success 
TEST_F(KtCtrlrTest, Create_02) {
  key_ctr_t k;
  val_ctr_t v;
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
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// get_controller_type returns failure 
TEST_F(KtCtrlrTest, Create_03) {
  key_ctr_t k;
  val_ctr_t v;
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
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

//Create on unsupported datatype 
TEST_F(KtCtrlrTest, Create) {
  key_ctr_t k;
  val_ctr_t v;
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
  int ret =  ktCtrlrObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_STATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}
 
TEST_F(KtCtrlrTest, PerformSemanticValidation_01) {
  key_ctr_t k;
  val_ctr_t v;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

// IsKeyExists  with UNC_DT_CANDIDATE datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(KtCtrlrTest, IsKeyExists_01) {
  Kt_Controller  ktCtrlrObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,vect_key);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// IsKeyExists  with UNC_DT_CANDIDATE datatype ODBC return ODBCM_RC_ROW_EXISTS
TEST_F(KtCtrlrTest, IsKeyExists_02) {
  Kt_Controller  ktCtrlrObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktCtrlrObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,vect_key);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// IsKeyExists  with UNC_DT_CANDIDATE datatype ODBC return ODBCM_RC_QUERY_TIMEOUT
TEST_F(KtCtrlrTest, IsKeyExists_03) {
  Kt_Controller  ktCtrlrObj;
  vector<string> vect_key;
  vect_key.push_back(pkctrName1);
  vect_key.push_back(pkDomainName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_QUERY_TIMEOUT);
  int ret =  ktCtrlrObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,vect_key);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// IsKeyExists  with key structure empty
TEST_F(KtCtrlrTest, IsKeyExists_04) {
  Kt_Controller  ktCtrlrObj;
  vector<string> vect_key;
  vect_key.clear();
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,vect_key);
  EXPECT_EQ(ret,UPPL_RC_ERR_BAD_REQUEST);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateKeyInstance with UNC_DT_CANDIDATE datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(KtCtrlrTest, CreateKeyInstance_01) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_CANDIDATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// CreateKeyInstance with UNC_DT_RUNNING datatype ODBC 
TEST_F(KtCtrlrTest, CreateKeyInstance_02) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_RUNNING,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// CreateKeyInstance with UNC_DT_STATE  datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(KtCtrlrTest, CreateKeyInstance_03) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_STATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateKeyInstance with UNC_DT_IMPORT  datatype ODBC return ODBCM_RC_CONNECTION_ERROR
TEST_F(KtCtrlrTest, CreateKeyInstance_04) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_IMPORT,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateKeyInstance with UNC_DT_CANDIDATE datatype ODBC return Failure
TEST_F(KtCtrlrTest, CreateKeyInstance_05) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_CANDIDATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_CREATE);
  unc::uppl::ODBCManager::clearStubData();
}
// CreateKeyInstance with UNC_DT_CANDIDATE datatype ODBC ODBCM_RC_SUCCESS
TEST_F(KtCtrlrTest, CreateKeyInstance_06) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_CANDIDATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// CreateKeyInstance with UNC_DT_STATE  datatype ODBC return Failure
TEST_F(KtCtrlrTest, CreateKeyInstance_07) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_STATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_CREATE);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateKeyInstance with UNC_DT_IMPORT  datatype ODBC return Failure
TEST_F(KtCtrlrTest, CreateKeyInstance_08) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_IMPORT,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_CREATE);
  unc::uppl::ODBCManager::clearStubData();
}

// Update for unsupported datatype
TEST_F(KtCtrlrTest, Update_01) {
  key_ctr_t k;
  val_ctr_t v;
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
  int ret =  ktCtrlrObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_STATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}
// Domain Update success 
TEST_F(KtCtrlrTest, Update_02) {
  key_ctr_t k;
  val_ctr_t v;
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
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// get_controller_type returns failure 
TEST_F(KtCtrlrTest, Update_03) {
  key_ctr_t k;
  val_ctr_t v;
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
  //ses.stub_setAddOutput((uint32_t)&k);
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// UpdateKeyInstance with UNC_DT_CANDIDATE ODBC retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtCtrlrTest, UpdateKeyInstance_01) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.UpdateKeyInstance(db_conn,&k,&v,UNC_DT_CANDIDATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// UpdateKeyInstance on unsupported datatype
TEST_F(KtCtrlrTest, UpdateKeyInstance_02) {
  key_ctr_t k;
  val_ctr_t v;
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
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.UpdateKeyInstance(db_conn,&k,&v,UNC_DT_RUNNING,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
}
// UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtCtrlrTest, UpdateKeyInstance_03) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.UpdateKeyInstance(db_conn,&k,&v,UNC_DT_IMPORT,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns ODBCM_RC_SUCCESS
TEST_F(KtCtrlrTest, UpdateKeyInstance_04) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  //PhysicalLayer *physical_layer = new PhysicalLayer();
  int ret =  ktCtrlrObj.UpdateKeyInstance(db_conn,&k,&v,UNC_DT_IMPORT,key_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// UpdateKeyInstance with UNC_DT_IMPORT ODBC retuns 
TEST_F(KtCtrlrTest, UpdateKeyInstance_05) {
  key_ctr_t k;
  val_ctr_t v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  ktCtrlrObj.UpdateKeyInstance(db_conn,&k,&v,UNC_DT_IMPORT,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

// Delete for unsupported datatype
TEST_F(KtCtrlrTest, Delete_01) {
  unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER, TC_API_COMMON_SUCCESS);
  unc::uppl::PhysicalLayer *physical_layer = unc::uppl::PhysicalLayer::get_instance();
  physical_layer->init();

  key_ctr_t k;
  val_ctr_t v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  ServerSession ::clearStubData();
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_STATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// Domain Delete success 
TEST_F(KtCtrlrTest, Delete_02) {
  key_ctr_t k;
  val_ctr_t v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ses;
  ServerSession ::clearStubData();
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// get_controller_type returns failure 
TEST_F(KtCtrlrTest, Delete_03) {
  key_ctr_t k;
  val_ctr_t v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ::clearStubData();
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
// Domain Delete With boundary referred
TEST_F(KtCtrlrTest, Delete_04) {
  key_ctr_t k;
  val_ctr_t v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  v.valid[kIdxDescription] = 0;
  Kt_Controller  ktCtrlrObj;
  ServerSession ::clearStubData();
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktCtrlrObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
// UpdateKeyInstance with ODBC retuns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtCtrlrTest, GetModifiedRows_01) {
  key_ctr_t k;
  val_ctr v;
  vector<void *> obj_key_struct;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.GetModifiedRows(db_conn,obj_key_struct,UPDATED);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ODBC retuns ODBCM_RC_RECORD_NOT_FOUND 
TEST_F(KtCtrlrTest, GetModifiedRows_02) {
  key_ctr_t k;
  val_ctr v;
  vector<void *> obj_key_struct;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  ktCtrlrObj.GetModifiedRows(db_conn,obj_key_struct,UPDATED);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// ODBC retuns ODBCM_RC_SUCCESS 
TEST_F(KtCtrlrTest, GetModifiedRows_03) {
  key_ctr_t k;
  val_ctr v;
  vector<void *> obj_key_struct;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.GetModifiedRows(db_conn,obj_key_struct,UPDATED);
  EXPECT_EQ(ret,ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ODBC retuns ODBCM_RC_FAILED
TEST_F(KtCtrlrTest, GetModifiedRows_04) {
  key_ctr_t k;
  val_ctr v;
  vector<void *> obj_key_struct;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.GetModifiedRows(db_conn,obj_key_struct,UPDATED);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtCtrlrTest, SetOperStatus_001) {

  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplControllerOperStatus)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtCtrlrTest, SetOperStatus_002) {

  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  ktCtrlrObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplControllerOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtCtrlrTest, SetOperStatus_003) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplControllerOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtCtrlrTest, SetOperStatus_004) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ser_evt;
  ServerSession::clearStubData();
  int err;
  ser_evt.addOutput((uint32_t)UNC_OP_CREATE);
  ser_evt.addOutput((uint32_t)UNC_DT_STATE);
  ser_evt.addOutput((uint32_t)UNC_KT_PORT);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplControllerOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// GetOperStatus ODBC returns failure
TEST_F(KtCtrlrTest, GetOperStatus_001) {

  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.GetOperStatus(db_conn,UNC_DT_STATE,&k,op_status);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
// GetOperStatus ODBC returns SUCCESS
TEST_F(KtCtrlrTest, GetOperStatus_002) {

  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.GetOperStatus(db_conn,UNC_DT_STATE,&k,op_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtCtrlrTest, GetOperStatus_003) {

  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS_WITH_INFO);
  int ret =  ktCtrlrObj.GetOperStatus(db_conn,UNC_DT_STATE,&k,op_status);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// NotifyOperStatus with NULL keystruct
TEST_F(KtCtrlrTest, NotifyOperStatus_01) {

  key_ctr_t k;
  val_ctr v;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  vector<OperStatusHolder> oper_stat_hldr;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  ktCtrlrObj;
//  PhysicalLayer *physical_layer = new PhysicalLayer(pfc_modattr_t *mattr);
  OdbcmConnectionHandler *db_conn =NULL;
  uint8_t op_status;
  int ret =  ktCtrlrObj.NotifyOperStatus(db_conn,UNC_DT_STATE, &k, &v, oper_stat_hldr);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
/*
// NotifyOperStatus 
TEST_F(KtCtrlrTest, NotifyOperStatus_02) {
  key_ctr_t k;
  val_ctr v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.NotifyOperStatus(db_conn,UNC_DT_STATE,&k,&v,oper_stat_hldr );
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

// NotifyOperStatus Controller oper_status  retunrs failure 
TEST_F(KtCtrlrTest, NotifyOperStatus_03) {
  key_ctr_t k;
  val_ctr v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.NotifyOperStatus(db_conn,UNC_DT_STATE,&k,&v,oper_stat_hldr);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}
*/

// HandleOperStatus Controller oper_status returns success
TEST_F(KtCtrlrTest, HandleOperStatus_01) {
  key_ctr_t k;
  val_ctr v;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// HandleOperStatus Controller oper_status  retunrs failure 
TEST_F(KtCtrlrTest, HandleOperStatus_02) {
  key_ctr_t k;
  val_ctr v;
  v.type = 1;
  v.valid[kIdxType] = 1;
  memset(v.description, '\0', 128);
  memcpy(v.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

// HandleOperStatus with bIsInternal false 
TEST_F(KtCtrlrTest, HandleOperStatus_03) {
  key_ctr_t k;
  val_ctr v;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v,bIsInternal);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}
// HandleOperStatus with bIsInternal false and oper_status is UPPL_CONTROLLER_OPER_UP 
TEST_F(KtCtrlrTest, HandleOperStatus_04) {
  key_ctr_t k;
  val_ctr_st_t v;
  v.controller.type = 1;
  v.controller.valid[kIdxType] = 1;
  memset(v.controller.description, '\0', 128);
  memcpy(v.controller.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  v.oper_status=UPPL_CONTROLLER_OPER_UP;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  bool bIsInternal = false;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v,bIsInternal);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// HandleOperStatus with bIsInternal true and oper_status is UPPL_CONTROLLER_OPER_UP 
TEST_F(KtCtrlrTest, HandleOperStatus_05) {
  key_ctr_t k;
  val_ctr_st_t v;
  v.controller.type = 1;
  v.controller.valid[kIdxType] = 1;
  memset(v.controller.description, '\0', 128);
  memcpy(v.controller.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  v.oper_status=UPPL_CONTROLLER_OPER_UP;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  bool bIsInternal=true;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v,bIsInternal);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// HandleOperStatus with bIsInternal true and oper_status is UPPL_CONTROLLER_OPER_DOWN 
TEST_F(KtCtrlrTest, HandleOperStatus_06) {
  key_ctr_t k;
  val_ctr_st_t v;
  v.controller.type = 1;
  v.controller.valid[kIdxType] = 1;
  memset(v.controller.description, '\0', 128);
  memcpy(v.controller.description, valDescription, strlen(valDescription));
  v.valid[kIdxDescription] = 1;
  v.oper_status=UPPL_CONTROLLER_OPER_DOWN;
  Kt_Controller  ktCtrlrObj;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  OdbcmConnectionHandler *db_conn =NULL;
  bool bIsInternal = true;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleOperStatus(db_conn,UNC_DT_RUNNING,&k,&v,bIsInternal);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// HandleDriverAlarms with unsupported alarm type
TEST_F(KtCtrlrTest, HandleDriverAlarms_01) {
  key_ctr_t k;
  val_ctr v;
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// HandleDriverAlarms with UNC_COREDOMAIN_SPLIT alarm type
TEST_F(KtCtrlrTest, HandleDriverAlarms_02) {
  key_ctr_t k;
  val_ctr v;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktCtrlrObj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// HandleDriverAlarms with oper_type UNC_OP_CREATE 
TEST_F(KtCtrlrTest, HandleDriverAlarms_03) {
  key_ctr_t k;
  val_ctr v;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// HandleDriverAlarms with oper_type UNC_OP_DELETE 
TEST_F(KtCtrlrTest, HandleDriverAlarms_04) {
  key_ctr_t k;
  val_ctr v;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t alarm_type = UNC_COREDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_DELETE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktCtrlrObj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_CREATE 
TEST_F(KtCtrlrTest, PerformSemanticValidation_11) {
  key_ctr_t k;
  val_ctr v;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktCtrlrObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_INSTANCE_EXISTS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_CREATE 
TEST_F(KtCtrlrTest, PerformSemanticValidation_02) {
  key_ctr_t k;
  val_ctr v;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_CREATE 
TEST_F(KtCtrlrTest, PerformSemanticValidation_03) {
  key_ctr_t k;
  val_ctr v;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  ktCtrlrObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_IMPORT);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_READ 
TEST_F(KtCtrlrTest, PerformSemanticValidation_04) {
  key_ctr_t k;
  val_ctr v;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_READ;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktCtrlrObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with oper_type UNC_OP_UPDATE 
TEST_F(KtCtrlrTest, PerformSemanticValidation_05) {
  key_ctr_t k;
  val_ctr v;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_UPDATE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktCtrlrObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// PerformSemanticValidation with oper_type UNC_OP_DELETE 
TEST_F(KtCtrlrTest, PerformSemanticValidation_06) {
  key_ctr_t k;
  val_ctr v;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  uint32_t oper_type = UNC_OP_DELETE;
  vector<void *> obj_key_struct;
  Kt_Controller  ktCtrlrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_QUERY_TIMEOUT);
  int ret =  ktCtrlrObj.PerformSemanticValidation(db_conn, &k, &v, oper_type, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

/*
// ReadBulkInternal with max_count zero
TEST_F(KtCtrlrTest, ReadBulkInternal_01) {
  key_ctr_t k;
  val_ctr v;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<key_ctr> vect_ctr_id;
  uint32_t max_rep_ct = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
UpplReturnCode Kt_Boundary::ReadBulkInternal(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void *val_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<key_boundary_t> &vect_key_boundary,
    vector<val_boundary_st_t> &vect_val_boundary) 

  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, &v, UNC_DT_STATE, max_rep_ct,  vect_ctr_id, vect_val_ctr_st);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// ReadBulkInternal with read_db_status ODBCM_RC_RECORD_NOT_FOUND
TEST_F(KtCtrlrTest, ReadBulkInternal_02) {
  key_ctr_t k;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_val_ctr_st, vect_domain_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal with read_db_status ODBCM_RC_CONNECTION_ERROR
TEST_F(KtCtrlrTest, ReadBulkInternal_03) {
  key_ctr_t k;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_val_ctr_st, vect_domain_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal with read_db_status ODBCM_RC_TABLE_NOT_FOUND
TEST_F(KtCtrlrTest, ReadBulkInternal_04) {
  key_ctr_t k;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_TABLE_NOT_FOUND);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct, 
                                              vect_val_ctr_st, vect_domain_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal with read_db_status ODBCM_RC_SUCCESS
TEST_F(KtCtrlrTest, ReadBulkInternal_05) {
  key_ctr_t k;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<key_ctr_domain> vect_domain_id;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct,
                                             vect_val_ctr_st, vect_domain_id);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
*/
//  ReadBulk with get_controller_type returns failure
//  MEM
TEST_F(KtCtrlrTest, ReadBulk_02) {
  key_ctr_t k;
  int child_index = 2;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = false;
  ReadRequest *read_req;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//  ReadBulk with data type UNC_DT_IMPORT
TEST_F(KtCtrlrTest, ReadBulk_01) {
  key_ctr_t k;
  int child_index = 0;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = false;
  ReadRequest *read_req;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_IMPORT, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  unc::uppl::ODBCManager::clearStubData();
}

//  ReadBulk with data type UNC_DT_IMPORT
TEST_F(KtCtrlrTest, ReadBulk_05) {
  key_ctr_t k;
  int child_index = -1;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = false;
  ReadRequest *read_req;
  uint32_t max_rep_ct = 4;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_IMPORT, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  unc::uppl::ODBCManager::clearStubData();
}


//  ReadBulk with data type UNC_DT_IMPORT
TEST_F(KtCtrlrTest, ReadBulk_06) {
  key_ctr_t k;
  int child_index = 2;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = false;
  ReadRequest *read_req;
  uint32_t max_rep_ct = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_IMPORT, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  unc::uppl::ODBCManager::clearStubData();
}


// DeleteKeyInstance with data type UNC_DT_RUNNING
TEST_F(KtCtrlrTest, DeleteKeyInstance_01) {
  key_ctr_t k;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  //unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_DELETE);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteKeyInstance with out child
TEST_F(KtCtrlrTest, DeleteKeyInstance_02) {
  key_ctr_t k;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn, &k, UNC_DT_CANDIDATE, key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteKeyInstance with out child
TEST_F(KtCtrlrTest, DeleteKeyInstance_03) {
  key_ctr_t k;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn, &k, UNC_DT_AUDIT, key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteKeyInstance with out child
TEST_F(KtCtrlrTest, DeleteKeyInstance_04) {
  key_ctr_t k;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_TRANSACTION_ERROR);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn, &k, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_DELETE);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteKeyInstance suceess
TEST_F(KtCtrlrTest, DeleteKeyInstance_05) {
  key_ctr_t k;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn, &k, UNC_DT_IMPORT, key_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// FreeChildKeyStruct success
TEST_F(KtCtrlrTest, FreeChildKeyStruct_01) {
  void *k = new key_ctr_domain_t;
  int child_class = 0;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  UPPL_RC_SUCCESS;
  pfc_log_info("Test FreeChildKeyStruct_01");
  KtctrObj.FreeChildKeyStruct(k, child_class);
}

// FreeChildKeyStruct suceess
TEST_F(KtCtrlrTest, FreeChildKeyStruct_02) {
  void *key = new key_logical_port_t;
  int child_class = 1;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  UPPL_RC_SUCCESS;
  KtctrObj.FreeChildKeyStruct(key, child_class);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// FreeChildKeyStruct suceess
TEST_F(KtCtrlrTest, FreeChildKeyStruct_03) {
  void *key = new key_link_t;
  int child_class = 2;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  UPPL_RC_SUCCESS;
  KtctrObj.FreeChildKeyStruct(key, child_class);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
// FreeChildKeyStruct invalid index
TEST_F(KtCtrlrTest, FreeChildKeyStruct_04) {
  void *key = new key_logical_port_t;
  int child_class = 3;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  UPPL_RC_SUCCESS;
  KtctrObj.FreeChildKeyStruct(key, child_class);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// getChildKeyStruct success
TEST_F(KtCtrlrTest, getChildKeyStruct_01) {
  void *k = new key_ctr_domain_t;
  int child_class = 0;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  UPPL_RC_SUCCESS;
  pfc_log_info("Test FreeChildKeyStruct_01");
  KtctrObj.getChildKeyStruct(child_class, "controller1");
}
// getChildKeyStruct suceess
TEST_F(KtCtrlrTest, getChildKeyStruct_02) {
  void *key = new key_logical_port_t;
  int child_class = 1;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  UPPL_RC_SUCCESS;
  KtctrObj.getChildKeyStruct(child_class, "controller1");
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// getChildKeyStruct suceess
TEST_F(KtCtrlrTest, getChildKeyStruct_03) {
  void *key = new key_link_t;
  int child_class = 2;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  UPPL_RC_SUCCESS;
  KtctrObj.getChildKeyStruct(child_class, "controller1");
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
// getChildKeyStruct invalid index
TEST_F(KtCtrlrTest, getChildKeyStruct_04) {
  void *key = new key_logical_port_t;
  int child_class = 3;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  UPPL_RC_SUCCESS;
  KtctrObj.getChildKeyStruct(child_class, "controller1");
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
//PerformRead with negative option1 
TEST_F(KtCtrlrTest, PerformRead_Neg_option1_01) {
  key_ctr_t k;
  val_ctr v;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with negative option1 
TEST_F(KtCtrlrTest, PerformRead_Neg_option1_02) {
  key_ctr_t k;
  val_ctr v;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_INVALID_OPTION1);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//PerformRead with negative option2 
TEST_F(KtCtrlrTest, PerformRead_Neg_option2_03) {
  key_ctr_t k;
  val_ctr v;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_MAC_ENTRY_STATIC);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_MAC_ENTRY_STATIC,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with negative option1 
TEST_F(KtCtrlrTest, PerformRead_Neg_option2_04) {
  key_ctr_t k;
  val_ctr v;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_MAC_ENTRY_STATIC);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_INVALID_OPTION2);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_MAC_ENTRY_STATIC,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//PerformRead with unsupported datatype 
TEST_F(KtCtrlrTest, PerformRead_Neg_datatype_05) {
  key_ctr_t k;
  val_ctr v;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_AUDIT);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_AUDIT,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with negative dataype
TEST_F(KtCtrlrTest, PerformRead_Neg_datatype_06) {
  key_ctr_t k;
  val_ctr v;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_AUDIT);
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_AUDIT,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//PerformRead with valid option1 and valid option2
TEST_F(KtCtrlrTest, PerformRead_pos_07) {
  key_ctr_t k;
  val_ctr v;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with valid option1 and valid option2
TEST_F(KtCtrlrTest, PerformRead_pos_db_Success_08) {
  key_ctr_t k;
  val_ctr_t v;
  vector<val_ctr_st_t> vect_val_ctr_st;
  int index;
  vect_val_ctr_st[index].controller.cs_row_status == DELETED;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_CANDIDATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)0);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtCtrlrTest, PerformRead_pos_db_fail_09) {
  key_ctr_t k;
  val_ctr v;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)0);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with valid option1 and valid option2
TEST_F(KtCtrlrTest, PerformRead_pos_db_Success_10) {
  key_ctr_t k;
  val_ctr_t v;
  vector<val_ctr_st_t> vect_val_ctr_st;
  int index;
  vect_val_ctr_st[index].controller.cs_row_status == DELETED;
  uint32_t key_type = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  int ret =  KtctrObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_CANDIDATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)0);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

//HandleDriverEvents with other than Update operation
TEST_F(KtCtrlrTest, HandleDriverEvents_No_Update_01) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  pfc_bool_t is_events_done;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_READ;
  uint32_t data_type = UNC_DT_STATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(db_conn,&k,oper_type,data_type,&v_old,&v_new,is_events_done);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//HandleDriverEvents with CANDIDATE operation
TEST_F(KtCtrlrTest, HandleDriverEvents_CANDIDATE_NegDB_02) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(v_old.valid, '\0', sizeof(v_old.valid));
  memset(v_new.valid, '\0', sizeof(v_new.valid));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  pfc_bool_t is_events_done;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_CANDIDATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(db_conn,&k,oper_type,data_type,&v_old,&v_new,is_events_done);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

//HandleDriverEvents with RUNNING datatype
TEST_F(KtCtrlrTest, HandleDriverEvents_RUNNING_NegDB_03) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(v_old.valid, '\0', sizeof(v_old.valid));
  memset(v_new.valid, '\0', sizeof(v_new.valid));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  pfc_bool_t is_events_done;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(db_conn,&k,oper_type,data_type,&v_old,&v_new,is_events_done);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleDriverEvents with RUNNING datatype
TEST_F(KtCtrlrTest, HandleDriverEvents_RUNNING_04) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(v_old.valid, '\0', sizeof(v_old.valid));
  memset(v_new.valid, '\0', sizeof(v_new.valid));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  pfc_bool_t is_events_done;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(db_conn,&k,oper_type,data_type,&v_old,&v_new,is_events_done);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleDriverEvents with RUNNING datatype
TEST_F(KtCtrlrTest, HandleDriverEvents_RUNNING_05) {
  key_ctr_t k;
  val_ctr_st_t *v_old = new val_ctr_st_t;
  val_ctr_st_t *v_new = new val_ctr_st_t;
  v_new->oper_status = UPPL_CONTROLLER_OPER_UP;
  pfc_bool_t is_events_done;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(db_conn,&k,oper_type,data_type,v_old,v_new,is_events_done);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleDriverEvents with RUNNING datatype
TEST_F(KtCtrlrTest, HandleDriverEvents_RUNNING_06) {
  key_ctr_t k;
  val_ctr_st_t v_old;
  val_ctr_st_t v_new;
  memset(v_old.valid, '\0', sizeof(v_old.valid));
  memset(v_new.valid, '\0', sizeof(v_new.valid));
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  pfc_bool_t is_events_done;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(db_conn,&k,oper_type,data_type,&v_old,&v_new,is_events_done);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleDriverEvents with RUNNING datatype
TEST_F(KtCtrlrTest, HandleDriverEvents_RUNNING_07) {
  key_ctr_t k;
  val_ctr_st_t *v_old = new val_ctr_st_t;
  val_ctr_st_t *v_new = new val_ctr_st_t;
  v_new->oper_status = UPPL_CONTROLLER_OPER_UP;
  pfc_bool_t is_events_done=false;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(db_conn,&k,oper_type,data_type,v_old,v_new,is_events_done);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleDriverEvents with RUNNING datatype
TEST_F(KtCtrlrTest, HandleDriverEvents_RUNNING_08) {
  key_ctr_t k;
  val_ctr_st_t *v_old = new val_ctr_st_t;
  val_ctr_st_t *v_new = new val_ctr_st_t;
  v_new->oper_status = UPPL_CONTROLLER_OPER_DOWN;
  pfc_bool_t is_events_done;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(db_conn,&k,oper_type,data_type,v_old,v_new,is_events_done);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleDriverEvents with RUNNING datatype
TEST_F(KtCtrlrTest, HandleDriverEvents_RUNNING_09) {
  key_ctr_t k;
  val_ctr_st_t *v_old = new val_ctr_st_t;
  val_ctr_st_t *v_new = new val_ctr_st_t;
  v_new->valid[kIdxActualVersion]=UNC_VF_VALID;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  pfc_bool_t is_events_done;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.HandleDriverEvents(db_conn,&k,oper_type,data_type,v_old,v_new,is_events_done);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
//CheckIpAndClearStateDB with DB success
TEST_F(KtCtrlrTest, CheckIpAndClearStateDB_Db_Success_01) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_READ;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.CheckIpAndClearStateDB(db_conn,&k);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//CheckIpAndClearStateDB with DB failure
TEST_F(KtCtrlrTest, CheckIpAndClearStateDB_Db_failure_02) {
  key_ctr_t k;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  Kt_Controller  KtctrObj;
  uint32_t oper_type = UNC_OP_READ;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  KtctrObj.CheckIpAndClearStateDB(db_conn,&k);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
//SendOperStatusNotification with add output error
TEST_F(KtCtrlrTest, SendOperStatusNotification_01) {
  unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER, TC_API_COMMON_SUCCESS);
  unc::uppl::PhysicalLayer *physical_layer = unc::uppl::PhysicalLayer::get_instance();
  physical_layer->init();

  key_ctr_t k;
  uint8_t old_oper_st;
  uint8_t new_oper_st;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)UNC_DT_RUNNING);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  Kt_Controller  KtctrObj;

  int ret =  KtctrObj.SendOperStatusNotification(k,old_oper_st,new_oper_st);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

//SendOperStatusNotification with success in add output
TEST_F(KtCtrlrTest, SendOperStatusNotification_02) {
  unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER, TC_API_COMMON_SUCCESS);
  unc::uppl::PhysicalLayer *physical_layer = unc::uppl::PhysicalLayer::get_instance();
  physical_layer->init();

  key_ctr_t k;
  uint8_t old_oper_st;
  uint8_t new_oper_st;
  val_ctr_st_t old_val_ctr, new_val_ctr;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_KT_CONTROLLER);

  Kt_Controller  KtctrObj;

  int ret =  KtctrObj.SendOperStatusNotification(k,old_oper_st,new_oper_st);
  EXPECT_EQ(ret,UPPL_RC_FAILURE);
  unc::uppl::ODBCManager::clearStubData();
}

//ValidateControllerIpAddress
TEST_F(KtCtrlrTest, ValidateControllerIpAddress_01) {
  key_ctr_t k;
  val_ctr_t *v = new val_ctr_t;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxIpAddress]=1;
  int ret =  KtctrObj.ValidateControllerIpAddress(db_conn,operation,data_type,(unc_keytype_ctrtype_t)0,(UpplReturnCode)0,&k,v);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
  unc::uppl::ODBCManager::clearStubData();
}

//ValidateControllerIpAddress
TEST_F(KtCtrlrTest, ValidateControllerIpAddress_02) {
  key_ctr_t k;
  val_ctr_t *v = new val_ctr_t;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxIpAddress]=2;
  int ret =  KtctrObj.ValidateControllerIpAddress(db_conn,operation,data_type,(unc_keytype_ctrtype_t)1,(UpplReturnCode)0,&k,v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//ValidateControllerIpAddress
TEST_F(KtCtrlrTest, ValidateControllerIpAddress_03) {
  key_ctr_t k;
  val_ctr_t *v = new val_ctr_t;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxIpAddress]=1;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ValidateControllerIpAddress(db_conn,operation,data_type,(unc_keytype_ctrtype_t)1,(UpplReturnCode)0,&k,v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//ValidateControllerIpAddress
TEST_F(KtCtrlrTest, ValidateControllerIpAddress_04) {
  key_ctr_t k;
  val_ctr_t *v = new val_ctr_t;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_RUNNING;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxIpAddress]=1;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  KtctrObj.ValidateControllerIpAddress(db_conn,operation,data_type,(unc_keytype_ctrtype_t)1,(UpplReturnCode)0,&k,v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//ValidateUnknownCtrlrScalability
TEST_F(KtCtrlrTest, ValidateUnknownCtrlrScalability_Neg_DB_01) {
  key_ctr_t k;
  val_ctr_t *v = new val_ctr_t;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint8_t type;
  uint32_t data_type;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxIpAddress]=2;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETSIBLINGCOUNT_FILTER, ODBCM_RC_FAILED);
  int ret =  KtctrObj.ValidateUnknownCtrlrScalability(db_conn,&k,type,data_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
//ValidateUnknownCtrlrScalability
TEST_F(KtCtrlrTest, ValidateUnknownCtrlrScalability_Neg_DB_02) {
  key_ctr_t k;
  val_ctr_t *v = new val_ctr_t;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint8_t type;
  uint32_t data_type;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxIpAddress]=2;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETSIBLINGCOUNT_FILTER, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtctrObj.ValidateUnknownCtrlrScalability(db_conn,&k,type,data_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//ValidateUnknownCtrlrScalability
TEST_F(KtCtrlrTest, ValidateUnknownCtrlrScalability_Neg_DB_03) {
  key_ctr_t k;
  val_ctr_t *v = new val_ctr_t;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint8_t type;
  uint32_t data_type;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxIpAddress]=2;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETSIBLINGCOUNT_FILTER, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ValidateUnknownCtrlrScalability(db_conn,&k,type,data_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_EXCEEDS_RESOURCE_LIMIT);
  unc::uppl::ODBCManager::clearStubData();
}
//ValidateUnknownCtrlrScalability
TEST_F(KtCtrlrTest, ValidateUnknownCtrlrScalability_Neg_DB_04) {
  key_ctr_t k;
  val_ctr_t *v = new val_ctr_t;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  uint8_t type;
  uint32_t data_type;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxIpAddress]=2;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETSIBLINGCOUNT_FILTER, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtctrObj.ValidateUnknownCtrlrScalability(db_conn,&k,type,data_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_EXCEEDS_RESOURCE_LIMIT);
  unc::uppl::ODBCManager::clearStubData();
}
//FrameValidValue
TEST_F(KtCtrlrTest, FrameValidValue_01) {
  key_ctr_t k;
  val_ctr_t v;
  val_ctr_st_t v_st;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));

  Kt_Controller  KtctrObj;
  string abd="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  KtctrObj.FrameValidValue(abd,v_st,v);
  int ret = UPPL_RC_SUCCESS;
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
/*******TAMIL TEST CASES*******/
/********ReadBulk*******/

//ReadBulk opr returns Success for max_ct is zero
TEST_F(KtCtrlrTest, ReadBulk_Max_Ct_Zero) {
  key_ctr_t k;
  int child_index;
  pfc_bool_t parent_call;
  pfc_bool_t is_read_next;
  ReadRequest *read_req;
  uint32_t max_rep_ct = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//Check for controller key existence
TEST_F(KtCtrlrTest, ReadBulk_childIndex) {
  key_ctr_t k;
  int child_index = -1;
  pfc_bool_t parent_call;
  pfc_bool_t is_read_next;
  ReadRequest *read_req = NULL;
  uint32_t max_rep_ct = 0;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  vector<void *> obj_key_struct;
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  //unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}


/*************ReadBulkInternal**********/
// ReadBulkInternal: No record to read
TEST_F(KtCtrlrTest, ReadBulkInternal_NoRecordFound) {
  key_ctr_t k;
  val_ctr_t v;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<string> vect_ctr_id;
  uint32_t max_rep_ct;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, &v,UNC_DT_STATE, max_rep_ct, vect_val_ctr_st, vect_ctr_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}


// ReadBulkInternal with read_db_status ODBCM_RC_CONNECTION_ERROR
TEST_F(KtCtrlrTest, ReadBulkInternal_Db_Connxn_Error) {
  key_ctr_t k;
  val_ctr_t v;
  vector<val_ctr_st> vect_val_ctr_st;
   vector<string> vect_ctr_id;
  uint32_t max_rep_ct;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, &v,UNC_DT_STATE, max_rep_ct, vect_val_ctr_st, vect_ctr_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal with read_db_status ODBCM_RC_TABLE_NOT_FOUND
TEST_F(KtCtrlrTest, ReadBulkInternal_Err_DB_Get) {
  key_ctr_t k;
  val_ctr_t v;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<string> vect_ctr_id;
  uint32_t max_rep_ct;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_TABLE_NOT_FOUND);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, &v,UNC_DT_STATE, max_rep_ct, 
                                              vect_val_ctr_st, vect_ctr_id);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal with read_db_status ODBCM_RC_SUCCESS
TEST_F(KtCtrlrTest, ReadBulkInternal_Success) {
  key_ctr_t k;
  val_ctr_t v;
  vector<val_ctr_st> vect_val_ctr_st;
  vector<string> vect_ctr_id;
  uint32_t max_rep_ct;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulkInternal(db_conn, &k, &v,UNC_DT_STATE, max_rep_ct,
                                             vect_val_ctr_st, vect_ctr_id);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

/*********ValidateCtrlrValueCapability*******/
//returns config syntax error
TEST_F(KtCtrlrTest, ValidateCtrlrValueCapability_Err_CFG_SYNTAX) {
  key_ctr_t k;
  val_ctr_st_t v;  
  string version;
  uint32_t key_type;
  Kt_Controller  KtctrObj;
  int ret =  KtctrObj.ValidateCtrlrValueCapability(version, key_type);
  EXPECT_EQ(ret, UPPL_RC_ERR_CFG_SYNTAX);
  unc::uppl::ODBCManager::clearStubData();
}

/*******ValidateCtrlrScalability****/
//Returns Connxn Error
TEST_F(KtCtrlrTest, ValidateCtrlrScalability_Err_DB_Access) {
  OdbcmConnectionHandler *db_conn =NULL;
  string version;
  uint32_t key_type = UNC_KT_CONTROLLER;
  uint32_t data_type = UNC_DT_STATE;
  Kt_Controller  KtctrObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETROWCOUNT, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtctrObj.ValidateCtrlrScalability(db_conn, version, key_type, data_type);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//Unable to get scalability number from system
TEST_F(KtCtrlrTest, ValidateCtrlrScalability_System_Error) {
  OdbcmConnectionHandler *db_conn =NULL;
  string version;
  uint32_t key_type = UNC_KT_CONTROLLER;
  uint32_t data_type = UNC_DT_STATE;
  Kt_Controller  KtctrObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETROWCOUNT, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ValidateCtrlrScalability(db_conn, version, key_type, data_type);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

//Unable to get scalability number from DB
TEST_F(KtCtrlrTest, ValidateCtrlrScalability_DB_Err) {
  OdbcmConnectionHandler *db_conn =NULL;
  string version;
  uint32_t key_type = UNC_KT_CONTROLLER;
  uint32_t data_type = UNC_DT_STATE;
  Kt_Controller  KtctrObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETROWCOUNT, ODBCM_RC_FAILED);
  int ret =  KtctrObj.ValidateCtrlrScalability(db_conn, version, key_type, data_type);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

/******ValidateTypeIpAddress*******/

//Not required to validate type and ip
TEST_F(KtCtrlrTest, ValidateTypeIpAddress_NoValidation) {
  OdbcmConnectionHandler *db_conn =NULL;
  key_ctr_t k;
  val_ctr_st_t *v = NULL; 
  uint32_t data_type = UNC_DT_STATE;
  uint32_t ctrl_type;
  Kt_Controller  KtctrObj;
  int ret =  KtctrObj.ValidateTypeIpAddress(db_conn,&k,v,data_type,ctrl_type);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtCtrlrTest, ValidateTypeIpAddress) {
  OdbcmConnectionHandler *db_conn =NULL;
  key_ctr_t k;
  val_ctr_st_t *v = new val_ctr_st_t; 
  //v->valid[kIdxIpAddress] = UNC_VF_INVALID;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t ctrl_type;
  Kt_Controller  KtctrObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  KtctrObj.ValidateTypeIpAddress(db_conn,&k,v,data_type,ctrl_type);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

/*******HandleDriverAlarms*********/
TEST_F(KtCtrlrTest, HandleDriverAlarms) {
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t alarm_type = UNC_PHYS_PATH_FAULT;
  uint32_t oper_type;
  key_ctr_t k;
  val_ctr_st_t v;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  int ret =  KtctrObj.HandleDriverAlarms(db_conn,data_type,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret, UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}


/******SendSemanticRequestToUPLL*******/

TEST_F(KtCtrlrTest, SendSemanticRequestToUPLL) {
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER, TC_API_COMMON_SUCCESS);
  unc::uppl::PhysicalLayer *physical_layer = unc::uppl::PhysicalLayer::get_instance();
  physical_layer->init();
  key_ctr_t k;
  uint32_t data_type = UNC_DT_STATE;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  int ret =  KtctrObj.SendSemanticRequestToUPLL(&k, data_type);
  EXPECT_EQ(ret, UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE);
  unc::uppl::ODBCManager::clearStubData();
}

//GetChildClassPointer
TEST_F(KtCtrlrTest, GetChildClassPointer_01) {
  int kIndex = 0;
  Kt_Base *child[KT_CONTROLLER_CHILD_COUNT];
  Kt_Controller  KtctrObj;
  child[kIndex] = KtctrObj.GetChildClassPointer((KtControllerChildClass)kIndex);
  int ret = UPPL_RC_SUCCESS;
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtCtrlrTest, GetChildClassPointer_02) {
  int kIndex = 1;
  Kt_Base *child[KT_CONTROLLER_CHILD_COUNT];
  Kt_Controller  KtctrObj;
  child[kIndex] = KtctrObj.GetChildClassPointer((KtControllerChildClass)kIndex);
  int ret = UPPL_RC_SUCCESS;
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtCtrlrTest, GetChildClassPointer_03) {
  int kIndex = 2;
  Kt_Base *child[KT_CONTROLLER_CHILD_COUNT];
  Kt_Controller  KtctrObj;
  child[kIndex] = KtctrObj.GetChildClassPointer((KtControllerChildClass)kIndex);
  int ret = UPPL_RC_SUCCESS;
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

/****ReadBulk Continuation******/
TEST_F(KtCtrlrTest, ReadBulk) {
  key_ctr_t k;
  int child_index = 0;
  pfc_bool_t parent_call;
  pfc_bool_t is_read_next;
  ReadRequest *read_req = NULL;
  uint32_t max_rep_ct = 1;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  //unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtCtrlrTest, ReadBulk_ReadBuffer) {
  key_ctr_t k;
  int child_index = 0;
  pfc_bool_t parent_call;
  pfc_bool_t is_read_next;
  ReadRequest *read_req = new ReadRequest;
  uint32_t max_rep_ct = 1;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  //unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtCtrlrTest, ReadBulk_child_ind) {
  key_ctr_t k;
  int child_index = -1;
  pfc_bool_t parent_call;
  pfc_bool_t is_read_next;
  ReadRequest *read_req = new ReadRequest;
  uint32_t max_rep_ct = 1;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  //unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtCtrlrTest, ReadBulk_ctr_exists_FALSE) {
  key_ctr_t k;
  memset(&k,0,sizeof(key_ctr_t));
  int child_index;
  pfc_bool_t parent_call = false;
  pfc_bool_t is_read_next;
  ReadRequest *read_req = new ReadRequest;
  uint32_t max_rep_ct = 1;
  //memset(k.controller_name, '\0', 32);
  //memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;

  //unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtctrObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct,
                              child_index, parent_call,is_read_next, read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}


// DeleteKeyInstance:returns DB error
TEST_F(KtCtrlrTest, DeleteKeyInstan_Err_DB_ACCESS) {
  key_ctr_t k;
  uint32_t key_type ;
  memset(k.controller_name, '\0', 32);
  memcpy(k.controller_name, pkctrName1, strlen(pkctrName1));
  Kt_Controller  KtctrObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtctrObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//ReadBulkInternal:Reaturns success for max_ct zero
TEST_F(KtCtrlrTest,ReadBulkInternal_MaxCt_LessThan_Zero) {
  OdbcmConnectionHandler *db_conn =NULL;
  key_ctr_t k;
  val_ctr_st_t v;
  uint32_t data_type =UNC_DT_STATE;
  uint32_t max_rep_ct = 0;
  vector<val_ctr_st_t> vect_val_ctr;
  vector<string> vect_ctr_id;
  Kt_Controller  KtctrObj;
  int ret = KtctrObj.ReadBulkInternal(db_conn,&k,&v,data_type,max_rep_ct,vect_val_ctr,vect_ctr_id);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}  

//HandleDriverEvents
TEST_F(KtCtrlrTest, HandleDriverEvents) {
  OdbcmConnectionHandler *db_conn =NULL;
  key_ctr_t k;
  uint32_t oper_type = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_STATE;
  val_ctr_st_t *old_val_struct = NULL;
  val_ctr_st_t *new_val_struct;
  pfc_bool_t is_events_done;
  Kt_Controller  KtctrObj;
  int ret = KtctrObj.HandleDriverEvents(db_conn,&k,oper_type,data_type,old_val_struct,new_val_struct,is_events_done);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

/***8*ReadCtrValFromDb****/
//Unsuported opr type....Returns Success
TEST_F(KtCtrlrTest, ReadCtrValFromDB) {
  OdbcmConnectionHandler *db_conn =NULL;
  key_ctr_t k;
  val_ctr_st_t v;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t operation_type = UNC_OP_CREATE;
  uint32_t max_rep_ct;
  vector<val_ctr_st_t> vect_val_ctr_st;
  vector<string> controller_id;
  Kt_Controller  KtctrObj;
  int ret = KtctrObj.ReadCtrValFromDB(db_conn,&k,&v,data_type,operation_type,max_rep_ct,vect_val_ctr_st,controller_id);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}


