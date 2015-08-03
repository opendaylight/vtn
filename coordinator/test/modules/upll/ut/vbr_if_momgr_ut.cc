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
#include <pfc/util.h>
#include <pfc/ipc.h>
#include <pfc/ipc_struct.h>
#include <unc/keytype.h>
#include <pfcxx/synch.hh>
#include <vtn_momgr.hh>
#include <vbr_momgr.hh>
#include <vbr_if_momgr.hh>
#include <unc/keytype.h>
#include <config_mgr.hh>
#include <dal_odbc_mgr.hh>
#include <dal_dml_intf.hh>
#include <capa_intf.hh>
#include <capa_module_stub.hh>
#include <tclib_module.hh>
#include <ctrlr_mgr.hh>
#include <momgr_intf_stub.hh>
#include <dal_cursor.hh>
#include <momgr_intf.hh>
#include <momgr_impl.hh>
#include "ut_util.hh"
// bool fatal_done;
// pfc::core::Mutex  fatal_mutex_lock;

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

class VbrIfMoMgrTest
  : public UpllTestEnv
{
};

static void GetKeyStruct(key_vbr_if *&kst) {
  const char *vtn_name = "VTN_1";
  const char *if_name = "IF_1";
  const char *vbr_name = "VBR_1";

  kst = ZALLOC_TYPE(key_vbr_if);
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(kst->vbr_key.vbridge_name),
          vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(kst->if_name),
          if_name, strlen(if_name)+1);
}

static void GetValStruct(val_vbr_if *&vst)
{
  const char *desc = "thisisvbridge";
  const char *logical_port_id ="lport1";

  vst = ZALLOC_TYPE(val_vbr_if);
  for (unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->valid); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  vst->cs_row_status = UNC_VF_VALID;

  for (unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->cs_attr);
       ++loop) {
    vst->cs_attr[loop] = UNC_CS_APPLIED;
  }

  vst->admin_status = UPLL_ADMIN_ENABLE;
  strncpy(reinterpret_cast<char *>(vst->description), desc, strlen(desc)+1);
  vst->portmap.vlan_id=1;
  vst->portmap.tagged=1;
  for (unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->portmap.valid);
       ++loop) {
    vst->portmap.valid[loop] = UNC_VF_VALID;
  }

  for (unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->portmap.valid);
       ++loop) {
    vst->portmap.cs_attr[loop] = UNC_CS_APPLIED;
  }
  strncpy(reinterpret_cast<char *>(vst->portmap.logical_port_id),
          logical_port_id, strlen(logical_port_id)+1);
}

static void GetKeyValStruct(key_vbr_if *&kst, val_vbr_if *&vst)
{
  GetKeyStruct(kst);
  GetValStruct(vst);
}

static void GetValDrvStruct(val_drv_vbr_if *&vst)
{
  const char *vex_name = "Vex_1";
  const char *vex_if_name = "Vex if_1";
  const char *vex_link_name = "Vex link_1";
  const char *desc = "thisisvbridge";
  const char *logical_port_id ="lport1";

  vst = ZALLOC_TYPE(val_drv_vbr_if);
  strncpy(reinterpret_cast<char *>(vst->vex_name), vex_name,
  strlen(vex_name)+1);
  strncpy(reinterpret_cast<char *>(vst->vex_if_name), vex_if_name,
  strlen(vex_if_name)+1);
  strncpy(reinterpret_cast<char *>(vst->vex_link_name), vex_link_name,
  strlen(vex_link_name)+1);

  for (unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->valid); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  for (unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->vbr_if_val.valid);
       ++loop) {
    vst->vbr_if_val.valid[loop] = UNC_VF_VALID;
  }

  vst->vbr_if_val.cs_row_status = UNC_VF_VALID;

  for (unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->vbr_if_val.valid);
       ++loop) {
    vst->vbr_if_val.cs_attr[loop] = UNC_CS_APPLIED;
  }

  vst->vbr_if_val.admin_status=UPLL_ADMIN_ENABLE;
  strncpy(reinterpret_cast<char *>(vst->vbr_if_val.description), desc,
          strlen(desc)+1);
  vst->vbr_if_val.portmap.vlan_id=1;
  vst->vbr_if_val.portmap.tagged=0;

  for (unsigned int loop = 0;
       loop < PFC_ARRAY_CAPACITY(vst->vbr_if_val.portmap.valid); ++loop) {
    vst->vbr_if_val.portmap.valid[loop] = UNC_VF_VALID;
  }
  for (unsigned int loop = 0;
       loop < PFC_ARRAY_CAPACITY(vst->vbr_if_val.portmap.valid); ++loop) {
    vst->vbr_if_val.portmap.cs_attr[loop] = UNC_CS_APPLIED;
  }

  strncpy(reinterpret_cast<char *>(vst->vbr_if_val.portmap.logical_port_id),
          logical_port_id, strlen(logical_port_id)+1);
}

static void GetKeyValDrvStruct(key_vbr_if *&kst, val_drv_vbr_if *&vst)
{
  GetKeyStruct(kst);
  GetValDrvStruct(vst);
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
  key_vbr_if *key(NULL);
  val_vbr_if *val=NULL;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
  delete req;
  delete okey;
}

TEST_F(VbrIfMoMgrTest, IsValidKey_InvalidIndex) {
  VbrIfMoMgr obj;
  uint64_t index = uudst::vbridge_interface::kDbiCtrlrName;
  key_vbr_if *key;
  GetKeyStruct(key);

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
  key = ZALLOC_TYPE(key_vbr_if);
  strncpy(reinterpret_cast<char *>(key->vbr_key.vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbr_key.vbridge_name),
          vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(key->if_name),
          if_name, strlen(if_name)+1);

  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key, NULL));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  strncpy(reinterpret_cast<char *>(key->vbr_key.vtn_key.vtn_name),"VTN_1",32);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key1, NULL));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey1));
  delete ikey;
  delete ikey1;
  delete okey;
}

TEST_F(VbrIfMoMgrTest, CopyToConfigkey_InValidName_01) {
  VbrIfMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr_if *key;
  const char *vtn_name = "vtn1";
  const char *vbr_name = "";
  const char *if_name = "";
  key = ZALLOC_TYPE(key_vbr_if);
  strncpy(reinterpret_cast<char *>(key->vbr_key.vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vbr_key.vbridge_name),
          vbr_name, strlen(vbr_name)+1);
  strncpy(reinterpret_cast<char *>(key->if_name),
          if_name, strlen(if_name)+1);

  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key, NULL));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey1));
  delete ikey1;
  delete okey;
}

TEST_F(VbrIfMoMgrTest, CompareValidValue_AuditTrue) {
  VbrIfMoMgr obj;
  val_drv_vbr_if *val, *val1;
  GetValDrvStruct(val);
  GetValDrvStruct(val1);

  void *vbrval = reinterpret_cast<void *>(val);
  ASSERT_TRUE(obj.CompareValidValue(vbrval, (void *)val1, true));
  free(val);
  free(val1);
}

