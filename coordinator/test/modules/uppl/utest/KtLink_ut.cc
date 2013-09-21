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
using namespace std;

ClientSession *cli_sess = NULL;
ClientSession *cli_sess_notify = NULL;
pfc_ipcid_t service = UPPL_SVC_CONFIGREQ;

class KtClassTest : public testing::Test {
  protected:

   virtual void SetUp() {
    if(cli_sess == NULL) {
      pfc_ipcconn_t connp = 0;
      int err = pfc_ipcclnt_altopen(UPPL_IPC_CHN_NAME, &connp);
      //ASSERT_EQ(0, err);
      cli_sess = new ClientSession(connp, UPPL_IPC_SVC_NAME, service, err);
      //ASSERT_EQ(0, err);
      connp = 0;
      err = pfc_ipcclnt_altopen("phynwd", &connp);
      ASSERT_EQ(0, err); cli_sess = new ClientSession(connp, UPPL_IPC_SVC_NAME, service, err);
     // ASSERT_EQ(0, err);
      connp = 0;
      err = pfc_ipcclnt_altopen("phynwd", &connp);
      //ASSERT_EQ(0, err);
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
// Can be changed based on testing need
char pkName1_ctr[] = "controller1";
char pkName1_switchid1[] = "{0x10,0xbc}";
char pkName1_portid1[] = "controller1 port";
char pkName1_switchid2[] = "{0x11,0xab}";
char pkName1_portid2[] = "controller1 port 4";
char pkName2_ctr[] = "controller2";
char pkName2_switchid1[] = "{0x12,0xdb}";
char pkName2_portid1[] = "controller2 port 3";
char pkName2_switchid2[] = "{0x21,0xcb}";
char pkName2_portid2[] = "controller2 port 5";
char pkName3_ctr[] = "controller3";
char pkName3_switchid1[] = "{0x23,0xab}";
char pkName3_portid1[] = "controller3 port";
char pkName3_switchid2[] = "{0x11,0xcb}";
char pkName3_portid2[] = "controller3 port 5";
char pkName4_ctr[] = "";
char pkName4_switchid1[] = "";
char pkName4_portid1[] = "";
char pkName4_switchid2[] = "";
char pkName4_portid2[] = "";
char pkName5_ctr[] = "NotExisting";
char pkName5_switchid1[] = "NotExisting";
char pkName5_portid1[] = "NotExisting";
char pkName5_switchid2[] = "NotExisting";
char pkName5_portid2[] = "NotExisting";

void getKeyForKtLink1(key_link_t& k,std::vector<string>& sw_vect_key_value) {
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

void getKeyForKtLink1(key_link_t& k) {
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



void getKeyForKtLink2(key_link_t& k) {
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName2_ctr, strlen(pkName2_ctr));
  memset(k.switch_id1, '\0', 256);
  memcpy(k.switch_id1, pkName2_switchid1, strlen(pkName2_switchid1));
  memset(k.port_id1, '\0', 32);
  memcpy(k.port_id1, pkName2_portid1, strlen(pkName2_portid1));
  memset(k.switch_id2, '\0', 256);
  memcpy(k.switch_id2, pkName2_switchid2, strlen(pkName2_switchid2));
  memset(k.port_id2, '\0', 32);
  memcpy(k.port_id2, pkName2_portid2, strlen(pkName2_portid2));
}

void getKeyForKtLink3(key_link_t& k) {
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName3_ctr, strlen(pkName3_ctr));
  memset(k.switch_id1, '\0', 256);
  memcpy(k.switch_id1, pkName3_switchid1, strlen(pkName3_switchid1));
  memset(k.port_id1, '\0', 32);
  memcpy(k.port_id1, pkName3_portid1, strlen(pkName3_portid1));
  memset(k.switch_id2, '\0', 256);
  memcpy(k.switch_id2, pkName3_switchid2, strlen(pkName3_switchid2));
  memset(k.port_id2, '\0', 32);
  memcpy(k.port_id2, pkName3_portid2, strlen(pkName3_portid2));
}

void getKeyForKtLink4(key_link_t& k) {
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

void getKeyForKtLink5(key_link_t& k) {
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName5_ctr, strlen(pkName5_ctr));
  memset(k.switch_id1, '\0', 256);
  memcpy(k.switch_id1, pkName5_switchid1, strlen(pkName5_switchid1));
  memset(k.port_id1, '\0', 32);
  memcpy(k.port_id1, pkName5_portid1, strlen(pkName5_portid1));
  memset(k.switch_id2, '\0', 256);
  memcpy(k.switch_id2, pkName5_switchid2, strlen(pkName5_switchid2));
  memset(k.port_id2, '\0', 32);
  memcpy(k.port_id2, pkName5_portid2, strlen(pkName5_portid2));
}
void getValForKtLink1(val_link_st_t& v) {
    memset(v.link.description, '\0', 128);
    memcpy(v.link.description, "linkdescription", strlen("linkdescription"));
                      // uint8_t                 description[128];
  // uint8_t oper_status
  v.oper_status = 0;
  memset(v.valid, 1, 2);  // uint8_t                 valid[2];
}

void getKeyForLinkNoKeyNotify(key_link_t& k) {
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

void getKeyForLinkNotify(key_link_t& k) {
  memset(k.ctr_key.controller_name, '\0', 32);
  memcpy(k.ctr_key.controller_name, pkName5_ctr, strlen(pkName5_ctr));
  memset(k.switch_id1, '\0', 256);
  memcpy(k.switch_id1, pkName5_switchid1, strlen(pkName5_switchid1));
  memset(k.port_id1, '\0', 32);
  memcpy(k.port_id1, pkName5_portid1, strlen(pkName5_portid1));
  memset(k.switch_id2, '\0', 256);
  memcpy(k.switch_id2, pkName5_switchid2, strlen(pkName5_switchid2));
  memset(k.port_id2, '\0', 32);
  memcpy(k.port_id2, pkName5_portid2, strlen(pkName5_portid2));
}

void getNewValForLinkNotify(val_link_st_t& v) {
  uint32_t VALID = 1;
  memcpy(v.link.description, "link description", 13);
                                 // uint8_t description[128];
  v.link.description[13] = '\0';
  v.oper_status = 1;
  memset(v.valid, VALID, 7);  // uint8_t valid[7];
  memset(v.link.valid, VALID, 5);
}

void getOldValForLinkNotify(val_link_st_t& v) {
  uint32_t VALID = 1;
  memcpy(v.link.description, "link description", 13);
                               // uint8_t description[128];
  v.link.description[13] = '\0';
  v.oper_status = 1;
  memset(v.valid, VALID, 7);  // uint8_t valid[7];
  memset(v.link.valid, VALID, 5);
}

void getInvalidValForKtLink1(val_link_st_t& v) {
  memset(v.link.description, '\0', 128);
  memcpy(v.link.description, "link description", strlen("link description"));
                             // uint8_t description[128];
  v.oper_status = 1;
  memset(v.valid, '1', 12);  // uint8_t valid[12];
}

void getNewValForLink(val_link_st& v) {
  memcpy(v.link.description, "link descr", 10);
  v.link.description[10] = '\0';
  v.oper_status = 1;
  memset(v.link.valid, VALID, 8);
}

void getOldValForLink(val_link_st_t& v) {
  memcpy(v.link.description, "link descr", 10);
  v.link.description[10] = '\0';
  v.oper_status = 2;
  memset(v.link.valid, VALID, 8);
}

int compare(val_link_st_t v1, val_link_st_t v2) {
  if (strcmp(reinterpret_cast<char *>(v1.link.description),
            reinterpret_cast<char *>(v2.link.description)))
  // return 1;
  if (v1.valid!= v2.valid)
    return 1;
  return 0;
}

void getReqHeader(physical_request_header& rh, unc_keytype_operation_t opern,
                  unc_keytype_datatype_t dtype) {
  rh.client_sess_id = 1;
  rh.config_id = 1;
  rh.operation = opern;
  rh.max_rep_count = 0;
  rh.option1 = 0;
  rh.option2 = 0;
  rh.data_type = dtype;
  rh.key_type = UNC_KT_LINK;
}

int sessOutReqHeader(ClientSession *cli_sess, physical_request_header rh) {
  int err = cli_sess->addOutput(rh.client_sess_id);
  err |= cli_sess->addOutput(rh.config_id);
  err |= cli_sess->addOutput(rh.operation);
  err |= cli_sess->addOutput(rh.max_rep_count);
  err |= cli_sess->addOutput(rh.option1);
  err |= cli_sess->addOutput(rh.option2);
  err |= cli_sess->addOutput(rh.data_type);
  err |= cli_sess->addOutput(rh.key_type);
  return err;
}


int sessGetRespHeader(ClientSession *cli_sess, physical_response_header& rsh) {
  int err = cli_sess->getResponse(0, rsh.client_sess_id);
  err |= cli_sess->getResponse(1, rsh.config_id);
  err |= cli_sess->getResponse(2, rsh.operation);
  err |= cli_sess->getResponse(3, rsh.max_rep_count);
  err |= cli_sess->getResponse(4, rsh.option1);
  err |= cli_sess->getResponse(5, rsh.option2);
  err |= cli_sess->getResponse(6, rsh.data_type);
  err |= cli_sess->getResponse(7, rsh.result_code);
  return err;
}

TEST_F(KtClassTest, PerformSyntxCheck_01) {

  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
   
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);

}
//Key Empty
TEST_F(KtClassTest, PerformSyntxCheck_02) {

  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  //getKeyForKtLink1(k);
  getReqHeader(rh, UNC_OP_READ_SIBLING_COUNT, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;
   
  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

//indfividual Attributes havin mepty values
TEST_F(KtClassTest, PerformSyntxCheck_03) {

  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForLinkNoKeyNotify(k);
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}
//Setting controller name as empty
TEST_F(KtClassTest, PerformSyntxCheck_04) {

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
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Setting switchID1 name as empty
TEST_F(KtClassTest, PerformSyntxCheck_05) {

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
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Setting switchID2 name as empty
TEST_F(KtClassTest, PerformSyntxCheck_06) {

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
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Setting portID1 name as empty
TEST_F(KtClassTest, PerformSyntxCheck_07) {

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
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}

//Setting portID2 name as empty
TEST_F(KtClassTest, PerformSyntxCheck_08) {

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
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
}
//Setting description as empty
TEST_F(KtClassTest, PerformSyntxCheck_09) {

  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);

  memset(v.link.description, '\0', 128);
  memcpy(v.link.description, "", strlen(""));
                      // uint8_t                 description[128];
  // uint8_t oper_status
  v.oper_status = 0;
  memset(v.valid, 1, 2);  // uint8_t

  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  //EXPECT_EQ(ret,UPPL_RC_ERR_CFG_SYNTAX);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

//Setting description length more than 128
TEST_F(KtClassTest, PerformSyntxCheck_10) {

  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);
 
  const char* strdes = "Alter the SV_INTERRUPT property of a signal handler. If interrupt is zero, system calls will be restarted after signal delivery kkkkkkkkkkkkkkkkkkk";
  
  memset(v.link.description, '\0', 128);
  memcpy(v.link.description,strdes , strlen(strdes));
                      // uint8_t                 description[128];
  // uint8_t oper_status
  v.oper_status = 0;
  memset(v.valid, 1, 2);  // uint8_t

  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

//Setting oper status as invalid
TEST_F(KtClassTest, PerformSyntxCheck_11) {

  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  memset(v.valid, '\0', sizeof(v.valid));
  memset(v.link.valid, '\0', sizeof(v.link.valid));
  physical_request_header rh;
  getKeyForKtLink1(k);
 
  const char* strdes = "link des";
  
  memset(v.link.description, '\0', 128);
  memcpy(v.link.description,strdes , strlen(strdes));

  v.oper_status = 4;
  memset(v.valid, 1, 2);  // uint8_t

  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  uint32_t operation = UNC_OP_CREATE;
  OdbcmConnectionHandler *db_conn =NULL;

  int ret =  ktlinkobj.PerformSyntaxValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

//key as Empty
TEST_F(KtClassTest, Link_IsKeyExists_01) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);
 
  OdbcmConnectionHandler *db_conn =NULL;
  vector<string> sw_vect_key_value;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktlinkobj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
  EXPECT_EQ(ret,UPPL_RC_ERR_BAD_REQUEST);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, Link_IsKeyExists_FailureIsrowexist02) {

  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;

  getKeyForKtLink1(k,sw_vect_key_value);
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, Link_IsKeyExists_SuccessIsrowexist03) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn =NULL;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktlinkobj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, Link_IsKeyExists_FailConnectionErr04) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;

  getKeyForKtLink1(k);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn =NULL;
  getKeyForKtLink1(k,sw_vect_key_value);

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktlinkobj.IsKeyExists(db_conn,UNC_DT_STATE,sw_vect_key_value);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_InstanceExist_create05) {
  
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;

  getKeyForKtLink1(k,sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_ROW_EXISTS);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_INSTANCE_EXISTS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_ConnectionErr_create06) {

  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;

  getKeyForKtLink1(k,sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}


TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_update07) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_UPDATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_ConnectionErr_update08) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_UPDATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_delete09) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_DELETE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_ConnectionErr_delete10) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_DELETE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_read11) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_READ;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_ConnectionErr_Read12) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_READ;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_CONNECTION_ERROR);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, PerformSemanticValidation_InstanceNOtExist_create13) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_STATE);
  EXPECT_EQ(ret,UPPL_RC_ERR_PARENT_DOES_NOT_EXIST);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, LinkDeleteKeyInstance_UnsupportedForSTARTUP_14) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_STARTUP, UNC_KT_LINK);
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}

