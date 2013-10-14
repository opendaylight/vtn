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
#include "vtn_momgr.hh"
#include "vbr_momgr.hh"
#include "vbr_if_momgr.hh"
#include "unc/keytype.h"
#include "config_mgr.hh"
#include "dal_odbc_mgr.hh"
#include "dal_dml_intf.hh"
#include "capa_intf.hh"
#include "capa_module_stub.hh"
#include "tclib_module.hh"
#include "ctrlr_mgr.hh"
#include "momgr_intf_stub.hh"
#include "dal_cursor.hh"
#include "momgr_intf.hh"
#include "momgr_impl.hh"

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

class VbrIfMoMgrTest: public VbrMoMgr, public ::testing::Test {
 public:
 protected:
  virtual void SetUp() {}

  virtual void TearDown() {}
};



void GetKeyValStruct(key_vbr_if *&kst, val_vbr_if *&vst) {
  const char *vtn_name = "VTN_1";
  const char *if_name = "IF_1";
  const char *vbr_name = "VBR_1";
  const char *desc = "thisisvbridge";
  const char *logical_port_id ="lport1";
  kst = reinterpret_cast<key_vbr_if *>(malloc
                 (sizeof(key_vbr_if)));
  memset(kst, 0, sizeof(key_vbr_if));
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(kst->if_name),
  if_name, strlen(if_name)+1);
  vst = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  memset(vst, 0, sizeof(val_vbr_if));
  for( unsigned int loop = 0; loop < sizeof( vst->valid )/
     sizeof( vst->valid[0] ); ++loop ) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  vst->cs_row_status = UNC_VF_VALID;
  for( unsigned int loop = 0; loop < sizeof( vst->valid )/
     sizeof(vst->valid[0]); ++loop) {
    vst->cs_attr[loop] = UNC_CS_APPLIED;
  }
  vst->admin_status = UPLL_ADMIN_ENABLE;
  strncpy(reinterpret_cast<char *>(vst->description), desc,
  strlen(desc)+1);
  vst->portmap.vlan_id=1;
  vst->portmap.tagged=1;
  for( unsigned int loop = 0; loop < sizeof( vst->portmap.valid )/
     sizeof( vst->portmap.valid[0]); ++loop ) {
    vst->portmap.valid[loop] = UNC_VF_VALID;
  }
  for( unsigned int loop = 0; loop < sizeof( vst->portmap.valid )/
     sizeof( vst->portmap.valid[0] ); ++loop ) {
    vst->portmap.cs_attr[loop] = UNC_CS_APPLIED;
  }
  strncpy(reinterpret_cast<char *>
          (vst->portmap.logical_port_id), logical_port_id,
  strlen(logical_port_id)+1);

}
void GetKeyValDrvStruct(key_vbr_if *&kst, val_drv_vbr_if *&vst) {

  const char *vex_name = "Vex_1";
  const char *vex_if_name = "Vex if_1";
  const char *vex_link_name = "Vex link_1";
  const char *vtn_name = "VTN_1";
  const char *if_name = "IF_1";
  const char *vbr_name = "VBR_1";
  const char *desc = "thisisvbridge";
  const char *logical_port_id ="lport1";
  kst = reinterpret_cast<key_vbr_if *>(malloc
                 (sizeof(key_vbr_if)));
  memset(kst,0,sizeof(key_vbr_if));
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(kst->if_name),
  if_name, strlen(if_name)+1);
  vst = reinterpret_cast<val_drv_vbr_if *>(malloc
  (sizeof(val_drv_vbr_if)));
  memset(vst,0,sizeof(val_drv_vbr_if));
  strncpy(reinterpret_cast<char *>(vst->vex_name), vex_name,
  strlen(vex_name)+1);
  strncpy(reinterpret_cast<char *>(vst->vex_if_name), vex_if_name,
  strlen(vex_if_name)+1);
  strncpy(reinterpret_cast<char *>(vst->vex_link_name), vex_link_name,
  strlen(vex_link_name)+1);
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  for(unsigned int loop = 0; loop < sizeof(vst->vbr_if_val.valid)/
     sizeof(vst->vbr_if_val.valid[0]); ++loop) {
    vst->vbr_if_val.valid[loop] = UNC_VF_VALID;
  }
  vst->vbr_if_val.cs_row_status = UNC_VF_VALID;
  for(unsigned int loop = 0; loop < sizeof(vst->vbr_if_val.valid)/
     sizeof(vst->vbr_if_val.valid[0]); ++loop) {
    vst->vbr_if_val.cs_attr[loop] = UNC_CS_APPLIED;
  }
  vst->vbr_if_val.admin_status=UPLL_ADMIN_ENABLE;
  strncpy(reinterpret_cast<char *>(vst->vbr_if_val.description), desc,
  strlen(desc)+1);
  vst->vbr_if_val.portmap.vlan_id=1;
  vst->vbr_if_val.portmap.tagged=0;
  for(unsigned int loop = 0; loop < sizeof(vst->vbr_if_val.portmap.valid)/
     sizeof(vst->vbr_if_val.portmap.valid[0]); ++loop) {
    vst->vbr_if_val.portmap.valid[loop] = UNC_VF_VALID;
  }
  for(unsigned int loop = 0; loop < sizeof(vst->vbr_if_val.portmap.valid)/
     sizeof(vst->vbr_if_val.portmap.valid[0]); ++loop) {
    vst->vbr_if_val.portmap.cs_attr[loop] = UNC_CS_APPLIED;
  }
  strncpy(reinterpret_cast<char *>(vst->vbr_if_val.portmap.logical_port_id), logical_port_id,
  strlen(logical_port_id)+1);

}
TEST_F(VbrIfMoMgrTest, DupConfigKeyVal_ReqInvalidKT) {
  VbrIfMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbrIf,
                                 val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVbrIf,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
  delete req;
  delete okey;
}
TEST_F(VbrIfMoMgrTest, DupConfigKeyVal_SuccessMAINTBL) {
  VbrIfMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,MAINTBL));
  delete req;
  delete okey;
}
TEST_F(VbrIfMoMgrTest, DupConfigKeyVal_Error) {
  VbrIfMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr_if *key;
  val_vbr_if *val=NULL;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
  delete okey;
}
TEST_F(VbrIfMoMgrTest, IsValidKey_InvalidIndex) {
  VbrIfMoMgr obj;
  uint64_t index = uudst::vbridge_interface::kDbiCtrlrName;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  EXPECT_EQ(true, obj.IsValidKey(reinterpret_cast<void *>(key),
                                 index));
}
TEST_F(VbrIfMoMgrTest, CopyToConfigkey_InValidName) {
  VbrIfMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr_if *key;
  const char *vtn_name = "";
  const char *vbr_name = "";
  const char *if_name = "";
  key = reinterpret_cast<key_vbr_if *>(malloc
                 (sizeof(key_vbr_if)));
  memset(key,0,sizeof(key_vbr_if));
  strncpy(reinterpret_cast<char *>(key->vbr_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbr_key.vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(key->if_name),
  if_name, strlen(if_name)+1);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  strncpy(reinterpret_cast<char *>(key->vbr_key.vtn_key.vtn_name),"VTN_1",32);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
}
TEST_F(VbrIfMoMgrTest, CopyToConfigkey_InValidName_01) {
  VbrIfMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr_if *key;
  const char *vtn_name = "vtn1";
  const char *vbr_name = "";
  const char *if_name = "";
  key = reinterpret_cast<key_vbr_if *>(malloc
                 (sizeof(key_vbr_if)));
  memset(key,0,sizeof(key_vbr_if));
  strncpy(reinterpret_cast<char *>(key->vbr_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbr_key.vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(key->if_name),
  if_name, strlen(if_name)+1);

  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey1));
}
TEST_F(VbrIfMoMgrTest, CompareValidValue_AuditTrue) {

  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val, *val1;
  GetKeyValStruct(key, val);
  GetKeyValStruct(key, val1);

  strncpy(reinterpret_cast<char *>(val1->description), "des1",
  strlen("des1")+1);
  void *vbrval = reinterpret_cast<void *>(&val);

  obj.CompareValidValue(vbrval, (void *)val,true);
  obj.CompareValidValue(vbrval, (void *)val1,true);

}
TEST_F(VbrIfMoMgrTest, CompareValidValue_01) {

  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val, *val1;
  GetKeyValDrvStruct(key, val);
  GetKeyValDrvStruct(key, val1);

  void *vbrval = reinterpret_cast<void *>(&val);
  val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  val->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  val->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  obj.CompareValidValue(vbrval, (void *)val,true);
  obj.CompareValidValue(vbrval, (void *)val1,true);
}
TEST_F(VbrIfMoMgrTest, CompareValidValue_02) {

  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val, *val1;
  GetKeyValDrvStruct(key, val);
  GetKeyValDrvStruct(key, val1);

  void *vbrval = reinterpret_cast<void *>(&val);
  val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
  val1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
  obj.CompareValidValue(vbrval, (void *)val,true);
  obj.CompareValidValue(vbrval, (void *)val1,true);

}
TEST(FilterAttributes,CreateOperation) {
  VbrIfMoMgr vbr;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vbr_if_t *valvbr1 = reinterpret_cast<val_vbr_if *>(malloc(sizeof(val_vbr_if_t)));
  valvbr1->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  EXPECT_EQ(false, vbr.FilterAttributes(val1,val2,audit_status,op));
}

