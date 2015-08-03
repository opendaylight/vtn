/*
 * Copyright (c) 2015 NEC Corporation
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
#include <vterminal_momgr.hh>
#include <config_mgr.hh>
#include <dal_odbc_mgr.hh>
#include <dal_dml_intf.hh>
//#include <capa_intf.hh>
//#include <capa_module_stub.hh>
#include <tclib_module.hh>
#include <ctrlr_mgr.hh>
#include "momgr_intf_stub.hh"
#include "../ut_util.hh"

//bool fatal_done;
//pfc::core::Mutex  fatal_mutex_lock;
/*
 * Allocate zeroed buffer.
 */
#define ZALLOC_TYPE(type)         \
    (reinterpret_cast<type *>(calloc(1, sizeof(type))))

using ::testing::TestWithParam;
using ::testing::Values;
using namespace unc::upll;
using namespace unc::tclib;
using namespace unc::upll::dal;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::config_momgr;
using namespace unc::upll::test;
using namespace unc::capa;
using namespace pfc::core;
using namespace unc::upll::dal::schema::table;

class VterminalMoMgrTest
: public UpllTestEnv {
};

static void GetKeyStruct(key_vterm *&kst) {
  const char *vtn_name = "VTN_1";
  const char *vterminal_name = "VTERM_1";

  kst = ZALLOC_TYPE(key_vterm);
  strncpy(reinterpret_cast<char *>(kst->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(kst->vterminal_name),
          vterminal_name, strlen(vterminal_name)+1);
}

static void GetValStruct(val_vterm *&vst) {
  const char *desc = "thisisvterminal";
  const char *ctrlr_id = "Controller1";
  const char *domain_id = "Domain1";

  vst = ZALLOC_TYPE(val_vterm);
  for (unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->valid); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }

  vst->cs_row_status = UNC_VF_VALID;

  for (unsigned int loop = 0; loop < PFC_ARRAY_CAPACITY(vst->cs_attr); ++loop) {
    vst->cs_attr[loop] = UNC_CS_APPLIED;
  }

  strncpy(reinterpret_cast<char *>(vst->vterm_description), desc,
          strlen(desc)+1);
  strncpy(reinterpret_cast<char *>(vst->controller_id), ctrlr_id,
          strlen(ctrlr_id)+1);
  strncpy(reinterpret_cast<char *>(vst->domain_id), domain_id,
          strlen(domain_id)+1);
}

static void GetKeyValStruct(key_vterm *&kst, val_vterm *&vst) {
  GetKeyStruct(kst);
  GetValStruct(vst);
}

//***********PartialMergeValidate****************//

TEST(VterminalMoMgrTest, PartialMergeValidate_1) {
  VterminalMoMgr obj;
  const char* ctrlr_id = "ctrlr1";
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_SINGLE,
                                 kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, 
            obj.PartialMergeValidate(UNC_KT_VTERMINAL, ctrlr_id, ikey, dmi));
  DalOdbcMgr::clearStubData();
  delete dmi;
}

TEST(VterminalMoMgrTest, PartialMergeValidate_2) {
  VterminalMoMgr obj;
  const char* ctrlr_id = "ctrlr1";
  ConfigKeyVal *ikey = new ConfigKeyVal(
      UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal, NULL, NULL);
  DalDmlIntf *dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_SINGLE,
                                 kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
            obj.PartialMergeValidate(UNC_KT_VTERMINAL, ctrlr_id, ikey, dmi));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
  delete dmi;
}

TEST(VterminalMoMgrTest, PartialMergeValidate_3) {
  VterminalMoMgr obj;
  const char* ctrlr_id = "ctrlr1";
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_SINGLE,
                                 kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_SUCCESS,
            obj.PartialMergeValidate(UNC_KT_VTERMINAL, ctrlr_id, ikey, dmi));
  DalOdbcMgr::clearStubData();
  delete dmi;
}

TEST(VterminalMoMgrTest, ValidateVterminalKey_Success) {
  VterminalMoMgr vtermmomgr;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  const char *vtn_name = "VTN_1";
  const char *vterm_name = "VTERM_1";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vterminal_name),
          vterm_name, strlen(vterm_name)+1);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermmomgr.ValidateVterminalKey(key));

  free(key);
}

TEST(VterminalMoMgrTest, ValidateVterminalKey_InvalidVtnName) {
  VterminalMoMgr vtermmomgr;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  const char *vtn_name = "";
  const char *vterm_name = "VTERM_1";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vterminal_name),
          vterm_name, strlen(vterm_name)+1);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermmomgr.ValidateVterminalKey(key));

  free(key);
}

TEST(VterminalMoMgrTest, ValidateVterminalKey_InvalidVtermName) {
  VterminalMoMgr vtermmomgr;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  const char *vtn_name = "VTN_1";
  const char *vterm_name = "";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vterminal_name),
          vterm_name, strlen(vterm_name)+1);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermmomgr.ValidateVterminalKey(key));

  free(key);
}

TEST(VterminalMoMgrTest, ValidateVterminalKey_InvalidKeyStruct) {
  VterminalMoMgr vtermmomgr;
  key_vterm *key = NULL;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermmomgr.ValidateVterminalKey(key));
}

TEST(VterminalMoMgrTest, ValidateVterminalValue_InvalidCtrlrID) {
  VterminalMoMgr vtermmomgr;
  uint32_t oper = UNC_OP_CREATE;
  val_vterm *val;
  GetValStruct(val);
  string ctrlr_id = "Controller 1";
  strncpy(reinterpret_cast<char *>(val->controller_id), ctrlr_id.c_str(),
          strlen(ctrlr_id.c_str())+1);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermmomgr.
            ValidateVterminalValue(val, oper));

  free(val);
}

TEST(VterminalMoMgrTest, ValidateVterminalValue_InvalidDesc) {
  VterminalMoMgr vtermmomgr;
  uint32_t oper = UNC_OP_CREATE;
  val_vterm *val;
  GetValStruct(val);
  string desc = "";
  strncpy(reinterpret_cast<char *>(val->vterm_description), desc.c_str(),
          strlen(desc.c_str())+1);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermmomgr.
            ValidateVterminalValue(val, oper));

  free(val);
}

TEST(VterminalMoMgrTest, ValidateVterminalValue_DescValidAttrInvalid) {
  VterminalMoMgr vtermmomgr;
  uint32_t oper = UNC_OP_UPDATE;
  val_vterm *val;
  GetValStruct(val);
  val->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS , vtermmomgr.ValidateVterminalValue(val, oper));

  free(val);
}

TEST(VterminalMoMgrTest, ValidateVterminalValue_invalidFlagNew) {
  VterminalMoMgr vterm;
  val_vterm_t *valvterm(ZALLOC_TYPE(val_vterm_t));
  uint32_t op = UNC_OP_CREATE;
  valvterm->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_INVALID;
  uuu::upll_strncpy((char*)valvterm->vterm_description, (const char *)"vterm1",
                    (kMaxLenDescription+1));

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vterm.ValidateVterminalValue(valvterm, op));

  free(valvterm);
}

TEST(VterminalMoMgrTest, ValidateVterminalValue_invalidFlag1) {
  VterminalMoMgr vterm;
  val_vterm_t *valvterm(ZALLOC_TYPE(val_vterm_t));
  uint32_t op = UNC_OP_UPDATE;
  valvterm->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_INVALID;
  uuu::upll_strncpy((char*)valvterm->vterm_description, (const char *)"vterm1",
                    (kMaxLenDescription+1));

  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.ValidateVterminalValue(valvterm, op));

  free(valvterm);
}

