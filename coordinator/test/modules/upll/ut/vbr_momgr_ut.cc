/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include <pfc/util.h>
#include <unc/keytype.h>
#include <pfcxx/synch.hh>
#include <vbr_momgr.hh>
#include <unc/keytype.h>
#include <config_mgr.hh>
#include <dal_odbc_mgr.hh>
#include <dal_dml_intf.hh>
#include <capa_intf.hh>
#include <capa_module_stub.hh>
#include <tclib_module.hh>
#include <ctrlr_mgr.hh>
#include <momgr_intf_stub.hh>
#include <alarm.hh>
#include "ut_util.hh"


using ::testing::TestWithParam;
using ::testing::Values;
using namespace std;
using namespace unc::upll;
using namespace unc::tclib;
using namespace unc::upll::dal;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::config_momgr;
using namespace unc::upll::test;
using namespace unc::capa;
using namespace pfc::core;
using namespace unc::upll::dal::schema::table;

class VbrMoMgrTest
  : public UpllTestEnv
{
};

static void GetKeyStruct(key_vbr *&kst) {
  const char *vtn_name = "VTN_1";
  const char *vbr_name = "VBR_1";

  kst = ZALLOC_TYPE(key_vbr);
  strncpy(reinterpret_cast<char *>(kst->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(kst->vbridge_name),
          vbr_name, strlen(vbr_name)+1);
}

static void GetValStruct(val_vbr *&vst) {
  const char *desc = "thisisvbridge";
  const char *ctrlr_id = "Controller1";

  vst = ZALLOC_TYPE(val_vbr);
  for(unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->valid); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }

  vst->cs_row_status = UNC_VF_VALID;

  for(unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->cs_attr); ++loop) {
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

static void GetKeyValStruct(key_vbr *&kst, val_vbr *&vst)
{
  GetKeyStruct(kst);
  GetValStruct(vst);
}

TEST_F(VbrMoMgrTest, ValidateVbrKey_Success) {
  VbrMoMgr vbrmomgr;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  const char *vtn_name = "VTN_1";
  const char *vbr_name = "VBR_1";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
          vbr_name, strlen(vbr_name)+1);

  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.ValidateVbrKey(key));

  free(key);
}

TEST_F(VbrMoMgrTest, ValidateVbrKey_InvalidVtnName) {
  VbrMoMgr vbrmomgr;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  const char *vtn_name = "";
  const char *vbr_name = "VBR_1";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
          vbr_name, strlen(vbr_name)+1);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrKey(key));

  free(key);
}

TEST_F(VbrMoMgrTest, ValidateVbrKey_InvalidVbrName) {
  VbrMoMgr vbrmomgr;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  const char *vtn_name = "VTN_1";
  const char *vbr_name = "";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
          vbr_name, strlen(vbr_name)+1);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrKey(key));

  free(key);
}

TEST_F(VbrMoMgrTest, ValidateVbrKey_InvalidKeyStruct) {
  VbrMoMgr vbrmomgr;
  key_vbr *key = NULL;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrKey(key));
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_InvalidCtrlrID) {
  VbrMoMgr vbrmomgr;
  uint32_t oper = UNC_OP_CREATE;
  val_vbr *val;
  GetValStruct(val);
  string ctrlr_id = "Controller 1";
  strncpy(reinterpret_cast<char *>(val->controller_id), ctrlr_id.c_str(),
  strlen(ctrlr_id.c_str())+1);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrValue(val, oper));

  free(val);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_InvalidDesc) {
  VbrMoMgr vbrmomgr;
  uint32_t oper = UNC_OP_CREATE;
  val_vbr *val;
  GetValStruct(val);
  string desc = "vbr_description 1";
  strncpy(reinterpret_cast<char *>(val->vbr_description), desc.c_str(),
  strlen(desc.c_str())+1);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrValue(val, oper));

  free(val);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_DescValidAttrInvalid) {
  VbrMoMgr vbrmomgr;
  uint32_t oper = UNC_OP_UPDATE;
  val_vbr *val;
  GetValStruct(val);
  val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX , vbrmomgr.ValidateVbrValue(val, oper));

  free(val);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_PrefLenValidAttrInvalid) {
  VbrMoMgr vbrmomgr;
  uint32_t oper = UNC_OP_UPDATE;
  val_vbr *val;
  GetValStruct(val);
  val->host_addr_prefixlen = 0;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrValue(val, oper));
  val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrValue(val, oper));

  free(val);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_InvalidIP) {
  VbrMoMgr vbrmomgr;
  uint32_t oper = UNC_OP_CREATE;
  val_vbr *val;
  GetValStruct(val);
  val->host_addr.s_addr = 0xffffffffU;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbrmomgr.ValidateVbrValue(val, oper));

  free(val);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_invalidFlagNew) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_invalidFlag1) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag2) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag3) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag4) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag5) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag6) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_UPDATE;
  valvbr->valid[UPLL_IDX_PACKET_SIZE_PING] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag7) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag8) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag9) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_INVALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag10) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag11) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_VALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
}

TEST_F(VbrMoMgrTest, ValidateVbrValue_validFlag12) {
  VbrMoMgr vbr;
  val_vbr_t *valvbr(ZALLOC_TYPE(val_vbr_t));
  uint32_t op = UNC_OP_CREATE;
  valvbr->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_VALID;
  strcpy((char*)valvbr->vbr_description,(const char *)"vbr1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.ValidateVbrValue(valvbr,op));

  free(valvbr);
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

  delete ockv;
  delete ickv;
}

TEST_F(VbrMoMgrTest, GetParentConfigKeyInvalidArg) {
  VbrMoMgr vbrmomgr;
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

  delete ockv;
  delete ickv;
}

TEST_F(VbrMoMgrTest, GetParentConfigKeyNullKey) {
  VbrMoMgr vbrmomgr;
  ConfigKeyVal *ickv = new ConfigKeyVal(UNC_KT_VBRIDGE);
  ConfigKeyVal *ockv = new ConfigKeyVal(UNC_KT_VBRIDGE);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbrmomgr.GetParentConfigKey(ockv, ickv));

  delete ickv;
  delete ockv;
}