TEST(FilterAttributes,OperationUpdate) {
  VbrIfMoMgr vbr;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_UPDATE;

  val_vbr_if_t *valvbr1 = reinterpret_cast<val_vbr_if *>(malloc(sizeof(val_vbr_if_t)));
  valvbr1->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  EXPECT_EQ(true, vbr.FilterAttributes(val1,val2,audit_status,op));
}

TEST_F(VbrIfMoMgrTest, ValidateCapability_ErrorInput) {

  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(req, ikey,NULL ));
}
TEST(ValidateCapability, ValidateCapability_Success) {

  VbrIfMoMgr vbr;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);


  CtrlrMgr::Ctrlr ctrlrobj("CTR_1", UNC_CT_PFC, "5.0");
  CtrlrMgr::GetInstance()->Add(ctrlrobj, UPLL_DT_CANDIDATE);

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

  req->operation = UNC_OP_READ_SIBLING_COUNT;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValidateCapability(req, ikey, "CTR_1"));
  req->operation = UNC_OP_DELETE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, "CTR_1"));
}
TEST(ValidateCapability, ValidateCapability_ikey_NULL) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req=NULL;
  const char *ctrlr_name="ctr1";
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, ctrlr_name));
}

TEST(vbr_if_momgr_test, ValidateCapability_SUCCESS) {
VbrIfMoMgr obj;
const char * ctrlr_name = reinterpret_cast<const char *>("PFC222");
  key_vbr_if *vbrif_key = reinterpret_cast<key_vbr_if *>
                                       (malloc(sizeof(key_vrt_if)));

memset(vbrif_key, 0, sizeof(key_vrt_if));
strcpy((char*)vbrif_key->vbr_key.vtn_key.vtn_name,(char*)"vtn1");
strcpy((char*)vbrif_key->vbr_key.vbridge_name,(char*)"VRT31");
strcpy((char*)vbrif_key->if_name,(char*)"VRTIF11");

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_UPDATE;
req->datatype =  UPLL_DT_CANDIDATE;

ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, vbrif_key, NULL);
EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey, ctrlr_name));
free(req);
}


TEST_F(VbrIfMoMgrTest, Success_01) {

  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);


  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);


  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfg_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey));
  delete ikey;
}
TEST_F(VbrIfMoMgrTest, Success_02) {

  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_INVALID;
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);


  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfg_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey));
  delete ikey;
}
TEST_F(VbrIfMoMgrTest, Success_03) {

  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);


  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbrIfSt, val);


  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfg_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey));
  delete ikey;
}
TEST_F(VbrIfMoMgrTest, AdaptValToVtnService_Failure) {

  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigKeyVal* ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey));

  ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                          IpctSt::kIpcStKeyVbr,
                          key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey));
  delete ikey;
}

TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_01) {
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER,unc::tclib::TC_API_COMMON_SUCCESS);
  TcLibModule::stub_loadtcLibModule();
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs1=0;
  uint8_t *attrs=&attrs1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_IMPORT));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_02) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs1=0;
  uint8_t *attrs=&attrs1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_03) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID;
  uint8_t attrs1=0;
  uint8_t *attrs=&attrs1;
  attrs[unc::capa::vbr_if::kCapDesc]=0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_04) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs1=0;
  uint8_t *attrs=&attrs1;
  val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID_NO_VALUE;
  attrs[unc::capa::vbr_if::kCapDesc]=0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_05) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  val->admin_status=UPLL_ADMIN_ENABLE;
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  attrs[unc::capa::vbr_if::kCapAdminStatus]=0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttrbuteSupportCheck_06) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID_NO_VALUE;
  attrs[unc::capa::vbr_if::kCapAdminStatus]=0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_07) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID_NO_VALUE;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  attrs[unc::capa::vbr_if::kCapLogicalPortId]=0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttrbuteSupportCheck_08) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  attrs[unc::capa::vbr_if::kCapLogicalPortId]=0;
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_09) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID_NO_VALUE;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  attrs[unc::capa::vbr_if::kCapVlanId]=0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttrbuteSupportCheck_10) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  attrs[unc::capa::vbr_if::kCapVlanId]=0;
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_11) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[20];
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  attrs[unc::capa::vbr_if::kCapTagged]=0;
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttrbuteSupportCheck_12) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[20];
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  attrs[unc::capa::vbr_if::kCapTagged]=0;
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}
TEST(ValVbrAttributeSupportCheck, ValVbrAttributeSupportCheck_13) {

  VbrIfMoMgr vbr;
  key_vbr_if *kst;
  val_vbr_if *vst=NULL;
  const char *vtn_name = "VTN_1";
  const char *if_name = "IF_1";
  const char *vbr_name = "VBR_1";
  kst = reinterpret_cast<key_vbr_if *>(malloc
                 (sizeof(key_vbr_if)));
  memset(kst,0,sizeof(key_vbr_if));
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(kst->if_name),
  if_name, strlen(if_name)+1);

  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs1=0;
  uint8_t *attrs=&attrs1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, vst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, kst, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_IMPORT));
  delete ikey;
}
TEST(RestoreUnInitOPerStatus, RestoreUnInitOPerStatus_01) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.RestoreUnInitOPerStatus(dmi));
  delete ikey;
}

