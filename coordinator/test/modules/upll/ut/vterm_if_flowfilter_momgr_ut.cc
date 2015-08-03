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
#include <vterm_if_flowfilter_momgr.hh>
#include <unc/keytype.h>
#include <config_mgr.hh>
#include <dal_odbc_mgr.hh>
#include <dal_dml_intf.hh>
//#include <capa_intf.hh>
//#include <capa_module_stub.hh>
#include <tclib_module.hh>
#include <ctrlr_mgr.hh>
#include <momgr_intf_stub.hh>
//#include <alarm.hh>
#include "ut_util.hh"
#include  "tx_update_util.hh"

//UpllDbConnMgr *objDb=NULL;
//bool fatal_done;
//pfc::core::Mutex  fatal_mutex_lock;
/*
 * Allocate zeroed buffer.
*/
#define ZALLOC_TYPE(type)         \
(reinterpret_cast<type *>(calloc(1, sizeof(type))))

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
using namespace unc::upll::tx_update_util;

class VterminalMoMgrTest
  : public UpllTestEnv
{
};

#if 0
#define DELETE_IF_NOT_NULL(key) \
do { \
  if (key)\
  delete key;\
  key = NULL;\
} while (0);
#endif
/*
static void ReqPopulate_Normal(IpcReqRespHeader* req) {
        req->option1 = UNC_OPT1_NORMAL;
        req->option2 = UNC_OPT2_NONE;
        req->datatype =  (upll_keytype_datatype_t) UNC_DT_STATE;
        req->operation = UNC_OP_READ;
}

static void ReqPopulate_Detail(IpcReqRespHeader* req) {
        req->option1 = UNC_OPT1_DETAIL;
        req->option2 = UNC_OPT2_NONE;
        req->datatype =  (upll_keytype_datatype_t) UNC_DT_STATE;
        req->operation = UNC_OP_READ;
}


static void ReqPopulate_Create(IpcReqRespHeader* req) {
        req->option1 = UNC_OPT1_NORMAL;
        req->option2 = UNC_OPT2_NONE;
        req->datatype =  (upll_keytype_datatype_t) UNC_DT_CANDIDATE;
        req->operation = UNC_OP_CREATE;
}
*/
static void AllocKeyStruct(key_vterm_if_flowfilter_t *&key) {
  key = reinterpret_cast<key_vterm_if_flowfilter_t*>
                  (malloc(sizeof(key_vterm_if_flowfilter_t)));
  memset(key, 0 ,sizeof(key_vterm_if_flowfilter_t)); 
}

static void AllocKeyRenameVnodeStruct(key_rename_vnode_info *&key) {
  key = reinterpret_cast<key_rename_vnode_info*>
                  (malloc(sizeof(key_rename_vnode_info)));
  memset(key, 0 ,sizeof(key_rename_vnode_info));
}

static void GetKeyRenameVnodeStruct(key_rename_vnode_info *key) {
  strcpy((char*)key->new_unc_vtn_name, "new_vtn");
  strcpy((char*)key->old_unc_vtn_name, "old_vtn");
  strcpy((char*)key->new_unc_vnode_name, "new_vnode");
  strcpy((char*)key->old_unc_vnode_name, "old_vnode");
}

static void GetKeyRenameVnodeStructNoFill(key_rename_vnode_info *key, int no_fill) {
  if (no_fill == 0) {
    strcpy((char*)key->old_unc_vtn_name, "old_vtn");
    strcpy((char*)key->new_unc_vnode_name, "new_vnode");
    strcpy((char*)key->old_unc_vnode_name, "old_vnode");
  } else  if (no_fill == 1) {
    strcpy((char*)key->new_unc_vtn_name, "new_vtn");
    strcpy((char*)key->new_unc_vnode_name, "new_vnode");
    strcpy((char*)key->old_unc_vnode_name, "old_vnode");
  } else if (no_fill == 2) {
    strcpy((char*)key->new_unc_vtn_name, "new_vtn");
    strcpy((char*)key->old_unc_vtn_name, "old_vtn");
    strcpy((char*)key->old_unc_vnode_name, "old_vnode");
  } else if (no_fill == 3) {
    strcpy((char*)key->new_unc_vtn_name, "new_vtn");
    strcpy((char*)key->old_unc_vtn_name, "old_vtn");
    strcpy((char*)key->new_unc_vnode_name, "new_vnode");
  }
}

static void AllocConfigVal(ConfigVal *&cfg_val) {
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t *>
          (malloc(sizeof(val_flowfilter_t)));
  memset(ff_val, 0, sizeof(val_flowfilter_t));
  cfg_val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                          ff_val);
}

