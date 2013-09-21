/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <limits.h>
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <pfc/ipc.h>
#include <pfc/ipc_struct.h>
#include <unc/keytype.h>
#include <pfcxx/synch.hh>
#include "vbr_momgr.hh"
#include "unc/keytype.h"
#include "config_mgr.hh"
#include "dal_odbc_mgr.hh"
#include "dal_dml_intf.hh"
#include "capa_intf.hh"
#include "capa_module_stub.hh"
#include "tclib_module.hh"
#include "ctrlr_mgr.hh"
#include "momgr_intf_stub.hh"


using ::testing::TestWithParam;
using ::testing::Values;
using namespace std;
using namespace unc::upll;
using namespace unc::tclib;
using namespace unc::upll::dal;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::config_momgr;
using namespace unc::capa;
using namespace pfc::core;
using namespace unc::upll::dal::schema::table;

namespace {

class VbrMoMgrTest: public VbrMoMgr, public ::testing::Test {
 public:

 protected:
  virtual void SetUp() {}

  virtual void TearDown() {}

};


uint32_t parseIPV4string(const char* ipAddress) {
  char ipbytes[4];
  sscanf(ipAddress, "%d.%d.%d.%d", &ipbytes[3], &ipbytes[2], &ipbytes[1], &ipbytes[0]);
  return ipbytes[0] & ipbytes[1] << 8 & ipbytes[2] << 16 & ipbytes[3] << 24;
}

void GetKeyValStruct(key_vbr *&kst, val_vbr *&vst) {

  const char *vtn_name = "VTN_1";
  const char *vbr_name = "VBR_1";
  const char *desc = "thisisvbridge";
  const char *ctrlr_id = "Controller1";
  kst = reinterpret_cast<key_vbr *>(malloc
                 (sizeof(key_vbr)));
  memset(kst,0,sizeof(key_vbr));
  strncpy(reinterpret_cast<char *>(kst->vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(kst->vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  vst = reinterpret_cast<val_vbr *>(malloc
  (sizeof(val_vbr)));
  memset(vst,0,sizeof(val_vbr));
  memset(vst->controller_id,'\0',
        (sizeof(vst->controller_id)/sizeof(vst->controller_id[0])));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  vst->cs_row_status = UNC_VF_VALID;
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->cs_attr[loop] = UNC_CS_APPLIED;
  }
  strncpy(reinterpret_cast<char *>(vst->vbr_description), desc,
  strlen(desc)+1);
  strncpy(reinterpret_cast<char *>(vst->controller_id), ctrlr_id,
  strlen(ctrlr_id)+1);
  vst->host_addr_prefixlen = (uint8_t)1;
  struct sockaddr_in sa;
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));

  vst->host_addr.s_addr = sa.sin_addr.s_addr;

}

TEST_F(VbrMoMgrTest, ValidateVbrKeySuccess) {

  VbrMoMgr vbrmomgr;
  key_vbr *key;
  const char *vtn_name = "VTN_1";
  const char *vbr_name = "VBR_1";
  key = reinterpret_cast<key_vbr *>(malloc
                 (sizeof(key_vbr)));
  memset(key,0,sizeof(key_vbr));
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
  vbr_name, strlen(vbr_name)+1);

  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.ValidateVbrKey(key));
}



TEST_F(VbrMoMgrTest, ValidateVbrKeyInvalidVtnName) {

  VbrMoMgr vbrmomgr;
  key_vbr *key;
  const char *vtn_name = "";
  const char *vbr_name = "VBR_1";
  key = reinterpret_cast<key_vbr *>(malloc
                 (sizeof(key_vbr)));
  memset(key,0,sizeof(key_vbr));
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
  vbr_name, strlen(vbr_name)+1);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrKey(key));
}

TEST_F(VbrMoMgrTest, ValidateVbrKeyInvalidVbrName) {

  VbrMoMgr vbrmomgr;
  key_vbr *key;
  const char *vtn_name = "VTN_1";
  const char *vbr_name = "";
  key = reinterpret_cast<key_vbr *>(malloc
                 (sizeof(key_vbr)));
  memset(key,0,sizeof(key_vbr));
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
  vbr_name, strlen(vbr_name)+1);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrKey(key));
}

TEST_F(VbrMoMgrTest, ValidateVbrKeyInvalidKeyStruct) {

  VbrMoMgr vbrmomgr;
  key_vbr *key = NULL;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrKey(key));

}

TEST_F(VbrMoMgrTest, ValidateVbrValueInvalidCtrlrID) {

  VbrMoMgr vbrmomgr;
  uint32_t oper = UNC_OP_CREATE;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  string ctrlr_id = "Controller 1";
  strncpy(reinterpret_cast<char *>(val->controller_id), ctrlr_id.c_str(),
  strlen(ctrlr_id.c_str())+1); 

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrValue(val, oper));

}

TEST_F(VbrMoMgrTest, ValidateVbrValueInvalidDesc) {

  VbrMoMgr vbrmomgr;
  uint32_t oper = UNC_OP_CREATE;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  string desc = "vbr_description 1";
  strncpy(reinterpret_cast<char *>(val->vbr_description), desc.c_str(),
  strlen(desc.c_str())+1); 

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrValue(val, oper));

}

TEST_F(VbrMoMgrTest, ValidateVbrValueDescValidAttrInvalid) {

  VbrMoMgr vbrmomgr;
  uint32_t oper = UNC_OP_UPDATE;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX , vbrmomgr.ValidateVbrValue(val, oper));

}

TEST_F(VbrMoMgrTest, ValidateVbrValuePrefLenValidAttrInvalid) {

  VbrMoMgr vbrmomgr;
  uint32_t oper = UNC_OP_UPDATE;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  val->host_addr_prefixlen = 0;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrValue(val, oper));

  val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrValue(val, oper));

}

TEST_F(VbrMoMgrTest, ValidateVbrValueInvalidIP) {

  VbrMoMgr vbrmomgr;
  uint32_t oper = UNC_OP_CREATE;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  val->host_addr.s_addr = parseIPV4string("255.255.255.255"); 
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrValue(val, oper));

}

TEST(ValidateVbrValue, invalidFlagNew) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.ValidateVbrValue(valvbr,op));
}

TEST(ValidateVbrValue, invalidFlag1) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));
}

TEST(ValidateVbrValue, validFlag2) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));
}

TEST(ValidateVbrValue, validFlag3) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));
}

TEST(ValidateVbrValue, validFlag4) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));
}
TEST(ValidateVbrValue, validFlag5) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));
}
TEST(ValidateVbrValue, validFlag6) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_PACKET_SIZE_PING] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));
}
TEST(ValidateVbrValue, validFlag7) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrValue(valvbr,op));
}

TEST(ValidateVbrValue, validFlag8) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrValue(valvbr,op));
}

TEST(ValidateVbrValue, validFlag9) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrValue(valvbr,op));
}

TEST(ValidateVbrValue, validFlag10) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.ValidateVbrValue(valvbr,op));
}

TEST(ValidateVbrValue, validFlag11) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_VALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.ValidateVbrValue(valvbr,op));
}
TEST(ValidateVbrValue, validFlag12) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_VALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.ValidateVbrValue(valvbr,op));
}



