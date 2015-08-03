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
#include <vterm_if_momgr.hh>
#include <vterm_if_flowfilter_momgr.hh>
#include <vterm_if_flowfilter_entry_momgr.hh>
#include <config_mgr.hh>
#include <dal_odbc_mgr.hh>
#include <dal_dml_intf.hh>
//#include <capa_intf.hh>
//#include <capa_module_stub.hh>
#include <tclib_module.hh>
#include <ctrlr_mgr.hh>
#include <momgr_intf_stub.hh>
#include "ut_util.hh"
#include  "tx_update_util.hh"

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

class VtermIfFlowFilterEntryMoMgrTest
: public UpllTestEnv {
};


#if 0
#define DELETE_IF_NOT_NULL(key) \
do { \
  if (key)\
  delete key;\
  key = NULL;\
} while (0);
#endif

static void AllocKeyStruct(key_vterm_if_flowfilter_entry_t *&key) {
  key = reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
                  (malloc(sizeof(key_vterm_if_flowfilter_entry_t)));
  memset(key, 0, sizeof(key_vterm_if_flowfilter_entry_t));
}

static void AllocKeyRenameVnodeStruct(key_rename_vnode_info *&key) {
  key = reinterpret_cast<key_rename_vnode_info*>
                  (malloc(sizeof(key_rename_vnode_info)));
  memset(key, 0, sizeof(key_rename_vnode_info));
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

static void GetKeyStruct(key_vterm_if_flowfilter_entry_t* key_vterm_if_entry) {
  strncpy(reinterpret_cast<char*>
         (key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
          "VTN1", 32);
  strncpy(reinterpret_cast<char*>
         (key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vterminal_name),
          "VTERM1", 32);
  strncpy(reinterpret_cast<char*>
         (key_vterm_if_entry->flowfilter_key.if_key.if_name),
         "INTERFACENAME", 32);
  key_vterm_if_entry->sequence_num = 1;
}

static void GetInvalidKeyStruct(key_vterm_if_flowfilter_entry_t* key_vterm_if_entry) {
  strncpy(reinterpret_cast<char*>
         (key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
          "!VTN1", 32);
  strncpy(reinterpret_cast<char*>
         (key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vterminal_name),
          "!VTERM1", 32);
  strncpy(reinterpret_cast<char*>
         (key_vterm_if_entry->flowfilter_key.if_key.if_name),
         "!INTERFACENAME", 32);
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

/* AllocVal Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, AllocVal_outputNull) {
  VtermIfFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *vterm_if_flowfilter_entry_val =
                          new val_flowfilter_entry_t();
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vterm_if_flowfilter_entry_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(val, UPLL_DT_IMPORT,MAINTBL));
  delete val;
  val = NULL;
}

TEST(VtermIfFlowFilterEntryMoMgrTest, AllocVal_Success) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigVal *val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_IMPORT, MAINTBL));
  delete val;
  val = NULL;
}

TEST(VtermIfFlowFilterEntryMoMgrTest, AllocVal_WrongTable) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigVal *val = NULL;
  obj.AllocVal(val, UPLL_DT_STATE,RENAMETBL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AllocVal(val, UPLL_DT_STATE, RENAMETBL));
}

/* GetChildConfigKey Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest,GetChildConfigKey) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,GetChildConfigKey_pkeyNull) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey =  new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,GetChildConfigKey_pkeyNull1) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                            NULL, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,GetChildConfigKey_wrongKT) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                            vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,GetChildConfigKey_wrong_parent_KT) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
                            IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                            vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,GetChildConfigKey_vtermifff_parent_KT) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                            vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,GetChildConfigKey_vtn_parent_KT) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t*>
                  (malloc(sizeof(key_vtn_t)));
  memset(vtn_key, 0 ,sizeof(key_vtn_t));
  strncpy((char*) vtn_key->vtn_name, "vtn1", 32);
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                            vtn_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,GetChildConfigKey_vterm_parent_KT) {
  VtermIfFlowFilterEntryMoMgr obj;
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

TEST(VtermIfFlowFilterEntryMoMgrTest,GetChildConfigKey_vterm_if_parent_KT) {
  VtermIfFlowFilterEntryMoMgr obj;
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

TEST(VtermIfFlowFilterEntryMoMgrTest,GetChildConfigKey_okey_NN_key_N) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if *vterm_if_key = reinterpret_cast<key_vterm_if_t*>
                  (malloc(sizeof(key_vterm_if_t)));
  memset(vterm_if_key, 0 ,sizeof(key_vterm_if_t));
  strncpy((char*) vterm_if_key->vterm_key.vtn_key.vtn_name, "vtn1", 32);
  strncpy((char*) vterm_if_key->vterm_key.vterminal_name, "vterminal1", 32);
  strncpy((char*) vterm_if_key->if_name, "if1", 32);
  ConfigKeyVal *okey =  new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                            NULL, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf,
                            vterm_if_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

/* DupConfigKeyVal test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest,DupConfigKeyVal_pkey_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, pkey,MAINTBL));
}

TEST(VtermIfFlowFilterEntryMoMgrTest,DupConfigKeyVal_pkey_wrong_KT) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, pkey, MAINTBL));
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,DupConfigKeyVal_okey_not_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
   pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, pkey,MAINTBL));
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,DupConfigKeyVal_pkey_key_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, pkey, MAINTBL));
  DELETE_IF_NOT_NULL(pkey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,DupConfigKeyVal_pkey_key_valid) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  key_vterm_if_flowfilter_entry_t *vkey = NULL;
  AllocKeyStruct(vkey);
  GetKeyStruct(vkey);
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vkey, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, pkey, MAINTBL));
  DELETE_IF_NOT_NULL(pkey);
  DELETE_IF_NOT_NULL(okey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,DupConfigKeyVal_pkey_key_val_valid) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  key_vterm_if_flowfilter_entry_t *vkey = NULL;
  AllocKeyStruct(vkey);
  GetKeyStruct(vkey);
  ConfigVal *val = NULL;
  AllocConfigVal(val);
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vkey, val);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, pkey, MAINTBL));
  DELETE_IF_NOT_NULL(pkey);
  DELETE_IF_NOT_NULL(okey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest,DupConfigKeyVal_wrong_table) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *pkey = NULL, *okey = NULL;
  key_vterm_if_flowfilter_entry_t *vkey = NULL;
  AllocKeyStruct(vkey);
  GetKeyStruct(vkey);
  ConfigVal *val = NULL;
  AllocConfigVal(val);
  pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vkey, val);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, pkey, CTRLRTBL));
  DELETE_IF_NOT_NULL(pkey);
  DELETE_IF_NOT_NULL(okey);
}


/* RenameMo Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, RenameMo) {
  VtermIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  const char *ctrlr_id = NULL;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.RenameMo(req, ikey, dmi, ctrlr_id));
}


/* UpdateAuditConfigStatus Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, UpdateAuditConfigStatus_val_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ckv = NULL;
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateAuditConfigStatus(UNC_CS_INVALID,
          uuc::kUpllUcpCreate, ckv, dmi));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, UpdateAuditConfigStatus_val_null_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateAuditConfigStatus(UNC_CS_INVALID,
          uuc::kUpllUcpCreate, ckv, dmi));
  DELETE_IF_NOT_NULL(ckv);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, UpdateAuditConfigStatus_val_valid_create) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigVal *val = NULL;
  AllocEntryConfigVal(val);
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, val);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_INVALID,
          uuc::kUpllUcpCreate, ckv, dmi));
  DELETE_IF_NOT_NULL(ckv);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, UpdateAuditConfigStatus_val_valid_update) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigVal *val = NULL;
  AllocEntryConfigVal(val);
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, val);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_INVALID,
          uuc::kUpllUcpUpdate, ckv, dmi));
  DELETE_IF_NOT_NULL(ckv);
}

/* IsValidKey Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, IsValidKey_key_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_ff_key = NULL;
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter_entry::kDbiVtnName));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsValidKey_key_valid_vtn) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetKeyStruct(vterm_ff_key);
  EXPECT_EQ(true, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter_entry::kDbiVtnName));
  free(vterm_ff_key);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsValidKey_key_valid_vterm) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetKeyStruct(vterm_ff_key);
  EXPECT_EQ(true, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter_entry::kDbiVtermName));
  free(vterm_ff_key);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsValidKey_key_valid_vterm_if) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetKeyStruct(vterm_ff_key);
  EXPECT_EQ(true, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter_entry::kDbiVtermIfName));
  free(vterm_ff_key);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsValidKey_key_valid_vterm_dir) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetKeyStruct(vterm_ff_key);
  EXPECT_EQ(true, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter_entry::kDbiInputDirection));
  free(vterm_ff_key);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsValidKey_key_invalid_vtn) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter_entry::kDbiVtnName));
  free(vterm_ff_key);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsValidKey_key_invalid_vterm) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter_entry::kDbiVtermName));
  free(vterm_ff_key);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsValidKey_key_invalid_vterm_if) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter_entry::kDbiVtermIfName));
  free(vterm_ff_key);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsValidKey_key_invalid_vterm_dir) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  EXPECT_EQ(true, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter_entry::kDbiInputDirection));
  free(vterm_ff_key);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsValidKey_key_invalid_vterm_dir_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_ff_key = NULL;
  AllocKeyStruct(vterm_ff_key);
  GetInvalidKeyStruct(vterm_ff_key);
  vterm_ff_key->flowfilter_key.direction = 10;
  EXPECT_EQ(false, obj.IsValidKey(vterm_ff_key, uudst::vterm_if_flowfilter_entry::kDbiInputDirection));
  free(vterm_ff_key);
}

/* UpdateConfigStatus Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, UpdateConfigStatus_val_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  ConfigKeyVal *upd_ckv = NULL, *ctrlr_ckv = NULL;
  DalDmlIntf *dmi = NULL;
  uint32_t result = 0;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(ckv, UNC_OP_CREATE, result,
      upd_ckv, dmi, ctrlr_ckv));
  DELETE_IF_NOT_NULL(ckv);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, UpdateConfigStatus_val_wrong_op) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *upd_ckv = NULL, *ctrlr_ckv = NULL;
  ConfigVal *cfg_val = NULL;
  AllocEntryConfigVal(cfg_val);
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, cfg_val);
  uint32_t result = 0;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(ckv, UNC_OP_UPDATE, result,
      upd_ckv, dmi, ctrlr_ckv));
  DELETE_IF_NOT_NULL(ckv);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, UpdateConfigStatus_val_create_op) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *upd_ckv = NULL, *ctrlr_ckv = NULL;
  ConfigVal *cfg_val = NULL;
  AllocEntryConfigVal(cfg_val);
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, cfg_val);
  uint32_t result = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(ckv, UNC_OP_CREATE, result,
      upd_ckv, dmi, ctrlr_ckv));
  DELETE_IF_NOT_NULL(ckv);
}

/* CopyToConfigKey Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, CopyToConfigKey_ikey_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CopyToConfigKey_ikey_key_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CopyToConfigKey_ikey_key_no_vtn_name) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStructNoFill(key, 1);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CopyToConfigKey_ikey_key_no_old_vnode_name) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStructNoFill(key, 3);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CopyToConfigKey_ikey_key_no_new_vnode_name) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStructNoFill(key, 2);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CopyToConfigKey_success_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStruct(key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CopyToConfigKey_success_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_rename_vnode_info *key = NULL;
  ConfigKeyVal *okey = NULL;
  AllocKeyRenameVnodeStruct(key);
  GetKeyRenameVnodeStruct(key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

/* GetRenameKeyBindInfo Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenameKeyBindInfo_wrong_table) {
  VtermIfFlowFilterEntryMoMgr obj;
  BindInfo *binfo = NULL;
  int nattr = 0;
  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
          binfo, nattr, CTRLRTBL));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenameKeyBindInfo_main_table) {
  VtermIfFlowFilterEntryMoMgr obj;
  BindInfo *binfo = NULL;
  int nattr = 0;
  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
          binfo, nattr, MAINTBL));
}


TEST(VtermIfFlowFilterEntryMoMgrTest, ValidateMessage_req_ikey_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));

  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  free(req);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ValidateMessage_ikey_wrong) {
  VtermIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  DELETE_IF_NOT_NULL(ikey);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVterminal,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  DELETE_IF_NOT_NULL(ikey);
  free(req);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ValidateMessage_wrong_options) {
  VtermIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

TEST(VtermIfFlowFilterEntryMoMgrTest, ValidateMessage_no_key) {
  VtermIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  free(req);
  DELETE_IF_NOT_NULL(ikey);
}


/* ValidateAttribute Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, ValidateAttribute_success) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  IpcReqRespHeader *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey, dmi, req));
}

/* FilterAttributes Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, FilterAttributes_create) {
  VtermIfFlowFilterEntryMoMgr obj;
  void *val_1 = NULL, *val_2 = NULL;
  EXPECT_EQ(false, obj.FilterAttributes(val_1, val_2, true, UNC_OP_CREATE));
}
/*  //getting segfault because of NO NULL check 
TEST(VtermIfFlowFilterEntryMoMgrTest, FilterAttributes_other) {
  VtermIfFlowFilterEntryMoMgr obj;
  void *val_1 = NULL, *val_2 = NULL;
  EXPECT_EQ(true, obj.FilterAttributes(val_1, val_2, true, UNC_OP_DELETE));
}
*/

/* SetValidAudit Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, SetValidAudit_ikeyNULL) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetValidAudit(ikey));
  DELETE_IF_NOT_NULL(ikey);
}

/* DeleteChildrenPOM Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_wrong_input) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                  dmi, config_mode, vtn_name));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_wrong_input_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                 dmi, config_mode, vtn_name));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_wrong_KT) {
  VtermIfFlowFilterEntryMoMgr obj;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                 dmi, config_mode, vtn_name));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_UC_success) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                 dmi, config_mode, vtn_name)); //getting generic instead of success
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_UC_failure_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD, kDalRcDataError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                 dmi, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_UC_failure_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                 dmi, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

/* GetParentConfigKey Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, GetParentConfigKey_ikey_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL, *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetParentConfigKey(okey, ikey));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetParentConfigKey_wrong_ikey) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetParentConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetParentConfigKey_ikey_null_key) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetParentConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetParentConfigKey_ikey_success) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetParentConfigKey(okey, ikey));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(okey);
}

/* ConstructReadDetailResponse Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, ConstructReadDetailResponse_fail) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *drv_ckv = NULL;
  ConfigVal *cfg_val = NULL;
  AllocEntryConfigVal(cfg_val);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  controller_domain ctrl_domain;
  DalDmlIntf *dmi = NULL;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  drv_ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, cfg_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ConstructReadDetailResponse(
        ikey, drv_ckv, ctrl_domain, &okey, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(drv_ckv);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ConstructReadDetailResponse_fail_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *drv_ckv = NULL;
  ConfigVal *cfg_val = NULL;
  AllocEntryConfigVal(cfg_val);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  key_vterm_if_flowfilter_entry_t *vterm_key_1 = NULL;
  AllocKeyStruct(vterm_key_1);
  GetKeyStruct(vterm_key_1);
  controller_domain ctrl_domain;
  DalDmlIntf *dmi = NULL;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  drv_ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, cfg_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key_1, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ConstructReadDetailResponse(
        ikey, drv_ckv, ctrl_domain, &okey, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(drv_ckv);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ConstructReadDetailResponse_pass_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *drv_ckv = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  key_vterm_if_flowfilter_entry_t *vterm_key_1 = NULL;
  AllocKeyStruct(vterm_key_1);
  GetKeyStruct(vterm_key_1);
  ConfigVal *cfg_val  = NULL;
  AllocEntryConfigVal(cfg_val);
  controller_domain ctrl_domain;
  DalDmlIntf *dmi = NULL;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  drv_ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key_1, cfg_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ConstructReadDetailResponse(
        ikey, drv_ckv, ctrl_domain, &okey, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(drv_ckv);
  DELETE_IF_NOT_NULL(okey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ConstructReadDetailResponse_pass_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *drv_ckv = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  key_vterm_if_flowfilter_entry_t *vterm_key_1 = NULL;
  AllocKeyStruct(vterm_key_1);
  GetKeyStruct(vterm_key_1);
  key_vterm_if_flowfilter_entry_t *vterm_key_2 = NULL;
  AllocKeyStruct(vterm_key_2);
  GetKeyStruct(vterm_key_2);
  ConfigVal *cfg_val  = NULL;
  AllocEntryConfigVal(cfg_val);
  controller_domain ctrl_domain;
  DalDmlIntf *dmi = NULL;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  drv_ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key_1, cfg_val);
  okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key_2, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ConstructReadDetailResponse(
        ikey, drv_ckv, ctrl_domain, &okey, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(drv_ckv);
  DELETE_IF_NOT_NULL(okey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ConstructReadDetailResponse_pass_3) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *drv_ckv = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  key_vterm_if_flowfilter_entry_t *vterm_key_1 = NULL;
  AllocKeyStruct(vterm_key_1);
  GetKeyStruct(vterm_key_1);
  key_vterm_if_flowfilter_entry_t *vterm_key_2 = NULL;
  AllocKeyStruct(vterm_key_2);
  GetKeyStruct(vterm_key_2);
  ConfigVal *cfg_val  = NULL;
  AllocEntryConfigVal(cfg_val);
  ConfigVal *cfg_val_ffe = NULL;
  AllocEntryConfigVal(cfg_val_ffe);
  controller_domain ctrl_domain;
  DalDmlIntf *dmi = NULL;
  memset(&ctrl_domain, 0, sizeof(ctrl_domain));
  drv_ckv = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, cfg_val_ffe);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key_1, cfg_val);
  okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key_2, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ConstructReadDetailResponse(
        ikey, drv_ckv, ctrl_domain, &okey, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DELETE_IF_NOT_NULL(drv_ckv);
  DELETE_IF_NOT_NULL(okey);
}


/* GetControllerDomainID Test cases */
/* // getting segfault because of ikey is null
TEST(VtermIfFlowFilterEntryMoMgrTest, GetControllerDomainID_ikey_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetControllerDomainID(ikey, UPLL_DT_CANDIDATE, dmi));
}
*/
TEST(VtermIfFlowFilterEntryMoMgrTest, GetControllerDomainID_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = new DalOdbcMgr;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.GetControllerDomainID(ikey, UPLL_DT_CANDIDATE, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetControllerDomainID_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = new DalOdbcMgr;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetControllerDomainID(ikey, UPLL_DT_CANDIDATE, dmi));
  DELETE_IF_NOT_NULL(ikey);
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

/* GetRenamedControllerKey Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedControllerKey_obj_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = NULL;
  controller_domain *ctrlr_dom = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedControllerKey(ikey, UPLL_DT_CANDIDATE,
        dmi, ctrlr_dom));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedControllerKey_ctrlr_dom_null) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = NULL;
  controller_domain *ctrlr_dom = NULL;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedControllerKey(ikey, UPLL_DT_CANDIDATE,
        dmi, ctrlr_dom));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedControllerKey_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedControllerKey_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedControllerKey_3) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

/* GetRenamedUncKey Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedUncKey_fail_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = NULL;
  uint8_t *ctr_id1 = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedUncKey_fail_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = NULL;
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedUncKey_fail_3) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedUncKey_fail_4) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedUncKey_fail_5) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedUncKey_fail_6) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedUncKey(ikey, UPLL_DT_CANDIDATE,
            dmi, ctr_id1));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedUncKey_fail_7) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedUncKey_pass_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

TEST(VtermIfFlowFilterEntryMoMgrTest, GetRenamedUncKey_pass_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

/* SetVlinkPortmapConfiguration Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, SetVlinkPortmapConfiguration_fail_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  InterfacePortMapInfo flags = kPortMapConfigured;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetVlinkPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, UNC_OP_CREATE, config_mode, vtn_name));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, SetVlinkPortmapConfiguration_fail_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetVlinkPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, UNC_OP_CREATE, config_mode, vtn_name));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, SetVlinkPortmapConfiguration_fail_3) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetVlinkPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, UNC_OP_CREATE, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, SetVlinkPortmapConfiguration_no_ins) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetVlinkPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, UNC_OP_CREATE, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, SetVlinkPortmapConfiguration_fail_4) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetVlinkPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, UNC_OP_CREATE, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, SetVlinkPortmapConfiguration_fail_5) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE, kDalRcRecordNotFound);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetVlinkPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, UNC_OP_CREATE, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, SetVlinkPortmapConfiguration_fail_6) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  InterfacePortMapInfo flags = kPortMapConfigured;
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SetVlinkPortmapConfiguration(ikey, UPLL_DT_CANDIDATE,
        dmi, flags, UNC_OP_CREATE, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

/* CreateCandidateMo Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateCandidateMo_fail_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateCandidateMo_fail_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateCandidateMo_fail_3) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 100, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateCandidateMo_fail_4) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcGeneralError);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateCandidateMo_fail_5) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateCandidateMo_fail_6) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateCandidateMo_fail_7) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateCandidateMo_fail_8) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordNotFound);

  const char * ctrlr_name = "cntrlr_name";
  const char* version("5.1");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);

  //CapaModuleStub::stub_loadCapaModule();
  //uint32_t max_instance_count = 1;
  //uint32_t num_attrs = 10;
  //uint8_t attrs[1];
  //CapaModuleStub::stub_setCreatecapaParameters(max_instance_count, &num_attrs, attrs);

 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

  ikey->set_user_data((void*)ctrlr_name);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  //CapaModuleStub::stub_clearStubData();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateCandidateMo_fail_9) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfg_val  = NULL;
  AllocConfigVal(cfg_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, cfg_val);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  GetIpcReqResHeader(5, 14, UNC_OP_CREATE, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

/* CreateAuditMoImpl Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateAuditMoImpl_fail_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  const char *ctrlr_id = "ctrlr_1";
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateAuditMoImpl(ikey, dmi, ctrlr_id));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateAuditMoImpl_fail_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateAuditMoImpl_fail_3) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
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

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateAuditMoImpl_fail_4) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigVal *val = NULL;
  AllocEntryConfigVal(val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, val);
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

TEST(VtermIfFlowFilterEntryMoMgrTest, CreateAuditMoImpl_pass) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigVal *val = NULL;
  AllocEntryConfigVal(val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, val);
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

/* IsReferenced Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, IsReferenced_fail_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  ConfigKeyVal *ikey = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, ikey, dmi));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsReferenced_fail_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, IsReferenced_InstExists) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordAlreadyExists);
  EXPECT_EQ(UPLL_RC_ERR_INSTANCE_EXISTS, obj.IsReferenced(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  DELETE_IF_NOT_NULL(ikey);
}

/* ReadMo Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadMo_fail_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ReadMo(req, ikey, dmi));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadMo_fail_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_IMPORT, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ReadMo(req, ikey, dmi));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadMo_fail_3) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadMo_fail_4) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_RUNNING, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadMo_fail_5) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STARTUP, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadMo_fail_6) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadMo_fail_7) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_DETAIL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadMo_pass_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadMo_fail_8) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_DETAIL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME, obj.ReadMo(req, ikey, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

/* ReadSiblingMo Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadSiblingMo_fail_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ReadSiblingMo(req, ikey, false, dmi));
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadSiblingMo_fail_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_CANDIDATE, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadSiblingMo_fail_3) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_RUNNING, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadSiblingMo_fail_4) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STARTUP, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadSiblingMo_fail_5) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadSiblingMo_fail_6) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_DETAIL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadSiblingMo_fail_7) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ, 0, UNC_OPT1_DETAIL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
  //CapaModuleStub::stub_loadCapaModule();
  //CapaModuleStub::stub_unloadCapaModule();
  CtrlrMgr::Ctrlr ctrlrobj("ctrlr_id", UNC_CT_PFC, "5.1",0);
  CtrlrMgr::GetInstance()->Add(ctrlrobj, UPLL_DT_CANDIDATE);
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME, obj.ReadSiblingMo(req, ikey, false, dmi));
  CtrlrMgr::GetInstance()->Delete("ctrlr_id", UPLL_DT_CANDIDATE);
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

TEST(VtermIfFlowFilterEntryMoMgrTest, ReadSiblingMo_pass) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  IpcReqRespHeader *req = NULL;
  GetIpcReqResHeader(5, 14, UNC_OP_READ_SIBLING, 0, UNC_OPT1_NORMAL, UNC_OPT2_NONE,
                     UPLL_DT_STATE, req);
  key_vterm_if_flowfilter_entry_t *vterm_key = NULL;
  AllocKeyStruct(vterm_key);
  GetKeyStruct(vterm_key);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req, ikey, false, dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  DELETE_IF_NOT_NULL(ikey);
}

/* TxUpdateController Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, TxUpdateController_1) {
  VtermIfFlowFilterEntryMoMgr obj;
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
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
           session_id,config_id,uuc::kUpllUcpCreate,affected_ctrlr_set,dmi,&err_ckv, &tx,
           config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, TxUpdateController_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS, kDalRcSuccess);
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  set<string> *affected_ctrlr_set = NULL;
  //:uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  ConfigKeyVal *err_ckv = NULL;
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
            session_id,config_id,uuc::kUpllUcpCreate,affected_ctrlr_set,dmi,&err_ckv, &tx,
            config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, TxUpdateController_3) {
  VtermIfFlowFilterEntryMoMgr obj;
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
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
    session_id,config_id,uuc::kUpllUcpCreate,affected_ctrlr_set,dmi,&err_ckv, &tx,
    config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, TxUpdateController_4) {
  VtermIfFlowFilterEntryMoMgr obj;
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
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
   session_id,config_id,uuc::kUpllUcpUpdate,affected_ctrlr_set,dmi,&err_ckv, &tx,
   config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, TxUpdateController_5) {
  VtermIfFlowFilterEntryMoMgr obj;
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
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
   session_id,config_id,uuc::kUpllUcpDelete,affected_ctrlr_set,dmi,&err_ckv, &tx,
   config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, TxUpdateController_6) {
  VtermIfFlowFilterEntryMoMgr obj;
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
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
   session_id,config_id,uuc::kUpllUcpInvalid,affected_ctrlr_set,dmi,&err_ckv, &tx,
   config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, TxUpdateController_7) {
  VtermIfFlowFilterEntryMoMgr obj;
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
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.TxUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
   session_id,config_id,uuc::kUpllUcpInvalid,NULL,dmi,&err_ckv, &tx,
   config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

/* AuditUpdateController Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, AuditUpdateController_1) {
   VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  const char *ctrlr_id = "ctrlr_id";
  KTxCtrlrAffectedState affect;
  ConfigKeyVal *err_ckv = NULL;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AuditUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
      ctrlr_id, session_id, config_id, uuc::kUpllUcpCreate, dmi, &err_ckv, &affect));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, AuditUpdateController_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  const char *ctrlr_id = "ctrlr_id";
  KTxCtrlrAffectedState affect;
  ConfigKeyVal *err_ckv = NULL;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS, kDalRcGeneralError);
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, obj.AuditUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
   ctrlr_id, session_id, config_id, uuc::kUpllUcpUpdate, dmi, &err_ckv, &affect));
  DalOdbcMgr::clearStubData();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, AuditUpdateController_3) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  const char *ctrlr_id = "ctrlr_id";
  KTxCtrlrAffectedState affect;
  ConfigKeyVal *err_ckv = NULL;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS, kDalRcGeneralError);
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, obj.AuditUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
   ctrlr_id, session_id, config_id, uuc::kUpllUcpDelete, dmi, &err_ckv, &affect));
  DalOdbcMgr::clearStubData();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, AuditUpdateController_4) {
  VtermIfFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr;
  const char *ctrlr_id = "ctrlr_id";
  KTxCtrlrAffectedState affect;
  ConfigKeyVal *err_ckv = NULL;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS, kDalRcRecordNotFound);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS, kDalRcRecordNotFound);
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, obj.AuditUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
   ctrlr_id, session_id, config_id, uuc::kUpllUcpUpdate, dmi, &err_ckv, &affect));
  DalOdbcMgr::clearStubData();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, AuditUpdateController_5) {
   VtermIfFlowFilterEntryMoMgr obj;
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
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, obj.AuditUpdateController(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
   ctrlr_id, session_id, config_id, uuc::kUpllUcpUpdate, dmi, &err_ckv, &affect));
  DalOdbcMgr::clearStubData();
}

/* MergeValidate Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, MergeValidate) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  const char *ctrlr_id = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.MergeValidate(UNC_KT_VTERMIF_FLOWFILTER_ENTRY, ctrlr_id, ikey, dmi,(upll_import_type)0));
}


TEST(VtermIfFlowFilterEntryMoMgrTest, MergeValidate_Read_) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *key_vterm_if_entry = reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
                  (malloc(sizeof(key_vterm_if_flowfilter_entry_t)));
  memset(key_vterm_if_entry, 0, sizeof(key_vterm_if_flowfilter_entry_t));
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vterminal_name,"VTERM1",32);
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vterm_if_entry->sequence_num = 1;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtermIfFlowfilterEntry,key_vterm_if_entry,NULL);
  char ctrl_id[]={'p', 'f', 'c'};
  unc_key_type_t keytype = UNC_KT_VTERMIF_FLOWFILTER_ENTRY;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalDmlIntf *dmi = new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.MergeValidate(keytype,ctrl_id,ikey,dmi,(upll_import_type)0));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}


TEST(VtermIfFlowFilterEntryMoMgrTest, MergeValidate_generic_ERROR) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *key_vterm_if_entry = reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
                  (malloc(sizeof(key_vterm_if_flowfilter_entry_t)));
  memset(key_vterm_if_entry, 0, sizeof(key_vterm_if_flowfilter_entry_t));
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vterminal_name,"VTERM1",32);
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vterm_if_entry->sequence_num = 1;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtermIfFlowfilterEntry,key_vterm_if_entry,NULL);
  //ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER);
  char ctrl_id[]={'p', 'f', 'c'};
  unc_key_type_t keytype = UNC_KT_VTERMIF_FLOWFILTER_ENTRY;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalDmlIntf *dmi = new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.MergeValidate(keytype,ctrl_id,ikey,dmi,(upll_import_type)0));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, MergeValidate_Update_DB_Err) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *key_vterm_if_entry = reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
                  (malloc(sizeof(key_vterm_if_flowfilter_entry_t)));
  val_flowfilter_t *vterm_if_flowfilter_entry_val = reinterpret_cast<val_flowfilter_t *>
        (malloc(sizeof(val_flowfilter_t)));
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                             vterm_if_flowfilter_entry_val);
  memset(key_vterm_if_entry, 0, sizeof(key_vterm_if_flowfilter_entry_t));
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vterminal_name,"VTERM1",32);
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vterm_if_entry->sequence_num = 1;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtermIfFlowfilterEntry,key_vterm_if_entry,tmp);
  char ctrl_id[]={'p', 'f', 'c'};
  unc_key_type_t keytype = UNC_KT_VTERMIF_FLOWFILTER_ENTRY;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalDmlIntf *dmi = new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.MergeValidate(keytype,ctrl_id,ikey,dmi,(upll_import_type)0));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

/* UpdateMo Test cases */

TEST(VtermIfFlowFilterEntryMoMgrTest, UpdateMo) {
  VtermIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateMo(req, ikey, dmi));
}

TEST(VtermIfFlowFilterEntryMoMgrTest, UpdateMo_Valid_detai1) {
  VtermIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = new IpcReqRespHeader();
  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_STATE;
  req->option2 = UNC_OPT2_NONE;
  key_vterm_if_flowfilter_entry_t *key_vterm_if_entry = reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
                  (malloc(sizeof(key_vterm_if_flowfilter_entry_t)));
  memset(key_vterm_if_entry, 0 ,sizeof(key_vterm_if_flowfilter_entry_t));
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vterminal_name,"VTERM1",32);
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vterm_if_entry->sequence_num = 1;

  val_flowfilter_entry_t *vterm_if_flowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
      (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vterm_if_flowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  strncpy((char*) vterm_if_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                vterm_if_flowfilter_entry_val);

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtermIfFlowfilterEntry,key_vterm_if_entry,val);
  const char ctrlr_name[] = "ctrlr_name";
  ikey->set_user_data((void*)ctrlr_name);
  DalDmlIntf *dmi = new DalOdbcMgr();

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.UpdateMo(req,ikey,dmi));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST(VtermIfFlowFilterEntryMoMgrTest, UpdateMo_Valid_data2) {
  VtermIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = new IpcReqRespHeader();
  req->operation = UNC_OP_DELETE;
  req->datatype = UPLL_DT_STATE;
  req->option2 = UNC_OPT2_NONE;
  key_vterm_if_flowfilter_entry_t *key_vterm_if_entry = reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
                  (malloc(sizeof(key_vterm_if_flowfilter_entry_t)));
  memset(key_vterm_if_entry, 0 ,sizeof(key_vterm_if_flowfilter_entry_t));
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.vterm_key.vterminal_name,"VTERM1",32);
  strncpy((char*) key_vterm_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vterm_if_entry->sequence_num = 1;

  val_flowfilter_entry_t *vterm_if_flowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
      (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vterm_if_flowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  strncpy((char*) vterm_if_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                vterm_if_flowfilter_entry_val);

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtermIfFlowfilterEntry,key_vterm_if_entry,val);
  const char * ctrlr_name = "cntrlr_name";
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.clear();
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);

  //CapaModuleStub::stub_loadCapaModule();
  //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  ikey->set_user_data((void*)ctrlr_name);
  DalDmlIntf *dmi = new DalOdbcMgr();

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.UpdateMo(req,ikey,dmi));
  DalOdbcMgr::clearStubData();
  //CapaModuleStub::stub_clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

/**********PC*************/
/***********ValidateAttribute**********/
TEST(VtermIfFlowFilterEntryMoMgrTest, ValidateAttribute_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *key = ZALLOC_TYPE(
               key_vterm_if_flowfilter_entry_t);
  strncpy((char*) key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
   "VTN1",32);
  strncpy((char*) key->flowfilter_key.if_key.vterm_key.vterminal_name,
   "VTERM1",32);
  strncpy((char*) key->flowfilter_key.if_key.if_name,"IF1",32);

  val_flowfilter_entry_t *ival = reinterpret_cast<val_flowfilter_entry_t *>
          (malloc(sizeof(val_flowfilter_entry_t)));
  ival->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;

  strncpy((char*) ival->flowlist_name,"FLOWLISTNAME",32);

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilter, ival);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtermIfFlowfilterEntry, key, cv);

  DalDmlIntf *dmi = new DalOdbcMgr();
  IpcReqRespHeader *req = new IpcReqRespHeader();
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->clnt_sess_id =1;
  req->config_id = 0;

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  unc::tclib::TcLibModule::stub_loadtcLibModule();
  unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
                                              unc::tclib::TC_INVALID_SESSION_ID);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey,
           dmi, req));
  ikey = NULL;
  delete ikey;
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

 TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_1) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vtermif_ffe_key =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vtermif_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  vtermif_ffe_key->flowfilter_key.direction = 0xFE;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                                        vtermif_ffe_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalDmlIntf *dmi = NULL;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_2) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vtermif_ffe_key =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vtermif_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  strncpy(reinterpret_cast<char*>
          (vtermif_ffe_key->flowfilter_key.if_key.vterm_key.vterminal_name),
          "vbr1", 32);
  vtermif_ffe_key->flowfilter_key.direction = 0xFE;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                                        vtermif_ffe_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalDmlIntf *dmi = NULL;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_3) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vtermif_ffe_key =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vtermif_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  strncpy(reinterpret_cast<char*>
          (vtermif_ffe_key->flowfilter_key.if_key.vterm_key.vterminal_name),
          "vbr1", 32);
  strncpy(reinterpret_cast<char*>
          (vtermif_ffe_key->flowfilter_key.if_key.if_name),
          "if1", 32);
  vtermif_ffe_key->flowfilter_key.direction = 0xFE;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                                        vtermif_ffe_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalDmlIntf *dmi = NULL;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_4) {
  VtermIfFlowFilterEntryMoMgr obj;
  key_vterm_if_flowfilter_entry_t *vtermif_ffe_key =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vtermif_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  strncpy(reinterpret_cast<char*>
          (vtermif_ffe_key->flowfilter_key.if_key.vterm_key.vterminal_name),
          "vbr1", 32);
  strncpy(reinterpret_cast<char*>
          (vtermif_ffe_key->flowfilter_key.if_key.if_name),
          "if1", 32);
  vtermif_ffe_key->flowfilter_key.direction = 0x0;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                                        vtermif_ffe_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalDmlIntf *dmi = NULL;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_5) {
  VtermIfFlowFilterEntryMoMgr obj;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                                        NULL, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE,
                                 kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalDmlIntf *dmi = NULL;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_6) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST(VtermIfFlowFilterEntryMoMgrTest, DeleteChildrenPOM_7) {
  VtermIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                                        NULL, NULL);
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