static void AllocEntryConfigVal(ConfigVal *&cfg_val) {
  val_flowfilter_entry_t *ffe_val = reinterpret_cast<val_flowfilter_entry_t *>
          (malloc(sizeof(val_flowfilter_entry_t)));
  memset(ffe_val, 0, sizeof(val_flowfilter_entry_t));
  cfg_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, ffe_val);
}
/*
static void AllocEntryConfigValWithValid(ConfigVal *&cfg_val) {
  val_flowfilter_entry_t *ffe_val = reinterpret_cast<val_flowfilter_entry_t *>
          (malloc(sizeof(val_flowfilter_entry_t)));
  memset(ffe_val, 0, sizeof(val_flowfilter_entry_t));
  for ( unsigned int loop = 0; loop < sizeof(ffe_val->valid); ++loop ) {
    ffe_val->valid[loop] = UNC_VF_VALID;
  }
  cfg_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, ffe_val);
}
*/
static void GetKeyStruct(key_vterm_if_flowfilter_t* key) {
        strcpy((char*)key->if_key.vterm_key.vtn_key.vtn_name,"vtn1");
        strcpy((char*)key->if_key.vterm_key.vterminal_name,"vterminal1");
        strcpy((char*)key->if_key.if_name,"ifname");
        key->direction = 0;
}

static void GetInvalidKeyStruct(key_vterm_if_flowfilter_t* key) {
        strcpy((char*)key->if_key.vterm_key.vtn_key.vtn_name,"!vtn1");
        strcpy((char*)key->if_key.vterm_key.vterminal_name,"!vterminal1");
        strcpy((char*)key->if_key.if_name,"!ifname");
        key->direction = 0xFE;
}

static void GetIpcReqResHeader(uint32_t clnt_sess_id,
    uint32_t config_id,
    unc_keytype_operation_t operation,
    uint32_t rep_count,
    unc_keytype_option1_t option1,
    unc_keytype_option2_t option2,
    upll_keytype_datatype_t datatype,
    IpcReqRespHeader *&req) {
  req = reinterpret_cast<IpcReqRespHeader *>
      (malloc(sizeof(IpcReqRespHeader)));
  memset(req, 0, sizeof(IpcReqRespHeader));
  req->clnt_sess_id = clnt_sess_id;
  req->config_id = config_id;
  req->operation = operation;
  req->rep_count = rep_count;
  req->option1 = option1;
  req->option2 = option2;
  req->datatype = datatype;
}
/******************************************************************************
 * Function - AllocVal
 *****************************************************************************/

TEST(VterminalMoMgrTest, AllocVal_Success) {
  VtermIfFlowFilterMoMgr obj;
  ConfigVal *val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_IMPORT,MAINTBL));
  delete val;
  val = NULL;
}

TEST(VterminalMoMgrTest, AllocVal_WrongTable) {
  VtermIfFlowFilterMoMgr obj;
  ConfigVal *val = NULL;
  obj.AllocVal(val, UPLL_DT_STATE,RENAMETBL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_STATE,RENAMETBL));
}

/******************************************************************************
 * Function - GetChildConfigKey
 *****************************************************************************/
TEST(VterminalMoMgrTest,GetChildConfigKey) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
}

/*TEST(VterminalMoMgrTest,GetChildConfigKey_pkeyNull) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey =  new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(pkey);
}
*/
TEST(VterminalMoMgrTest,GetChildConfigKey_pkeyNull1) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,GetChildConfigKey_wrongKT) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,GetChildConfigKey_wrong_parent_KT) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,GetChildConfigKey_vtermifff_parent_KT) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,GetChildConfigKey_vtn_parent_KT) {
  VtermIfFlowFilterMoMgr obj;
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t*>
                  (malloc(sizeof(key_vtn_t)));
  memset(vtn_key, 0 ,sizeof(key_vtn_t));
  strncpy((char*) vtn_key->vtn_name, "vtn1", 32);
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            vtn_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,GetChildConfigKey_vterm_parent_KT) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_t *vterm_key = reinterpret_cast<key_vterm_t*>
                  (malloc(sizeof(key_vterm_t)));
  memset(vterm_key, 0 ,sizeof(key_vterm_t));
  strncpy((char*) vterm_key->vtn_key.vtn_name, "vtn1", 32);
  strncpy((char*) vterm_key->vterminal_name, "vterminal1", 32);
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                            IpctSt::kIpcStKeyVterminal,
                            vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,GetChildConfigKey_vterm_if_parent_KT) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if *vterm_if_key = reinterpret_cast<key_vterm_if_t*>
                  (malloc(sizeof(key_vterm_if_t)));
  memset(vterm_if_key, 0 ,sizeof(key_vterm_if_t));
  strncpy((char*) vterm_if_key->vterm_key.vtn_key.vtn_name, "vtn1", 32);
  strncpy((char*) vterm_if_key->vterm_key.vterminal_name, "vterminal1", 32);
  strncpy((char*) vterm_if_key->if_name, "if1", 32);
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf,
                            vterm_if_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,GetChildConfigKey_okey_NN_key_N) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if *vterm_if_key = reinterpret_cast<key_vterm_if_t*>
                  (malloc(sizeof(key_vterm_if_t)));
  memset(vterm_if_key, 0 ,sizeof(key_vterm_if_t));
  strncpy((char*) vterm_if_key->vterm_key.vtn_key.vtn_name, "vtn1", 32);
  strncpy((char*) vterm_if_key->vterm_key.vterminal_name, "vterminal1", 32);
  strncpy((char*) vterm_if_key->if_name, "if1", 32);
  ConfigKeyVal *okey =  new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf,
                            vterm_if_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,GetChildConfigKey_okey_NN_key_1) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if *vterm_if_key = reinterpret_cast<key_vterm_if_t*>
                  (malloc(sizeof(key_vterm_if_t)));
  memset(vterm_if_key, 0 ,sizeof(key_vterm_if_t));
  strncpy((char*) vterm_if_key->vterm_key.vtn_key.vtn_name, "vtn1", 32);
  strncpy((char*) vterm_if_key->vterm_key.vterminal_name, "vterminal1", 32);
  strncpy((char*) vterm_if_key->if_name, "if1", 32);
  key_vterm_if_flowfilter_t *o_vterm_key = NULL;
  AllocKeyStruct(o_vterm_key);
  ConfigKeyVal *okey =  new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            o_vterm_key, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf,
                            vterm_if_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}