TEST_F(VbrIfMoMgrTest, CompareValidValue_01) {
  VbrIfMoMgr obj;
  val_drv_vbr_if *val, *val1, *val2;
  GetValDrvStruct(val);
  GetValDrvStruct(val1);
  GetValDrvStruct(val2);

  void *vbrval = reinterpret_cast<void *>(val);
  val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  val->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  val->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  val1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  val1->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  val1->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  ASSERT_FALSE(obj.CompareValidValue(vbrval, (void *)val1, true));

  val->valid[PFCDRV_IDX_VAL_VBRIF] = UNC_VF_VALID;
  val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  val->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  val->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
  ASSERT_FALSE(obj.CompareValidValue(vbrval, (void *)val2, true));

  free(val);
  free(val1);
  free(val2);
}

TEST_F(VbrIfMoMgrTest, CompareValidValue_02) {
  VbrIfMoMgr obj;
  val_drv_vbr_if *val, *val1;
  GetValDrvStruct(val);
  GetValDrvStruct(val1);

  void *vbrval = reinterpret_cast<void *>(val);
  val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
  val1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
  ASSERT_TRUE(obj.CompareValidValue(vbrval, (void *)val1, true));

  free(val);
  free(val1);
}

TEST_F(VbrIfMoMgrTest, FilterAttributes_CreateOperation) {
  VbrIfMoMgr vbr;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vbr_if_t *valvbr1(ZALLOC_TYPE(val_vbr_if));
  valvbr1->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  EXPECT_EQ(PFC_FALSE, vbr.FilterAttributes(val1,val2,audit_status,op));

  free(valvbr1);
}

TEST_F(VbrIfMoMgrTest, FilterAttributes_OperationUpdate) {
  VbrIfMoMgr vbr;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_UPDATE;

  val_drv_vbr_if_t *valvbr1(ZALLOC_TYPE(val_drv_vbr_if));
  val_drv_vbr_if_t *valvbr2(ZALLOC_TYPE(val_drv_vbr_if_t));
  valvbr1->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  valvbr2->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvbr2));
  EXPECT_EQ(true, vbr.FilterAttributes(val1,val2,audit_status,op));

  free(valvbr1);
  free(valvbr2);
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
  IPC_REQ_RESP_HEADER_DECL(req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(req, ikey, NULL));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateCapability_Success) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  CtrlrMgr::Ctrlr ctrlrobj("CTR_1", UNC_CT_PFC, "5.0", true);
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

  req->operation = UNC_OP_READ_SIBLING_COUNT;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValidateCapability(req, ikey, "CTR_1"));
  req->operation = UNC_OP_DELETE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, "CTR_1"));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateCapability_ikey_NULL) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req=NULL;
  const char *ctrlr_name="ctr1";
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateCapability(req, ikey, ctrlr_name));
}

TEST_F(VbrIfMoMgrTest, ValidateCapability_SUCCESS) {
  VbrIfMoMgr obj;
  const char * ctrlr_name = reinterpret_cast<const char *>("PFC222");
  key_vbr_if *vbrif_key(ZALLOC_TYPE(key_vbr_if));
  strcpy((char*)vbrif_key->vbr_key.vtn_key.vtn_name,(char*)"vtn1");
  strcpy((char*)vbrif_key->vbr_key.vbridge_name,(char*)"VRT31");
  strcpy((char*)vbrif_key->if_name,(char*)"VRTIF11");

  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_UPDATE;
  req->datatype =  UPLL_DT_CANDIDATE;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, vbrif_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey, ctrlr_name));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, AdaptValToVtnService_Success_01) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfg_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey, ADAPT_ONE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, AdaptValToVtnService_Success_02) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_INVALID;
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfg_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey, ADAPT_ONE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, AdaptValToVtnService_Success_03) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbrIfSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfg_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey, ADAPT_ONE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, AdaptValToVtnService_Failure) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigKeyVal* ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey, ADAPT_ONE));

  ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                          IpctSt::kIpcStKeyVbr,
                          key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey, ADAPT_ONE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_01) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);

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
}

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_02) {
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

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_03) {
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

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_04) {
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

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_05) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  memset(attrs, 0xff, sizeof(attrs));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  val->admin_status=UPLL_ADMIN_ENABLE;
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  attrs[unc::capa::vbr_if::kCapAdminStatus]=0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_06) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  memset(attrs, 0xff, sizeof(attrs));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID_NO_VALUE;
  attrs[unc::capa::vbr_if::kCapAdminStatus]=0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_07) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  memset(attrs, 0xff, sizeof(attrs));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID_NO_VALUE;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  attrs[unc::capa::vbr_if::kCapLogicalPortId]=0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_08) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  memset(attrs, 0xff, sizeof(attrs));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  attrs[unc::capa::vbr_if::kCapLogicalPortId]=0;
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_09) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  memset(attrs, 0xff, sizeof(attrs));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID_NO_VALUE;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  attrs[unc::capa::vbr_if::kCapVlanId]=0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_10) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[10];
  memset(attrs, 0xff, sizeof(attrs));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  attrs[unc::capa::vbr_if::kCapVlanId]=0;
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_11) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[20];
  memset(attrs, 0xff, sizeof(attrs));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  attrs[unc::capa::vbr_if::kCapTagged]=0;
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_12) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[20];
  memset(attrs, 0xff, sizeof(attrs));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  attrs[unc::capa::vbr_if::kCapTagged]=0;
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_STATE));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValVbrAttributeSupportCheck_13) {
  VbrIfMoMgr vbr;
  key_vbr_if *kst;
  val_vbr_if *vst=NULL;
  GetKeyStruct(kst);

  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs1=0;
  uint8_t *attrs=&attrs1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, vst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, kst, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValVbrIfAttributeSupportCheck(attrs,ikey,operation,UPLL_DT_IMPORT));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, GetVexternal_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  GetKeyValDrvStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  const char *ctrlr_id = "pfc1";
  uint8_t vexternal1=1;
  uint8_t *vexternal=&vexternal1;
  uint8_t vex_if1=1;
  uint8_t *vex_if=&vex_if1;
  InterfacePortMapInfo iftype;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetVexternal(ikey,dt_type,dmi,
                                    vexternal,vex_if,iftype, ctrlr_id));

  delete ikey;
}

