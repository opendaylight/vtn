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
#include "physical_common_def.hh"
#include "unc/uppl_common.h"
#include "unc/keytype.h"
#include "stub/ODBC/include/odbcm_mgr.hh"
#include "itc_kt_base.hh"
#include "itc_kt_root.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_port.hh"
#include "itc_kt_link.hh"
#include "itc_kt_boundary.hh"
#include "ipct_util.hh"
#include "physicallayer.hh"
#include "PhysicalLayerStub.hh"
#include "itc_read_request.hh"

using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
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
      //ASSERT_TRUE(connp != 0);
      cli_sess = new ClientSession(connp, UPPL_IPC_SVC_NAME, service, err);
      ASSERT_EQ(0, err);
      connp = 0;
      err = pfc_ipcclnt_altopen("phynwd", &connp);
      ASSERT_EQ(0, err);
      //ASSERT_TRUE(connp!=0);
      cli_sess_notify = new ClientSession(connp, "sample2", 0, err);
  
  PhysicalLayerStub::loadphysicallayer();
    //ASSERT_EQ(0, err);

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


char pkName1[] = "{0x10,0xbc}";
char pkName2[] = "controller1";
char pkName3[] = "switch_id2";
char pkName4[] = "controller2";
char pkName5[] = "switch_id3";
char pkName6[] = "controller3";
char pkName7[] = "";
char pkName8[]= "NotExisting";

void getKeyForKtSwitch1(key_switch_t& k) {
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
}

void getValForKtSwitch1(val_switch_st_t& v) {
  // uint8_t description[128]
  memset(v.switch_val.description, '\0', 32);
  memcpy(v.switch_val.description,
         "switch description",
         strlen("switch description"));

  // uint8_t model[16]
  memset(v.switch_val.model, '\0', 16);
  memcpy(v.switch_val.model,
         "switch model",
         strlen("switch model"));


  string ip_add = "10.100.12.34";
  inet_pton(AF_INET,
            (const char *)ip_add.c_str(),
            &v.switch_val.ip_address.s_addr);

  string ipv6_add = "";
  inet_pton(AF_INET6, (const char *)ipv6_add.c_str(),
            &v.switch_val.ipv6_address.s6_addr);

  // uint8_t admin_status
  v.switch_val.admin_status = 0;

  // uint8_t domain_name[32]
  memset(v.switch_val.domain_name, '\0', 32);
  memcpy(v.switch_val.domain_name, "domain_name", strlen("domain_name"));

  // uint8_t oper_status
  v.oper_status = 0;

  // uint8_t manufacturer
  memset(v.manufacturer, '\0' , 256);
  memcpy(v.manufacturer, "NEC CORP", strlen("NEC CORP"));

  // uint8_t hardware
  memset(v.hardware, '\0', 256);
  memcpy(v.hardware, "HW", strlen("HW"));

  // uint8_t software
  memset(v.software, '\0', 256);
  memcpy(v.software, "SW", strlen("SW"));

  // uint64_t alarms_status
  v.alarms_status = 0x01;

  // uint8_t valid[6];
  memset(v.valid, '\0', 7);
}
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
  rh.key_type = UNC_KT_SWITCH;
}