/******************************************************************************
 * Function - DupConfigKeyVal
 *****************************************************************************/

TEST(VterminalMoMgrTest,DupConfigKeyVal_pkey_null) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, pkey,MAINTBL));
}

TEST(VterminalMoMgrTest,DupConfigKeyVal_pkey_wrong_KT) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, pkey, MAINTBL));
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,DupConfigKeyVal_okey_not_null) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
   pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, pkey,MAINTBL));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,DupConfigKeyVal_pkey_key_null) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, pkey, MAINTBL));
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VterminalMoMgrTest,DupConfigKeyVal_pkey_key_valid) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  key_vterm_if_flowfilter_t *vkey = NULL;
  AllocKeyStruct(vkey);
  GetKeyStruct(vkey);
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vkey, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, pkey, MAINTBL));
  DELETE_IF_NOT_NULL(pkey);
  DELETE_IF_NOT_NULL(okey);
}

TEST(VterminalMoMgrTest,DupConfigKeyVal_pkey_key_val_valid) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  key_vterm_if_flowfilter_t *vkey = NULL;
  AllocKeyStruct(vkey);
  GetKeyStruct(vkey);
  ConfigVal *val = NULL;
  AllocConfigVal(val);
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vkey, val);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, pkey, MAINTBL));
  DELETE_IF_NOT_NULL(pkey);
  DELETE_IF_NOT_NULL(okey);
}

TEST(VterminalMoMgrTest,DupConfigKeyVal_wrong_table) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  key_vterm_if_flowfilter_t *vkey = NULL;
  AllocKeyStruct(vkey);
  GetKeyStruct(vkey);
  ConfigVal *val = NULL;
  AllocConfigVal(val);
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vkey, val);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, pkey, CTRLRTBL));
  DELETE_IF_NOT_NULL(pkey);
  DELETE_IF_NOT_NULL(okey);
}

TEST(VterminalMoMgrTest,DupConfigKeyVal_okey_N_NULL) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  key_vterm_if_flowfilter_t *vkey = NULL;
  AllocKeyStruct(vkey);
  GetKeyStruct(vkey);
  ConfigVal *val = NULL;
  AllocConfigVal(val);
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vkey, val);
  okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, pkey, MAINTBL));
  DELETE_IF_NOT_NULL(pkey);
  DELETE_IF_NOT_NULL(okey);
}

/******************************************************************************
 * Function - UpdateMo
 *****************************************************************************/
TEST(VterminalMoMgrTest, UpdateMo) {
  VtermIfFlowFilterMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.UpdateMo(req, ikey, dmi));
}

/******************************************************************************
 * Function - RenameMo
 *****************************************************************************/
TEST(VterminalMoMgrTest, RenameMo) {
  VtermIfFlowFilterMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  const char *ctrlr_id = NULL;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.RenameMo(req, ikey, dmi, ctrlr_id));
}


/******************************************************************************
 * Function - UpdateAuditConfigStatus
 *****************************************************************************/
TEST(VterminalMoMgrTest, UpdateAuditConfigStatus_val_null) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ckv = NULL;
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateAuditConfigStatus(UNC_CS_INVALID,
          uuc::kUpllUcpCreate, ckv, dmi));
}

TEST(VterminalMoMgrTest, UpdateAuditConfigStatus_val_null_1) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateAuditConfigStatus(UNC_CS_INVALID,
          uuc::kUpllUcpCreate, ckv, dmi));
  DELETE_IF_NOT_NULL(ckv);
}

TEST(VterminalMoMgrTest, UpdateAuditConfigStatus_val_valid_create) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigVal *val = NULL;
  AllocConfigVal(val);
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, val);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_INVALID,
          uuc::kUpllUcpCreate, ckv, dmi));
  DELETE_IF_NOT_NULL(ckv);
}

TEST(VterminalMoMgrTest, UpdateAuditConfigStatus_val_valid_update) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigVal *val = NULL;
  AllocConfigVal(val);
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, val);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_INVALID,
          uuc::kUpllUcpUpdate, ckv, dmi));
  DELETE_IF_NOT_NULL(ckv);
}

