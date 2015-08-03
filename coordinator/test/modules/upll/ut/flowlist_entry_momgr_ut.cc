/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <gtest/gtest.h>
#include <flowlist_entry_momgr.hh>
#include <upll_validation.hh>
#include <unc/upll_ipc_enum.h>
#include <momgr_impl.hh>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <ctrlr_mgr.hh>
//#include <capa_module/capa_intf.hh>
//#include <capa_module/capa_module_stub.hh>
#include <config_mgr.hh>
//#include <tclib_module/tclib_module.hh>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ut_util.hh"
#include "tx_update_util.hh"
//bool fatal_done;
//pfc::core::Mutex  fatal_mutex_lock;

using namespace unc::tclib;
using namespace std;
using namespace unc::upll::dal;
using namespace unc::capa;
using namespace unc::upll::config_momgr;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::ipc_util;
using namespace unc::upll::test;

int i = 0;
#define NUM_FL_KEY_MAIN_COL 3
#define NUM_FL_KEY_CTRLR_COL 3
#define RENAME_FLOWLIST 0x01

class FlowListEntryUt :
    public UpllTestEnv
{
};

/*******************AllocVal ************************/
TEST_F(FlowListEntryUt,AllocVal_ConfigValNOtNull) {
  FlowListEntryMoMgr obj;
  ConfigVal* val = new ConfigVal;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(val, UPLL_DT_IMPORT, MAINTBL));
}

TEST_F(FlowListEntryUt,AllocVal_MAINTBL_Success) {
  FlowListEntryMoMgr obj;
  ConfigVal* val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_IMPORT,MAINTBL)); 
}

TEST_F(FlowListEntryUt,AllocVal_RENAMETBL_Success) {
  FlowListEntryMoMgr obj;
  ConfigVal* val = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(val, UPLL_DT_STATE,RENAMETBL));   
}

TEST_F(FlowListEntryUt,AllocVal_CTRLRTBL_Success) {
  FlowListEntryMoMgr obj;
  ConfigVal* val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_STATE, CTRLRTBL));
}
/*================GetChildConfigKey====================================*/
TEST_F(FlowListEntryUt,GetChildConfigKey_parentkey_NULL) {
  FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey)); 
}

TEST_F(FlowListEntryUt,GetChildConfigKey_okey_NULL_pkey_keystructNULL) {
  FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_FLOWLIST);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetChildConfigKey(okey, pkey));
}

TEST_F(FlowListEntryUt,GetChildConfigKey_okey_NULL_pkey_validStruct) {
  FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_flowlist_entry_t *flowlist_key = ZALLOC_TYPE(key_flowlist_entry_t);
  memset(&(flowlist_key->flowlist_key.flowlist_name), 0, kMaxLenFlowListName);
  memcpy(&(flowlist_key->flowlist_key.flowlist_name), "FLOWLIST", kMaxLenFlowListName);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, flowlist_key);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey));

  key_flowlist_entry_t *output = reinterpret_cast<key_flowlist_entry_t *> (okey->get_key());

  EXPECT_STREQ("FLOWLIST",(reinterpret_cast<const char *> (output->flowlist_key.flowlist_name)));
}

/*==========================DupConfigKeyVal============================*/
TEST_F(FlowListEntryUt, DupConfigKeyVal_reqnull) {
  FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req,MAINTBL));
}

TEST_F(FlowListEntryUt, DupConfigKeyVal_okeyNULL) {
  FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req,MAINTBL));
}

TEST_F(FlowListEntryUt, DupConfigKeyVal_reqInvalidKT) {
 FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req,MAINTBL));

}

TEST_F(FlowListEntryUt, DupConfigKeyVal_MAINTBL_ValStruct_NotNULL_butEmpty) {
  FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  //key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
  val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                                        val_flowlist_entry);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                      IpctSt::kIpcStKeyFlowlistEntry, NULL, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req, MAINTBL));

}

TEST_F(FlowListEntryUt, DupConfigKeyVal_MAINTBL_Success) {
  FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
  val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
 val_flowlist_entry->ip_dscp = 3;
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, req,MAINTBL));
}


TEST_F(FlowListEntryUt, DupConfigKeyVal_CTRLRTBL_Success) {
  FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);

   val_flowlist_entry_ctrl *flowlist_entry_ctrlr_val =
            ZALLOC_TYPE(val_flowlist_entry_ctrl);
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, flowlist_entry_ctrlr_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, req, CTRLRTBL));

}

/*======================CopyToConfigKey=========================*/
TEST_F(FlowListEntryUt, CopyToConfigKey_ikeyNULL) {
  FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey, NULL));
}

TEST_F(FlowListEntryUt, CopyToConfigKey_KTNULL) {
  FlowListEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey, ikey));

}
/** failed test case need to check the code */
/*Invalid TC, as InvalidKT check doesnot exist in code */
TEST_F(FlowListEntryUt, CopyToConfigKey_InvalidKT) {
  FlowListEntryMoMgr obj;
  key_flowlist_entry_t *key_flowlist_entry = new key_flowlist_entry_t;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry);
  ConfigKeyVal *okey = NULL; 
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.CopyToConfigKey(okey, ikey));

}

TEST_F(FlowListEntryUt, CopyToConfigKey_EmptyKeyStruct_validKT) {
  FlowListEntryMoMgr obj;
  key_rename_vnode_info_t *key_rename =
      ZALLOC_TYPE(key_rename_vnode_info_t);
  memcpy(&(key_rename->old_flowlist_name), "FLOWLIST", kMaxLenFlowListName);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, key_rename);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));

}
/*============================UpdateAuditConfigStatus=========================================*/
TEST_F(FlowListEntryUt, UpdateAuditConfigStatus_ckv_runningNULL) {
FlowListEntryMoMgr obj;
ConfigKeyVal *ikey =NULL;
DalDmlIntf *dmi(getDalDmlIntf());
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey, dmi));
 val_flowlist_entry_t *val = ZALLOC_TYPE(val_flowlist_entry_t);
  memset(val, 0, sizeof(val_flowlist_entry_t));

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val);

  ikey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey, dmi));
  val_flowlist_entry_t *output = reinterpret_cast<val_flowlist_entry_t *> (GetVal(ikey));
  EXPECT_EQ(UPLL_RC_SUCCESS,output->cs_row_status);
}
/*===============================SetConsolidatedStatus============================*/
TEST_F(FlowListEntryUt, SetConsolidatedStatus_ikeyNotNull) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *l_cval = NULL;
  val_flowlist_entry_t* val_flowlist = 
                       new val_flowlist_entry_t();
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_flowlist);
  key_flowlist_entry_t *key_flowlist = 
    new key_flowlist_entry_t();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess); 
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            key_flowlist,l_cval);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 SetConsolidatedStatus
                    (l_ikey,dmi));
delete l_ikey;
}

/*============================GLetRenameKeyBindInfo=====================*/
TEST_F(FlowListEntryUt, GetRenameKeyBindInfo_InvalidKT) {
 FlowListEntryMoMgr flowlist_obj;
 BindInfo *info = NULL;
  int nattrs;
EXPECT_EQ(PFC_TRUE,  flowlist_obj.GetRenameKeyBindInfo(UNC_KT_FLOWLIST_ENTRY, info, nattrs, MAINTBL));
}

TEST_F(FlowListEntryUt, GetRenameKeyBindInfo_SuccessMAINTBL) {
 FlowListEntryMoMgr flowlist_obj;
 BindInfo *info = NULL;
  int nattrs;
EXPECT_EQ(PFC_TRUE,  flowlist_obj.GetRenameKeyBindInfo(UNC_KT_FLOWLIST_ENTRY, info, nattrs, MAINTBL));
EXPECT_EQ(NUM_FL_KEY_MAIN_COL, nattrs);
}

TEST_F(FlowListEntryUt, GetRenameKeyBindInfo_SuccessCTRLRTBL) {
 FlowListEntryMoMgr flowlist_obj;
 BindInfo *info = NULL;
  int nattrs;
EXPECT_EQ(PFC_TRUE,  flowlist_obj.GetRenameKeyBindInfo(UNC_KT_FLOWLIST_ENTRY, info, nattrs, CTRLRTBL));
EXPECT_EQ(NUM_FL_KEY_CTRLR_COL, nattrs);
}

TEST_F(FlowListEntryUt, GetRenameKeyBindInfo_RENAMETBLdefaultCase) {
 FlowListEntryMoMgr flowlist_obj;
 BindInfo *info = NULL;
  int nattrs;
EXPECT_EQ(PFC_TRUE,  flowlist_obj.GetRenameKeyBindInfo(UNC_KT_FLOWLIST_ENTRY, info, nattrs, RENAMETBL));
}

/*================================GetValid========================================================*/
TEST_F(FlowListEntryUt, GetValid_valNULL) {
FlowListEntryMoMgr flowlist_obj;
uint8_t *valid = NULL;
EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.GetValid((void*)valid, (uint64_t )1, valid, UPLL_DT_CANDIDATE, MAINTBL));
}

TEST_F(FlowListEntryUt, GetValid_MAINTBLSuccess) {
FlowListEntryMoMgr flowlist_obj;
uint8_t *valid = NULL;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.GetValid((void*)val_flowlist_entry, (uint64_t )2, valid, UPLL_DT_CANDIDATE, MAINTBL));
}

TEST_F(FlowListEntryUt, GetValid_CTRLRTBLSuccess) {
FlowListEntryMoMgr flowlist_obj;
uint8_t *valid = NULL;
val_flowlist_entry_ctrl *flowlist_ctrl = new val_flowlist_entry_ctrl;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.GetValid((void*)flowlist_ctrl, (uint64_t )2, valid, UPLL_DT_CANDIDATE, CTRLRTBL));
}