TEST_F(VbrMoMgrTest, GetParentConfigKeySuccess) {

  VbrMoMgr vbrmomgr;
  key_vbr *key;
  val_vbr *val; 
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ickv = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,key, cfg_val);
  ConfigKeyVal *ockv= NULL;
 
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.GetParentConfigKey(ockv, ickv));
}

TEST_F(VbrMoMgrTest, GetParentConfigKeyInvalidArg) {

  VbrMoMgr vbrmomgr;
  key_vbr *key;
  val_vbr *val; 
  GetKeyValStruct(key, val);
  ConfigKeyVal *ickv = NULL;
  ConfigKeyVal *ockv = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbrmomgr.GetParentConfigKey(ockv, ickv));
}

TEST_F(VbrMoMgrTest, GetParentConfigKeyInvalidKT) {

  VbrMoMgr vbrmomgr;
  key_vbr *key;
  val_vbr *val; 
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, val);

  ConfigKeyVal *ockv = NULL;
  ConfigKeyVal *ickv = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVbr,key, cfg_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbrmomgr.GetParentConfigKey(ockv, ickv));
} 
TEST_F(VbrMoMgrTest, GetParentConfigKeyNullKey) {

  VbrMoMgr vbrmomgr;
  val_vbr *val=(val_vbr *)malloc(sizeof(val_vbr));;
  ConfigKeyVal *ickv = new ConfigKeyVal(UNC_KT_VBRIDGE);
  ConfigKeyVal *ockv = new ConfigKeyVal(UNC_KT_VBRIDGE);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbrmomgr.GetParentConfigKey(ockv, ickv));
}



TEST_F(VbrMoMgrTest, AllocVal_outputNull) {
  VbrMoMgr obj;
  val_vbr *val = new val_vbr_t();
  ConfigVal* cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(cfgval, UPLL_DT_IMPORT,MAINTBL));
  delete cfgval;
}

TEST_F(VbrMoMgrTest, AllocVal_SuccessMAINTBL) {
  VbrMoMgr obj;
  ConfigVal* cfgval = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(cfgval, UPLL_DT_IMPORT,MAINTBL));
}

TEST_F(VbrMoMgrTest, AllocVal_SuccessRENAMETBL) {
  VbrMoMgr obj;
  ConfigVal* cfgval = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(cfgval, UPLL_DT_IMPORT,RENAMETBL));
}

TEST_F(VbrMoMgrTest, AllocVal_SuccessDT_STATE) {
  VbrMoMgr obj;
  ConfigVal* cfgval = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(cfgval, UPLL_DT_STATE,MAINTBL));
}

TEST_F(VbrMoMgrTest, AllocVal_Error) {
  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(cfgval, UPLL_DT_STATE,RENAMETBL));
}

TEST_F(VbrMoMgrTest, AllocVal_ErrorDefaultCase) {
  VbrMoMgr obj;
  ConfigVal* cfgval = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(cfgval, UPLL_DT_STATE,CTRLRTBL));
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_ReqNull) {

  VbrMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, NULL);
  ConfigKeyVal *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req, MAINTBL));
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_OkeyNotNull) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, NULL);

  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req, MAINTBL));
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_ReqInvalidKT) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr,
                                 val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVbr,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_Req_InValid) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_SuccessMAINTBL) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
  tmp->AppendCfgVal(tmp1); 
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,MAINTBL));
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_SuccessRENAMETBL) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,RENAMETBL));
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_SuccessRENAMETBLInvalidStNum) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,RENAMETBL));
}


TEST_F(VbrMoMgrTest, DupConfigKeyValNullValStuct) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key=(key_vbr*)malloc(sizeof(key_vbr));
  ConfigVal *tmp=NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,RENAMETBL));
}

TEST_F(VbrMoMgrTest, DupConfigKeyValNullValStuctMainTbl) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key=(key_vbr*)malloc(sizeof(key_vbr));
  ConfigVal *tmp=NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                       IpctSt::kIpcStKeyVbr,
                                       key, tmp);
   EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,MAINTBL));
    }
 

TEST_F(VbrMoMgrTest, IsValidKey_SuccessVTNName) {
  VbrMoMgr obj;
  uint64_t index = uudst::vbridge::kDbiVtnName;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  EXPECT_EQ(true, obj.IsValidKey(reinterpret_cast<void *>(key),
                                 index));
}

TEST_F(VbrMoMgrTest, IsValidKey_SuccessVBRName) {
  VbrMoMgr obj;
  uint64_t index = uudst::vbridge::kDbiVbrName;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  EXPECT_EQ(true, obj.IsValidKey(reinterpret_cast<void *>(key),
                                 index));
}

TEST_F(VbrMoMgrTest, IsValidKey_InvalidIndex) {
  VbrMoMgr obj;
  uint64_t index = uudst::vbridge::kDbiCtrlrName;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  EXPECT_EQ(false, obj.IsValidKey(reinterpret_cast<void *>(key),
                                 index));
}

TEST_F(VbrMoMgrTest, IsValidKey_InvalidVTNName) {
  VbrMoMgr obj;
  uint64_t index = uudst::vbridge::kDbiVtnName;
  key_vbr *key;
  const char *vtn_name = "";
  const char *vbr_name = "VBR_1";
  key = reinterpret_cast<key_vbr *>(malloc
                 (sizeof(key_vbr)));
  memset(key,0,sizeof(key_vbr));
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
  vbr_name, strlen(vbr_name)+1);

  EXPECT_EQ(false, obj.IsValidKey(reinterpret_cast<void *>(key),
                                 index));
}

TEST_F(VbrMoMgrTest, IsValidKey_InvalidVBRName) {
  VbrMoMgr obj;
  uint64_t index = uudst::vbridge::kDbiVbrName;
  key_vbr *key;
  const char *vtn_name = "VTN";
  const char *vbr_name = "";
  key = reinterpret_cast<key_vbr *>(malloc
                 (sizeof(key_vbr)));
  memset(key,0,sizeof(key_vbr));
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
  vbr_name, strlen(vbr_name)+1);

  EXPECT_EQ(false, obj.IsValidKey(reinterpret_cast<void *>(key),
                                 index));
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_SuccessNullObjs) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_pkeyNull) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL; 
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_SuccesspkeyVBR) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vbr *key = reinterpret_cast<key_vbr*>
                  (malloc(sizeof(key_vbr_t)));
  memset(key, 0 ,sizeof(key_vbr_t));

  strncpy((char*) key->vbridge_name,"VBR1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("VBR1",(reinterpret_cast<const char *> (output->vbridge_name)));
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_SuccesspkeyVTN) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vbr *key = reinterpret_cast<key_vbr*>
                  (malloc(sizeof(key_vbr_t)));
  memset(key, 0 ,sizeof(key_vbr_t));

  strncpy((char*) key->vbridge_name,"VBR1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("",(reinterpret_cast<const char *> (output->vbridge_name)));
}
TEST_F(VbrMoMgrTest, GetChildConfigKey_SuccessOkeyVTN) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vbr *key = reinterpret_cast<key_vbr*>
                  (malloc(sizeof(key_vbr_t)));
  memset(key, 0 ,sizeof(key_vbr_t));

  strncpy((char*) key->vbridge_name,"VBR1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("",(reinterpret_cast<const char *> (output->vbridge_name)));
}


TEST_F(VbrMoMgrTest, CopyToConfigkey_ikeyokeyNull) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
}