TEST_F(VbrMoMgrTest, AllocVal_outputNull) {
  VbrMoMgr obj;
  val_vbr *val(ZALLOC_TYPE(val_vbr));
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
  val_vbr *val;
  GetValStruct(val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(cfgval, UPLL_DT_STATE,RENAMETBL));
  delete cfgval;
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

  delete okey;
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

  delete okey;
  delete req;
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

  delete okey;
  delete req;
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_Req_InValid) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  val_vbr *val;
  GetValStruct(val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));

  delete req;
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_SuccessMAINTBL) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr, val);

  val_vbr *val1(UT_CLONE(val_vbr, val));
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum, val1);
  tmp->AppendCfgVal(tmp1);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,MAINTBL));

  delete req;
  delete okey;
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

  delete req;
  delete okey;
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

  delete req;
  delete okey;
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_NullValStuct) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  ConfigVal *tmp=NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,RENAMETBL));

  delete req;
  delete okey;
}

TEST_F(VbrMoMgrTest, DupConfigKeyVal_NullValStuctMainTbl) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  ConfigVal *tmp=NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                       IpctSt::kIpcStKeyVbr,
                                       key, tmp);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,MAINTBL));

  delete req;
  delete okey;
}

TEST_F(VbrMoMgrTest, IsValidKey_SuccessVTNName) {
  VbrMoMgr obj;
  uint64_t index = uudst::vbridge::kDbiVtnName;
  key_vbr *key;
  GetKeyStruct(key);
  EXPECT_EQ(true, obj.IsValidKey(reinterpret_cast<void *>(key), index));

  free(key);
}

TEST_F(VbrMoMgrTest, IsValidKey_SuccessVBRName) {
  VbrMoMgr obj;
  uint64_t index = uudst::vbridge::kDbiVbrName;
  key_vbr *key;
  GetKeyStruct(key);
  EXPECT_EQ(true, obj.IsValidKey(reinterpret_cast<void *>(key), index));

  free(key);
}

TEST_F(VbrMoMgrTest, IsValidKey_InvalidIndex) {
  VbrMoMgr obj;
  uint64_t index = uudst::vbridge::kDbiCtrlrName;
  key_vbr *key;
  GetKeyStruct(key);
  EXPECT_EQ(PFC_FALSE, obj.IsValidKey(reinterpret_cast<void *>(key), index));

  free(key);
}

TEST_F(VbrMoMgrTest, IsValidKey_InvalidVTNName) {
  VbrMoMgr obj;
  uint64_t index = uudst::vbridge::kDbiVtnName;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  const char *vtn_name = "";
  const char *vbr_name = "VBR_1";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
          vbr_name, strlen(vbr_name)+1);
  EXPECT_EQ(PFC_FALSE, obj.IsValidKey(reinterpret_cast<void *>(key), index));

  free(key);
}

TEST_F(VbrMoMgrTest, IsValidKey_InvalidVBRName) {
  VbrMoMgr obj;
  uint64_t index = uudst::vbridge::kDbiVbrName;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  const char *vtn_name = "VTN";
  const char *vbr_name = "";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbridge_name),
          vbr_name, strlen(vbr_name)+1);

  EXPECT_EQ(PFC_FALSE, obj.IsValidKey(reinterpret_cast<void *>(key), index));

  free(key);
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_SuccessNullObjs) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  delete okey;
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_pkeyNull) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));

  delete pkey;
  delete okey;
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_SuccesspkeyVBR) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vbr *key(ZALLOC_TYPE(key_vbr));
  strncpy((char*) key->vbridge_name,"VBR1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("VBR1",(reinterpret_cast<const char *> (output->vbridge_name)));

  delete okey;
  delete pkey;
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_SuccesspkeyVTN) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vbr *key(ZALLOC_TYPE(key_vbr));
  strncpy((char*) key->vbridge_name,"VBR1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("",(reinterpret_cast<const char *> (output->vbridge_name)));

  delete okey;
  delete pkey;
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_SuccessOkeyVTN) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vbr *key(ZALLOC_TYPE(key_vbr));
  strncpy((char*) key->vbridge_name,"VBR1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("",(reinterpret_cast<const char *> (output->vbridge_name)));

  delete okey;
  delete pkey;
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

  delete okey;
  delete ikey;
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_PkeyVbrSuccess) {
  VbrMoMgr vbr;
  ConfigKeyVal *okey = NULL;

  key_vbr *key(ZALLOC_TYPE(key_vbr));
  strncpy((char*) key->vbridge_name,"VBR1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("VBR1",(reinterpret_cast<const char *> (output->vbridge_name)));

  delete okey;
  delete pkey;
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_OkeyVtnSuccess) {
  VbrMoMgr vbr;

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN);
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  strncpy((char*) key->vbridge_name,"VBR1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("",(reinterpret_cast<const char *> (output->vbridge_name)));

  delete okey;
  delete pkey;
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_PkeyVlinkSuccess) {
  VbrMoMgr vbr;

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VLINK) ;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  strncpy((char*) key->vbridge_name,"VLINK1",32);
  strncpy((char*) key->vtn_key.vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVbr,
                                        key, NULL);

 EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));
  key_vbr_t *output = reinterpret_cast<key_vbr_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_key.vtn_name)));
  EXPECT_STREQ("",(reinterpret_cast<const char *> (output->vbridge_name)));

  delete okey;
  delete pkey;
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_05) {
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  VbrMoMgr vbr_obj;
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge");

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VLINK,IpctSt::kIpcStKeyVbr,key_vbr);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr_obj.GetChildConfigKey(okey, pkey));

  delete okey;
  delete pkey;
}

TEST_F(VbrMoMgrTest, GetChildConfigKey_06) {
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  VbrMoMgr vbr_obj;
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge");
  val_vlink *vlink_val(ZALLOC_TYPE(val_vlink));
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVlink, vlink_val);

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VLINK,IpctSt::kIpcStKeyVbr,key_vbr,cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr_obj.GetChildConfigKey(okey, pkey));

  delete okey;
  delete pkey;
}

TEST_F(VbrMoMgrTest, CopyToConfigkey_InValidName) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  const char *vtn_name = "";
  const char *vbr_name = "";
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

  delete okey;
  delete ikey;
}