TEST_F(FlowListEntryUt, GetValid_InvalidTblNameRENAMETBL) {
FlowListEntryMoMgr flowlist_obj;
uint8_t *valid = NULL;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.GetValid((void*)val_flowlist_entry, (uint64_t )1, valid, UPLL_DT_CANDIDATE, RENAMETBL));
}


/*------------------------------**CompareKey****--------------*/
TEST_F(FlowListEntryUt, CompareKey_key1key2NULL) {
FlowListEntryMoMgr flowlist_obj;
EXPECT_EQ(true, flowlist_obj.CompareKey(NULL,NULL));
}

TEST_F(FlowListEntryUt, CompareKey_key1validkey2empty) {
FlowListEntryMoMgr flowlist_obj;
key_flowlist_entry_t *flowlist_key1 = new key_flowlist_entry_t;
ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry,
   flowlist_key1);
ConfigKeyVal *key1 = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
EXPECT_EQ(false, flowlist_obj.CompareKey(key, key1));
}

TEST_F(FlowListEntryUt, CompareKey_keyMisMatch) {
FlowListEntryMoMgr flowlist_obj;
key_flowlist_entry_t *flowlist_key1 = new key_flowlist_entry_t;
strncpy(reinterpret_cast<char *>(flowlist_key1->flowlist_key.flowlist_name),
  "FLOWLIST",kMaxLenFlowListName);
ConfigKeyVal *key1 = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry,
   flowlist_key1);
key_flowlist_entry_t *flowlist_key2 = new key_flowlist_entry_t;
strncpy(reinterpret_cast<char *>(flowlist_key2->flowlist_key.flowlist_name),
  "FLOWLIST1",kMaxLenFlowListName);
ConfigKeyVal *key2 = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry,
   flowlist_key2);
EXPECT_EQ(false, flowlist_obj.CompareKey(key1, key2));
}

TEST_F(FlowListEntryUt, CompareKey_keyMisMatch1) {
FlowListEntryMoMgr flowlist_obj;
key_flowlist_entry_t *flowlist_key1 = new key_flowlist_entry_t;
strncpy(reinterpret_cast<char *>(flowlist_key1->flowlist_key.flowlist_name),
  "FLOWLIST",kMaxLenFlowListName);
flowlist_key1->sequence_num = 20;
ConfigKeyVal *key1 = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry,
   flowlist_key1);
key_flowlist_entry_t *flowlist_key2 = new key_flowlist_entry_t;
strncpy(reinterpret_cast<char *>(flowlist_key2->flowlist_key.flowlist_name),
  "FLOWLIST",kMaxLenFlowListName);
flowlist_key2->sequence_num = 10;
ConfigKeyVal *key2 = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry,
   flowlist_key2);
EXPECT_EQ(false, flowlist_obj.CompareKey(key1, key2));
}

TEST_F(FlowListEntryUt, CompareKey_keyMatch) {
FlowListEntryMoMgr flowlist_obj;
key_flowlist_entry_t *flowlist_key1 = new key_flowlist_entry_t;
strncpy(reinterpret_cast<char *>(flowlist_key1->flowlist_key.flowlist_name),
  "FLOWLIST",kMaxLenFlowListName);
flowlist_key1->sequence_num = 20;
ConfigKeyVal *key1 = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry,
   flowlist_key1);
key_flowlist_entry_t *flowlist_key2 = new key_flowlist_entry_t;
strncpy(reinterpret_cast<char *>(flowlist_key2->flowlist_key.flowlist_name),
  "FLOWLIST",kMaxLenFlowListName);
flowlist_key2->sequence_num = 20;
ConfigKeyVal *key2 = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry,
   flowlist_key2);
EXPECT_EQ(true, flowlist_obj.CompareKey(key1, key2));
}

/*** ValidateMessage UT starts ***************************/
TEST_F(FlowListEntryUt,ValidateMsg1_Validreq_NULLkey) {
FlowListEntryMoMgr flowlist_obj;
/** set Invalid keytype */
ConfigKeyVal *ikey = NULL;
IPC_REQ_RESP_HEADER_DECL(req);

EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg2_InvalidKeytype) {
FlowListEntryMoMgr flowlist_obj;
/** set Invalid keytype */
ConfigKeyVal ikey(UNC_KT_FLOWLIST);
IPC_REQ_RESP_HEADER_DECL(req);

EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, &ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg3_NoKeyStNumFilled) {
FlowListEntryMoMgr flowlist_obj;
/** set valid keytype */
ConfigKeyVal ikey(UNC_KT_FLOWLIST_ENTRY);

/** default value for st_num is kIpcInvalidStNum*/
IPC_REQ_RESP_HEADER_DECL(req);
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, &ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg3_InvalidKeyStNum) {
FlowListEntryMoMgr flowlist_obj;
/** set valid keytype */
ConfigKeyVal ikey(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlist);

/** default value for st_num is kIpcInvalidStNum*/
IPC_REQ_RESP_HEADER_DECL(req);
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, &ikey));
}


TEST_F(FlowListEntryUt,ValidateMsg4_NullKeyStruct) {
FlowListEntryMoMgr flowlist_obj;
/** set valid keytype, st_num */
ConfigKeyVal ikey(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry);
IPC_REQ_RESP_HEADER_DECL(req);
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

/** key struct is NULL */
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, &ikey));
}

/** valid value structu, invalid st_num */
TEST_F(FlowListEntryUt,ValidateMsg5_InvalidValStNum) {
FlowListEntryMoMgr flowlist_obj;
/** set valid keytype */
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num =10;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
memset(val_flowlist_entry, 0, sizeof(val_flowlist_entry_t));
ConfigVal *config_val= new ConfigVal(IpctSt::kIpcInvalidStNum, val_flowlist_entry);
ConfigKeyVal ikey(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);

/** default value for st_num is kIpcInvalidStNum*/
IPC_REQ_RESP_HEADER_DECL(req);
req->operation = UNC_OP_CREATE;
req->datatype = UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, &ikey));
}

/**TODO have to recheck this output */
TEST_F(FlowListEntryUt,ValidateMsg6_NoValueInReqKey) {
FlowListEntryMoMgr flowlist_obj;

key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
IPC_REQ_RESP_HEADER_DECL(req);

/** set valid keytype, st_num, key-struct */
ConfigKeyVal ikey(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry);
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateMessage(req, &ikey));
}
TEST_F(FlowListEntryUt,ValidateMsg7_ValidKeyStruct_ValStructNull) {
FlowListEntryMoMgr flowlist_obj;
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

/** set valid keytype, st_num, key-struct */
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);

/** Rename value struct and its st_num is not filled*/
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg7_ValidKeyStruct_seqNumInvalid) {
FlowListEntryMoMgr flowlist_obj;
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 0;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

/** set valid keytype, st_num, key-struct */
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);

/** Rename value struct and its st_num is not filled*/
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg7_ValidKeyStruct_flowlistnameInvalid) {
FlowListEntryMoMgr flowlist_obj;
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "~FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 0;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

/** set valid keytype, st_num, key-struct */
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);

/** Rename value struct and its st_num is not filled*/
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateMessage(req, ikey));
}


TEST_F(FlowListEntryUt,ValidateMsg7_ValidKeyStruct_Invalidkeyname) {
FlowListEntryMoMgr flowlist_obj;
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "_$FLOWLIST" , kMaxLenFlowListName);

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

/** set valid keytype, st_num, key-struct */
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, NULL);

/** Rename value struct and its st_num is not filled*/
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateMessage(req, ikey));
}
TEST_F(FlowListEntryUt,ValidateMsg11_CREATEInvalidDT) {
FlowListEntryMoMgr flowlist_obj;

key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry);
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, ikey));
}
TEST_F(FlowListEntryUt,ValidateMsg13_CREATESuccess) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);

val_flowlist_entry->ip_dscp = 10;
val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE]
      = UNC_VF_VALID;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}
TEST_F(FlowListEntryUt,ValidateMsg13_ResetCheck) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_UPDATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}
TEST_F(FlowListEntryUt,ValidateMsg13_ResetCheck_CREATE) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg13_InvalidOP_NoReset) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_RUNNING;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg_ValFieldSuccess) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST11" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_UPDATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] =UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] =UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;

memset(val_flowlist_entry->mac_dst, 10, sizeof(val_flowlist_entry->mac_dst));
memset(val_flowlist_entry->mac_src,11, sizeof(val_flowlist_entry->mac_dst));

val_flowlist_entry->mac_eth_type = 10;
//val_flowlist_entry->dst_ip_prefixlen =10;
//val_flowlist_entry->src_ip_prefixlen = 10;
val_flowlist_entry->vlan_priority = 7;
//val_flowlist_entry->dst_ipv6_prefixlen = 10;
//val_flowlist_entry->src_ipv6_prefixlen =10;
val_flowlist_entry->ip_dscp = 10;
val_flowlist_entry->ip_proto = 5;
val_flowlist_entry->l4_dst_port = 10;
val_flowlist_entry->l4_src_port = 10;
val_flowlist_entry->l4_src_port_endpt = 11;
val_flowlist_entry->l4_dst_port_endpt = 12;
//val_flowlist_entry->icmp_type = 10;
//val_flowlist_entry->icmp_code = 20;

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
DalBindInfo::clearStubData();
}
TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortError) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST11" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_UPDATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateL4Port(val_flowlist_entry, NULL, UNC_OP_UPDATE,true));
}
TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortError1) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateL4Port(val_flowlist_entry, NULL, UNC_OP_UPDATE,true));
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortError2) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateL4Port(val_flowlist_entry, NULL, UNC_OP_UPDATE,true));
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortError4) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateL4Port(val_flowlist_entry, NULL, UNC_OP_UPDATE,true));
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortError5) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
db_val_fle->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateL4Port(val_flowlist_entry, db_val_fle, UNC_OP_UPDATE,true));
free(val_flowlist_entry);free(db_val_fle);
}

TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortError6) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
db_val_fle->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateL4Port(val_flowlist_entry, db_val_fle, UNC_OP_UPDATE,true));
free(val_flowlist_entry);free(db_val_fle);
}

TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortenptInvalid) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->l4_src_port = 10;
val_flowlist_entry->l4_src_port_endpt = 10;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateL4Port(val_flowlist_entry, db_val_fle, UNC_OP_CREATE,true));
free(val_flowlist_entry);free(db_val_fle);
}

TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortSuccess) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
db_val_fle->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE] = UNC_VF_VALID;

val_flowlist_entry->l4_src_port = 10;
val_flowlist_entry->l4_src_port_endpt = 11;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateL4Port(val_flowlist_entry, db_val_fle, UNC_OP_UPDATE,true));
free(val_flowlist_entry);free(db_val_fle);
}

TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortSuccess12) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
db_val_fle->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_INVALID;

val_flowlist_entry->l4_src_port = 10;
val_flowlist_entry->l4_src_port_endpt = 11;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateL4Port(val_flowlist_entry, db_val_fle, UNC_OP_CREATE,true));
free(val_flowlist_entry);free(db_val_fle);
}
TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortReset) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] = UNC_VF_VALID_NO_VALUE;
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE] = UNC_VF_VALID_NO_VALUE;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_INVALID;

val_flowlist_entry->l4_src_port = 10;
val_flowlist_entry->l4_src_port_endpt = 11;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateL4Port(val_flowlist_entry, db_val_fle, UNC_OP_CREATE,true));
free(val_flowlist_entry);free(db_val_fle);
}
TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortReset1) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] = UNC_VF_VALID_NO_VALUE;
val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_INVALID;

val_flowlist_entry->l4_src_port = 10;
val_flowlist_entry->l4_src_port_endpt = 11;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateL4Port(val_flowlist_entry, db_val_fle, UNC_OP_CREATE,true));
free(val_flowlist_entry);free(db_val_fle);
}


TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortSuccess1) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
db_val_fle->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_INVALID;

val_flowlist_entry->l4_dst_port = 10;
val_flowlist_entry->l4_dst_port_endpt = 11;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateL4Port(val_flowlist_entry, db_val_fle, UNC_OP_CREATE,false));
free(val_flowlist_entry);free(db_val_fle);
}

TEST_F(FlowListEntryUt,ValidateIcmp_Error) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] =UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateIcmp(val_flowlist_entry, NULL, UPLL_FLOWLIST_TYPE_IP, UNC_OP_CREATE));
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,ValidateIcmp_Error1) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_VALID_NO_VALUE;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateIcmp(val_flowlist_entry, NULL, UPLL_FLOWLIST_TYPE_IP, UNC_OP_CREATE));
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,ValidateIcmp_Error2) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_VALID_NO_VALUE;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateIcmp(val_flowlist_entry, NULL, UPLL_FLOWLIST_TYPE_IPV6, UNC_OP_CREATE));
free(val_flowlist_entry);
}
#if 0
this check is done in ValidateL4Port 
TEST_F(FlowListEntryUt,ValidateIcmp_Error3) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_VALID;
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
db_val_fle->valid[UPLL_IDX_L4_DST_PORT_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateIcmp(val_flowlist_entry, db_val_fle, UPLL_FLOWLIST_TYPE_IP, UNC_OP_UPDATE));
free(val_flowlist_entry); free(db_val_fle);
}
#endif
TEST_F(FlowListEntryUt,ValidateIcmp_Success) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_VALID;
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
db_val_fle->valid[UPLL_IDX_L4_DST_PORT_FLE] = UNC_VF_INVALID;
db_val_fle->valid[UPLL_IDX_L4_SRC_PORT_FLE] = UNC_VF_INVALID;
db_val_fle->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]  = UNC_VF_INVALID;
db_val_fle->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] = UNC_VF_INVALID;
val_flowlist_entry->icmp_type = 20;
val_flowlist_entry->icmp_code = 25;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateIcmp(val_flowlist_entry, db_val_fle, UPLL_FLOWLIST_TYPE_IP, UNC_OP_UPDATE));
free(val_flowlist_entry); free(db_val_fle);
}


TEST_F(FlowListEntryUt,ValidateIPAddress_err1) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IP, UNC_OP_UPDATE, false));
}

TEST_F(FlowListEntryUt,ValidateIPAddress_err2) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IP, UNC_OP_UPDATE, false));
}

TEST_F(FlowListEntryUt,ValidateIPAddress_err3) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_FLE] = UNC_VF_VALID_NO_VALUE;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IP, UNC_OP_UPDATE, false));
}

TEST_F(FlowListEntryUt,ValidateIPAddress_err4) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_FLE] = UNC_VF_VALID_NO_VALUE;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IP, UNC_OP_UPDATE, false));
}

TEST_F(FlowListEntryUt,ValidateIPAddress_err11) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IPV6, UNC_OP_UPDATE, false));
}

TEST_F(FlowListEntryUt,ValidateIPAddress_err21) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_DST_IP_FLE] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IPV6, UNC_OP_UPDATE, false));
}

TEST_F(FlowListEntryUt,ValidateIPAddress_err31) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE] = UNC_VF_VALID_NO_VALUE;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IPV6, UNC_OP_UPDATE, false));
}

TEST_F(FlowListEntryUt,ValidateIPAddress_err41) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_DST_IP_FLE] = UNC_VF_VALID_NO_VALUE;
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IPV6, UNC_OP_UPDATE, false));
}

TEST_F(FlowListEntryUt,ValidateIPAddress_succ1) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_DST_IP_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_DST_IP_PREFIX_FLE] = UNC_VF_VALID;
val_flowlist_entry->dst_ip_prefixlen = 10;

EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IP, UNC_OP_UPDATE, false));
}


TEST_F(FlowListEntryUt,ValidateIPAddress_succ4) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] = UNC_VF_VALID;
val_flowlist_entry->src_ip_prefixlen = 10;

EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IP, UNC_OP_UPDATE, true));
}

TEST_F(FlowListEntryUt,ValidateIPAddress_succ5) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = new val_flowlist_entry_t;
val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE] = UNC_VF_VALID_NO_VALUE;
val_flowlist_entry->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] = UNC_VF_VALID_NO_VALUE;
val_flowlist_entry->src_ip_prefixlen = 10;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateIPAddress(val_flowlist_entry,
          UPLL_FLOWLIST_TYPE_IP, UNC_OP_UPDATE, true));
EXPECT_EQ(0, val_flowlist_entry->src_ip_prefixlen);
}


TEST_F(FlowListEntryUt,ValidateIcmp_Success1) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_VALID;
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
db_val_fle->valid[UPLL_IDX_L4_DST_PORT_FLE] = UNC_VF_INVALID;
db_val_fle->valid[UPLL_IDX_L4_SRC_PORT_FLE] = UNC_VF_INVALID;
db_val_fle->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]  = UNC_VF_INVALID;
db_val_fle->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] = UNC_VF_INVALID;
val_flowlist_entry->icmpv6_type = 20;
val_flowlist_entry->icmpv6_code = 25;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateIcmp(val_flowlist_entry, db_val_fle, UPLL_FLOWLIST_TYPE_IPV6, UNC_OP_UPDATE));
free(val_flowlist_entry); free(db_val_fle);
}

TEST_F(FlowListEntryUt,ValidateMsg_ValFieldPortSuccess11) {
FlowListEntryMoMgr flowlist_obj;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
val_flowlist_entry_t *db_val_fle = ZALLOC_TYPE(val_flowlist_entry_t);
db_val_fle->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;
db_val_fle->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_INVALID;
db_val_fle->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_INVALID;
db_val_fle->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_INVALID;
val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_FLE] = UNC_VF_VALID;
val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_INVALID;

val_flowlist_entry->l4_src_port = 10;
val_flowlist_entry->l4_src_port_endpt = 11;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateL4Port(val_flowlist_entry, db_val_fle, UNC_OP_UPDATE,true));
free(val_flowlist_entry);free(db_val_fle);
}

TEST_F(FlowListEntryUt,ValidateMsg_VlanInvalidRange) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST12" , kMaxLenFlowListName);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);

key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_UPDATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
val_flowlist_entry->vlan_priority = 8;

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateMessage(req, ikey));
DalBindInfo::clearStubData();
}

TEST_F(FlowListEntryUt,ValidateMsg13_Success) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);

val_flowlist_entry->ip_dscp = 10;
val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE]
      = UNC_VF_VALID;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg14_READNEXT) {
FlowListEntryMoMgr flowlist_obj;

key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ_SIBLING_COUNT;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg15_READ_RUNNING)  {
FlowListEntryMoMgr flowlist_obj;

key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_RUNNING;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg_InvalidOPTION2) {
FlowListEntryMoMgr flowlist_obj;

key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STARTUP;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_L2DOMAIN;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry);
EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg16_READNoValStruct)  {
FlowListEntryMoMgr flowlist_obj;
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg17_READSuccess)  {
FlowListEntryMoMgr flowlist_obj;

key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);

val_flowlist_entry->ip_dscp = 20;
val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE]
      = UNC_VF_VALID;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg18_READSIBLING_IMPORT)  {
FlowListEntryMoMgr flowlist_obj;

key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ_SIBLING;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateMsg19_DELETE) {
FlowListEntryMoMgr flowlist_obj;

key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
key_flowlist_entry->sequence_num = 10;
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_DELETE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
}

TEST_F(FlowListEntryUt,ValidateCapa1_reqNULL) {
FlowListEntryMoMgr flowlist_obj;

EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.ValidateCapability(NULL,NULL));
}

TEST_F(FlowListEntryUt,ValidateCapa2_keyNULL) {
FlowListEntryMoMgr flowlist_obj;

IPC_REQ_RESP_HEADER_DECL(req);

EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.ValidateCapability(req,NULL));
}