TEST(GetVexternal, GetVexternal_01) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t vexternal1=1;
  uint8_t *vexternal=&vexternal1;
  uint8_t vex_if1=1;
  uint8_t *vex_if=&vex_if1;
  InterfacePortMapInfo iftype;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetVexternal(ikey,dt_type,dmi,vexternal,vex_if,iftype));
  delete ikey;
}
TEST(GetVexternal, GetVexternal_02) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  upll_keytype_datatype_t dt_type=UPLL_DT_AUDIT;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t vexternal1=1;
  uint8_t *vexternal=&vexternal1;
  uint8_t vex_if1=1;
  uint8_t *vex_if=&vex_if1;
  InterfacePortMapInfo iftype;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetVexternal(ikey,dt_type,dmi,vexternal,vex_if,iftype));
  delete ikey;
}
TEST(GetVexternal, GetVexternal_03) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t vexternal1=1;
  uint8_t *vexternal=&vexternal1;
  uint8_t vex_if1=1;
  uint8_t *vex_if=&vex_if1;
  InterfacePortMapInfo iftype;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetVexternal(ikey,dt_type,dmi,vexternal,vex_if,iftype));
  delete ikey;
}

TEST(GetVexternal, GetVexternal_04) {

  VbrIfMoMgr vbr;
  key_vbr_if *key=NULL;
  val_vbr_if *val=NULL;
  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t vexternal1=1;
  uint8_t *vexternal=&vexternal1;
  uint8_t vex_if1=1;
  uint8_t *vex_if=&vex_if1;
  InterfacePortMapInfo iftype;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetVexternal(ikey,dt_type,dmi,vexternal,vex_if,iftype));
  delete ikey;
}
TEST(IsReferenced, IsReferenced_01) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi=new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.IsReferenced(ikey,dt_type,dmi));
  delete ikey;
}
TEST(IsReferenced, IsReferenced_02) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.IsReferenced(ikey,dt_type,dmi));
  delete ikey;
}
TEST(IsReferenced, IsReferenced_03) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.IsReferenced(ikey,dt_type,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(IsReferenced, IsReferenced_04) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.IsReferenced(ikey,dt_type,dmi));
  delete ikey;
}
TEST(UpdateMo, UpdateMo_01) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, vbr.UpdateMo(req,ikey,dmi));
  delete ikey;
}
TEST(UpdateMo, UpdateMo_02) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi=new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.UpdateMo(req,ikey,dmi));
  delete ikey;
}
TEST(UpdateMo, UpdateMo_03) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);

  key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
  uuu::upll_strncpy(user_data->ctrlr_id,"ctr_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.UpdateMo(req,ikey,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(UpdateMo, UpdateMo_04) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vbr.UpdateMo(req,ikey,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(UpdateMo, UpdateMo_05) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
  uuu::upll_strncpy(user_data->ctrlr_id,"ctr_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.UpdateMo(req,ikey,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(ValidateAttribute, ValidateAttribute_01) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateAttribute(ikey,dmi,req));
  delete ikey;
}
TEST(ValidateAttribute, ValidateAttribute_02) {

  VbrIfMoMgr vbr;
  key_vbr_if *kst;
  val_vbr_if *vst=NULL;

  const char *vtn_name = "VTN_1";
  const char *if_name = "IF_1";
  const char *vbr_name = "VBR_1";
  kst = reinterpret_cast<key_vbr_if *>(malloc
                 (sizeof(key_vbr_if)));
  memset(kst,0,sizeof(key_vbr_if));
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(kst->if_name),
  if_name, strlen(if_name)+1);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, vst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, kst, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateAttribute(ikey,dmi,req));
  delete ikey;
}
TEST(ValidateAttribute, ValidateAttribute_03) {

  VbrIfMoMgr vbr;
  key_vbr_if *kst;
  val_vbr_if *vst=NULL;

  const char *vtn_name = "VTN_1";
  const char *if_name = "IF_1";
  const char *vbr_name = "VBR_1";
  kst = reinterpret_cast<key_vbr_if *>(malloc
                 (sizeof(key_vbr_if)));
  memset(kst,0,sizeof(key_vbr_if));
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(kst->if_name),
  if_name, strlen(if_name)+1);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, vst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, kst, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateAttribute(ikey,dmi,req));
}

 TEST(ValidateAttribute, ValidateAttribute_04) {
  VbrIfMoMgr vbr;
  key_vbr_if *kst;
  val_vbr_if *vst=NULL;
  const char *vtn_name = "VTN_1";
  const char *if_name = "IF_1";
  const char *vbr_name = "VBR_1";
  kst = reinterpret_cast<key_vbr_if *>(malloc
                     (sizeof(key_vbr_if)));
  memset(kst,0,sizeof(key_vbr_if));
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vbridge_name),
  vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(kst->if_name),
  if_name, strlen(if_name)+1);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, vst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, kst, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateAttribute(ikey,dmi,req));
  delete ikey;
 }

TEST(CopyToConfigkey, CopyToConfigkey_01) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.CopyToConfigKey(okey,ikey));

}
TEST(CopyToConfigkey, CopyToConfigkey_02) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.CopyToConfigKey(okey,ikey));
}

TEST(CopyToConfigkey, CopyToConfigkey_03) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey=NULL;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.CopyToConfigKey(okey,ikey));
}
TEST(CopyToConfigkey, CopyToConfigkey_04) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey=NULL;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  uuu::upll_strncpy(key->vbr_key.vtn_key.vtn_name, key_rename->old_unc_vtn_name,(kMaxLenVtnName + 1));


  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.CopyToConfigKey(okey,ikey));
}
TEST(CopyToConfigkey, CopyToConfigkey_05) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey=NULL;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.CopyToConfigKey(okey,ikey));
}

TEST(UpdateConfigVal, UpdateConfigVal_01) {
  VbrIfMoMgr vbr;
  upll_keytype_datatype_t datatype=UPLL_DT_CANDIDATE;
  key_vbr_if *key;
  val_vbr_if *val;
  DalDmlIntf *dmi=new DalOdbcMgr();
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.UpdateConfigVal(ikey,datatype,dmi));
  delete ikey;
}
TEST(UpdateConfigVal, UpdateConfigVal_03) {
  VbrIfMoMgr vbr;
  upll_keytype_datatype_t datatype=UPLL_DT_CANDIDATE;
  key_vbr_if *key;
  val_vbr_if *val;
  DalDmlIntf *dmi=new DalOdbcMgr();
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.UpdateConfigVal(ikey,datatype,dmi));
}

