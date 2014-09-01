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
#include <itc_read_request.hh>
#include <itc_kt_logical_member_port.hh>
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
#include "PhysicalLayerStub.hh"
#include "ut_util.hh"

using namespace pfc;
using namespace pfc::core;
using namespace pfc::core::ipc;
using namespace std;
using namespace unc::uppl::test;

class LogicalMemberPortTest
  : public UpplTestEnv {
};

static void getKeyForKtLogicalMemberPort1(key_logical_member_port_t& k) {
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, "{0x10, 0xbc}", strlen("{0x10, 0xbc}"));
  memset(k.physical_port_id, '\0', 32);
  memcpy(k.physical_port_id, "controller1 pord id:1",
         strlen("controller1 pord id:1"));
  memset(k.logical_port_key.port_id, '\0', 320);
  memcpy(k.logical_port_key.port_id, "{0x00, 0xa}", strlen("{0x00, 0xa}"));
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.domain_name, "controller1 domain_name",
         strlen("controller1 domain_name"));
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.ctr_key.controller_name, "controller1" ,
         strlen("controller1"));
}

static void getKeyForKtLogicalMemberPort2(key_logical_member_port_t& k) {
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, "{0x11, 0xac}", strlen("{0x11, 0xac}"));
  memset(k.physical_port_id, '\0', 32);
  memcpy(k.physical_port_id, "controller3 pord id:1",
         strlen("controller3 pord id:1"));
  memset(k.logical_port_key.port_id, '\0', 291);
  memcpy(k.logical_port_key.port_id, "{0xab, 0xc}", strlen("{0xab, 0xc}"));
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.domain_name, "controller3_domain_name" ,
         strlen("controller3_domain_name"));
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.ctr_key.controller_name, "controller3" ,
         strlen("controller3"));
}

TEST_F(LogicalMemberPortTest,
     IsKeyExists_01) {
  string controller_name = "Controller1";
  string domain_name = "Domain1";
  string port_id = "log_port1";
  string switch_id = "switch1";
  string physical_port_id = "phy_port1";
  vector<string> key_values;
  key_values.push_back(controller_name);
  key_values.push_back(domain_name);
  key_values.push_back(port_id);
  key_values.push_back(switch_id);
  key_values.push_back(physical_port_id);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  lmp_obj.IsKeyExists(db_conn, UNC_DT_STATE, key_values);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LogicalMemberPortTest,
     IsKeyExists_02) {
  string controller_name = "Controller1";
  string domain_name = "Domain1";
  string port_id = "log_port1";
  string switch_id = "switch1";
  string physical_port_id = "phy_port1";
  vector<string> key_values;
  key_values.push_back(controller_name);
  key_values.push_back(domain_name);
  key_values.push_back(port_id);
  key_values.push_back(switch_id);
  key_values.push_back(physical_port_id);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  lmp_obj.IsKeyExists(db_conn, UNC_DT_STATE, key_values);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LogicalMemberPortTest,
     IsKeyExists_03) {
  string controller_name = "Controller1";
  string domain_name = "Domain1";
  string port_id = "log_port1";
  string switch_id = "switch1";
  string physical_port_id = "phy_port1";
  vector<string> key_values;
  key_values.push_back(controller_name);
  key_values.push_back(domain_name);
  key_values.push_back(port_id);
  key_values.push_back(switch_id);
  key_values.push_back(physical_port_id);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  lmp_obj.IsKeyExists(db_conn, UNC_DT_STATE, key_values);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LogicalMemberPortTest,
     IsKeyExists_04) {
  vector<string> key_values;
  key_values.clear();
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  lmp_obj.IsKeyExists(db_conn, UNC_DT_STATE, key_values);
  EXPECT_EQ(UNC_UPPL_RC_ERR_BAD_REQUEST, ret);
}