TEST_F(KtClassTest, LinkDeleteKeyInstance_UnsupportedForSTARTUP_15) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_CANDIDATE, UNC_KT_LINK);
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}

TEST_F(KtClassTest, LinkDeleteKeyInstance_UnsupportedForSTARTUP_16) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_RUNNING, UNC_KT_LINK);
  EXPECT_EQ(ret, UPPL_RC_ERR_OPERATION_NOT_ALLOWED);
}

TEST_F(KtClassTest, LinkDeleteKeyInstance_Support_17) { 
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_LINK);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_DELETE);
}

TEST_F(KtClassTest, DeleteKeyInstance_Support_18) { 
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_LINK);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, DeleteKeyInstance_Support_19) {
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_LINK);
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, LinkDeleteKeyInstance_Support_20) { 
  key_link_t k;
  memset(&k, 0, sizeof(key_link_t));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::DELETEONEROW, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.DeleteKeyInstance(db_conn, &k, UNC_DT_STATE, UNC_KT_LINK);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, LinkPerformRead_incorrectoption1_21) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_LINK;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_LINK);

  int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

TEST_F(KtClassTest, LinkPerformRead_incorrectDT_22) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_CANDIDATE);
  rh.key_type = UNC_KT_LINK;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_LINK);
int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_L2DOMAIN,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}
TEST_F(KtClassTest, LinkPerformRead_incorrectoption2_23) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_LINK;
  OdbcmConnectionHandler *db_conn =NULL;
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
int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_NORMAL,(uint32_t)UNC_OPT2_L2DOMAIN,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