TEST_F(FlowListEntryUt,ValidateCapa3_reqkey_Empty) {
FlowListEntryMoMgr flowlist_obj;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.ValidateCapability(NULL, ikey, NULL));
}
TEST_F(FlowListEntryUt,ValidateCapa4_max_attrs_check) {
FlowListEntryMoMgr flowlist_obj;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 //CapaModuleStub::stub_loadCapaModule();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

 //CapaModuleStub::method_capa_map[CapaModuleStub::GET_CREATE_CAPABILITY] = true;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, NULL,
config_val);
EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
//CapaModuleStub::stub_clearStubData();
delete ikey;
}


TEST_F(FlowListEntryUt,ValidateCapa5_UPDATEValStructNULL) {
FlowListEntryMoMgr flowlist_obj;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_UPDATE;
req->datatype =  UPLL_DT_CANDIDATE;
const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 //CapaModuleStub::stub_loadCapaModule();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);
 //CapaModuleStub::method_capa_map[CapaModuleStub::GET_UPDATE_CAPABILITY] = true;


ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, NULL,
config_val);
EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
//CapaModuleStub::stub_clearStubData();
delete ikey;
}


TEST_F(FlowListEntryUt,ValidateCapa_READ_max_attrs_check) {
FlowListEntryMoMgr flowlist_obj;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
const char * ctrlr_name = "cntrlr_name";
 const char* version("version");

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 //CapaModuleStub::stub_loadCapaModule();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);


val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, NULL,
config_val);
EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
//CapaModuleStub::stub_clearStubData();
delete ikey;
}

TEST_F(FlowListEntryUt,ValidateCapa_READ_valstruct_NULL) {
FlowListEntryMoMgr flowlist_obj;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
const char * ctrlr_name = "cntrlr_name";
 const char* version("version");

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 //CapaModuleStub::stub_loadCapaModule();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, NULL,
config_val);
EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
//CapaModuleStub::stub_clearStubData();
delete ikey;
}

TEST_F(FlowListEntryUt,ValidateCapa_DELETE) {
FlowListEntryMoMgr flowlist_obj;

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_DELETE;
const char * ctrlr_name = "cntrlr_name";
 const char* version("version");

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 //CapaModuleStub::stub_loadCapaModule();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, NULL,
config_val);
EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
delete ikey;
//CapaModuleStub::stub_clearStubData();
}


TEST_F(FlowListEntryUt,_SupportCheckSuccess) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
free (val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_dstMacAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[0] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_MAC_DST_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_srcMacAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[1] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_MAC_SRC_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_ethtypeAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[2] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_MAC_ETH_TYPE_FLE]);
free(val_flowlist_entry);
}
TEST_F(FlowListEntryUt,_SupportCheck_dstIpAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[3] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_DST_IP_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_dstIpPrefixAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[4] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_DST_IP_PREFIX_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_srcipAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[5] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_srcIPPrefixAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[6] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_SRC_IP_PREFIX_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_VlanPriAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[7] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_VLAN_PRIORITY_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_dstIpv6Attrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[8] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_dstIpv6Prefix_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[9] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_srcIPv6Attrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[10] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_srcIpV6PrefixAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[11] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_IPProtoAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[12] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_IP_PROTOCOL_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_IPDscp_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[13] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_dstportAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[14] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_dstportendptAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[15] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_srcportAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[16] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_srcportendptAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[17] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_icmptypeAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[18] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt,_SupportCheck_IcmpCodeAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[19] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE]);
free(val_flowlist_entry);
}
TEST_F(FlowListEntryUt,_SupportCheck_icmpV6TypeAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[20] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE]);
free(val_flowlist_entry);
}
TEST_F(FlowListEntryUt,_SupportCheck_icmpV6CodeAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
attrs[21] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheckSuccess) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
free (val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_dstMacAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[0] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_MAC_DST_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_srcMacAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[1] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_MAC_SRC_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_ethtypeAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[2] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_MAC_ETH_TYPE_FLE]);
free(val_flowlist_entry);
}
TEST_F(FlowListEntryUt, SupportCheck_dstIpAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[3] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_DST_IP_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_dstIpPrefixAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[4] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_DST_IP_PREFIX_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_srcipAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[5] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_srcIPPrefixAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[6] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_SRC_IP_PREFIX_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_VlanPriAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[7] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_VLAN_PRIORITY_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_dstIpv6Attrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[8] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_dstIpv6Prefix_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[9] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_srcIPv6Attrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[10] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_srcIpV6PrefixAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[11] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_IPProtoAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[12] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_IP_PROTOCOL_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_IPDscp_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[13] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_dstportAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[14] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_dstportendptAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[15] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_srcportAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[16] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_srcportendptAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[17] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_icmptypeAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[18] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE]);
free(val_flowlist_entry);
}

TEST_F(FlowListEntryUt, SupportCheck_IcmpCodeAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[19] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE]);
free(val_flowlist_entry);
}
TEST_F(FlowListEntryUt, SupportCheck_icmpV6TypeAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[20] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE]);
free(val_flowlist_entry);
}
TEST_F(FlowListEntryUt, SupportCheck_icmpV6CodeAttrs_zero) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
uint8_t attrs[22];
for(int i= 0; i<22; i++) {
attrs[i] = 1;
val_flowlist_entry->valid[i] = UNC_VF_VALID;
}
attrs[21] = 0;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE]);
free(val_flowlist_entry);
}
/*=======================CompareValidValue==============================================*/
TEST_F(FlowListEntryUt, CompareValidValue_auditTrue) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *flowlist_entry_val1 = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry_t *flowlist_entry_val2 = ZALLOC_TYPE(val_flowlist_entry_t);

for ( unsigned int loop = 0; loop < sizeof(flowlist_entry_val1->valid);
          ++loop ) {
 flowlist_entry_val1->valid[loop] = UNC_VF_INVALID;
 flowlist_entry_val2->valid[loop] = UNC_VF_VALID;
    }

void *v1 = reinterpret_cast<void *>(flowlist_entry_val1);
void *v2 = reinterpret_cast<void *>(flowlist_entry_val2);

obj.CompareValidValue(v1, v2, true);
for ( unsigned int loop = 0; loop < sizeof(flowlist_entry_val1->valid);
          ++loop ) {
EXPECT_EQ(UNC_VF_VALID_NO_VALUE, flowlist_entry_val1->valid[loop]);
}
}

TEST_F(FlowListEntryUt, CompareValidValue_auditTrue_ValNoMatch) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *flowlist_entry_val1 = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry_t *flowlist_entry_val2 = ZALLOC_TYPE(val_flowlist_entry_t);
memset(flowlist_entry_val1, 1, sizeof(val_flowlist_entry_t));
memset(flowlist_entry_val2, 10, sizeof(val_flowlist_entry_t));

for ( unsigned int loop = 0; loop < sizeof(flowlist_entry_val1->valid);
          ++loop ) {
 flowlist_entry_val1->valid[loop] = UNC_VF_VALID;
 flowlist_entry_val2->valid[loop] = UNC_VF_VALID;
}

void *v1 = reinterpret_cast<void *>(flowlist_entry_val1);
void *v2 = reinterpret_cast<void *>(flowlist_entry_val2);

obj.CompareValidValue(v1, v2, true);
for ( unsigned int loop = 0; loop < sizeof(flowlist_entry_val1->valid);
          ++loop ) {
EXPECT_EQ(UNC_VF_VALID, flowlist_entry_val1->valid[loop]);
}
}
TEST_F(FlowListEntryUt, CompareValidValue_auditFalse_ValMatch) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *flowlist_entry_val1 = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry_t *flowlist_entry_val2 = ZALLOC_TYPE(val_flowlist_entry_t);
memset(flowlist_entry_val1, 0, sizeof(val_flowlist_entry_t));
memset(flowlist_entry_val2, 0, sizeof(val_flowlist_entry_t));

for ( unsigned int loop = 0; loop < sizeof(flowlist_entry_val1->valid);
          ++loop ) {
 flowlist_entry_val1->valid[loop] = UNC_VF_VALID;
 flowlist_entry_val2->valid[loop] = UNC_VF_VALID;
}
memset(flowlist_entry_val1->mac_dst, 10, sizeof(flowlist_entry_val1->mac_dst));
memset(flowlist_entry_val2->mac_dst, 10, sizeof(flowlist_entry_val1->mac_dst));
memset(flowlist_entry_val1->mac_src, 10, sizeof(flowlist_entry_val1->mac_src));
memset(flowlist_entry_val2->mac_src, 10, sizeof(flowlist_entry_val1->mac_src));

struct in_addr temp,temp1;
inet_aton("1.2.3.4",&temp);
inet_aton("1.2.3.4",&temp1);

//memset(flowlist_entry_val1->mac_dst, 2, sizeof(flowlist_entry_val1->mac_dst));
//memset(flowlist_entry_val1->mac_src, 2, sizeof(flowlist_entry_val1->mac_src));

flowlist_entry_val1->mac_eth_type = 6;
flowlist_entry_val2->mac_eth_type = 6;

flowlist_entry_val1->dst_ip =  temp;
flowlist_entry_val2->dst_ip =  temp1;

flowlist_entry_val1->dst_ip_prefixlen =  10;
flowlist_entry_val2->dst_ip_prefixlen = 10;

flowlist_entry_val1->src_ip =  temp;
flowlist_entry_val2->src_ip =  temp1;

flowlist_entry_val1->src_ip_prefixlen =  10;
flowlist_entry_val2->src_ip_prefixlen = 10;
flowlist_entry_val1->vlan_priority = 0;
flowlist_entry_val2->vlan_priority = 0;

memset(&flowlist_entry_val1->dst_ipv6, 3, sizeof(flowlist_entry_val1->dst_ipv6));
memset(&flowlist_entry_val2->dst_ipv6, 3, sizeof(flowlist_entry_val2->dst_ipv6));

