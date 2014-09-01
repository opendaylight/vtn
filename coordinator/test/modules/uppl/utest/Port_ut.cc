/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <limits.h>
#include <gtest/gtest.h>

#include <pfcxx/ipc_client.hh>

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
#include <odbcm_mgr.hh>
#include <physical_common_def.hh>
#include <unc/uppl_common.h>
#include <unc/keytype.h>
#include <itc_kt_base.hh>
#include <itc_kt_root.hh>
#include <itc_kt_controller.hh>
#include <itc_kt_port.hh>
#include <itc_kt_switch.hh>
#include <itc_kt_link.hh>
#include <itc_kt_boundary.hh>
#include <ipct_util.hh>
#include <itc_kt_state_base.hh>
#include <itc_kt_logicalport.hh>
#include <itc_read_request.hh>
#include <physicallayer.hh>
#include "PhysicalLayerStub.hh"
#include <tclib_module.hh>
#include "ut_util.hh"

using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
using namespace std;
using namespace unc::tclib;
using namespace unc::uppl::test;

class PortTest
  : public UpplTestEnv  {
};

// Can be changed based on testing need
static char pkName1_ctr[]  =  "Controller1";
static char pkName1_swtch[]  =  "0000-0000-0000-0001";
static char pkName1_port[]  =  "ethernet:1";

static void getKeyForKtPort1(key_port_t& k) {
  memset(&k, 0, sizeof(k));
  memcpy(k.sw_key.ctr_key.controller_name, pkName1_ctr, strlen(pkName1_ctr));
  memcpy(k.sw_key.switch_id, pkName1_swtch, strlen(pkName1_swtch));
  memcpy(k.port_id, pkName1_port, strlen(pkName1_port));
}

static void getValForKtPort1(val_port_st_t& v) {
  memset(&v, 0, sizeof(v));
  v.port.port_number  =  223;
  memcpy(v.port.description, "port description",
         strlen("port description"));  //  uint8_t description[128];
  v.port.admin_status  =  1;  //  uint8_t admin_status
  v.direction  =  1;      //  uint8_t direction
  v.port.trunk_allowed_vlan  =  1;  //  uint16_t trunk_allowed_vlan
  v.oper_status  =  1;
  v.direction  =  1;
  memcpy(v.mac_address, "port macAddr", strlen("port macAddr"));
  v.duplex  =  1;
  v.speed  =  1;
  v.alarms_status  =  1;
  memcpy(v.logical_port_id, "port logical_port_id",
         strlen("port logical_port_id"));
}

/* PerformSyntaxValidation when controller name is not given*/
TEST_F(PortTest, PerformSyntaxValidation_No_CtrName_01) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  getKeyForKtPort1(k);
  memset(k.sw_key.ctr_key.controller_name, '\0', 32);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktportobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

/* PerformSyntaxValidation when switch id is not given*/
TEST_F(PortTest, PerformSyntaxValidation_No_SwitchId_02) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  getKeyForKtPort1(k);
  memset(k.sw_key.switch_id, '\0', 256);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktportobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

/* PerformSyntaxValidation when Port id is not given */
TEST_F(PortTest, PerformSyntaxValidation_No_PortId_03) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  getKeyForKtPort1(k);
  memset(k.port_id, '\0', 32);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktportobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

/* PerformSyntaxValidation for positive case */
TEST_F(PortTest, PerformSyntaxValidation_Pos_04) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktportobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* PerformSemanticValidation when row already exists */
TEST_F(PortTest, PerformSemanticValidation_Create_Neg_01) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret  =  ktportobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);

  EXPECT_EQ(UNC_UPPL_RC_ERR_INSTANCE_EXISTS, ret);
}

/* PerformSemanticValidation when DB Connection error occurs for UPDATE */
TEST_F(PortTest, PerformSemanticValidation_update_neg_02) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t operation  =  UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret  =  ktportobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);

  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/* PerformSemanticValidation when DB error occurs for DELETE */
TEST_F(PortTest, PerformSemanticValidation_del_neg_03) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t operation  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret  =  ktportobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);

  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/* PerformSemanticValidation when creating a row */
TEST_F(PortTest, PerformSemanticValidation_Create_pos_04) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  int ret  =  ktportobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);

  EXPECT_EQ(UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST, ret);
}