//  PerformSyntaxValidation with value structure
TEST_F(LogicalMemberPortTest,
     PerformSyntaxValidation_01) {
  key_logical_member_port k;
  getKeyForKtLogicalMemberPort2(k);
  key_logical_member_port val;
  memset(&val, 0, sizeof(val));
  uint32_t operation = UNC_OP_READ;
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  lmp_obj.PerformSyntaxValidation(
    db_conn, &k, &val, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

//  PerformSyntaxValidation success
TEST_F(LogicalMemberPortTest,
     PerformSyntaxValidation_02) {
  key_logical_member_port k;
  getKeyForKtLogicalMemberPort2(k);
  void* val = NULL;
  uint32_t operation = UNC_OP_READ;
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  lmp_obj.PerformSyntaxValidation(
    db_conn, &k, val, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  PerformSyntaxValidation without switch id
TEST_F(LogicalMemberPortTest,
     PerformSyntaxValidation_03) {
  key_logical_member_port k;
  memset(k.switch_id, '\0', 256);
  memset(k.physical_port_id, '\0', 32);
  memcpy(k.physical_port_id, "phy_port1", strlen("phy_port1"));
  memset(k.logical_port_key.port_id, '\0', 291);
  memcpy(k.logical_port_key.port_id, "log_port1", strlen("log_port1"));
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.domain_name, "Domain1" ,
         strlen("Domain1"));
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.ctr_key.controller_name, "controller4" ,
         strlen("controller4"));
  void* val = NULL;
  uint32_t operation = UNC_OP_READ;
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  lmp_obj.PerformSyntaxValidation(
    db_conn, &k, val, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
//  PerformSyntaxValidation without physical port id
TEST_F(LogicalMemberPortTest,
     PerformSyntaxValidation_04) {
  key_logical_member_port k;
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, "switch1", strlen("switch1"));
  memset(k.physical_port_id, '\0', 32);
  memset(k.logical_port_key.port_id, '\0', 320);
  memcpy(k.logical_port_key.port_id, "log_port1", strlen("log_port1"));
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.domain_name, "Domain1" ,
         strlen("Domain1"));
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.ctr_key.controller_name, "controller4" ,
         strlen("controller4"));
  void* val = NULL;
  uint32_t operation = UNC_OP_READ;
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  lmp_obj.PerformSyntaxValidation(
    db_conn, &k, val, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
//  PerformSyntaxValidation without logical_port_id
TEST_F(LogicalMemberPortTest,
     PerformSyntaxValidation_05) {
  key_logical_member_port k;
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, "switch1", strlen("switch1"));
  memset(k.physical_port_id, '\0', 32);
  memcpy(k.physical_port_id, "phy_port1", strlen("phy_port1"));
  memset(k.logical_port_key.port_id, '\0', 320);
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.domain_name, "Domain1" ,
         strlen("Domain1"));
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.ctr_key.controller_name, "controller4" ,
         strlen("controller4"));
  void* val = NULL;
  uint32_t operation = UNC_OP_READ;
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  lmp_obj.PerformSyntaxValidation(
    db_conn, &k, val, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
//  PerformSyntaxValidation without domain name
TEST_F(LogicalMemberPortTest,
     PerformSyntaxValidation_06) {
  key_logical_member_port k;
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, "switch1", strlen("switch1"));
  memset(k.physical_port_id, '\0', 32);
  memcpy(k.physical_port_id, "phy_port1", strlen("phy_port1"));
  memset(k.logical_port_key.port_id, '\0', 320);
  memcpy(k.logical_port_key.port_id, "log_port1", strlen("log_port1"));
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.ctr_key.controller_name, "controller4" ,
         strlen("controller4"));
  void* val = NULL;
  uint32_t operation = UNC_OP_READ;
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  lmp_obj.PerformSyntaxValidation(
    db_conn, &k, val, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}
//  PerformSyntaxValidation without controller name
TEST_F(LogicalMemberPortTest,
     PerformSyntaxValidation_07) {
  key_logical_member_port k;
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, "switch1", strlen("switch1"));
  memset(k.physical_port_id, '\0', 32);
  memcpy(k.physical_port_id, "phy_port1", strlen("phy_port1"));
  memset(k.logical_port_key.port_id, '\0', 291);
  memcpy(k.logical_port_key.port_id, "log_port1", strlen("log_port1"));
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.domain_name, "Domain1" ,
         strlen("Domain1"));
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  void* val = NULL;
  uint32_t operation = UNC_OP_READ;
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  lmp_obj.PerformSyntaxValidation(
    db_conn, &k, val, operation, UNC_DT_STATE);
  EXPECT_EQ(UNC_UPPL_RC_ERR_CFG_SYNTAX, ret);
}