/********TEST CASES***************/
//Validating Key:ControllerId Not found, Operation:UNC_OP_CREATE
TEST_F(KtClassTest, PerformSyntxCheck_controllerId_notFound_01) {
key_switch_t k;
val_switch_st_t v;
Kt_Switch KtSwitchObj;
physical_request_header rh;
char pkName1[] = "{0x10,0xbc}";
memset(k.ctr_key.controller_name, '\0', 32);
memset(k.switch_id, '\0', 256);
memcpy(k.switch_id, pkName1, strlen(pkName1));
uint32_t operation = UNC_OP_CREATE;
OdbcmConnectionHandler *db_conn =NULL;
int ret = KtSwitchObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Validating Key:ControllerId Not found, Operation:UNC_OP_UPDATE
TEST_F(KtClassTest, PerformSyntxCheck_controllerId_notFound_02) {
key_switch_t k;
val_switch_st_t v;
Kt_Switch KtSwitchObj;
physical_request_header rh;
char pkName1[] = "{0x10,0xbc}";
memset(k.ctr_key.controller_name, '\0', 32);
memset(k.switch_id, '\0', 256);
memcpy(k.switch_id, pkName1, strlen(pkName1));
getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
rh.key_type = UNC_KT_SWITCH;
uint32_t operation = UNC_OP_UPDATE;
OdbcmConnectionHandler *db_conn =NULL;
int ret = KtSwitchObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Validating Key:ControllerId Not found, Operation:UNC_OP_DELETE
TEST_F(KtClassTest, PerformSyntxCheck_controllerId_notFound_03) {
key_switch_t k;
val_switch_st_t v;
Kt_Switch KtSwitchObj;
physical_request_header rh;
char pkName1[] = "{0x10,0xbc}";
memset(k.ctr_key.controller_name, '\0', 32);
memset(k.switch_id, '\0', 256);
memcpy(k.switch_id, pkName1, strlen(pkName1));
getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
rh.key_type = UNC_KT_SWITCH;
uint32_t operation = UNC_OP_DELETE;
OdbcmConnectionHandler *db_conn =NULL;
int ret = KtSwitchObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Validating Key:ControllerId Not found, Operation:UNC_OP_READ
TEST_F(KtClassTest, PerformSyntxCheck_controllerId_notFound_04) {
key_switch_t k;
val_switch_st_t v;
Kt_Switch KtSwitchObj;
physical_request_header rh;
char pkName1[] = "{0x10,0xbc}";
memset(k.ctr_key.controller_name, '\0', 32);
memset(k.switch_id, '\0', 256);
memcpy(k.switch_id, pkName1, strlen(pkName1));
getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
rh.key_type = UNC_KT_SWITCH;
uint32_t operation = UNC_OP_READ;
OdbcmConnectionHandler *db_conn =NULL;
int ret = KtSwitchObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Validating Key:SwitchId_notFound, Operation:UNC_OP_CREATE
TEST_F(KtClassTest, PerformSyntxCheck_SwitchId_notFound_01) {
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
int ret = KtSwitchObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Validating Key:SwitchId_notFound, Operation:UNC_OP_UPDATE
TEST_F(KtClassTest, PerformSyntxCheck_SwitchId_notFound_02) {
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
int ret = KtSwitchObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}
//Validating Key:SwitchId_notFound, Operation:UNC_OP_DELETE
TEST_F(KtClassTest, PerformSyntxCheck_SwitchId_notFound_03) {
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
int ret = KtSwitchObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Validating Key:SwitchId_notFound, Operation:UNC_OP_READ
TEST_F(KtClassTest, PerformSyntxCheck_SwitchId_notFound_04) {
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
int ret = KtSwitchObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Validation of key and value struct-Success
TEST_F(KtClassTest, PerformSyntxCheck_ValStruct_success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  getKeyForKtSwitch1(k);
  getValForKtSwitch1(v);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

/*******IsKeyExists*************/
//No key given..Returning error
TEST_F(KtClassTest, IskeyExists_NoKey) {
key_switch_t k;
val_switch_st_t v;
Kt_Switch KtSwitchObj;
physical_request_header rh;
char pkName2[] = "controller1";
memset(k.ctr_key.controller_name, '\0', 32);
memset(k.switch_id, '\0', 256);
getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
rh.key_type = UNC_KT_SWITCH;
uint32_t operation = UNC_OP_READ;
vector<string> sw_vect_key_value;
OdbcmConnectionHandler *db_conn =NULL;
int ret = KtSwitchObj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
EXPECT_EQ(ret,UPPL_RC_ERR_BAD_REQUEST);
}

//DB connection not available,Returns Error
TEST_F(KtClassTest, IskeyExists_Db_Connxn_Error) {
key_switch_t k;
val_switch_st_t v;
Kt_Switch KtSwitchObj;
physical_request_header rh;
char pkName1[] = "{0x10,0xbc}";
char pkName2[] = "controller1";
memset(k.ctr_key.controller_name, '\0', 32);
memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
memset(k.switch_id, '\0', 256);
memcpy(k.switch_id, pkName1, strlen(pkName1));
getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
rh.key_type = UNC_KT_SWITCH;
uint32_t operation = UNC_OP_READ;
vector<string> sw_vect_key_value;
sw_vect_key_value.push_back(pkName1);
sw_vect_key_value.push_back(pkName2);
OdbcmConnectionHandler *db_conn =NULL;
unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
int ret = KtSwitchObj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
unc::uppl::ODBCManager::clearStubData();
}

//DB returned success for Row exists
TEST_F(KtClassTest, IskeyExists_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  uint32_t operation = UNC_OP_READ;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//DB Returned failure for IsRowExists
TEST_F(KtClassTest, IskeyExists_Error) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  uint32_t operation = UNC_OP_READ;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1);
  sw_vect_key_value.push_back(pkName2);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_FAILED);
  int ret = KtSwitchObj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

/****DeleteKeyInstance****/

//Delete operation on unsupported DB: STARTUP
TEST_F(KtClassTest, DeleteKeyInstance_UnsupportedDB_01) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STARTUP, UNC_KT_SWITCH);  
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}

//Delete for unsupported DB UNC_DT_CANDIDATE
TEST_F(KtClassTest, DeleteKeyInstance_UnsupportedDB_02) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn, &k, UNC_DT_CANDIDATE, UNC_KT_SWITCH);  
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}

//Delete for unsupported DB UNC_DT_RUNNING
TEST_F(KtClassTest, DeleteKeyInstance_UnsupportedDB_03) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn, &k, UNC_DT_RUNNING, UNC_KT_SWITCH);  
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}

//Delete operation failed in switch common table
TEST_F(KtClassTest, DeleteKeyInstance_Fail_In_Db) {
  key_switch_t k;
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_SWITCH);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_DELETE);
}

