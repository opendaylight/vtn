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
#include <tclib_module.hh>
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
#include <itc_kt_logical_member_port.hh>
#include <itc_kt_logicalport.hh>
#include <ipct_util.hh>
#include <physicallayer.hh>
#include "PhysicalLayerStub.hh"
#include <itc_read_request.hh>
#include "ut_util.hh"

using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
using namespace std;
using namespace unc::uppl::test;

class LinkTest
  : public UpplTestEnv {
};

//  Can be changed based on testing need
static char pkName1_ctr[]  =  "controller1";
static char pkName1_switchid1[]  =  "{0x10, 0xbc}";
static char pkName1_portid1[]  =  "controller1 port";
static char pkName1_switchid2[]  =  "{0x11, 0xab}";
static char pkName1_portid2[]  =  "controller1 port 4";
static char pkName4_ctr[]  =  "";
static char pkName4_switchid1[]  =  "";
static char pkName4_portid1[]  =  "";
static char pkName4_switchid2[]  =  "";
static char pkName4_portid2[]  =  "";

static void getKeyForKtLink1(key_link_t& k,
                             std::vector<string>& sw_vect_key_value) {
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName1_ctr, strlen(pkName1_ctr));
  memset(k.switch_id1, '\0', 256);
  memcpy(k.switch_id1, pkName1_switchid1, strlen(pkName1_switchid1));
  memset(k.port_id1, '\0', 32);
  memcpy(k.port_id1, pkName1_portid1, strlen(pkName1_portid1));
  memset(k.switch_id2, '\0', 256);
  memcpy(k.switch_id2, pkName1_switchid2, strlen(pkName1_switchid2));
  memset(k.port_id2, '\0', 32);
  memcpy(k.port_id2, pkName1_portid2, strlen(pkName1_portid2));

  sw_vect_key_value.push_back(pkName1_ctr);
  sw_vect_key_value.push_back(pkName1_switchid1);
  sw_vect_key_value.push_back(pkName1_portid1);
  sw_vect_key_value.push_back(pkName1_switchid2);
  sw_vect_key_value.push_back(pkName1_portid2);
}

static void getKeyForKtLink1(key_link_t& k) {
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName1_ctr, strlen(pkName1_ctr));
  memset(k.switch_id1, '\0', 256);
  memcpy(k.switch_id1, pkName1_switchid1, strlen(pkName1_switchid1));
  memset(k.port_id1, '\0', 32);
  memcpy(k.port_id1, pkName1_portid1, strlen(pkName1_portid1));
  memset(k.switch_id2, '\0', 256);
  memcpy(k.switch_id2, pkName1_switchid2, strlen(pkName1_switchid2));
  memset(k.port_id2, '\0', 32);
  memcpy(k.port_id2, pkName1_portid2, strlen(pkName1_portid2));
}

static void getValForKtLink1(val_link_st_t& v) {
  //  uint8_t description[128];
  memset(v.link.description, '\0', 128);
  memcpy(v.link.description, "linkdescription", strlen("linkdescription"));

  //  uint8_t oper_status
  v.oper_status  =  0;

  //  uint8_t valid[2];
  memset(v.valid, 1, 2);
}

static void getKeyForLinkNoKeyNotify(key_link_t& k) {
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName4_ctr, strlen(pkName4_ctr));
  memset(k.switch_id1, '\0', 256);
  memcpy(k.switch_id1, pkName4_switchid1, strlen(pkName4_switchid1));
  memset(k.port_id1, '\0', 32);
  memcpy(k.port_id1, pkName4_portid1, strlen(pkName4_portid1));
  memset(k.switch_id2, '\0', 256);
  memcpy(k.switch_id2, pkName4_switchid2, strlen(pkName4_switchid2));
  memset(k.port_id2, '\0', 32);
  memcpy(k.port_id2, pkName4_portid2, strlen(pkName4_portid2));
}

static void getReqHeader(physical_request_header& rh,
                         unc_keytype_operation_t opern,
                         unc_keytype_datatype_t dtype) {
  rh.client_sess_id  =  1;
  rh.config_id  =  1;
  rh.operation  =  opern;
  rh.max_rep_count  =  0;
  rh.option1  =  0;
  rh.option2  =  0;
  rh.data_type  =  dtype;
  rh.key_type  =  UNC_KT_LINK;
}