TEST_F(KtClassTest, LinkSetOperStatus_24) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplLinkOperStatus)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_UPDATE);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, LinkSetOperStatus_25) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_ROW_NOT_EXISTS);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplLinkOperStatus)0);
  EXPECT_EQ(ret,UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, LinkSetOperStatus_26) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplLinkOperStatus)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, LinkSetOperStatus_27) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getKeyForKtLink1(k);
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplLinkOperStatus)1);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, LinkSetOperStatus_28) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  physical_request_header rh;
  getKeyForKtLink1(k);
  getReqHeader(rh, UNC_OP_UPDATE, UNC_DT_STATE);
  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::UPDATEONEROW, ODBCM_RC_SUCCESS);
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.SetOperStatus(db_conn,UNC_DT_STATE,&k,(UpplLinkOperStatus)2);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, LinkReadBulk_Opnt_allow_29) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

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

TEST_F(KtClassTest, LinkReadBulk_Success_30) {
 key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, LinkReadBulk_MaxCntSuccess_31) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
}


TEST_F(KtClassTest, LinkReadBulk_DBFail_32) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, LinkReadBulk_DBSuccess) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  //EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
}

TEST_F(KtClassTest, LinkReadBulkInternal_Success_33) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;
  vector<val_link_st_t> vec_val_link;
  vector<key_link_t> vec_key_link_t;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct, vec_val_link, vec_key_link_t);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
}