TEST(GetChildConfigKey, PkeyVbrSuccess) {
  VbrMoMgr vbr;
  ConfigKeyVal *okey = NULL;

  key_vbr *key = reinterpret_cast<key_vbr*>
                  (malloc(sizeof(key_vbr_t)));
  memset(key, 0 ,sizeof(key_vbr_t));

  strncpy((char*) key->vbridge_name,"VBR1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("VBR1",(reinterpret_cast<const char *> (output->vbridge_name)));
}

TEST(GetChildConfigKey, OkeyVtnSuccess) {
  VbrMoMgr vbr;

   ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN);
  key_vbr *key = reinterpret_cast<key_vbr*>
                  (malloc(sizeof(key_vbr_t)));
  memset(key, 0 ,sizeof(key_vbr_t));

  strncpy((char*) key->vbridge_name,"VBR1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("",(reinterpret_cast<const char *> (output->vbridge_name)));
}

TEST(GetChildConfigKey, PkeyVlinkSuccess) {
  VbrMoMgr vbr;

   ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VLINK) ;
  key_vbr *key = reinterpret_cast<key_vbr*>
                  (malloc(sizeof(key_vbr_t)));
  memset(key, 0 ,sizeof(key_vbr_t));

  strncpy((char*) key->vbridge_name,"VLINK1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

 ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

 EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("",(reinterpret_cast<const char *> (output->vbridge_name)));
}

TEST(VbrMoMgr, GetChildConfigKey5) {
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  VbrMoMgr vbr_obj;
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge");

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VLINK,IpctSt::kIpcStKeyVbr,key_vbr);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr_obj.GetChildConfigKey(okey, pkey));
}

TEST(VbrMoMgr, GetChildConfigKey6) {
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  VbrMoMgr vbr_obj;
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge");
  val_vlink *vlink_val = reinterpret_cast<val_vlink *>( malloc(sizeof(val_vlink)));
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVlink, vlink_val);

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VLINK,IpctSt::kIpcStKeyVbr,key_vbr,cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr_obj.GetChildConfigKey(okey, pkey));
}



TEST_F(VbrMoMgrTest, CopyToConfigkey_InValidName) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key;
  const char *vtn_name = "";
  const char *vbr_name = "";
  key = reinterpret_cast<key_vbr *>(malloc
                 (sizeof(key_vbr)));
  memset(key,0,sizeof(key_vbr));
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
  vbr_name, strlen(vbr_name)+1);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
  cout<<"TEST: Negative: InvalidVbrName"<<endl;

  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),"VTN_1",32);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
}

TEST_F(VbrMoMgrTest, CopyToConfigkey_Valid) {
  VbrMoMgr obj;
  ConfigKeyVal *okey;
  key_vbr *key;
  val_vbr *val;
  const char *vtn_name = "VTN_1";
  const char *vbr_name = "VBR_1";
  key = reinterpret_cast<key_vbr *>(malloc
                 (sizeof(key_vbr)));
  memset(key,0,sizeof(key_vbr));
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.CopyToConfigKey(okey,ikey));

}


TEST_F(VbrMoMgrTest, ValidateVbrPingValue_Success) {
  VbrMoMgr obj;
  uint32_t oper = UNC_OP_CREATE;
  val_ping *vst = reinterpret_cast<val_ping *>(malloc
  (sizeof(val_ping)));
  memset(vst,0,sizeof(val_ping));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  struct sockaddr_in sa;
  inet_pton(AF_INET, "192.168.1.1", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  vst->dfbit = UPLL_DF_BIT_ENABLE;
  vst->packet_size = 5;
  vst->count = 14;
  vst->interval = 23;
  vst->timeout = 32; 

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ValidateVbrPingValue(vst));
  cout<<"TEST: Positive: Success"<<endl;

}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_InvalidTgtAddr) {
  VbrMoMgr obj;
  uint32_t oper = UNC_OP_CREATE;
  val_ping *vst = reinterpret_cast<val_ping *>(malloc
  (sizeof(val_ping)));
  memset(vst,0,sizeof(val_ping));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  struct sockaddr_in sa;
  inet_pton(AF_INET, "255.255.255.255", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  vst->dfbit = UPLL_DF_BIT_ENABLE;
  vst->packet_size = 5;
  vst->count = 14;
  vst->interval = 23;
  vst->timeout = 32;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateVbrPingValue(vst));

  inet_pton(AF_INET, "224.1.1.1", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateVbrPingValue(vst));
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_InvalidSrcAddr) {
  VbrMoMgr obj;
  uint32_t oper = UNC_OP_CREATE;
  val_ping *vst = reinterpret_cast<val_ping *>(malloc
  (sizeof(val_ping)));
  memset(vst,0,sizeof(val_ping));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  struct sockaddr_in sa;
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")
  inet_pton(AF_INET, "255.255.255.255", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  vst->dfbit = UPLL_DF_BIT_ENABLE;
  vst->packet_size = 5;
  vst->count = 14;
  vst->interval = 23;
  vst->timeout = 32;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateVbrPingValue(vst));

  inet_pton(AF_INET, "224.1.1.1", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateVbrPingValue(vst));
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_DFbitValidation) {
  VbrMoMgr obj;
  uint32_t oper = UNC_OP_CREATE;
  val_ping *vst = reinterpret_cast<val_ping *>(malloc
  (sizeof(val_ping)));
  memset(vst,0,sizeof(val_ping));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  struct sockaddr_in sa;
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  vst->dfbit = 3;
  vst->packet_size = 5;
  vst->count = 14;
  vst->interval = 23;
  vst->timeout = 32;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateVbrPingValue(vst));

  vst->valid[UPLL_IDX_DF_BIT_PING] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ValidateVbrPingValue(vst));
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_PktSzValidation) {
  VbrMoMgr obj;
  uint32_t oper = UNC_OP_UPDATE;
  val_ping *vst = reinterpret_cast<val_ping *>(malloc
  (sizeof(val_ping)));
  memset(vst,0,sizeof(val_ping));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  struct sockaddr_in sa;
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  vst->dfbit = UPLL_DF_BIT_ENABLE;
  vst->packet_size = 65535;
  vst->count = 14;
  vst->interval = 23;
  vst->timeout = 32;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateVbrPingValue(vst));

  vst->valid[UPLL_IDX_PACKET_SIZE_PING] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ValidateVbrPingValue(vst));
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_CntValidation) {
  VbrMoMgr obj;
  uint32_t oper = UNC_OP_UPDATE;
  val_ping *vst = reinterpret_cast<val_ping *>(malloc
  (sizeof(val_ping)));
  memset(vst,0,sizeof(val_ping));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  struct sockaddr_in sa;
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  vst->dfbit = UPLL_DF_BIT_ENABLE;
  vst->packet_size = 6;
  vst->count = 0;
  vst->interval = 23;
  vst->timeout = 32;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateVbrPingValue(vst));

  vst->valid[UPLL_IDX_COUNT_PING] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ValidateVbrPingValue(vst));
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_InterValidation) {
  VbrMoMgr obj;
  uint32_t oper = UNC_OP_CREATE;
  val_ping *vst = reinterpret_cast<val_ping *>(malloc
  (sizeof(val_ping)));
  memset(vst,0,sizeof(val_ping));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  struct sockaddr_in sa;
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  vst->dfbit = UPLL_DF_BIT_ENABLE;
  vst->packet_size = 6;
  vst->count = 1;
  vst->interval = 100;
  vst->timeout = 32;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateVbrPingValue(vst));

  vst->valid[UPLL_IDX_INTERVAL_PING] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ValidateVbrPingValue(vst));
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_TimeOutValidation) {
  VbrMoMgr obj;
  uint32_t oper = UNC_OP_CREATE;
  val_ping *vst = reinterpret_cast<val_ping *>(malloc
  (sizeof(val_ping)));
  memset(vst,0,sizeof(val_ping));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  struct sockaddr_in sa;
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  vst->dfbit = UPLL_DF_BIT_ENABLE;
  vst->packet_size = 6;
  vst->count = 1;
  vst->interval = 1;
  vst->timeout = 255;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateVbrPingValue(vst));

  vst->valid[UPLL_IDX_TIMEOUT_PING] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ValidateVbrPingValue(vst));
}