//  DeleteKeyInstance success
TEST_F(LogicalMemberPortTest,
     DeleteKeyInstance_01) {
  key_logical_member_port k;
  uint32_t key_type = 1;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret =  lmp_obj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LogicalMemberPortTest,
     DeleteKeyInstance_02) {
  key_logical_member_port k;
  uint32_t key_type = 1;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret =  lmp_obj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LogicalMemberPortTest,
     DeleteKeyInstance_03) {
  key_logical_member_port k;
  uint32_t key_type = 1;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  lmp_obj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LogicalMemberPortTest,
     DeleteKeyInstance_04) {
  key_logical_member_port k;
  uint32_t key_type = 1;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_FAILED);
  int ret =  lmp_obj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_DELETE, ret);
}

TEST_F(LogicalMemberPortTest,
     DeleteKeyInstance_05) {
  key_logical_member_port k;
  uint32_t key_type = 1;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_FAILED);
  int ret =  lmp_obj.DeleteKeyInstance(db_conn, &k, UNC_DT_CANDIDATE, key_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

//  DeleteKeyInstance without switch id
TEST_F(LogicalMemberPortTest,
     DeleteKeyInstance_06) {
  key_logical_member_port k;
  uint32_t key_type = 1;
  memset(k.switch_id, '\0', 256);
  memset(k.physical_port_id, '\0', 32);
  memcpy(k.physical_port_id, "phy_port1", strlen("phy_port1"));
  memset(k.logical_port_key.port_id, '\0', 291);
  memcpy(k.logical_port_key.port_id, "log_port1", strlen("log_port1"));
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.domain_name, "Domain1" ,
         strlen("Domain1"));
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.ctr_key.controller_name, "controller4" ,
         strlen("controller4"));
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret =  lmp_obj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
//  DeleteKeyInstance without physical port id
TEST_F(LogicalMemberPortTest,
     DeleteKeyInstance_07) {
  key_logical_member_port k;
  uint32_t key_type = 1;
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, "switch1", strlen("switch1"));
  memset(k.physical_port_id, '\0', 32);
  memset(k.logical_port_key.port_id, '\0', 320);
  memcpy(k.logical_port_key.port_id, "log_port1", strlen("log_port1"));
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.domain_name, "Domain1" ,
         strlen("Domain1"));
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.ctr_key.controller_name, "controller4" ,
         strlen("controller4"));
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret =  lmp_obj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
//  DeleteKeyInstance without logical_port_id
TEST_F(LogicalMemberPortTest,
     DeleteKeyInstance_08) {
  key_logical_member_port k;
  uint32_t key_type = 1;
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, "switch1", strlen("switch1"));
  memset(k.physical_port_id, '\0', 32);
  memcpy(k.physical_port_id, "phy_port1", strlen("phy_port1"));
  memset(k.logical_port_key.port_id, '\0', 320);
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.domain_name, "Domain1" ,
         strlen("Domain1"));
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.ctr_key.controller_name, "controller4" ,
         strlen("controller4"));
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret =  lmp_obj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
//  DeleteKeyInstance without domain name
TEST_F(LogicalMemberPortTest,
     DeleteKeyInstance_09) {
  key_logical_member_port k;
  uint32_t key_type = 1;
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, "switch1", strlen("switch1"));
  memset(k.physical_port_id, '\0', 32);
  memcpy(k.physical_port_id, "phy_port1", strlen("phy_port1"));
  memset(k.logical_port_key.port_id, '\0', 320);
  memcpy(k.logical_port_key.port_id, "log_port1", strlen("log_port1"));
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.ctr_key.controller_name, "controller4" ,
         strlen("controller4"));
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret =  lmp_obj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
//  DeleteKeyInstance without controller name
TEST_F(LogicalMemberPortTest,
     DeleteKeyInstance_10) {
  key_logical_member_port k;
  uint32_t key_type = 1;
  memset(k.switch_id, '\0', 256);
  memcpy(k.switch_id, "switch1", strlen("switch1"));
  memset(k.physical_port_id, '\0', 32);
  memcpy(k.physical_port_id, "phy_port1", strlen("phy_port1"));
  memset(k.logical_port_key.port_id, '\0', 291);
  memcpy(k.logical_port_key.port_id, "log_port1", strlen("log_port1"));
  memset(k.logical_port_key.domain_key.domain_name, '\0', 32);
  memcpy(k.logical_port_key.domain_key.domain_name, "Domain1" ,
         strlen("Domain1"));
  memset(k.logical_port_key.domain_key.ctr_key.controller_name, '\0', 32);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret =  lmp_obj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, key_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

//  ReadBulkInternal success
TEST_F(LogicalMemberPortTest,
     ReadBulkInternal_01) {
  key_logical_member_port k;
  uint32_t max_rep_ct = 2;
  vector<key_logical_member_port_t> vect_logical_mem_port;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  lmp_obj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_logical_mem_port);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LogicalMemberPortTest,
     ReadBulkInternal_02) {
  key_logical_member_port k;
  uint32_t max_rep_ct = 2;
  vector<key_logical_member_port_t> vect_logical_mem_port;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret =  lmp_obj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_logical_mem_port);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LogicalMemberPortTest,
     ReadBulkInternal_03) {
  key_logical_member_port k;
  uint32_t max_rep_ct = 2;
  vector<key_logical_member_port_t> vect_logical_mem_port;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret =  lmp_obj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_logical_mem_port);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