TEST(VterminalMoMgrTest, UpdateAuditConfigStatus_val_valid_update_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigVal *val = NULL;
  AllocConfigVal(val);
  val_flowfilter_t *val_ff = reinterpret_cast<val_flowfilter_t *>(val->get_val());
  val_ff->cs_row_status = UNC_CS_INVALID;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, val);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_INVALID,
          uuc::kUpllUcpUpdate, ckv, dmi));
  DELETE_IF_NOT_NULL(ckv);
}

/******************************************************************************
 * Function - IsValidKey
 *****************************************************************************/
TEST(VterminalMoMgrTest, IsValidKey_key_null) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiVtnName));
}

TEST(VterminalMoMgrTest, IsValidKey_key_valid_vtn) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetKeyStruct(vterm_ff_key);
  EXPECT_EQ(true, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiVtnName));
  free(vterm_ff_key);
}

TEST(VterminalMoMgrTest, IsValidKey_key_valid_vterm) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetKeyStruct(vterm_ff_key);
  EXPECT_EQ(true, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiVtermName));
  free(vterm_ff_key);
}

TEST(VterminalMoMgrTest, IsValidKey_key_valid_vterm_if) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetKeyStruct(vterm_ff_key);
  EXPECT_EQ(true, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiVtermIfName));
  free(vterm_ff_key);
}

TEST(VterminalMoMgrTest, IsValidKey_key_valid_vterm_dir) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetKeyStruct(vterm_ff_key);
  EXPECT_EQ(true, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiInputDirection));
  free(vterm_ff_key);
}

TEST(VterminalMoMgrTest, IsValidKey_key_invalid_vtn) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiVtnName));
  free(vterm_ff_key);
}

TEST(VterminalMoMgrTest, IsValidKey_key_invalid_vterm) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiVtermName));
  free(vterm_ff_key);
}

TEST(VterminalMoMgrTest, IsValidKey_key_invalid_vterm_if) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiVtermIfName));
  free(vterm_ff_key);
}

TEST(VterminalMoMgrTest, IsValidKey_key_invalid_vterm_dir) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiInputDirection));
  free(vterm_ff_key);
}

TEST(VterminalMoMgrTest, IsValidKey_key_invalid_vterm_dir_1) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  vterm_ff_key->direction = 10;
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiInputDirection));
  free(vterm_ff_key);
}

TEST(VterminalMoMgrTest, IsValidKey_key_invalid_vterm_dir_2) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  vterm_ff_key->direction = 10;
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter::kDbiCsRowStatus));
  free(vterm_ff_key);
}

/******************************************************************************
 * Function - UpdateConfigStatus
 *****************************************************************************/
TEST(VterminalMoMgrTest, UpdateConfigStatus_val_null) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  ConfigKeyVal *upd_ckv = NULL, *ctrlr_ckv = NULL;
  DalDmlIntf *dmi = NULL;
  uint32_t result = 0;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(ckv, UNC_OP_CREATE, result,
      upd_ckv, dmi, ctrlr_ckv)); 
  DELETE_IF_NOT_NULL(ckv);
}

TEST(VterminalMoMgrTest, UpdateConfigStatus_val_wrong_op) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *upd_ckv = NULL, *ctrlr_ckv = NULL;
  ConfigVal *cfg_val = NULL;
  AllocConfigVal(cfg_val);
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, cfg_val);
  uint32_t result = 0;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(ckv, UNC_OP_UPDATE, result,
      upd_ckv, dmi, ctrlr_ckv));
  DELETE_IF_NOT_NULL(ckv);
}

TEST(VterminalMoMgrTest, UpdateConfigStatus_val_create_op) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *upd_ckv = NULL, *ctrlr_ckv = NULL;
  ConfigVal *cfg_val = NULL;
  AllocConfigVal(cfg_val);
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, cfg_val);
  uint32_t result = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(ckv, UNC_OP_CREATE, result,
      upd_ckv, dmi, ctrlr_ckv));
  DELETE_IF_NOT_NULL(ckv);
}

/******************************************************************************
 * Function - CopyToConfigKey
 *****************************************************************************/
TEST(VterminalMoMgrTest, CopyToConfigKey_ikey_null) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
}

TEST(VterminalMoMgrTest, CopyToConfigKey_ikey_key_null) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CopyToConfigKey_ikey_key_no_vtn_name) {
  VtermIfFlowFilterMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStructNoFill(key, 1);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CopyToConfigKey_ikey_key_no_old_vnode_name) {
  VtermIfFlowFilterMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStructNoFill(key, 3);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CopyToConfigKey_ikey_key__old_vnode_name) {
  VtermIfFlowFilterMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStruct(key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CopyToConfigKey_ikey_key_no_new_vnode_name) {
  VtermIfFlowFilterMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStructNoFill(key, 2);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CopyToConfigKey_success_1) {
  VtermIfFlowFilterMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStruct(key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CopyToConfigKey_success_2) {
  VtermIfFlowFilterMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStruct(key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - GetRenameKeyBindInfo
 *****************************************************************************/
TEST(VterminalMoMgrTest, GetRenameKeyBindInfo_wrong_table) {
  VtermIfFlowFilterMoMgr obj;
  BindInfo *binfo = NULL;
  int nattr = 0;
  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(UNC_KT_VTERMIF_FLOWFILTER,
          binfo, nattr, CTRLRTBL));
}

TEST(VterminalMoMgrTest, GetRenameKeyBindInfo_main_table) {
  VtermIfFlowFilterMoMgr obj;
  BindInfo *binfo = NULL;
  int nattr = 0;
  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(UNC_KT_VTERMIF_FLOWFILTER,
          binfo, nattr, MAINTBL));
}


/******************************************************************************
 * Function - ValidateMessage
 *****************************************************************************/
TEST(VterminalMoMgrTest, ValidateMessage_req_ikey_null) {
  VtermIfFlowFilterMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));

  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  free(req);
}