TEST_F(KtClassTest, LinkReadBulkInternal_Success_34) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=0;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;

  getKeyForKtLink1(k);
 
  vector<val_link_st_t> vec_val_link;
  vector<key_link_t> vec_key_link_t;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = ktlinkobj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct, vec_val_link, vec_key_link_t);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
}

TEST_F(KtClassTest, HandleOperStatus_KeyStruct_NULL_Handdle) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn =NULL;
//  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
}


TEST_F(KtClassTest, GetLinkValidFlag_ConnError) {
  key_link_t k;
  val_link_st_t v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st_t));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.GetLinkValidFlag(db_conn, &k, v, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, GetLinkValidFlag_DBGetErr) {
  key_link_t k;
  val_link_st_t v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st_t));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_DATA_ERROR);
  int ret = ktlinkobj.GetLinkValidFlag(db_conn, &k, v, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, GetLinkValidFlag_Success) {
  key_link_t k;
  val_link_st_t v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st_t));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.GetLinkValidFlag(db_conn, &k, v, UNC_DT_STATE);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, LinkGetOperStatus_DBErr) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  uint8_t operstat = 0;

  UpplLinkOperStatus new_oper_status = UPPL_LINK_OPER_UNKNOWN;
  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_FAILED);
  int ret = ktlinkobj.GetOperStatus(db_conn,(uint32_t)UNC_DT_STATE,&k,operstat);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_GET);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, LinkGetOperStatus_success) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  uint8_t operstat = 0;

  UpplLinkOperStatus new_oper_status = UPPL_LINK_OPER_UNKNOWN;
  OdbcmConnectionHandler *db_conn = NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.GetOperStatus(db_conn,(uint32_t)UNC_DT_STATE,&k,operstat);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, LinkReadInternal_Create) {
  key_link_t *k = new key_link_t;
  //getKeyForKtLink1(k);
  vector<void *> vecVal_link;
  vector<void *> veckey_link;
  veckey_link.push_back(k);
  Kt_Link ktlinkobj;


  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.ReadInternal(db_conn,veckey_link,vecVal_link,UNC_DT_STATE,UNC_OP_CREATE);
  //int ret =  ktlinkobj.ReadInternal(db_conn,veckey_link,vecVal_link,UNC_DT_STATE,UNC_OP_READ);
  EXPECT_EQ(ret,UPPL_RC_SUCCESS);
}