//Delete operation failed due to DB connection not available
TEST_F(KtClassTest, DeleteKeyInstance_Db_Conxn_Error) { 
  key_switch_t k;
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_SWITCH);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}

//Row not Exixts to Delete
TEST_F(KtClassTest, DeleteKeyInstance_RowNotExists) { 
  key_switch_t k;
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_SWITCH);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_DELETE);
  ODBCManager::clearStubData();
}

//DElete operation Success
TEST_F(KtClassTest, DeleteKeyInstance_Success) { 
  key_switch_t k;
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_SWITCH);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}

//Delete Child classess Success
TEST_F(KtClassTest, DeleteKeyInstance_Child_Del) { 
  key_logical_port_t k;
  memset(&k, 0, sizeof(key_logical_port_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_SWITCH);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}

//Error Deleting port in DB
TEST_F(KtClassTest, DeleteKeyInstance_Del_port) { 
  key_port_t k;
  memset(&k, 0, sizeof(key_port_t));
  // getKeyForKtSwitch1(k);
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_FAILED);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_SWITCH);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_DELETE);
  ODBCManager::clearStubData();
}

//No Child Instance exist to delete
TEST_F(KtClassTest, DeleteKeyInstance_NoChildInstance) { 
  key_switch_t k;
  memset(&k, 0, sizeof(key_port_t));
  getKeyForKtSwitch1(k);
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_SWITCH);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_DELETE);
  ODBCManager::clearStubData();
}

/******PerformSemanticValidatin***********/
//Key for parent not exists
TEST_F(KtClassTest, PerformSemanticValid_NoParentKey) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  vector<string> sw_vect_key_value;
  
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn, &k, &v, operation, UNC_DT_STATE);
  
  EXPECT_EQ(ret, UPPL_RC_ERR_PARENT_DOES_NOT_EXIST);
  unc::uppl::ODBCManager::clearStubData();
}

//Update operation not allowed
TEST_F(KtClassTest, PerformSemanticValid_Upd_NotAllowed) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
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
  
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn, &k, &v, operation, UNC_DT_STATE);
  
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

//Delete operation not allowed
TEST_F(KtClassTest, PerformSemanticValid_DEl_NotAllowed) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
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
  
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn, &k, &v, operation, UNC_DT_STATE);
  
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

//Read operation not allowed
TEST_F(KtClassTest, PerformSemanticValid_Read_NotAllowed) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
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
  
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn, &k, &v, operation, UNC_DT_STATE);
  
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