TEST_F(VbrMoMgrTest, UpdateConfigStatus_Success) {

  VbrMoMgr obj;
  DalDmlIntf *dmi= NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

}

TEST_F(VbrMoMgrTest,UpdateConfigStatus_SuccessUPDATE) {

  VbrMoMgr obj;
  DalDmlIntf *dmi= NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

}


TEST_F(VbrMoMgrTest, UpdateConfigStatus_InvalidOP) {

  VbrMoMgr obj;
  DalDmlIntf *dmi= NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_READ,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

}

TEST_F(VbrMoMgrTest,UpdateConfigStatus_InvalidArg) {

  VbrMoMgr obj;
  DalDmlIntf *dmi= NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, NULL);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
}

TEST_F(VbrMoMgrTest, UpdateConfigStatus_AttrNotSupp_ValNoValue) {

  VbrMoMgr obj;
  DalDmlIntf *dmi= NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  val->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_NOT_SUPPORTED;
  val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_VALID_NO_VALUE;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

}


TEST_F(VbrMoMgrTest, CreateVnodeConfigKey_Success) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigKeyVal *okey = NULL;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.CreateVnodeConfigKey(ikey, okey));

}

TEST_F(VbrMoMgrTest, CreateVnodeConfigKey_NULLArg) {

  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateVnodeConfigKey(ikey, okey));
}

TEST_F(VbrMoMgrTest, CompareValidValue_AuditTrue) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val, *val1;
  GetKeyValStruct(key, val);
  GetKeyValStruct(key, val1);

  strncpy(reinterpret_cast<char *>(val1->controller_id), "CTR1",
  strlen("CTR1")+1);
  void *vbrval = reinterpret_cast<void *>(&val);

  obj.CompareValidValue(vbrval, (void *)val,true);
  obj.CompareValidValue(vbrval, (void *)val1,true);

}

TEST(FilterAttributes,CreateOperation) {
  VbrMoMgr vbr;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vbr_t *valvbr1 = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  valvbr1->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  EXPECT_EQ(false, vbr.FilterAttributes(val1,val2,audit_status,op));
}

TEST(FilterAttributes,OperationUpdate) {
  VbrMoMgr vbr;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_UPDATE;

  val_vbr_t *valvbr1 = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  valvbr1->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  EXPECT_EQ(true, vbr.FilterAttributes(val1,val2,audit_status,op));
}


TEST_F(VbrMoMgrTest, GetRenameKeyBindInfo) {

  VbrMoMgr obj;
  BindInfo *binfo;
  int nattr;

  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(UNC_KT_VBRIDGE, binfo, nattr, MAINTBL));

  EXPECT_EQ(5, nattr);
  EXPECT_EQ(VbrMoMgr::key_vbr_maintbl_bind_info, binfo);


  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(UNC_KT_VBRIDGE, binfo, nattr, RENAMETBL));

  EXPECT_EQ(4, nattr);
  EXPECT_EQ(VbrMoMgr::key_vbr_renametbl_update_bind_info, binfo);

}

TEST(GetRenameKeyBindInfo, OutputUnknownTbl) {
  VbrMoMgr vbr;
  unc_key_type_t key_type = UNC_KT_VBRIDGE;
  BindInfo *binfo = NULL;
  int nattr = 2;
  MoMgrTables tbl;

  EXPECT_EQ(false, vbr.GetRenameKeyBindInfo(key_type, binfo, nattr,CTRLRTBL ));
  EXPECT_EQ(2,nattr);
}

TEST_F(VbrMoMgrTest, GetVnodeName) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  uint8_t *vtn_name, *vnode_name;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetVnodeName(ikey, vtn_name, vnode_name));
  EXPECT_STREQ("VTN_1",(reinterpret_cast<const char *> (vtn_name)));
  EXPECT_STREQ("VBR_1",(reinterpret_cast<const char *> (vnode_name)));

  ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, cfgval);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetVnodeName(ikey, vtn_name, vnode_name));

}

TEST_F(VbrMoMgrTest, ValidateCapability_ErrorInput) {

  VbrMoMgrStub obj;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
 IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(req, ikey,NULL ));


}

TEST(ValidateCapability, ValidateCapability_Success) {

  VbrMoMgr vbr;
  DalDmlIntf *dmi= NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);


  CtrlrMgr::Ctrlr ctrlrobj("CTR_1", UNC_CT_PFC, "5.0");
  CtrlrMgr::GetInstance()->Add(ctrlrobj, UPLL_DT_CANDIDATE);

  uint8_t attrs[3];
  attrs[unc::capa::vbr::kCapDesc] = 1;
  uint32_t max_inst = 5;
  uint32_t num_attrs =5;

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValidateCapability(req, ikey, "CTR_1"));

  req->operation = UNC_OP_UPDATE;

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValidateCapability(req, ikey, "CTR_1"));

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
}


TEST(ValidateCapability, ValidateCapability_Success1) {

  VbrMoMgr vbr;
  DalDmlIntf *dmi= NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);


  CtrlrMgr::Ctrlr ctrlrobj("CTR_1", UNC_CT_PFC, "5.0");
  CtrlrMgr::GetInstance()->Add(ctrlrobj, UPLL_DT_CANDIDATE);

  uint8_t attrs[3];
  attrs[unc::capa::vbr::kCapDesc] = 1;
  uint32_t max_inst = 5;
  uint32_t num_attrs =5;

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValidateCapability(req, ikey, "CTR_1"));

  req->operation = UNC_OP_UPDATE;

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValidateCapability(req, ikey, "CTR_1"));

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->datatype = UPLL_DT_STATE;

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValidateCapability(req, ikey, "CTR_1"));

  val_vbr* no_val = NULL;
  cfgval = new ConfigVal(IpctSt::kIpcStValVbr, no_val);
  ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValidateCapability(req, ikey, "CTR_1"));

  req->operation = UNC_OP_DELETE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, "CTR_1"));

  CtrlrMgr::GetInstance()->Delete("CTR_1", UPLL_DT_CANDIDATE);
}
	
TEST(ValidateCapability, ValidateCapability_ikey_NULL) {

  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req=NULL;
  const char *ctrlr_name="ctr1";
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, ctrlr_name));
}
TEST(ValidateCapability, ValidateCapability_ctrName_NULL) {

  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));

  const char *ctrlr_name=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, ctrlr_name));
}