memset(&flowlist_entry_val1->src_ipv6, 3, sizeof(flowlist_entry_val1->src_ipv6));
memset(&flowlist_entry_val2->src_ipv6, 3, sizeof(flowlist_entry_val2->src_ipv6));

flowlist_entry_val1->dst_ipv6_prefixlen =  10;
flowlist_entry_val2->dst_ipv6_prefixlen = 10;

flowlist_entry_val1->src_ipv6_prefixlen =  10;
flowlist_entry_val2->src_ipv6_prefixlen = 10;

flowlist_entry_val1->ip_proto = 4;
flowlist_entry_val2->ip_proto = 4;

flowlist_entry_val1->ip_dscp = 4;
flowlist_entry_val2->ip_dscp = 4;
flowlist_entry_val1->l4_dst_port = 3;
flowlist_entry_val2->l4_dst_port = 3;

flowlist_entry_val1->l4_src_port = 3;
flowlist_entry_val2->l4_src_port = 3;

flowlist_entry_val1->l4_dst_port_endpt = 3;
flowlist_entry_val2->l4_dst_port_endpt = 3;

flowlist_entry_val1->l4_src_port_endpt = 3;
flowlist_entry_val2->l4_src_port_endpt = 3;


flowlist_entry_val1->icmp_type = 10;
flowlist_entry_val2->icmp_type = 10;

flowlist_entry_val1->icmp_code = 10;
flowlist_entry_val2->icmp_code = 10;

flowlist_entry_val1->icmpv6_type = 10;
flowlist_entry_val2->icmpv6_type = 10;

flowlist_entry_val1->icmpv6_code = 10;
flowlist_entry_val2->icmpv6_code = 10;

void *v1 = reinterpret_cast<void *>(flowlist_entry_val1);
void *v2 = reinterpret_cast<void *>(flowlist_entry_val2);

obj.CompareValidValue(v1, v2, true);
for ( unsigned int loop = 0; loop < sizeof(flowlist_entry_val1->valid);
          ++loop ) { 
printf("%d",flowlist_entry_val1->valid[loop]);

EXPECT_EQ(UNC_VF_INVALID, flowlist_entry_val1->valid[loop]);
//EXPECT_EQ(UNC_VF_INVALID, flowlist_entry_val1->valid[UPLL_IDX_DST_IP_FLE]);
}
}
#if 0 
//NULL check is not done
TEST_F(FlowListEntryUt, ValidateAttributeSuccess) {
FlowListEntryMoMgr obj;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey, NULL));
delete ikey;
}
#endif

TEST_F(FlowListEntryUt, ValidateAttributeError) {
FlowListEntryMoMgr obj;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey, NULL));
delete ikey;
}


/*============================GetRenamedUncKey==================================*/
TEST_F(FlowListEntryUt, GetRenamedUncKey_keyStructIn_ctrlr_keyNULL) {
 FlowListEntryMoMgr flowlist_obj;
  ConfigKeyVal *key=new ConfigKeyVal(UNC_KT_FLOWLIST);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.GetRenamedUncKey(key,UPLL_DT_CANDIDATE,NULL,NULL));
delete key;
}
TEST_F(FlowListEntryUt, GetRenamedUncKey_CtrlidNULL) {
  FlowListEntryMoMgr flowlist_obj;
  key_flowlist_entry_t *flowlist_entry_key = new key_flowlist_entry_t;
   memset(flowlist_entry_key, 0 ,sizeof(key_flowlist_entry_t));
DalDmlIntf *dmi(getDalDmlIntf());
strncpy(reinterpret_cast<char *>(flowlist_entry_key->flowlist_key.flowlist_name),
  "FLOWLIST",kMaxLenFlowListName);
  ConfigKeyVal *key=new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry,flowlist_entry_key);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.GetRenamedUncKey(key,UPLL_DT_CANDIDATE,dmi,NULL));
 delete key;
}


/*=========================ReadRecord===========================*/
TEST_F(FlowListEntryUt,ReadRecord_req_ikey_NULL) {
FlowListEntryMoMgr obj;
DalDmlIntf *dmi(getDalDmlIntf());
EXPECT_EQ(UNC_UPLL_RC_ERR_BAD_REQUEST, obj.ReadRecord(NULL,NULL, dmi));
}

TEST_F(FlowListEntryUt,ReadRecord_STATESuccess) {
FlowListEntryMoMgr obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST_NEW" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);

DalOdbcMgr::stub_setSingleRecordExists(true);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
DalDmlIntf *dmi(getDalDmlIntf());
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadRecord(req, ikey,dmi));
delete ikey; 
}

TEST_F(FlowListEntryUt,ReadRecord_RUNNING_Success1) {
FlowListEntryMoMgr obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST_NEW1" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_RUNNING;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
DalDmlIntf *dmi(getDalDmlIntf());
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadRecord(req, ikey,dmi));
delete ikey; 
}

TEST_F(FlowListEntryUt,ReadRecord_STATRUP_Success1) {
FlowListEntryMoMgr obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST_NEW1" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STARTUP;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_INVALID;
}
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
DalDmlIntf *dmi(getDalDmlIntf());
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadRecord(req, ikey,dmi));
delete ikey; 
}

TEST_F(FlowListEntryUt,ReadRecord_CANDIDATE_Success1) {
FlowListEntryMoMgr obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST_NEW1" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
DalOdbcMgr::stub_setSingleRecordExists(true);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_UPDATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
DalDmlIntf *dmi(getDalDmlIntf());
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadRecord(req, ikey,dmi));
delete ikey; 
}
/*
TEST_F(FlowListEntryUt,ReadMo_Success) {
FlowListEntryMoMgr obj;
IPC_REQ_RESP_HEADER_DECL(req);
req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_RUNNING;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST_NEW1" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
DalDmlIntf *dmi(getDalDmlIntf());

EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
delete ikey; 
}

TEST_F(FlowListEntryUt,ReadSiblingMo_Success) {
FlowListEntryMoMgr obj;
bool begin = false;
IPC_REQ_RESP_HEADER_DECL(req);
req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_RUNNING;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST_NEW1" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
for(uint8_t i =0; i<sizeof(val_flowlist_entry->valid); i++) {
  val_flowlist_entry->valid[i] = UNC_VF_VALID_NO_VALUE;
}
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
DalDmlIntf *dmi(getDalDmlIntf());

EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadSiblingMo(req, ikey, begin, dmi));
delete ikey; 
}
*/
/*=====================UpdateConfigStatus====================================================*/
TEST_F(FlowListEntryUt, UpdateConfigStatus_key_NULL) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(NULL, UNC_OP_DELETE, 2, NULL, dmi, NULL));
}
TEST_F(FlowListEntryUt, UpdateConfigStatus_ctrlr_keyNULL) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *keyy= new ConfigKeyVal(UNC_KT_FLOWLIST);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(keyy, UNC_OP_DELETE, 2, NULL, dmi, NULL));
  delete keyy;
}

TEST_F(FlowListEntryUt, UpdateConfigStatus_valstructNULL) {
FlowListEntryMoMgr obj;
 DalDmlIntf *dmi(getDalDmlIntf());
 //ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(key,
                        UNC_OP_DELETE, 2, NULL, dmi, ctrlr_key));
  delete key;delete ctrlr_key;
}

TEST_F(FlowListEntryUt, UpdateConfigStatus_ctrlkey_valstructNULL) {
FlowListEntryMoMgr obj;
 DalDmlIntf *dmi(getDalDmlIntf());
 //ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
val_flowlist_entry_t *flowlist_val = ZALLOC_TYPE(val_flowlist_entry_t);
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                                      flowlist_val);
ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                              IpctSt::kIpcStKeyFlowlistEntry, NULL, config_val);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(key,UNC_OP_DELETE,
                                            2, NULL, dmi, ctrlr_key));
  delete key; delete ctrlr_key;
}

TEST_F(FlowListEntryUt, UpdateConfigStatus_Success) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

val_flowlist_entry_t *val = ZALLOC_TYPE(val_flowlist_entry_t); 
val_flowlist_entry_ctrl_t *val_ctrl = ZALLOC_TYPE(val_flowlist_entry_ctrl_t);

for ( unsigned int loop = 0;
          loop < PFC_ARRAY_CAPACITY(val->valid);
          ++loop ) {
 val->valid[loop] = UNC_VF_NOT_SUPPORTED;
 val_ctrl->valid[loop] = UNC_VF_NOT_SUPPORTED;

}
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            NULL, tmp);
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_ctrl);
 ctrlr_key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            NULL, tmp1);
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
 val_flowlist_entry_t *output = reinterpret_cast<val_flowlist_entry_t *> (GetVal(key));
  val_flowlist_entry_ctrl_t *val_ctrl1 = reinterpret_cast<val_flowlist_entry_ctrl_t *>(GetVal(ctrlr_key));
for ( unsigned int loop = 0;
          loop < PFC_ARRAY_CAPACITY(output->valid);
          ++loop ) {
 // EXPECT_EQ(UNC_CS_UNKNOWN,output->cs_row_status);
 EXPECT_EQ(1, output->cs_attr[loop]);
 EXPECT_EQ(UNC_CS_APPLIED, val_ctrl1->cs_attr[loop]);
}
delete key; delete ctrlr_key;

}
TEST_F(FlowListEntryUt, UpdateConfigStatus_Success1) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowlist_entry_t *val = ZALLOC_TYPE(val_flowlist_entry_t); 
val->cs_row_status = UNC_CS_APPLIED;
val_flowlist_entry_ctrl_t *val_ctrl = ZALLOC_TYPE(val_flowlist_entry_ctrl_t); 