//create operation not allowed for Already
//existing key
TEST_F(KtClassTest, PerformSemanticValid_CREATE_NotAllowed) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
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
  
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn, &k, &v, operation, UNC_DT_STATE);
  
  EXPECT_EQ(ret, UPPL_RC_ERR_INSTANCE_EXISTS);
  unc::uppl::ODBCManager::clearStubData();
}
//PerformSemanticValid Success for update
TEST_F(KtClassTest, PerformSemanticValid_Update_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
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
  
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn, &k, &v, operation, UNC_DT_STATE);
  
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//PerformSemanticValid Success for DELETE
TEST_F(KtClassTest, PerformSemanticValid_Delete_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
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
  
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn, &k, &v, operation, UNC_DT_STATE);
  
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//PerformSemanticValid Success for READ
TEST_F(KtClassTest, PerformSemanticValid_Read_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
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
  
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn, &k, &v, operation, UNC_DT_STATE);
  
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//Parent Key Not exists, DB:UNC_DT_IMPORT
TEST_F(KtClassTest, PerformSemanticValid_Create_In_IMPORT_Fail) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  char pkName1[] = "{0x10,0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  vector<string> sw_vect_key_value;
  
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.PerformSemanticValidation(db_conn, &k, &v, operation, UNC_DT_IMPORT);
  
  EXPECT_EQ(ret, UPPL_RC_ERR_PARENT_DOES_NOT_EXIST);
  unc::uppl::ODBCManager::clearStubData();
}

/******ReadSwtchValFromDb******/
//Operation Other Than Read -Success
TEST_F(KtClassTest, ReadSwtchValFromDb_NoREAD) {
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
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn, &k, &v, UNC_DT_STATE, operation_type, max_rep_ct, vect_val_switch_st, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//No Record Found to read
TEST_F(KtClassTest, ReadSwtchValFromDb_NoRecordFound) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn, &k, &v, UNC_DT_STATE, operation_type, max_rep_ct, vect_val_switch_st, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

//Db connxn error in Read
TEST_F(KtClassTest, ReadSwtchValFromDb_DbConnxnError) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn, &k, &v, UNC_DT_STATE, operation_type, max_rep_ct, vect_val_switch_st, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//Read operation failed with DB
TEST_F(KtClassTest, ReadSwtchValFromDb_Error_Db_Get) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn, &k, &v, UNC_DT_STATE, operation_type, max_rep_ct, vect_val_switch_st, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

//Read From DB Success
TEST_F(KtClassTest, ReadSwtchValFromDb_READ_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn, &k, &v, UNC_DT_STATE, operation_type, max_rep_ct, vect_val_switch_st, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//ReadBulk operation failed
TEST_F(KtClassTest, ReadSwtchValFromDb_READBULK_NoInstance) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn, &k, &v, UNC_DT_STATE, operation_type, max_rep_ct, vect_val_switch_st, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

//ReadBulk operation failed:Db conxn error
TEST_F(KtClassTest, ReadSwtchValFromDb_READBULK_07) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn, &k, &v, UNC_DT_STATE, operation_type, max_rep_ct, vect_val_switch_st, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//ReadBulk operation failed:Db err
TEST_F(KtClassTest, ReadSwtchValFromDb_READBULK_Db_Get) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn, &k, &v, UNC_DT_STATE, operation_type, max_rep_ct, vect_val_switch_st, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

//ReadBulk operation Success
TEST_F(KtClassTest, ReadSwtchValFromDb_READBULK_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  vector<val_switch_st_t> vect_val_switch_st;
  vector<key_switch_t> vect_switch_id;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  OdbcmConnectionHandler *db_conn = NULL;
  memset(k.ctr_key.controller_name, '\0', 32);
  memset(k.switch_id, '\0', 256);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadSwitchValFromDB(db_conn, &k, &v, UNC_DT_STATE, operation_type, max_rep_ct, vect_val_switch_st, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}


/********ReadInternal****/

TEST_F(KtClassTest, ReadInternal) {
  vector<void*> k;
  vector<void*> v;
  Kt_Switch KtSwitchObj;
  uint32_t operation_type = UNC_OP_READ;
  uint32_t max_rep_ct;
  char pkName1[] = "{0x10,0xbc}";
  char pkName2[] = "controller1";
  k.push_back(pkName1);
  k.push_back(pkName2);
  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadInternal(db_conn, k, v, UNC_DT_STATE, operation_type);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

/*****ReadBulkInternal****/
TEST_F(KtClassTest, ReadBulkInternal_MaxCt_01) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct = 0;
  vector<val_switch_st_t> vect_val_switch;
  vector<key_switch_t> vect_switch_id;
  OdbcmConnectionHandler *db_conn = NULL;
  int ret = KtSwitchObj.ReadBulkInternal(db_conn, &k, &v, UNC_DT_STATE, max_rep_ct, vect_val_switch, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulkInternal_NoRecordFound_02) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  vector<val_switch_st_t> vect_val_switch;
  vector<key_switch_t> vect_switch_id;
  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtSwitchObj.ReadBulkInternal(db_conn, &k, &v, UNC_DT_STATE, max_rep_ct, vect_val_switch, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulkInternal_ConnError_03) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  vector<val_switch_st_t> vect_val_switch;
  vector<key_switch_t> vect_switch_id;
  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtSwitchObj.ReadBulkInternal(db_conn, &k, &v, UNC_DT_STATE, max_rep_ct, vect_val_switch, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulkInternal_DbGetError_04) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  vector<val_switch_st_t> vect_val_switch;
  vector<key_switch_t> vect_switch_id;
  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret = KtSwitchObj.ReadBulkInternal(db_conn, &k, &v, UNC_DT_STATE, max_rep_ct, vect_val_switch, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulkInternal_Success_04) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  vector<val_switch_st_t> vect_val_switch;
  vector<key_switch_t> vect_switch_id;
  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadBulkInternal(db_conn, &k, &v, UNC_DT_STATE, max_rep_ct, vect_val_switch, vect_switch_id);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

/********ReadBulk********/
TEST_F(KtClassTest, ReadBulk_NoStateDb_01) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  int child_index;
  pfc_bool_t parent_call;
  pfc_bool_t is_read_next;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = KtSwitchObj.ReadBulk(db_conn, &k, UNC_DT_CANDIDATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulk_Max_Rpct_01) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct = 0;
  int child_index;
  pfc_bool_t parent_call;
  pfc_bool_t is_read_next;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = KtSwitchObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulkInternal_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  uint32_t max_rep_ct;
  int child_index;
  pfc_bool_t parent_call;
  pfc_bool_t is_read_next;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtSwitchObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulkInternal_NoRecordFound) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct = 1;
  int child_index;
  pfc_bool_t parent_call;
  pfc_bool_t is_read_next;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = new ReadRequest;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND); 
  int ret = KtSwitchObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulk_childkey) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10,0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct = 1;
  int child_index = -1;
  pfc_bool_t parent_call;
  pfc_bool_t is_read_next;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtSwitchObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulk_SwitchExists_FALSE) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  char pkName1[] = "{0x10,0xbc}";
  char pkName2[] = "controller1";
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2, strlen(pkName2));
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, pkName1, strlen(pkName1));
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct = 1;
  int child_index ;
  pfc_bool_t parent_call = false;
  pfc_bool_t is_read_next;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = new ReadRequest;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS); 
  int ret = KtSwitchObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