TEST_F(LinkTest, PerformSyntxCheck_01) {
  key_link_t k;
  val_link_st v;
  memset(&v, 0, sizeof(v));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getKeyForKtLink1(k);
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_CREATE;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// Key Empty
TEST_F(LinkTest, PerformSyntxCheck_02) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(&v, 0, sizeof(v));
  getKeyForKtLink1(k);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// indfividual Attributes havin mepty values
TEST_F(LinkTest, PerformSyntxCheck_03) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForLinkNoKeyNotify(k);
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
// Setting controller name as empty
TEST_F(LinkTest, PerformSyntxCheck_04) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName4_ctr, strlen(pkName4_ctr));
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Setting switchID1 name as empty
TEST_F(LinkTest, PerformSyntxCheck_05) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);

  memset(k.switch_id1, '\0', 256);
  memcpy(k.switch_id1, pkName4_switchid1, strlen(pkName4_switchid1));

  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Setting switchID2 name as empty
TEST_F(LinkTest, PerformSyntxCheck_06) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);

  memset(k.switch_id2, '\0', 256);
  memcpy(k.switch_id2, pkName4_switchid2, strlen(pkName4_switchid2));

  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Setting portID1 name as empty
TEST_F(LinkTest, PerformSyntxCheck_07) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);

  memset(k.port_id1, '\0', 32);
  memcpy(k.port_id1, pkName4_portid1, strlen(pkName4_portid1));

  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

// Setting portID2 name as empty
TEST_F(LinkTest, PerformSyntxCheck_08) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);

  memset(k.port_id2, '\0', 32);
  memcpy(k.port_id2, pkName4_portid2, strlen(pkName4_portid2));

  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
// Setting description as empty
TEST_F(LinkTest, PerformSyntxCheck_09) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);

  memset(v.link.description, '\0', 128);
  memcpy(v.link.description, "", strlen(""));
  //  uint8_t                 description[128];
  //  uint8_t oper_status
  v.oper_status  =  0;
  memset(v.valid, 1, 2);  //  uint8_t

  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  // EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// Setting description length more than 128
TEST_F(LinkTest, PerformSyntxCheck_10) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);

  const char* strdes  =  "Alter the SV_INTERRUPT property of a signal handler. If interrupt is zero, system calls will be restarted after signal delivery kkkkkkkkkkkkkkkkkkk";

  //  uint8_t                 description[128];
  memset(v.link.description, '\0', sizeof(v.link.description));
  strncpy(reinterpret_cast<char *>(v.link.description), strdes,
          sizeof(v.link.description));
  EXPECT_NE(static_cast<uint8_t>('\0'), v.link.description[127]);

  //  uint8_t oper_status
  v.oper_status  =  0;
  memset(v.valid, 1, 2);  //  uint8_t

  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);

  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// Setting oper status as invalid
TEST_F(LinkTest, PerformSyntxCheck_11) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);

  const char* strdes  =  "link des";

  memset(v.link.description, '\0', 128);
  memcpy(v.link.description, strdes , strlen(strdes));

  v.oper_status  =  4;
  memset(v.valid, 1, 2);  //  uint8_t

  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation  =  UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn  = NULL;

  int ret = ktlinkobj.PerformSyntaxValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);

  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// key as Empty
TEST_F(LinkTest, Link_IsKeyExists_01) {
  key_link_t k;
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = ktlinkobj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

TEST_F(LinkTest, Link_IsKeyExists_FailureIsrowexist02) {
  key_link_t k;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;

  getKeyForKtLink1(k, sw_vect_key_value);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret = ktlinkobj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LinkTest, Link_IsKeyExists_SuccessIsrowexist03) {
  key_link_t k;
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn  = NULL;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = ktlinkobj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, Link_IsKeyExists_FailConnectionErr04) {
  key_link_t k;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;

  getKeyForKtLink1(k);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn  = NULL;
  getKeyForKtLink1(k, sw_vect_key_value);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.IsKeyExists(db_conn, UNC_DT_STATE, sw_vect_key_value);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LinkTest, PerformSemanticValidation_InstanceExist_create05) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;

  getKeyForKtLink1(k, sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = ktlinkobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_INSTANCE_EXISTS, ret);
}