/* PerformSemanticValidation when DB Connection error occurs for READ */
TEST_F(PortTest, PerformSemanticValidation_Read_neg_05) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t operation  =  UNC_OP_READ;
  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret  =  ktportobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);

  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/* PerformSemanticValidation when Updating row successfully */
TEST_F(PortTest, PerformSemanticValidation_update_pos_06) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t operation  =  UNC_OP_UPDATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret  =  ktportobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);

  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* PerformSemanticValidation when Deleting row successfully */
TEST_F(PortTest, PerformSemanticValidation_del_pos_07) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t operation  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret  =  ktportobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);

  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}


/* PerformSemanticValidation when Reading a row successfully */
TEST_F(PortTest, PerformSemanticValidation_read_pos_08) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t operation  =  UNC_OP_READ;
  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret  =  ktportobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);

  EXPECT_EQ(ODBCM_RC_SUCCESS, ret);
}

/* PerformSemanticValidation for IMPORT */
TEST_F(PortTest, PerformSemanticValidation_Create_Import_neg_09) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_IMPORT);

  EXPECT_EQ(UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST, ret);
}

/* HandleOperStatus when no key struct is given */
TEST_F(PortTest, HandleOperStatus_NoKeyStruct_01) {
  key_port_t *k;
  val_port_st *v;
  Kt_Port ktportobj;
  k  =  NULL;
  v  =  NULL;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktportobj.HandleOperStatus(db_conn, UNC_DT_STATE, k, v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

/* HandleOperStatus when GetOneRow fails */
TEST_F(PortTest, HandleOperStatus_GetOneRow_Fail_02) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = ktportobj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/* HandleOperStatus when GetOneRow is success */
TEST_F(PortTest, HandleOperStatus_GetOneRow_pos_03) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

/* HandleOperStatus when GetBulkRows fail */
TEST_F(PortTest, HandleOperStatus_GetBulkRows_Fail_04) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret = ktportobj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/* HandleOperStatus when GetBulkRows is success */
TEST_F(PortTest, HandleOperStatus_GetBulkRows_pos_05) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = ktportobj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

/* SetOperStatus when UPDATEONEROW fails*/
TEST_F(PortTest, SetOperStatus_DbNeg_01) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  int ret = ktportobj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplPortOperStatus)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

/* SetOperStatus when UPDATEONEROW fails*/
TEST_F(PortTest, SetOperStatus_DbNeg_02) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret = ktportobj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplPortOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* SetOperStatus when UPDATEONEROW is SUCCESS*/
TEST_F(PortTest, SetOperStatus_Db_Pos_03) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplPortOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* SetOperStatus when UPDATEONEROW is SUCCESS and GETONEROW fails*/
TEST_F(PortTest, SetOperStatus_Pos_04) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = ktportobj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplPortOperStatus)0);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* SetOperStatus when UPDATEONEROW and GETONEROW is SUCCESS
 * and addOutput err value is 0*/
TEST_F(PortTest, SetOperStatus_Pos_05) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplPortOperStatus)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* SetOperStatus when UPDATEONEROW and GETONEROW is SUCCESS
 * and addOutput err value is 0*/
TEST_F(PortTest, SetOperStatus_Pos_06) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.SetOperStatus(
    db_conn, UNC_DT_STATE, &k, (UpplPortOperStatus)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* FrameValidValue when string is more than 48 characters */
TEST_F(PortTest, FrameValidValue_01) {
  val_port_st v;
  memset(&v, 0xff, sizeof(v));

  string abd("01231234567");
  Kt_Port ktportobj;
  ktportobj.FrameValidValue(abd, v);

  for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(v.port.valid); i++) {
    ASSERT_EQ(i, v.port.valid[i]);
  }

  for (uint8_t i(1); i < PFC_ARRAY_CAPACITY(v.valid); i++) {
    ASSERT_EQ(i, v.valid[i]);
  }
}