TEST(ValidateCapability, ValidateCapability_ctrName) {

  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));

  const char *ctrlr_name=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, ctrlr_name));
}

TEST_F(VbrMoMgrTest, ValidateMessage_Success) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ikey));

  val_rename_vbr *renameval;
  renameval = reinterpret_cast<val_rename_vbr *>(malloc
  (sizeof(val_rename_vbr)));
  memset(renameval,0,sizeof(val_rename_vbr));
  for(unsigned int loop = 0; loop < sizeof(renameval->valid)/
     sizeof(renameval->valid[0]); ++loop) {
    renameval->valid[loop] = UNC_VF_VALID;
  }
  strncpy(reinterpret_cast<char *>(renameval->new_name),
  "renamed", strlen("renamed")+1);

  ConfigVal *rename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, renameval);
  ConfigKeyVal *rename_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, rename_cfgval);

  req->operation = UNC_OP_RENAME;
  req->datatype = UPLL_DT_IMPORT;
  EXPECT_EQ(UPPL_RC_SUCCESS,obj.ValidateMessage(req, rename_ikey));

  req->operation = UNC_OP_READ_SIBLING;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, rename_ikey));

  ConfigVal *invrename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, NULL);
  ConfigKeyVal *invrename_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, invrename_cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, invrename_ikey));

  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_RUNNING;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ikey));

  req->operation = UNC_OP_DELETE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ikey));

  req->operation = UNC_OP_CONTROL;
  val_ping *vst = reinterpret_cast<val_ping *>(malloc
  (sizeof(val_ping)));
  memset(vst,0,sizeof(val_ping));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  struct sockaddr_in sa;
  inet_pton(AF_INET, "192.168.1.1", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  vst->dfbit = UPLL_DF_BIT_ENABLE;
  vst->packet_size = 5;
  vst->count = 14;
  vst->interval = 23;
  vst->timeout = 32;

  ConfigVal *ping_cfgval = new ConfigVal(IpctSt::kIpcStValPing, vst);
  ConfigKeyVal *ping_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, ping_cfgval);
  req->option2 = UNC_OPT2_PING;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ping_ikey));

} 

TEST_F(VbrMoMgrTest, ValidateMessage_ReadSuccess) {
  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val ;
  GetKeyValStruct(key,val);
 ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr,val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ikey));
}

TEST_F(VbrMoMgrTest, ValidateMessage_ReadFailure) {
  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val ;
  GetKeyValStruct(key,val);
 ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr,val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
}


TEST_F(VbrMoMgrTest, ValidateMessage_ReadOption2Failure) {
  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val ;
  GetKeyValStruct(key,val);
 ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr,val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_L2DOMAIN;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrMoMgrTest, ValidateMessage_InvalidValVbr) {
  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val ;
  GetKeyValStruct(key,val);
 ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr,val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrMoMgrTest, ValidateMessage_NullVal) {
  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val ;
  GetKeyValStruct(key,val);
 ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr,val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrMoMgrTest, ValidateMessage_DiffOption) {
  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val ;
  GetKeyValStruct(key,val);
 ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr,val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_STATE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrMoMgrTest, ValidateMessage_DiffOption2) {
  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val ;
  GetKeyValStruct(key,val);
 ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr,val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_IP_ROUTE;
  req->datatype = UPLL_DT_STATE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));
}
TEST_F(VbrMoMgrTest, ValidateMessage_ValidDiffOption) {
  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val ;
  GetKeyValStruct(key,val);
 ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr,val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 =UNC_OPT2_MAC_ENTRY;
  req->option2 = UNC_OPT2_MAC_ENTRY_STATIC;
  req->option2 = UNC_OPT2_MAC_ENTRY_DYNAMIC;
  req->option2 = UNC_OPT2_L2DOMAIN;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_STATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrMoMgrTest, ValidateMessage_InvalidInputCREATE) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  ConfigKeyVal *invalkey = NULL;
  IpcReqRespHeader *inreq = NULL;

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;


  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateMessage(req, invalkey));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateMessage(inreq, ikey));

  ConfigVal *inval_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, val);
  ConfigKeyVal *invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyLogicalPort,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  invalcfgkey = new ConfigKeyVal(UNC_KT_VTUNNEL,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  req->datatype = UPLL_DT_AUDIT;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,obj.ValidateMessage(req, ikey));

  req->datatype = UPLL_DT_CANDIDATE;
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
  "", strlen("")+1);

  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, invalcfgkey));

  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
  "VTN_1", strlen("VTN_1")+1);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.ValidateMessage(req, invalcfgkey));

  ConfigVal *invalcfgval = new ConfigVal(IpctSt::kIpcStValVbr, NULL);

  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, invalcfgval);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.ValidateMessage(req, invalcfgkey));

  string ctrlr_id = "Controller 1";
  strncpy(reinterpret_cast<char *>(val->controller_id), ctrlr_id.c_str(),
  strlen(ctrlr_id.c_str())+1);

  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, invalcfgkey));

  delete invalcfgkey;
}

TEST_F(VbrMoMgrTest, ValidateMessage_InvalidInputRENAME) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_RENAME;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_IMPORT;

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));

  val_rename_vbr *renameval;
  renameval = reinterpret_cast<val_rename_vbr *>(malloc
  (sizeof(val_rename_vbr)));
  memset(renameval,0,sizeof(val_rename_vbr));
  for(unsigned int loop = 0; loop < sizeof(renameval->valid)/
     sizeof(renameval->valid[0]); ++loop) {
    renameval->valid[loop] = UNC_VF_VALID;
  }
  strncpy(reinterpret_cast<char *>(renameval->new_name),
  "renamed", strlen("renamed")+1);

  ConfigVal *rename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, renameval);
  ConfigKeyVal *rename_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, rename_cfgval);

  ConfigVal *invrename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, NULL);
  ConfigKeyVal *invrename_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, invrename_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invrename_ikey));

  strncpy(reinterpret_cast<char *>(renameval->new_name),
  "", strlen("")+1);

  invrename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, renameval);
  invrename_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, invrename_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, invrename_ikey));

  strncpy(reinterpret_cast<char *>(renameval->new_name),
  "renamed", strlen("renamed")+1);

  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, rename_ikey));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_PING;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, rename_ikey));

  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));

  strncpy(reinterpret_cast<char *>(renameval->new_name),
  "", strlen("")+1);

  invrename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, renameval);
  invrename_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, invrename_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, invrename_ikey));

  strncpy(reinterpret_cast<char *>(renameval->new_name),
  "renamed", strlen("renamed")+1);

  req->operation = UNC_OP_READ_BULK;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, rename_ikey));
}

TEST_F(VbrMoMgrTest, ValidateMessage_InvalidInputREAD) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_STARTUP;

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));

  req->option2 = UNC_OPT2_NONE;
  ConfigVal *inval_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, val);
  ConfigKeyVal *invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.ValidateMessage(req, invalcfgkey));

  inval_cfgval = new ConfigVal(IpctSt::kIpcStValVbr, NULL);

  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, inval_cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, invalcfgkey));

  string ctrlr_id = "Controller 1";
  strncpy(reinterpret_cast<char *>(val->controller_id), ctrlr_id.c_str(),
  strlen(ctrlr_id.c_str())+1);

  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, invalcfgkey));

  req->datatype = UPLL_DT_AUDIT;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, invalcfgkey));

  req->operation = UNC_OP_DELETE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, invalcfgkey));
}