TEST(VterminalMoMgrTest, ValidateVterminalValue_validFlag4) {
  VterminalMoMgr vterm;
  val_vterm_t *valvterm(ZALLOC_TYPE(val_vterm_t));
  uint32_t op = UNC_OP_UPDATE;
  valvterm->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
  uuu::upll_strncpy((char*)valvterm->vterm_description, (const char *)"vterm1",
                    (kMaxLenDescription+1));

  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.ValidateVterminalValue(valvterm, op));

  free(valvterm);
}

TEST(VterminalMoMgrTest, ValidateVterminalValue_validFlag5) {
  VterminalMoMgr vterm;
  val_vterm_t *valvterm(ZALLOC_TYPE(val_vterm_t));
  uint32_t op = UNC_OP_UPDATE;
  valvterm->valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_INVALID;
  uuu::upll_strncpy((char*)valvterm->vterm_description, (const char *)"vterm1",
                    (kMaxLenDescription+1));

  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.ValidateVterminalValue(valvterm, op));

  free(valvterm);
}
/*
TEST(VterminalMoMgrTest, ValidateVterminalValue_validFlag6) {
  VterminalMoMgr vterm;
  val_vterm_t *valvterm(ZALLOC_TYPE(val_vterm_t));
  uint32_t op = UNC_OP_UPDATE;
  valvterm->valid[UPLL_IDX_PACKET_SIZE_PING] = UNC_VF_INVALID;
  uuu::upll_strncpy((char*)valvterm->vterm_description, (const char *)"vterm1",
                    (kMaxLenDescription+1));

  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.ValidateVterminalValue(valvterm, op));

  free(valvterm);
}
*/
TEST(VterminalMoMgrTest, ValidateVterminalValue_validFlag7) {
  VterminalMoMgr vterm;
  val_vterm_t *valvterm(ZALLOC_TYPE(val_vterm_t));
  uint32_t op = UNC_OP_CREATE;
  valvterm->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
  uuu::upll_strncpy((char*)valvterm->vterm_description, (const char *)"vterm1",
                    (kMaxLenDescription+1));

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vterm.ValidateVterminalValue(valvterm, op));

  free(valvterm);
}

TEST(VterminalMoMgrTest, ValidateVterminalValue_validFlag8) {
  VterminalMoMgr vterm;
  val_vterm_t *valvterm(ZALLOC_TYPE(val_vterm_t));
  uint32_t op = UNC_OP_CREATE;
  valvterm->valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_INVALID;
  uuu::upll_strncpy((char*)valvterm->vterm_description, (const char *)"vterm1",
                    (kMaxLenDescription+1));

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vterm.ValidateVterminalValue(valvterm, op));

  free(valvterm);
}

TEST(VterminalMoMgrTest, ValidateVterminalValue_validFlag10) {
  VterminalMoMgr vterm;
  val_vterm_t *valvterm(ZALLOC_TYPE(val_vterm_t));
  uint32_t op = UNC_OP_CREATE;
  valvterm->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_VALID;
  uuu::upll_strncpy((char*)valvterm->vterm_description, (const char *)"vterm1",
                    (kMaxLenDescription+1));

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vterm.ValidateVterminalValue(valvterm, op));

  free(valvterm);
}

TEST(VterminalMoMgrTest, GetParentConfigKeySuccess) {
  VterminalMoMgr vtermmomgr;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ickv = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfg_val);
  ConfigKeyVal *ockv= NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermmomgr.GetParentConfigKey(ockv, ickv));

  delete ockv;
  delete ickv;
}

TEST(VterminalMoMgrTest, GetParentConfigKeyInvalidArg) {
  VterminalMoMgr vtermmomgr;
  ConfigKeyVal *ickv = NULL;
  ConfigKeyVal *ockv = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermmomgr.GetParentConfigKey(ockv, ickv));
}

TEST(VterminalMoMgrTest, GetParentConfigKeyInvalidKT) {
  VterminalMoMgr vtermmomgr;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVterminal, val);

  ConfigKeyVal *ockv = NULL;
  ConfigKeyVal *ickv = new ConfigKeyVal(UNC_KT_VTN,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfg_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermmomgr.GetParentConfigKey(ockv, ickv));

  delete ockv;
  delete ickv;
}

TEST(VterminalMoMgrTest, GetParentConfigKeyNullKey) {
  VterminalMoMgr vtermmomgr;
  ConfigKeyVal *ickv = new ConfigKeyVal(UNC_KT_VTERMINAL);
  ConfigKeyVal *ockv = new ConfigKeyVal(UNC_KT_VTERMINAL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermmomgr.GetParentConfigKey(ockv, ickv));

  delete ickv;
  delete ockv;
}
TEST(VterminalMoMgrTest, AllocVal_outputNull) {
  VterminalMoMgr obj;
  val_vterm *val(ZALLOC_TYPE(val_vterm));
  ConfigVal* cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AllocVal(cfgval, UPLL_DT_IMPORT, MAINTBL));
  delete cfgval;
}

TEST(VterminalMoMgrTest, AllocVal_SuccessMAINTBL) {
  VterminalMoMgr obj;
  ConfigVal* cfgval = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(cfgval, UPLL_DT_IMPORT, MAINTBL));
}

TEST(VterminalMoMgrTest, AllocVal_SuccessRENAMETBL) {
  VterminalMoMgr obj;
  ConfigVal* cfgval = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(cfgval, UPLL_DT_IMPORT, RENAMETBL));
}

TEST(VterminalMoMgrTest, AllocVal_SuccessDT_STATE) {
  VterminalMoMgr obj;
  ConfigVal* cfgval = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(cfgval, UPLL_DT_STATE, MAINTBL));
}

TEST(VterminalMoMgrTest, AllocVal_Error) {
  VterminalMoMgr obj;
  val_vterm *val;
  GetValStruct(val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            obj.AllocVal(cfgval, UPLL_DT_STATE, RENAMETBL));
  delete cfgval;
}

TEST(VterminalMoMgrTest, AllocVal_ErrorDefaultCase) {
  VterminalMoMgr obj;
  ConfigVal* cfgval = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(cfgval, UPLL_DT_STATE, CTRLRTBL));
}
TEST(VterminalMoMgrTest, DupConfigKeyVal_ReqNull) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        NULL, NULL);
  ConfigKeyVal *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));

  delete okey;
}

TEST(VterminalMoMgrTest, DupConfigKeyVal_OkeyNotNull) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        NULL, NULL);

  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));

  delete okey;
  delete req;
}

TEST(VterminalMoMgrTest, DupConfigKeyVal_ReqInvalidKT) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVterminal,
                                 val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VTN,
                                       IpctSt::kIpcStKeyVterminal,
                                       key, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));

  delete okey;
  delete req;
}

TEST(VterminalMoMgrTest, DupConfigKeyVal_Req_InValid) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  val_vterm *val;
  GetValStruct(val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       NULL, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));

  delete req;
}

TEST(VterminalMoMgrTest, DupConfigKeyVal_SuccessMAINTBL) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVterminal, val);

  val_vterm *val1(UT_CLONE(val_vterm, val));
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum, val1);
  tmp->AppendCfgVal(tmp1);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(okey, req, MAINTBL));

  delete req;
  delete okey;
}

TEST(VterminalMoMgrTest, DupConfigKeyVal_SuccessRENAMETBL) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(okey, req, RENAMETBL));

  delete req;
  delete okey;
}