TEST_F(LogicalMemberPortTest,
     ReadBulkInternal_04) {
  key_logical_member_port k;
  uint32_t max_rep_ct = 2;
  vector<key_logical_member_port_t> vect_logical_mem_port;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  lmp_obj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_logical_mem_port);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

TEST_F(LogicalMemberPortTest,
     ReadBulkInternal_05) {
  key_logical_member_port k;
  uint32_t max_rep_ct = 0;
  vector<key_logical_member_port_t> vect_logical_mem_port;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  lmp_obj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_logical_mem_port);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LogicalMemberPortTest,
     ReadBulkInternal_06) {
  key_logical_member_port k;
  uint32_t max_rep_ct = 2;
  vector<key_logical_member_port_t> vect_logical_mem_port;
  getKeyForKtLogicalMemberPort1(k);
  Kt_LogicalMemberPort lmp_obj;
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_FAILED);
  int ret =  lmp_obj.ReadBulkInternal(
    db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_logical_mem_port);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/*ReadInternal function when operation is lessthan READ */
TEST_F(LogicalMemberPortTest,
     ReadInternal_No_Read_01) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(k));

  vector<void *> vectVal_logicalmemberport;
  vector<void *> vectkey_logicalmemberport;
  vectkey_logicalmemberport.push_back(&k);
  Kt_LogicalMemberPort ktlmportobj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlmportobj.ReadInternal(db_conn, vectkey_logicalmemberport,
    vectVal_logicalmemberport, UNC_DT_STATE, UNC_OP_CREATE);
  EXPECT_EQ(2008, ret);
}

/*ReadInternal function where value struct is NULL */
TEST_F(LogicalMemberPortTest,
     ReadInternal_VAlStructNull_02) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(k));

  vector<void *> vectVal_logicalmemberport;
  vector<void *> vectkey_logicalmemberport;
  vectkey_logicalmemberport.push_back(&k);
  Kt_LogicalMemberPort ktlmportobj;
  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlmportobj.ReadInternal(db_conn, vectkey_logicalmemberport,
    vectVal_logicalmemberport, UNC_DT_STATE, UNC_OP_READ);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

/*PerformRead function which has invalid option1 */
TEST_F(LogicalMemberPortTest,
     PerformRead_01) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort ktlmportobj;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);

  int ret =  ktlmportobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, NULL,
    (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ, sess,
    (uint32_t)UNC_OPT1_DETAIL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*PerformRead function which has invalid option1 */
TEST_F(LogicalMemberPortTest,
     PerformRead_02) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort ktlmportobj;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);

  int ret =  ktlmportobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, NULL,
    (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ, sess,
    (uint32_t)UNC_OPT1_DETAIL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PerformRead function which has invalid datatype */
TEST_F(LogicalMemberPortTest,
     PerformRead_03) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort ktlmportobj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_DETAIL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);

  int ret =  ktlmportobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, NULL, (uint32_t)UNC_DT_CANDIDATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*PerformRead function which has valid option2 */
TEST_F(LogicalMemberPortTest,
     PerformRead_04) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort ktlmportobj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);

  int ret =  ktlmportobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, NULL, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*PerformRead function which has invalid datatype */
TEST_F(LogicalMemberPortTest,
     PerformRead_05) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort ktlmportobj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_CANDIDATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);

  int ret =  ktlmportobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, NULL, (uint32_t)UNC_DT_CANDIDATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PerformRead function which has invalid option2 */
TEST_F(LogicalMemberPortTest,
     PerformRead_06) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort ktlmportobj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);

  int ret =  ktlmportobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, NULL,
    (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ, sess,
    (uint32_t)UNC_OPT1_NORMAL, (uint32_t)UNC_OPT2_MAC_ENTRY, (uint32_t)1);
  EXPECT_EQ(UNC_UPPL_RC_ERR_IPC_WRITE_ERROR, ret);
}