for ( unsigned int loop = 0;
          loop < PFC_ARRAY_CAPACITY(val->valid);
          ++loop ) {
 val->valid[loop] = UNC_VF_VALID;
 val_ctrl->valid[loop] = UNC_VF_VALID;

}
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            NULL, tmp);
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_ctrl);
 ctrlr_key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            NULL, tmp1);
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
 val_flowlist_entry_t *output = reinterpret_cast<val_flowlist_entry_t *> (GetVal(key));
  val_flowlist_entry_ctrl_t *val_ctrl1 = reinterpret_cast<val_flowlist_entry_ctrl_t *>(GetVal(ctrlr_key));
for ( unsigned int loop = 0;
          loop < PFC_ARRAY_CAPACITY(output->valid);
          ++loop ) {
 //EXPECT_EQ(UNC_CS_UNKNOWN,output->cs_row_status);
 //EXPECT_EQ(UNC_CS_PARTIALLY_APPLIED, output->cs_attr[loop]);
 EXPECT_EQ(UNC_CS_APPLIED, val_ctrl1->cs_attr[loop]);
}
delete key; delete ctrlr_key;

}
TEST_F(FlowListEntryUt, UpdateConfigStatus_Success2) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowlist_entry_t *val =  ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry_ctrl_t *val_ctrl = ZALLOC_TYPE(val_flowlist_entry_ctrl_t);

for ( unsigned int loop = 0;
          loop < PFC_ARRAY_CAPACITY(val->valid);
          ++loop ) {
 val->valid[loop] = UNC_VF_VALID;
 val_ctrl->valid[loop] = UNC_VF_NOT_SUPPORTED;
}
val->cs_row_status = UNC_CS_UNKNOWN;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            NULL, tmp);
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_ctrl);
 ctrlr_key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            NULL, tmp1);
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_UPDATE, 0, upd_key, dmi, ctrlr_key));
 val_flowlist_entry_t *output = reinterpret_cast<val_flowlist_entry_t *> (GetVal(key));
  //val_flowlist_entry_ctrl_t *val_ctrl1 = reinterpret_cast<val_flowlist_entry_ctrl_t *>(GetVal(ctrlr_key));
for ( unsigned int loop = 0;
          loop < PFC_ARRAY_CAPACITY(output->valid);
          ++loop ) {
 // EXPECT_EQ(UNC_CS_UNKNOWN,output->cs_row_status);
 EXPECT_EQ(UNC_CS_UNKNOWN, output->cs_attr[loop]);
 //EXPECT_EQ(UNC_CS_NOT_SUPPORTED, val_ctrl1->cs_attr[loop]);
}
delete key; delete ctrlr_key;
}

TEST_F(FlowListEntryUt, UpdateConfigStatus_Success3) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowlist_entry_t *val = ZALLOC_TYPE(val_flowlist_entry_t);
val_flowlist_entry_ctrl_t *val_ctrl = ZALLOC_TYPE(val_flowlist_entry_ctrl_t); 

for ( unsigned int loop = 0;
          loop < PFC_ARRAY_CAPACITY(val->valid);
          ++loop ) {
 val->valid[loop] = UNC_VF_VALID;
 val_ctrl->valid[loop] = UNC_VF_VALID;
}
val->cs_row_status = UNC_CS_UNKNOWN;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            NULL, tmp);
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_ctrl);
 ctrlr_key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            NULL, tmp1);
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_UPDATE, 0, upd_key, dmi, ctrlr_key));
 val_flowlist_entry_t *output = reinterpret_cast<val_flowlist_entry_t *> (GetVal(key));
  //val_flowlist_entry_ctrl_t *val_ctrl1 = reinterpret_cast<val_flowlist_entry_ctrl_t *>(GetVal(ctrlr_key));
for ( unsigned int loop = 0;
          loop < PFC_ARRAY_CAPACITY(output->valid);
          ++loop ) {
 // EXPECT_EQ(UNC_CS_UNKNOWN,output->cs_row_status);
 EXPECT_EQ(UNC_CS_UNKNOWN, output->cs_attr[loop]);
 //EXPECT_EQ(UNC_CS_APPLIED, val_ctrl1->cs_attr[loop]);
}
delete key; delete ctrlr_key;
}
/*=====================================MergeValidate=========================*/
TEST_F(FlowListEntryUt, MergeValidate_Success) {
 FlowListEntryMoMgr obj;
 DalDmlIntf *dmi(getDalDmlIntf());
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.MergeValidate(UNC_KT_FLOWLIST_ENTRY, NULL,NULL, dmi,UPLL_IMPORT_TYPE_FULL));
}

/*====================RenameMo=======================================*/
TEST_F(FlowListEntryUt, RenameMo_Success) {
 FlowListEntryMoMgr obj;
EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.RenameMo(NULL, NULL,NULL, NULL));
}
/*=================================IsReferenced==================================*/
TEST_F(FlowListEntryUt, IsReferenced_ikeyNULL) {
FlowListEntryMoMgr obj;
IPC_REQ_RESP_HEADER_DECL(req);
req->datatype = UPLL_DT_CANDIDATE;
EXPECT_EQ(UPLL_RC_ERR_CFG_SEMANTIC, obj.IsReferenced(req, NULL, NULL));
}
#if 0 
//Not handled in code
TEST_F(FlowListEntryUt, IsReferenced_keystructNULL) {
 FlowListEntryMoMgr obj;
ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
DalDmlIntf *dmi(getDalDmlIntf());
EXPECT_EQ(UPLL_RC_ERR_CFG_SEMANTIC, obj.IsReferenced(key, UPLL_DT_CANDIDATE, dmi));
delete key;
}
#endif

#if 0
//Not handled in code
TEST_F(FlowListEntryUt, IsReferenced_ValstructNULL) {
FlowListEntryMoMgr obj;
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry);
EXPECT_EQ(UPLL_RC_ERR_CFG_SEMANTIC, obj.IsReferenced(key, UPLL_DT_CANDIDATE, NULL));
delete key;
}
#endif 

TEST_F(FlowListEntryUt, SetValidAttributesForController) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val = new val_flowlist_entry_t;
 for ( unsigned int loop = 0;
      loop < PFC_ARRAY_CAPACITY(val->valid); ++loop ) {
val->valid[loop] = UNC_VF_NOT_SUPPORTED;
}
obj.SetValidAttributesForController(val);
for ( unsigned int loop = 0;
      loop < PFC_ARRAY_CAPACITY(val->valid); ++loop ) {
EXPECT_EQ(UNC_VF_INVALID, val->valid[loop]);
}
delete val;
}
TEST_F(FlowListEntryUt, SetValidAttributesForControllerSucc1) {
FlowListEntryMoMgr obj;
val_flowlist_entry_t *val = new val_flowlist_entry_t;
 for ( unsigned int loop = 0;
      loop < PFC_ARRAY_CAPACITY(val->valid); ++loop ) {
val->valid[loop] = UNC_VF_VALID_NO_VALUE;
}
obj.SetValidAttributesForController(val);
for ( unsigned int loop = 0;
      loop < PFC_ARRAY_CAPACITY(val->valid); ++loop ) {
EXPECT_EQ(UNC_VF_INVALID, val->valid[loop]);
}
delete val;
}

/*=======================================GetRenamedControllerKey==============================*/
TEST_F(FlowListEntryUt, GetRenamedControllerKey_ctrlr_domNULL) {
FlowListEntryMoMgr obj;
ConfigKeyVal *key = NULL;
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedControllerKey(key, UPLL_DT_CANDIDATE, NULL, NULL));
}

/* this function is not returning SUCCESS as DB is not filling Rename struct*/
TEST_F(FlowListEntryUt, GetRenamedControllerKey_Success) {
FlowListEntryMoMgr obj;
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memcpy(key_flowlist_entry->flowlist_key.flowlist_name, "FLOWLIST", kMaxLenFlowListName);
val_rename_flowlist_t *rename_val = NULL;
rename_val = ZALLOC_TYPE(val_rename_flowlist_t);

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, rename_val);
ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
unc::upll::kt_momgr::key_user_data_t *user_data = ZALLOC_TYPE(unc::upll::kt_momgr::key_user_data_t);
memset(user_data, 0, sizeof(unc::upll::kt_momgr::key_user_data_t));
user_data->flags = 1;
key->set_user_data((void*)user_data);
controller_domain *dom = ZALLOC_TYPE(controller_domain_t);
uint8_t name[3] = {'P','F','C'};
dom->ctrlr = name;

DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
DalDmlIntf *dmi(getDalDmlIntf());
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedControllerKey(key, UPLL_DT_IMPORT, dmi, NULL));
delete key;
}

TEST_F(FlowListEntryUt, TxCopyCandidateToRunning_cs_null) {
  FlowListEntryMoMgr obj;
  typedef list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  CtrlrCommitStatusList * l_CtrlrCommitStatusList = NULL;

  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  string vtn_name = "vtn1";

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
           TxCopyCandidateToRunning
          (UNC_KT_FLOWLIST_ENTRY, l_CtrlrCommitStatusList, dmi,
           config_mode, vtn_name));
DalOdbcMgr::clearStubData();

}
TEST_F(FlowListEntryUt, UpdateControllerTable_KEYcheck) {
  FlowListEntryMoMgr obj;
  upll_keytype_datatype_t dt_type=UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr() ;
  ConfigVal *l_cval = NULL;
  val_flowlist_entry_t* val_flowlist = new val_flowlist_entry_t();
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_flowlist);
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  string vtn_name = "vtn1";

  char ctrl_id[] ={'P','F','C'};
  key_flowlist_entry_t *key_flowlist = NULL;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  key_flowlist = new key_flowlist_entry_t();
  strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_key.flowlist_name),
		"flowlist" , kMaxLenFlowListName);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            key_flowlist, l_cval);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 UpdateControllerTable
                    (l_ikey,UNC_OP_UPDATE, dt_type, dmi, ctrl_id, config_mode,
                     vtn_name));

  delete l_ikey;
DalOdbcMgr::clearStubData();
}