TEST(ValidateVbrifKey, ValidateVbrifKey_01) {
  VbrIfMoMgr vbr;
  unc_keytype_operation_t operation=UNC_OP_CREATE;
  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t*>
                     (malloc(sizeof(key_vbr_if_t)));
  memset(key, 0 ,sizeof(key_vbr_if_t));

  strncpy((char*) key->if_name,"IF_1",32);
  strncpy((char*) key->vbr_key.vbridge_name," ",32);
  strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",32);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.ValidateVbrifKey(key,operation));
}
TEST(ValidateVbrifKey, ValidateVbrifKey_02) {
  VbrIfMoMgr vbr;
  unc_keytype_operation_t operation=UNC_OP_CREATE;
  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t*>
                     (malloc(sizeof(key_vbr_if_t)));
  memset(key, 0 ,sizeof(key_vbr_if_t));
  strncpy((char*) key->if_name," ",32);
  strncpy((char*) key->vbr_key.vbridge_name,"vbr ",32);
  strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",32);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.ValidateVbrifKey(key,operation));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_011) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  strncpy((char*) val->description," ",32);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_02) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_03) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_04) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_INVALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_05) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_06) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_07) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_08) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_09) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_10) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_11) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_12) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  val->portmap.logical_port_id[0] = 'S';
  val->portmap.logical_port_id[1] = 'W';
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_13) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVbrIfValue, ValidateVbrIfValue_14) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  key_vbr_if *key;
  GetKeyValStruct(key, val);
  val = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));
}
TEST(ValidateVtnNeighborValue, ValidateVtnNeighborValue_01) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = "VTN_1";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = "VBR_1";
  val_vtn_neighbor *vtn_neighbor= reinterpret_cast<val_vtn_neighbor *>(malloc
                                  (sizeof(val_vtn_neighbor)));
  memset(vtn_neighbor,0,sizeof(val_vtn_neighbor));
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vnode_name),
  connected_vnode_name, strlen(connected_vnode_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_if_name),
  connected_if_name, strlen(connected_if_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vlink_name),
  connected_vlink_name, strlen(connected_vlink_name)+1);
  for(unsigned int loop = 0; loop < sizeof(vtn_neighbor->valid)/
     sizeof(vtn_neighbor->valid[0]); ++loop) {
    vtn_neighbor->valid[loop] = UNC_VF_VALID;
  }
  unc_keytype_operation_t operation = UNC_OP_CREATE;
  vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));
}
TEST(ValidateVtnNeighborValue, ValidateVtnNeighborValue_01_) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = " ";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = "VBR_1";
  val_vtn_neighbor *vtn_neighbor= reinterpret_cast<val_vtn_neighbor *>(malloc
                                  (sizeof(val_vtn_neighbor)));
  memset(vtn_neighbor,0,sizeof(val_vtn_neighbor));
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vnode_name),
  connected_vnode_name, strlen(connected_vnode_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_if_name),
  connected_if_name, strlen(connected_if_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vlink_name),
  connected_vlink_name, strlen(connected_vlink_name)+1);
  for(unsigned int loop = 0; loop < sizeof(vtn_neighbor->valid)/
     sizeof(vtn_neighbor->valid[0]); ++loop) {
    vtn_neighbor->valid[loop] = UNC_VF_VALID;
  }
  unc_keytype_operation_t operation = UNC_OP_CREATE;
  vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));
}

TEST(ValidateVtnNeighborValue, ValidateVtnNeighborValue_02) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = "VTN_1";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = "VBR_1";
  val_vtn_neighbor *vtn_neighbor= reinterpret_cast<val_vtn_neighbor *>(malloc
                                  (sizeof(val_vtn_neighbor)));
  memset(vtn_neighbor,0,sizeof(val_vtn_neighbor));
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vnode_name),
  connected_vnode_name, strlen(connected_vnode_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_if_name),
  connected_if_name, strlen(connected_if_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vlink_name),
  connected_vlink_name, strlen(connected_vlink_name)+1);
  for(unsigned int loop = 0; loop < sizeof(vtn_neighbor->valid)/
     sizeof(vtn_neighbor->valid[0]); ++loop) {
    vtn_neighbor->valid[loop] = UNC_VF_VALID;
  }
  unc_keytype_operation_t operation = UNC_OP_CREATE;
  vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));
}
TEST(ValidateVtnNeighborValue, ValidateVtnNeighborValue_03) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = "VTN_1";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = "VBR_1";
  val_vtn_neighbor *vtn_neighbor= reinterpret_cast<val_vtn_neighbor *>(malloc
                                  (sizeof(val_vtn_neighbor)));
  memset(vtn_neighbor,0,sizeof(val_vtn_neighbor));
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vnode_name),
  connected_vnode_name, strlen(connected_vnode_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_if_name),
  connected_if_name, strlen(connected_if_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vlink_name),
  connected_vlink_name, strlen(connected_vlink_name)+1);
  for(unsigned int loop = 0; loop < sizeof(vtn_neighbor->valid)/
     sizeof(vtn_neighbor->valid[0]); ++loop) {
    vtn_neighbor->valid[loop] = UNC_VF_VALID;
  }
  unc_keytype_operation_t operation = UNC_OP_CREATE;
  vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));
}
TEST(ValidateVtnNeighborValue, ValidateVtnNeighborValue_04) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = "VTN_1";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = "VBR_1";
  val_vtn_neighbor *vtn_neighbor= reinterpret_cast<val_vtn_neighbor *>(malloc
                                  (sizeof(val_vtn_neighbor)));
  memset(vtn_neighbor,0,sizeof(val_vtn_neighbor));
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vnode_name),
  connected_vnode_name, strlen(connected_vnode_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_if_name),
  connected_if_name, strlen(connected_if_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vlink_name),
  connected_vlink_name, strlen(connected_vlink_name)+1);
  for(unsigned int loop = 0; loop < sizeof(vtn_neighbor->valid)/
     sizeof(vtn_neighbor->valid[0]); ++loop) {
    vtn_neighbor->valid[loop] = UNC_VF_VALID;
  }
  unc_keytype_operation_t operation = UNC_OP_CREATE;
  vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));
}
TEST(ValidateVtnNeighborValue, ValidateVtnNeighborValue_05) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = "VTN_1";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = " ";
  val_vtn_neighbor *vtn_neighbor= reinterpret_cast<val_vtn_neighbor *>(malloc
                                  (sizeof(val_vtn_neighbor)));

  memset(vtn_neighbor,0,sizeof(val_vtn_neighbor));
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vnode_name),
  connected_vnode_name, strlen(connected_vnode_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_if_name),
  connected_if_name, strlen(connected_if_name)+1);
  strncpy(reinterpret_cast<char *>(vtn_neighbor->connected_vlink_name),
  connected_vlink_name, strlen(connected_vlink_name)+1);
  for(unsigned int loop = 0; loop < sizeof(vtn_neighbor->valid)/
     sizeof(vtn_neighbor->valid[0]); ++loop) {
    vtn_neighbor->valid[loop] = UNC_VF_VALID;
  }
  unc_keytype_operation_t operation = UNC_OP_CREATE;
  vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));
}

TEST(IsLogicalPortAndVlanIdInUse, IsLogicalPortAndVlanIdInUse_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_IMPORT;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.IsLogicalPortAndVlanIdInUse(ikey,dmi,req));
  delete ikey;
}
TEST(IsLogicalPortAndVlanIdInUse, IsLogicalPortAndVlanIdInUse_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  GetKeyValStruct(key, val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.IsLogicalPortAndVlanIdInUse(ikey,dmi,req));
  delete ikey;
}
TEST(IsLogicalPortAndVlanIdInUse, IsLogicalPortAndVlanIdInUse_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  GetKeyValStruct(key, val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);

  DalDmlIntf *dmi= new DalOdbcMgr();
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.IsLogicalPortAndVlanIdInUse(ikey,dmi,req));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(IsLogicalPortAndVlanIdInUse, IsLogicalPortAndVlanIdInUse_04) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_IMPORT;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  GetKeyValStruct(key, val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);


  DalDmlIntf *dmi= new DalOdbcMgr();
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.IsLogicalPortAndVlanIdInUse(ikey,dmi,req));
  DalOdbcMgr::clearStubData();
}

TEST(IsLogicalPortAndVlanIdInUse, IsLogicalPortAndVlanIdInUse_05) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_IMPORT;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  GetKeyValStruct(key, val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);


  DalDmlIntf *dmi= new DalOdbcMgr();
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM]=UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.IsLogicalPortAndVlanIdInUse(ikey,dmi,req));
  DalOdbcMgr::clearStubData();
}
TEST(GetBoundaryInterfaces, 01) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vnode_if_t boundary_if;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vbr.GetBoundaryInterfaces(boundary_if,dmi,ikey));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(SetBoundaryIfOperStatusforPathFault, 01) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  state_notification notification;
  const set<key_vnode_if_t, key_vnode_if_compare> boundary_if_set;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.SetBoundaryIfOperStatusforPathFault(boundary_if_set,notification,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(SetBoundaryIfOperStatusforPathFault, 02) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  state_notification notification;
  const set<key_vnode_if_t, key_vnode_if_compare> boundary_if_set;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.SetBoundaryIfOperStatusforPathFault(boundary_if_set,notification,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(CreateAuditMoImpl, 01) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.CreateAuditMoImpl(ikey,dmi,ctrlr_name));
}