# if 0
TEST_F(VbrIfMoMgrTest, GetVexternal_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  upll_keytype_datatype_t dt_type = UPLL_DT_AUDIT;
  GetKeyValDrvStruct(key, val);
  ConfigVal *config_val =  new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(
    UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t vexternal1 = 1;
  uint8_t *vexternal = &vexternal1;
  uint8_t vex_if1 = 1;
  uint8_t *vex_if = &vex_if1;
  InterfacePortMapInfo iftype;
  DalOdbcMgr::stub_setResultcode(
    DalOdbcMgr::SINGLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS,
     vbr.GetVexternal(ikey, dt_type, dmi, vexternal, vex_if, iftype));

  delete ikey;
}
# endif

TEST_F(VbrIfMoMgrTest, GetVexternal_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  const char *ctrlr_id = "pfc1";
  uint8_t vexternal1=1;
  uint8_t *vexternal=&vexternal1;
  uint8_t vex_if1=1;
  uint8_t *vex_if=&vex_if1;
  InterfacePortMapInfo iftype;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetVexternal(ikey,dt_type,dmi,vexternal,vex_if,iftype, ctrlr_id));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, GetVexternal_04) {
  VbrIfMoMgr vbr;
  key_vbr_if *key=NULL;
  val_vbr_if *val=NULL;
  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  const char *ctrlr_id = "pfc1";
  uint8_t vexternal1=1;
  uint8_t *vexternal=&vexternal1;
  uint8_t vex_if1=1;
  uint8_t *vex_if=&vex_if1;
  InterfacePortMapInfo iftype;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.GetVexternal(ikey,dt_type,dmi,vexternal,vex_if,iftype, ctrlr_id));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, IsReferenced_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  //upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  GetKeyValStruct(key, val);
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.IsReferenced(req,ikey,dmi));

  delete ikey;
}
/*
TEST_F(VbrIfMoMgrTest, IsReferenced_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  //upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  GetKeyValStruct(key, val);
  IpcReqRespHeader *req = NULL;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.IsReferenced(req,ikey,dmi));

  delete ikey;
}
*/
#if 0
TEST_F(VbrIfMoMgrTest, IsReferenced_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  //DalOdbcMgr::stub_getMappedResultCode(map);
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.IsReferenced(ikey,dt_type,dmi));

  delete ikey;
}
#endif
/*
TEST_F(VbrIfMoMgrTest, IsReferenced_04) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  //upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  GetKeyValStruct(key, val);
  IpcReqRespHeader *req = NULL;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.IsReferenced(req,ikey,dmi));

  delete ikey;
}
*/
TEST_F(VbrIfMoMgrTest, UpdateMo_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, vbr.UpdateMo(req,ikey,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, UpdateMo_02) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.UpdateMo(req,ikey,dmi));
}

TEST_F(VbrIfMoMgrTest, UpdateMo_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);

  key_user_data_t *user_data(ZALLOC_TYPE(key_user_data_t));
  uuu::upll_strncpy(user_data->ctrlr_id,"ctr_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.UpdateMo(req,ikey,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, UpdateMo_04) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vbr.UpdateMo(req,ikey,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, UpdateMo_05) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  key_user_data_t *user_data(ZALLOC_TYPE(key_user_data_t));
  uuu::upll_strncpy(user_data->ctrlr_id,"ctr_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vbr.UpdateMo(req,ikey,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateAttribute_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateAttribute(ikey,dmi,req));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateAttribute_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *kst;
  val_vbr_if *vst=NULL;
  GetKeyStruct(kst);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, vst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, kst, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateAttribute(ikey,dmi,req));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateAttribute_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *kst;
  val_vbr_if *vst=NULL;
  GetKeyStruct(kst);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, vst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, kst, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateAttribute(ikey,dmi,req));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateAttribute_04) {
  VbrIfMoMgr vbr;
  key_vbr_if *kst;
  val_vbr_if *vst=NULL;
  GetKeyStruct(kst);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, vst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, kst, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateAttribute(ikey,dmi,req));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, CopyToConfigkey_01) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.CopyToConfigKey(okey,ikey));
}

TEST_F(VbrIfMoMgrTest, CopyToConfigkey_02) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.CopyToConfigKey(okey,ikey));

  delete ikey;
  delete okey;
}

TEST_F(VbrIfMoMgrTest, CopyToConfigkey_03) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey=NULL;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.CopyToConfigKey(okey,ikey));

  delete ikey;
  delete okey;
}

TEST_F(VbrIfMoMgrTest, CopyToConfigkey_04) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey=NULL;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.CopyToConfigKey(okey,ikey));

  delete ikey;
  delete okey;
}

TEST_F(VbrIfMoMgrTest, CopyToConfigkey_05) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey=NULL;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  key_rename_vnode_info *key_rename(ZALLOC_TYPE(key_rename_vnode_info));
  pfc_strlcpy(reinterpret_cast<char *>(key_rename->old_unc_vtn_name),
              reinterpret_cast<const char *>(key->vbr_key.vtn_key.vtn_name),
              sizeof(key_rename->old_unc_vtn_name));
  pfc_strlcpy(reinterpret_cast<char *>(key_rename->old_unc_vnode_name),
              reinterpret_cast<const char *>(key->vbr_key.vbridge_name),
              sizeof(key_rename->old_unc_vnode_name));
  free(key);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbrIf, key_rename, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.CopyToConfigKey(okey,ikey));

  delete ikey;
  delete okey;
}

TEST_F(VbrIfMoMgrTest, UpdateConfigVal_01) {
  VbrIfMoMgr vbr;
  upll_keytype_datatype_t datatype=UPLL_DT_CANDIDATE;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  DalDmlIntf *dmi(getDalDmlIntf());
  GetKeyValDrvStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name("vtn1");
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.UpdateConfigVal(ikey,datatype,dmi,
                                                    config_mode,vtn_name));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, UpdateConfigVal_03) {
  VbrIfMoMgr vbr;
  upll_keytype_datatype_t datatype=UPLL_DT_CANDIDATE;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  DalDmlIntf *dmi(getDalDmlIntf());
  GetKeyValDrvStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name("vtn1");
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.UpdateConfigVal(ikey,datatype,dmi,
                                                config_mode,vtn_name));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateVbrifKey_01) {
  VbrIfMoMgr vbr;
  unc_keytype_operation_t operation=UNC_OP_CREATE;
  key_vbr_if_t *key(ZALLOC_TYPE(key_vbr_if_t));
  strncpy((char*) key->if_name,"IF_1",32);
  strncpy((char*) key->vbr_key.vbridge_name," ",32);
  strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",32);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.ValidateVbrifKey(key,operation));

  free(key);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrifKey_02) {
  VbrIfMoMgr vbr;
  unc_keytype_operation_t operation=UNC_OP_CREATE;
  key_vbr_if_t *key(ZALLOC_TYPE(key_vbr_if_t));
  strncpy((char*) key->if_name," ",32);
  strncpy((char*) key->vbr_key.vbridge_name,"vbr ",32);
  strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",32);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,vbr.ValidateVbrifKey(key,operation));

  free(key);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_01) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->description[0] = '\0';
  val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_02) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_03) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_04) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_INVALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_05) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  val->portmap.logical_port_id[0] = '\0';
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_06) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_07) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val->portmap.vlan_id = 0;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_08) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_09) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_10) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_11) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  val->admin_status = 0;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_12) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  val->portmap.logical_port_id[0] = 'S';
  val->portmap.logical_port_id[1] = 'W';
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_13) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  val->portmap.tagged = 10;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVbrIfValue_14) {
  VbrIfMoMgr vbr;
  val_vbr_if *val;
  GetValStruct(val);

  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVbrIfValue(val, op));

  free(val);
}