TEST_F(VbrMoMgrTest, ValidateMessage_InvalidInputCONTROL) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CONTROL;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_RUNNING;

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_MAC_ENTRY;

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));

  req->option2 = UNC_OPT2_PING;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.ValidateMessage(req, ikey));

  val_ping *vst = reinterpret_cast<val_ping *>(malloc
  (sizeof(val_ping)));
  memset(vst,0,sizeof(val_ping));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  struct sockaddr_in sa;
  inet_pton(AF_INET, "255.255.255.255", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  vst->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  vst->dfbit = UPLL_DF_BIT_ENABLE;
  vst->packet_size = 5;
  vst->count = 14;
  vst->interval = 23;
  vst->timeout = 32;

  ConfigVal *ping_cfgval = new ConfigVal(IpctSt::kIpcStValPing, vst);
  ConfigKeyVal *ping_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, ping_cfgval);
  
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ping_ikey));

  inet_pton(AF_INET, "192.168.1.1", &(sa.sin_addr));
  vst->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")


  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ping_ikey));

  req->operation = UNC_OP_INVALID;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,obj.ValidateMessage(req, ping_ikey));
}


TEST_F(VbrMoMgrTest, AdaptValToVtnService_Success) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  val_vbr_st *valst;
  valst = reinterpret_cast<val_vbr_st *>(malloc
  (sizeof(val_vbr_st)));

  memset(valst,0,sizeof(val_vbr_st));
  valst->valid[0] = UNC_VF_VALID;
  valst->oper_status = UPLL_OPER_STATUS_UP;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbrSt, valst);


  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfg_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey));
  delete ikey;
}

TEST_F(VbrMoMgrTest, AdaptValToVtnService_Failure) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigKeyVal* ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey));

  ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                          IpctSt::kIpcStKeyVbr,
                          key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey));
  delete ikey;
}

TEST_F(VbrMoMgrTest, GetValid) {

  VbrMoMgr obj;
  key_vbr *key;
  val_vbr *val;
  uint8_t * valid;
  GetKeyValStruct(key, val);

  void *in_val = reinterpret_cast<void *>(val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val, vbridge::kDbiCtrlrName,
                                          valid, UPLL_DT_CANDIDATE, MAINTBL));
  EXPECT_EQ(val->valid[UPLL_IDX_CONTROLLER_ID_VBR], valid[UPLL_IDX_CONTROLLER_ID_VBR]);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val, vbridge::kDbiDomainId,
                                          valid, UPLL_DT_CANDIDATE, MAINTBL));
  EXPECT_EQ(val->valid[UPLL_IDX_DOMAIN_ID_VBR], valid[UPLL_IDX_DOMAIN_ID_VBR]);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val, vbridge::kDbiVbrDesc,
                                          valid, UPLL_DT_CANDIDATE, MAINTBL));
  EXPECT_EQ(val->valid[UPLL_IDX_DESC_VBR], valid[UPLL_IDX_DESC_VBR]);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val, vbridge::kDbiHostAddr,
                                          valid, UPLL_DT_CANDIDATE, MAINTBL));
  EXPECT_EQ(val->valid[UPLL_IDX_HOST_ADDR_VBR], valid[UPLL_IDX_HOST_ADDR_VBR]);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val, vbridge::kDbiHostAddrMask,
                                          valid, UPLL_DT_CANDIDATE, MAINTBL));
  EXPECT_EQ(val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR], valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR]);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val, vbridge::kDbiHostAddrMask,
                                          valid, UPLL_DT_CANDIDATE, RENAMETBL));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetValid(in_val, vbridge::kDbiVbrName,
                                          valid, UPLL_DT_CANDIDATE, MAINTBL));

  in_val = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetValid(in_val, vbridge::kDbiCtrlrName,
                                          valid, UPLL_DT_CANDIDATE, MAINTBL));

  val_vbr_st *valst;
  valst = reinterpret_cast<val_vbr_st *>(malloc
  (sizeof(val_vbr_st)));

  memset(valst,0,sizeof(val_vbr_st));
  valst->valid[0] = UNC_VF_VALID;
  valst->oper_status = UPLL_OPER_STATUS_UP;

  in_val = reinterpret_cast<void *>(valst);
  
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val, 0, valid, UPLL_DT_STATE, RENAMETBL));
  EXPECT_EQ(val->valid[UPLL_IDX_OPER_STATUS_VBRS], valid[UPLL_IDX_OPER_STATUS_VBRS]);

}


TEST(SwapKeyVal,IpctSt_valid ) {

  VbrMoMgr vbr;
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vbr_name = "VBR_1";
  bool no_rename;
  key_vbr_t *key = (key_vbr_t *)malloc(sizeof(key_vbr_t));
  memset(key,0,sizeof(key_vbr));
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  val_rename_vbr_t *val = (val_rename_vbr_t *)malloc(sizeof(val_rename_vbr_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));

  val->valid[UPLL_IDX_NEW_NAME_RVBR] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));

  ConfigVal *config_val1= new ConfigVal(IpctSt::kIpcStValVbrSt, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVbr, key, config_val1);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.SwapKeyVal(ikey1,okey,dmi,ctr_id1,no_rename));

  ConfigVal *config_val2= NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val2);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.SwapKeyVal(ikey2,okey,dmi,ctr_id1,no_rename));

  val_rename_vbr_t *val3 = NULL;
  ConfigVal *config_val3= new ConfigVal(IpctSt::kIpcStValVbrSt, val3);
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val3);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.SwapKeyVal(ikey3,okey,dmi,ctr_id1,no_rename));

  delete ikey,ikey1,ikey2,okey;
}



TEST_F(VbrMoMgrTest, UpdateAuditConfigStatus_ValidCsStatus) {
  VbrMoMgr vbrmomgr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, val);
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  UpdateCtrlrPhase phase = kUpllUcpCreate;
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, (void*)key, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running));
}

TEST_F(VbrMoMgrTest, UpdateAuditConfigStatus_InvalidCsStatus) {
  VbrMoMgr vbrmomgr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, val);
  unc_keytype_configstatus_t cs_status = UNC_CS_INVALID;
  UpdateCtrlrPhase phase = kUpllUcpCreate;
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, (void*)key, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running));
}

TEST_F(VbrMoMgrTest, UpdateAuditConfigStatus_EmptyVal) {
  VbrMoMgr vbrmomgr;
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  UpdateCtrlrPhase phase = kUpllUcpCreate;
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running));
}



TEST_F(VbrMoMgrTest, GetRenamedUncKey) {
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER,unc::tclib::TC_API_COMMON_SUCCESS);
  TcLibModule::stub_loadtcLibModule();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VbrMoMgr vbrmomgr;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  cout << "ctr_id1:" << ctr_id1<<endl;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, (void*)key, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.GetRenamedUncKey(ikey,dt_type,dmi,ctr_id1));
}

TEST(VbrMoMgr, GetRenamedControllerKey1) {
  ConfigKeyVal *ikey = NULL;
  VbrMoMgr vbr;
  controller_domain *ctrl_domain = NULL;
  DalDmlIntf *dmi = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.GetRenamedControllerKey(ikey,UPLL_DT_STATE,dmi,ctrl_domain));
}