TEST(VterminalMoMgrTest, ValidateMessage_ikey_wrong) {
  VtermIfFlowFilterMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  DELETE_IF_NOT_NULL(ikey);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVterminal,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  DELETE_IF_NOT_NULL(ikey);
  free(req);
}

TEST(VterminalMoMgrTest, ValidateMessage_wrong_options) {
  VtermIfFlowFilterMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_L2DOMAIN,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));
  free(req);

  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_COUNT, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
  free(req);

  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING_COUNT,
                     100, UNC_OPT1_DETAIL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
  free(req);

  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING,
                     100, UNC_OPT1_DETAIL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
  free(req);

  GetIpcReqResHeader(5, 14, UNC_OP_READ,
                     100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
  free(req);

  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING_BEGIN,
                     100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
  free(req);

  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING_COUNT,
                     100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
  free(req);

  GetIpcReqResHeader(5, 14, UNC_OP_READ_NEXT,
                     100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
  free(req);

  GetIpcReqResHeader(5, 14, UNC_OP_READ_BULK,
                     100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
  free(req);

  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING_COUNT,
                     100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
  free(req);
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ValidateMessage_no_key) {
  VtermIfFlowFilterMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  free(req);
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - ValidateAttribute
 *****************************************************************************/
TEST(VterminalMoMgrTest, ValidateAttribute_success) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  IpcReqRespHeader *req = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey, dmi, req));
}

/******************************************************************************
 * Function - FilterAttributes
 *****************************************************************************/
TEST(VterminalMoMgrTest, FilterAttributes_create) {
  VtermIfFlowFilterMoMgr obj;
  void *val_1 = NULL, *val_2 = NULL;
  EXPECT_EQ(false, obj.FilterAttributes(val_1, val_2, true, UNC_OP_CREATE));
}

TEST(VterminalMoMgrTest, FilterAttributes_other) {
  VtermIfFlowFilterMoMgr obj;
  void *val_1 = NULL, *val_2 = NULL;
  EXPECT_EQ(true, obj.FilterAttributes(val_1, val_2, true, UNC_OP_DELETE));
}

/******************************************************************************
 * Function - SetValidAudit
 *****************************************************************************/
TEST(VterminalMoMgrTest, SetValidAudit_success) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.SetValidAudit(ikey));
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - DeleteChildrenPOM
 *****************************************************************************/
TEST(VterminalMoMgrTest, DeleteChildrenPOM_wrong_input) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
            dmi, config_mode, vtn_name));
}

TEST(VterminalMoMgrTest, DeleteChildrenPOM_wrong_input_1) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  DalDmlIntf *dmi = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
            dmi, config_mode, vtn_name));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, DeleteChildrenPOM_wrong_KT) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  DalDmlIntf *dmi = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE, dmi,
            config_mode, vtn_name));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, DeleteChildrenPOM_UC_success) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalDmlIntf *dmi = new DalOdbcMgr;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE, dmi,
            config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, DeleteChildrenPOM_UC_failure_1) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalDmlIntf *dmi = new DalOdbcMgr;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD, kDalRcDataError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE, 
            dmi, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, DeleteChildrenPOM_UC_failure_2) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalDmlIntf *dmi = new DalOdbcMgr;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE, dmi,
            config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, DeleteChildrenPOM_UC_failure_3) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalDmlIntf *dmi = new DalOdbcMgr;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
             dmi, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - GetParentConfigKey
 *****************************************************************************/
TEST(VterminalMoMgrTest, GetParentConfigKey_ikey_null) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL, *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetParentConfigKey(okey, ikey));
}

TEST(VterminalMoMgrTest, GetParentConfigKey_wrong_ikey) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetParentConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, GetParentConfigKey_ikey_null_key) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetParentConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, GetParentConfigKey_ikey_success) {
  VtermIfFlowFilterMoMgr obj;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetParentConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(okey);
}

/******************************************************************************
 * Function - ConstructReadDetailResponse
 *****************************************************************************/
TEST(VterminalMoMgrTest, ConstructReadDetailResponse_fail) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *drv_ckv = NULL;
  ConfigVal *cfg_val = NULL;
  AllocEntryConfigVal(cfg_val);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  controller_domain ctrl_domain;
  DalDmlIntf *dmi = NULL;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  drv_ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ConstructReadDetailResponse(
        ikey, drv_ckv, ctrl_domain, &okey, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(drv_ckv);
}