/*PerformRead with negative option1 */
TEST_F(PortTest, PerformRead_Neg_Option1_01) {
  key_port_t k;
  val_port_st_t v;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  Kt_Port ktportobj;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess,
    (uint32_t)UNC_OPT1_DETAIL, (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*PerformRead with negative option1 */
TEST_F(PortTest, PerformRead_Neg_Option1_02) {
  key_port_t k;
  val_port_st_t v;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_DETAIL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PerformRead with negative option1 */
TEST_F(PortTest, PerformRead_Neg_Option1_WriteError) {
  key_port_t k;
  val_port_st_t v;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess,
    (uint32_t)UNC_OPT1_DETAIL, (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}


/*PerformRead with unsupported datatype */
TEST_F(PortTest, PerformRead_unsupported_datatype_03) {
  key_port_t k;
  val_port_st_t v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);
  getValForKtPort1(v);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_CANDIDATE, (uint32_t)UNC_OP_READ,
     sess, (uint32_t)UNC_OPT1_DETAIL, (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*PerformRead with negative option2 */
TEST_F(PortTest, PerformRead_neg_Option2_04) {
  key_port_t k;
  val_port_st_t v;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_MAC_ENTRY_STATIC);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_MAC_ENTRY_STATIC, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*PerformRead with negative option2 */
TEST_F(PortTest, PerformRead_neg_Option2_05) {
  key_port_t k;
  val_port_st_t v;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_MAC_ENTRY_STATIC);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION2);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_MAC_ENTRY_STATIC, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PerformRead with positive option1 */
TEST_F(PortTest, PerformRead_POS_Option2_06) {
  key_port_t k;
  val_port_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*PerformRead with positive option2 */
TEST_F(PortTest, PerformRead_POS_Option2_07) {
  key_port_t k;
  val_port_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
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
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_DB_GET);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PerformRead with positive option2 */
TEST_F(PortTest, PerformRead_POS_Option2_08) {
  key_port_t k;
  val_port_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
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
  sess.stub_setAddOutput((uint32_t)UNC_RC_SUCCESS);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PerformRead with option2 as Neighbor */
TEST_F(PortTest, PerformRead_Neighbor_09) {
  key_port_t k;
  val_port_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NEIGHBOR);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NEIGHBOR, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*PerformRead with positive option2 Neighbor */
TEST_F(PortTest, PerformRead_Neighbor_10) {
  key_port_t k;
  val_port_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NEIGHBOR);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_RC_SUCCESS);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NEIGHBOR, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PerformRead with positive option2 Neighbor*/
TEST_F(PortTest, PerformRead_Neighbor_11) {
  key_port_t k;
  val_port_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NEIGHBOR);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_DB_GET);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NEIGHBOR, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PerformRead with positive option2 Neighbor*/
TEST_F(PortTest, PerformRead_Neighbor_12) {
  key_port_t k;
  val_port_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NEIGHBOR);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
    (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NEIGHBOR, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*PerformRead with Invalid operation */
TEST_F(PortTest, PerformRead_Invalid_operation_13) {
  key_port_t k;
  val_port_st_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);

  int ret = ktportobj.PerformRead(db_conn, (uint32_t)0,
  (uint32_t)0, &k, &v, (uint32_t)UNC_DT_STATE,
  (uint32_t)UNC_OP_INVALID, sess, (uint32_t)UNC_OPT1_NORMAL,
  (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* DeleteKeyInstance for Unsupported datatype CANDIDATE */
TEST_F(PortTest, DeleteKeyInstance_UnsupportedForCANDIDATE_01) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_CANDIDATE, UNC_KT_PORT);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

/* DeleteKeyInstance for Unsupported datatype RUNNING */
TEST_F(PortTest, DeleteKeyInstance_UnsupportedForRUNNING_02) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_RUNNING, UNC_KT_PORT);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

/* DeleteKeyInstance for Unsupported datatype STARTUP */
TEST_F(PortTest, DeleteKeyInstance_UnsupportedForSTARTUP_03) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_STARTUP,
                                        UNC_KT_PORT);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

/* DeleteKeyInstance for Unsupported datatype AUDIT */
TEST_F(PortTest, DeleteKeyInstance_UnsupportedForAUDIT_04) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.DeleteKeyInstance(
    db_conn,
                                        &k, UNC_DT_AUDIT, UNC_KT_PORT);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

/* DeleteKeyInstance for DB error */
TEST_F(PortTest, DeleteKeyInstance_DbNeg_05) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.DeleteKeyInstance(
    db_conn,
                                        &k, UNC_DT_STATE, UNC_KT_PORT);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_DELETE, ret);
}

/* DeleteKeyInstance for DB error */
TEST_F(PortTest, DeleteKeyInstance_DbNeg_06) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktportobj.DeleteKeyInstance(
    db_conn,
                                        &k, UNC_DT_STATE, UNC_KT_PORT);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

/* DeleteKeyInstance for DB error */
TEST_F(PortTest, DeleteKeyInstance_DbNeg07) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret = ktportobj.DeleteKeyInstance(
    db_conn,
                                        &k, UNC_DT_STATE, UNC_KT_PORT);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/* DeleteKeyInstance for Positive DB  */
TEST_F(PortTest, DeleteKeyInstance_DbPos_08) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.DeleteKeyInstance(
    db_conn,
                                        &k, UNC_DT_STATE, UNC_KT_PORT);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* DeleteKeyInstance for Positive DB  */
TEST_F(PortTest, DeleteKeyInstance_DbPos_09) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_STATE, UNC_KT_PORT);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*IsKeyExists No key struct */
TEST_F(PortTest, IsKeyExists_NoKeyStruct_01) {
  key_port_t k;
  val_port_st_t v;
  memset(&v, 0, sizeof(val_port_st_t));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  int ret = ktportobj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

/*IsKeyExists when Db error occurs */
TEST_F(PortTest, IsKeyExists_DbNeg_02) {
  key_port_t k;
  val_port_st_t v;
  memset(&v, 0, sizeof(val_port_st_t));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret  =  ktportobj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

/*IsKeyExists when Db error occurs */
TEST_F(PortTest, IsKeyExists_DbNeg_03) {
  key_port_t k;
  val_port_st_t v;
  memset(&v, 0, sizeof(val_port_st_t));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_FAILED);
  int ret  =  ktportobj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/*IsKeyExists when Db Success occurs */
TEST_F(PortTest, IsKeyExists_Db_Pos_04) {
  key_port_t k;
  val_port_st_t v;
  memset(&v, 0, sizeof(val_port_st_t));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_swtch);
  sw_vect_key_value.push_back(pkName1_port);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret  =  ktportobj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*ReadInternal when it is other than Read operation */
TEST_F(PortTest, ReadInternal_NotRead_01) {
  vector<void*> k;
  vector<void*> v;
  Kt_Port ktportobj;
  uint32_t operation_type  =  UNC_OP_READ_BULK;

  key_port_t key;
  val_port_st_t value;
  memset(&value, 0, sizeof(value));
  getKeyForKtPort1(key);
  getValForKtPort1(value);
  k.push_back(&key);
  v.push_back(&value);

  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.ReadInternal(db_conn, k, v, UNC_DT_STATE,
                                     operation_type);

  EXPECT_EQ(2008, ret);
}

/*ReadInternal when Db is SUCCESS */
TEST_F(PortTest, ReadInternal_Read_02) {
  vector<void*> k;
  vector<void*> v;
  Kt_Port ktportobj;
  uint32_t operation_type  =  UNC_OP_READ;

  key_port_t key;
  val_port_st_t val;
  getKeyForKtPort1(key);
  getValForKtPort1(val);
  k.push_back(&key);
  v.push_back(&val);

  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.ReadInternal(db_conn, k, v, UNC_DT_STATE,
                                     operation_type);

  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*ReadBulk Unsupported datatype UNC_DT_CANDIDATE */
TEST_F(PortTest, ReadBulk_Unsupporteddatatype_01) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct(1);
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;

  int ret = ktportobj.ReadBulk(
    db_conn, &k, (uint32_t)UNC_DT_CANDIDATE,
    max_rep_ct, 0, PFC_TRUE, PFC_TRUE, read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

/*ReadBulk Unsupported datatype UNC_DT_RUNNING */
TEST_F(PortTest, ReadBulk_Unsupporteddatatype_02) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct(1);
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = ktportobj.ReadBulk(
    db_conn, &k, (uint32_t)UNC_DT_RUNNING,
    max_rep_ct, 0, PFC_TRUE, PFC_TRUE, read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

/*ReadBulk Unsupported datatype UNC_DT_STARTUP */
TEST_F(PortTest, ReadBulk_Unsupporteddatatype_03) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct(1);
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = ktportobj.ReadBulk(
    db_conn, &k, (uint32_t)UNC_DT_STARTUP,
    max_rep_ct, 0, PFC_TRUE, PFC_TRUE, read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

/*ReadBulk when max_rep_ct is NULL */
TEST_F(PortTest, ReadBulk_max_rep_ct_NULL_04) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct(1);
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = ktportobj.ReadBulk(
    db_conn, &k, (uint32_t)UNC_DT_STATE,
     max_rep_ct, 0, PFC_TRUE, PFC_TRUE, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*ReadBulk when DB is SUCCESS */
TEST_F(PortTest, ReadBulk_Pos_05) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct = 1;
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  int child_index = 1;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = ktportobj.ReadBulk(
    db_conn, &k, (uint32_t)UNC_DT_STATE,
    max_rep_ct, child_index, parent_call,
    is_read_next, &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*ReadBulk when DB Connection error occurs  */
TEST_F(PortTest, ReadBulk_DbNeg_06) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct = 1;
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  int child_index = 1;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktportobj.ReadBulk(
    db_conn, &k, (uint32_t)UNC_DT_STATE,
    max_rep_ct, child_index, parent_call,
    is_read_next, &read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

/*ReadBulk when parent_call is false*/
TEST_F(PortTest, ReadBulk_Pos_parent_false_07) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct = 1;
  pfc_bool_t parent_call = false;
  pfc_bool_t is_read_next = true;
  int child_index = 1;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret = ktportobj.ReadBulk(
    db_conn, &k, (uint32_t)UNC_DT_STATE,
    max_rep_ct, child_index, parent_call,
    is_read_next, &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* ReadBulkInternal when there is no record in DB */
TEST_F(PortTest, ReadBulkInternal_DbNeg_01) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct(1);
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = ktportobj.ReadBulkInternal(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/* ReadBulkInternal when DB connection error occurs */
TEST_F(PortTest, ReadBulkInternal_DbNeg_02) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct(1);
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktportobj.ReadBulkInternal(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

/* ReadBulkInternal when DB error occurs */
TEST_F(PortTest, ReadBulkInternal_DbNeg_03) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct(1);
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret = ktportobj.ReadBulkInternal(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/* ReadBulkInternal when DB is success */
TEST_F(PortTest, ReadBulkInternal_Pos_04) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct(1);
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = ktportobj.ReadBulkInternal(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* ReadBulkInternal when DB is success */
TEST_F(PortTest, ReadBulkInternal_Pos_05) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t max_rep_ct = 0;
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadBulkInternal(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*ReadPortValFromDB when it is other than READ operation */
TEST_F(PortTest, ReadPortValFromDB_NoREAD_01) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  memset(k.sw_key.ctr_key.controller_name, '\0', 32);
  memset(k.sw_key.switch_id, '\0', 256);
  memset(k.port_id, '\0', 32);
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  uint32_t operation_type  =  UNC_OP_CREATE;
  uint32_t max_rep_ct;

  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadPortValFromDB(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     operation_type, max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*ReadPortValFromDB when there is no record in DB for GETONEROW */
TEST_F(PortTest, ReadPortValFromDB_ReadNeg_02) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  memset(k.sw_key.ctr_key.controller_name, '\0', 32);
  memset(k.sw_key.switch_id, '\0', 256);
  memset(k.port_id, '\0', 32);
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  uint32_t operation_type  =  UNC_OP_READ;
  uint32_t max_rep_ct;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadPortValFromDB(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     operation_type, max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/*ReadPortValFromDB when there is DB connection error for GETONEROW */
TEST_F(PortTest, ReadPortValFromDB_ReadNeg_03) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  memset(k.sw_key.ctr_key.controller_name, '\0', 32);
  memset(k.sw_key.switch_id, '\0', 256);
  memset(k.port_id, '\0', 32);
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  uint32_t operation_type  =  UNC_OP_READ;
  uint32_t max_rep_ct;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadPortValFromDB(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     operation_type, max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

/*ReadPortValFromDB when there is DB Failure for GETONEROW */
TEST_F(PortTest, ReadPortValFromDB_ReadNeg_04) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  memset(k.sw_key.ctr_key.controller_name, '\0', 32);
  memset(k.sw_key.switch_id, '\0', 256);
  memset(k.port_id, '\0', 32);
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  uint32_t operation_type  =  UNC_OP_READ;
  uint32_t max_rep_ct;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadPortValFromDB(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     operation_type, max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/*ReadPortValFromDB when there is DB Success for GETONEROW */
TEST_F(PortTest, ReadPortValFromDB_ReadPOS_05) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  memset(k.sw_key.ctr_key.controller_name, '\0', 32);
  memset(k.sw_key.switch_id, '\0', 256);
  memset(k.port_id, '\0', 32);
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  uint32_t operation_type  =  UNC_OP_READ;
  uint32_t max_rep_ct;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadPortValFromDB(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     operation_type, max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(ODBCM_RC_SUCCESS, ret);
}

/*ReadPortValFromDB when there is DB Failure for GETONEROW */
TEST_F(PortTest, ReadPortValFromDB_ReadBulkNeg_06) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  memset(k.sw_key.ctr_key.controller_name, '\0', 32);
  memset(k.sw_key.switch_id, '\0', 256);
  memset(k.port_id, '\0', 32);
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  uint32_t operation_type  =  UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadPortValFromDB(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     operation_type, max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/*ReadPortValFromDB when there is DB connection error for GETBULKROWS */
TEST_F(PortTest, ReadPortValFromDB_ReadBulkNeg_07) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  memset(k.sw_key.ctr_key.controller_name, '\0', 32);
  memset(k.sw_key.switch_id, '\0', 256);
  memset(k.port_id, '\0', 32);
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  uint32_t operation_type  =  UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadPortValFromDB(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     operation_type, max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

/*ReadPortValFromDB when there is DB Failure for GETBULKROWS */
TEST_F(PortTest, ReadPortValFromDB_ReadBulkNeg_08) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  memset(k.sw_key.ctr_key.controller_name, '\0', 32);
  memset(k.sw_key.switch_id, '\0', 256);
  memset(k.port_id, '\0', 32);
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  uint32_t operation_type  =  UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadPortValFromDB(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     operation_type, max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/*ReadPortValFromDB when there is DB Success for GETBULKROWS */
TEST_F(PortTest, ReadPortValFromDB_09_ReadBulkPos) {
  key_port_t k;
  val_port_st v;
  Kt_Port ktportobj;
  memset(k.sw_key.ctr_key.controller_name, '\0', 32);
  memset(k.sw_key.switch_id, '\0', 256);
  memset(k.port_id, '\0', 32);
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  vector<val_port_st_t> vect_val_port;
  vector<key_port_t> vect_port_id;
  uint32_t operation_type  =  UNC_OP_READ_BULK;
  uint32_t max_rep_ct;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadPortValFromDB(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     operation_type, max_rep_ct, vect_val_port, vect_port_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*ReadNeighbor when negative case of DB */
TEST_F(PortTest, ReadNeighbor_Dbneg_01) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  val_port_st_neighbor neighbor_obj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktportobj.ReadNeighbor(
    db_conn, &k, &k, (uint32_t)UNC_DT_STATE,
     neighbor_obj);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/*ReadNeighbor when Positive case of DB */
TEST_F(PortTest, ReadNeighbor_Dbneg_02) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  val_port_st_neighbor neighbor_obj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.ReadNeighbor(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     neighbor_obj);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*ReadNeighbor when there is no record in DB */
TEST_F(PortTest, ReadNeighbor_Dbneg_03) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  val_port_st_neighbor neighbor_obj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = ktportobj.ReadNeighbor(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     neighbor_obj);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/*ReadNeighbor when there is Connection error in DB */
TEST_F(PortTest, ReadNeighbor_Dbneg_04) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  val_port_st_neighbor neighbor_obj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktportobj.ReadNeighbor(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     neighbor_obj);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

/*ReadNeighbor when DB fails */
TEST_F(PortTest, ReadNeighbor_Dbneg_05) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  val_port_st_neighbor neighbor_obj;
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = ktportobj.ReadNeighbor(
    db_conn, &k, &v, (uint32_t)UNC_DT_STATE,
     neighbor_obj);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/*PopulateSchemaForValidFlag when UPDATEONEROW is Success */
TEST_F(PortTest, PopulateSchemaForValidFlag_pos_01) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  string valid_new  =  "123456789ab";
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.PopulateSchemaForValidFlag(
    db_conn, &k, &v, valid_new, (uint32_t)UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PopulateSchemaForValidFlag when UPDATEONEROW is failed */
TEST_F(PortTest, PopulateSchemaForValidFlag_neg_02) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  string valid_new  =  "asss";
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  int ret = ktportobj.PopulateSchemaForValidFlag(
    db_conn, &k, &v, valid_new, (uint32_t)UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

/*UpdatePortValidFlag when UPDATEONEROW is failed */
TEST_F(PortTest, UpdatePortValidFlag_01) {
  key_port_t k;
  val_port_st v;
  memset(&v, 0, sizeof(v));
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  string valid_new  =  "asss";
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_FAILED);
  int ret = ktportobj.PopulateSchemaForValidFlag(
    db_conn, &k, &v, valid_new, (uint32_t)UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

/*GetAlarmStatus function when GETONEROW is success */
TEST_F(PortTest, GetAlarmStatus_sucess_01) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint64_t alarm_status  =  1;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.GetAlarmStatus(db_conn, UNC_DT_STATE, &k, alarm_status);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*GetAlarmStatus function when GETONEROW is failed */
TEST_F(PortTest, GetAlarmStatus_sucess_02) {
  key_port_t k;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint64_t alarm_status  =  1;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = ktportobj.GetAlarmStatus(db_conn, UNC_DT_STATE, &k, alarm_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/*HandleDriverAlarms function when GETONEROW and UPDATEONEROW
 * is SUCCESS for UNC_DEFAULT_FLOW alarm */
TEST_F(PortTest, HandleDriverAlarms_01) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_DEFAULT_FLOW;
  uint32_t oper_type  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*HandleDriverAlarms function when GETONEROW and UPDATEONEROW
 * is SUCCESS for UNC_DEFAULT_FLOW alarm */
TEST_F(PortTest, HandleDriverAlarms_02) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_DEFAULT_FLOW;
  uint32_t oper_type  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*HandleDriverAlarms function when GETONEROW and UPDATEONEROW
 * is SUCCESS for UNC_PORT_DIRECTION alarm */
TEST_F(PortTest, HandleDriverAlarms_03) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_DIRECTION;
  uint32_t oper_type  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*HandleDriverAlarms function when GETONEROW and UPDATEONEROW
 * is SUCCESS for UNC_PORT_DIRECTION alarm */
TEST_F(PortTest, HandleDriverAlarms_04) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_DIRECTION;
  uint32_t oper_type  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*HandleDriverAlarms function when GETONEROW
 * and UPDATEONEROW is SUCCESS for UNC_PORT_CONGES alarm */
TEST_F(PortTest, HandleDriverAlarms_05) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_CONGES;
  uint32_t oper_type  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*HandleDriverAlarms function when GETONEROW and
 * UPDATEONEROW is SUCCESS for UNC_PORT_CONGES alarm */
TEST_F(PortTest, HandleDriverAlarms_06) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_CONGES;
  uint32_t oper_type  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*HandleDriverAlarms function when GETONEROW is Failed
 * and UPDATEONEROW is SUCCESS for UNC_PORT_DIRECTION alarm */
TEST_F(PortTest, HandleDriverAlarms_07) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_DIRECTION;
  uint32_t oper_type  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*HandleDriverAlarms function when GETONEROW is Failed
 * and UPDATEONEROW is SUCCESS for UNC_PORT_DIRECTION alarm */
TEST_F(PortTest, HandleDriverAlarms_08) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_DIRECTION;
  uint32_t oper_type  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*HandleDriverAlarms function when GETONEROW is Failed
 * and UPDATEONEROW is SUCCESS for UNC_DEFAULT_FLOW alarm */
TEST_F(PortTest, HandleDriverAlarms_09) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_DEFAULT_FLOW;
  uint32_t oper_type  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*HandleDriverAlarms function when GETONEROW is Failed
 * and UPDATEONEROW is SUCCESS for UNC_DEFAULT_FLOW alarm */
TEST_F(PortTest, HandleDriverAlarms_10) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_DEFAULT_FLOW;
  uint32_t oper_type  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*HandleDriverAlarms function when GETONEROW is Failed
 * and UPDATEONEROW is SUCCESS for UNC_PORT_CONGES alarm */
TEST_F(PortTest, HandleDriverAlarms_11) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_CONGES;
  uint32_t oper_type  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*HandleDriverAlarms function when GETONEROW
 * is Failed and UPDATEONEROW is SUCCESS for UNC_PORT_CONGES alarm */
TEST_F(PortTest, HandleDriverAlarms_12) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_CONGES;
  uint32_t oper_type  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*HandleDriverAlarms function when GETONEROW
 * and UPDATEONEROW is SUCCESS for UNC_PORT_CONGES alarm */
TEST_F(PortTest, HandleDriverAlarms_13) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_CONGES;
  uint32_t oper_type  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_IMPORT, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*HandleDriverAlarms function when GETONEROW
 * and UPDATEONEROW is SUCCESS for UNC_PORT_CONGES alarm */
TEST_F(PortTest, HandleDriverAlarms_14) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_CONGES;
  uint32_t oper_type  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/* HandleDriverAlarms when GETONEROW and UPDATEONEROW are success */
TEST_F(PortTest, HandleDriverAlarm_15) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  Kt_Port ktportobj;
  uint32_t alarm_type  =  UNC_PORT_CONGES;
  uint32_t oper_type  =  UNC_OP_DELETE;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)UNC_OP_UPDATE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_KT_PORT);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.HandleDriverAlarms(
    db_conn, UNC_DT_STATE, alarm_type, oper_type, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_FAILURE, ret);
}

/*SubDomainOperStatusHandling when GETBULKROWS is success*/
TEST_F(PortTest, SubDomainOperStatusHandling_Dbneg_01) {
  val_port_st v;
  Kt_Port ktportobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  string controller_name  =  "Controller1";
  string switch_id  =  "switch id1";
  string physical_port_id  =  "Port1";
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret  =  ktportobj.SubDomainOperStatusHandling(
    db_conn, UNC_DT_STATE, controller_name, switch_id, physical_port_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*SubDomainOperStatusHandling when GETBULKROWS is failed */
TEST_F(PortTest, SubDomainOperStatusHandling_Dbneg_02) {
  val_port_st v;
  Kt_Port ktportobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.port.valid, '\0', sizeof(v.port.valid));
  string controller_name  =  "Controller1";
  string switch_id  =  "switch id1";
  string physical_port_id  =  "Port1";
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret  =  ktportobj.SubDomainOperStatusHandling(
    db_conn, UNC_DT_STATE, controller_name, switch_id, physical_port_id);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*NotifyOperStatus when GETBULKROWS is failed */
TEST_F(PortTest, NotifyOperStatus_01) {
  key_port_t k;
  val_port_st v;
  getKeyForKtPort1(k);

  vector<OperStatusHolder> ref_oper_status;
  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret  =  ktportobj.NotifyOperStatus(
    db_conn, UNC_DT_STATE, &k, &v, ref_oper_status);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

/*GetPortValStructure function calling */
TEST_F(PortTest, GetPortValStructure_01) {
  key_port_t k;
  getKeyForKtPort1(k);

  val_port_st_t *obj_val_port(NULL);
  val_port_st_t *val_port_valid_st(NULL);
  vector<TableAttrSchema> vect_table_attr_schema;
  vector<string> vect_prim_keys;
  uint8_t operation_type = UNC_OP_UPDATE;
  stringstream valid;
  Kt_Port ktportobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret  =  UNC_RC_SUCCESS;
  ktportobj.GetPortValStructure(
    db_conn, obj_val_port, vect_table_attr_schema, vect_prim_keys,
                         operation_type, val_port_valid_st, valid);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/* UpdatePortValidFlag when there is no record in DB*/
TEST_F(PortTest, UpdatePortValidFlag_DbNeg_01) {
  key_port_t k;
  val_port_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
  val_port_st_t v_st;
  memset(&v_st, 0, sizeof(v_st));

  unc_keytype_validflag_t new_valid_val  =  UNC_VF_VALID;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = ktportobj.UpdatePortValidFlag(db_conn, &k, &v, v_st,
                                           new_valid_val, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

/* UpdatePortValidFlag when GETONEROW is success */
TEST_F(PortTest, UpdatePortValidFlag_Dbpos_02) {
  key_port_t k;
  val_port_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
  val_port_st_t v_st;
  memset(&v_st, 0, sizeof(v_st));

  unc_keytype_validflag_t new_valid_val  =  UNC_VF_VALID;
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.UpdatePortValidFlag(db_conn, &k, &v, v_st,
                                           new_valid_val, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*UpdatePortValidFlag when GETONEROW is success */
TEST_F(PortTest, UpdatePortValidFlag_NoFillValue_03) {
  key_port_t k;
  val_port_t v;
  memset(&k, 0, sizeof(k));
  memset(&v, 0, sizeof(v));

  Kt_Port ktportobj;
  val_port_st_t v_st;
  memset(&v_st, 0, sizeof(v_st));

  unc_keytype_validflag_t new_valid_val(UNC_VF_INVALID);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktportobj.UpdatePortValidFlag(db_conn, &k, &v, v_st,
                                           new_valid_val, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}