TEST_F(VbrIfMoMgrTest, ValidateVtnNeighborValue_01) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = "VTN_1";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = "VBR_1";
  val_vtn_neighbor *vtn_neighbor(ZALLOC_TYPE(val_vtn_neighbor));
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
  vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));

  free(vtn_neighbor);
}

TEST_F(VbrIfMoMgrTest, ValidateVtnNeighborValue_02) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = " ";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = "VBR_1";
  val_vtn_neighbor *vtn_neighbor(ZALLOC_TYPE(val_vtn_neighbor));
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
  vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));

  free(vtn_neighbor);
}

TEST_F(VbrIfMoMgrTest, ValidateVtnNeighborValue_03) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = "VTN_1";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = "VBR_1";
  val_vtn_neighbor *vtn_neighbor(ZALLOC_TYPE(val_vtn_neighbor));
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
  vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));

  free(vtn_neighbor);
}

TEST_F(VbrIfMoMgrTest, ValidateVtnNeighborValue_04) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = "VTN_1";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = "VBR_1";
  val_vtn_neighbor *vtn_neighbor(ZALLOC_TYPE(val_vtn_neighbor));
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
  vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));

  free(vtn_neighbor);
}

TEST_F(VbrIfMoMgrTest, ValidateVtnNeighborValue_05) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = "VTN_1";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = "VBR_1";
  val_vtn_neighbor *vtn_neighbor(ZALLOC_TYPE(val_vtn_neighbor));
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
  vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] = UNC_VF_VALID_NO_VALUE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));

  free(vtn_neighbor);
}

TEST_F(VbrIfMoMgrTest, ValidateVtnNeighborValue_06) {
  VbrIfMoMgr vbr;
  const char *connected_vnode_name = "VTN_1";
  const char *connected_if_name = "IF_1";
  const char *connected_vlink_name = " ";
  val_vtn_neighbor *vtn_neighbor(ZALLOC_TYPE(val_vtn_neighbor));
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
  vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] = UNC_VF_VALID;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateVtnNeighborValue(vtn_neighbor, op));

  free(vtn_neighbor);
}

#if 0
TEST_F(VbrIfMoMgrTest, IsLogicalPortAndVlanIdInUse_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_IMPORT;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC , vbr.IsLogicalPortAndVlanIdInUse<val_vbr_if_t>(ikey,dmi,req));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, IsLogicalPortAndVlanIdInUse_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  IPC_REQ_RESP_HEADER_DECL(req);
  GetKeyValStruct(key, val);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.IsLogicalPortAndVlanIdInUse<val_vbr_if_t>(ikey,dmi,req));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, IsLogicalPortAndVlanIdInUse_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  IPC_REQ_RESP_HEADER_DECL(req);
  GetKeyValStruct(key, val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);

  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.IsLogicalPortAndVlanIdInUse<val_vbr_if_t> (ikey,dmi,req));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, IsLogicalPortAndVlanIdInUse_04) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_IMPORT;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  GetKeyValStruct(key, val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);

  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.IsLogicalPortAndVlanIdInUse<val_vbr_if_t> (ikey,dmi,req));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, IsLogicalPortAndVlanIdInUse_05) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_IMPORT;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  GetKeyValStruct(key, val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);


  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM]=UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.IsLogicalPortAndVlanIdInUse<val_vbr_if_t>(ikey,dmi,req));

  delete ikey;
}
#endif
#if 0
TEST_F(VbrIfMoMgrTest, GetBoundaryInterfaces_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vnode_if_t boundary_if;
  memset(&boundary_if, 0, sizeof(boundary_if));
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vbr.GetBoundaryInterfaces(boundary_if,dmi,ikey));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, SetBoundaryIfOperStatusforPathFault_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  const set<key_vnode_if_t, key_vnode_if_compare> boundary_if_set;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.SetBoundaryIfOperStatusforPathFault(boundary_if_set,kPathFault, dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, SetBoundaryIfOperStatusforPathFault_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  const set<key_vnode_if_t, key_vnode_if_compare> boundary_if_set;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.SetBoundaryIfOperStatusforPathFault(boundary_if_set,kPathFault,dmi));

  delete ikey;
}
#endif
/*
TEST_F(VbrIfMoMgrTest, CreateAuditMoImpl_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.CreateAuditMoImpl(ikey,dmi,ctrlr_name));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, CreateAuditMoImpl_02) {
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  const char *ctrlr_name ="ctr1";
  IPC_REQ_RESP_HEADER_DECL(req);
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
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CreateAuditMoImpl(ikey, dmi,ctrlr_name));

  delete ikey;
}
*/
TEST_F(VbrIfMoMgrTest, CreateAuditMoImpl_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbrIf,
                            key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.CreateAuditMoImpl(ikey,dmi,ctrlr_name));

  delete ikey;
}
/*
TEST_F(VbrIfMoMgrTest, CreateAuditMoImpl_04) {
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  const char *ctrlr_id ="pfc001";
  IPC_REQ_RESP_HEADER_DECL(req);
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

  delete ikey;
}
*/
#if 0
TEST_F(VbrIfMoMgrTest, GetMappedVbridges_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name = "ctr1";
  const char *domain_id = "dom1";
  std::string logportid;
  set<key_vnode_type_t, key_vnode_type_compare> sw_vbridge_set;
  GetKeyValStruct(key, val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
     vbr.GetMappedVbridges(ctrlr_name, domain_id,
     logportid, dmi, &sw_vbridge_set));
}