TEST_F(LinkTest, PerformSemanticValidation_ConnectionErr_create06) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;

  getKeyForKtLink1(k, sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LinkTest, PerformSemanticValidation_InstanceNOtExist_update07) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_UPDATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret = ktlinkobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LinkTest, PerformSemanticValidation_ConnectionErr_update08) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_UPDATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LinkTest, PerformSemanticValidation_InstanceNOtExist_delete09) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_DELETE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret = ktlinkobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LinkTest, PerformSemanticValidation_ConnectionErr_delete10) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_DELETE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LinkTest, PerformSemanticValidation_InstanceNOtExist_read11) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_READ;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret = ktlinkobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LinkTest, PerformSemanticValidation_ConnectionErr_Read12) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_READ;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LinkTest, PerformSemanticValidation_InstanceNOtExist_create13) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret = ktlinkobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST, ret);
}

TEST_F(LinkTest, LinkDeleteKeyInstance_UnsupportedForSTARTUP_14) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret  =  ktlinkobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_STARTUP, UNC_KT_LINK);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

TEST_F(LinkTest, LinkDeleteKeyInstance_UnsupportedForSTARTUP_15) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret  =  ktlinkobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_CANDIDATE, UNC_KT_LINK);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

TEST_F(LinkTest, LinkDeleteKeyInstance_UnsupportedForSTARTUP_16) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret  =  ktlinkobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_RUNNING, UNC_KT_LINK);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

TEST_F(LinkTest, LinkDeleteKeyInstance_Support_17) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret  =  ktlinkobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_STATE, UNC_KT_LINK);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_DELETE, ret);
}

TEST_F(LinkTest, DeleteKeyInstance_Support_18) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret  =  ktlinkobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_STATE, UNC_KT_LINK);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LinkTest, DeleteKeyInstance_Support_19) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret  =  ktlinkobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_STATE, UNC_KT_LINK);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LinkTest, LinkDeleteKeyInstance_Support_20) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktlinkobj.DeleteKeyInstance(
    db_conn, &k, UNC_DT_STATE, UNC_KT_LINK);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, LinkPerformRead_incorrectoption1_21) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type  =  UNC_KT_LINK;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_LINK);

  int ret = ktlinkobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
     (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ, sess,
     (uint32_t)UNC_OPT1_DETAIL, (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(LinkTest, LinkPerformRead_incorrectDT_22) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type  =  UNC_KT_LINK;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LINK);
int ret = ktlinkobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
    (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ,
    sess, (uint32_t)UNC_OPT1_NORMAL, (uint32_t)UNC_OPT2_L2DOMAIN, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}
TEST_F(LinkTest, LinkPerformRead_incorrectoption2_23) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type  =  UNC_KT_LINK;
  OdbcmConnectionHandler *db_conn  = NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_L2DOMAIN);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LINK);
int ret = ktlinkobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
     (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ,
     sess, (uint32_t)UNC_OPT1_NORMAL, (uint32_t)UNC_OPT2_L2DOMAIN, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(LinkTest, LinkSetOperStatus_24) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = ktlinkobj.SetOperStatus(
      db_conn, UNC_DT_STATE, &k, (UpplLinkOperStatus)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_UPDATE, ret);
}