/*PerformRead function which has invalid option2 */
TEST_F(LogicalMemberPortTest,
     PerformRead_07) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort ktlmportobj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_MAC_ENTRY);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_INVALID_OPTION2);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);

  int ret =  ktlmportobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, NULL,
    (uint32_t)UNC_DT_STATE, (uint32_t)UNC_OP_READ, sess,
    (uint32_t)UNC_OPT1_NORMAL, (uint32_t)UNC_OPT2_MAC_ENTRY, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PerformRead function with valid option1 and option2 */
TEST_F(LogicalMemberPortTest,
     PerformRead_08) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort ktlmportobj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_RC_SUCCESS);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);

  int ret =  ktlmportobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, NULL, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/*PerformRead function with valid option1 and option2 */
TEST_F(LogicalMemberPortTest,
     PerformRead_09) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort ktlmportobj;
  OdbcmConnectionHandler *db_conn =NULL;
  ServerSession sess;
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)0);
  sess.stub_setAddOutput((uint32_t)UNC_OP_READ);
  sess.stub_setAddOutput((uint32_t)1);
  sess.stub_setAddOutput((uint32_t)UNC_OPT1_NORMAL);
  sess.stub_setAddOutput((uint32_t)UNC_OPT2_NONE);
  sess.stub_setAddOutput((uint32_t)UNC_DT_STATE);
  sess.stub_setAddOutput((uint32_t)UNC_UPPL_RC_ERR_DB_GET);
  sess.stub_setAddOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);

  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktlmportobj.PerformRead(
    db_conn, (uint32_t)0, (uint32_t)0, &k, NULL, (uint32_t)UNC_DT_STATE,
    (uint32_t)UNC_OP_READ, sess, (uint32_t)UNC_OPT1_NORMAL,
    (uint32_t)UNC_OPT2_NONE, (uint32_t)1);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// DB Access failure for UPDATE opr

TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_UPD_DBFail) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// DB Access failure for DEL opr
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_DEL_DBFail) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_DELETE;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// DB Access failure for READ opr
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_READ_DBFail) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_READ;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// key does not exist.
// update operation not allowed
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_KeyNotExists_For_UPD) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_FAILED);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

// key does not exist.
// Delete operation not allowed
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_KeyNotExists_For_DEL) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_DELETE;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_FAILED);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

// key does not exist.
// Read operation not allowed
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_KeyNotExists_For_READ) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_READ;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_FAILED);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

// key instance exist.
// update operation allowed
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_KeyExists_UPD_Allow) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_UPDATE;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// key instance exist.
// Delete operation allowed
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_KeyExists_Del_Allow) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_DELETE;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// key instance exist.
// Read operation allowed
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_KeyExists_Read_Allow) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_READ;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// key already exist
// create operation not allowed
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_KeyExist_CREATE_NotAllow) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_CREATE;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_INSTANCE_EXISTS, ret);
}

// IsRowExits:DB Access failure
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_KeyExist_CREATE_DB_Error) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_CREATE;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// Error:ParentKey does not exist
TEST_F(LogicalMemberPortTest,
     PerformSemanticValid_KeyExist_CREATE_ParentKeyNotExist) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  void *v = NULL;
  uint32_t operation = UNC_OP_CREATE;
  uint32_t data_type = UNC_DT_STATE;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_SUCCESS);
  int ret = KtObj.PerformSemanticValidation(
    db_conn, &k, v, operation, data_type);
  EXPECT_EQ(UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST, ret);
}

/********ReadLogicalMemberPortValFromDB*********/
// Unsupported operation type returns Success
TEST_F(LogicalMemberPortTest,
     ReadLogicalMemberPortValFromDB_UnSupport_Opr) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t operation = UNC_OP_CREATE;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t max_rep_ct;
  vector<key_logical_member_port_t> logical_mem_port;
  int ret = KtObj.ReadLogicalMemberPortValFromDB(db_conn, &k, data_type,
    operation, max_rep_ct, logical_mem_port);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// Read Error:No record found
TEST_F(LogicalMemberPortTest,
     ReadLogicalMemberPortValFromDB_READ_NoRecordFound) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t operation = UNC_OP_READ;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t max_rep_ct;
  vector<key_logical_member_port_t> logical_mem_port;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtObj.ReadLogicalMemberPortValFromDB(db_conn, &k, data_type,
    operation, max_rep_ct, logical_mem_port);
  EXPECT_EQ(UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE, ret);
}