TEST(VterminalMoMgrTest, ConstructReadDetailResponse_fail_1) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *drv_ckv = NULL;
  ConfigVal *cfg_val = NULL;
  AllocEntryConfigVal(cfg_val);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  key_vterm_if_flowfilter_t *vterm_key_1 = NULL;
  AllocKeyStruct(vterm_key_1);
  GetKeyStruct(vterm_key_1);
  controller_domain ctrl_domain;
  DalDmlIntf *dmi = NULL;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  drv_ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key_1, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ConstructReadDetailResponse(
        ikey, drv_ckv, ctrl_domain, &okey, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(drv_ckv);
}

TEST(VterminalMoMgrTest, ConstructReadDetailResponse_pass_1) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *drv_ckv = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  key_vterm_if_flowfilter_t *vterm_key_1 = NULL;
  AllocKeyStruct(vterm_key_1);
  GetKeyStruct(vterm_key_1);
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  controller_domain ctrl_domain;
  DalDmlIntf *dmi = NULL;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  drv_ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key_1, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ConstructReadDetailResponse(
        ikey, drv_ckv, ctrl_domain, &okey, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(drv_ckv);
  DELETE_IF_NOT_NULL(okey);
}

TEST(VterminalMoMgrTest, ConstructReadDetailResponse_pass_2) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *drv_ckv = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  key_vterm_if_flowfilter_t *vterm_key_1 = NULL;
  AllocKeyStruct(vterm_key_1);
  GetKeyStruct(vterm_key_1);
  key_vterm_if_flowfilter_t *vterm_key_2 = NULL;
  AllocKeyStruct(vterm_key_2);
  GetKeyStruct(vterm_key_2);
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  controller_domain ctrl_domain;
  DalDmlIntf *dmi = NULL;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  drv_ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key_1, cfg_val);
  okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key_2, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ConstructReadDetailResponse(
        ikey, drv_ckv, ctrl_domain, &okey, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(drv_ckv);
  DELETE_IF_NOT_NULL(okey);
}

TEST(VterminalMoMgrTest, ConstructReadDetailResponse_pass_3) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *drv_ckv = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  key_vterm_if_flowfilter_t *vterm_key_1 = NULL;
  AllocKeyStruct(vterm_key_1);
  GetKeyStruct(vterm_key_1);
  key_vterm_if_flowfilter_t *vterm_key_2 = NULL;
  AllocKeyStruct(vterm_key_2);
  GetKeyStruct(vterm_key_2);
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigVal *cfg_val_ffe = NULL;
  AllocEntryConfigVal(cfg_val_ffe);
  controller_domain ctrl_domain;
  DalDmlIntf *dmi = NULL;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  drv_ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val_ffe);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key_1, cfg_val);
  okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key_2, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ConstructReadDetailResponse(
        ikey, drv_ckv, ctrl_domain, &okey, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(drv_ckv);
  DELETE_IF_NOT_NULL(okey);
}

/******************************************************************************
 * Function - GetControllerDomainID
 *****************************************************************************/
TEST(VterminalMoMgrTest, GetControllerDomainID_ikey_null) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetControllerDomainID(ikey, UPLL_DT_CANDIDATE, dmi));
}

TEST(VterminalMoMgrTest, GetControllerDomainID_1) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = new DalOdbcMgr;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.GetControllerDomainID(ikey, UPLL_DT_CANDIDATE, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST(VterminalMoMgrTest, GetControllerDomainID_2) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = new DalOdbcMgr;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetControllerDomainID(ikey, UPLL_DT_CANDIDATE, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST(VterminalMoMgrTest, GetControllerDomainID_3) {
  VtermIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetControllerDomainID(ikey, UPLL_DT_CANDIDATE, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DalOdbcMgr::clearStubData();
}

/******************************************************************************
 * Function - GetRenamedControllerKey
 *****************************************************************************/
TEST(VterminalMoMgrTest, GetRenamedControllerKey_obj_null) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = NULL;
  controller_domain *ctrlr_dom = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedControllerKey(ikey, UPLL_DT_CANDIDATE,
        dmi, ctrlr_dom));
}

TEST(VterminalMoMgrTest, GetRenamedControllerKey_ctrlr_dom_null) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = NULL;
  controller_domain *ctrlr_dom = NULL;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedControllerKey(ikey, UPLL_DT_CANDIDATE,
        dmi, ctrlr_dom));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST(VterminalMoMgrTest, GetRenamedControllerKey_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  controller_domain ctrlr_dom1;
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  ctrlr_dom1.ctrlr = ctr_id1;
  ctrlr_dom1.domain = NULL;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetRenamedControllerKey(ikey, UPLL_DT_CANDIDATE,
        dmi, &ctrlr_dom1));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST(VterminalMoMgrTest, GetRenamedControllerKey_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  controller_domain ctrlr_dom1;
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  ctrlr_dom1.ctrlr = ctr_id1;
  ctrlr_dom1.domain = NULL;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedControllerKey(ikey, UPLL_DT_CANDIDATE,
        dmi, &ctrlr_dom1));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST(VterminalMoMgrTest, GetRenamedControllerKey_3) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  controller_domain ctrlr_dom1;
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  ctrlr_dom1.ctrlr = ctr_id1;
  ctrlr_dom1.domain = NULL;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetRenamedControllerKey(ikey, UPLL_DT_CANDIDATE,
        dmi, &ctrlr_dom1));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