TEST_F(KtClassTest, LinkReadInternal_Read) {
  key_link_t *k = new key_link_t;
  vector<void *> vecVal_link;
  vector<void *> veckey_link;
  veckey_link.push_back(k);
  Kt_Link ktlinkobj;


  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.ReadInternal(db_conn,veckey_link,vecVal_link,UNC_DT_STATE,UNC_OP_READ);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
}

TEST_F(KtClassTest, LinkReadInternal_Val_empty) {
  key_link_t *k = new key_link_t;
  vector<void *> vecVal_link;
  vector<void *> veckey_link;
  veckey_link.push_back(k);
  Kt_Link ktlinkobj;
  
  vecVal_link.clear();

  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.ReadInternal(db_conn,veckey_link,vecVal_link,UNC_DT_STATE,UNC_OP_READ);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
}
TEST_F(KtClassTest, LinkReadInternal_Val_nonempty) {
  key_link_t *k = new key_link_t;
  vector<void *> vecVal_link;
  vector<void *> veckey_link;
  veckey_link.push_back(k);
  Kt_Link ktlinkobj;
  
  val_link_st_t v;
  veckey_link[0] = &v;
  getValForKtLink1(v);
  vecVal_link.clear();

  OdbcmConnectionHandler *db_conn =NULL;
  int ret =  ktlinkobj.ReadInternal(db_conn,veckey_link,veckey_link,UNC_DT_STATE,UNC_OP_READ);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_GET);
}
TEST_F(KtClassTest, PerformSemanticValidation_Import) {
  key_link_t k;
  val_link_st v;
  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  int ret =  ktlinkobj.PerformSemanticValidation(db_conn,&k,&v,operation,UNC_DT_IMPORT);
  EXPECT_EQ(ret,UPPL_RC_ERR_PARENT_DOES_NOT_EXIST);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, ReadLinkValFromDB_Success) {
  key_link_t k;
  val_link_st v;

  void *key_struct;
  void *void_val_struct = NULL;
 
  pfc_bool_t bflag =true;

  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  uint32_t data_type = (uint32_t)UNC_DT_IMPORT;
  uint32_t operation_type = (uint32_t) UNC_OP_CREATE;
  uint32_t max_rep_ct = 1;
  uint32_t option = 0;

  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  UpplReturnCode read_status = ktlinkobj.ReadLinkValFromDB(db_conn, key_struct,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option);

  EXPECT_EQ(read_status,UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, ReadLinkValFromDB_Success_Read) {
  key_link_t k;
  val_link_st v;

  void *key_struct;
  void *void_val_struct = NULL;
 
  pfc_bool_t bflag =true;

  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  uint32_t data_type = (uint32_t)UNC_DT_IMPORT;
  uint32_t operation_type = (uint32_t) UNC_OP_READ;
  uint32_t max_rep_ct = 1;
  uint32_t option = 0;

  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  UpplReturnCode read_status = ktlinkobj.ReadLinkValFromDB(db_conn, key_struct,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option);

  EXPECT_EQ(read_status,UPPL_RC_ERR_DB_GET);
  ODBCManager::clearStubData();
}
TEST_F(KtClassTest, ReadLinkValFromDB_Success_option1_sibling) {
  key_link_t k;
  val_link_st v;

  void *key_struct;
  void *void_val_struct = NULL;
 
  pfc_bool_t bflag =true;

  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  uint32_t data_type = (uint32_t)UNC_DT_IMPORT;
  uint32_t operation_type = (uint32_t) UNC_OP_READ_SIBLING_BEGIN;
  uint32_t max_rep_ct = 1;
  uint32_t option = 0;

  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  UpplReturnCode read_status = ktlinkobj.ReadLinkValFromDB(db_conn, key_struct,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option);

  EXPECT_EQ(read_status,UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, HandleOperStatus_Success) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  OdbcmConnectionHandler *db_conn =NULL;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETONEROW, ODBCM_RC_SUCCESS);
  int ret =  ktlinkobj.HandleOperStatus(db_conn,UNC_DT_STATE,&k,&v);
  EXPECT_EQ(ret,UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulk_ADDTOBUFFER_maxrepCT0) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct = 0;
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

TEST_F(KtClassTest, ReadBulk_ADDTOBUFFER_maxrepCT1) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

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


TEST_F(KtClassTest, ReadBulk_readreq) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct = 1;
  int child_index = 1;
  pfc_bool_t parent_call=false;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = new ReadRequest;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_SUCCESS);
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_SUCCESS);
  unc::uppl::ODBCManager::clearStubData();
}