TEST_F(VbrMoMgrTest, CopyToConfigkey_Valid) {
  VbrMoMgr obj;
  ConfigKeyVal *okey(NULL);
  key_rename_vnode_info *key_rename(ZALLOC_TYPE(key_rename_vnode_info));

  const char *vtn_name("VTN_1");
  const char *vbr_name("VBR_1");
  pfc_strlcpy(reinterpret_cast<char *>(key_rename->old_unc_vtn_name),
              vtn_name, sizeof(key_rename->old_unc_vtn_name));
  pfc_strlcpy(reinterpret_cast<char *>(key_rename->old_unc_vnode_name),
              vbr_name, sizeof(key_rename->old_unc_vnode_name));
  ConfigVal *cfgval(new ConfigVal(IpctSt::kIpcStValVbr, NULL));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                      key_rename, cfgval));
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));

  ASSERT_TRUE(okey != NULL);

  key_vbr_t *key(reinterpret_cast<key_vbr_t *>(okey->get_key()));
  if (key != NULL) {
    EXPECT_STREQ(vtn_name, reinterpret_cast<char *>(key->vtn_key.vtn_name));
    EXPECT_STREQ(vbr_name, reinterpret_cast<char *>(key->vbridge_name));
  }

  delete ikey;
  delete okey;
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_Success) {
  VbrMoMgr obj;
  val_ping *vst(ZALLOC_TYPE(val_ping));
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

  free(vst);
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_InvalidTgtAddr) {
  VbrMoMgr obj;
  val_ping *vst(ZALLOC_TYPE(val_ping));
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

  free(vst);
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_InvalidSrcAddr) {
  VbrMoMgr obj;
  val_ping *vst(ZALLOC_TYPE(val_ping));
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

  free(vst);
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_DFbitValidation) {
  VbrMoMgr obj;
  val_ping *vst(ZALLOC_TYPE(val_ping));
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

  free(vst);
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_PktSzValidation) {
  VbrMoMgr obj;
  val_ping *vst(ZALLOC_TYPE(val_ping));
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

  free(vst);
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_CntValidation) {
  VbrMoMgr obj;
  val_ping *vst(ZALLOC_TYPE(val_ping));
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

  free(vst);
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_InterValidation) {
  VbrMoMgr obj;
  val_ping *vst(ZALLOC_TYPE(val_ping));
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

  free(vst);
}

TEST_F(VbrMoMgrTest, ValidateVbrPingValue_TimeOutValidation) {
  VbrMoMgr obj;
  val_ping *vst(ZALLOC_TYPE(val_ping));
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

  free(vst);
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

  key_vbr *key1(UT_CLONE(key_vbr, key));
  val_vbr *val1(UT_CLONE(val_vbr, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbr, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

  delete ikey;
  delete upd_key;
}

TEST_F(VbrMoMgrTest, UpdateConfigStatus_SuccessUPDATE) {
  VbrMoMgr obj;
  DalDmlIntf *dmi= NULL;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  key_vbr *key1(UT_CLONE(key_vbr, key));
  val_vbr *val1(UT_CLONE(val_vbr, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbr, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

  delete ikey;
  delete upd_key;
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

  key_vbr *key1(UT_CLONE(key_vbr, key));
  val_vbr *val1(UT_CLONE(val_vbr, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbr, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_READ,
                                                       UPLL_RC_SUCCESS,
                                                       upd_key, dmi, ikey));

  delete ikey;
  delete upd_key;
}

TEST_F(VbrMoMgrTest, UpdateConfigStatus_InvalidArg) {
  VbrMoMgr obj;
  DalDmlIntf *dmi= NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            NULL, NULL);
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                         NULL, NULL));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                       UPLL_RC_SUCCESS,
                                                       upd_key, dmi, ikey));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                       UPLL_RC_SUCCESS,
                                                       upd_key, dmi, ikey));

  delete ikey;
  delete upd_key;
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

  key_vbr *key1(UT_CLONE(key_vbr, key));
  val_vbr *val1(UT_CLONE(val_vbr, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbr, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

  delete ikey;
  delete upd_key;
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

  delete ikey;
  delete okey;
}

TEST_F(VbrMoMgrTest, CreateVnodeConfigKey_NULLArg) {
  VbrMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateVnodeConfigKey(ikey, okey));
}

TEST_F(VbrMoMgrTest, CompareValidValue_AuditTrue) {
  VbrMoMgr obj;
  val_vbr val1, val2;
  memset(&val1, 0, sizeof(val1));
  memset(&val2, 0, sizeof(val2));

  for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(val1.valid); i++) {
    val1.valid[i] = UNC_VF_INVALID;
    val2.valid[i] = UNC_VF_VALID;
  }
  void *v1(&val1);
  void *v2(&val2);

  ASSERT_FALSE(obj.CompareValidValue(v1, v2, true));

  for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(val1.valid); i++) {
    ASSERT_EQ(UNC_VF_VALID_NO_VALUE, val1.valid[i]);
  }
}

TEST_F(VbrMoMgrTest, FilterAttributes_CreateOperation) {
  VbrMoMgr vbr;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vbr_t *valvbr1(ZALLOC_TYPE(val_vbr_t));
  valvbr1->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  EXPECT_EQ(PFC_FALSE, vbr.FilterAttributes(val1,val2,audit_status,op));

  free(valvbr1);
}

TEST_F(VbrMoMgrTest, FilterAttributes_OperationUpdate) {
  VbrMoMgr vbr;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_UPDATE;

  val_vbr_t *valvbr1(ZALLOC_TYPE(val_vbr_t));
  valvbr1->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  EXPECT_EQ(true, vbr.FilterAttributes(val1,val2,audit_status,op));

  free(valvbr1);
}

TEST_F(VbrMoMgrTest, GetRenameKeyBindInfo) {
  VbrMoMgr obj;
  BindInfo *binfo;
  int nattr;

  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(UNC_KT_VBRIDGE, binfo, nattr, MAINTBL));

  EXPECT_EQ(5, nattr);
  EXPECT_EQ(&VbrMoMgr::key_vbr_maintbl_bind_info[0], binfo);


  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(UNC_KT_VBRIDGE, binfo, nattr, RENAMETBL));

  EXPECT_EQ(4, nattr);
  EXPECT_EQ(&VbrMoMgr::key_vbr_renametbl_update_bind_info[0], binfo);

}