TEST(VterminalMoMgrTest, DupConfigKeyVal_SuccessRENAMETBLInvalidStNum) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(okey, req, RENAMETBL));

  delete req;
  delete okey;
}

TEST(VterminalMoMgrTest, DupConfigKeyVal_NullValStuct) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  ConfigVal *tmp = NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(okey, req, RENAMETBL));

  delete req;
  delete okey;
}

TEST(VterminalMoMgrTest, DupConfigKeyVal_NullValStuctMainTbl) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  ConfigVal *tmp = NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key, tmp);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(okey, req, MAINTBL));

  delete req;
  delete okey;
}
TEST(VterminalMoMgrTest, IsValidKey_SuccessVTNName) {
  VterminalMoMgr obj;
  uint64_t index = uudst::vterminal::kDbiVtnName;
  key_vterm *key;
  GetKeyStruct(key);
  EXPECT_EQ(true, obj.IsValidKey(reinterpret_cast<void *>(key), index));

  free(key);
}

TEST(VterminalMoMgrTest, IsValidKey_SuccessVTERMName) {
  VterminalMoMgr obj;
  uint64_t index = uudst::vterminal::kDbiVterminalName;
  key_vterm *key;
  GetKeyStruct(key);
  EXPECT_EQ(true, obj.IsValidKey(reinterpret_cast<void *>(key), index));

  free(key);
}

TEST(VterminalMoMgrTest, IsValidKey_InvalidIndex) {
  VterminalMoMgr obj;
  uint64_t index = uudst::vterminal::kDbiCtrlrName;
  key_vterm *key;
  GetKeyStruct(key);
  EXPECT_EQ(false, obj.IsValidKey(reinterpret_cast<void *>(key), index));

  free(key);
}

TEST(VterminalMoMgrTest, IsValidKey_InvalidVTNName) {
  VterminalMoMgr obj;
  uint64_t index = uudst::vterminal::kDbiVtnName;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  const char *vtn_name = "";
  const char *vterm_name = "VTERM_1";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vterminal_name),
          vterm_name, strlen(vterm_name)+1);
  EXPECT_EQ(false, obj.IsValidKey(reinterpret_cast<void *>(key), index));

  free(key);
}

TEST(VterminalMoMgrTest, IsValidKey_InvalidVTERMName) {
  VterminalMoMgr obj;
  uint64_t index = uudst::vterminal::kDbiVterminalName;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  const char *vtn_name = "VTN";
  const char *vterm_name = "";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vterminal_name),
          vterm_name, strlen(vterm_name)+1);

  EXPECT_EQ(false, obj.IsValidKey(reinterpret_cast<void *>(key), index));

  free(key);
}
TEST(VterminalMoMgrTest, GetChildConfigKey_SuccessNullObjs) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey));
  delete okey;
}

TEST(VterminalMoMgrTest, GetChildConfigKey_pkeyNull) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetChildConfigKey(okey, pkey));

  delete pkey;
  delete okey;
}

TEST(VterminalMoMgrTest, GetChildConfigKey_SuccesspkeyVTERM) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vterm *key(ZALLOC_TYPE(key_vterm));
  strncpy((char*) key->vterminal_name, "VTERM1", 32);
  strncpy((char*) key->vtn_key.vtn_name, "VTN1", 32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey));
  key_vterm_t *output = reinterpret_cast<key_vterm_t *> (okey->get_key());

  EXPECT_STREQ("VTN1", (reinterpret_cast<const char *>
                       (output->vtn_key.vtn_name)));
  EXPECT_STREQ("VTERM1", (reinterpret_cast<const char *>
                         (output->vterminal_name)));

  delete okey;
  delete pkey;
}

TEST(VterminalMoMgrTest, GetChildConfigKey_SuccesspkeyVTN) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vterm *key(ZALLOC_TYPE(key_vterm));
  strncpy((char*) key->vterminal_name, "VTERM1", 32);
  strncpy((char*) key->vtn_key.vtn_name, "VTN1", 32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey));
  key_vterm_t *output = reinterpret_cast<key_vterm_t *> (okey->get_key());

  EXPECT_STREQ("VTN1", (reinterpret_cast<const char *>
                        (output->vtn_key.vtn_name)));
  EXPECT_STREQ("", (reinterpret_cast<const char *> (output->vterminal_name)));

  delete okey;
  delete pkey;
}

TEST(VterminalMoMgrTest, GetChildConfigKey_SuccessOkeyVTN) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vterm *key(ZALLOC_TYPE(key_vterm));
  strncpy((char*) key->vterminal_name, "VTERM1", 32);
  strncpy((char*) key->vtn_key.vtn_name, "VTN1", 32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey));
  key_vterm_t *output = reinterpret_cast<key_vterm_t *> (okey->get_key());

  EXPECT_STREQ("VTN1", (reinterpret_cast<const char *>
                       (output->vtn_key.vtn_name)));
  EXPECT_STREQ("", (reinterpret_cast<const char *> (output->vterminal_name)));

  delete okey;
  delete pkey;
}
TEST(VterminalMoMgrTest, CopyToConfigkey_ikeyokeyNull) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));

  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                          IpctSt::kIpcStKeyVterminal,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));

  delete okey;
  delete ikey;
}
TEST(VterminalMoMgrTest, GetChildConfigKey_PkeyVtermSuccess) {
  VterminalMoMgr vterm;
  ConfigKeyVal *okey = NULL;

  key_vterm *key(ZALLOC_TYPE(key_vterm));
  strncpy((char*) key->vterminal_name, "VTERM1", 32);
  strncpy((char*) key->vtn_key.vtn_name, "VTN1", 32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.GetChildConfigKey(okey, pkey));
  key_vterm_t *output = reinterpret_cast<key_vterm_t *> (okey->get_key());

  EXPECT_STREQ("VTN1", (reinterpret_cast<const char *>
                        (output->vtn_key.vtn_name)));
  EXPECT_STREQ("VTERM1", (reinterpret_cast<const char *>
                          (output->vterminal_name)));

  delete okey;
  delete pkey;
}

TEST(VterminalMoMgrTest, GetChildConfigKey_OkeyVtnSuccess) {
  VterminalMoMgr vterm;

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN);
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  strncpy((char*) key->vterminal_name, "VTERM1", 32);
  strncpy((char*) key->vtn_key.vtn_name, "VTN1", 32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.GetChildConfigKey(okey, pkey));
  key_vterm_t *output = reinterpret_cast<key_vterm_t *> (okey->get_key());

  EXPECT_STREQ("VTN1", (reinterpret_cast<const char *>
                        (output->vtn_key.vtn_name)));
  EXPECT_STREQ("", (reinterpret_cast<const char *>
                   (output->vterminal_name)));

  delete okey;
  delete pkey;
}

TEST(VterminalMoMgrTest, GetChildConfigKey_PkeyVlinkSuccess) {
  VterminalMoMgr vterm;

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VLINK);
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  strncpy((char*) key->vterminal_name, "VLINK1", 32);
  strncpy((char*) key->vtn_key.vtn_name, "VTN1", 32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVterminal,
                                        key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.GetChildConfigKey(okey, pkey));
  key_vterm_t *output = reinterpret_cast<key_vterm_t *> (okey->get_key());

  EXPECT_STREQ("VTN1", (reinterpret_cast<const char *>
                        (output->vtn_key.vtn_name)));
  EXPECT_STREQ("", (reinterpret_cast<const char *> (output->vterminal_name)));

  delete okey;
  delete pkey;
}