TEST_F(KtClassTest, ReadBulk_DBErr) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);

  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct = 1;
  int child_index = 1;
  pfc_bool_t parent_call=false;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = new ReadRequest;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  int ret = ktlinkobj.ReadBulk(db_conn, &k, UNC_DT_STATE, max_rep_ct, child_index, parent_call, is_read_next,read_req);
  EXPECT_EQ(ret, UPPL_RC_ERR_DB_ACCESS);
  unc::uppl::ODBCManager::clearStubData();
}


TEST_F(KtClassTest, ReadBulkInternal_RecordNotFound) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;

  getKeyForKtLink1(k);
  uint32_t operation_type = UNC_OP_READ_BULK;
  uint32_t max_rep_ct=1;
  int child_index=-1;
  pfc_bool_t parent_call=true;
  pfc_bool_t is_read_next=true;
  OdbcmConnectionHandler *db_conn = NULL;
  ReadRequest *read_req = NULL;

  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_key;

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);
  int ret = ktlinkobj.ReadBulkInternal(db_conn, &k, UNC_DT_STATE, max_rep_ct, vect_val_link_st, vect_link_key);
  EXPECT_EQ(ret, UPPL_RC_ERR_NO_SUCH_INSTANCE);
  unc::uppl::ODBCManager::clearStubData();
}
TEST_F(KtClassTest, LinkPerformRead_DBErr) {
  key_link_t k;
  val_link_st v;
  memset(&k, 0, sizeof(key_link_t));
  memset(&v, 0, sizeof(val_link_st));
  Kt_Link ktlinkobj;
  physical_request_header rh;
  getReqHeader(rh, UNC_OP_READ, UNC_DT_STATE);
  rh.key_type = UNC_KT_LINK;
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
  sess.stub_setAddOutput((uint32_t)UNC_KT_LINK);

  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_RECORD_NOT_FOUND);

  int ret =  ktlinkobj.PerformRead(db_conn,(uint32_t)0,(uint32_t)0,&k,&v,(uint32_t)UNC_DT_STATE,(uint32_t)UNC_OP_READ,sess,(uint32_t)UNC_OPT1_DETAIL,(uint32_t)UNC_OPT2_NONE,(uint32_t)1);
  EXPECT_EQ(ret,UPPL_RC_ERR_IPC_WRITE_ERROR);
}