TEST_F(VbrIfMoMgrTest, GetMappedVbridges_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name = "ctr1";
  const char *domain_id = "dom1";
  std::string logportid;
  set<key_vnode_type_t, key_vnode_type_compare> sw_vbridge_set;
  GetKeyValStruct(key, val);
  ConfigVal *config_val =  new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(
    UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(
    DalOdbcMgr::MULTIPLE, kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_SUCCESS,
     vbr.GetMappedVbridges(ctrlr_name, domain_id,
     logportid, dmi, &sw_vbridge_set));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, GetMappedVbridges_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name = "ctr1";
  const char *domain_id = "dom1";
  std::string logportid;
  set<key_vnode_type_t, key_vnode_type_compare> sw_vbridge_set;
  GetKeyValStruct(key, val);
  ConfigVal *config_val =  new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(
    UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(
    DalOdbcMgr::MULTIPLE, kDalRcSuccess);

  std::map<uint8_t, DalResultCode> map;
  map.insert(std::make_pair(1, kDalRcRecordNoMore));
  map.insert(std::make_pair(0, kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  EXPECT_EQ(UPLL_RC_SUCCESS,
     vbr.GetMappedVbridges(ctrlr_name, domain_id,
     logportid, dmi, &sw_vbridge_set));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, GetMappedVbridges_04) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name = "ctr1";
  const char *domain_id = "dom1";
  std::string logportid;
  set<key_vnode_type_t, key_vnode_type_compare> sw_vbridge_set;
  GetKeyValStruct(key, val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
     vbr.GetMappedVbridges(ctrlr_name, domain_id,
     logportid, dmi, &sw_vbridge_set));
}


TEST_F(VbrIfMoMgrTest, PathFaultHandler_03) {
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
  DalDmlIntf *dmi(getDalDmlIntf());

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.PathFaultHandler(ctrlr_name,domain_id,ingress_ports,egress_ports,alarm_asserted,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, PathFaultHandler_04) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  const char *ctrlr_name="ctr1";
  const char *domain_id="dom1";
  std::vector<std::string> ingress_ports;
  std::vector<std::string> egress_ports;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.PathFaultHandler(ctrlr_name,domain_id,ingress_ports,egress_ports,false,dmi));

  delete ikey;
}

#endif
TEST_F(VbrIfMoMgrTest, TxUpdateController_NULL) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;

  set<std::string> affected_ctrlr_set;
  DalDmlIntf *dmi(getDalDmlIntf());
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcRecordNoMore);
  TxUpdateUtil *tx_util = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name("vtn1");
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.TxUpdateController(UNC_KT_VBR_IF,
                                session_id,config_id,phase,&affected_ctrlr_set,
                                dmi,&ikey,tx_util,config_mode,vtn_name));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, TxUpdateController_default) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  uint8_t *ctr_id1(ZALLOC_ARRAY(uint8_t, 32));
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1(ZALLOC_ARRAY(uint8_t, 32));
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom(ZALLOC_TYPE(controller_domain_t));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;

  set<std::string> affected_ctrlr_set;
  affected_ctrlr_set.insert
                  (string(reinterpret_cast<char *>(ctrlr_dom->ctrlr)));
  DalDmlIntf *dmi(getDalDmlIntf());
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  TxUpdateUtil *tx_util = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name("vtn1");
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.TxUpdateController(UNC_KT_VBR_IF,
                            session_id,config_id,phase,&affected_ctrlr_set,
                            dmi,&ikey,tx_util,config_mode,vtn_name));

  free(ctr_id1);
  free(dom_id1);
  free(ctrlr_dom);
  delete ikey;
}
/*
TEST_F(VbrIfMoMgrTest, TxUpdateController_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
  uint8_t *ctr_id1(ZALLOC_ARRAY(uint8_t, 32));
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1(ZALLOC_ARRAY(uint8_t, 32));
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom(ZALLOC_TYPE(controller_domain_t));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;

  set<std::string> affected_ctrlr_set;
  affected_ctrlr_set.insert
                  (string(reinterpret_cast<char *>(ctrlr_dom->ctrlr)));
  DalDmlIntf *dmi(getDalDmlIntf());
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS,kDalRcSuccess);
  TxUpdateUtil *tx_util = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name("vtn1");
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);

  key_user_data_t *user_data(ZALLOC_TYPE(key_user_data_t));
  uuu::upll_strncpy(user_data->ctrlr_id,"ctr_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.TxUpdateController(UNC_KT_VBR_IF,
                            session_id,config_id,phase,&affected_ctrlr_set,
                            dmi,&ikey,tx_util,config_mode,vtn_name));

  free(ctr_id1);
  free(dom_id1);
  free(ctrlr_dom);
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, TxUpdateController_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpDelete;
  uint8_t *ctr_id1(ZALLOC_ARRAY(uint8_t, 32));
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1(ZALLOC_ARRAY(uint8_t, 32));
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom(ZALLOC_TYPE(controller_domain_t));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;

  set<std::string> affected_ctrlr_set;
  affected_ctrlr_set.insert
                  (string(reinterpret_cast<char *>(ctrlr_dom->ctrlr)));
  DalDmlIntf *dmi(getDalDmlIntf());
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  TxUpdateUtil *tx_util = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name("vtn1");
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.TxUpdateController(UNC_KT_VBR_IF,
                              session_id,config_id,phase,&affected_ctrlr_set,
                              dmi,&ikey,tx_util,config_mode,vtn_name));

  free(ctr_id1);
  free(dom_id1);
  free(ctrlr_dom);
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, TxUpdateController_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpDelete;

  set<std::string> affected_ctrlr_set;
  DalDmlIntf *dmi(getDalDmlIntf());
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  TxUpdateUtil *tx_util = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name("vtn1");
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.TxUpdateController(UNC_KT_VBR_IF,
                            session_id,config_id,phase,&affected_ctrlr_set,
                            dmi,&ikey,tx_util,config_mode,vtn_name));

  delete ikey;
}
*/
TEST_F(VbrIfMoMgrTest, AuditUpdateController_01) {
  VbrIfMoMgr vbr;
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  const char *ctrlr_id="Controller1";

  DalDmlIntf *dmi(getDalDmlIntf());
  KTxCtrlrAffectedState ctrlr_affected = kCtrlrAffectedNoDiff;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS,kDalRcSuccess);
  ConfigKeyVal *err_ckv = NULL;
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, vbr.AuditUpdateController(UNC_KT_VBR_IF,ctrlr_id,
                    session_id,config_id,phase,dmi,&err_ckv,&ctrlr_affected));
}

#if 0
TEST_F(VbrIfMoMgrTest, AuditUpdateController_02) {
  VbrIfMoMgr vbr;
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  const char *ctrlr_id="Controller1";

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcGeneralError));
  map.insert(std::make_pair(0,kDalRcSuccess));
  KTxCtrlrAffectedState ctrlr_affected = kCtrlrAffectedNoDiff;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *err_ckv = NULL;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.AuditUpdateController(UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, dmi,&err_ckv,&ctrlr_affected));
}
#endif

TEST_F(VbrIfMoMgrTest, AuditUpdateController_03) {
  VbrIfMoMgr vbr;
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  const char *ctrlr_id="Controller1";

  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *err_ckv = NULL;
  KTxCtrlrAffectedState ctrlr_affected = kCtrlrAffectedNoDiff;
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, vbr.AuditUpdateController
            (UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, dmi,&err_ckv,&ctrlr_affected));
}

TEST_F(VbrIfMoMgrTest, AuditUpdateController_04) {
  VbrIfMoMgr vbr;
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  const char *ctrlr_id="Controller1";

  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *err_ckv = NULL;
  KTxCtrlrAffectedState ctrlr_affected = kCtrlrAffectedNoDiff;
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, vbr.AuditUpdateController
            (UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, dmi,&err_ckv,&ctrlr_affected));
}

TEST_F(VbrIfMoMgrTest, AuditUpdateController_05) {
  VbrIfMoMgr vbr;
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpDelete;
  const char *ctrlr_id="Controller1";

  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *err_ckv = NULL;
  KTxCtrlrAffectedState ctrlr_affected = kCtrlrAffectedNoDiff;
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, vbr.AuditUpdateController
            (UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, dmi, &err_ckv,&ctrlr_affected));
}

TEST_F(VbrIfMoMgrTest, AuditUpdateController_06) {
  VbrIfMoMgr vbr;
  uint32_t session_id=1;
  uint32_t config_id=2;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
  const char *ctrlr_id="Controller1";

  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *err_ckv = NULL;
  KTxCtrlrAffectedState ctrlr_affected = kCtrlrAffectedNoDiff;
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, vbr.AuditUpdateController
            (UNC_KT_VBR_IF,ctrlr_id,session_id,config_id,phase, dmi,&err_ckv,&ctrlr_affected));
}