TEST_F(LinkTest, LinkSetOperStatus_25) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =
      ktlinkobj.SetOperStatus(db_conn, UNC_DT_STATE, &k, (UpplLinkOperStatus)0);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LinkTest, LinkSetOperStatus_26) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =
      ktlinkobj.SetOperStatus(db_conn, UNC_DT_STATE, &k, (UpplLinkOperStatus)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, LinkSetOperStatus_27) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getKeyForKtLink1(k);
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret  =
      ktlinkobj.SetOperStatus(db_conn, UNC_DT_STATE, &k, (UpplLinkOperStatus)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, LinkSetOperStatus_28) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getKeyForKtLink1(k);
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.SetOperStatus(db_conn,
                                       UNC_DT_STATE, &k, (UpplLinkOperStatus)2);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, LinkReadBulk_Opnt_allow_29) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t max_rep_ct  =  1;
  int child_index = 0;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = true;
  OdbcmConnectionHandler *db_conn  =  NULL;
  ReadRequest *read_req  =  NULL;
  int ret  =  ktlinkobj.ReadBulk(
    db_conn, &k, UNC_DT_CANDIDATE,
    max_rep_ct, child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

TEST_F(LinkTest, LinkReadBulk_Success_30) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t max_rep_ct  =  0;
  int child_index = -1;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = true;
  OdbcmConnectionHandler *db_conn  =  NULL;
  ReadRequest *read_req  =  NULL;
  int ret  =  ktlinkobj.ReadBulk(
    db_conn, &k, UNC_DT_STATE,
    max_rep_ct, child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, LinkReadBulk_MaxCntSuccess_31) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t max_rep_ct  =  0;
  int child_index = -1;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = true;
  OdbcmConnectionHandler *db_conn  =  NULL;
  ReadRequest *read_req  =  NULL;
  int ret  =  ktlinkobj.ReadBulk(
    db_conn, &k, UNC_DT_STATE,
    max_rep_ct, child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}


TEST_F(LinkTest, LinkReadBulk_DBFail_32) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t max_rep_ct  =  0;
  int child_index = -1;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = true;
  OdbcmConnectionHandler *db_conn  =  NULL;
  ReadRequest *read_req  =  NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret  =  ktlinkobj.ReadBulk(
    db_conn, &k, UNC_DT_STATE,
    max_rep_ct, child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, LinkReadBulk_DBSuccess) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t max_rep_ct  =  0;
  int child_index = -1;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = true;
  OdbcmConnectionHandler *db_conn  =  NULL;
  ReadRequest *read_req  =  NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret  =  ktlinkobj.ReadBulk(
    db_conn, &k, UNC_DT_STATE, max_rep_ct,
    child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, LinkReadBulkInternal_Success_33) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t max_rep_ct  =  0;
  OdbcmConnectionHandler *db_conn  =  NULL;
  vector<val_link_st_t> vec_val_link;
  vector<key_link_t> vec_key_link_t;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret  =  ktlinkobj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, vec_val_link, vec_key_link_t);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LinkTest, LinkReadBulkInternal_Success_34) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t max_rep_ct  =  0;
  OdbcmConnectionHandler *db_conn  =  NULL;

  getKeyForKtLink1(k);

  vector<val_link_st_t> vec_val_link;
  vector<key_link_t> vec_key_link_t;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret  =  ktlinkobj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, vec_val_link, vec_key_link_t);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LinkTest, HandleOperStatus_KeyStruct_NULL_Handdle) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn  = NULL;
//   unc::uppl::ODBCManager::stub_setResultcode(
//    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}


TEST_F(LinkTest, GetLinkValidFlag_ConnError) {
  key_link_t k;
  val_link_st_t v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st_t));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn  =  NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret  =  ktlinkobj.GetLinkValidFlag(db_conn, &k, v, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LinkTest, GetLinkValidFlag_DBGetErr) {
  key_link_t k;
  val_link_st_t v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st_t));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn  =  NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_DATA_ERROR);
  int ret  =  ktlinkobj.GetLinkValidFlag(db_conn, &k, v, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(LinkTest, GetLinkValidFlag_Success) {
  key_link_t k;
  val_link_st_t v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st_t));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn  =  NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktlinkobj.GetLinkValidFlag(db_conn, &k, v, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, LinkGetOperStatus_DBErr) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  uint8_t operstat  =  0;

  OdbcmConnectionHandler *db_conn  =  NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret  =  ktlinkobj.GetOperStatus(db_conn,
     (uint32_t)UNC_DT_STATE, &k, operstat);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(LinkTest, LinkGetOperStatus_success) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  uint8_t operstat  =  0;

  OdbcmConnectionHandler *db_conn  =  NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret  =  ktlinkobj.GetOperStatus(db_conn,
     (uint32_t)UNC_DT_STATE, &k, operstat);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, LinkReadInternal_Create) {
  key_link_t k;
  vector<void *> vecVal_link;
  vector<void *> veckey_link;
  memset(&k, 0, sizeof(k));
  veckey_link.push_back(&k);

  Kt_Link ktlinkobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktlinkobj.ReadInternal(
    db_conn, veckey_link, vecVal_link, UNC_DT_STATE, UNC_OP_CREATE);
  // int ret = ktlinkobj.ReadInternal(
  //  db_conn, veckey_link, vecVal_link, UNC_DT_STATE, UNC_OP_READ);
  EXPECT_EQ(2008, ret);
}