TEST_F(KtClassTest, ReadLinkValFromDB_Success_Update) {
  key_link_t k;
  val_link_st v;

  void *key_struct;
  void *void_val_struct = NULL;
 
  pfc_bool_t bflag =true;

  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  uint32_t data_type = (uint32_t)UNC_DT_IMPORT;
  uint32_t operation_type = (uint32_t) UNC_OP_UPDATE;
  uint32_t max_rep_ct = 1;
  uint32_t option = 0;

  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::ISROWEXISTS, ODBCM_RC_MORE_ROWS_FOUND);
  UpplReturnCode read_status = ktlinkobj.ReadLinkValFromDB(db_conn, key_struct,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option);

  EXPECT_EQ(read_status,UPPL_RC_SUCCESS);
  ODBCManager::clearStubData();
}

#if 0

TEST_F(KtClassTest, ReadLinkValFromDB_Success_option1_sibling) {
  key_link_t k;
  val_link_st v;

  void *key_struct;
  void *void_val_struct = NULL;
 
  pfc_bool_t bflag =true;

  Kt_Link ktlinkobj;
  vector<string> sw_vect_key_value;
  getKeyForKtLink1(k,sw_vect_key_value);
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;

  physical_request_header rh;
  getReqHeader(rh, UNC_OP_CREATE, UNC_DT_STATE);

  uint32_t data_type = (uint32_t)UNC_DT_IMPORT;
  uint32_t operation_type = (uint32_t) UNC_OP_READ_SIBLING_BEGIN;
  uint32_t max_rep_ct = 1;
  uint32_t option = (uint32_t)UNC_OPT1_NORMAL;
  uint32_t option2 = (uint32_t)UNC_OPT2_MATCH_SWITCH2;

  OdbcmConnectionHandler *db_conn =NULL;
  uint32_t operation = UNC_OP_CREATE;
  unc::uppl::ODBCManager::stub_setResultcode(unc::uppl::ODBCManager::GETBULKROWS, ODBCM_RC_CONNECTION_ERROR);
  UpplReturnCode read_status = ktlinkobj.ReadLinkValFromDB(db_conn, key_struct,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option2);

  EXPECT_EQ(read_status,UPPL_RC_ERR_DB_ACCESS);
  ODBCManager::clearStubData();
}

#endif