TEST_F(VbrMoMgrTest, GetRenameKeyBindInfo_OutputUnknownTbl) {
  VbrMoMgr vbr;
  unc_key_type_t key_type = UNC_KT_VBRIDGE;
  BindInfo *binfo = NULL;
  int nattr = 2;

  EXPECT_EQ(PFC_FALSE, vbr.GetRenameKeyBindInfo(key_type, binfo, nattr,CTRLRTBL ));
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

  val_vbr *val1(UT_CLONE(val_vbr, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbr, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       NULL, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetVnodeName(ikey1, vtn_name, vnode_name));

  delete ikey;
  delete ikey1;
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
  IPC_REQ_RESP_HEADER_DECL(req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(req, ikey, NULL));

  delete ikey;
}
/*
TEST_F(VbrMoMgrTest, ValidateCapability_Success) {
  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  CtrlrMgr::Ctrlr ctrlrobj("CTR_1", UNC_CT_PFC, "5.0");
  CtrlrMgr::GetInstance()->Add(ctrlrobj, UPLL_DT_CANDIDATE);

  IPC_REQ_RESP_HEADER_DECL(req);
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
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValidateCapability(req, ikey, "CTR_1"));

  delete ikey;
}
TEST_F(VbrMoMgrTest, ValidateCapability_Success1) {
  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);


  CtrlrMgr::Ctrlr ctrlrobj("CTR_1", UNC_CT_PFC, "5.0");
  CtrlrMgr::GetInstance()->Add(ctrlrobj, UPLL_DT_CANDIDATE);

  IPC_REQ_RESP_HEADER_DECL(req);
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

  key_vbr *key1(UT_CLONE(key_vbr, key));
  val_vbr* no_val(NULL);
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbr, no_val));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValidateCapability(req, ikey1, "CTR_1"));

  req->operation = UNC_OP_DELETE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey1, "CTR_1"));

  CtrlrMgr::GetInstance()->Delete("CTR_1", UPLL_DT_CANDIDATE);

  delete ikey;
  delete ikey1;
}
*/

TEST_F(VbrMoMgrTest, ValidateCapability_ikey_NULL) {
  VbrMoMgr vbr;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req=NULL;
  const char *ctrlr_name="ctr1";
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, ctrlr_name));
}

TEST_F(VbrMoMgrTest, ValidateCapability_ctrName_NULL) {
  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IPC_REQ_RESP_HEADER_DECL(req);
  const char *ctrlr_name=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, ctrlr_name));

  delete ikey;
}

TEST_F(VbrMoMgrTest, ValidateCapability_ctrName) {
  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  IPC_REQ_RESP_HEADER_DECL(req);
  const char *ctrlr_name=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, ctrlr_name));

  delete ikey;
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

  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ikey));

  val_rename_vbr *renameval(ZALLOC_TYPE(val_rename_vbr));
  for(unsigned int loop = 0; loop < sizeof(renameval->valid)/
     sizeof(renameval->valid[0]); ++loop) {
    renameval->valid[loop] = UNC_VF_VALID;
  }
  strncpy(reinterpret_cast<char *>(renameval->new_name),
  "renamed", strlen("renamed")+1);

  key_vbr *key1(UT_CLONE(key_vbr, key));
  ConfigVal *rename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, renameval);
  ConfigKeyVal *rename_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key1, rename_cfgval);

  req->operation = UNC_OP_RENAME;
  req->datatype = UPLL_DT_IMPORT;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ValidateMessage(req, rename_ikey));

  req->operation = UNC_OP_READ_SIBLING;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, rename_ikey));

  ConfigVal *invrename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, NULL);
  key_vbr *key2(UT_CLONE(key_vbr, key));
  ConfigKeyVal *invrename_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key2, invrename_cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, invrename_ikey));

  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_RUNNING;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ikey));

  req->operation = UNC_OP_DELETE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ikey));

  req->operation = UNC_OP_CONTROL;
  val_ping *vst(ZALLOC_TYPE(val_ping));
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

  key_vbr *key3(UT_CLONE(key_vbr, key));
  ConfigVal *ping_cfgval = new ConfigVal(IpctSt::kIpcStValPing, vst);
  ConfigKeyVal *ping_ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key3, ping_cfgval);
  req->option2 = UNC_OPT2_PING;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ping_ikey));

  delete ikey;
  delete rename_ikey;
  delete invrename_ikey;
  delete ping_ikey;
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
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ikey));

  delete ikey;
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
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));

  delete ikey;
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
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_L2DOMAIN;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));

  delete ikey;
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
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));

  delete ikey;
}

TEST_F(VbrMoMgrTest, ValidateMessage_NullVal) {
  VbrMoMgr obj;
  key_vbr *key;
  GetKeyStruct(key);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));

  delete ikey;
}

TEST_F(VbrMoMgrTest, ValidateMessage_DiffOption) {
  VbrMoMgr obj;
  key_vbr *key;
  GetKeyStruct(key);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_STATE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));

  delete ikey;
}

TEST_F(VbrMoMgrTest, ValidateMessage_DiffOption2) {
  VbrMoMgr obj;
  key_vbr *key;
  GetKeyStruct(key);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_IP_ROUTE;
  req->datatype = UPLL_DT_STATE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));

  delete ikey;
}