/******************************************************************************
 * Function - GetRenamedUncKey
 *****************************************************************************/
TEST(VterminalMoMgrTest, GetRenamedUncKey_fail_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = NULL;
  uint8_t *ctr_id1 = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
}

TEST(VterminalMoMgrTest, GetRenamedUncKey_fail_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = NULL;
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
}

TEST(VterminalMoMgrTest, GetRenamedUncKey_fail_3) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, GetRenamedUncKey_fail_4) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, GetRenamedUncKey_fail_5) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, GetRenamedUncKey_fail_6) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, GetRenamedUncKey_fail_7) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, GetRenamedUncKey_pass_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, GetRenamedUncKey_pass_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - SetPortmapConfiguration
 *****************************************************************************/
TEST(VterminalMoMgrTest, SetPortmapConfiguration_fail_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  InterfacePortMapInfo flags = kPortMapConfigured;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, config_mode, vtn_name));
}

TEST(VterminalMoMgrTest, SetPortmapConfiguration_fail_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, config_mode, vtn_name));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, SetPortmapConfiguration_fail_3) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, SetPortmapConfiguration_no_ins) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.SetPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, SetPortmapConfiguration_fail_4) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, SetPortmapConfiguration_fail_5) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.SetPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, SetPortmapConfiguration_fail_6) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - CreateCandidateMo
 *****************************************************************************/
TEST(VterminalMoMgrTest, CreateCandidateMo_fail_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_3) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_4) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcGeneralError);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_5) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_junk) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}


TEST(VterminalMoMgrTest, CreateCandidateMo_fail_junk_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  EXPECT_EQ(UNC_UPLL_RC_ERR_PARENT_DOES_NOT_EXIST, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_6) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_7) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_PARENT_DOES_NOT_EXIST, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_8) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_9) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_10) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_11) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateCandidateMo_fail_12) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_PARENT_DOES_NOT_EXIST, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - CreateAuditMoImpl
 *****************************************************************************/
TEST(VterminalMoMgrTest, CreateAuditMoImpl_fail_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  const char *ctrlr_id = "ctrlr_1";
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateAuditMoImpl(ikey, dmi, ctrlr_id));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateAuditMoImpl_fail_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  const char *ctrlr_id = "ctrlr_1";
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.CreateAuditMoImpl(ikey, dmi, ctrlr_id));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateAuditMoImpl_fail_3) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  const char *ctrlr_id = "ctrlr_1";
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.CreateAuditMoImpl(ikey, dmi, ctrlr_id));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateAuditMoImpl_fail_4) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  const char *ctrlr_id = "ctrlr_1";
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateAuditMoImpl(ikey, dmi, ctrlr_id));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, CreateAuditMoImpl_pass) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  const char *ctrlr_id = "ctrlr_1";
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CreateAuditMoImpl(ikey, dmi, ctrlr_id));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - IsReferenced
 *****************************************************************************/
TEST(VterminalMoMgrTest, IsReferenced_fail_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, ikey, dmi));
}

TEST(VterminalMoMgrTest, IsReferenced_fail_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, IsReferenced_pass) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.IsReferenced(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - ReadMo
 *****************************************************************************/
TEST(VterminalMoMgrTest, ReadMo_fail_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ReadMo(req, ikey, dmi));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadMo_fail_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ReadMo(req, ikey, dmi));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadMo_fail_3) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadMo_fail_4) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_RUNNING, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadMo_fail_5) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STARTUP, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadMo_fail_6) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadMo_fail_7) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_DETAIL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadMo_pass_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadMo_fail_8) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_DETAIL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - ReadSiblingMo
 *****************************************************************************/
TEST(VterminalMoMgrTest, ReadSiblingMo_fail_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ReadSiblingMo(req, ikey, false, dmi));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadSiblingMo_fail_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadSiblingMo_fail_3) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_RUNNING, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadSiblingMo_fail_4) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STARTUP, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadSiblingMo_fail_5) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadSiblingMo_fail_6) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_DETAIL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadSiblingMo_fail_7) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_DETAIL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);

  //CapaModuleStub::stub_loadCapaModule();
  //CapaModuleStub::stub_unloadCapaModule();
  CtrlrMgr::Ctrlr ctrlrobj("ctrlr_id", UNC_CT_PFC, "5.1",0);
  CtrlrMgr::GetInstance()->Add(ctrlrobj, UPLL_DT_CANDIDATE);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  CtrlrMgr::GetInstance()->Delete("ctrlr_id", UPLL_DT_CANDIDATE);
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VterminalMoMgrTest, ReadSiblingMo_pass) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

