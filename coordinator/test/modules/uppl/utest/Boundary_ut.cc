/*
 * Copyright (c) 2012-2013 NEC Corporation
 *           * All rights reserved.
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
#include "PhysicalLayerStub.hh"
#include "itc_read_request.hh"
#include "tclib_module.hh"
#include "physical_common_def.hh"
#include "unc/uppl_common.h"
#include "unc/keytype.h"
#include "unc/upll_svc.h"
#include "itc_kt_boundary.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_port.hh"
#include "itc_kt_link.hh"
#include "ipct_util.hh"


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

char pkempty[] = "";
char pkName1[] = "Boundary1";
char pkName2[] = "Boundary2";
char pkName3[] = "Boundary3";
char pkName4[] = "Boundary4";
char pkName5[] = "Boundary5";
char pkNameNonExistent[] = "NonExistent";

void getKeyForKtBoundary0(key_boundary_t& k) {
  memset(k.boundary_id, '\0', 32);
}

void getKeyForKtBoundary1(key_boundary_t& k) {
  memset(k.boundary_id, '\0', 32);
  memcpy(k.boundary_id, pkName1, strlen(pkName1));
}

void getKeyForKtBoundary2(key_boundary_t& k) {
  memset(k.boundary_id, '\0', 32);
  memcpy(k.boundary_id, pkName2, strlen(pkName2));
}

void getKeyForKtBoundary3(key_boundary_t& k) {
  memset(k.boundary_id, '\0', 32);
  memcpy(k.boundary_id, pkName3, strlen(pkName3));
}

void getKeyForKtBoundary4(key_boundary_t& k) {
  memset(k.boundary_id, '\0', 32);
  memcpy(k.boundary_id, pkName4, strlen(pkName4));
}

void getKeyForKtBoundary5(key_boundary_t& k) {
  memset(k.boundary_id, '\0', 32);
  memcpy(k.boundary_id, pkName5, strlen(pkName5));
}


void getValForKtBoundary1(val_boundary_t& v) {
 
  memset(v.description, '\0', 128);
  memcpy(v.description, "boundary description", strlen("boundary description"));

  
  memset(v.controller_name1, '\0', 32);
  memcpy(v.controller_name1, "Controller5", strlen("Controller5"));

 
  memset(v.domain_name1, '\0', 32);
  memcpy(v.domain_name1, "domain1", strlen("domain1"));

 
  memset(v.logical_port_id1, '\0', 320);
  memcpy(v.logical_port_id1, "{0xab,0xc}", strlen("{0xab,0xc}"));


  memset(v.controller_name2, '\0', 32);
  memcpy(v.controller_name2, "Controller7", strlen("Controller7"));

  
  memset(v.domain_name2, '\0', 32);
  memcpy(v.domain_name2, "domain2", strlen("domain2"));

  memset(v.logical_port_id2, '\0', 320);
  memcpy(v.logical_port_id2, "{0xcd,0xe}", strlen("{0xcd,0xe}"));

  memset(v.valid, 1, 7);  // uint8_t valid[7];
  v.cs_row_status = 0;  // uint8_t cs_row_status;
  memset(v.cs_attr, 1, 7);  // uint8_t cs_attr[7]
  v.cs_attr[6] =  '\0';
}

void getValForKtBoundary2(val_boundary_t& v) {

  memset(v.description, '\0', 128);
  memcpy(v.description, "bndry2 description", strlen("bndry2 description"));


  memset(v.controller_name1, '\0', 32);
  memcpy(v.controller_name1, "Controller5", strlen("Controller5"));


  memset(v.domain_name1, '\0', 32);
  memcpy(v.domain_name1, "domain4", strlen("domain4"));

  
  memset(v.logical_port_id1, '\0', 320);
  memcpy(v.logical_port_id1, "{0x9a,0xa}", strlen("{0x9a,0xa}"));


  memset(v.controller_name2, '\0', 32);
  memcpy(v.controller_name2, "Controller7", strlen("Controller7"));

  memset(v.domain_name2, '\0', 32);
  memcpy(v.domain_name2, "domain5", strlen("domain5"));

  memset(v.logical_port_id2, '\0', 320);
  memcpy(v.logical_port_id2, "{0xef,0xf}", strlen("{0xef,0xf}"));

  memset(v.valid, 1, 7);  // uint8_t valid[7];
  v.cs_row_status = 0;  // uint8_t cs_row_status;
  memset(v.cs_attr, 1, 7);  // uint8_t cs_attr[7]
  v.cs_attr[6] =  '\0';
}

void getValForKtBoundary3(val_boundary_t& v) {
  memset(v.description, '\0', 128);
  memcpy(v.description, "", strlen(""));  // uint8_t description[128];

  memset(v.controller_name1, '\0', 32);
  memcpy(v.controller_name1, "", strlen(""));  // uint8_t controller_name1[32];

  memset(v.domain_name1, '\0', 32);
  memcpy(v.domain_name1, "", strlen(""));  // uint8_t domain_name1[32];

  memset(v.logical_port_id1, '\0', 320);
  memcpy(v.logical_port_id1, "", strlen(""));  // uint8_t logical_port_id1[320];

  memset(v.controller_name2, '\0', 32);
  memcpy(v.controller_name2, "", strlen(""));  // uint8_t controller_name1[32];

  memset(v.domain_name2, '\0', 32);
  memcpy(v.domain_name2, "", strlen(""));  // uint8_t domain_name2[32];

  memset(v.logical_port_id2, '\0', 320);
  memcpy(v.logical_port_id2, "", strlen(""));  // uint8_t logical_port_id2[320];

  memset(v.valid, '\0', 7);  // uint8_t valid[7];
  v.cs_row_status = 0;  // uint8_t cs_row_status;
  memset(v.cs_attr, '\0', 7);  // uint8_t cs_attr[7];
}

void getValForKtBoundary5(val_boundary_t& v) {
 
  memset(v.description, '\0', 128);
  memcpy(v.description, "boundary description", strlen("boundary description"));

  
  memset(v.controller_name1, '\0', 32);
  memcpy(v.controller_name1, "Controller5", strlen("Controller5"));

 
  memset(v.domain_name1, '\0', 32);
  memcpy(v.domain_name1, "domain1", strlen("domain1"));

 
  memset(v.logical_port_id1, '\0', 320);
  memcpy(v.logical_port_id1, "{0xab,0xc}", strlen("{0xab,0xc}"));


  memset(v.controller_name2, '\0', 32);
  memcpy(v.controller_name2, "Controller5", strlen("Controller5"));

  
  memset(v.domain_name2, '\0', 32);
  memcpy(v.domain_name2, "domain1", strlen("domain1"));

  memset(v.logical_port_id2, '\0', 320);
  memcpy(v.logical_port_id2, "{0xcd,0xe}", strlen("{0xcd,0xe}"));

  memset(v.valid, 1, 7);  // uint8_t valid[7];
  v.cs_row_status = 0;  // uint8_t cs_row_status;
  memset(v.cs_attr, 1, 7);  // uint8_t cs_attr[7]
  v.cs_attr[6] =  '\0';
}
// Create for unsupported datatype
TEST_F(KtClassTest, Create_01) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  PhysicalLayerStub::loadphysicallayer();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_STATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

// Boundary  Create success 
TEST_F(KtClassTest, Create_02) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
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
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// Boundary  Create Server session addOutput failed 
TEST_F(KtClassTest, Create_03) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_CREATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Create(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateOneRow returns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, CreateKeyInstance_01) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t key_type = 1;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_STATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateOneRow returns ODBCM_RC_PKEY_VIOLATION
TEST_F(KtClassTest, CreateKeyInstance_02) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t key_type = 1;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_PKEY_VIOLATION);
  int ret =  KtboundaryObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_STATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_INSTANCE_EXISTS);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateOneRow returns ODBCM_RC_QUERY_FAILED
TEST_F(KtClassTest, CreateKeyInstance_03) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t key_type = 1;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_QUERY_FAILED);
  int ret =  KtboundaryObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_STATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_CREATE);
  unc::uppl::ODBCManager::clearStubData();
}

// CreateOneRow returns ODBCM_RC_QUERY_FAILED
TEST_F(KtClassTest, CreateKeyInstance_04) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t key_type = 1;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::CREATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.CreateKeyInstance(db_conn,&k,&v,UNC_DT_STATE,key_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// Update for unsupported datatype
TEST_F(KtClassTest, Update_01) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_STATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

// Boundary  Update success 
TEST_F(KtClassTest, Update_02) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// Boundary  Update Server session addOutput failed 
TEST_F(KtClassTest, Update_03) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// UpdateOneRow returns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, Update_04) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t key_type = 1;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_DB_ACCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// UpdateOneRow returns ODBCM_RC_PKEY_VIOLATION
TEST_F(KtClassTest, Update_05) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t key_type = 1;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_DB_UPDATE);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_PKEY_VIOLATION);
  int ret =  KtboundaryObj.Update(db_conn,session_id,configuration_id,&k,&v,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// ReadBulkInternal GetBulkRows returns ODBCM_RC_RECORD_NOT_FOUND 
TEST_F(KtClassTest, ReadBulkInternal_01) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtboundaryObj.ReadBulkInternal(db_conn,&k,&v,UNC_DT_STATE,max_rep_ct,vect_key_boundary,vect_val_boundary);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal GetBulkRows returns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, ReadBulkInternal_02) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.ReadBulkInternal(db_conn,&k,&v,UNC_DT_STATE,max_rep_ct,vect_key_boundary,vect_val_boundary);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulkInternal GetBulkRows returns ODBCM_RC_QUERY_FAILED 
TEST_F(KtClassTest, ReadBulkInternal_03) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_QUERY_FAILED);
  int ret =  KtboundaryObj.ReadBulkInternal(db_conn,&k,&v,UNC_DT_STATE,max_rep_ct,vect_key_boundary,vect_val_boundary);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
// ReadBulkInternal GetBulkRows returns ODBCM_RC_SUCCESS
TEST_F(KtClassTest, ReadBulkInternal_04) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.ReadBulkInternal(db_conn,&k,&v,UNC_DT_STATE,max_rep_ct,vect_key_boundary,vect_val_boundary);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// Boundary  Delete on unsupported data type server sessio failed
TEST_F(KtClassTest, Delete_01) {
  key_boundary_t k;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ServerSession::clearStubData();
  unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER, TC_API_COMMON_SUCCESS);
  unc::uppl::PhysicalLayer *physical_layer = unc::uppl::PhysicalLayer::get_instance();
  physical_layer->init();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)ODBCM_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_RUNNING);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_RUNNING,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

// Boundary  Delete on unsupported data type server session pass
TEST_F(KtClassTest, Delete_02) {
  key_boundary_t k;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  ses.stub_setAddOutput((uint32_t)UNC_DT_RUNNING);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_RUNNING,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
#if 0
// DeleteOneRow returns ODBCM_RC_SUCCESS
TEST_F(KtClassTest, Delete_03) {
  key_boundary_t k;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER, TC_API_COMMON_SUCCESS);
  unc::uppl::PhysicalLayer *physical_layer = unc::uppl::PhysicalLayer::get_instance();
  physical_layer->init();
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UPLL_IS_KEY_TYPE_IN_USE_OP);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteOneRow returns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, Delete_04) {
  key_boundary_t k;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_DB_ACCESS);
  ses.stub_setAddOutput((uint32_t)UPLL_IS_KEY_TYPE_IN_USE_OP);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteOneRow returns ODBCM_RC_ROW_NOT_EXISTS
TEST_F(KtClassTest, Delete_05) {
  key_boundary_t k;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// DeleteOneRow returns ODBCM_RC_FAILED
TEST_F(KtClassTest, Delete_06) {
  key_boundary_t k;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_ERR_DB_DELETE);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_FAILED);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
#endif
// DeleteOneRow returns ODBCM_RC_FAILED
TEST_F(KtClassTest, Delete_07) {
  key_boundary_t k;
  uint32_t session_id = 1;
  uint32_t configuration_id = 2;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)configuration_id);
  ses.stub_setAddOutput((uint32_t)session_id);
  ses.stub_setAddOutput((uint32_t)UNC_OP_DELETE);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)0);
  ses.stub_setAddOutput((uint32_t)UPPL_RC_SUCCESS);
  ses.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_FAILED);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.Delete(db_conn,session_id,configuration_id,&k,UNC_DT_CANDIDATE,ses);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
// ReadInternal success
TEST_F(KtClassTest, ReadInternal_01) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  vector<void *> boundary_key;
  vector<void *> boundary_val;
  boundary_key.push_back(&k);
  boundary_val.push_back(&v);
  uint32_t operation_type = UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.ReadInternal(db_conn,boundary_key,boundary_val,UNC_DT_STATE,operation_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadInternal operation_type != UNC_OP_READ
TEST_F(KtClassTest, ReadInternal_02) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  vector<void *> boundary_key;
  vector<void *> boundary_val;
  boundary_key.push_back(&k);
  boundary_val.push_back(&v);
  uint32_t operation_type = UNC_OP_CREATE;
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.ReadInternal(db_conn,boundary_key,boundary_val,UNC_DT_STATE,operation_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// ReadBoundaryValFromDB returns failure
TEST_F(KtClassTest, ReadInternal_03) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  vector<void *> boundary_key;
  vector<void *> boundary_val;
  boundary_key.push_back(&k);
  boundary_val.push_back(&v);
  uint32_t operation_type = UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.ReadInternal(db_conn,boundary_key,boundary_val,UNC_DT_STATE,operation_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulk 
TEST_F(KtClassTest, ReadBulk_01) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct = 2;
  int child_index = 1;
  ReadRequest *read_req  = NULL;
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.ReadBulk(db_conn,&k,UNC_DT_IMPORT,max_rep_ct,child_index,(pfc_bool_t)false,(pfc_bool_t)false,read_req);
  EXPECT_EQ(ret,UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}

// ReadBulk 
TEST_F(KtClassTest, ReadBulk_02) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct = 0;
  int child_index = 1;
  ReadRequest *read_req  = NULL;
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.ReadBulk(db_conn,&k,UNC_DT_CANDIDATE,max_rep_ct,child_index,(pfc_bool_t)false,(pfc_bool_t)false,read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// ReadBulk 
TEST_F(KtClassTest, ReadBulk_03) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct = 1;
  int child_index = 1;
  ReadRequest *read_req  = new ReadRequest;
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.ReadBulk(db_conn,&k,UNC_DT_CANDIDATE,max_rep_ct,child_index,(pfc_bool_t)false,(pfc_bool_t)false,read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulk 
TEST_F(KtClassTest, ReadBulk_04) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct = 2;
  int child_index = 1;
  ReadRequest *read_req  = new ReadRequest;
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.ReadBulk(db_conn,&k,UNC_DT_STATE,max_rep_ct,child_index,(pfc_bool_t)false,(pfc_bool_t)false,read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulk 
TEST_F(KtClassTest, ReadBulk_05) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct = 1;
  int child_index = 1;
  ReadRequest *read_req  = new ReadRequest;
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.ReadBulk(db_conn,&k,UNC_DT_STATE,max_rep_ct,child_index,(pfc_bool_t)false,(pfc_bool_t)false,read_req);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// ReadBulk 
TEST_F(KtClassTest, ReadBulk_06) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct = 1;
  int child_index = 1;
  ReadRequest *read_req  = new ReadRequest;
  Kt_Boundary  KtboundaryObj;
  ServerSession ses;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  KtboundaryObj.ReadBulk(db_conn,&k,UNC_DT_STATE,max_rep_ct,child_index,(pfc_bool_t)false,(pfc_bool_t)false,read_req);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSyntaxValidation returns success
TEST_F(KtClassTest, PerformSyntaxValidation_01) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  uint32_t operation_type = UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.PerformSyntaxValidation(db_conn,&k,&v,operation_type,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// PerformSyntaxValidation without boundary name
TEST_F(KtClassTest, PerformSyntaxValidation_02) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary0(k);
  uint32_t operation_type = UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.PerformSyntaxValidation(db_conn,&k,&v,operation_type,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

// PerformSyntaxValidation without controller name1
TEST_F(KtClassTest, PerformSyntaxValidation_03) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.controller_name1, '\0', 32);
  uint32_t operation_type = UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.PerformSyntaxValidation(db_conn,&k,&v,operation_type,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}
// PerformSyntaxValidation without controller name2
TEST_F(KtClassTest, PerformSyntaxValidation_04) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.controller_name2, '\0', 32);
  uint32_t operation_type = UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.PerformSyntaxValidation(db_conn,&k,&v,operation_type,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

// PerformSyntaxValidation without domain name1
TEST_F(KtClassTest, PerformSyntaxValidation_05) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.domain_name1, '\0', 32);
  uint32_t operation_type = UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.PerformSyntaxValidation(db_conn,&k,&v,operation_type,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}
// PerformSyntaxValidation without domain name2
TEST_F(KtClassTest, PerformSyntaxValidation_06) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.domain_name2, '\0', 32);
  uint32_t operation_type = UNC_OP_READ;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.PerformSyntaxValidation(db_conn,&k,&v,operation_type,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}
// PerformSyntaxValidation returns success
TEST_F(KtClassTest, PerformSyntaxValidation_07) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  uint32_t operation_type = UNC_OP_UPDATE;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.PerformSyntaxValidation(db_conn,&k,&v,operation_type,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

// PerformSyntaxValidation returns success
TEST_F(KtClassTest, PerformSyntaxValidation_08) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  uint32_t operation_type = UNC_OP_READ_SIBLING_BEGIN;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.PerformSyntaxValidation(db_conn,&k,&v,operation_type,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// GETONEROW returns ODBCM_RC_RECORD_NOT_FOUND 
TEST_F(KtClassTest, ReadBoundaryValFromDB_01) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtboundaryObj.ReadBoundaryValFromDB(db_conn,&k,&v,UNC_DT_STATE,UNC_OP_READ,max_rep_ct,vect_key_boundary,vect_val_boundary,false);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// GETONEROW returns ODBCM_RC_CONNECTION_ERROR 
TEST_F(KtClassTest, ReadBoundaryValFromDB_02) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.ReadBoundaryValFromDB(db_conn,&k,&v,UNC_DT_STATE,UNC_OP_READ,max_rep_ct,vect_key_boundary,vect_val_boundary,false);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// GETONEROW returns ODBCM_RC_FAILED 
TEST_F(KtClassTest, ReadBoundaryValFromDB_03) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtboundaryObj.ReadBoundaryValFromDB(db_conn,&k,&v,UNC_DT_STATE,UNC_OP_READ,max_rep_ct,vect_key_boundary,vect_val_boundary,false);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// GETBULKROWS returns ODBCM_RC_RECORD_NOT_FOUND 
TEST_F(KtClassTest, ReadBoundaryValFromDB_04) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtboundaryObj.ReadBoundaryValFromDB(db_conn,&k,&v,UNC_DT_STATE,UNC_OP_READ_NEXT,max_rep_ct,vect_key_boundary,vect_val_boundary,false);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// GETBULKROWS returns ODBCM_RC_CONNECTION_ERROR 
TEST_F(KtClassTest, ReadBoundaryValFromDB_05) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.ReadBoundaryValFromDB(db_conn,&k,&v,UNC_DT_STATE,UNC_OP_READ_NEXT,max_rep_ct,vect_key_boundary,vect_val_boundary,false);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// GETBULKROWS returns ODBCM_RC_FAILED 
TEST_F(KtClassTest, ReadBoundaryValFromDB_06) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  KtboundaryObj.ReadBoundaryValFromDB(db_conn,&k,&v,UNC_DT_STATE,UNC_OP_READ_NEXT,max_rep_ct,vect_key_boundary,vect_val_boundary,false);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

//  operation_type < UNC_OP_READ
TEST_F(KtClassTest, ReadBoundaryValFromDB_07) {
  key_boundary_t k;
  val_boundary_t v;
  uint32_t max_rep_ct = 2;
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary;
  vect_key_boundary.clear();
  vect_val_boundary.clear();
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.ReadBoundaryValFromDB(db_conn,&k,&v,UNC_DT_STATE,UNC_OP_CREATE,max_rep_ct,vect_key_boundary,vect_val_boundary,false);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

// GetModifiedRows returns ODBCM_RC_RECORD_NOT_FOUND 
TEST_F(KtClassTest, GetModifiedRows_01) {
  vector<void *> obj_key_struct;
  obj_key_struct.clear();
  CsRowStatus row_status = UPDATED; 
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtboundaryObj.GetModifiedRows(db_conn,obj_key_struct,row_status);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// GetModifiedRows returns ODBCM_RC_CONNECTION_ERROR 
TEST_F(KtClassTest, GetModifiedRows_02) {
  vector<void *> obj_key_struct;
  obj_key_struct.clear();
  CsRowStatus row_status = UPDATED; 
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.GetModifiedRows(db_conn,obj_key_struct,row_status);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// GetModifiedRows returns ODBCM_RC_SUCCESS 
TEST_F(KtClassTest, GetModifiedRows_03) {
  vector<void *> obj_key_struct;
  obj_key_struct.clear();
  CsRowStatus row_status = UPDATED; 
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.GetModifiedRows(db_conn,obj_key_struct,row_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// GetModifiedRows returns ODBCM_RC_FAILED 
TEST_F(KtClassTest, GetModifiedRows_04) {
  vector<void *> obj_key_struct;
  obj_key_struct.clear();
  CsRowStatus row_status = UPDATED; 
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETMODIFIEDROWS, ODBCM_RC_FAILED);
  int ret =  KtboundaryObj.GetModifiedRows(db_conn,obj_key_struct,row_status);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// IsKeyExists with empty vector 
TEST_F(KtClassTest, IsKeyExists_01) {
  vector<string> obj_key_struct;
  obj_key_struct.clear();
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,obj_key_struct);
  EXPECT_EQ(ret,UPPL_RC_ERR_BAD_REQUEST);
}

// IsRowExists returns  ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, IsKeyExists_02) {
  vector<string> obj_key_struct;
  obj_key_struct.clear();
  string BD1 = "boundary1";
  obj_key_struct.push_back(BD1);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,obj_key_struct);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// IsRowExists returns  ODBCM_RC_ROW_EXISTS
TEST_F(KtClassTest, IsKeyExists_03) {
  vector<string> obj_key_struct;
  obj_key_struct.clear();
  string BD1 = "boundary1";
  obj_key_struct.push_back(BD1);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  KtboundaryObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,obj_key_struct);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// IsRowExists returns  ODBCM_RC_FAILED
TEST_F(KtClassTest, IsKeyExists_04) {
  vector<string> obj_key_struct;
  obj_key_struct.clear();
  string BD1 = "boundary1";
  obj_key_struct.push_back(BD1);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_FAILED);
  int ret =  KtboundaryObj.IsKeyExists(db_conn,UNC_DT_CANDIDATE,obj_key_struct);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with UNC_OP_CREATE
TEST_F(KtClassTest, PerformSemanticValidation_01) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_CREATE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_INSTANCE_EXISTS);
  unc::uppl::ODBCManager::clearStubData();
}

// IsKeyExists returns ODBCM_RC_CONNECTION_ERROR
TEST_F(KtClassTest, PerformSemanticValidation_02) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_CREATE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with UNC_OP_UPDATE
TEST_F(KtClassTest, PerformSemanticValidation_03) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_UPDATE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with UNC_OP_DELETE
TEST_F(KtClassTest, PerformSemanticValidation_04) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_DELETE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}// PerformSemanticValidation with UNC_OP_READ
TEST_F(KtClassTest, PerformSemanticValidation_05) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_READ,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with UNC_OP_UPDATE and IsKeyExists returns ODBCM_RC_ROW_NOT_EXISTS
TEST_F(KtClassTest, PerformSemanticValidation_06) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_UPDATE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with UNC_OP_DELETE and IsKeyExists returns ODBCM_RC_ROW_NOT_EXISTS
TEST_F(KtClassTest, PerformSemanticValidation_07) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_DELETE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}// PerformSemanticValidation with UNC_OP_READ and IsKeyExists returns ODBCM_RC_ROW_NOT_EXISTS
TEST_F(KtClassTest, PerformSemanticValidation_08) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_READ,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with UNC_OP_UPDATE and IsKeyExists returns ODBCM_RC_ROW_EXISTS
TEST_F(KtClassTest, PerformSemanticValidation_09) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_UPDATE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// controller1 == controller2 && domain1 == domain2
TEST_F(KtClassTest, PerformSemanticValidation_10) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary5(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_UPDATE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SEMANTIC);
  unc::uppl::ODBCManager::clearStubData();
}

// Val struct NULL
TEST_F(KtClassTest, PerformSemanticValidation_11) {
  key_boundary_t k;
  val_boundary_t *v = NULL;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,v,UNC_OP_UPDATE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// PerformSemanticValidation with UNC_OP_DELETE and IsKeyExists returns ODBCM_RC_ROW_EXISTS
TEST_F(KtClassTest, PerformSemanticValidation_12) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_DELETE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// Getctr type failure
TEST_F(KtClassTest, PerformSemanticValidation_13) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_UPDATE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SEMANTIC);
  unc::uppl::ODBCManager::clearStubData();
}

// Domain1 name Default
TEST_F(KtClassTest, PerformSemanticValidation_14) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.domain_name1, '\0', 32);
  memcpy(v.domain_name1, "(DEFAULT)", strlen("(DEFAULT)"));
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_UPDATE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SEMANTIC);
  unc::uppl::ODBCManager::clearStubData();
}

// Domain2 name Default
TEST_F(KtClassTest, PerformSemanticValidation_15) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  memset(v.domain_name2, '\0', 32);
  memcpy(v.domain_name2, "(DEFAULT)", strlen("(DEFAULT)"));
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.PerformSemanticValidation(db_conn,&k,&v,UNC_OP_UPDATE,UNC_DT_CANDIDATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SEMANTIC);
  unc::uppl::ODBCManager::clearStubData();
}

// ValidateSiblingFiltering
TEST_F(KtClassTest, ValidateSiblingFiltering_01) {
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.ValidateSiblingFiltering(0,0,1,1);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

// ValidateSiblingFiltering
TEST_F(KtClassTest, ValidateSiblingFiltering_02) {
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.ValidateSiblingFiltering(1,0,0,1);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

// ValidateSiblingFiltering
TEST_F(KtClassTest, ValidateSiblingFiltering_03) {
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.ValidateSiblingFiltering(1,1,1,1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
// SendOperStatusNotification Success
TEST_F(KtClassTest, SendOperStatusNotification_01) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint8_t old_oper_st = 1;
  uint8_t new_oper_st = 0;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.SendOperStatusNotification(k,old_oper_st,new_oper_st);
  EXPECT_EQ(ret,UPPL_RC_FAILURE);
}

// SendOperStatusNotification Failure
TEST_F(KtClassTest, SendOperStatusNotification_02) {
  key_boundary_t k;
  getKeyForKtBoundary1(k);
  uint8_t old_oper_st = 1;
  uint8_t new_oper_st = 0;
  ServerSession ses;
  ServerSession::clearStubData();
  ses.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  ses.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.SendOperStatusNotification(k,old_oper_st,new_oper_st);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

// GetAllBoundaryOperStatus success
TEST_F(KtClassTest, GetAllBoundaryOperStatus_01) {
  string ctr_name = "controller1";
  string domain = "domain1";
  string lp_name = "logical_port1";
  map<string, uint8_t> bdry_notfn;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.GetAllBoundaryOperStatus(db_conn,ctr_name,domain,lp_name,bdry_notfn,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// GetAllBoundaryOperStatus Failure
TEST_F(KtClassTest, GetAllBoundaryOperStatus_02) {
  string ctr_name = "controller1";
  string domain = "domain1";
  string lp_name = "logical_port1";
  map<string, uint8_t> bdry_notfn;
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  KtboundaryObj.GetAllBoundaryOperStatus(db_conn,ctr_name,domain,lp_name,bdry_notfn,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// CheckBoundaryExistence 
TEST_F(KtClassTest, CheckBoundaryExistence_01) {
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);
  getValForKtBoundary1(v);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.CheckBoundaryExistence(db_conn,&k,&v,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SEMANTIC);
  unc::uppl::ODBCManager::clearStubData();
}

// GetOperStatus success 
TEST_F(KtClassTest, GetOperStatus_01) {
  key_boundary_t k;
  uint8_t oper_status = 0;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.GetOperStatus(db_conn,UNC_DT_STATE,&k,oper_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// GetOperStatus Failure 
TEST_F(KtClassTest, GetOperStatus_02) {
  key_boundary_t k;
  uint8_t oper_status = 0;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.GetOperStatus(db_conn,UNC_DT_STATE,&k,oper_status);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

// GetBoundaryValidFlag 
TEST_F(KtClassTest, GetBoundaryValidFlag_01) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v,0,sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.GetBoundaryValidFlag(db_conn,&k,v,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// IsBoundaryReferred with key type UNC_KT_CONTROLLER
TEST_F(KtClassTest, IsBoundaryReferred_01) {
  key_boundary_t k;
  unc_key_type_t keytype = UNC_KT_CONTROLLER;
  uint8_t oper_status = 0;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.IsBoundaryReferred(db_conn,keytype,&k,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_FAILURE);
  unc::uppl::ODBCManager::clearStubData();
}

// IsBoundaryReferred with key type UNC_KT_CONTROLLER and row not exists
TEST_F(KtClassTest, IsBoundaryReferred_02) {
  key_boundary_t k;
  unc_key_type_t keytype = UNC_KT_CONTROLLER;
  uint8_t oper_status = 0;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.IsBoundaryReferred(db_conn,keytype,&k,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// IsBoundaryReferred with key type UNC_KT_CTR_DOMAIN
TEST_F(KtClassTest, IsBoundaryReferred_03) {
  key_boundary_t k;
  unc_key_type_t keytype = UNC_KT_CTR_DOMAIN;
  uint8_t oper_status = 0;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.IsBoundaryReferred(db_conn,keytype,&k,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_FAILURE);
  unc::uppl::ODBCManager::clearStubData();
}

// IsBoundaryReferred with key type UNC_KT_CTR_DOMAIN and row not exists
TEST_F(KtClassTest, IsBoundaryReferred_04) {
  key_boundary_t k;
  unc_key_type_t keytype = UNC_KT_CTR_DOMAIN;
  uint8_t oper_status = 0;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.IsBoundaryReferred(db_conn,keytype,&k,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

// IsBoundaryReferred with unknown key type
TEST_F(KtClassTest, IsBoundaryReferred_05) {
  key_boundary_t k;
  unc_key_type_t keytype = UNC_KT_LINK;
  uint8_t oper_status = 0;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.IsBoundaryReferred(db_conn,keytype,&k,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with negative option1
TEST_F(KtClassTest, PerformRead_Neg_option1_01) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v,0,sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret =  KtboundaryObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with negative option1
TEST_F(KtClassTest, PerformRead_Neg_option1_02) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v,0,sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret =  KtboundaryObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//PerformRead with unsupported datatype
TEST_F(KtClassTest, PerformRead_Neg_datatype_03) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v,0,sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret =  KtboundaryObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_AUDIT,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with unsupported datatype
TEST_F(KtClassTest, PerformRead_Neg_datatype_04) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v,0,sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
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
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret =  KtboundaryObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_AUDIT,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with negative dB
TEST_F(KtClassTest, PerformRead_Neg_DB_05) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v,0,sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret =  KtboundaryObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_CANDIDATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with positive dB
TEST_F(KtClassTest, PerformRead_pos_DB_06) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v,0,sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
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
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret =  KtboundaryObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_CANDIDATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//PerformRead with max_rep_cnt NULL 
TEST_F(KtClassTest, PerformRead_max_rep_count_07) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v,0,sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret =  KtboundaryObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_CANDIDATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)0);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with Datatype UNC_DT_STATE
TEST_F(KtClassTest, PerformRead_STATE_08) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v,0,sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
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

  int ret =  KtboundaryObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformRead with max_rep_cnt NULL 
TEST_F(KtClassTest, PerformRead_max_rep_count_09) {
  key_boundary_t k;
  val_boundary_st_t v;
  memset(&v,0,sizeof(val_boundary_st_t));
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
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
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_NO_SUCH_INSTANCE);
  sess.stub_setAddOutput((uint32_t)UNC_KT_BOUNDARY);

  int ret =  KtboundaryObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_CANDIDATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)0);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleOperStatus with Val_Struct NULL 
TEST_F(KtClassTest, HandleOperStatus_Val_Struct_NULL_01) {
  key_boundary_t k;
  val_boundary_t *v=NULL;
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtboundaryObj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,v,ref_oper_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//HandleOperStatus 
TEST_F(KtClassTest, HandleOperStatus_02) {
  key_boundary_t k;
  val_boundary_t *v = new val_boundary_t;
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxBoundaryControllerName1]=0;
  v->valid[kIdxBoundaryControllerName2]=0;

  int ret =  KtboundaryObj.HandleOperStatus(db_conn,UNC_DT_CANDIDATE,&k,v,ref_oper_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleOperStatus 
TEST_F(KtClassTest, HandleOperStatus_03) {
  key_boundary_t k;
  val_boundary_t *v = new val_boundary_t;
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxBoundaryControllerName1]=1;
  v->valid[kIdxBoundaryControllerName2]=0;

  int ret =  KtboundaryObj.HandleOperStatus(db_conn,UNC_DT_CANDIDATE,&k,v,ref_oper_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleOperStatus 
TEST_F(KtClassTest, HandleOperStatus_04) {
  key_boundary_t k;
  val_boundary_t *v = new val_boundary_t;
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxBoundaryControllerName2]=1;
  v->valid[kIdxBoundaryControllerName1]=0;

  int ret =  KtboundaryObj.HandleOperStatus(db_conn,UNC_DT_CANDIDATE,&k,v,ref_oper_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleOperStatus 
TEST_F(KtClassTest, HandleOperStatus_05) {
  key_boundary_t k;
  val_boundary_t *v = new val_boundary_t;
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxBoundaryControllerName2]=1;
  v->valid[kIdxBoundaryControllerName1]=1;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);

  int ret =  KtboundaryObj.HandleOperStatus(db_conn,UNC_DT_RUNNING,&k,v,ref_oper_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//HandleOperStatus 
TEST_F(KtClassTest, HandleOperStatus_06) {
  key_boundary_t k;
  val_boundary_t *v = new val_boundary_t;
  vector<OperStatusHolder> ref_oper_status;
  getKeyForKtBoundary1(k);
  Kt_Boundary  KtboundaryObj;
  OdbcmConnectionHandler *db_conn =NULL;
  v->valid[kIdxBoundaryControllerName2]=1;
  v->valid[kIdxBoundaryControllerName1]=1;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);

  int ret =  KtboundaryObj.HandleOperStatus(db_conn,UNC_DT_RUNNING,&k,v,ref_oper_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
// FrameValidValue
TEST_F(KtClassTest, FrameValidValue_01) {
  val_boundary_st v_st;
  val_boundary_t v;
  string at_value = "boundary";
  Kt_Boundary  KtboundaryObj;
  int ret =  UPPL_RC_SUCCESS;
  KtboundaryObj.FrameValidValue(at_value,v_st,v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
// FrameCsAttrValue
TEST_F(KtClassTest, FrameCsAttrValue_01) {
  val_boundary_t v;
  string at_value = "boundary";
  Kt_Boundary  KtboundaryObj;
  int ret =  UPPL_RC_SUCCESS;
  KtboundaryObj.FrameCsAttrValue(at_value,v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

/********TAMIL TC's*********/
/*****getBoundaryInputOperStatus*****/
//Send controller name as empty
TEST_F(KtClassTest, getBoundaryInputOperStatus_BadRequest) {
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t data_type;
  string controller_name;
  string domain_name;
  string logical_port_id;
  vector<OperStatusHolder> ref_oper_status;
  Kt_Boundary  KtboundaryObj;
  int ret =  KtboundaryObj.getBoundaryInputOperStatus(db_conn, data_type, controller_name, domain_name, logical_port_id,ref_oper_status);
  EXPECT_EQ(ret, UPPL_BOUNDARY_OPER_UNKNOWN);
  unc::uppl::ODBCManager::clearStubData();
}