TEST(vbrif_momgr_test, CreateAuditMoImplSuccess ) {
  VbrIfMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr();
  const char *ctrlr_id ="pfc001";
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
  key_vbr_if *key ;
  val_vbr_if *val ;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CreateAuditMoImpl(ikey, dmi, ctrlr_id));
}

TEST(CreateAuditMoImpl, 02) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbrIf,
                            key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.CreateAuditMoImpl(ikey,dmi,ctrlr_name));
}

TEST(vbrif_momgr_test, 03 ) {
  VbrIfMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr();
  const char *ctrlr_id ="pfc001";
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
  key_vbr_if *key ;
  val_vbr_if *val ;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CreateAuditMoImpl(ikey, dmi, ctrlr_id));
}

TEST(GetMappedVbridges, 01) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  const char *domain_id="dom1";
  std::string logportid;
  set<key_vnode_type_t, key_vnode_type_compare> sw_vbridge_set;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetMappedVbridges(ctrlr_name,domain_id,logportid,dmi,&sw_vbridge_set));
  DalOdbcMgr::clearStubData();
}
TEST(GetMappedVbridges, 02) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  const char *domain_id="dom1";
  std::string logportid;
  set<key_vnode_type_t, key_vnode_type_compare> sw_vbridge_set;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetMappedVbridges(ctrlr_name,domain_id,logportid,dmi,&sw_vbridge_set));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetMappedVbridges, 03) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  const char *domain_id="dom1";
  std::string logportid;
  set<key_vnode_type_t, key_vnode_type_compare> sw_vbridge_set;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.GetMappedVbridges(ctrlr_name,domain_id,logportid,dmi,&sw_vbridge_set));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetMappedVbridges, 04) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  const char *domain_id="dom1";
  std::string logportid;
  set<key_vnode_type_t, key_vnode_type_compare> sw_vbridge_set;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, NULL);
  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetMappedVbridges(ctrlr_name,domain_id,logportid,dmi,&sw_vbridge_set));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(PathFaultHandler, 03) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  const char *domain_id="dom1";
  std::vector<std::string> ingress_ports;
  std::vector<std::string> egress_ports;
  bool alarm_asserted=UPLL_OPER_STATUS_UP;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.PathFaultHandler(ctrlr_name,domain_id,ingress_ports,egress_ports,alarm_asserted,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(PathFaultHandler, 04) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  const char *domain_id="dom1";
  std::vector<std::string> ingress_ports;
  std::vector<std::string> egress_ports;
  bool alarm_asserted;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.PathFaultHandler(ctrlr_name,domain_id,ingress_ports,egress_ports,alarm_asserted,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(TxUpdateController,NULL ) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;

  set<std::string> affected_ctrlr_set;
  DalDmlIntf *dmi= new DalOdbcMgr();
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.TxUpdateController(UNC_KT_VBR_IF,session_id,config_id,phase,&affected_ctrlr_set  ,dmi,&ikey));
  DalOdbcMgr::clearStubData();
}
TEST(TxUpdateController,default ) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;

  set<std::string> affected_ctrlr_set;
  affected_ctrlr_set.insert
                  (string(reinterpret_cast<char *>(ctrlr_dom->ctrlr)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.TxUpdateController(UNC_KT_VBR_IF,session_id,config_id,phase,&affected_ctrlr_set  ,dmi,&ikey));
  DalOdbcMgr::clearStubData();
}

TEST(TxUpdateController, 01 ) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;

  set<std::string> affected_ctrlr_set;
  affected_ctrlr_set.insert
                  (string(reinterpret_cast<char *>(ctrlr_dom->ctrlr)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS,kDalRcSuccess);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);

  key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
  uuu::upll_strncpy(user_data->ctrlr_id,"ctr_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.TxUpdateController(UNC_KT_VBR_IF,session_id,config_id,phase,&affected_ctrlr_set  ,dmi,&ikey));
  DalOdbcMgr::clearStubData();
}
TEST(TxUpdateController, 02 ) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpDelete;
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;

  set<std::string> affected_ctrlr_set;
  affected_ctrlr_set.insert
                  (string(reinterpret_cast<char *>(ctrlr_dom->ctrlr)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.TxUpdateController(UNC_KT_VBR_IF,session_id,config_id,phase,&affected_ctrlr_set  ,dmi,&ikey));
  DalOdbcMgr::clearStubData();
}
 TEST(TxUpdateController, 03 ) {

      VbrIfMoMgr vbr;
      key_vbr_if *key;
      val_vbr_if *val;

      GetKeyValStruct(key, val);
      uint32_t session_id=1;
      uint32_t config_id=2;
      uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpDelete;
      uint8_t *ctr_id1 = (uint8_t *)malloc(32);
      memset(ctr_id1, '\0', 32);
      memcpy(ctr_id1, "Controller1", 11);
      uint8_t *dom_id1 = (uint8_t *)malloc(32);
      memset(dom_id1, '\0', 32);
      memcpy(dom_id1, "Domain1", 7);
      controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
      ctrlr_dom->ctrlr = ctr_id1;
      ctrlr_dom->domain = dom_id1;

      set<std::string> affected_ctrlr_set ;
      DalDmlIntf *dmi= new DalOdbcMgr();
      GetKeyValStruct(key, val);
      ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
      ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
      DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
      DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
      EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.TxUpdateController(UNC_KT_VBR_IF,session_id,config_id,phase,&affected_ctrlr_set  ,dmi,&ikey));
      DalOdbcMgr::clearStubData();
    }

TEST(AuditUpdateController, 01 ) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  const char *ctrlr_id="Controller1";
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;

  DalDmlIntf *dmi= new DalOdbcMgr();
  GetKeyValStruct(key, val);
  bool ctrlr_affected=true;
  DalCursor *cursor = new DalCursor();
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.AuditUpdateController(UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, &ctrlr_affected ,dmi));
  DalOdbcMgr::clearStubData();
}
TEST(AuditUpdateController, 02 ) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  const char *ctrlr_id="Controller1";
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;

  DalDmlIntf *dmi= new DalOdbcMgr();
  GetKeyValStruct(key, val);
  bool ctrlr_affected=true;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.AuditUpdateController(UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, &ctrlr_affected ,dmi));
  DalOdbcMgr::clearStubData();
}