TEST_F(VbrMoMgrTest, ValidateMessage_ValidDiffOption) {
  VbrMoMgr obj;
  key_vbr *key;
  GetKeyStruct(key);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  IPC_REQ_RESP_HEADER_DECL(req);
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

  delete ikey;
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

  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;


  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateMessage(req, invalkey));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateMessage(inreq, ikey));

  key_vbr *key1(UT_CLONE(key_vbr, key));
  val_vbr *val1(UT_CLONE(val_vbr, val));
  ConfigVal *inval_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, val1);
  ConfigKeyVal *invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key1, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  key_vbr *key2(UT_CLONE(key_vbr, key));
  val_vbr *val2(UT_CLONE(val_vbr, val));
  inval_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, val2);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyLogicalPort,
                                 key2, inval_cfgval);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  delete invalcfgkey;
  key2 = UT_CLONE(key_vbr, key);
  val2 = UT_CLONE(val_vbr, val);
  inval_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, val2);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVbr,
                                 key2, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  req->datatype = UPLL_DT_AUDIT;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,obj.ValidateMessage(req, ikey));

  req->datatype = UPLL_DT_CANDIDATE;

  delete invalcfgkey;
  key2 = UT_CLONE(key_vbr, key);
  key2->vtn_key.vtn_name[0] = '\0';
  val2 = UT_CLONE(val_vbr, val);
  inval_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVbr, val2);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                 key2, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, invalcfgkey));

  delete invalcfgkey;
  key2 = UT_CLONE(key_vbr, key);
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          "VTN_1", strlen("VTN_1")+1);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                 key2, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ValidateMessage(req, invalcfgkey));

  delete invalcfgkey;
  key2 = UT_CLONE(key_vbr, key);
  inval_cfgval = new ConfigVal(IpctSt::kIpcStValVbr, NULL);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                 key2, inval_cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ValidateMessage(req, invalcfgkey));

  delete invalcfgkey;
  key2 = UT_CLONE(key_vbr, key);
  val2 = UT_CLONE(val_vbr, val);
  pfc_strlcpy(reinterpret_cast<char *>(val2->controller_id), "Controller 1",
              sizeof(val2->controller_id));
  inval_cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val2);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                 key2, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, invalcfgkey));

  delete ikey;
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
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_RENAME;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_IMPORT;

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));

  key_vbr *key1(UT_CLONE(key_vbr, key));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValRenameVbr, NULL));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey1));

  key_vbr *key2(UT_CLONE(key_vbr, key));
  val_rename_vbr *rval2(ZALLOC_TYPE(val_rename_vbr));
  for(unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(rval2->valid); ++loop){
    rval2->valid[loop] = UNC_VF_VALID;
  }
  ConfigVal *cfgval2(new ConfigVal(IpctSt::kIpcStValRenameVbr, rval2));
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key2, cfgval2));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey2));

  key_vbr *key3(UT_CLONE(key_vbr, key));
  val_rename_vbr *rval3(UT_CLONE(val_rename_vbr, rval2));
  pfc_strlcpy(reinterpret_cast<char *>(rval3->new_name), "renamed",
              sizeof(rval3->new_name));
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValRenameVbr, rval3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key3, cfgval3));

  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey3));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_PING;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey3));

  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));

  key_vbr *key4(UT_CLONE(key_vbr, key));
  val_rename_vbr *rval4(UT_CLONE(val_rename_vbr, rval2));
  ConfigVal *cfgval4(new ConfigVal(IpctSt::kIpcStValRenameVbr, rval4));
  ConfigKeyVal *ikey4(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key4, cfgval4));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey4));

  key_vbr *key5(UT_CLONE(key_vbr, key));
  val_rename_vbr *rval5(UT_CLONE(val_rename_vbr, rval3));
  ConfigVal *cfgval5(new ConfigVal(IpctSt::kIpcStValRenameVbr, rval5));
  ConfigKeyVal *ikey5(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key5, cfgval5));
  req->operation = UNC_OP_READ_BULK;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey5));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
  delete ikey4;
  delete ikey5;
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
  IPC_REQ_RESP_HEADER_DECL(req);
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

  key_vbr *key1(UT_CLONE(key_vbr, key));
  val_rename_vbr *val1(ZALLOC_TYPE(val_rename_vbr));
  req->option2 = UNC_OPT2_NONE;
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValRenameVbr, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.ValidateMessage(req, ikey1));

  key_vbr *key2(UT_CLONE(key_vbr, key));
  ConfigVal *cfgval2(new ConfigVal(IpctSt::kIpcStValVbr, NULL));
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key2, cfgval2));
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey2));

  key_vbr *key3(UT_CLONE(key_vbr, key));
  val_vbr *val3(UT_CLONE(val_vbr, val));
  pfc_strlcpy(reinterpret_cast<char *>(val3->controller_id), "Controller 1",
              sizeof(val3->controller_id));
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValVbr, val3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey3));

  req->datatype = UPLL_DT_AUDIT;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,
            obj.ValidateMessage(req, ikey3));

  req->operation = UNC_OP_DELETE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateMessage(req, ikey3));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
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
  IPC_REQ_RESP_HEADER_DECL(req);
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

  key_vbr *key1(UT_CLONE(key_vbr, key));
  val_ping *val1(ZALLOC_TYPE(val_ping));
  for(unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(val1->valid); ++loop){
    val1->valid[loop] = UNC_VF_VALID;
  }

  struct sockaddr_in sa;
  inet_pton(AF_INET, "255.255.255.255", &(sa.sin_addr));
  val1->target_addr = sa.sin_addr.s_addr;
  inet_pton(AF_INET, "192.168.1.2", &(sa.sin_addr));
  val1->src_addr = sa.sin_addr.s_addr; // ("192.168.1.2")
  val1->dfbit = UPLL_DF_BIT_ENABLE;
  val1->packet_size = 5;
  val1->count = 14;
  val1->interval = 23;
  val1->timeout = 32;
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValPing, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey1));

  key_vbr *key2(UT_CLONE(key_vbr, key));
  val_ping *val2(UT_CLONE(val_ping, val1));
  inet_pton(AF_INET, "192.168.1.1", &(sa.sin_addr));
  val2->target_addr = sa.sin_addr.s_addr;// ("192.168.1.1")

  ConfigVal *cfgval2(new ConfigVal(IpctSt::kIpcStValPing, val2));
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key2, cfgval2));

  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,
            obj.ValidateMessage(req, ikey2));

  req->operation = UNC_OP_INVALID;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,
            obj.ValidateMessage(req, ikey2));

  delete ikey;
  delete ikey1;
  delete ikey2;
}

TEST_F(VbrMoMgrTest, AdaptValToVtnService_Success) {
  VbrMoMgr obj;
  key_vbr *key;
  GetKeyStruct(key);

  val_vbr_st *valst(ZALLOC_TYPE(val_vbr_st));
  valst->valid[0] = UNC_VF_VALID;
  valst->oper_status = UPLL_OPER_STATUS_UP;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbrSt, valst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfg_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey, ADAPT_ONE));
  delete ikey;
}

TEST_F(VbrMoMgrTest, AdaptValToVtnService_Failure) {
  VbrMoMgr obj;
  key_vbr *key;
  GetKeyStruct(key);

  ConfigKeyVal* ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey, ADAPT_ONE));

  ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey, ADAPT_ONE));
  delete ikey;
}

TEST_F(VbrMoMgrTest, GetValid) {
  VbrMoMgr obj;
  val_vbr *val;
  uint8_t *valid(NULL);
  GetValStruct(val);

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

  val_vbr_st *valst(ZALLOC_TYPE(val_vbr_st));
  valst->valid[0] = UNC_VF_VALID;
  valst->oper_status = UPLL_OPER_STATUS_UP;

  in_val = reinterpret_cast<void *>(valst);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val, 0, valid, UPLL_DT_STATE, RENAMETBL));
  EXPECT_EQ(val->valid[UPLL_IDX_OPER_STATUS_VBRS], valid[UPLL_IDX_OPER_STATUS_VBRS]);

  free(val);
  free(valst);
}