//Fill the controler name and send
TEST_F(KtClassTest, getBoundaryInputOperStatus_ctrlName_fill) {
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t data_type = UNC_DT_STATE;
  string controller_name = "ctr1";
  string domain_name;
  string logical_port_id;
  vector<OperStatusHolder> ref_oper_status;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.getBoundaryInputOperStatus(db_conn, data_type, controller_name, domain_name, logical_port_id,ref_oper_status);
  EXPECT_EQ(ret, UPPL_RC_ERR_BAD_REQUEST);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, getBoundaryInputOperStatus) {
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t data_type = UNC_DT_IMPORT;
  string controller_name = "ctr1";
  string domain_name = "";
  string logical_port_id = "";

  unc_key_type_t key_type = UNC_KT_CONTROLLER;
  key_ctr_t *key_struct;
  uint8_t oper_status = 1;
  OperStatusHolder obj(key_type,key_struct,oper_status);
  vector<OperStatusHolder> ref_oper_status;
  ref_oper_status.push_back(obj);
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.getBoundaryInputOperStatus(db_conn, data_type, controller_name, domain_name, logical_port_id,ref_oper_status);
  EXPECT_EQ(ret, UPPL_BOUNDARY_OPER_UNKNOWN);
  unc::uppl::ODBCManager::clearStubData();
}