TEST(AuditUpdateController, 03 ) {

          VbrIfMoMgr vbr;
          key_vbr_if *key;
          val_vbr_if *val;

          GetKeyValStruct(key, val);
          uint32_t session_id=1;
          uint32_t config_id=2;
          uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
          const char *ctrlr_id="Controller1";
          uint8_t *ctr_id1 = (uint8_t *)malloc(32);
          memset(ctr_id1, '\0', 32);
          memcpy(ctr_id1, "Controller1", 11);
          uint8_t *dom_id1 = (uint8_t *)malloc(32);
          memset(dom_id1, '\0', 32);
          memcpy(dom_id1, "Domain1", 7);
          controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
          ctrlr_dom->ctrlr = ctr_id1;
          ctrlr_dom->domain = dom_id1;

          DalDmlIntf *dmi= new DalOdbcMgr();
          GetKeyValStruct(key, val);
          bool ctrlr_affected=true;
          EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.AuditUpdateController(UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, &ctrlr_affected ,dmi));
 }

 TEST(AuditUpdateController, 04 ) {
     VbrIfMoMgr vbr;
     key_vbr_if *key;
     val_vbr_if *val;
     GetKeyValStruct(key, val);
     uint32_t session_id=1;
     uint32_t config_id=2;
     uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
     const char *ctrlr_id="Controller1";
     uint8_t *ctr_id1 = (uint8_t *)malloc(32);
     memset(ctr_id1, '\0', 32);
     memcpy(ctr_id1, "Controller1", 11);
     uint8_t *dom_id1 = (uint8_t *)malloc(32);
     memset(dom_id1, '\0', 32);
     memcpy(dom_id1, "Domain1", 7);
     controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
     ctrlr_dom->ctrlr = ctr_id1;
     ctrlr_dom->domain = dom_id1;

     DalDmlIntf *dmi= new DalOdbcMgr();
     bool ctrlr_affected=true;
     EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.AuditUpdateController(UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, &ctrlr_affected ,dmi));
}


     TEST(AuditUpdateController, 05 ) {
         VbrIfMoMgr vbr;
         key_vbr_if *key;
         val_vbr_if *val;
         GetKeyValStruct(key, val);
         uint32_t session_id=1;
         uint32_t config_id=2;
         uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpDelete;
         const char *ctrlr_id="Controller1";
         uint8_t *ctr_id1 = (uint8_t *)malloc(32);
         memset(ctr_id1, '\0', 32);
         memcpy(ctr_id1, "Controller1", 11);
         uint8_t *dom_id1 = (uint8_t *)malloc(32);
         memset(dom_id1, '\0', 32);
         memcpy(dom_id1, "Domain1", 7);
         controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
         ctrlr_dom->ctrlr = ctr_id1;
         ctrlr_dom->domain = dom_id1;

         DalDmlIntf *dmi= new DalOdbcMgr();
         GetKeyValStruct(key, val);
         bool ctrlr_affected=true;
         EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.AuditUpdateController(UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, &ctrlr_affected ,dmi));
    }

 TEST(AuditUpdateController, 06 ) {
   VbrIfMoMgr vbr;
   key_vbr_if *key;
   val_vbr_if *val;
   GetKeyValStruct(key, val);
   uint32_t session_id=1;
   uint32_t config_id=2;
   uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
   const char *ctrlr_id="Controller1";
   uint8_t *ctr_id1 = (uint8_t *)malloc(32);
   memset(ctr_id1, '\0', 32);
   memcpy(ctr_id1, "Controller1", 11);
   uint8_t *dom_id1 = (uint8_t *)malloc(32);
   memset(dom_id1, '\0', 32);
   memcpy(dom_id1, "Domain1", 7);
   controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
   ctrlr_dom->ctrlr = ctr_id1;
   ctrlr_dom->domain = dom_id1;

   DalDmlIntf *dmi= new DalOdbcMgr();
   GetKeyValStruct(key, val);
   bool ctrlr_affected=true;
   EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.AuditUpdateController(UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, &ctrlr_affected ,dmi));
  }



TEST(vbr_if_test,UpdatePortMap_01){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval );
  upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING ;
  DalDmlIntf * dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdatePortMap(okey,dt_type,dmi,ikey));
}
TEST(vbr_if_test,UpdatePortMap_02){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval );
  upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING ;
  DalDmlIntf * dmi= new DalOdbcMgr();
  val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdatePortMap(okey,dt_type,dmi,ikey));
}
TEST(vbr_if_test,UpdatePortMap_03){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval );
  upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING ;
  DalDmlIntf * dmi= new DalOdbcMgr();
  val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdatePortMap(okey,dt_type,dmi,ikey));
}
TEST(vbr_if_test,UpdatePortMap_04){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval );
  upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING ;
  DalDmlIntf * dmi= new DalOdbcMgr();
  val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdatePortMap(okey,dt_type,dmi,ikey));
}
TEST (GetRenameKeyBindInfo, PMainTbl)
{
  VbrIfMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_IF;
  BindInfo *bin;
  int nattr;
  MoMgrTables tbl = MAINTBL;

  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, tbl));
}

TEST (GetRenameKeyBindInfo, FRenameTBL)
{
  VbrIfMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_IF;
  BindInfo *bin;
  int nattr;
  MoMgrTables tbl = RENAMETBL;
  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, tbl));
}

TEST (GetRenameKeyBindInfo, FRename)
{
  VbrIfMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_IF;
  BindInfo *bin = NULL;
  int nattr = 0;
  MoMgrTables tbl = RENAMETBL;
  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, tbl));
}

TEST (GetRenameKeyBindInfo, FNoTBL)
{
  VbrIfMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_IF;
  BindInfo *bin;
  int nattr;
  MoMgrTables tbl = CTRLRTBL;
  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, tbl));
}

TEST(vbr_if_test,GetVbrIfValfromDBTrue){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);

  ConfigKeyVal *ck_drv_vbr_if = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE ; 
  DalDmlIntf * dmi= new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetVbrIfValfromDB(ikey,ck_drv_vbr_if,dt_type,dmi));
  DalOdbcMgr::clearStubData();
}

TEST(vbr_if_test,GetVbrIfValfromDBFalse){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  ConfigKeyVal *ck_drv_vbr_if = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbr,key,NULL );
  upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING ;
  DalDmlIntf * dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetVbrIfValfromDB(ikey,ck_drv_vbr_if,dt_type,dmi));
}

TEST(vbr_if_test,GetVbrIfValfromDBParentKeyNull){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,key, cfgval);
  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ck_drv_vbr_if = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval1);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE ;
  DalDmlIntf * dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetVbrIfValfromDB(ck_drv_vbr_if,ikey,dt_type,dmi));
  delete ikey;
}
TEST(vbr_if_test, UpdateVbrIf_1) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  DalDmlIntf * dmi= new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.updateVbrIf(req,ikey,dmi));
  DalOdbcMgr::clearStubData();
}