/********PerformRead******/
TEST_F(KtClassTest, PerformRead_01) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ::clearStubData();
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

  int ret =  KtSwitchObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

TEST_F(KtClassTest, PerformRead_002) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ::clearStubData();
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
int ret =  KtSwitchObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_L2DOMAIN,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

TEST_F(KtClassTest, PerformRead_ReadSwtVal_Succes_03) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ::clearStubData();
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_L2DOMAIN,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformRead_ReadSwtVal_OPT1_NoMatch) {
  val_switch_st_t v;
  key_switch_t k;
  //key_switch_t *key = reinterpret_cast<key_switch_t>(k);
  //memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ::clearStubData();
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_INVALID_OPTION1);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  //sess.stub_setAddOutput((uint32_t)key);
  int ret =  KtSwitchObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)UNC_OPT2_L2DOMAIN,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformRead_ReadSwtVal_DataType_NoMatch_Success) {
  val_switch_st_t v;
  key_switch_t k;
  memset(&v, 0, sizeof(val_switch_st_t));
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ::clearStubData();
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  sess.stub_setAddOutput((uint32_t)UNC_KT_SWITCH);
  
  int ret =  KtSwitchObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_CANDIDATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_L2DOMAIN,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformRead_ReadSwtVal_addOutput_fail) {
  val_switch_st_t v;
  key_switch_t k;
  memset(&v, 0, sizeof(val_switch_st_t));
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ::clearStubData();
  ServerSession sess;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_L2DOMAIN,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}


TEST_F(KtClassTest, PerformRead_005) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ::clearStubData();
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
int ret =  KtSwitchObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_INVALID,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, PerformRead_UnsupportDB_STARTUP) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ::clearStubData();
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
int ret =  KtSwitchObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STARTUP,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

TEST_F(KtClassTest, PerformRead_UnsupportDb_CANDIDDATE) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ::clearStubData();
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
int ret =  KtSwitchObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_CANDIDATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}