TEST(VterminalMoMgrTest, GetChildConfigKey_05) {
  key_vterm_t *key_vterm(ZALLOC_TYPE(key_vterm_t));
  VterminalMoMgr vterm_obj;
  uuu::upll_strncpy((char *)key_vterm->vtn_key.vtn_name, (char *)"vtn1",
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy((char *)key_vterm->vterminal_name, (char *)"vterminal",
                    (kMaxLenVnodeName+1));

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VLINK,
                                        IpctSt::kIpcStKeyVterminal,
                                        key_vterm);
  EXPECT_EQ(UPLL_RC_SUCCESS, vterm_obj.GetChildConfigKey(okey, pkey));

  delete okey;
  delete pkey;
}

TEST(VterminalMoMgrTest, GetChildConfigKey_06) {
  key_vterm_t *key_vterm(ZALLOC_TYPE(key_vterm_t));
  VterminalMoMgr vterm_obj;
  uuu::upll_strncpy((char *)key_vterm->vtn_key.vtn_name, (char *)"vtn1",
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy((char *)key_vterm->vterminal_name, (char *)"vterminal",
                    (kMaxLenVnodeName+1));
  val_vlink *vlink_val(ZALLOC_TYPE(val_vlink));
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVlink, vlink_val);

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VLINK,
                                        IpctSt::kIpcStKeyVterminal,
                                        key_vterm, cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS, vterm_obj.GetChildConfigKey(okey, pkey));

  delete okey;
  delete pkey;
}
TEST(VterminalMoMgrTest, CopyToConfigkey_InValidName) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  const char *vtn_name = "";
  const char *vterm_name = "";
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key->vterminal_name),
          vterm_name, strlen(vterm_name)+1);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  cout << "TEST: Negative: InvalidVtermName" << endl;

  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name), "VTN_1", 32);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));

  delete okey;
  delete ikey;
}

TEST(VterminalMoMgrTest, CopyToConfigkey_Valid) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey(NULL);
  key_rename_vnode_info *key_rename(ZALLOC_TYPE(key_rename_vnode_info));

  const char *vtn_name("VTN_1");
  const char *vterm_name("VTERM_1");
  pfc_strlcpy(reinterpret_cast<char *>(key_rename->old_unc_vtn_name),
              vtn_name, sizeof(key_rename->old_unc_vtn_name));
  pfc_strlcpy(reinterpret_cast<char *>(key_rename->old_unc_vnode_name),
              vterm_name, sizeof(key_rename->old_unc_vnode_name));
  ConfigVal *cfgval(new ConfigVal(IpctSt::kIpcStValVterminal, NULL));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                      IpctSt::kIpcStKeyVterminal,
                                      key_rename, cfgval));
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));

  ASSERT_TRUE(okey != NULL);

  key_vterm_t *key(reinterpret_cast<key_vterm_t *>(okey->get_key()));
  if (key != NULL) {
    EXPECT_STREQ(vtn_name, reinterpret_cast<char *>(key->vtn_key.vtn_name));
    EXPECT_STREQ(vterm_name, reinterpret_cast<char *>(key->vterminal_name));
  }

  delete ikey;
  delete okey;
}
TEST(VterminalMoMgrTest, UpdateConfigStatus_Success) {
  VterminalMoMgr obj;
  DalDmlIntf *dmi= NULL;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);

  key_vterm *key1(UT_CLONE(key_vterm, key));
  val_vterm *val1(UT_CLONE(val_vterm, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVterminal, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                         IpctSt::kIpcStKeyVterminal,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key, dmi, ikey));

  delete ikey;
  delete upd_key;
}

TEST(VterminalMoMgrTest, UpdateConfigStatus_SuccessUPDATE) {
  VterminalMoMgr obj;
  DalDmlIntf *dmi= NULL;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);

  key_vterm *key1(UT_CLONE(key_vterm, key));
  val_vterm *val1(UT_CLONE(val_vterm, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVterminal, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                         IpctSt::kIpcStKeyVterminal,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key, dmi, ikey));

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key, dmi, ikey));
  delete ikey;
  delete upd_key;
}

TEST(VterminalMoMgrTest, UpdateConfigStatus_InvalidOP) {
  VterminalMoMgr obj;
  DalDmlIntf *dmi= NULL;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);

  key_vterm *key1(UT_CLONE(key_vterm, key));
  val_vterm *val1(UT_CLONE(val_vterm, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVterminal, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                         IpctSt::kIpcStKeyVterminal,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(ikey,
                                                       UNC_OP_READ,
                                                       UPLL_RC_SUCCESS,
                                                       upd_key, dmi, ikey));

  delete ikey;
  delete upd_key;
}

TEST(VterminalMoMgrTest, UpdateConfigStatus_InvalidArg) {
  VterminalMoMgr obj;
  DalDmlIntf *dmi= NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        NULL, NULL);
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                         IpctSt::kIpcStKeyVterminal,
                                         NULL, NULL));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                       UPLL_RC_SUCCESS,
                                                       upd_key, dmi, ikey));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                       UPLL_RC_SUCCESS,
                                                       upd_key, dmi, ikey));

  delete ikey;
  delete upd_key;
}

TEST(VterminalMoMgrTest, UpdateConfigStatus_AttrNotSupp_ValNoValue) {
  VterminalMoMgr obj;
  DalDmlIntf *dmi= NULL;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);

  key_vterm *key1(UT_CLONE(key_vterm, key));
  val_vterm *val1(UT_CLONE(val_vterm, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVterminal, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                         IpctSt::kIpcStKeyVterminal,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key, dmi, ikey));

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key, dmi, ikey));

  delete ikey;
  delete upd_key;
}
TEST(VterminalMoMgrTest, CreateVnodeConfigKey_Success) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigKeyVal *okey = NULL;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CreateVnodeConfigKey(ikey, okey));

  delete ikey;
  delete okey;
}

TEST(VterminalMoMgrTest, CreateVnodeConfigKey_NULLArg) {
  VterminalMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateVnodeConfigKey(ikey, okey));
}
TEST(VterminalMoMgrTest, CompareValidValue_AuditTrue) {
  VterminalMoMgr obj;
  val_vterm val1, val2;
  memset(&val1, 0, sizeof(val1));
  memset(&val2, 0, sizeof(val2));

  void *v11 = NULL;
  void *v22 = NULL;

  EXPECT_EQ(false, obj.CompareValidValue(v11, v22, true));
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
TEST(VterminalMoMgrTest, CompareValidValue_Auditfalse1) {
  VterminalMoMgr obj;
  val_vterm val1, val2;
  memset(&val1, 0, sizeof(val1));
  memset(&val2, 0, sizeof(val2));

  for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(val1.valid); i++) {
    val1.valid[i] = UNC_VF_VALID;
    val2.valid[i] = UNC_VF_VALID;
  }
  void *v1(&val1);
  void *v2(&val2);

  ASSERT_TRUE(obj.CompareValidValue(v1, v2, false));

  for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(val1.valid); i++) {
    ASSERT_EQ(UNC_VF_INVALID, val1.valid[i]);
  }
}

TEST(VterminalMoMgrTest, FilterAttributes_CreateOperation) {
  VterminalMoMgr vterm;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vterm_t *valvterm1(ZALLOC_TYPE(val_vterm_t));
  valvterm1->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_VALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvterm1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvterm1));
  EXPECT_EQ(false, vterm.FilterAttributes(val1, val2, audit_status, op));
  EXPECT_EQ(UNC_VF_INVALID, valvterm1->valid[UPLL_IDX_DESC_VTERM]);

  free(valvterm1);
}