TEST(vbr_if_test, UpdateVbrIf_2) {
  VbrIfMoMgr obj;
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                        (sizeof(IpcReqRespHeader)));
  req->operation = UNC_OP_UPDATE;
  req->rep_count = 1;
  req->option1 = UNC_OPT1_NORMAL;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  DalDmlIntf * dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.updateVbrIf(req,ikey,dmi));
  DalOdbcMgr::clearStubData();
}
TEST(vbr_if_test, UpdateVbrIf_3) {
  VbrIfMoMgr obj;
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                        (sizeof(IpcReqRespHeader)));
  req->operation = UNC_OP_UPDATE;
  req->rep_count = 1;
  req->option1 = UNC_OPT1_NORMAL;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  DalDmlIntf * dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.updateVbrIf(req,ikey,dmi));
  DalOdbcMgr::clearStubData();
}
TEST(vbr_if_test, UpdateVbrIf_4) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  DalDmlIntf * dmi= new DalOdbcMgr();
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.updateVbrIf(req,ikey,dmi));
  DalOdbcMgr::clearStubData();
}
TEST(GetChildConfigKey, PkeyVlinkSuccess) {
  VbrIfMoMgr vbr;
  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t*>
                     (malloc(sizeof(key_vbr_if_t)));
  val_vbr_if *val = reinterpret_cast<val_vbr_if *>
                     (malloc(sizeof(val_vbr_if)));
  memset(key, 0 ,sizeof(key_vbr_if_t));

  strncpy((char*) key->if_name,"IF_1",32);
  strncpy((char*) key->vbr_key.vbridge_name,"VLINK1",32);
  strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",32);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VLINK,
                       IpctSt::kIpcStKeyVbrIf,
                       key,cfgval);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));
  delete okey;
}
TEST(GetChildConfigKey, PkeyVtnSuccess) {
  VbrIfMoMgr vbr;
  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t*>
                     (malloc(sizeof(key_vbr_if_t)));
  val_vbr_if *val = reinterpret_cast<val_vbr_if *>
                     (malloc(sizeof(val_vbr_if)));
  memset(key, 0 ,sizeof(key_vbr_if_t));

  strncpy((char*) key->if_name,"IF_1",32);
  strncpy((char*) key->vbr_key.vbridge_name,"VLINK1",32);
  strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",32);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                       IpctSt::kIpcStKeyVbrIf,
                       key,cfgval);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));
  delete okey;
}
TEST(GetChildConfigKey, PkeyVbridgeSuccess) {
  VbrIfMoMgr vbr;
  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t*>
                     (malloc(sizeof(key_vbr_if_t)));
  val_vbr_if *val = reinterpret_cast<val_vbr_if *>
                     (malloc(sizeof(val_vbr_if)));
  memset(key, 0 ,sizeof(key_vbr_if_t));

  strncpy((char*) key->if_name,"IF_1",32);
  strncpy((char*) key->vbr_key.vbridge_name,"VLINK1",32);
  strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",32);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                       IpctSt::kIpcStKeyVbrIf,
                       key,cfgval);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));
  delete okey;
}
TEST(GetParentConfigKey, GetParentConfigKey_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.GetParentConfigKey(okey, ikey));
}
TEST(GetParentConfigKey, GetParentConfigKey_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetParentConfigKey(okey, ikey));
  delete okey;
}
TEST(GetParentConfigKey, GetParentConfigKey_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);

  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.GetParentConfigKey(okey, ikey));
}
TEST(GetParentConfigKey, GetParentConfigKey_09) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);

  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetParentConfigKey(okey, ikey));
}
TEST_F(VbrIfMoMgrTest, AllocVal_Error) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(cfgval, UPLL_DT_CANDIDATE,RENAMETBL));
}
TEST_F(VbrIfMoMgrTest, ConverttoDriverPortMap_01) {

  VbrIfMoMgr obj;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ConverttoDriverPortMap(ikey));
}
TEST_F(VbrIfMoMgrTest, ConverttoDriverPortMap_02) {

  VbrIfMoMgr obj;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  const char *if_name = "IF_1";

  strncpy(reinterpret_cast<char *>(key->if_name),
  if_name, strlen(if_name)+1);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ConverttoDriverPortMap(ikey));
}
TEST_F(VbrIfMoMgrTest, UpdateConfigStatus_Success) {

  VbrIfMoMgr obj;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
  delete ikey;
}
TEST_F(VbrIfMoMgrTest,UpdateConfigStatus_SuccessUPDATE) {

  VbrIfMoMgr obj;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
  delete ikey;
}
TEST_F(VbrIfMoMgrTest,UpdateConfigStatus_SuccessUPDATE_01) {

  VbrIfMoMgr obj;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest,UpdateConfigStatus_SuccessUPDATE_02) {

  VbrIfMoMgr obj;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, UpdateConfigStatus_InvalidOP) {

  VbrIfMoMgr obj;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_READ,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
  delete ikey;
}
TEST_F(VbrIfMoMgrTest, UpdateAuditConfigStatus_ValidCsStatus) {
  VbrIfMoMgr vbrmomgr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  UpdateCtrlrPhase phase = kUpllUcpCreate;
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, (void*)key, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running));
  delete ckv_running;
}

TEST_F(VbrIfMoMgrTest, UpdateAuditConfigStatus_UpdatePhase) {
  VbrIfMoMgr vbrmomgr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  unc_keytype_configstatus_t cs_status = UNC_CS_NOT_APPLIED;
  UpdateCtrlrPhase phase = kUpllUcpUpdate;
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, (void*)key, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running));
  delete ckv_running;
}
TEST_F(VbrIfMoMgrTest, UpdateAuditConfigStatus_InvalidCsStatus) {
  VbrIfMoMgr vbrmomgr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  unc_keytype_configstatus_t cs_status = UNC_CS_INVALID;
  UpdateCtrlrPhase phase = kUpllUcpCreate;
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, (void*)key, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running));
  delete ckv_running;
}

TEST_F(VbrIfMoMgrTest, UpdateAuditConfigStatus_EmptyVal) {
  VbrIfMoMgr vbrmomgr;
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  UpdateCtrlrPhase phase = kUpllUcpCreate;
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running));
  delete ckv_running;
}
TEST(VbrIfMoMgr, UpdateAuditConfigStatus1) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *ikey =NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey));
  delete ikey;
}
TEST(VbrIfMoMgr, UpdateAuditConfigStatus2) {
  VbrIfMoMgr vbr;
  val_vbr_if_t *val = reinterpret_cast<val_vbr_if_t *>
                   (malloc(sizeof(val_vbr_if_t)));
  memset(val, 0, sizeof(val_vbr_if_t));

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbrIf,
                                 val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf, NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.UpdateAuditConfigStatus(UNC_CS_INVALID,uuc::kUpllUcpCreate, ikey));
  delete ikey;
}
TEST(ValidateMessage, UNC_KT_VBRIDGE) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbrIf,
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
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey));
  free(req);
}
TEST(ValidateMessage, req_NULL) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = NULL;

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateMessage(req, ikey));
  free(req);
}
TEST(ValidateMessage, kIpcStValVtnNeighbor) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtnNeighbor, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
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
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey));
}
TEST(ValidateMessage, kIpcStPfcdrvValVbrIf) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
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
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey));
}
TEST(ValidateMessage, kIpcStKeyVbr) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
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
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey));
}

TEST(ValidateMessage, ValidateMessage_01) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
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

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey));

  ConfigVal *cfgval2 = NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval2);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey2));


  const char *if_name = " ";
  key_vbr_if *key5;
  val_vbr_if *val5;
  key5 = reinterpret_cast<key_vbr_if *>(malloc
                 (sizeof(key_vbr_if)));
  memset(key5,0,sizeof(key_vbr_if));
  strncpy(reinterpret_cast<char *>(key5->if_name),
  if_name, strlen(if_name)+1);
  val5 = reinterpret_cast<val_vbr_if *>(malloc
  (sizeof(val_vbr_if)));
  memset(val5,0,sizeof(val_vbr_if));
  ConfigVal *cfgval5 = new ConfigVal(IpctSt::kIpcStValVbrIf, val5);
  ConfigKeyVal *ikey5 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key5, cfgval5);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateMessage(req, ikey5));

  ikey=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateMessage(req, ikey));


  ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey));
  delete ikey;
}