TEST_F(KtClassTest, PerformRead_GetOneRow_006) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_SWITCH;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession ::clearStubData();
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
int ret =  KtSwitchObj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

/*******PopulateSchemaForValidFlag*********/
TEST_F(KtClassTest, PopulateSchemaForValidFlag_Success) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  string valid_new;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.PopulateSchemaForValidFlag(db_conn, &k ,&v, valid_new, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PopulateSchemaForValidFlag_Failure) {
  key_switch_t k;
  val_switch_st_t v;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  Kt_Switch KtSwitchObj;
  string valid_new;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_GENERAL_ERROR);
  int ret =  KtSwitchObj.PopulateSchemaForValidFlag(db_conn, &k ,&v, valid_new, UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

/********SetOperSatus******/
//Faliure of GetoneRow
TEST_F(KtClassTest, SetOperStatus_001) {

  key_switch_t k;
  memset(&k, 0, sizeof(key_switch_t));
  Kt_Switch KtSwitchObj;
  getKeyForKtSwitch1(k);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtSwitchObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplSwitchOperStatus)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, SetOperStatus_002) {

  key_switch_t k;
  Kt_Switch KtSwitchObj;
  getKeyForKtSwitch1(k);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplSwitchOperStatus)0);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, SetOperStatus_004) {

  key_switch_t k;
  Kt_Switch KtSwitchObj;
  getKeyForKtSwitch1(k);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplSwitchOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, SetOperStatus_005) {

  key_switch_t k;
  Kt_Switch KtSwitchObj;
  getKeyForKtSwitch1(k);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int err=0;
  ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_PORT, err);
  ser_evt.addOutput((uint32_t)UNC_OP_UPDATE);
  ser_evt.addOutput((uint32_t)UNC_DT_STATE);
  ser_evt.addOutput((uint32_t)UNC_KT_SWITCH);
 
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplSwitchOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

/******GetAlarmStatus******/
//GetOneRow Success
TEST_F(KtClassTest, GetAlarmStatus_sucess_01) {
  key_switch_t k;
  Kt_Switch KtSwitchObj;
  uint64_t alarm_status = 1;
  getKeyForKtSwitch1(k);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.GetAlarmStatus(db_conn, UNC_DT_STATE, &k, alarm_status);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//GetOneRow Failure
TEST_F(KtClassTest, GetAlarmStatus_Nosuccess_01) {
  key_switch_t k;
  Kt_Switch KtSwitchObj;
  uint64_t alarm_status = 1;
  getKeyForKtSwitch1(k);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtSwitchObj.GetAlarmStatus(db_conn, UNC_DT_STATE, &k, alarm_status);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
 
/*****HandleDriverAlarms******/

//Reading alarm status from db failed
TEST_F(KtClassTest, HandleDriverAlarm_01) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  getKeyForKtSwitch1(k);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtSwitchObj.HandleDriverAlarms(db_conn, UNC_DT_STATE, alarm_type, oper_type, &k,&v);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

//Error creating ServerEvent object
//alarm_type:UNC_FLOW_ENT_FULL, oper_type:UNC_OP_CREATE
TEST_F(KtClassTest, HandleDriverAlarm_02) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  getKeyForKtSwitch1(k);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleDriverAlarms(db_conn, UNC_DT_STATE, alarm_type, oper_type, &k,&v);
  EXPECT_EQ(ret, UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, HandleDriverAlarm_03) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  uint32_t alarm_type = UNC_FLOW_ENT_FULL;
  uint32_t oper_type = UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn =NULL;
  getKeyForKtSwitch1(k);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleDriverAlarms(db_conn, UNC_DT_STATE, alarm_type, oper_type, &k,&v);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, HandleDriverAlarm_04) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  uint32_t alarm_type = UNC_OFS_LACK_FEATURES;
  uint32_t oper_type = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
  getKeyForKtSwitch1(k);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleDriverAlarms(db_conn, UNC_DT_STATE, alarm_type, oper_type, &k,&v);
  EXPECT_EQ(ret, UPPL_RC_ERR_IPC_WRITE_ERROR);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, HandleDriverAlarm_05) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  uint32_t alarm_type = UNC_OFS_LACK_FEATURES;
  uint32_t oper_type = UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn =NULL;
  getKeyForKtSwitch1(k);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleDriverAlarms(db_conn, UNC_DT_STATE, alarm_type, oper_type, &k,&v);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