/********SetOperStatus********/
//key_struct and val_struct are NULL
//return success
TEST_F(KtClassTest, SetOperStatus_NULL_check) {
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t data_type;
  key_boundary_t *k = NULL;
  val_boundary_st_t *v = NULL;
  UpplBoundaryOperStatus oper_status;
  Kt_Boundary  KtboundaryObj;
  int ret =  KtboundaryObj.SetOperStatus(db_conn,data_type,k,v,oper_status);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//controller_name1/2 flag is not valid
TEST_F(KtClassTest, SetOperStatus_InValid_CtrName) {
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t data_type;
  key_boundary_t *k = NULL;
  val_boundary_st_t *v = new val_boundary_st_t;
  UpplBoundaryOperStatus oper_status;
  Kt_Boundary  KtboundaryObj;
  int ret =  KtboundaryObj.SetOperStatus(db_conn,data_type,k,v,oper_status);
  EXPECT_EQ(ret, UPPL_RC_ERR_BAD_REQUEST);
  unc::uppl::ODBCManager::clearStubData();
}

//SetOperStatus Success_check
TEST_F(KtClassTest, SetOperStatus_Success_check) {
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t data_type;
  key_boundary_t k;
  val_boundary_t v;
  getKeyForKtBoundary1(k);  
  getValForKtBoundary1(v);
  v.valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
  UpplBoundaryOperStatus oper_status;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.SetOperStatus(db_conn,data_type,&k,&v,oper_status);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//oper_status update operation not success
TEST_F(KtClassTest, SetOperStatus_UPD_NotSucc_Ctr1_Valid) {
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t data_type;
  key_boundary_t *k = NULL;
  val_boundary_t v;
  //getKeyForKtBoundary1(k);  
  getValForKtBoundary1(v);
  v.valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
  UpplBoundaryOperStatus oper_status = UPPL_BOUNDARY_OPER_UNKNOWN;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.SetOperStatus(db_conn,data_type,k,&v,oper_status);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//oper_status update operation not success
//ctr2 name is set as Invaild
TEST_F(KtClassTest, SetOperStatus_Ctr2_Valid) {
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t data_type;
  key_boundary_t *k = NULL;
  val_boundary_t v;
  //getKeyForKtBoundary1(k);  
  getValForKtBoundary1(v);
  v.valid[kIdxBoundaryControllerName1] = 0;
  UpplBoundaryOperStatus oper_status = UPPL_BOUNDARY_OPER_UNKNOWN;
  Kt_Boundary  KtboundaryObj;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtboundaryObj.SetOperStatus(db_conn,data_type,k,&v,oper_status);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