TEST(VterminalMoMgrTest, FilterAttributes_OperationUpdate) {
  VterminalMoMgr vterm;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_UPDATE;

  val_vterm_t *valvterm1(ZALLOC_TYPE(val_vterm_t));
  valvterm1->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_VALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvterm1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valvterm1));
  EXPECT_EQ(true, vterm.FilterAttributes(val1, val2, audit_status, op));
  EXPECT_EQ(UNC_VF_INVALID, valvterm1->valid[UPLL_IDX_DESC_VTERM]);

  free(valvterm1);
}
TEST(VterminalMoMgrTest, GetRenameKeyBindInfo) {
  VterminalMoMgr obj;
  BindInfo *binfo;
  int nattr;

  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(UNC_KT_VTERMINAL, binfo, nattr,
                                               MAINTBL));

  EXPECT_EQ(5, nattr);
  EXPECT_EQ(&VterminalMoMgr::key_vterminal_maintbl_bind_info[0], binfo);


  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(UNC_KT_VTERMINAL, binfo, nattr,
                                               RENAMETBL));

  EXPECT_EQ(4, nattr);
  EXPECT_EQ(&VterminalMoMgr::key_vterminal_renametbl_update_bind_info[0],
            binfo);
}

TEST(VterminalMoMgrTest, GetRenameKeyBindInfo_OutputUnknownTbl) {
  VterminalMoMgr vterm;
  unc_key_type_t key_type = UNC_KT_VTERMINAL;
  BindInfo *binfo = NULL;
  int nattr = 2;

  EXPECT_EQ(false,
            vterm.GetRenameKeyBindInfo(key_type, binfo, nattr, CTRLRTBL));
  EXPECT_EQ(2, nattr);
}
TEST(VterminalMoMgrTest, GetVnodeName) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  uint8_t *vtn_name, *vnode_name;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetVnodeName(ikey, vtn_name, vnode_name));
  EXPECT_STREQ("VTN_1", (reinterpret_cast<const char *> (vtn_name)));
  EXPECT_STREQ("VTERM_1", (reinterpret_cast<const char *> (vnode_name)));

  val_vterm *val1(UT_CLONE(val_vterm, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVterminal, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       NULL, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetVnodeName(ikey1, vtn_name, vnode_name));

  delete ikey;
  delete ikey1;
}
TEST(VterminalMoMgrTest, ValidateCapability_ErrorInput) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);
  IPC_REQ_RESP_HEADER_DECL(req);
  const char *ctrlr_name = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(req, ikey, ctrlr_name));

  delete ikey;
}

/*
TEST(VterminalMoMgrTest, ValidateCapability_ikey_NULL) {
  VterminalMoMgr vterm;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req = NULL;
  const char *ctrlr_name="ctr1";
  EXPECT_EQ(UPLL_RC_ERR_CFG_SEMANTIC, vterm.ValidateCapability(req, ikey,
                                                          ctrlr_name));
}
*/
TEST(VterminalMoMgrTest, ValidateCapability_ctrName_NULL) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);
  IPC_REQ_RESP_HEADER_DECL(req);
  const char *ctrlr_name = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vterm.ValidateCapability(req, ikey,
                                                          ctrlr_name));

  delete ikey;
}

TEST(VterminalMoMgrTest, ValidateCapability_ctrName) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);
  IPC_REQ_RESP_HEADER_DECL(req);
  const char *ctrlr_name = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vterm.ValidateCapability(req,
                                                          ikey,
                                                          ctrlr_name));

  delete ikey;
}
TEST(VterminalMoMgrTest, ValidateCapability_Attributesupportcheckcreate) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  uint8_t attrs[2];
  val->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_VALID_NO_VALUE;
  attrs[unc::capa::vterminal::kCapDesc] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
            vterm.ValVterminalAttributeSupportCheck(val, attrs, UNC_OP_CREATE));
  EXPECT_EQ(UNC_VF_INVALID, val->valid[UPLL_IDX_DESC_VTERM]);
}

TEST(VterminalMoMgrTest, ValidateCapability_Attributesupportcheckupdate) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  uint8_t attrs[2];
  attrs[unc::capa::vterminal::kCapDesc] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
            vterm.ValVterminalAttributeSupportCheck(val, attrs, UNC_OP_UPDATE));
  EXPECT_EQ(UNC_VF_INVALID, val->valid[UPLL_IDX_DESC_VTERM]);
}
TEST(VterminalMoMgrTest, ValidateCapability_Attributesupportcheckread) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  uint8_t attrs[2];
  attrs[unc::capa::vterminal::kCapDesc] = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS,
            vterm.ValVterminalAttributeSupportCheck(val,
                                                    attrs,
                                                    UNC_OP_READ));
  EXPECT_EQ(UNC_VF_INVALID, val->valid[UPLL_IDX_DESC_VTERM]);
}
TEST(VterminalMoMgrTest, ValidateCapability_AttributesupportcheckVALNULL) {
  VterminalMoMgr vterm;
  uint8_t attrs[2];
  attrs[unc::capa::vterminal::kCapDesc] = 0;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vterm.ValVterminalAttributeSupportCheck(NULL,
      attrs,
      UNC_OP_CREATE));
}
TEST(VterminalMoMgrTest, ValidateCapability_Attributesupportchecksuccess) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  uint8_t attrs[2];
  attrs[unc::capa::vterminal::kCapDesc] = 1;
  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.ValVterminalAttributeSupportCheck(val,
      attrs,
      UNC_OP_CREATE));
}
TEST(VterminalMoMgrTest, ValidateMessage_Success) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);

  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));

  val_rename_vterm *renameval(ZALLOC_TYPE(val_rename_vterm));
  for (unsigned int loop = 0; loop < sizeof(renameval->valid)/
      sizeof(renameval->valid[0]); ++loop) {
    renameval->valid[loop] = UNC_VF_VALID;
  }
  strncpy(reinterpret_cast<char *>(renameval->new_name),
          "renamed", strlen("renamed")+1);

  key_vterm *key1(UT_CLONE(key_vterm, key));
  ConfigVal *rename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVterminal,
                                           renameval);
  ConfigKeyVal *rename_ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                               IpctSt::kIpcStKeyVterminal,
                                               key1, rename_cfgval);

  req->operation = UNC_OP_RENAME;
  req->datatype = UPLL_DT_IMPORT;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, rename_ikey));

  req->operation = UNC_OP_READ_SIBLING;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, rename_ikey));

  ConfigVal *invrename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVterminal,
                                              NULL);
  key_vterm *key2(UT_CLONE(key_vterm, key));
  ConfigKeyVal *invrename_ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                                  IpctSt::kIpcStKeyVterminal,
                                                  key2, invrename_cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, invrename_ikey));

  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_RUNNING;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));

  req->operation = UNC_OP_DELETE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));

  delete ikey;
  delete rename_ikey;
  delete invrename_ikey;
}

TEST(VterminalMoMgrTest, ValidateMessage_ReadSuccess) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);
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

TEST(VterminalMoMgrTest, ValidateMessage_ReadFailure) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
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

TEST(VterminalMoMgrTest, ValidateMessage_ReadOption2Failure) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
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

TEST(VterminalMoMgrTest, ValidateMessage_InvalidValVterm) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValRenameVterminal, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
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