# if 0
TEST_F(VbrIfMoMgrTest, UpdatePortMap_01) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key, val);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_drv_vbr_if *val1(UT_CLONE(val_drv_vbr_if, val));
  ConfigVal *cfgval(new ConfigVal(IpctSt::kIpcStValVbrIf, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *okey(new ConfigKeyVal(
    UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key, cfgval));
  ConfigKeyVal *ikey(new ConfigKeyVal(
    UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key1, cfgval1));
  upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
     obj.UpdatePortMap(okey, dt_type, dmi, ikey));

  delete ikey;
  delete okey;
}


TEST_F(VbrIfMoMgrTest, UpdatePortMap_02) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key, val);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_drv_vbr_if *val1(UT_CLONE(val_drv_vbr_if, val));
  ConfigVal *cfgval(new ConfigVal(IpctSt::kIpcStValVbrIf, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *okey(new ConfigKeyVal(
    UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key, cfgval));
  ConfigKeyVal *ikey(new ConfigKeyVal(
    UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key1, cfgval1));
  upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING;
  DalDmlIntf *dmi(getDalDmlIntf());
  val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
     obj.UpdatePortMap(okey, dt_type, dmi, ikey));

  delete ikey;
  delete okey;
}


TEST_F(VbrIfMoMgrTest, UpdatePortMap_03) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key, val);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_drv_vbr_if *val1(UT_CLONE(val_drv_vbr_if, val));
  ConfigVal *cfgval(new ConfigVal(IpctSt::kIpcStValVbrIf, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *okey(new ConfigKeyVal(
    UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key, cfgval));
  ConfigKeyVal *ikey(new ConfigKeyVal(
    UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key1, cfgval1));
  upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING;
  DalDmlIntf *dmi(getDalDmlIntf());
  val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
     obj.UpdatePortMap(okey, dt_type, dmi, ikey));

  delete ikey;
  delete okey;
}

# endif

TEST_F(VbrIfMoMgrTest, UpdatePortMap_04){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key,val);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_drv_vbr_if *val1(UT_CLONE(val_drv_vbr_if, val));
  ConfigVal *cfgval(new ConfigVal(IpctSt::kIpcStValVbrIf, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key, cfgval));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key1, cfgval1));
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name("vtn1");
  upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING ;
  DalDmlIntf *dmi(getDalDmlIntf());
  val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdatePortMap(okey,dt_type,dmi,ikey,
                                          config_mode,vtn_name));

  delete ikey;
  delete okey;
}

TEST_F(VbrIfMoMgrTest, GetRenameKeyBindInfo_PMainTbl)
{
  VbrIfMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_IF;
  BindInfo *bin;
  int nattr;
  MoMgrTables tbl = MAINTBL;

  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, tbl));
}

TEST_F(VbrIfMoMgrTest, GetRenameKeyBindInfo_FRenameTBL)
{
  VbrIfMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_IF;
  BindInfo *bin;
  int nattr;
  MoMgrTables tbl = RENAMETBL;
  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, tbl));
}

TEST_F(VbrIfMoMgrTest, GetRenameKeyBindInfo_FRename)
{
  VbrIfMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_IF;
  BindInfo *bin = NULL;
  int nattr = 0;
  MoMgrTables tbl = RENAMETBL;
  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, tbl));
}

TEST_F(VbrIfMoMgrTest, GetRenameKeyBindInfo_FNoTBL)
{
  VbrIfMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_IF;
  BindInfo *bin;
  int nattr;
  MoMgrTables tbl = CTRLRTBL;
  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, tbl));
}

TEST_F(VbrIfMoMgrTest, GetVbrIfValfromDBTrue){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);

  ConfigKeyVal *ck_drv_vbr_if(NULL);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE ; 
  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetVbrIfValfromDB(ikey,ck_drv_vbr_if,dt_type,dmi));
  ASSERT_TRUE(ck_drv_vbr_if != NULL);

  delete ck_drv_vbr_if;
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, GetVbrIfValfromDBFalse){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  ConfigKeyVal *ck_drv_vbr_if(NULL);
  upll_keytype_datatype_t dt_type = UPLL_DT_RUNNING ;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetVbrIfValfromDB(ikey,ck_drv_vbr_if,dt_type,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, GetVbrIfValfromDBParentKeyNull){
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,IpctSt::kIpcStKeyVbr,key, cfgval);
  ConfigKeyVal *ck_drv_vbr_if(NULL);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE ;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetVbrIfValfromDB(ck_drv_vbr_if,ikey,dt_type,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, UpdateVbrIf_1) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.updateVbrIf(req,ikey,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, UpdateVbrIf_2) {
  VbrIfMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_UPDATE;
  req->rep_count = 1;
  req->option1 = UNC_OPT1_NORMAL;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.updateVbrIf(req,ikey,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, UpdateVbrIf_3) {
  VbrIfMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_UPDATE;
  req->rep_count = 1;
  req->option1 = UNC_OPT1_NORMAL;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.updateVbrIf(req,ikey,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, UpdateVbrIf_4) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key,val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key, cfgval);
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.updateVbrIf(req,ikey,dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, GetChildConfigKey_PkeyVlinkSuccess) {
  VbrIfMoMgr vbr;
  key_vbr_if_t *key(ZALLOC_TYPE(key_vbr_if_t));
  val_vbr_if *val(ZALLOC_TYPE(val_vbr_if));
  strncpy((char*) key->if_name,"IF_1",32);
  strncpy((char*) key->vbr_key.vbridge_name,"VLINK1",32);
  strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",32);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VLINK,
                       IpctSt::kIpcStKeyVbrIf,
                       key,cfgval);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key1, cfgval1));
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));

  delete okey;
  delete pkey;
}

TEST_F(VbrIfMoMgrTest, GetChildConfigKey_PkeyVtnSuccess) {
  VbrIfMoMgr vbr;
  key_vbr_if_t *key(ZALLOC_TYPE(key_vbr_if_t));
  val_vbr_if *val(ZALLOC_TYPE(val_vbr_if));

  strncpy((char*) key->if_name,"IF_1",32);
  strncpy((char*) key->vbr_key.vbridge_name,"VLINK1",32);
  strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",32);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                       IpctSt::kIpcStKeyVbrIf,
                       key,cfgval);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key1, cfgval1));
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));

  delete okey;
  delete pkey;
}

TEST_F(VbrIfMoMgrTest, GetChildConfigKey_PkeyVbridgeSuccess) {
  VbrIfMoMgr vbr;
  key_vbr_if_t *key(ZALLOC_TYPE(key_vbr_if_t));
  val_vbr_if *val(ZALLOC_TYPE(val_vbr_if));
  strncpy((char*) key->if_name,"IF_1",32);
  strncpy((char*) key->vbr_key.vbridge_name,"VLINK1",32);
  strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",32);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                       IpctSt::kIpcStKeyVbrIf,
                       key,cfgval);
  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key1, cfgval1));
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetChildConfigKey(okey, pkey));

  delete okey;
  delete pkey;
}