TEST(ValidateMessage, ValidateMessage_02) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_UPDATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey));
  req->operation = UNC_OP_CONTROL;
  ConfigKeyVal *ikey4 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, vbr.ValidateMessage(req, ikey));

  req->operation = UNC_OP_UPDATE;
  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey1));

  ConfigVal *cfgval2 = NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval2);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey2));

  val = NULL;
  ConfigVal *cfgval3 = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval3);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey3));
  delete ikey;
}
TEST(ValidateMessage, ValidateMessage_03) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vbr.ValidateMessage(req, ikey));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vbr.ValidateMessage(req, ikey));

  req->option2 = UNC_OPT2_NONE;
  req->option1 = UNC_OPT1_NORMAL;

  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey1));

  ConfigVal *cfgval2 = NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey2));

  val_vbr_if *val1=NULL;
  ConfigVal *cfgval3 = new ConfigVal(IpctSt::kIpcStValVbrIf, val1);
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval3);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey3));
}
TEST(ValidateMessage, ValidateMessage_04) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vbr.ValidateMessage(req, ikey));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vbr.ValidateMessage(req, ikey));

  req->option2 = UNC_OPT2_NONE;
  req->option1 = UNC_OPT1_NORMAL;

  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey1));

  ConfigVal *cfgval2 = NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey2));

  val_vbr_if *val1=NULL;
  ConfigVal *cfgval3 = new ConfigVal(IpctSt::kIpcStValVbrIf, val1);
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval3);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey3));
}

TEST(ValidateMessage, ValidateMessage_06) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vbr.ValidateMessage(req, ikey));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vbr.ValidateMessage(req, ikey));

  req->option2 = UNC_OPT2_NONE;
  req->option1 = UNC_OPT1_NORMAL;

  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey1));

  ConfigVal *cfgval2 = NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey2));

  val_vbr_if *val1=NULL;
  ConfigVal *cfgval3 = new ConfigVal(IpctSt::kIpcStValVbrIf, val1);
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval3);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey3));
  req->operation = UNC_OP_DELETE;
  ConfigVal *cfgval4 = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey4 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval4);
}
TEST(ValidateMessage, ValidateMessage_05) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vbr.ValidateMessage(req, ikey));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey));

  req->option2 = UNC_OPT2_NONE;
  req->option1 = UNC_OPT1_NORMAL;

  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey1));

  ConfigVal *cfgval2 = NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey2));

  val_vbr_if *val1=NULL;
  ConfigVal *cfgval3 = new ConfigVal(IpctSt::kIpcStValVbrIf, val1);
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval3);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey3));
  req->option2 = UNC_OPT2_L2DOMAIN;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vbr.ValidateMessage(req, ikey3));

}
TEST(ValidateMessage, ValidateMessage_07) {

  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_DELETE;

  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey1));
}
TEST(GetVbrIfFromVexternal,TmpckvNonNull){
  VbrIfMoMgr obj;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if *key;
  val_vbr_if *val;
  uint8_t vtnname=1;
  uint8_t vexternal1=1;
  uint8_t *vexternal=&vexternal1;
  uint8_t *vtn1=&vtnname;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
  IpctSt::kIpcStKeyVbrIf,key,cfgval);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetVbrIfFromVExternal(vtn1,vexternal,ikey,dmi));

delete ikey;
}

TEST(GetVbrIfFromVexternal,TmpckvNull){
  VbrIfMoMgr obj;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if *key;
  val_vbr_if *val;
  uint8_t vtnname=1;
  uint8_t vexternal1=1;
  uint8_t *vexternal=&vexternal1;
  uint8_t *vtn1=&vtnname;
  GetKeyValStruct(key, val);
  ConfigKeyVal *ikey =NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetVbrIfFromVExternal(vtn1,vexternal,ikey,dmi));
}
TEST(PortStatusHandler,TrueOperStatus){
  VbrIfMoMgr obj;
  DalDmlIntf * dmi= new DalOdbcMgr();
  const char *ctrlr_name="PFC_1";
  const char * domain_name="DOMAIN_1";
  const char *port_id="VLAN";
  bool oper_status = true;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.PortStatusHandler(ctrlr_name,domain_name,port_id,oper_status,dmi));
}

TEST(PortStatusHandler,FalseOperStatus){
  VbrIfMoMgr obj;
  val_vbr_if * val;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if * key;
  GetKeyValStruct(key,val);
  const char *ctrlr_name="PFC_1";
  const char * domain_name="DOMAIN_1";
  const char *port_id="VLAN";
  bool oper_status = UPLL_OPER_STATUS_UP;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.PortStatusHandler(ctrlr_name,domain_name,port_id,oper_status,dmi));
  DalOdbcMgr::clearStubData();
}
TEST(PortStatusHandler,FalseOperStatus_02){
  VbrIfMoMgr obj;
  val_vbr_if * val;
  DalDmlIntf * dmi= new DalOdbcMgr();
  key_vbr_if * key;
  GetKeyValStruct(key,val);
  const char *ctrlr_name="PFC_1";
  const char * domain_name="DOMAIN_1";
  const char *port_id="VLAN";
  bool oper_status = UPLL_OPER_STATUS_DOWN;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.PortStatusHandler(ctrlr_name,domain_name,port_id,oper_status,dmi));
  DalOdbcMgr::clearStubData();
}
TEST(UpdateConfigVal, UpdateConfigVal_07) {
  VbrIfMoMgr vbr;
  upll_keytype_datatype_t datatype=UPLL_DT_CANDIDATE;
  key_vbr_if *key;
  val_vbr_if *val;

  DalDmlIntf * dmi= new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.UpdateConfigVal(ikey,datatype,dmi));
}
TEST(vbrif_momgr_test,CompareValidValue_auditTrue) {
  VbrIfMoMgr obj;
  val_vbr_if *val1 =
  reinterpret_cast<val_vbr_if *>(malloc(sizeof(val_vbr_if)));
  val_vbr_if *val2 =
  reinterpret_cast<val_vbr_if *>(malloc(sizeof(val_vbr_if)));

  for ( unsigned int loop = 0; loop < sizeof(val1->valid);
  ++loop ) {
  val1->valid[loop] = UNC_VF_INVALID;
  val2->valid[loop] = UNC_VF_VALID;
  }
  obj.CompareValidValue((void*&)val1, (void*)val2, true);
  for ( unsigned int loop = 0; loop < sizeof(val1->valid);
  ++loop ) {
  EXPECT_EQ(UNC_VF_VALID_NO_VALUE, val1->valid[loop]);
}
free(val1); free(val2);
}
TEST(vbrif_momgr_test,CompareValidValue_auditfalse) {
  VbrIfMoMgr obj;
  val_vbr_if *val1 =
  reinterpret_cast<val_vbr_if *>(malloc(sizeof(val_vbr_if)));
  val_vbr_if *val2 =
  reinterpret_cast<val_vbr_if *>(malloc(sizeof(val_vbr_if)));
  val1->valid[1] = UNC_VF_VALID;
  val2->valid[1] = UNC_VF_VALID;
  val1->admin_status = 1;
  val2->admin_status = 1;
  obj.CompareValidValue((void*&)val1, (void*)val2, false);
}

TEST(vbrif_momgr_test,CompareValidValue_Invalid_case1) {
  VbrIfMoMgr obj;
  val_vbr_if *val1 =
  reinterpret_cast<val_vbr_if *>(malloc(sizeof(val_vbr_if)));
  memset(val1,0 ,sizeof(val_vbr_if));
  val_vbr_if *val2 =
  reinterpret_cast<val_vbr_if *>(malloc(sizeof(val_vbr_if)));
  memset(val2,0 ,sizeof(val_vbr_if));
  for (unsigned int loop = 0;loop < sizeof(val1->valid) / sizeof(uint8_t); ++loop) {
    val1->valid[loop] = UNC_VF_VALID;
    val2->valid[loop] = UNC_VF_VALID;
  }
  obj.CompareValidValue((void*&)val1, (void*)val2, false);
}
}