TEST(VbrMoMgr, GetRenamedControllerKey2) {
  VbrMoMgr vbr;
  controller_domain *ctrl_domain = new controller_domain();
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalDmlIntf *dmi = new DalOdbcMgr;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,key_vbr);

  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));
}

TEST_F(VbrMoMgrTest, GetRenamedControllerKey) {
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER,unc::tclib::TC_API_COMMON_SUCCESS);
  TcLibModule::stub_loadtcLibModule();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VbrMoMgr vbrmomgr;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  cout << "ctr_id1:" << ctr_id1<<endl;
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  cout << "dom_id1:" << dom_id1<<endl;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctr_id1;
  ctrlr_dom.domain = dom_id1;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, (void*)key, cfg_val);
  controller_domain ctrlr_dom1;
  ctrlr_dom1.ctrlr = ctr_id1;
  ctrlr_dom1.domain = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.GetRenamedControllerKey(ikey,dt_type,dmi,&ctrlr_dom1));
}

TEST(GetRenamedControllerKey, GetRenamedControllerKey_01) {
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER,unc::tclib::TC_API_COMMON_SUCCESS);
  TcLibModule::stub_loadtcLibModule();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VbrMoMgr vbr;
  key_vbr_t *keyvbr = (key_vbr_t *)malloc(sizeof(key_vbr_t));
  val_vbr_t *valVbr = (val_vbr_t *)malloc(sizeof(val_vbr_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, valVbr);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, keyvbr, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  cout << "ctr_id1:" << ctr_id1<<endl;
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  cout << "dom_id1:" << dom_id1<<endl;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctr_id1;
  ctrlr_dom.domain = dom_id1;
  controller_domain ctrlr_dom1;
  ctrlr_dom1.ctrlr = ctr_id1;
  ctrlr_dom1.domain = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetRenamedControllerKey(ikey,dt_type,dmi,&ctrlr_dom1));
}

TEST(VbrMoMgr, GetControllerDomainId1) {
  VbrMoMgr vbr;
  controller_domain *ctrl_domain = new controller_domain();
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,key_vbr);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetControllerDomainId(ikey,ctrl_domain));
}

TEST(VbrMoMgr, GetControllerDomainId2) {
  VbrMoMgr vbr;
  controller_domain *ctrl_domain = new controller_domain();
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.GetControllerDomainId(ikey,ctrl_domain));
}
TEST(VbrMoMgr, GetControllerDomainId3) {
  VbrMoMgr vbr;
  controller_domain *ctrl_domain = NULL;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.GetControllerDomainId(ikey,ctrl_domain));
}

TEST(VbrMoMgr, GetControllerDomainId4) {
  VbrMoMgr vbr;
  controller_domain *ctrl_domain = new controller_domain();
  val_vbr *vbr_val = (val_vbr_t *)(malloc(sizeof(val_vbr_t)));
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));

  strcpy((char *)vbr_val->controller_id, (char*)"");
  strcpy ((char*)vbr_val->vbr_description, (char *)"vbrcompleted");
 // vbr_val->dhcp_relay_admin_status = UPLL_ADMIN_ENABLE;

  vbr_val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  vbr_val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  //vbr_val->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VBR] = UNC_VF_VALID;


  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, vbr_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key_vbr, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetControllerDomainId(ikey,ctrl_domain));
}
TEST(VbrMoMgr, GetControllerDomainId5) {
  VbrMoMgr vbr;
  controller_domain *ctrl_domain = new controller_domain();
  val_vbr *vbr_val = (val_vbr_t *)(malloc(sizeof(val_vbr_t)));
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));

  strcpy((char *)vbr_val->controller_id, (char*)"one");
  strcpy ((char*)vbr_val->vbr_description, (char *)"vbrcompleted");
//  vrt_val->dhcp_relay_admin_status = UPLL_ADMIN_ENABLE;

  vbr_val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  vbr_val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
//  vrt_val->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT] = UNC_VF_VALID;


  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetControllerDomainId(ikey,ctrl_domain));
}

TEST(GetControllerDomainId,InvalidVal){
  VbrMoMgr vbr;
  key_vbr *key= (key_vbr *)malloc(sizeof(key_vbr));
  val_vbr *val =(val_vbr *)malloc(sizeof(val_vbr));
  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;
  strlen(reinterpret_cast<char*>(val->controller_id));
  strlen(reinterpret_cast<char*>(val->domain_id));
  controller_domain *ctrlr_dom =(controller_domain *)malloc(sizeof(controller_domain));
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetControllerDomainId(ikey,ctrlr_dom));
}

TEST(GetControllerDomainId,SetDomainData){
  VbrMoMgr vbr;
  key_vbr *key= (key_vbr *)malloc(sizeof(key_vbr));
  val_vbr *val =(val_vbr *)malloc(sizeof(val_vbr));
  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;
   const char  *controller_id = "pfc1";
   const char  *domain_id = "dom1";
  strlen(reinterpret_cast<char*>(val->controller_id));
  strlen(reinterpret_cast<char*>(val->domain_id));
  controller_domain *ctrlr_dom =(controller_domain *)malloc(sizeof(controller_domain));

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  SET_USER_DATA_CTRLR(ikey,val->controller_id)
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetControllerDomainId(ikey,ctrlr_dom));
}


TEST(VbrMoMgr, GetRenameInfo1) {
  VbrMoMgr vbr;
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(NULL, okey, rename_info, NULL, NULL, no_rename));
}
TEST(VbrMoMgr, GetRenameInfo2) {
  VbrMoMgr vbr;
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, NULL, NULL, no_rename));
}
TEST(VbrMoMgr, GetRenameInfo3) {
  VbrMoMgr vbr;
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"name");
  val_rename_vnode_t *val =  (val_rename_vnode_t *) (malloc(sizeof(val_rename_vnode_t)));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1)");
  strcpy((char*)val->ctrlr_vnode_name, "vnode1");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));
}
TEST(VbrMoMgr, GetRenameInfo4) {
  VbrMoMgr vbr;
  bool no_rename = true;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"name");
  val_rename_vnode_t *val =  (val_rename_vnode_t *) (malloc(sizeof(val_rename_vnode_t)));
  strcpy((char*)val->ctrlr_vtn_name,"");
  strcpy((char*)val->ctrlr_vnode_name, "vnode1");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));
}
TEST(VbrMoMgr, GetRenameInfo5) {
  VbrMoMgr vbr;
  bool no_rename = true;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"name");
  val_rename_vnode_t *val =  (val_rename_vnode_t *) (malloc(sizeof(val_rename_vnode_t)));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1");
  strcpy((char*)val->ctrlr_vnode_name, "");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));
}

TEST(VbrMoMgr, GetRenameInfo6) {
  VbrMoMgr vbr;
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"");
  val_rename_vnode_t *val =  (val_rename_vnode_t *) (malloc(sizeof(val_rename_vnode_t)));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1");
  strcpy((char*)val->ctrlr_vnode_name, "");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));
}