TEST_F(VbrMoMgrTest, SwapKeyVal_IpctSt_valid) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  VbrMoMgr vbr;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vbr_name = "VBR_1";
  bool no_rename;
  key_vbr_t *key(ZALLOC_TYPE(key_vbr_t));
  pfc_strlcpy(reinterpret_cast<char *>(key->vbridge_name),
              vbr_name, sizeof(key->vbridge_name));
  val_rename_vbr_t *val(ZALLOC_TYPE(val_rename_vbr_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vbr.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  delete okey;

  val->valid[UPLL_IDX_NEW_NAME_RVBR] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS,
            vbr.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  delete okey;

  key_vbr_t *key1(UT_CLONE(key_vbr_t, key));
  val_rename_vbr_t *val1(UT_CLONE(val_rename_vbr_t, val));
  ConfigVal *config_val1(new ConfigVal(IpctSt::kIpcStValVbrSt, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVbr,
                                       key1, config_val1));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,
            vbr.SwapKeyVal(ikey1, okey, dmi, ctr_id1, no_rename));
  delete okey;

  key_vbr_t *key2(UT_CLONE(key_vbr_t, key));
  ConfigVal *config_val2(NULL);
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key2, config_val2));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vbr.SwapKeyVal(ikey2, okey, dmi, ctr_id1, no_rename));
  delete okey;

  key_vbr_t *key3(UT_CLONE(key_vbr_t, key));
  val_rename_vbr_t *val3(NULL);
  ConfigVal *config_val3(new ConfigVal(IpctSt::kIpcStValVbrSt, val3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key3, config_val3));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vbr.SwapKeyVal(ikey3, okey, dmi, ctr_id1, no_rename));
  delete okey;

  delete ikey;
  delete ikey1;
  delete ikey2;

  free(ctr_id1);
}

TEST_F(VbrMoMgrTest, UpdateAuditConfigStatus_ValidCsStatus) {
  VbrMoMgr vbrmomgr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, val);
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  DalDmlIntf *dmi(getDalDmlIntf());
  UpdateCtrlrPhase phase = kUpllUcpCreate;
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, (void*)key, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));

  delete ckv_running;
}

TEST_F(VbrMoMgrTest, UpdateAuditConfigStatus_InvalidCsStatus) {
  VbrMoMgr vbrmomgr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, val);
  unc_keytype_configstatus_t cs_status = UNC_CS_INVALID;
  UpdateCtrlrPhase phase = kUpllUcpCreate;
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, (void*)key, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));

  delete ckv_running;
}

TEST_F(VbrMoMgrTest, UpdateAuditConfigStatus_EmptyVal) {
  VbrMoMgr vbrmomgr;
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  UpdateCtrlrPhase phase = kUpllUcpCreate;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));

  delete ckv_running;
}

TEST_F(VbrMoMgrTest, GetRenamedUncKey) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbrmomgr;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  cout << "ctr_id1:" << ctr_id1<<endl;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, (void*)key, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.GetRenamedUncKey(ikey,dt_type,dmi,ctr_id1));

  delete ikey;
  free(ctr_id1);
}

TEST_F(VbrMoMgrTest, GetRenamedControllerKey_01) {
  ConfigKeyVal *ikey = NULL;
  VbrMoMgr vbr;
  controller_domain *ctrl_domain = NULL;
  DalDmlIntf *dmi = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.GetRenamedControllerKey(ikey,UPLL_DT_STATE,dmi,ctrl_domain));
}

TEST_F(VbrMoMgrTest, GetRenamedControllerKey_02) {
  VbrMoMgr vbr;
  controller_domain ctrl_domain;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,key_vbr);

  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_SUCCESS,
            vbr.GetRenamedControllerKey(ikey, UPLL_DT_CANDIDATE, dmi,
                                        &ctrl_domain));

  delete ikey;
}

TEST_F(VbrMoMgrTest, GetRenamedControllerKey_03) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbrmomgr;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  cout << "ctr_id1:" << ctr_id1<<endl;
  uint8_t *dom_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(dom_id1, "Domain1", 7);
  cout << "dom_id1:" << dom_id1<<endl;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, (void*)key, cfg_val);
  controller_domain ctrlr_dom1;
  ctrlr_dom1.ctrlr = ctr_id1;
  ctrlr_dom1.domain = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.GetRenamedControllerKey(ikey,dt_type,dmi,&ctrlr_dom1));

  delete ikey;
  free(ctr_id1);
  free(dom_id1);
}

TEST_F(VbrMoMgrTest, GetRenamedControllerKey_04) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbr;
  key_vbr_t *keyvbr(ZALLOC_TYPE(key_vbr_t));
  val_vbr_t *valVbr(ZALLOC_TYPE(val_vbr_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, valVbr);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, keyvbr, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  cout << "ctr_id1:" << ctr_id1<<endl;
  uint8_t *dom_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(dom_id1, "Domain1", 7);
  cout << "dom_id1:" << dom_id1<<endl;
  controller_domain ctrlr_dom1;
  ctrlr_dom1.ctrlr = ctr_id1;
  ctrlr_dom1.domain = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetRenamedControllerKey(ikey,dt_type,dmi,&ctrlr_dom1));

  delete ikey;
  free(ctr_id1);
  free(dom_id1);
}

TEST_F(VbrMoMgrTest, GetControllerDomainId1) {
  VbrMoMgr vbr;
  controller_domain ctrl_domain;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,key_vbr);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetControllerDomainId(ikey, &ctrl_domain));

  delete ikey;
}

TEST_F(VbrMoMgrTest, GetControllerDomainId2) {
  VbrMoMgr vbr;
  controller_domain ctrl_domain;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetControllerDomainId(ikey, &ctrl_domain));
}

TEST_F(VbrMoMgrTest, GetControllerDomainId3) {
  VbrMoMgr vbr;
  controller_domain *ctrl_domain = NULL;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.GetControllerDomainId(ikey,ctrl_domain));

  delete ikey;
}

TEST_F(VbrMoMgrTest, GetControllerDomainId4) {
  VbrMoMgr vbr;
  controller_domain ctrl_domain;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key_vbr, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetControllerDomainId(ikey, &ctrl_domain));

  delete ikey;
}