TEST(VterminalMoMgrTest, ValidateMessage_NullVal) {
  VterminalMoMgr obj;
  key_vterm *key;
  GetKeyStruct(key);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
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

TEST(VterminalMoMgrTest, ValidateMessage_DiffOption) {
  VterminalMoMgr obj;
  key_vterm *key;
  GetKeyStruct(key);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
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

TEST(VterminalMoMgrTest, ValidateMessage_DiffOption2) {
  VterminalMoMgr obj;
  key_vterm *key;
  GetKeyStruct(key);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, NULL);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_IP_ROUTE;
  req->datatype = UPLL_DT_STATE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));

  delete ikey;
}

TEST(VterminalMoMgrTest, ValidateMessage_ValidDiffOption) {
  VterminalMoMgr obj;
  key_vterm *key;
  GetKeyStruct(key);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, NULL);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_READ;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_STATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));

  delete ikey;
}

TEST(VterminalMoMgrTest, ValidateMessage_InvalidInputCREATE) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
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

  key_vterm *key1(UT_CLONE(key_vterm, key));
  val_vterm *val1(UT_CLONE(val_vterm, val));
  ConfigVal *inval_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVterminal,
                                          val1);
  ConfigKeyVal *invalcfgkey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                               IpctSt::kIpcStKeyVterminal,
                                               key1, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  key_vterm *key2(UT_CLONE(key_vterm, key));
  val_vterm *val2(UT_CLONE(val_vterm, val));
  inval_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVterminal, val2);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyLogicalPort,
                                 key2, inval_cfgval);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  delete invalcfgkey;
  key2 = UT_CLONE(key_vterm, key);
  val2 = UT_CLONE(val_vterm, val);
  inval_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVterminal, val2);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVterminal,
                                 key2, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  req->datatype = UPLL_DT_AUDIT;

  req->datatype = UPLL_DT_CANDIDATE;

  delete invalcfgkey;
  key2 = UT_CLONE(key_vterm, key);
  key2->vtn_key.vtn_name[0] = '\0';
  val2 = UT_CLONE(val_vterm, val);
  inval_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVterminal, val2);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
                                 key2, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, invalcfgkey));

  delete invalcfgkey;
  key2 = UT_CLONE(key_vterm, key);
  strncpy(reinterpret_cast<char *>(key->vtn_key.vtn_name),
          "VTN_1", strlen("VTN_1")+1);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
                                 key2, NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  delete invalcfgkey;
  key2 = UT_CLONE(key_vterm, key);
  inval_cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, NULL);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
                                 key2, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, invalcfgkey));

  delete invalcfgkey;
  key2 = UT_CLONE(key_vterm, key);
  val2 = UT_CLONE(val_vterm, val);
  pfc_strlcpy(reinterpret_cast<char *>(val2->controller_id), "Controller 1",
              sizeof(val2->controller_id));
  inval_cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val2);
  invalcfgkey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
                                 key2, inval_cfgval);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, invalcfgkey));

  delete ikey;
  delete invalcfgkey;
}

TEST(VterminalMoMgrTest, ValidateMessage_InvalidInputRENAME) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
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

  key_vterm *key1(UT_CLONE(key_vterm, key));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValRenameVterminal, NULL));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey1));

  key_vterm *key2(UT_CLONE(key_vterm, key));
  val_rename_vterm *rval2(ZALLOC_TYPE(val_rename_vterm));
  for (unsigned int loop = 0;
      loop < PFC_ARRAY_CAPACITY(rval2->valid); ++loop) {
    rval2->valid[loop] = UNC_VF_VALID;
  }
  ConfigVal *cfgval2(new ConfigVal(IpctSt::kIpcStValRenameVterminal, rval2));
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key2, cfgval2));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey2));

  key_vterm *key3(UT_CLONE(key_vterm, key));
  val_rename_vterm *rval3(UT_CLONE(val_rename_vterm, rval2));
  pfc_strlcpy(reinterpret_cast<char *>(rval3->new_name), "renamed",
              sizeof(rval3->new_name));
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValRenameVterminal, rval3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key3, cfgval3));

  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey3));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_PING;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey3));

  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));

  key_vterm *key4(UT_CLONE(key_vterm, key));
  val_rename_vterm *rval4(UT_CLONE(val_rename_vterm, rval2));
  ConfigVal *cfgval4(new ConfigVal(IpctSt::kIpcStValRenameVterminal, rval4));
  ConfigKeyVal *ikey4(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key4, cfgval4));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey4));

  key_vterm *key5(UT_CLONE(key_vterm, key));
  val_rename_vterm *rval5(UT_CLONE(val_rename_vterm, rval3));
  ConfigVal *cfgval5(new ConfigVal(IpctSt::kIpcStValRenameVterminal, rval5));
  ConfigKeyVal *ikey5(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key5, cfgval5));
  req->operation = UNC_OP_RENAME;
  req->option1 = UNC_OPT1_COUNT;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey5));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_L2DOMAIN;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey5));
  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
  delete ikey4;
  delete ikey5;
}

TEST(VterminalMoMgrTest, ValidateMessage_InvalidInputREAD) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
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

  key_vterm *key1(UT_CLONE(key_vterm, key));
  val_rename_vterm *val1(ZALLOC_TYPE(val_rename_vterm));
  req->option2 = UNC_OPT2_NONE;
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValRenameVterminal, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey1));

  key_vterm *key2(UT_CLONE(key_vterm, key));
  ConfigVal *cfgval2(new ConfigVal(IpctSt::kIpcStValVterminal, NULL));
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key2, cfgval2));
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey2));

  key_vterm *key3(UT_CLONE(key_vterm, key));
  val_vterm *val3(UT_CLONE(val_vterm, val));
  pfc_strlcpy(reinterpret_cast<char *>(val3->controller_id), "Controller 1",
              sizeof(val3->controller_id));
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValVterminal, val3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey3));

  req->datatype = UPLL_DT_AUDIT;
  EXPECT_EQ(UPLL_RC_SUCCESS,
            obj.ValidateMessage(req, ikey3));

  req->operation = UNC_OP_DELETE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey3));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
}
TEST(VterminalMoMgrTest, ValidateMessage_DELETE_CANDIDATE) {
  VterminalMoMgr obj;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_DELETE;
  req->rep_count = 100;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
  delete ikey;
}
TEST(VterminalMoMgrTest, AdaptValToVtnService_Success) {
  VterminalMoMgr obj;
  key_vterm *key;
  GetKeyStruct(key);
  AdaptType adaptType = ADAPT_ALL;

  val_vterm_st *valst(ZALLOC_TYPE(val_vterm_st));
  valst->valid[0] = UNC_VF_VALID;
  valst->oper_status = UPLL_OPER_STATUS_UP;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVterminalSt, valst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfg_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey, adaptType));
  delete ikey;
}

TEST(VterminalMoMgrTest, AdaptValToVtnService_Failure) {
  VterminalMoMgr obj;
  key_vterm *key;
  GetKeyStruct(key);
  AdaptType adaptType = ADAPT_ALL;

  ConfigKeyVal* ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey, adaptType));

  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey, adaptType));
  delete ikey;
}