TEST(VbrMoMgr, GetRenameInfo7) {
  VbrMoMgr vbr;
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"");
  strcpy((char *)key_vbr->vbridge_name, (char *)"name");
  val_rename_vnode_t *val =  (val_rename_vnode_t *) (malloc(sizeof(val_rename_vnode_t)));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1");
  strcpy((char*)val->ctrlr_vnode_name, "");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));
}

TEST(VbrMoMgr, GetRenameInfo8) {
  VbrMoMgr vbr;
  bool no_rename = true;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"");
  strcpy((char *)key_vbr->vbridge_name, (char *)"name");
  val_rename_vnode_t *val =  (val_rename_vnode_t *) (malloc(sizeof(val_rename_vnode_t)));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1");
  strcpy((char*)val->ctrlr_vnode_name, "");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));
}

TEST(VbrMoMgr, GetRenameInfo9) {
  VbrMoMgr vbr;
  bool no_rename = true;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1`");
  strcpy((char *)key_vbr->vbridge_name, (char *)"");
  val_rename_vnode_t *val =  (val_rename_vnode_t *) (malloc(sizeof(val_rename_vnode_t)));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1");
  strcpy((char*)val->ctrlr_vnode_name, "");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));
}

TEST(VbrMoMgr, IsReferenced1) {
  VbrMoMgr vbr;
  DalDmlIntf *dmi = new DalOdbcMgr;
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                        IpctSt::kIpcStKeyVbr,
                                        NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.IsReferenced(ikey, UPLL_DT_STATE, dmi));
}

TEST(VbrMoMgr, IsReferenced2) {
  VbrMoMgr vbr;
  DalDmlIntf *dmi = new DalOdbcMgr;
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  val_vbr *val;
 
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                        IpctSt::kIpcStKeyVbr,
                                        key_vbr, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.IsReferenced(ikey, UPLL_DT_STATE, dmi));
}
TEST(VbrMoMgr, UpdateAuditConfigStatus1) {
  VbrMoMgr vbr;
  ConfigKeyVal *ikey =NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey));
}
TEST(VbrMoMgr, UpdateAuditConfigStatus2) {
  VbrMoMgr vbr;
  val_vbr_t *val = reinterpret_cast<val_vbr_t *>
                   (malloc(sizeof(val_vbr_t)));
  memset(val, 0, sizeof(val_vbr_t));

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr,
                                 val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.UpdateAuditConfigStatus(UNC_CS_INVALID,uuc::kUpllUcpCreate, ikey));
}
TEST(VbrMoMgr, UpdateAuditConfigStatus3) {
  VbrMoMgr vbr;
  val_vbr_t *val = reinterpret_cast<val_vbr_t *>
                   (malloc(sizeof(val_vbr_t)));
  memset(val, 0, sizeof(val_vbr_t));
  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
  val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  //val->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT] = UNC_VF_VALID;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr,
                                 val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.UpdateAuditConfigStatus(UNC_CS_INVALID,uuc::kUpllUcpCreate, ikey));
  val_vbr_t *output = reinterpret_cast<val_vbr_t *> (GetVal(ikey));
  EXPECT_EQ(UNC_CS_INVALID,output->cs_attr[0]);
}

TEST(VbrMoMgr, SwapKeyVal1) {
  VbrMoMgr vbr;
  ConfigKeyVal *key = NULL;
  bool no_rename = false;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.SwapKeyVal(NULL, key, NULL, NULL, no_rename));
}

TEST(VbrMoMgr, SwapKeyVal2) {
  VbrMoMgr vbr;
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge1");

  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVbr, key_vbr);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.SwapKeyVal(ikey, okey, NULL, NULL, no_rename));
  delete ikey;
}

TEST(VbrMoMgr, SwapKeyVal3) {
  VbrMoMgr vbr;
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge1");

  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  val_rename_vbr *vbr_rename_val = NULL;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValRenameVbr, vbr_rename_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.SwapKeyVal(ikey, okey, NULL, NULL, no_rename));
  delete ikey;
}

TEST(VbrMoMgr, SwapKeyVal4) {
  VbrMoMgr vbr;
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge1");

  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  val_rename_vbr *vbr_rename_val = (val_rename_vbr *)(malloc(sizeof(val_rename_vbr)));
}
 
TEST(VbrMoMgr,IsHostAddrAndPrefixLenInUse1 ){
  VbrMoMgr vbr;
  DalDmlIntf *dmi= NULL;
  key_vbr *key= (key_vbr *)malloc(sizeof(key_vbr));
  val_vbr *val =(val_vbr *)malloc(sizeof(val_vbr));

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,key, cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.IsHostAddrAndPrefixLenInUse(ckv,dmi,req));
}
TEST(VbrMoMgr, SwapKeyVal7) {
  VbrMoMgr vbr;
  key_vbr_t *key_vbr = (key_vbr_t *)(malloc(sizeof(key_vbr_t)));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge1");
  const char * ctrlr_name = "ctrlr1";
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();

  val_rename_vbr *vbr_rename_val = (val_rename_vbr *)(malloc(sizeof(val_rename_vbr)));
  strcpy((char *)vbr_rename_val->new_name, (char*)"hhh");
  vbr_rename_val->valid[UPLL_IDX_NEW_NAME_RVRT] =  UNC_VF_VALID;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValRenameVbr, vbr_rename_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVrt, key_vbr, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.SwapKeyVal(ikey, okey, dmi, (uint8_t *)ctrlr_name, no_rename));
  delete ikey;
  }




TEST(VbrMoMgr,IsHostAddrAndPrefixLenInUse2 ){
  VbrMoMgr vbr;
  DalDmlIntf *dmi= NULL;
  key_vbr *key= (key_vbr *)malloc(sizeof(key_vbr));
  strcpy((char *)key->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key->vbridge_name, (char *)"vbridge1");
  val_vbr_t *val =(val_vbr_t *)malloc(sizeof(val_vbr_t));
  memset(val, 0, sizeof(val_vbr));
  val->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_INVALID;
  val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_INVALID;

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,key, cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.IsHostAddrAndPrefixLenInUse(ckv,dmi,req));
}


TEST(VbrMoMgr,CtrlrIdAndDomainIdUpdationCheck2){
  VbrMoMgr vbr;
  key_vbr *key= (key_vbr *)malloc(sizeof(key_vbr));
  val_vbr *val= (val_vbr *)malloc(sizeof(val_vbr));
 val_vbr *val1= (val_vbr *)malloc(sizeof(val_vbr));

  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;

  val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbr, val1);
  ConfigKeyVal *ckey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.CtrlrIdAndDomainIdUpdationCheck (ikey,ckey));
}
TEST(VbrMoMgr,CtrlrIdAndDomainIdUpdationCheck3){
  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;

  GetKeyValStruct(key, val);
 val_vbr *val1;

  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;

  val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbr, val1);
  ConfigKeyVal *ckey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.CtrlrIdAndDomainIdUpdationCheck (ikey,ckey));
}

TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_01) {

  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  const uint8_t *attrs;
  attrs[unc::capa::vbr::kCapDomainId] == 0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, val); 
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_02) {

  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  const uint8_t *attrs;
  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}

TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_03) {

  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
 uint8_t *attrs;
  val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  attrs[unc::capa::vbr::kCapDomainId];
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_04) {

  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation;
  val->valid[UPLL_IDX_DESC_VBR] == UNC_VF_VALID;
  const uint8_t *attrs;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValVbrAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}

}