TEST_F(LinkTest, LinkReadInternal_Read) {
  key_link_t k;
  vector<void *> vecVal_link;
  vector<void *> veckey_link;
  memset(&k, 0, sizeof(k));
  veckey_link.push_back(&k);

  Kt_Link ktlinkobj;
  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktlinkobj.ReadInternal(
    db_conn, veckey_link, vecVal_link, UNC_DT_STATE, UNC_OP_READ);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(LinkTest, LinkReadInternal_Val_empty) {
  key_link_t k;
  vector<void *> vecVal_link;
  vector<void *> veckey_link;
  memset(&k, 0, sizeof(k));
  veckey_link.push_back(&k);

  Kt_Link ktlinkobj;
  vecVal_link.clear();

  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktlinkobj.ReadInternal(
    db_conn, veckey_link, vecVal_link, UNC_DT_STATE, UNC_OP_READ);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(LinkTest, LinkReadInternal_Val_nonempty) {
  key_link_t k;
  vector<void *> vecVal_link;
  vector<void *> veckey_link;
  memset(&k, 0, sizeof(k));
  veckey_link.push_back(&k);

  Kt_Link ktlinkobj;
  val_link_st_t v;
  memset(&v, 0, sizeof(v));
  vecVal_link.push_back(&v);
  getValForKtLink1(v);

  OdbcmConnectionHandler *db_conn  = NULL;
  int ret = ktlinkobj.ReadInternal(
    db_conn, veckey_link, veckey_link, UNC_DT_STATE, UNC_OP_READ);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

TEST_F(LinkTest, PerformSemanticValidation_Import) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret = ktlinkobj.PerformSemanticValidation(
    db_conn, &k, &v, operation, UNC_DT_IMPORT);
  EXPECT_EQ(UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST, ret);
}

TEST_F(LinkTest, ReadLinkValFromDB_Success) {
  key_link_t k;

  void *key_struct(NULL);
  void *void_val_struct  =  NULL;

  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  uint32_t data_type  =  (uint32_t)UNC_DT_IMPORT;
  uint32_t operation_type  =  (uint32_t) UNC_OP_CREATE;
  uint32_t max_rep_ct  =  1;
  uint32_t option  =  0;

  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  UncRespCode read_status  =  ktlinkobj.ReadLinkValFromDB(db_conn, key_struct,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option);

  EXPECT_EQ(UNC_RC_SUCCESS, read_status);
}

TEST_F(LinkTest, ReadLinkValFromDB_Success_Read) {
  key_link_t k;
  void *void_val_struct  =  NULL;

  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  uint32_t data_type  =  (uint32_t)UNC_DT_IMPORT;
  uint32_t operation_type  =  (uint32_t) UNC_OP_READ;
  uint32_t max_rep_ct  =  1;
  uint32_t option  =  0;

  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  UncRespCode read_status  =  ktlinkobj.ReadLinkValFromDB(db_conn, &k,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option);

  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, read_status);
}

TEST_F(LinkTest, ReadLinkValFromDB_Success_option1_sibling) {
  key_link_t k;
  void *void_val_struct  =  NULL;

  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  uint32_t data_type  =  (uint32_t)UNC_DT_IMPORT;
  uint32_t operation_type  =  (uint32_t) UNC_OP_READ_SIBLING_BEGIN;
  uint32_t max_rep_ct  =  1;
  uint32_t option  =  0;

  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  UncRespCode read_status  =  ktlinkobj.ReadLinkValFromDB(db_conn, &k,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option);

  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, read_status);
}