TEST(VterminalMoMgrTest, GetValid) {
  VterminalMoMgr obj;
  val_vterm *val;
  uint8_t *valid(NULL);
  GetValStruct(val);

  void *in_val = reinterpret_cast<void *>(val);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val,
      unc::upll::dal::schema::table::vterminal::kDbiCtrlrName,
      valid, UPLL_DT_CANDIDATE, MAINTBL));
  EXPECT_EQ(val->valid[UPLL_IDX_CONTROLLER_ID_VTERM],
            valid[UPLL_IDX_CONTROLLER_ID_VTERM]);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val,
      unc::upll::dal::schema::table::vterminal::kDbiDomainId,
      valid, UPLL_DT_CANDIDATE, MAINTBL));
  EXPECT_EQ(val->valid[UPLL_IDX_DOMAIN_ID_VTERM],
            valid[UPLL_IDX_DOMAIN_ID_VTERM]);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(in_val,
      unc::upll::dal::schema::table::vterminal::kDbiVterminalDesc,
      valid, UPLL_DT_CANDIDATE, MAINTBL));
  EXPECT_EQ(val->valid[UPLL_IDX_DESC_VTERM], valid[UPLL_IDX_DESC_VTERM]);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetValid(in_val,
      unc::upll::dal::schema::table::vterminal::kDbiVterminalName,
      valid, UPLL_DT_CANDIDATE, MAINTBL));

  in_val = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetValid(in_val,
      unc::upll::dal::schema::table::vterminal::kDbiCtrlrName,
      valid, UPLL_DT_CANDIDATE, MAINTBL));

  free(val);
}

/*
TEST(VterminalMoMgrTest, SwapKeyVal_IpctSt_valid) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT, kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);

  VterminalMoMgr vterminal;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vterminal_name = "VTERM_1";
  bool no_rename;
  key_vterm_t *key(ZALLOC_TYPE(key_vterm_t));
  pfc_strlcpy(reinterpret_cast<char *>(key->vterminal_name),
              vterminal_name, sizeof(key->vterminal_name));
  val_rename_vterm_t *val(ZALLOC_TYPE(val_rename_vterm_t));
  const char *newname = "VTERM_1";
  pfc_strlcpy(reinterpret_cast<char *>(val->new_name),
              newname, sizeof(val->new_name));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVterminalSt, val);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vterminal.SwapKeyVal(NULL, okey, dmi, ctr_id1, no_rename));

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        NULL, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vterminal.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  delete okey;

  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
                          key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vterminal.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  delete okey;

  ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVterminal,
                          key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,
            vterminal.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  delete okey;

  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
                          key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vterminal.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  delete okey;
  newname = "VTERM_2";
  pfc_strlcpy(reinterpret_cast<char *>(val->new_name),
              newname, sizeof(val->new_name));
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                          IpctSt::kIpcStKeyVterminal, key, config_val);
  val->valid[UPLL_IDX_NEW_NAME_RVTERM] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS,
            vterminal.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  delete okey;

  val->valid[UPLL_IDX_NEW_NAME_RVTERM] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_SUCCESS,
            vterminal.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  delete okey;
  newname = "";
  pfc_strlcpy(reinterpret_cast<char *>(val->new_name),
              newname, sizeof(val->new_name));
  val->valid[UPLL_IDX_NEW_NAME_RVTERM] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vterminal.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  delete okey;

  val->valid[UPLL_IDX_NEW_NAME_RVTERM] = UNC_VF_INVALID;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vterminal.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  delete okey;

  key_vterm_t *key1(UT_CLONE(key_vterm_t, key));
  val_rename_vterm_t *val1(UT_CLONE(val_rename_vterm_t, val));
  ConfigVal *config_val1(new ConfigVal(IpctSt::kIpcStValVterminalSt, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVterminal,
                                       key1, config_val1));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,
            vterminal.SwapKeyVal(ikey1, okey, dmi, ctr_id1, no_rename));
  delete okey;

  key_vterm_t *key2(UT_CLONE(key_vterm_t, key));
  ConfigVal *config_val2(NULL);
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key2, config_val2));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vterminal.SwapKeyVal(ikey2, okey, dmi, ctr_id1, no_rename));
  delete okey;

  key_vterm_t *key3(UT_CLONE(key_vterm_t, key));
  val_rename_vterm_t *val3(NULL);
  ConfigVal *config_val3(new ConfigVal(IpctSt::kIpcStValVterminalSt, val3));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                       IpctSt::kIpcStKeyVterminal,
                                       key3, config_val3));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vterminal.SwapKeyVal(ikey3, okey, dmi, ctr_id1, no_rename));
  delete okey;

  delete ikey;
  delete ikey1;
  delete ikey2;

  free(ctr_id1);
}
*/
TEST(VterminalMoMgrTest, GetControllerDomainId1) {
  VterminalMoMgr vterm;
  controller_domain ctrl_domain;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  key_vterm_t *key_vterm(ZALLOC_TYPE(key_vterm_t));
  uuu::upll_strncpy((char *)key_vterm->vtn_key.vtn_name, (char *)"vtn1",
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy((char *)key_vterm->vterminal_name, (char *)"vterminal1",
                    (kMaxLenVnodeName+1));
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key_vterm);
  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.GetControllerDomainId(ikey, &ctrl_domain));

  delete ikey;
}

TEST(VterminalMoMgrTest, GetControllerDomainId2) {
  VterminalMoMgr vterm;
  controller_domain ctrl_domain;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vterm.GetControllerDomainId(ikey,
                                                             &ctrl_domain));
}

TEST(VterminalMoMgrTest, GetControllerDomainId3) {
  VterminalMoMgr vterm;
  controller_domain *ctrl_domain = NULL;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vterm.GetControllerDomainId(ikey,
                                                             ctrl_domain));

  delete ikey;
}

TEST(VterminalMoMgrTest, GetControllerDomainId4) {
  VterminalMoMgr vterm;
  controller_domain ctrl_domain;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  key_vterm_t *key_vterm(ZALLOC_TYPE(key_vterm_t));

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key_vterm, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.GetControllerDomainId(ikey, &ctrl_domain));

  delete ikey;
}

TEST(VterminalMoMgrTest, GetControllerDomainId5) {
  VterminalMoMgr vterm;
  controller_domain ctrl_domain;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  key_vterm_t *key_vterm(ZALLOC_TYPE(key_vterm_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVterminal, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key_vterm, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.GetControllerDomainId(ikey, &ctrl_domain));

  delete ikey;
}

TEST(VterminalMoMgrTest, GetControllerDomainId_InvalidVal) {
  VterminalMoMgr vterm;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  val_vterm *val(ZALLOC_TYPE(val_vterm));
  val->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_VALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_VALID;
  pfc_strlcpy(reinterpret_cast<char *>(val->controller_id), "pfc1",
              sizeof(val->controller_id));
  pfc_strlcpy(reinterpret_cast<char *>(val->domain_id), "dom1",
              sizeof(val->domain_id));
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(ctrlr_dom));
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.GetControllerDomainId(ikey, &ctrlr_dom));

  delete ikey;
}

TEST(VterminalMoMgrTest, GetControllerDomainId_SetDomainData) {
  VterminalMoMgr vterm;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  val_vterm *val(ZALLOC_TYPE(val_vterm));
  val->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_VALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_VALID;
  pfc_strlcpy(reinterpret_cast<char *>(val->controller_id), "pfc1",
              sizeof(val->controller_id));
  pfc_strlcpy(reinterpret_cast<char *>(val->domain_id), "dom1",
              sizeof(val->domain_id));
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(ctrlr_dom));

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);

  SET_USER_DATA_CTRLR(ikey, val->controller_id)
      EXPECT_EQ(UPLL_RC_SUCCESS, vterm.GetControllerDomainId(ikey, &ctrlr_dom));

  delete ikey;
}
TEST(VterminalMoMgrTest, GetRenameInfo1) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);

  VterminalMoMgr vterm;
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vterm.GetRenameInfo(NULL, okey,
                                                     rename_info,
                                                     NULL,
                                                     NULL,
                                                     no_rename));
}