/*****HandleOperStaus******/
TEST_F(KtClassTest, Handleoperstatus_BadRequest) {
  key_switch_t *k = NULL;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&v, 0, sizeof(val_switch_st_t));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  KtSwitchObj.HandleOperStatus(db_conn, UNC_DT_STATE, k, &v);
  EXPECT_EQ(ret, UPPL_RC_ERR_BAD_REQUEST);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, Handleoperstatus_GetOneRow_success) {
  key_switch_t k ;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, Handleoperstatus_GetOneRow_Fail) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  KtSwitchObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, Handleoperstatus_GetBulkRow_Success) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, Handleoperstatus_GetBulkRow_Failure) {
  key_switch_t k;
  val_switch_st_t v;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  memset(&v, 0, sizeof(val_switch_st_t));
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  KtSwitchObj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, GetSwitchValStructure) {
  key_switch_t k;
  val_switch_st_t *obj_val_switch;
  vector<TableAttrSchema> vect_table_attr_schema;
  vector<string> vect_prim_keys;
  uint8_t operation_type;
  val_switch_st_t *val_switch_valid_st;
  stringstream valid;
  Kt_Switch KtSwitchObj;
  memset(&k, 0, sizeof(key_switch_t));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = UPPL_RC_SUCCESS;
  KtSwitchObj.GetSwitchValStructure(db_conn, obj_val_switch,vect_table_attr_schema, vect_prim_keys,
                         operation_type, val_switch_valid_st,valid);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, GetSwitchValStructure_valid) {
  key_switch_t k;
  val_switch_st_t obj_val_switch;
  memset(&obj_val_switch,0,sizeof(obj_val_switch));
  vector<TableAttrSchema> vect_table_attr_schema;
  vector<string> vect_prim_keys;
  uint8_t operation_type;
  val_switch_st_t *val_switch_valid_st;
  stringstream valid;
  Kt_Switch KtSwitchObj;
  obj_val_switch.valid[kIdxSwitch] = UNC_VF_VALID;
  memset(&k, 0, sizeof(key_switch_t));
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = UPPL_RC_SUCCESS;
  KtSwitchObj.GetSwitchValStructure(db_conn, &obj_val_switch,vect_table_attr_schema, vect_prim_keys,
                         operation_type, val_switch_valid_st,valid);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}


/*******UpdateSwitchValidFlag******/

TEST_F(KtClassTest, UpdateSwitchValidFlag) {
  key_switch_t *k = new key_switch_t;
  val_switch_t *v = new val_switch_t;
  val_switch_st_t v_st;
 
  Kt_Switch KtSwitchObj;
  unc_keytype_validflag_t new_valid_val = UNC_VF_VALID;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  KtSwitchObj.UpdateSwitchValidFlag(db_conn, k, v, v_st, new_valid_val, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, UpdateSwitchValidFlag_Success) {
  key_switch_t *k = new key_switch_t;
  val_switch_t *v = new val_switch_t;
  val_switch_st_t v_st;
 
  Kt_Switch KtSwitchObj;
  unc_keytype_validflag_t new_valid_val = UNC_VF_VALID;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.UpdateSwitchValidFlag(db_conn, k, v, v_st, new_valid_val, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, UpdateSwitchValidFlag_NoFillVal) {
  key_switch_t *k = new key_switch_t;
  val_switch_t *v = new val_switch_t;
  val_switch_st_t v_st;
 
  Kt_Switch KtSwitchObj;
  unc_keytype_validflag_t new_valid_val;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.UpdateSwitchValidFlag(db_conn, k, v, v_st, new_valid_val, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, UpdateSwitchValidFlag_ret_NULL) {
  key_switch_t *k = new key_switch_t;
  val_switch_t *v = NULL;
  val_switch_st_t v_st;
 
  Kt_Switch KtSwitchObj;
  unc_keytype_validflag_t new_valid_val;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  KtSwitchObj.UpdateSwitchValidFlag(db_conn, k, v, v_st, new_valid_val, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

/********FrameValidValue*******/
TEST_F(KtClassTest, FrameValidValue) {
  string attr_value = "ctr";
  val_switch_st obj_val_switch;
  Kt_Switch KtSwitchObj;
  int ret = UPPL_RC_SUCCESS;
  obj_val_switch.valid[kIdxSwitch] = UNC_VF_VALID;
  KtSwitchObj.FrameValidValue(attr_value, obj_val_switch);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