/******************************************************************************
 * Function - TxUpdateController
 *****************************************************************************/
TEST(VterminalMoMgrTest, TxUpdateController_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS, kDalRcGeneralError);
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  set<string> *affected_ctrlr_set = NULL;
  //uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  ConfigKeyVal *err_ckv = NULL;
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                           obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER,session_id,config_id,uuc::kUpllUcpCreate,affected_ctrlr_set,dmi,&err_ckv, &tx,
                                                  config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VterminalMoMgrTest, TxUpdateController_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS, kDalRcSuccess);
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  set<string> *affected_ctrlr_set = NULL;
  //uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  ConfigKeyVal *err_ckv = NULL;
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                           obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER,session_id,config_id,uuc::kUpllUcpCreate,affected_ctrlr_set,dmi,&err_ckv, &tx,
                                                  config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VterminalMoMgrTest, TxUpdateController_3) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  set<string> *affected_ctrlr_set = NULL;
  //uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  ConfigKeyVal *err_ckv = NULL;
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                           obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER,session_id,config_id,uuc::kUpllUcpCreate,affected_ctrlr_set,dmi,&err_ckv, &tx,
                                                  config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VterminalMoMgrTest, TxUpdateController_4) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  set<string> *affected_ctrlr_set = NULL;
  //uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  ConfigKeyVal *err_ckv = NULL;
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                           obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER,session_id,config_id,uuc::kUpllUcpUpdate,affected_ctrlr_set,dmi,&err_ckv, &tx,
                                                  config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VterminalMoMgrTest, TxUpdateController_5) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  set<string> *affected_ctrlr_set = NULL;
  //uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  ConfigKeyVal *err_ckv = NULL;
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                           obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER,session_id,config_id,uuc::kUpllUcpDelete,affected_ctrlr_set,dmi,&err_ckv, &tx,
                                                  config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VterminalMoMgrTest, TxUpdateController_6) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  set<string> *affected_ctrlr_set = NULL;
  //uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  ConfigKeyVal *err_ckv = NULL;
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                           obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER,session_id,config_id,uuc::kUpllUcpInvalid,affected_ctrlr_set,dmi,&err_ckv, &tx,
                                                  config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VterminalMoMgrTest, TxUpdateController_7) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  //set<string> *affected_ctrlr_set;
  //uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  ConfigKeyVal *err_ckv = NULL;
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                           obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER,session_id,config_id,uuc::kUpllUcpInvalid,NULL,dmi,&err_ckv, &tx,
                                                  config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

/******************************************************************************
 * AuditUpdateController
 *****************************************************************************/
TEST(VterminalMoMgrTest, AuditUpdateController_1) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  const char *ctrlr_id = "ctrlr_id";
  KTxCtrlrAffectedState affect;
  ConfigKeyVal *err_ckv = NULL;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AuditUpdateController(UNC_KT_VTERMIF_FLOWFILTER, ctrlr_id, session_id, config_id, uuc::kUpllUcpCreate,
                                                       dmi, &err_ckv, &affect));
}

TEST(VterminalMoMgrTest, AuditUpdateController_2) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  const char *ctrlr_id = "ctrlr_id";
  KTxCtrlrAffectedState affect;
  ConfigKeyVal *err_ckv = NULL;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, obj.AuditUpdateController(UNC_KT_VTERMIF_FLOWFILTER, ctrlr_id, session_id, config_id, uuc::kUpllUcpUpdate,
                                                       dmi, &err_ckv, &affect));
  DalOdbcMgr::clearStubData();
}

TEST(VterminalMoMgrTest, AuditUpdateController_3) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  const char *ctrlr_id = "ctrlr_id";
  KTxCtrlrAffectedState affect;
  ConfigKeyVal *err_ckv = NULL;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, obj.AuditUpdateController(UNC_KT_VTERMIF_FLOWFILTER, ctrlr_id, session_id, config_id, uuc::kUpllUcpDelete,
                                                       dmi, &err_ckv, &affect));
  DalOdbcMgr::clearStubData();
}

TEST(VterminalMoMgrTest, AuditUpdateController_4) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  const char *ctrlr_id = "ctrlr_id";
  KTxCtrlrAffectedState affect;
  ConfigKeyVal *err_ckv = NULL;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS, kDalRcRecordNotFound);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, obj.AuditUpdateController(UNC_KT_VTERMIF_FLOWFILTER, ctrlr_id, session_id, config_id, uuc::kUpllUcpUpdate,
                                                       dmi, &err_ckv, &affect));
  DalOdbcMgr::clearStubData();
}

TEST(VterminalMoMgrTest, AuditUpdateController_5) {
  VtermIfFlowFilterMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  const char *ctrlr_id = "ctrlr_id";
  KTxCtrlrAffectedState affect;
  ConfigKeyVal *err_ckv = NULL;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, obj.AuditUpdateController(UNC_KT_VTERMIF_FLOWFILTER, ctrlr_id, session_id, config_id, uuc::kUpllUcpUpdate,
                                                       dmi, &err_ckv, &affect));
  DalOdbcMgr::clearStubData();
}