// Read :DB Connxn Error
TEST_F(LogicalMemberPortTest,
     ReadLogicalMemberPortValFromDB_READ_Db_Connxn_Error) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t operation = UNC_OP_READ;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t max_rep_ct;
  vector<key_logical_member_port_t> logical_mem_port;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtObj.ReadLogicalMemberPortValFromDB(db_conn, &k, data_type,
    operation, max_rep_ct, logical_mem_port);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// Read :Error returning DB_GET
TEST_F(LogicalMemberPortTest,
     ReadLogicalMemberPortValFromDB_READ_Db_GetError) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t operation = UNC_OP_READ;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t max_rep_ct;
  vector<key_logical_member_port_t> logical_mem_port;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = KtObj.ReadLogicalMemberPortValFromDB(db_conn, &k, data_type,
    operation, max_rep_ct, logical_mem_port);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_GET, ret);
}

// Read:GetOneRow Success
TEST_F(LogicalMemberPortTest,
     ReadLogicalMemberPortValFromDB_READ_GetOneRow_Success) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(key_logical_member_port_t));
  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t operation = UNC_OP_READ;
  uint32_t data_type = UNC_DT_STATE;
  uint32_t max_rep_ct;
  vector<key_logical_member_port_t> logical_mem_port;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = KtObj.ReadLogicalMemberPortValFromDB(db_conn, &k, data_type,
    operation, max_rep_ct, logical_mem_port);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

/******ReadBulk**********/
// ReadBulk Not allowed for UNC_DT_CANDIDATE
TEST_F(LogicalMemberPortTest,
     ReadBulk_DB_NoMatch) {
  key_logical_member_port_t k;
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t max_rep_ct(1);
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest *read_req = NULL;
  int ret = KtObj.ReadBulk(db_conn, &k, UNC_DT_RUNNING,
    max_rep_ct, child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED, ret);
}

// If max_rep_count is 0, return success
TEST_F(LogicalMemberPortTest,
     ReadBulk_MaxCt_Zero) {
  key_logical_member_port_t k;
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t max_rep_ct = 0;
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest *read_req = NULL;
  int ret = KtObj.ReadBulk(db_conn, &k, UNC_DT_STATE,
    max_rep_ct, child_index, parent_call, is_read_next, read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// GetBulkRows Read Status Success
TEST_F(LogicalMemberPortTest,
     ReadBulk__Success) {
  key_logical_member_port_t k;
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t max_rep_ct = 1;
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index,
                           parent_call, is_read_next, &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// GetBulkRows Returns DB Access Error
TEST_F(LogicalMemberPortTest,
     ReadBulk_Db_Access_Error) {
  key_logical_member_port_t k;
  Kt_LogicalMemberPort KtObj;
  getKeyForKtLogicalMemberPort1(k);
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t max_rep_ct = 1;
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = KtObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index,
                           parent_call, is_read_next, &read_req);
  EXPECT_EQ(UNC_UPPL_RC_ERR_DB_ACCESS, ret);
}

// Read Status for next Kin Success
TEST_F(LogicalMemberPortTest,
     ReadBulk_Next_Kin_Success) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(k));
  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t max_rep_ct = 1;
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = KtObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index,
                           parent_call, is_read_next, &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

// Read Status Success for no such instance
TEST_F(LogicalMemberPortTest,
     ReadBulk_Succes__For_NoInstance) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(k));

  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t max_rep_ct = 1;
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index,
                           parent_call, is_read_next, &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}

TEST_F(LogicalMemberPortTest,
     ReadBulk_Succes_NOSUCHINSTANCE) {
  key_logical_member_port_t k;
  memset(&k, 0, sizeof(k));

  Kt_LogicalMemberPort KtObj;
  OdbcmConnectionHandler *db_conn = NULL;
  uint32_t max_rep_ct = 1;
  int child_index(0);
  pfc_bool_t parent_call(PFC_FALSE);
  pfc_bool_t is_read_next(PFC_FALSE);
  ReadRequest read_req;
  unc::uppl::ODBCManager::stub_setResultcode(
    unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = KtObj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index,
                           parent_call, is_read_next, &read_req);
  EXPECT_EQ(UNC_RC_SUCCESS, ret);
}
