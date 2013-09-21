 /*
 * Copyright (c) 2012-2013 NEC Corporation
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
#include <pfc/ipc_struct.h>
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
#include "itc_kt_logical_member_port.hh"
#include "itc_kt_logicalport.hh"
#include "ipct_util.hh"
#include "itc_kt_logical_member_port.hh"
#include "physicallayer.hh"
#include "PhysicalLayerStub.hh"
#include "tclib_module.hh"
#include "itc_read_request.hh"
using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
using namespace unc::uppl;
using namespace std;

ClientSession *cli_sess = NULL;
ClientSession *cli_sess_notify = NULL;
pfc_ipcid_t service = UPPL_SVC_READREQ;
class KtClassTest : public testing::Test {
  protected:
  virtual void SetUp() {
    if(cli_sess == NULL) {
      pfc_ipcconn_t connp = 0;
      int err = pfc_ipcclnt_altopen(UPPL_IPC_CHN_NAME, &connp);
      ASSERT_EQ(0, err);
      cli_sess = new ClientSession(connp, UPPL_IPC_SVC_NAME, service, err);
      ASSERT_EQ(0, err);
      connp = 0;
      err = pfc_ipcclnt_altopen("phynwd", &connp);
      ASSERT_EQ(0, err); cli_sess = new ClientSession(connp, UPPL_IPC_SVC_NAME, service, err);
      ASSERT_EQ(0, err);
      connp = 0;
      err = pfc_ipcclnt_altopen("phynwd", &connp);
      ASSERT_EQ(0, err);
     cli_sess_notify = new ClientSession(connp, "sample2", 0, err);
     
     PhysicalLayerStub::loadphysicallayer();
   }
  }
  virtual void TearDown() {
    if (cli_sess != NULL) {
      cli_sess->cancel();
      delete cli_sess;
      cli_sess = NULL;
    }
    if (cli_sess_notify != NULL) {
      cli_sess_notify->cancel();
      delete cli_sess_notify;
      cli_sess_notify = NULL;
    }
  }

};

void getReqHeader(physical_request_header& rh,
                  unc_keytype_operation_t opern,
                  unc_keytype_datatype_t dtype) {
  rh.client_sess_id = 1;
  rh.config_id = 1;
  rh.operation = opern;
  rh.max_rep_count = 0;
  rh.option1 = 0;
  rh.option2 = 0;
  rh.data_type = dtype;
  rh.key_type = UNC_KT_LOGICAL_PORT;
}
// Can be changed based on testing need
 char pkName1_ctr[] = "controller1";
 char pkName1_domain[] = "controller1 domain name";
 char pkName1_logicalport[] = "{0x00,0xa}";
 char pkName2_ctr[] = "controller2";
 char pkName2_domain[] = "domainName2";
 char pkName2_logicalport[] = "logicalport2";
 char pkName3_ctr[] = "controller22";
 char pkName3_domain[] = "domainName3";
 char pkName3_logicalport[] = "logicalport3";
 char pkName4_ctr[] = "";
 char pkName4_domain[] = "";
 char pkName4_logicalport[] = "";
 char pkName5_ctr[] = "controller2";
 char pkName5_domain[] = "controller2 domain name";
 char pkName5_logicalport[] = "{0x00,0xa1}";
 char pkName6_ctr[] = "controller1";
 char pkName6_domain[] = "controller1 domain name";
 char pkName6_logicalport[] = "{0x00,0xa}";
 char Desription[] = "create demo";
 char SWitchID[] = "switch01";
 char PhyPortID[] = "PhyPort01";
 char controller_name[] = "Controller1";
 char switch_id[] = "Switch1";
 char phy_port_id[] = "port1";
TEST_F(KtClassTest, PerformSyntaxValidation_ControllernameNotFound_01) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}
TEST_F(KtClassTest, PerformSyntaxValidation_DomainNameNotFound_02) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}
TEST_F(KtClassTest, PerformSyntaxValidation_PortIDNNotFound_03) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);

}
TEST_F(KtClassTest, PerformSyntaxValidation_SyntaxValidation_Success_04) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, PerformSyntaxValidation_OptionalValPass_Success_05) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.description,Desription,strlen(Desription));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, IsKeyExists_01) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  vector<string> sw_vect_key_value;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktlinkobj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
  EXPECT_EQ(ret,UPPL_RC_ERR_BAD_REQUEST);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, IsKeyExists_FailureIsrowexist02) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_domain);
  sw_vect_key_value.push_back(pkName1_logicalport);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, IsKeyExists_SuccessIsrowexist03) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_domain);
  sw_vect_key_value.push_back(pkName1_logicalport);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktlinkobj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_InstanceExist_create01) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_INSTANCE_EXISTS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_create02) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_PARENT_DOES_NOT_EXIST);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_update03) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_UPDATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_delete04) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_DELETE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_read03) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_READ;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_read_InstanceExist04) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_READ;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_update_InstanceExist04) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_UPDATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_delete_InstanceExist04) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  rh.key_type = UNC_KT_LOGICAL_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_DELETE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, DeleteKeyInstance_UnsupportedForSTARTUP_01) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_STARTUP, UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}
TEST_F(KtClassTest, DeleteKeyInstance_UnsupportedForCANDIDATE_01) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_CANDIDATE, UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}
TEST_F(KtClassTest, DeleteKeyInstance_UnsupportedForRUNNING_01) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_RUNNING, UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}
TEST_F(KtClassTest, DeleteKeyInstance_UnsupportedForAUDIT_01) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_AUDIT, UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}
TEST_F(KtClassTest, DeleteKeyInstance_Support_03) { //return code not handle from HandleOperDownCriteriaFromPortStatus function
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_DELETE);
}


TEST_F(KtClassTest, DeleteKeyInstance_Support_04) { //return code not handle from HandleOperDownCriteriaFromPortStatus function
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, DeleteKeyInstance_Support_05) { //return code not handle from HandleOperDownCriteriaFromPortStatus function
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, DeleteKeyInstance_Support_06) { //return code not handle from HandleOperDownCriteriaFromPortStatus function
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}


TEST_F(KtClassTest, PerformRead_001) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_PORT;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}



TEST_F(KtClassTest, PerformRead_002) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_PORT;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);
int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_L2DOMAIN,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

TEST_F(KtClassTest, PerformRead_003) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_PORT;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);
int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_RUNNING,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}
TEST_F(KtClassTest, PerformRead_004) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_PORT;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);
int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_INVALID,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, PerformRead_GetOneRow_005) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_PORT;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);
int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}


TEST_F(KtClassTest, SetOperStatus_001) {

  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  //getKeyForKtPort2(k);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,&v,(UpplLogicalPortOperStatus)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, SetOperStatus_002) {

  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  //getKeyForKtPort2(k);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,&v,(UpplLogicalPortOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, SetOperStatus_003) {

  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  //getKeyForKtPort2(k);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,&v,(UpplLogicalPortOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, SetOperStatus_004) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,&v,(UpplLogicalPortOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}


TEST_F(KtClassTest, SetOperStatus_005) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,&v,(UpplLogicalPortOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, SetOperStatus_006) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,&v,(UpplLogicalPortOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}


TEST_F(KtClassTest, SetOperStatus_007) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,&v,(UpplLogicalPortOperStatus)2);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulk_NotAllowOperation_01) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=1;
  int child_index=0;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}
TEST_F(KtClassTest, ReadBulk_MaxCountZERO_02) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_LogicalPort ktlinkobj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=0;
  int child_index=0;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, ReadBulk_UPPL_RC_ERR_DB_GET_02) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=1;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, ReadBulkInternal__SUCCESS01) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  vector<val_logical_port_st_t> vect_val_logical_port_st;
  vector<key_logical_port_t> vect_logicalport_id;
  int ret = ktlinkobj.ReadBulkInternal(db_conn, &k, &v, UNC_DT_STATE, max_rep_ct, vect_val_logical_port_st, vect_logicalport_id);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, GetLogicalPortValidFlag_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  val_logical_port_st_t *val_logical_port_valid_st=NULL;
  val_logical_port_valid_st = new val_logical_port_st_t();
  Kt_LogicalPort ktlinkobj;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = ktlinkobj.GetLogicalPortValidFlag(db_conn, &k,  *val_logical_port_valid_st, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetLogicalPortValidFlag_UPPL_RC_ERR_DB_ACCESS) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  val_logical_port_st_t *val_logical_port_valid_st=NULL;
  val_logical_port_valid_st = new val_logical_port_st_t();
  Kt_LogicalPort ktlinkobj;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.GetLogicalPortValidFlag(db_conn, &k,  *val_logical_port_valid_st, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetLogicalPortValidFlag_OtherThanSuccess) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  val_logical_port_st_t *val_logical_port_valid_st=NULL;
  val_logical_port_valid_st = new val_logical_port_st_t();
  Kt_LogicalPort ktlinkobj;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_COMMON_LINK_FAILURE);
  int ret = ktlinkobj.GetLogicalPortValidFlag(db_conn, &k,  *val_logical_port_valid_st, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetLogicalPortValidFlag_UPPL_RC_SUCCESS) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  val_logical_port_st_t *val_logical_port_valid_st=NULL;
  val_logical_port_valid_st = new val_logical_port_st_t();
  Kt_LogicalPort ktlinkobj;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.GetLogicalPortValidFlag(db_conn, &k,  *val_logical_port_valid_st, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetOperStatusFromOperDownCriteria_DB_ERROR) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  UpplLogicalPortOperStatus new_oper_status = UPPL_LOGICAL_PORT_OPER_UNKNOWN;
  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = ktlinkobj.GetOperStatusFromOperDownCriteria(db_conn,UNC_DT_STATE,&k,&v,new_oper_status);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetOperStatusFromOperDownCriteria_DB_Success) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  UpplLogicalPortOperStatus new_oper_status = UPPL_LOGICAL_PORT_OPER_UNKNOWN;
  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.GetOperStatusFromOperDownCriteria(db_conn,UNC_DT_STATE,&k,&v,new_oper_status);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, HandleDriverAlarms_AlaramOtherThanUNC_SUBDOMAIN_SPLIT) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_ERR_BAD_REQUEST);

}
//Unable to get current oper_status from db
TEST_F(KtClassTest, HandleDriverAlarms_UNC_SUBDOMAIN_SPLIT_01) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  uint32_t alarm_type = UNC_SUBDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktlinkobj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//operstatus new and old naot same
TEST_F(KtClassTest, HandleDriverAlarms_UNC_SUBDOMAIN_SPLIT_02) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  uint32_t alarm_type = UNC_SUBDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_CREATE;
  vector<void *> obj_key_struct;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
//Update oper_status
TEST_F(KtClassTest, HandleDriverAlarms_UNC_SUBDOMAIN_SPLIT_03) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  uint32_t alarm_type = UNC_SUBDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_DELETE;
  vector<void *> obj_key_struct;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, HandleDriverAlarms_UNC_SUBDOMAIN_SPLIT_04) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  uint32_t alarm_type = UNC_SUBDOMAIN_SPLIT;
  uint32_t oper_type = UNC_OP_UPDATE;
  vector<void *> obj_key_struct;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.HandleDriverAlarms(db_conn,UNC_DT_STATE,alarm_type,oper_type,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, NotifyOperStatus_success) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  vector<OperStatusHolder> refer;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.NotifyOperStatus(db_conn,UNC_DT_STATE,&k,&v,refer);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, NotifyOperStaus_valstructureNull) {
  key_logical_port_t k;
  val_logical_port_st_t *v=NULL;
  memset(&k, 0, sizeof(key_logical_port_t));
  //memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  vector<OperStatusHolder> refer;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.NotifyOperStatus(db_conn,UNC_DT_STATE,&k,v,refer);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, GetAllPortId_va) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  string controller_name1 =  reinterpret_cast<char *> (pkName1_ctr);
  string switch_id1 = reinterpret_cast<char *> (pkName1_logicalport);
  string domain_name1 = reinterpret_cast<char *> (pkName1_domain);
  pfc_bool_t is_single_logical_port1=true;
  vector <string> logical_port_id1;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = UPPL_RC_SUCCESS;
  ktlinkobj.GetAllPortId(db_conn,UNC_DT_STATE,controller_name1,switch_id1,domain_name1,logical_port_id1,true);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, GetAllPortId_GetSibling) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  string controller_name1 =  reinterpret_cast<char *> (pkName1_ctr);
  string switch_id1 = reinterpret_cast<char *> (pkName1_logicalport);
  string domain_name1 = reinterpret_cast<char *> (pkName1_domain);
  pfc_bool_t is_single_logical_port1=true;
  vector <string> logical_port_id1;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETSIBLINGROWS, ODBCM_RC_SUCCESS);
  int ret = UPPL_RC_SUCCESS;
  ktlinkobj.GetAllPortId(db_conn,UNC_DT_STATE,controller_name1,switch_id1,domain_name1,logical_port_id1,true);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetAllPortId_IsSingleFALSE) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  string controller_name1 =  reinterpret_cast<char *> (pkName1_ctr);
  string switch_id1 = reinterpret_cast<char *> (pkName1_logicalport);
  string domain_name1 = reinterpret_cast<char *> (pkName1_domain);
  pfc_bool_t is_single_logical_port1=true;
  vector <string> logical_port_id1;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETSIBLINGROWS, ODBCM_RC_SUCCESS);
  int ret = UPPL_RC_SUCCESS;
  ktlinkobj.GetAllPortId(db_conn,UNC_DT_STATE,controller_name1,switch_id1,domain_name1,logical_port_id1,false);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, HandleOperStatus_GetOneRow_SUCCESS) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  vector<OperStatusHolder> refer;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v,refer,UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, HandleOperStatus_SwitchID_Validation) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  v.logical_port.valid[kIdxLogicalPortSwitchId] = 1;
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  //string key_instance = "controller1";
  unc_key_type_t key_type = UNC_KT_CONTROLLER;
  uint8_t oper_status =7;
  void* key_struct;
  OperStatusHolder obj(key_type,key_struct,oper_status);
  vector<OperStatusHolder> refer;
  refer.push_back(obj);
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v,refer,UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetPortOperStatus_DT_STATE) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  key_port_t obj_key_port;
  memset(&obj_key_port, 0, sizeof(key_port_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  memcpy(obj_key_port.port_id,phy_port_id,strlen(phy_port_id));
  memcpy(obj_key_port.sw_key.switch_id,switch_id,strlen(switch_id));
  memcpy(obj_key_port.sw_key.ctr_key.controller_name,controller_name,strlen(controller_name));
  
  uint8_t port_oper_status =1;
  uint32_t data_type = UNC_DT_STATE;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.GetPortOperStatus(db_conn,obj_key_port,&port_oper_status,data_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, GetPortOperStatus_DT_INVALID) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  key_port_t obj_key_port;
  memset(&obj_key_port, 0, sizeof(key_port_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  memcpy(obj_key_port.port_id,phy_port_id,strlen(phy_port_id));
  memcpy(obj_key_port.sw_key.switch_id,switch_id,strlen(switch_id));
  memcpy(obj_key_port.sw_key.ctr_key.controller_name,controller_name,strlen(controller_name));
  uint8_t port_oper_status =1;
  uint32_t data_type = UNC_DT_INVALID;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.GetPortOperStatus(db_conn,obj_key_port,&port_oper_status,data_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetPortOperStatus_DT_CANDIDATE) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  key_port_t obj_key_port;
  memset(&obj_key_port, 0, sizeof(key_port_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  memcpy(obj_key_port.port_id,phy_port_id,strlen(phy_port_id));
  memcpy(obj_key_port.sw_key.switch_id,switch_id,strlen(switch_id));
  memcpy(obj_key_port.sw_key.ctr_key.controller_name,controller_name,strlen(controller_name));
  uint8_t port_oper_status =1;
  uint32_t data_type = UNC_DT_CANDIDATE;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.GetPortOperStatus(db_conn,obj_key_port,&port_oper_status,data_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetPortOperStatus_DT_RUNNING) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  key_port_t obj_key_port;
  memset(&obj_key_port, 0, sizeof(key_port_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  memcpy(obj_key_port.port_id,phy_port_id,strlen(phy_port_id));
  memcpy(obj_key_port.sw_key.switch_id,switch_id,strlen(switch_id));
  memcpy(obj_key_port.sw_key.ctr_key.controller_name,controller_name,strlen(controller_name));
  uint8_t port_oper_status =1;
  uint32_t data_type = UNC_DT_RUNNING;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.GetPortOperStatus(db_conn,obj_key_port,&port_oper_status,data_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetPortOperStatus_DT_STARTUP) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  key_port_t obj_key_port;
  memset(&obj_key_port, 0, sizeof(key_port_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  memcpy(obj_key_port.port_id,phy_port_id,strlen(phy_port_id));
  memcpy(obj_key_port.sw_key.switch_id,switch_id,strlen(switch_id));
  memcpy(obj_key_port.sw_key.ctr_key.controller_name,controller_name,strlen(controller_name));
  uint8_t port_oper_status =1;
  uint32_t data_type = UNC_DT_STARTUP;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.GetPortOperStatus(db_conn,obj_key_port,&port_oper_status,data_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetPortOperStatus_DT_IMPORT) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  key_port_t obj_key_port;
  memset(&obj_key_port, 0, sizeof(key_port_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  memcpy(obj_key_port.port_id,phy_port_id,strlen(phy_port_id));
  memcpy(obj_key_port.sw_key.switch_id,switch_id,strlen(switch_id));
  memcpy(obj_key_port.sw_key.ctr_key.controller_name,controller_name,strlen(controller_name));
  uint8_t port_oper_status =1;
  uint32_t data_type = UNC_DT_IMPORT;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.GetPortOperStatus(db_conn,obj_key_port,&port_oper_status,data_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetPortOperStatus_DT_AUDIT) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  key_port_t obj_key_port;
  memset(&obj_key_port, 0, sizeof(key_port_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  memcpy(obj_key_port.port_id,phy_port_id,strlen(phy_port_id));
  memcpy(obj_key_port.sw_key.switch_id,switch_id,strlen(switch_id));
  memcpy(obj_key_port.sw_key.ctr_key.controller_name,controller_name,strlen(controller_name));
  uint8_t port_oper_status =1;
  uint32_t data_type = UNC_DT_AUDIT;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.GetPortOperStatus(db_conn,obj_key_port,&port_oper_status,data_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformRead_Option1_SUCCESS) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_PORT;
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
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_INVALID_OPTION1);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_PORT);

  int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, PerformRead_Option2_SUCCESS) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_INVALID_OPTION2);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_PORT);

  int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, PerformRead_Datatype_SUCCESS) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_RUNNING);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_PORT);

  int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_RUNNING,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, PerformRead_DB_SUCCESS) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_PORT;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  ServerSession::clearStubData();
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_RUNNING);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_PORT);

  int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_UPDATE,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, GetAllLogicalPort_LogicalPortNotAvailable) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  string controller_name1 =  reinterpret_cast<char *> (pkName1_ctr);
  string switch_id1 = reinterpret_cast<char *> (pkName1_logicalport);
  string domain_name1 = reinterpret_cast<char *> (pkName1_domain);
  string physicalportid = reinterpret_cast<char *> (PhyPortID);
  vector<key_logical_port_t> vectLogicalPortKey;
  vectLogicalPortKey.push_back(k);
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = UPPL_RC_SUCCESS;
  ktlinkobj.GetAllLogicalPort(db_conn,controller_name1,domain_name1,switch_id1,physicalportid,vectLogicalPortKey,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, GetAllLogicalPort_DBSUCCESS) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  string controller_name1 =  reinterpret_cast<char *> (pkName1_ctr);
  string switch_id1 = reinterpret_cast<char *> (pkName1_logicalport);
  string domain_name1 = reinterpret_cast<char *> (pkName1_domain);
  string physicalportid = reinterpret_cast<char *> (PhyPortID);
  vector<key_logical_port_t> vectLogicalPortKey;
  vectLogicalPortKey.push_back(k);
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = UPPL_RC_SUCCESS;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  ktlinkobj.GetAllLogicalPort(db_conn,controller_name1,domain_name1,switch_id1,physicalportid,vectLogicalPortKey,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, ValidatePortType_LP_SWITCH_SUCCESS) {
  uint8_t port_type = 1;//1 for UPPL_LP_SWITCH
  Kt_LogicalPort ktlinkobj;
  int ret = ktlinkobj.ValidatePortType(port_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, ValidatePortType_LP_PHYSICALPORT_SUCCESS) {
  uint8_t port_type = 2;//1 for UPPL_LP_PHYSICAL_PORT
  Kt_LogicalPort ktlinkobj;
  int ret = ktlinkobj.ValidatePortType(port_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, ValidatePortType_LP_TRUNKPORT_SUCCESS) {
  uint8_t port_type = 11;//1 for UPPL_LP_TRUNK_PORT
  Kt_LogicalPort ktlinkobj;
  int ret = ktlinkobj.ValidatePortType(port_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, ValidatePortType_LP_SUBDOMAIN_SUCCESS) {
  uint8_t port_type = 12;//1 for UPPL_LP_SUBDOMAIN
  Kt_LogicalPort ktlinkobj;
  int ret = ktlinkobj.ValidatePortType(port_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, ValidatePortType_LP_TUNNEL_SUCCESS) {
  uint8_t port_type = 13;//1 for UPPL_LP_TUNNEL_ENDPOINT
  Kt_LogicalPort ktlinkobj;
  int ret = ktlinkobj.ValidatePortType(port_type);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, ValidatePortType_FAILL_FOR_UNKNOWNPORT) {
  uint8_t port_type = 3;//1 for UNKNOWNPORT
  Kt_LogicalPort ktlinkobj;
  int ret = ktlinkobj.ValidatePortType(port_type);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtClassTest, ReadBulk_ADDTOBUFFER_maxrepCT1) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct = 1;
  int child_index = 1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = new ReadRequest;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, ReadBulk_DB_ACCESS_ERROR) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct = 2;
  int child_index = 1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = new ReadRequest;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulkInternal_RecordNot_Found) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=1;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  vector<val_logical_port_st_t> vect_val_logical_port_st;
  vector<key_logical_port_t> vect_logicalport_id;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = ktlinkobj.ReadBulkInternal(db_conn, &k, &v, UNC_DT_STATE, max_rep_ct, vect_val_logical_port_st, vect_logicalport_id);
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, PerformSyntaxValidation_ValStrutValidation_PortType_04) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.description,Desription,strlen(Desription));
  uint8_t porttype = 3;
  v.logical_port.port_type= porttype;
  v.logical_port.valid[kIdxLogicalPortType]=1;
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

TEST_F(KtClassTest, PerformSyntaxValidation_ValStrutValidation_oper_down_criteria_05) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.description,Desription,strlen(Desription));
  uint8_t operdowncriteria = 3;
  v.logical_port.oper_down_criteria= operdowncriteria;
  v.logical_port.valid[kIdxLogicalPortOperDownCriteria]=1;
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}
TEST_F(KtClassTest, PerformSyntaxValidation_ValStrutNULL) {
  key_logical_port_t k;
  val_logical_port_st_t *v=NULL;
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, ReadInternal_max_rep_ct) {
  key_logical_port_t *k=new key_logical_port_t;
  vector<void *> vectVal_logicalport;
  vector<void *> vectkey_logicalport;
  vectkey_logicalport.push_back(k);
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.ReadInternal(db_conn,vectkey_logicalport,vectVal_logicalport,UNC_DT_STATE,UNC_OP_CREATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, ReadInternal_VAlStructNull) {
  key_logical_port_t *k=new key_logical_port_t;
  val_logical_port_st_t *v=new val_logical_port_st_t;
  vector<void *> vectVal_logicalport;
  vector<void *> vectkey_logicalport;
  vectkey_logicalport.push_back(k);
  vectVal_logicalport.push_back(v);
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.ReadInternal(db_conn,vectkey_logicalport,vectVal_logicalport,UNC_DT_STATE,UNC_OP_READ);
  EXPECT_EQ(ret,ODBCM_RC_MORE_ROWS_FOUND);
}
TEST_F(KtClassTest, HandleOperStatus_NotSuccess) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  v.logical_port.valid[kIdxLogicalPortPhysicalPortId] = 1;
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  //string key_instance = "controller1";
  unc_key_type_t key_type = UNC_KT_CONTROLLER;
  uint8_t oper_status =0;
  void* key_struct;
  OperStatusHolder obj(key_type,key_struct,oper_status);
  vector<OperStatusHolder> refer;
  refer.push_back(obj);
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktlinkobj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v,refer,UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, HandleOperStatus_GetOneRow_FAILED) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  v.logical_port.valid[kIdxLogicalPortPhysicalPortId] = 1;
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  vector<OperStatusHolder> refer;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktlinkobj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v,refer,UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, HandleOperStatus_GetOneRow_SUCCESS_CONTROLLERUP) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  v.logical_port.valid[kIdxLogicalPortPhysicalPortId] = 1;
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  string key_instance = "controller1";
  unc_key_type_t key_type = UNC_KT_CONTROLLER;
  uint8_t oper_status =7;
  void* key_struct;
  OperStatusHolder obj(key_type,key_struct,oper_status);
  vector<OperStatusHolder> refer;
  refer.push_back(obj);
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v,refer,UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, HandleOperDownCriteriaFromPortStatus_KEY_STRUCT_NULL) {
  key_logical_port_t *k = NULL;
  val_logical_port_st_t v;
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  pfc_bool_t is_delete_call = true;
  vector<uint32_t> vectOperStatus;
  int ret =  ktlinkobj.HandleOperDownCriteriaFromPortStatus(db_conn,UNC_DT_STATE,k,&v,vectOperStatus,is_delete_call);
  EXPECT_EQ(ret,UPPL_RC_ERR_BAD_REQUEST);
}
TEST_F(KtClassTest, HandleOperDownCriteriaFromPortStatus_Key_struct_success) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  memcpy(v.logical_port.physical_port_id,PhyPortID,strlen(PhyPortID));
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  pfc_bool_t is_delete_call = true;
  vector<uint32_t> vectOperStatus;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.HandleOperDownCriteriaFromPortStatus(db_conn,UNC_DT_STATE,&k,&v,vectOperStatus,is_delete_call);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}


TEST_F(KtClassTest, GetOperDownCriteria_Success) {
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  uint32_t oper_down_criteria = 1;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.GetOperDownCriteria(db_conn,UNC_DT_STATE,&k,oper_down_criteria);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
/*TEST_F(KtClassTest, HandleOperStatus_KeyStruct_NULL_Handdle) {
  key_logical_port_t k;
  val_logical_port_st_t v;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  memcpy(v.logical_port.switch_id,SWitchID,strlen(SWitchID));
  unc_key_type_t key_type_= UNC_KT_BOUNDARY;
  string key_instance_ = "physical";
  uint8_t oper_status_ = 1;
  vector<OperStatusHolder> refer;
  refer.push_back(key_type_);
  refer.push_back(key_instance_);
  refer.push_back(oper_status_);
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v,refer,UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}*/
#if 0
TEST_F(KtClassTest, PerformSyntaxValidation_ValueStructureNullCheck_06) { //Null check properly not handle for value structure
  key_logical_port_t k;
  val_logical_port_st_t *v =NULL;
  memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  memcpy(k.domain_key.ctr_key.controller_name,pkName1_ctr,strlen(pkName1_ctr));
  memcpy(k.domain_key.domain_name,pkName1_domain,strlen(pkName1_domain));
  memcpy(k.port_id,pkName1_logicalport,strlen(pkName1_logicalport));
  Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}
TEST_F(KtClassTest, DeleteKeyInstance_supported_01) { //Bug in  HandleOperDownCriteriaFromPortStatus(return statement not handle,code crash)
   key_logical_port_t *k = NULL;
  //memset(&k, 0, sizeof(key_logical_port_t));
   Kt_LogicalPort ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
   OdbcmConnectionHandler *db_conn =NULL;
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, k, UNC_DT_STATE, UNC_KT_LOGICAL_PORT);
   EXPECT_EQ(ret, UPPL_RC_ERR_BAD_REQUEST);
  }
//Key-structure handle
TEST_F(KtClassTest, HandleOperStatus_KeyStruct_NULL_Handdle) {
  key_logical_port_t *k=NULL;
  val_logical_port_st_t v;
  //memset(&k, 0, sizeof(key_logical_port_t));
  memset(&v, 0, sizeof(val_logical_port_st_t));
  vector<OperStatusHolder> refer;
  Kt_LogicalPort ktlinkobj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.HandleOperStatus(db_conn,UNC_DT_STATE,k,&v,refer,UNC_KT_LOGICAL_PORT);
  EXPECT_EQ(ret,UPPL_RC_ERR_BAD_REQUEST);
}
#endif