TEST_F(FlowListEntryUt, UpdateControllerTable_Audit) {
  FlowListEntryMoMgr obj;
  upll_keytype_datatype_t dt_type=UPLL_DT_AUDIT;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr() ;
  ConfigVal *l_cval = NULL;
  val_flowlist_entry_t* val_flowlist = new val_flowlist_entry_t();
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_flowlist);
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  string vtn_name = "vtn1";

  char ctrl_id[] ={'P','F','C'};
  key_flowlist_entry_t *key_flowlist = NULL;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  key_flowlist = new key_flowlist_entry_t();
  strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_key.flowlist_name),
		"flowlist" , kMaxLenFlowListName);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            key_flowlist, l_cval);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 UpdateControllerTable
                    (l_ikey,UNC_OP_UPDATE, dt_type, dmi, ctrl_id,
                     config_mode, vtn_name));

  delete l_ikey;
DalOdbcMgr::clearStubData();
}

// This test may fail if the task queue is not initialized.
TEST_F(FlowListEntryUt, DISABLED_UpdateControllerTable_Createentry) {
  FlowListEntryMoMgr obj;
  upll_keytype_datatype_t dt_type=UPLL_DT_CANDIDATE; 
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *l_cval = NULL;
  val_flowlist_entry_t* val_flowlist = new val_flowlist_entry_t();
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_flowlist);
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  string vtn_name = "vtn1";

  char ctrl_id[] ={'P','F','C'};
  key_flowlist_entry_t *key_flowlist = NULL;
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrl_id, UNC_CT_PFC, version, 0);
   CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_CANDIDATE));
   CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
   DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
    DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNoMore);
    DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS, kDalRcSuccess);
//    DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
//  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::READ_RECORD,kDalRcRecordNoMore);
  //CapaModuleStub::stub_clearStubData();
   //CapaModuleStub::stub_loadCapaModule();
  //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);
  key_flowlist = new key_flowlist_entry_t();
  strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_key.flowlist_name),
		"flowlist" , kMaxLenFlowListName);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            key_flowlist, l_cval);
  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.
                 UpdateControllerTable
                    (l_ikey,UNC_OP_CREATE, dt_type, dmi, ctrl_id,
                     config_mode, vtn_name));

  delete l_ikey;
  //CapaModuleStub::stub_clearStubData();
DalOdbcMgr::clearStubData();
}
TEST_F(FlowListEntryUt, TxCopyCandidateToRunning_CTRLTBL_FAIL) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  ConfigVal *l_cval = NULL;
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD,kDalRcRecordNoMore);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD, kDalRcRecordNoMore);
   DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD, kDalRcRecordNoMore);

  CtrlrCommitStatusList.push_back(l_CtrlrTxResult);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc_key_type_t keytype = UNC_KT_FLOWLIST_ENTRY;
  val_flowlist_entry_t* val_flowlist = new val_flowlist_entry_t();
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_flowlist);
  key_flowlist_entry_t *key_flowlist = ZALLOC_TYPE(key_flowlist_entry_t);
 dmi = new DalOdbcMgr();
 ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_flowlist, l_cval);
 
  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi, config_mode,
                     vtn_name));
DalOdbcMgr::clearStubData();
}
TEST_F(FlowListEntryUt, TxCopyCandidateToRunning_DELETE_UPDCONFG) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigVal *l_cval = NULL;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNoMore);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD,kDalRcRecordNoMore);
  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  CtrlrCommitStatusList.push_back(l_CtrlrTxResult); 
  unc_key_type_t keytype = UNC_KT_FLOWLIST_ENTRY;
  val_flowlist_entry_t* val_flowlist = new val_flowlist_entry_t();
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_flowlist);
 key_flowlist_entry_t*  key_flowlist = new key_flowlist_entry_t();
 dmi = new DalOdbcMgr();
 ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            key_flowlist,l_cval);

  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi, config_mode,
                     vtn_name));
 //CapaModuleStub::stub_clearStubData();
DalOdbcMgr::clearStubData();

}
TEST_F(FlowListEntryUt, TxCopyCandidateToRunningDELETEErr) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  string vtn_name = "vtn1";

  ConfigVal *l_cval = NULL;
   l_CtrlrTxResult->ctrlr_id.assign("ctrl",5);
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  CtrlrCommitStatusList.push_back((CtrlrCommitStatus*)l_CtrlrTxResult); 
  unc_key_type_t keytype = UNC_KT_FLOWLIST_ENTRY;
  val_flowlist_entry_t *ival =
                         new val_flowlist_entry_t();
  l_cval = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             ival);
  key_flowlist_entry_t*  key_flowlist = new key_flowlist_entry_t();
  dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            key_flowlist,l_cval);
 
  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi,
                     config_mode, vtn_name));
DalOdbcMgr::clearStubData();

}
TEST_F(FlowListEntryUt, TxCopyCandidateToRunningDELETESucc) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  string vtn_name = "vtn1";
                                
  ConfigVal *l_cval = NULL;
   l_CtrlrTxResult->ctrlr_id.assign("ctrl",5);
   DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess); 
 //  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcRecordNoMore);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
//   DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS, kDalRcRecordNoMore);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcRecordNoMore);

  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  CtrlrCommitStatusList.push_back((CtrlrCommitStatus*)l_CtrlrTxResult); 
  unc_key_type_t keytype = UNC_KT_FLOWLIST_ENTRY;
  val_flowlist_entry_t *ival =
                         new val_flowlist_entry_t();
  l_cval = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             ival);
  key_flowlist_entry_t*  key_flowlist = new key_flowlist_entry_t();
  dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            key_flowlist,l_cval);
 
  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi, config_mode,
                     vtn_name));
DalOdbcMgr::clearStubData();
}

TEST_F(FlowListEntryUt, EntryValdmi_err1) {
//  FlowListEntryMoMgr obj;
FlowListEntryMoMgr obj;

key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memcpy(key_flowlist_entry->flowlist_key.flowlist_name, "FLOWLIST", kMaxLenFlowListName);
ConfigVal *l_cval = NULL;
  val_flowlist_entry_t* val_flowlist = new val_flowlist_entry_t();
val_flowlist->valid[UPLL_IDX_SRC_IP_FLE] = UNC_VF_VALID;
val_flowlist->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] = UNC_VF_VALID;

  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_flowlist);
ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, l_cval);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
key_flowlist_t *key_flowlist = ZALLOC_TYPE(key_flowlist_t);
memcpy(key_flowlist->flowlist_name, "FLOWLIST", kMaxLenFlowListName);
IPC_REQ_RESP_HEADER_DECL(req);
req->operation = UNC_OP_READ;
DalDmlIntf *dmi =  new DalOdbcMgr();
 EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateFlowlistEntryVal(req, key, dmi));
}
TEST_F(FlowListEntryUt, TxCopyCandidateToRunning_noNULL) {
  FlowListEntryMoMgr obj;
  typedef list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  CtrlrCommitStatusList * l_CtrlrCommitStatusList = NULL;
  l_CtrlrCommitStatusList = new CtrlrCommitStatusList();
  DalDmlIntf *dmi = NULL;
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  string vtn_name = "vtn1";

  dmi = new DalOdbcMgr();
/*  CtrlrTxResult *l_CtrlrTxResult = new CtrlrTxResult ();
 *    l_CtrlrCommitStatusList.insert(l_CtrlrTxResult); */
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
           TxCopyCandidateToRunning
         (UNC_KT_FLOWLIST_ENTRY, l_CtrlrCommitStatusList, dmi,
          config_mode, vtn_name));
}
#if 0
//NULL not checked  in source code
TEST_F(FlowListEntryUt, TxUpdateControllerdmiNULL) {
  FlowListEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (UNC_KT_FLOWLIST_ENTRY,&CtrlrCommitStatusList,dmi));
}
#endif

TEST_F(FlowListEntryUt, TxUpdateControllerDiffconfigFail) {
  FlowListEntryMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> affected_ctrlr_set;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  string vtn_name = "vtn1";

  key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t); 
  strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
		"flowlist" , kMaxLenFlowListName);
   //key_flowlist_entry_t *l_key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t); 
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcTxnError);
  //val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
 //ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowlist_entry);
  //IpcRequest *ipc_req = ZALLOC_TYPE(IpcRequest);
   //IpcResponse *ipc_response = ZALLOC_TYPE(IpcResponse);

 ConfigKeyVal *l_err_ckv = NULL;

 DalDmlIntf *dmi(getDalDmlIntf());
 TxUpdateUtil *util = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS, 
                obj.TxUpdateController(UNC_KT_FLOWLIST_ENTRY,session_id,
                config_id,phase,&affected_ctrlr_set,dmi,&l_err_ckv, util,
                config_mode, vtn_name));
 DalOdbcMgr::clearStubData();
}
/*
TEST_F(FlowListEntryUt,diffconfig_Default) {
  FlowListEntryMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> affected_ctrlr_set;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpUpdate;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
  strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
		"Flowlist" , kMaxLenFlowListName);
   //key_flowlist_entry_t *l_key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
 i = 0;
char ctrlr_name[] = {'p','f','c'};
 const char* version("version");
CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS,kDalRcTxnError);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS, kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNoMore);


//CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);
  //val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
 //ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist_entry);
  //IpcRequest *ipc_req = ZALLOC_TYPE(IpcRequest);
   //IpcResponse *ipc_response = ZALLOC_TYPE(IpcResponse);

  ConfigKeyVal *l_err_ckv = NULL;
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  string vtn_name = "vtn1";

  DalDmlIntf *dmi(getDalDmlIntf());
  TxUpdateUtil *util = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, 
                obj.TxUpdateController(UNC_KT_FLOWLIST_ENTRY,session_id,
                config_id,phase,&affected_ctrlr_set,dmi,&l_err_ckv, util,
                config_mode, vtn_name));
 DalOdbcMgr::clearStubData();
}
*/
TEST_F(FlowListEntryUt, TxUpdateControllerGetnextRecord_succ) {
  FlowListEntryMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> affected_ctrlr_set;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  string vtn_name = "vtn1";

  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
  strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
		"flowlist" , kMaxLenFlowListName);
   //key_flowlist_entry_t *l_key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
 i = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNoMore);
  //val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
 //ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowlist_entry);
  //IpcRequest *ipc_req = ZALLOC_TYPE(IpcRequest);
   //IpcResponse *ipc_response = ZALLOC_TYPE(IpcResponse);

 ConfigKeyVal *l_err_ckv = NULL;

 DalDmlIntf *dmi(getDalDmlIntf());
 TxUpdateUtil *util = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS, 
                obj.TxUpdateController(UNC_KT_FLOWLIST_ENTRY,session_id,
                config_id,phase,&affected_ctrlr_set,dmi,&l_err_ckv, util,
                config_mode, vtn_name));
 DalOdbcMgr::clearStubData();
}
#if 0
TxUpdateController tested in momgr_impl_ut
TEST_F(FlowListEntryUt, TxUpdateControllerUpdateProcessSucc) {
  FlowListEntryMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> affected_ctrlr_set;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
  strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
		"vtn" , kMaxLenFlowListName);
   key_flowlist_entry_t *l_key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
   char ctrlr_name[] = {'p','f','c'};
 const char* version("version");
CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 
 i = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);
  IpcRequest *ipc_req = ZALLOC_TYPE(IpcRequest);
   IpcResponse *ipc_response = ZALLOC_TYPE(IpcResponse);

 ConfigKeyVal *l_err_ckv = NULL;
system("psql -p 12730 -d uncdb uncdbuser -a -f sql_cmds/delete_tables.sql");
  system("psql -p 12730 -d uncdb uncdbuser -a -f sql_cmds/insert_fl_create_table.sql");
    system("psql -p 12730 -d uncdb uncdbuser -a -f sql_cmds/insert_fle_create_table.sql");

 DalDmlIntf *dmi(getDalDmlIntf());
 TxUpdateUtil *util;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, 
                obj.TxUpdateController(UNC_KT_FLOWLIST_ENTRY,session_id,config_id,phase,&affected_ctrlr_set,dmi,&l_err_ckv, util));
 DalOdbcMgr::clearStubData();
}
#endif
TEST_F(FlowListEntryUt,AddFlowListToControllerctrlid_NULL) {
FlowListEntryMoMgr obj;
DalDmlIntf *dmi(getDalDmlIntf());
upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
string vtn_name = "vtn1";

char fl_name[4] = {'F','L','O','W'};
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AddFlowListToController(fl_name,dmi, NULL,
           dt_type, UNC_OP_READ, config_mode, vtn_name));
}

TEST_F(FlowListEntryUt,AddFlowListToControllerctrlid_CtrlName) {
FlowListEntryMoMgr obj;
DalDmlIntf *dmi(getDalDmlIntf());
upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
string vtn_name = "vtn1";

char ctrlr_name[] = {'c','n','t','r','l','r'};
const char* version("version");
CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 //CapaModuleStub::stub_loadCapaModule();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNoMore);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
char fl_name[4] = {'F','L','O','W'};
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();

EXPECT_EQ(UPLL_RC_SUCCESS, obj.AddFlowListToController(fl_name,dmi,
          ctrlr_name, dt_type, UNC_OP_CREATE, config_mode, vtn_name));
//CapaModuleStub::stub_clearStubData();
DalOdbcMgr::clearStubData();
}

TEST_F(FlowListEntryUt,AddFlowListToControllerctrlid_Err1) {
FlowListEntryMoMgr obj;
DalDmlIntf *dmi(getDalDmlIntf());
upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
char ctrlr_name[] = {'c','n','t','r','l','r'};
const char* version("version");
TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
string vtn_name = "vtn1";

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 //CapaModuleStub::stub_loadCapaModule();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

char fl_name[4] = {'F','L','O','W'};
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();

EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AddFlowListToController(fl_name,dmi,
          ctrlr_name, dt_type, UNC_OP_CREATE, config_mode, vtn_name));
//CapaModuleStub::stub_clearStubData();
}
#if 0
TEST_F(FlowListEntryUt,AddFlowListToControllerctrlid_Err2) {
MoMgrTest obj;
DalDmlIntf *dmi(getDalDmlIntf());
upll_keytype_datatype_t dt_type;
char * ctrlr_name = "cntrlr_name";
const char* version("version");
MoMgrTest::stub_result[ValidateCapability1] = UPLL_RC_ERR_GENERIC;
MoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
char fl_name[4] = {'F','L','O','W'};

EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AddFlowListToController(fl_name,dmi, ctrlr_name, dt_type, UNC_OP_CREATE));
CapaModuleStub::stub_clearStubData();
}

#endif
TEST_F(FlowListEntryUt,CreateCandidateMo_ikeyNULL) {
FlowListEntryMoMgr obj;
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(NULL,NULL,NULL));
}
TEST_F(FlowListEntryUt,CreateCandidateMo_err) {
FlowListEntryMoMgr obj;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
IPC_REQ_RESP_HEADER_DECL(req);
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.CreateCandidateMo(req, ikey,NULL));
}

TEST_F(FlowListEntryUt,CreateCandidateMo_Success) {
FlowListEntryMoMgr flowlist_obj;
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
key_flowlist_entry->sequence_num = 10;
//DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
//DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT, kDalRcSuccess);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordNotFound);

DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE, kDalRcRecordNotFound);

IPC_REQ_RESP_HEADER_DECL(req);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);

val_flowlist_entry->ip_dscp = 10;
val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE]
      = UNC_VF_VALID;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
  IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);
DalDmlIntf *dmi(getDalDmlIntf());
EXPECT_EQ(UNC_UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID, flowlist_obj.CreateCandidateMo(req, ikey, dmi));
DalOdbcMgr::clearStubData();
}
TEST_F(FlowListEntryUt, ParentConfigKey) {
FlowListEntryMoMgr obj;
ConfigKeyVal *ikey =  NULL;
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetParentConfigKey(ikey, NULL));
}

#if 0
TEST_F(FlowListEntryUt, CreateCandidate) {
MoMgrTest obj;
IPC_REQ_RESP_HEADER_DECL(req);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
DalDmlIntf *dmi(getDalDmlIntf());
MoMgrTest::stub_result[ValidateMessage1] = UPLL_RC_SUCCESS;
MoMgrTest::stub_result[FlowlistEntryVal] = UPLL_RC_SUCCESS;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.CreateCandidateMo(req, ikey, dmi));
}
#endif


/**********InstanceExistsInScratchTbl*****/
TEST_F(FlowListEntryUt, InstanceExistsInScratchTbl_1) {
  FlowListEntryMoMgr obj;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name = "VTN1";
  DalDmlIntf *dmi(getDalDmlIntf());

  key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
  memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
  strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
    "FLOWLIST" , kMaxLenFlowListName);
  key_flowlist_entry->sequence_num = 10;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordNotFound);

  val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);

  val_flowlist_entry->ip_dscp = 10;
  val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE] = UNC_VF_VALID;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

  ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
    IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.InstanceExistsInScratchTbl(ikey, config_mode,
                                 vtn_name, dmi));
  DalOdbcMgr::clearStubData();
}

TEST_F(FlowListEntryUt, InstanceExistsInScratchTbl_2) {
  FlowListEntryMoMgr obj;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name = "VTN1";
  DalDmlIntf *dmi(getDalDmlIntf());

  key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
  memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
  strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
    "FLOWLIST" , kMaxLenFlowListName);
  key_flowlist_entry->sequence_num = 10;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordNotFound);

  val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);

  val_flowlist_entry->ip_dscp = 10;
  val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE] = UNC_VF_VALID;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

  ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
    IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.InstanceExistsInScratchTbl(ikey, config_mode,
                                 vtn_name, dmi));
  DalOdbcMgr::clearStubData();
}

TEST_F(FlowListEntryUt, InstanceExistsInScratchTbl_3) {
  FlowListEntryMoMgr obj;
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  std::string vtn_name = "VTN1";
  DalDmlIntf *dmi(getDalDmlIntf());

  key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
  memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
  strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
    "FLOWLIST" , kMaxLenFlowListName);
  key_flowlist_entry->sequence_num = 10;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordNotFound);

  val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);

  val_flowlist_entry->ip_dscp = 10;
  val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE] = UNC_VF_VALID;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

  ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
    IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.InstanceExistsInScratchTbl(ikey, config_mode,
                                 vtn_name, dmi));
  DalOdbcMgr::clearStubData();
}

TEST_F(FlowListEntryUt, InstanceExistsInScratchTbl_4) {
  FlowListEntryMoMgr obj;
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  std::string vtn_name = "VTN1";
  DalDmlIntf *dmi(getDalDmlIntf());

  key_flowlist_entry_t *key_flowlist_entry = ZALLOC_TYPE(key_flowlist_entry_t);
  memset(key_flowlist_entry, 0, sizeof(key_flowlist_entry_t));
  strncpy(reinterpret_cast<char*>(key_flowlist_entry->flowlist_key.flowlist_name),
    "FLOWLIST" , kMaxLenFlowListName);
  key_flowlist_entry->sequence_num = 10;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcRecordNotFound);

  val_flowlist_entry_t *val_flowlist_entry = ZALLOC_TYPE(val_flowlist_entry_t);

  val_flowlist_entry->ip_dscp = 10;
  val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE] = UNC_VF_VALID;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val_flowlist_entry);

  ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
    IpctSt::kIpcStKeyFlowlistEntry, key_flowlist_entry, config_val);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcRecordNoMore);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.InstanceExistsInScratchTbl(ikey, config_mode,
                                 vtn_name, dmi));
  DalOdbcMgr::clearStubData();
}