TEST_F(VbrMoMgrTest, GetControllerDomainId5) {
  VbrMoMgr vbr;
  controller_domain ctrl_domain;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetControllerDomainId(ikey, &ctrl_domain));

  delete ikey;
}

TEST_F(VbrMoMgrTest, GetControllerDomainId_InvalidVal){
  VbrMoMgr vbr;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  val_vbr *val(ZALLOC_TYPE(val_vbr));
  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;
  pfc_strlcpy(reinterpret_cast<char *>(val->controller_id), "pfc1",
              sizeof(val->controller_id));
  pfc_strlcpy(reinterpret_cast<char *>(val->domain_id), "dom1",
              sizeof(val->domain_id));
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(ctrlr_dom));
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetControllerDomainId(ikey, &ctrlr_dom));

  delete ikey;
}

TEST_F(VbrMoMgrTest, GetControllerDomainId_SetDomainData){
  VbrMoMgr vbr;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  val_vbr *val(ZALLOC_TYPE(val_vbr));
  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;
  pfc_strlcpy(reinterpret_cast<char *>(val->controller_id), "pfc1",
              sizeof(val->controller_id));
  pfc_strlcpy(reinterpret_cast<char *>(val->domain_id), "dom1",
              sizeof(val->domain_id));
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(ctrlr_dom));

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  SET_USER_DATA_CTRLR(ikey,val->controller_id)
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetControllerDomainId(ikey, &ctrlr_dom));

  delete ikey;
}

TEST_F(VbrMoMgrTest, GetRenameInfo1) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbr;
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(NULL, okey, rename_info, NULL, NULL, no_rename));
}

TEST_F(VbrMoMgrTest, GetRenameInfo2) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbr;
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, NULL, NULL, no_rename));

  delete ikey;
  delete rename_info;
}
#if 0
TEST_F(VbrMoMgrTest, GetRenameInfo3) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbr;
  bool no_rename = false;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"name");
  val_rename_vnode_t *val(ZALLOC_TYPE(val_rename_vnode_t));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1)");
  strcpy((char*)val->ctrlr_vnode_name, "vnode1");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);

  key_vbr_t *key_vbr1(UT_CLONE(key_vbr_t, key_vbr));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,
                                      key_vbr1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));

  delete ikey;
  delete okey;
  delete rename_info;
}
#endif

TEST_F(VbrMoMgrTest, GetRenameInfo4) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbr;
  bool no_rename = true;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"name");
  val_rename_vnode_t *val(ZALLOC_TYPE(val_rename_vnode_t));
  strcpy((char*)val->ctrlr_vtn_name,"");
  strcpy((char*)val->ctrlr_vnode_name, "vnode1");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);

  key_vbr_t *key_vbr1(UT_CLONE(key_vbr_t, key_vbr));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,
                                      key_vbr1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));

  delete ikey;
  delete okey;
  delete rename_info;
}

TEST_F(VbrMoMgrTest, GetRenameInfo5) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbr;
  bool no_rename = true;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"name");
  val_rename_vnode_t *val(ZALLOC_TYPE(val_rename_vnode_t));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1");
  strcpy((char*)val->ctrlr_vnode_name, "");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);

  key_vbr_t *key_vbr1(UT_CLONE(key_vbr_t, key_vbr));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,
                                      key_vbr1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));

  delete ikey;
  delete okey;
  delete rename_info;
}

TEST_F(VbrMoMgrTest, GetRenameInfo6) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbr;
  bool no_rename = false;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"");
  val_rename_vnode_t *val(ZALLOC_TYPE(val_rename_vnode_t));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1");
  strcpy((char*)val->ctrlr_vnode_name, "");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);

  key_vbr_t *key_vbr1(UT_CLONE(key_vbr_t, key_vbr));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,
                                      key_vbr1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));

  delete ikey;
  delete okey;
  delete rename_info;
}

TEST_F(VbrMoMgrTest, GetRenameInfo7) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbr;
  bool no_rename = false;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"");
  strcpy((char *)key_vbr->vbridge_name, (char *)"name");
  val_rename_vnode_t *val(ZALLOC_TYPE(val_rename_vnode_t));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1");
  strcpy((char*)val->ctrlr_vnode_name, "");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);

  key_vbr_t *key_vbr1(UT_CLONE(key_vbr_t, key_vbr));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,
                                      key_vbr1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));

  delete ikey;
  delete okey;
  delete rename_info;
}

TEST_F(VbrMoMgrTest, GetRenameInfo8) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbr;
  bool no_rename = true;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"");
  strcpy((char *)key_vbr->vbridge_name, (char *)"name");
  val_rename_vnode_t *val(ZALLOC_TYPE(val_rename_vnode_t));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1");
  strcpy((char*)val->ctrlr_vnode_name, "");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);

  key_vbr_t *key_vbr1(UT_CLONE(key_vbr_t, key_vbr));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,
                                      key_vbr1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));

  delete ikey;
  delete okey;
  delete rename_info;
}