TEST_F(VbrIfMoMgrTest, GetParentConfigKey_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbrIf,
                                      key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.GetParentConfigKey(okey, ikey));

  delete okey;
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, GetParentConfigKey_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key1, cfgval1));
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetParentConfigKey(okey, ikey));

  delete okey;
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, GetParentConfigKey_01) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.GetParentConfigKey(okey, ikey));
}

TEST_F(VbrIfMoMgrTest, GetParentConfigKey_09) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;

  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);

  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf,key,cfgval) ;
  EXPECT_EQ(UPLL_RC_SUCCESS,vbr.GetParentConfigKey(okey, ikey));

  delete okey;
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, AllocVal_Error) {
  VbrIfMoMgr obj;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(cfgval, UPLL_DT_CANDIDATE,RENAMETBL));

  delete cfgval;
}

# if 0
TEST_F(VbrIfMoMgrTest, ConverttoDriverPortMap_01) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(
    UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS,
     obj.ConverttoDriverPortMap(ikey, dmi));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ConverttoDriverPortMap_02) {
  VbrIfMoMgr obj;
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key, val);
  const char *if_name = "IF_1";

  strncpy(reinterpret_cast<char *>(key->if_name),
  if_name, strlen(if_name)+1);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(
    UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS,
     obj.ConverttoDriverPortMap(ikey, dmi));

  delete ikey;
}

# endif
/*
TEST_F(VbrIfMoMgrTest, UpdateConfigStatus_Success) {
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key, dmi, ikey));
  delete ikey;
  delete upd_key;
}
*/
TEST_F(VbrIfMoMgrTest, UpdateConfigStatus_SuccessUPDATE) {
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_drv_vbr_if *val1(UT_CLONE(val_drv_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key, dmi, ikey));
  delete ikey;
  delete upd_key;
}

TEST_F(VbrIfMoMgrTest, UpdateConfigStatus_SuccessUPDATE_01) {
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_drv_vbr_if *val1(UT_CLONE(val_drv_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                         key1, cfgval1));
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));

  delete ikey;
  delete upd_key;
}

TEST_F(VbrIfMoMgrTest, UpdateConfigStatus_SuccessUPDATE_02) {
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_drv_vbr_if *val1(UT_CLONE(val_drv_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                         key1, cfgval1));
  val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key, dmi, ikey));

  delete ikey;
  delete upd_key;
}

TEST_F(VbrIfMoMgrTest, UpdateConfigStatus_InvalidOP) {
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vbr_if *key;
  val_drv_vbr_if *val;
  GetKeyValDrvStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_drv_vbr_if *val1(UT_CLONE(val_drv_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                         key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(ikey, UNC_OP_READ,
                                                       UPLL_RC_SUCCESS,
                                                       upd_key, dmi, ikey));

  delete ikey;
  delete upd_key;
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
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));
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
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));
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
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));
  delete ckv_running;
}

TEST_F(VbrIfMoMgrTest, UpdateAuditConfigStatus_EmptyVal) {
  VbrIfMoMgr vbrmomgr;
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  UpdateCtrlrPhase phase = kUpllUcpCreate;
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbrmomgr.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));
  delete ckv_running;
}

TEST_F(VbrIfMoMgrTest, VbrIfMoMgr_UpdateAuditConfigStatus1) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *ikey =NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey, dmi));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, VbrIfMoMgr_UpdateAuditConfigStatus2) {
  VbrIfMoMgr vbr;
  val_vbr_if *val(ZALLOC_TYPE(val_vbr_if));
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbrIf,
                                 val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,IpctSt::kIpcStKeyVbrIf, NULL,tmp);
  DalDmlIntf *dmi(getDalDmlIntf());

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.UpdateAuditConfigStatus(UNC_CS_INVALID,uuc::kUpllUcpCreate, ikey, dmi));
  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_UNC_KT_VBRIDGE) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_req_NULL) {
  VbrIfMoMgr vbr;
  ConfigKeyVal *ikey = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateMessage(req, ikey));
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_kIpcStValVtnNeighbor) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtnNeighbor, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_kIpcStPfcdrvValVbrIf) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_kIpcStKeyVbr) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
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
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_01) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey));
  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  ConfigVal *cfgval1 = NULL;
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key1, cfgval1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey1));


  const char *if_name = " ";
  key_vbr_if *key2(ZALLOC_TYPE(key_vbr_if));
  val_vbr_if *val2(ZALLOC_TYPE(val_vbr_if));
  strncpy(reinterpret_cast<char *>(key2->if_name),
          if_name, strlen(if_name)+1);
  ConfigVal *cfgval2 = new ConfigVal(IpctSt::kIpcStValVbrIf, val2);
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key2, cfgval2);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vbr.ValidateMessage(req, ikey2));

  ConfigKeyVal *nullkey(NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vbr.ValidateMessage(req, nullkey));

  key_vbr_if *key3(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val3(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValVbrIf, val3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey3));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_02) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_UPDATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey));

  req->operation = UNC_OP_CONTROL;
  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,
            vbr.ValidateMessage(req, ikey1));

  req->operation = UNC_OP_UPDATE;
  key_vbr_if *key2(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val2(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval2(new ConfigVal(IpctSt::kIpcStValVbr, val2));
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key2, cfgval2));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey2));

  key_vbr_if *key3(UT_CLONE(key_vbr_if, key));
  ConfigVal *cfgval3(NULL);
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey3));

  key_vbr_if *key4(UT_CLONE(key_vbr_if, key));
  ConfigVal *cfgval4(new ConfigVal(IpctSt::kIpcStValVtn, NULL));
  ConfigKeyVal *ikey4(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key4, cfgval4));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vbr.ValidateMessage(req, ikey4));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
  delete ikey4;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_03) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IPC_REQ_RESP_HEADER_DECL(req);
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

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey1));

  key_vbr_if *key2(UT_CLONE(key_vbr_if, key));
  ConfigVal *cfgval2(NULL);
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key2, cfgval2));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey2));

  key_vbr_if *key3(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val3(NULL);
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValVbrIf, val3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey3));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_04) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IPC_REQ_RESP_HEADER_DECL(req);
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

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey1));

  key_vbr_if *key2(UT_CLONE(key_vbr_if, key));
  ConfigVal *cfgval2(NULL);
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key2, cfgval2));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey2));

  key_vbr_if *key3(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val3(NULL);
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValVbrIf, val3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey3));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_06) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IPC_REQ_RESP_HEADER_DECL(req);
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

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey1));

  key_vbr_if *key2(UT_CLONE(key_vbr_if, key));
  ConfigVal *cfgval2(NULL);
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key2, cfgval2));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey2));

  key_vbr_if *key3(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val3(NULL);
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValVbrIf, val3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey3));

  req->operation = UNC_OP_DELETE;
  key_vbr_if *key4(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val4(UT_CLONE(val_vbr_if, val));
  pfc_strlcpy(reinterpret_cast<char *>(key4->if_name), "IF_1",
              sizeof(key4->if_name));

  ConfigVal *cfgval4(new ConfigVal(IpctSt::kIpcStValVbrIf, val4));
  ConfigKeyVal *ikey4(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key4, cfgval4));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey4));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
  delete ikey4;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_05) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                            IpctSt::kIpcStKeyVbrIf,
                            key, cfgval);

  IPC_REQ_RESP_HEADER_DECL(req);
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

  key_vbr_if *key1(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val1(UT_CLONE(val_vbr_if, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbrIf, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey1));

  key_vbr_if *key2(UT_CLONE(key_vbr_if, key));
  ConfigVal *cfgval2(NULL);
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key2, cfgval2));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey2));

  key_vbr_if *key3(UT_CLONE(key_vbr_if, key));
  val_vbr_if *val3(NULL);
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValVbrIf, val3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey3));
  req->option2 = UNC_OPT2_L2DOMAIN;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vbr.ValidateMessage(req, ikey3));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
}