TEST(VterminalMoMgrTest, GetRenameInfo2) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);

  VterminalMoMgr vterm;
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vterm.GetRenameInfo(ikey, okey,
                                                     rename_info,
                                                     NULL,
                                                     NULL,
                                                     no_rename));

  delete ikey;
  delete rename_info;
}


TEST(VterminalMoMgrTest, SwapKeyVal1) {
  VterminalMoMgr vterm;
  ConfigKeyVal *key = NULL;
  bool no_rename = false;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vterm.SwapKeyVal(NULL, key,
                                                  NULL, NULL, no_rename));
}

TEST(VterminalMoMgrTest, SwapKeyVal2) {
  VterminalMoMgr vterm;
  key_vterm_t *key_vterm(ZALLOC_TYPE(key_vterm_t));
  uuu::upll_strncpy((char *)key_vterm->vtn_key.vtn_name, (char *)"vtn1",
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy((char *)key_vterm->vterminal_name, (char *)"vterminal1",
                    (kMaxLenVnodeName+1));

  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                                        IpctSt::kIpcStKeyVterminal, key_vterm);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vterm.SwapKeyVal(ikey, okey, NULL,
                                                      NULL, no_rename));

  delete ikey;
  delete okey;
}

TEST(VterminalMoMgrTest, SwapKeyVal3) {
  VterminalMoMgr vterm;
  key_vterm_t *key_vterm(ZALLOC_TYPE(key_vterm_t));
  uuu::upll_strncpy((char *)key_vterm->vtn_key.vtn_name, (char *)"vtn1",
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy((char *)key_vterm->vterminal_name, (char *)"vterminal1",
                    (kMaxLenVnodeName+1));

  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  val_rename_vterm *vterm_rename_val = NULL;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValRenameVterminal,
                                       vterm_rename_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key_vterm, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vterm.SwapKeyVal(ikey, okey,
                                                  NULL, NULL, no_rename));

  delete ikey;
  delete okey;
}

/*
TEST(VterminalMoMgrTest, SwapKeyVal7) {
  VterminalMoMgr vterm;
  key_vterm_t *key_vterm(ZALLOC_TYPE(key_vterm_t));
  uuu::upll_strncpy((char *)key_vterm->vtn_key.vtn_name, (char *)"vtn1",
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy((char *)key_vterm->vterminal_name, (char *)"vterminal1",
                    (kMaxLenVnodeName+1));
  const char * ctrlr_name = "ctrlr1";
  bool no_rename = false;
  ConfigKeyVal *okey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());

  val_rename_vterm *vterm_rename_val(ZALLOC_TYPE(val_rename_vterm));
  uuu::upll_strncpy((char *)vterm_rename_val->new_name, (char*)"hhh",
                    (kMaxLenVnodeName+1));
  vterm_rename_val->valid[UPLL_IDX_NEW_NAME_RVRT] =  UNC_VF_VALID;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValRenameVterminal,
                                       vterm_rename_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVrt,
                                        key_vterm, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.SwapKeyVal(ikey, okey, dmi,
                                              (uint8_t *)ctrlr_name,
                                              no_rename));

  delete ikey;
}
*/
TEST(VterminalMoMgrTest, CtrlrIdAndDomainIdUpdationCheck2) {
  VterminalMoMgr vterm;
  key_vterm *key(ZALLOC_TYPE(key_vterm));
  val_vterm *val(ZALLOC_TYPE(val_vterm));

  val->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_INVALID;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);

  key_vterm *key1(UT_CLONE(key_vterm, key));
  val_vterm *val1(ZALLOC_TYPE(val_vterm));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVterminal, val1));
  ConfigKeyVal *ckey(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                      IpctSt::kIpcStKeyVterminal,
                                      key1, cfgval1));
  EXPECT_EQ(UPLL_RC_SUCCESS, vterm.CtrlrIdAndDomainIdUpdationCheck(ikey, ckey));

  delete ikey;
  delete ckey;
}

TEST(VterminalMoMgrTest, CtrlrIdAndDomainIdUpdationCheck3) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);

  val->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_VALID;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, cfgval);

  key_vterm *key1(UT_CLONE(key_vterm, key));
  val_vterm *val1(ZALLOC_TYPE(val_vterm));
  pfc_strlcpy(reinterpret_cast<char *>(val1->domain_id), "dom1",
              sizeof(val1->domain_id));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVterminal, val1));
  ConfigKeyVal *ckey(new ConfigKeyVal(UNC_KT_VTERMINAL,
                                      IpctSt::kIpcStKeyVterminal,
                                      key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,
            vterm.CtrlrIdAndDomainIdUpdationCheck(ikey, ckey));
  val->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_VALID;
  pfc_strlcpy(reinterpret_cast<char *>(val1->controller_id), "ctr1",
              sizeof(val1->controller_id));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,
            vterm.CtrlrIdAndDomainIdUpdationCheck(ikey, ckey));
  pfc_strlcpy(reinterpret_cast<char *>(val->domain_id), "dom1",
              sizeof(val->domain_id));
  pfc_strlcpy(reinterpret_cast<char *>(val->controller_id), "ctr1",
              sizeof(val->controller_id));

  EXPECT_EQ(UPLL_RC_SUCCESS,
            vterm.CtrlrIdAndDomainIdUpdationCheck(ikey, ckey));
  delete ikey;
  delete ckey;
}
TEST(VterminalMoMgrTest, ValVterminalAttributeSupportCheck_01) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[8];
  memset(attrs, 0xff, sizeof(attrs));
  attrs[unc::capa::vterminal::kCapDomainId] = 0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,
            vterm.ValVterminalAttributeSupportCheck(val, attrs, operation));
  delete ikey;
}

TEST(VterminalMoMgrTest, ValVterminalAttributeSupportCheck_02) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[8];
  memset(attrs, 0xff, sizeof(attrs));
  attrs[unc::capa::vterminal::kCapDomainId] = 0;

  val->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
  val->valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,
            vterm.ValVterminalAttributeSupportCheck(val, attrs, operation));
  delete ikey;
}

TEST(VterminalMoMgrTest, ValVterminalAttributeSupportCheck_03) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation= UNC_OP_CREATE;
  uint8_t attrs[8];
  memset(attrs, 0xff, sizeof(attrs));
  attrs[unc::capa::vterminal::kCapDesc] = 0;
  val->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
            vterm.ValVterminalAttributeSupportCheck(val, attrs, operation));
  delete ikey;
}

TEST(VterminalMoMgrTest, ValVterminalAttributeSupportCheck_04) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;

  GetKeyValStruct(key, val);
  unc_keytype_operation_t operation(UNC_OP_READ);
  val->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_VALID;
  uint8_t attrs[8];
  memset(attrs, 0xff, sizeof(attrs));
  attrs[unc::capa::vterminal::kCapDesc] = 0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,
            vterm.ValVterminalAttributeSupportCheck(val, attrs, operation));
  delete ikey;
}
/*
TEST(VterminalMoMgrTest, ValidateAttribute) {
  VterminalMoMgr vterm;
  key_vterm *key;
  val_vterm *val;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVterminal, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                        IpctSt::kIpcStKeyVterminal,
                                        key, config_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  //DalOdbcMgr::stub_setSingleRecordExists(false);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcSuccess);

  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vterm.
            ValidateAttribute(ikey, dmi, req));
  delete ikey;
}
*/