TEST_F(LinkTest, HandleOperStatus_Success) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.HandleOperStatus(db_conn, UNC_DT_STATE, &k, &v);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LinkTest, ReadBulk_ADDTOBUFFER_maxrepCT0) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  uint32_t max_rep_ct  =  0;
  int child_index  =  1;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = true;
  OdbcmConnectionHandler *db_conn  =  NULL;
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret  =  ktlinkobj.ReadBulk(
    db_conn, &k, UNC_DT_STATE, max_rep_ct,
    child_index, parent_call, is_read_next,
                               &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, ReadBulk_ADDTOBUFFER_maxrepCT1) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  uint32_t max_rep_ct  =  1;
  int child_index  =  1;
  pfc_bool_t parent_call = true;
  pfc_bool_t is_read_next = true;
  OdbcmConnectionHandler *db_conn  =  NULL;
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret  =  ktlinkobj.ReadBulk(
    db_conn, &k, UNC_DT_STATE, max_rep_ct,
    child_index, parent_call, is_read_next,
                               &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, ReadBulk_readreq) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  uint32_t max_rep_ct  =  1;
  int child_index  =  1;
  pfc_bool_t parent_call = false;
  pfc_bool_t is_read_next = true;
  OdbcmConnectionHandler *db_conn  =  NULL;
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret  =  ktlinkobj.ReadBulk(
    db_conn, &k, UNC_DT_STATE, max_rep_ct,
    child_index, parent_call, is_read_next,
                               &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LinkTest, ReadBulk_DBErr) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  uint32_t max_rep_ct  =  1;
  int child_index  =  1;
  pfc_bool_t parent_call = false;
  pfc_bool_t is_read_next = true;
  OdbcmConnectionHandler *db_conn  =  NULL;
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret  =  ktlinkobj.ReadBulk(
    db_conn, &k, UNC_DT_STATE, max_rep_ct,
    child_index, parent_call, is_read_next,
                               &read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LinkTest, ReadBulkInternal_RecordNotFound) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);
  uint32_t max_rep_ct  =  1;
  OdbcmConnectionHandler *db_conn  =  NULL;

  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_key;

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret  =  ktlinkobj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_val_link_st, vect_link_key);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LinkTest, LinkPerformRead_DBErr) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type  =  UNC_KT_LINK;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_LINK);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);

  int ret = ktlinkobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, &v,
    (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ, sess,
    (uint32_t)UNC_OPT1_DETAIL, (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

TEST_F(LinkTest, ReadLinkValFromDB_Success_Update) {
  key_link_t k;

  void *key_struct(NULL);
  void *void_val_struct  =  NULL;

  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  uint32_t data_type  =  (uint32_t)UNC_DT_IMPORT;
  uint32_t operation_type  =  (uint32_t) UNC_OP_UPDATE;
  uint32_t max_rep_ct  =  1;
  uint32_t option  =  0;

  OdbcmConnectionHandler *db_conn  = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  UncRespCode read_status  =  ktlinkobj.ReadLinkValFromDB(db_conn, key_struct,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option);

  EXPECT_EQ(UNC_RC_SUCCESS, read_status);
}

#if 0

TEST_F(LinkTest, ReadLinkValFromDB_Success_option1_sibling) {
  key_link_t k;
  val_link_st v;

  void *key_struct;
  void *void_val_struct  =  NULL;

  pfc_bool_t bflag  = true;

  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k, sw_vect_key_value);
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  uint32_t data_type  =  (uint32_t)UNC_DT_IMPORT;
  uint32_t operation_type  =  (uint32_t) UNC_OP_READ_SIBLING_BEGIN;
  uint32_t max_rep_ct  =  1;
  uint32_t option  =  (uint32_t)UNC_OPT1_NORMAL;
  uint32_t option2  =  (uint32_t)UNC_OPT2_MATCH_SWITCH2;

  OdbcmConnectionHandler *db_conn  = NULL;
  uint32_t operation  =  UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  UncRespCode read_status  =  ktlinkobj.ReadLinkValFromDB(db_conn, key_struct,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option2);

  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, read_status);
}

#endif