TEST_F(VbrIfMoMgrTest, ValidateMessage_07) {
  VbrIfMoMgr vbr;
  key_vbr_if *key;
  val_vbr_if *val;
  GetKeyValStruct(key, val);

  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_DELETE;

  ConfigVal *cfgval(new ConfigVal(IpctSt::kIpcStValVbrIf, val));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key, cfgval));
  EXPECT_EQ(UPLL_RC_SUCCESS, vbr.ValidateMessage(req, ikey));

  delete ikey;
}
/*
TEST_F(VbrIfMoMgrTest, GetVbrIfFromVexternal_TmpckvNonNull){
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vbr_if *key;
  val_vbr_if *val;
  uint8_t vtnname=1;
  uint8_t vexternal1=1;
  uint8_t *vexternal=&vexternal1;
  uint8_t *vtn1=&vtnname;
  GetKeyValStruct(key, val);
  controller_domain ctrlrdom ; 
  upll_keytype_datatype_t  dt_type = UPLL_DT_CANDIDATE;


  ConfigVal *cfgval(new ConfigVal(IpctSt::kIpcStValVbrIf, val));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                      key ,cfgval));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetVbrIfFromVExternal(vtn1,vexternal,ikey,dmi,ctrlrdom,dt_type));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, GetVbrIfFromVexternal_TmpckvNull){
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t vtnname[2] = {1, 0};
  uint8_t vexternal[2] = {1, 0};
  ConfigKeyVal *ikey =NULL;
  controller_domain ctrlrdom ;
  upll_keytype_datatype_t  dt_type = UPLL_DT_CANDIDATE;


  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            obj.GetVbrIfFromVExternal(vtnname, vexternal, ikey, dmi, ctrlrdom, dt_type));
}
*/
#if 0
TEST_F(VbrIfMoMgrTest, PortStatusHandler_TrueOperStatus){
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  const char *ctrlr_name="PFC_1";
  const char * domain_name="DOMAIN_1";
  const char *port_id="VLAN";
  bool oper_status = true;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.PortStatusHandler(ctrlr_name,domain_name,port_id,oper_status,dmi));
}

TEST_F(VbrIfMoMgrTest, PortStatusHandler_FalseOperStatus){
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());

  const char *ctrlr_name="PFC_1";
  const char * domain_name="DOMAIN_1";
  const char *port_id="VLAN";
  bool oper_status = UPLL_OPER_STATUS_UP;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.PortStatusHandler(ctrlr_name,domain_name,port_id,oper_status,dmi));
}

TEST_F(VbrIfMoMgrTest, PortStatusHandler_FalseOperStatus_02){
  VbrIfMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());

  const char *ctrlr_name="PFC_1";
  const char * domain_name="DOMAIN_1";
  const char *port_id="VLAN";
  bool oper_status = UPLL_OPER_STATUS_DOWN;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.PortStatusHandler(ctrlr_name,domain_name,port_id,oper_status,dmi));
}
#endif
TEST_F(VbrIfMoMgrTest, UpdateConfigVal_UpdateConfigVal_07) {
  VbrIfMoMgr vbr;
  upll_keytype_datatype_t datatype=UPLL_DT_CANDIDATE;
  key_vbr_if *key;
  val_drv_vbr_if *val;

  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  GetKeyValDrvStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIf, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  TcConfigMode config_mode =  TC_CONFIG_VTN;
  std::string vtn_name("vtn1");
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vbr.UpdateConfigVal(ikey,datatype,dmi,
                                              config_mode,vtn_name));

  delete ikey;
}

TEST_F(VbrIfMoMgrTest, CompareValidValue_auditTrue) {
  VbrIfMoMgr obj;
  val_drv_vbr_if *val1(ZALLOC_TYPE(val_drv_vbr_if));
  val_drv_vbr_if *val2(ZALLOC_TYPE(val_drv_vbr_if));
  void *v1(val1);
  void *v2(val2);

  for (unsigned int loop = 0; loop < sizeof(val1->valid); ++loop) {
    val1->valid[loop] = UNC_VF_INVALID;
    val2->valid[loop] = UNC_VF_VALID;
  }
  ASSERT_FALSE(obj.CompareValidValue(v1, v2, true));

  ASSERT_EQ(UNC_VF_INVALID, val1->valid[0]);
  for (unsigned int loop = 1; loop < sizeof(val1->valid); ++loop) {
    EXPECT_EQ(UNC_VF_VALID_NO_VALUE, val1->valid[loop]);
  }
  free(val1);
  free(val2);
}

TEST_F(VbrIfMoMgrTest, CompareValidValue_auditfalse) {
  VbrIfMoMgr obj;
  val_drv_vbr_if *val1(ZALLOC_TYPE(val_drv_vbr_if));
  val_drv_vbr_if *val2(ZALLOC_TYPE(val_drv_vbr_if));
  void *v1(val1);
  void *v2(val2);

  val1->valid[1] = UNC_VF_VALID;
  val2->valid[1] = UNC_VF_VALID;
  val1->vbr_if_val.admin_status = 1;
  val2->vbr_if_val.admin_status = 1;
  ASSERT_TRUE(obj.CompareValidValue(v1, v2, true));

  free(val1);
  free(val2);
}

TEST_F(VbrIfMoMgrTest, CompareValidValue_Invalid_case1) {
  VbrIfMoMgr obj;
  val_drv_vbr_if *val1(ZALLOC_TYPE(val_drv_vbr_if));
  val_drv_vbr_if *val2(ZALLOC_TYPE(val_drv_vbr_if));
  void *v1(val1);
  void *v2(val2);

  for (unsigned int loop = 0;loop < sizeof(val1->valid) / sizeof(uint8_t);
       ++loop) {
    val1->valid[loop] = UNC_VF_VALID;
    val2->valid[loop] = UNC_VF_VALID;
  }
  ASSERT_TRUE(obj.CompareValidValue(v1, v2, true));

  free(val1);
  free(val2);
}