TEST_F(VbrMoMgrTest, GetRenameInfo9) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);

  VbrMoMgr vbr;
  bool no_rename = true;
  ConfigKeyVal *rename_info = NULL;
  const char * ctrlr_name = "ctrlr1";
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1`");
  strcpy((char *)key_vbr->vbridge_name, (char *)"");
  val_rename_vnode_t *val(ZALLOC_TYPE(val_rename_vnode_t));
  strcpy((char*)val->ctrlr_vtn_name,"vtn1");
  strcpy((char*)val->ctrlr_vnode_name, "");
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, key_vbr, config_val);

  key_vbr_t *key_vbr1(UT_CLONE(key_vbr_t, key_vbr));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,
                                      key_vbr1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetRenameInfo(ikey, okey, rename_info, dmi, ctrlr_name, no_rename));

  delete ikey;
  delete okey;
  delete rename_info;
}

TEST_F(VbrMoMgrTest, IsReferenced1) {
  VbrMoMgr vbr;
  IpcReqRespHeader *req = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                        IpctSt::kIpcStKeyVbr,
                                        NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.IsReferenced(req, ikey, dmi));

  delete ikey;
}

/*
TEST_F(VbrMoMgrTest, IsReferenced2) {
  VbrMoMgr vbr;
  IpcReqRespHeader *req = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                        IpctSt::kIpcStKeyVbr,
                                        key_vbr, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.IsReferenced(req, ikey, dmi));

  delete ikey;
}
*/
TEST_F(VbrMoMgrTest, UpdateAuditConfigStatus1) {
  VbrMoMgr vbr;
  ConfigKeyVal *ikey =NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey, dmi));
}

TEST_F(VbrMoMgrTest, UpdateAuditConfigStatus2) {
  VbrMoMgr vbr;
  DalDmlIntf *dmi(getDalDmlIntf());
  val_vbr_t *val(ZALLOC_TYPE(val_vbr_t));
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.UpdateAuditConfigStatus(UNC_CS_INVALID,uuc::kUpllUcpCreate, ikey, dmi));

  delete ikey;
}

TEST_F(VbrMoMgrTest, UpdateAuditConfigStatus3) {
  VbrMoMgr vbr;
  val_vbr_t *val(ZALLOC_TYPE(val_vbr_t));
  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
  val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  //val->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT] = UNC_VF_VALID;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr, val);
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr, NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.UpdateAuditConfigStatus(UNC_CS_INVALID,uuc::kUpllUcpCreate, ikey, dmi));
  val_vbr_t *output = reinterpret_cast<val_vbr_t *> (GetVal(ikey));
  EXPECT_EQ(UNC_CS_INVALID,output->cs_attr[0]);

  delete ikey;
}

TEST_F(VbrMoMgrTest, SwapKeyVal1) {
  VbrMoMgr vbr;
  ConfigKeyVal *key = NULL;
  bool no_rename = false;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.SwapKeyVal(NULL, key, NULL, NULL, no_rename));
}

TEST_F(VbrMoMgrTest, SwapKeyVal2) {
  VbrMoMgr vbr;
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge1");

  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVbr, key_vbr);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.SwapKeyVal(ikey, okey, NULL, NULL, no_rename));

  delete ikey;
  delete okey;
}

TEST_F(VbrMoMgrTest, SwapKeyVal3) {
  VbrMoMgr vbr;
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge1");

  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  val_rename_vbr *vbr_rename_val = NULL;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValRenameVbr, vbr_rename_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key_vbr, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.SwapKeyVal(ikey, okey, NULL, NULL, no_rename));

  delete ikey;
  delete okey;
}

TEST_F(VbrMoMgrTest, IsHostAddrAndPrefixLenInUse1){
  VbrMoMgr vbr;
  DalDmlIntf *dmi= NULL;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  val_vbr *val(ZALLOC_TYPE(val_vbr));
  IPC_REQ_RESP_HEADER_DECL(req);

  pfc_strlcpy(reinterpret_cast<char *>(key->vtn_key.vtn_name), "vtn_name1",
              sizeof(key->vtn_key.vtn_name));
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,key, cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.IsHostAddrAndPrefixLenInUse(ckv,dmi,req));

  delete ckv;
}

TEST_F(VbrMoMgrTest, SwapKeyVal7) {
  VbrMoMgr vbr;
  key_vbr_t *key_vbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char *)key_vbr->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key_vbr->vbridge_name, (char *)"vbridge1");
  const char * ctrlr_name = "ctrlr1";
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());

  val_rename_vbr *vbr_rename_val(ZALLOC_TYPE(val_rename_vbr));
  strcpy((char *)vbr_rename_val->new_name, (char*)"hhh");
  vbr_rename_val->valid[UPLL_IDX_NEW_NAME_RVRT] =  UNC_VF_VALID;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValRenameVbr, vbr_rename_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVrt, key_vbr, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.SwapKeyVal(ikey, okey, dmi, (uint8_t *)ctrlr_name, no_rename));

  delete ikey;
}

TEST_F(VbrMoMgrTest, IsHostAddrAndPrefixLenInUse2){
  VbrMoMgr vbr;
  DalDmlIntf *dmi= NULL;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  strcpy((char *)key->vtn_key.vtn_name, (char *)"vtn1");
  strcpy((char *)key->vbridge_name, (char *)"vbridge1");

  val_vbr_t *val(ZALLOC_TYPE(val_vbr_t));
  val->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_INVALID;
  val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_INVALID;

  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,key, cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.IsHostAddrAndPrefixLenInUse(ckv,dmi,req));

  delete ckv;
}

TEST_F(VbrMoMgrTest, CtrlrIdAndDomainIdUpdationCheck2){
  VbrMoMgr vbr;
  key_vbr *key(ZALLOC_TYPE(key_vbr));
  val_vbr *val(ZALLOC_TYPE(val_vbr));

  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  key_vbr *key1(UT_CLONE(key_vbr, key));
  val_vbr *val1(ZALLOC_TYPE(val_vbr));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbr, val1));
  ConfigKeyVal *ckey(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                      key1, cfgval1));
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.CtrlrIdAndDomainIdUpdationCheck(ikey, ckey));

  delete ikey;
  delete ckey;
}

TEST_F(VbrMoMgrTest, CtrlrIdAndDomainIdUpdationCheck3){
  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;
  GetKeyValStruct(key, val);

  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);

  key_vbr *key1(UT_CLONE(key_vbr, key));
  val_vbr *val1(ZALLOC_TYPE(val_vbr));
  pfc_strlcpy(reinterpret_cast<char *>(val1->domain_id), "dom1",
              sizeof(val1->domain_id));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbr, val1));
  ConfigKeyVal *ckey(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                      key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,
            vbr.CtrlrIdAndDomainIdUpdationCheck(ikey, ckey));

  delete ikey;
  delete ckey;
}

TEST_F(VbrMoMgrTest, ValVbrAttributeSupportCheck_01) {
  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[8];
  memset(attrs, 0xff, sizeof(attrs));
  attrs[unc::capa::vbr::kCapDomainId] = 0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}

TEST_F(VbrMoMgrTest, ValVbrAttributeSupportCheck_02) {
  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[8];
  memset(attrs, 0xff, sizeof(attrs));
  attrs[unc::capa::vbr::kCapDomainId] = 0;
  
  val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}

TEST_F(VbrMoMgrTest, ValVbrAttributeSupportCheck_03) {
  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[8];
  memset(attrs, 0xff, sizeof(attrs));
  attrs[unc::capa::vbr::kCapDesc] = 0;
  val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}

TEST_F(VbrMoMgrTest, ValVbrAttributeSupportCheck_04) {
  VbrMoMgr vbr;
  key_vbr *key;
  val_vbr *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation(UNC_OP_READ);
  val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  uint8_t attrs[8];
  memset(attrs, 0xff, sizeof(attrs));
  attrs[unc::capa::vbr::kCapDesc] = 0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValVbrAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}
