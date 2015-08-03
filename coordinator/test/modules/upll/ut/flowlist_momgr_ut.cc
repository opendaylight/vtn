/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <upll/flowlist_momgr.hh>
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "dal/dal_dml_intf.hh"
#include "dal/dal_odbc_mgr.hh"
#include "ctrlr_mgr.hh"
#include <config_mgr.hh>
//#include "capa_module/capa_intf.hh"
//#include <capa_module/capa_module_stub.hh>
#include <upll/config_mgr.hh>
#include "include/flowlist_stub.hh"
#include "ut_util.hh"
//int testcase_id;
//std::map<function, upll_rc_t> FlowlistTest::stub_result;

//bool fatal_done;
//pfc::core::Mutex  fatal_mutex_lock;

using namespace std;
using namespace unc::upll::dal;
using namespace unc::capa;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::config_momgr;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::test;
using namespace unc::upll::tx_update_util;


#define  NUM_KEY_MAIN_COL    3
#define  NUM_KEY_CTRL_COL    3
#define  NUM_KEY_RENAME_COL  2
int i_flowlist=0;
class FlowListMoMgrUt :
        public UpllTestEnv
{
};
/*******************AllocVal ************************/
TEST(FlowListMoMgrUt,AllocVal_ConfigValNOtNull) {
  FlowListMoMgr obj;
  ConfigVal* val = new ConfigVal;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(val, UPLL_DT_IMPORT, MAINTBL));
  delete val;
}

TEST(FlowListMoMgrUt,AllocVal_MAINTBL_Success) {
  FlowListMoMgr obj;
  ConfigVal* val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_IMPORT,MAINTBL)); 
}

TEST(FlowListMoMgrUt,AllocVal_RENAMETBL_Success) {
  FlowListMoMgr obj;
  ConfigVal* val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_STATE,RENAMETBL));   
}

TEST(FlowListMoMgrUt,AllocVal_CTRLRTBL_Success) {
  FlowListMoMgr obj;
  ConfigVal* val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_STATE, CTRLRTBL));
}

TEST(FlowListMoMgrUt,AllocVal_Error) {
  FlowListMoMgr obj;
  ConfigVal* val = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(val, UPLL_DT_STATE, MAX_MOMGR_TBLS));
}

/*================GetChildConfigKey====================================*/
TEST(FlowlistMoMgr,GetChildConfigKey_okey_NULL_pkey_keystructNULL) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_FLOWLIST);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetChildConfigKey(okey, pkey));
  delete pkey; if (okey) delete okey;

}
TEST(FlowlistMoMgr,GetChildConfigKey_ROOT) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_flowlist_t *flowlist_key = reinterpret_cast<key_flowlist_t*>(
    malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key->flowlist_name, 0, kMaxLenFlowListName);
  memcpy(&(flowlist_key->flowlist_name), "FLOWLIST", kMaxLenFlowListName);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_ROOT, IpctSt::kIpcStKeyFlowlist, flowlist_key);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey));
}

TEST(FlowlistMoMgr,GetChildConfigKey_InvalidKT) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_flowlist_t *flowlist_key = reinterpret_cast<key_flowlist_t*>(
    malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key->flowlist_name, 0, kMaxLenFlowListName);
  memcpy(&(flowlist_key->flowlist_name), "FLOWLIST", kMaxLenFlowListName);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlist, flowlist_key);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetChildConfigKey(okey, pkey));
}


TEST(FlowlistMoMgr,GetChildConfigKey_okey_NULL_pkey_validStruct) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_flowlist_t *flowlist_key = reinterpret_cast<key_flowlist_t*>(
    malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key->flowlist_name, 0, kMaxLenFlowListName);
  memcpy(&(flowlist_key->flowlist_name), "FLOWLIST", kMaxLenFlowListName);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, flowlist_key);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey));

/** have to check why it is failing
  key_flowlist_t *output = reinterpret_cast<key_flowlist_t *> (okey->get_key());
   
  if (output)
  EXPECT_STREQ("FLOWLIST",(reinterpret_cast<const char *> (output->flowlist_name)));
*/
  delete pkey; if (okey) delete okey;

}

/*==========================DupConfigKeyVal============================*/
TEST(FlowlistMoMgr, DupConfigKeyVal_reqnull) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req,MAINTBL));
   if (okey) delete okey; if(req) delete req;

}

TEST(FlowlistMoMgr, DupConfigKeyVal_okeyNULL) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req,MAINTBL));
  delete req;if(okey) delete okey;
}

TEST(FlowlistMoMgr, DupConfigKeyVal_reqInvalidKT) {
 FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req,MAINTBL));
  delete req;if(okey) delete okey;
}

TEST(FlowlistMoMgr, DupConfigKeyVal_MAINTBL_ConfigVal_withNoValStruct) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist);
  key_flowlist_t *flowlist_key = reinterpret_cast<key_flowlist_t*>(
     malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key->flowlist_name, 0, kMaxLenFlowListName);
  memcpy(&(flowlist_key->flowlist_name), "FLOWLIST", kMaxLenFlowListName);

  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, NULL, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req,MAINTBL));
  delete req; if(okey) delete okey;
}

TEST(FlowlistMoMgr, DupConfigKeyVal_MAINTBL_ValStruct_NotNULL_butEmpty) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
     malloc(sizeof(key_flowlist_t)));
  val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
    malloc(sizeof(val_flowlist_t)));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, req,MAINTBL));
  delete req; if(okey) delete okey;
}

TEST(FlowlistMoMgr, DupConfigKeyVal_MAINTBL_Success) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
     malloc(sizeof(key_flowlist_t)));
  val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
    malloc(sizeof(val_flowlist_t)));
 val_flowlist->ip_type = UPLL_FLOWLIST_TYPE_IPV6;
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, req,MAINTBL));
  delete req; if (okey) delete okey;
}

TEST(FlowlistMoMgr, DupConfigKeyVal_RENAMETBL_Success) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
     malloc(sizeof(key_flowlist_t)));

  val_rename_flowlist_t *val_rename_flowlist = reinterpret_cast<val_rename_flowlist_t*>(
    malloc(sizeof(val_rename_flowlist_t)));
  memset(val_rename_flowlist->flowlist_newname, 0, kMaxLenFlowListName);
  memcpy(&(val_rename_flowlist->flowlist_newname), "FLOWLIST", kMaxLenFlowListName);

 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, val_rename_flowlist);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, req,RENAMETBL));
   delete req; if (okey) delete okey;

}

TEST(FlowlistMoMgr, DupConfigKeyVal_CTRLRTBL_Success) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
     malloc(sizeof(key_flowlist_t)));

   val_flowlist_ctrl *flowlist_ctrlr_val =
          reinterpret_cast<val_flowlist_ctrl *>(malloc
            (sizeof(val_flowlist_ctrl)));
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, flowlist_ctrlr_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey, req, CTRLRTBL));
   delete req;
if (okey) delete okey;
}
/*=====================SetConsolidatedStatus=========================*/
/* unable to make the Read-Multiple pass using stub*/
TEST(FlowListMoMgrUt, SetConsolidatedStatus_ikeyNotNull) {
  FlowListMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigVal *l_cval = NULL;
  val_flowlist_t* val_flowlist =
                       new val_flowlist_t();
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val_flowlist);
  dmi = new DalOdbcMgr();
  key_flowlist_t *key_flowlist =
    new key_flowlist_t();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNotFound);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT, kDalRcSuccess);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            key_flowlist,l_cval);
  EXPECT_EQ(UNC_UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.
                 SetConsolidatedStatus
                    (l_ikey,dmi));
DalOdbcMgr::clearStubData();

delete dmi;
delete l_ikey;
}

/*=====================UpdateConfigStatus====================================================*/
TEST(FlowlistMoMgr, UpdateConfigStatus_flowlistKey_NULL) {
  FlowListMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(NULL, UNC_OP_DELETE, 2, NULL, dmi, NULL));
}
TEST(FlowlistMoMgr, UpdateConfigStatus_ctrlr_keyNULL) {
  FlowListMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr();
  val_flowlist_t *flowlist = new val_flowlist_t;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, flowlist);
  ConfigKeyVal *keyy= new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, NULL, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(keyy, UNC_OP_DELETE, 2, NULL, dmi, NULL));
  delete keyy;
}
TEST(FlowlistMoMgr, UpdateConfigStatus_InvalidKT) {
  FlowListMoMgr obj;
  DalDmlIntf *dmi = new DalOdbcMgr();
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowlist_t *val = reinterpret_cast<val_flowlist_t *>
        (malloc(sizeof(val_flowlist_t)));
 memset(val, 0, sizeof(val_flowlist_t));
 val->valid[0] = UNC_VF_NOT_SUPPORTED;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp);
  val_flowlist_ctrl_t *val_ctrl = reinterpret_cast<val_flowlist_ctrl_t *>
        (malloc(sizeof(val_flowlist_ctrl_t)));
 memset(val_ctrl, 0, sizeof(val_flowlist_ctrl_t));
  val_ctrl->valid[0] = UNC_VF_NOT_SUPPORTED;
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val_ctrl);
 ctrlr_key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp1);
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_UPDATE, 0, upd_key, dmi, ctrlr_key));
 val_flowlist_t *output = reinterpret_cast<val_flowlist_t *> (GetVal(key));
  EXPECT_EQ(UNC_CS_UNKNOWN,output->cs_row_status);
delete key; delete ctrlr_key;

}

TEST(FlowListMoMgrUt, UpdateConfigStatus_Success) {
 FlowListMoMgr obj;
 DalDmlIntf *dmi = new DalOdbcMgr();
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowlist_t *val = reinterpret_cast<val_flowlist_t *>
        (malloc(sizeof(val_flowlist_t)));
 memset(val, 0, sizeof(val_flowlist_t));

 val->cs_row_status = UNC_CS_NOT_APPLIED;
 val->valid[0] = UNC_VF_VALID;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp);
  val_flowlist_ctrl_t *val_ctrl = reinterpret_cast<val_flowlist_ctrl_t *>
        (malloc(sizeof(val_flowlist_ctrl_t)));
 memset(val_ctrl, 0, sizeof(val_flowlist_ctrl_t));
val_ctrl->valid[0] = UNC_VF_NOT_SUPPORTED;
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val_ctrl);
 ctrlr_key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp1);

 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
 val_flowlist_t *output = reinterpret_cast<val_flowlist_t *> (GetVal(key));
  EXPECT_EQ(UNC_CS_PARTIALLY_APPLIED,output->cs_row_status);
delete key; delete ctrlr_key;
}
TEST(FlowListMoMgrUt, UpdateConfigStatus_Success2) {
 FlowListMoMgr obj;
 DalDmlIntf *dmi = new DalOdbcMgr();
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowlist_t *val = reinterpret_cast<val_flowlist_t *>
        (malloc(sizeof(val_flowlist_t)));
 memset(val, 0, sizeof(val_flowlist_t));

 val->cs_row_status = UNC_CS_PARTIALLY_APPLIED;
 val->valid[0] = UNC_VF_VALID;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp);
  val_flowlist_ctrl_t *val_ctrl = reinterpret_cast<val_flowlist_ctrl_t *>
        (malloc(sizeof(val_flowlist_ctrl_t)));
 memset(val_ctrl, 0, sizeof(val_flowlist_ctrl_t));
val_ctrl->valid[0] = UNC_VF_NOT_SUPPORTED;
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val_ctrl);
 ctrlr_key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp1);

 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
 //val_flowlist_t *output = reinterpret_cast<val_flowlist_t *> (GetVal(key));
  //EXPECT_EQ(UNC_CS_UNKNOWN,output->cs_row_status);
delete key; delete ctrlr_key;
}
TEST(FlowListMoMgrUt, UpdateConfigStatus_Success3) {
 FlowListMoMgr obj;
 DalDmlIntf *dmi = new DalOdbcMgr();
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowlist_t *val = reinterpret_cast<val_flowlist_t *>
        (malloc(sizeof(val_flowlist_t)));
 memset(val, 0, sizeof(val_flowlist_t));

 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp);
  val_flowlist_ctrl_t *val_ctrl = reinterpret_cast<val_flowlist_ctrl_t *>
        (malloc(sizeof(val_flowlist_ctrl_t)));
 memset(val_ctrl, 0, sizeof(val_flowlist_ctrl_t));
val_ctrl->valid[0] = UNC_VF_NOT_SUPPORTED;
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val_ctrl);
 ctrlr_key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp1);

 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 1, upd_key, dmi, ctrlr_key));
 val_flowlist_t *output = reinterpret_cast<val_flowlist_t *> (GetVal(key));
  EXPECT_EQ(UNC_CS_NOT_APPLIED,output->cs_row_status);
delete key; delete ctrlr_key;
}

TEST(FlowListMoMgrUt, UpdateConfigStatus_SuccessOpUpdate) {
 FlowListMoMgr obj;
 DalDmlIntf *dmi = new DalOdbcMgr();
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowlist_t *val = reinterpret_cast<val_flowlist_t *>
        (malloc(sizeof(val_flowlist_t)));
 memset(val, 0, sizeof(val_flowlist_t));

 val->cs_row_status = UNC_CS_NOT_APPLIED;
 val->valid[0] = UNC_VF_VALID;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp);
  val_flowlist_ctrl_t *val_ctrl = reinterpret_cast<val_flowlist_ctrl_t *>
        (malloc(sizeof(val_flowlist_ctrl_t)));
 memset(val_ctrl, 0, sizeof(val_flowlist_ctrl_t));
 val_ctrl->valid[0] = UNC_VF_NOT_SUPPORTED;
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val_ctrl);
 ctrlr_key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp1);

 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_UPDATE, 0, upd_key, dmi, ctrlr_key));
 //val_flowlist_t *output = reinterpret_cast<val_flowlist_t *> (GetVal(key));
  //EXPECT_EQ(UNC_CS_UNKNOWN,output->cs_row_status);
delete key; delete ctrlr_key;
}

TEST(FlowListMoMgrUt, UpdateConfigStatus_SuccessOpUpdate1) {
 FlowListMoMgr obj;
 DalDmlIntf *dmi = new DalOdbcMgr();
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowlist_t *val = reinterpret_cast<val_flowlist_t *>
        (malloc(sizeof(val_flowlist_t)));
 memset(val, 0, sizeof(val_flowlist_t));

 val->cs_row_status = UNC_CS_NOT_APPLIED;
 val->valid[0] = UNC_VF_VALID;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp);
  val_flowlist_ctrl_t *val_ctrl = reinterpret_cast<val_flowlist_ctrl_t *>
        (malloc(sizeof(val_flowlist_ctrl_t)));
 memset(val_ctrl, 0, sizeof(val_flowlist_ctrl_t));
 val_ctrl->valid[0] = UNC_VF_VALID;
  ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val_ctrl);
 ctrlr_key = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            NULL, tmp1);

 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_UPDATE, 0, upd_key, dmi, ctrlr_key));
 //val_flowlist_t *output = reinterpret_cast<val_flowlist_t *> (GetVal(key));
  //EXPECT_EQ(UNC_CS_UNKNOWN,output->cs_row_status);
delete key; delete ctrlr_key;
}


/*======================CopyToConfigKey=========================*/
TEST(FlowlistMoMgr, CopyToConfigKey_ikeyNULL) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey, NULL));
   if (okey) delete okey;
}

TEST(FlowlistMoMgr, CopyToConfigKey_KTNULL) {
  FlowListMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey, ikey));
  delete ikey; if (okey) delete okey;
}

TEST(FlowlistMoMgr, CopyToConfigKey_InvalidKT) {
  FlowListMoMgr obj;
  key_flowlist_t *key_flowlist = new key_flowlist_t;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlist, key_flowlist);
  ConfigKeyVal *okey = NULL; 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey, ikey));
  delete ikey; if (okey) delete okey;
}

/*===========================SwapKeyVal===================================*/
TEST(FlowlistMoMgr, SwapKeyValikeyNULL) {
FlowListMoMgr obj;
DalDmlIntf *dmi = new DalOdbcMgr();
ConfigKeyVal *ikey = NULL;
//key_flowlist_t *key_flowlist = new key_flowlist_t;
ConfigKeyVal *okey = NULL;
bool no_rename;
uint8_t ctrlr[32] = "pfc1";
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.SwapKeyVal(ikey, okey,
                                                  dmi, ctrlr, no_rename));
if(okey) delete okey; 
}

TEST(FlowlistMoMgr, SwapKeyVal_InvalidkeyType) {
FlowListMoMgr obj;
DalDmlIntf *dmi = new DalOdbcMgr();
key_flowlist_t *ikey_flowlist = new key_flowlist_t;
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, ikey_flowlist);
ConfigKeyVal *okey = NULL;
bool no_rename;
uint8_t ctrlr[32] = "pfc1";
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.SwapKeyVal(ikey, okey, dmi, ctrlr, no_rename));
delete ikey;  delete dmi;
if(okey)delete okey; 
}

TEST(FlowlistMoMgr, SwapKeyVal_key_NULL) {
FlowListMoMgr obj;
DalDmlIntf *dmi = new DalOdbcMgr();
//key_flowlist_t *ikey_flowlist = new key_flowlist_t;
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlist, NULL);
ConfigKeyVal *okey = NULL;
bool no_rename;
uint8_t ctrlr[32] = "pfc1";
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.SwapKeyVal(ikey, okey, dmi, ctrlr, no_rename));
delete ikey; 
if(okey)delete okey; delete dmi;
}
TEST(FlowlistMoMgr, SwapKeyVal_Configval_NULL) {
FlowListMoMgr obj;
DalDmlIntf *dmi = new DalOdbcMgr();
key_flowlist_t *ikey_flowlist = new key_flowlist_t;
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY, IpctSt::kIpcStKeyFlowlist, ikey_flowlist);
ConfigKeyVal *okey = NULL; 
bool no_rename;
uint8_t ctrlr[32] = "pfc1";
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.SwapKeyVal(ikey, okey, dmi, ctrlr, no_rename));
delete ikey;  delete dmi;
if(okey)delete okey; 
}

TEST(FlowlistMoMgr, SwapKeyVal_namemismatch) {
FlowListMoMgr obj;
ConfigKeyVal *key = NULL;
bool no_rename;
uint8_t ctrlr[32] = "pfc1";
DalDmlIntf *dmi = new DalOdbcMgr();
val_rename_flowlist_t *tval =
      reinterpret_cast<val_rename_flowlist_t *>(malloc(sizeof(val_rename_flowlist_t)));
memset(tval, 0, sizeof(val_rename_flowlist_t));
memcpy(tval->flowlist_newname, "FLOWLIST",32);
key_flowlist_t *key_flowlist = new key_flowlist_t;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist,tval);
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, obj.SwapKeyVal(ikey, key, dmi, ctrlr, no_rename));
delete ikey; if(key) delete key; delete dmi;
}

TEST(FlowlistMoMgr, SwapKeyVal_namematch) {
FlowListMoMgr obj;
ConfigKeyVal *key = NULL;
bool no_rename;
uint8_t ctrlr[32] = "pfc1";
DalDmlIntf *dmi = new DalOdbcMgr();
val_rename_flowlist_t *tval =
      reinterpret_cast<val_rename_flowlist_t *>(malloc(sizeof(val_rename_flowlist_t)));
memset(tval, 0, sizeof(val_rename_flowlist_t));
memcpy(tval->flowlist_newname, "FLOWLIST",32);
tval->valid[0] = UNC_VF_VALID;
key_flowlist_t *key_flowlist = new key_flowlist_t;
memset(key_flowlist, 0, sizeof(key_flowlist_t));
memcpy(key_flowlist->flowlist_name, "FLOWLIST",32);

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist,tval);
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SwapKeyVal(ikey, key, dmi, ctrlr, no_rename));
delete ikey; if(key) delete key; delete dmi;
}

TEST(FlowlistMoMgr, SwapKeyVal_strlen_zero) {
FlowListMoMgr obj;
ConfigKeyVal *key = NULL;
bool no_rename;
uint8_t ctrlr[32] = "pfc1";
DalDmlIntf *dmi = new DalOdbcMgr();
val_rename_flowlist_t *tval =
      reinterpret_cast<val_rename_flowlist_t *>(malloc(sizeof(val_rename_flowlist_t)));
memset(tval, 0, sizeof(val_rename_flowlist_t));
tval->valid[0] = UNC_VF_VALID;
key_flowlist_t *key_flowlist = new key_flowlist_t;
memset(key_flowlist, 0, sizeof(key_flowlist_t));

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist,tval);
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.SwapKeyVal(ikey, key, dmi, ctrlr, no_rename));
delete ikey; if(key) delete key; delete dmi;
}

/*===========================IsReferenced====================================*/
#if 0
invalid testcases, no chance of NULL for ikey/dmi
TEST(FlowlistMoMgr,IsReferencedikeyNULL) {
FlowListMoMgr obj;
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.IsReferenced(NULL, UPLL_DT_CANDIDATE, NULL));
}
TEST(FlowlistMoMgr,IsReferencedikeyValid) {
FlowListMoMgr obj;
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(ikey, UPLL_DT_CANDIDATE, NULL));
delete ikey;
}
#endif
TEST(FlowlistMoMgr,IsReferencedikeySuccess) {
FlowListMoMgr obj;
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST_NEW" , kMaxLenFlowListName);
DalOdbcMgr::stub_setSingleRecordExists(true);;
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);

ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,IpctSt::kIpcStKeyFlowlist, key_flowlist);
DalDmlIntf *dmi = new DalOdbcMgr();

IPC_REQ_RESP_HEADER_DECL(req);
req->datatype = UPLL_DT_CANDIDATE;
EXPECT_EQ(UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID, obj.IsReferenced(req, ikey, dmi));
DalOdbcMgr::clearStubData();

delete ikey;delete dmi;
}
/*=====================GetRenamedControllerKey===================================*/
TEST(FlowlistMoMgr, GetRenamedControllerKey) {
FlowListMoMgr obj;
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE, NULL, NULL));
if(ikey) delete ikey;
} 

/*==============================GetRenameInfo=========================================*/
TEST(FlowlistMoMgr,GetRenameInfo_NULL) {
FlowListMoMgr obj;
bool flag;
ConfigKeyVal *rename_info = NULL;
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenameInfo(NULL, NULL, rename_info, NULL, NULL, flag));
}

TEST(FlowlistMoMgr,GetRenameInfo_ikey_keystruct_null) {
FlowListMoMgr obj;
bool flag;
ConfigKeyVal *rename_info =  new ConfigKeyVal(UNC_KT_FLOWLIST);
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST);
ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_FLOWLIST);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenameInfo(ikey, okey, rename_info, NULL, NULL, flag));
delete ikey; delete okey; delete rename_info;
}

TEST(FlowlistMoMgr,GetRenameInfo_okey_keystruct_null) {
FlowListMoMgr obj;
bool flag;
ConfigKeyVal *rename_info =  new ConfigKeyVal(UNC_KT_FLOWLIST);
key_flowlist_t *key_flowlist = new key_flowlist_t;
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist);
ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_FLOWLIST);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenameInfo(ikey, okey, rename_info, NULL, NULL, flag));
delete ikey; delete okey; delete rename_info;
}

TEST(FlowlistMoMgr,GetRenameInfo_okey_field_strlen_null) {
FlowListMoMgr obj;
bool flag;
ConfigKeyVal *rename_info =  new ConfigKeyVal(UNC_KT_FLOWLIST);
key_flowlist_t *key_flowlist = new key_flowlist_t;
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist);
key_flowlist_t *key_flowlist1 = new key_flowlist_t;
ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist1);

EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenameInfo(ikey, okey, rename_info, NULL, NULL, flag));
delete ikey; delete okey; delete rename_info;
}

TEST(FlowlistMoMgr,GetRenameInfo_okey_field_renamed_false) {
FlowListMoMgr obj;
bool flag = false;
ConfigKeyVal *rename_info =  new ConfigKeyVal(UNC_KT_FLOWLIST);
key_flowlist_t *key_flowlist = new key_flowlist_t;
memset(key_flowlist->flowlist_name, 0, 32);
memcpy(key_flowlist->flowlist_name, "FLOWLIST", 32);
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist);
char ctrl[] ={'p','f','c'};
key_flowlist_t *key_flowlist1 = new key_flowlist_t;
memset(key_flowlist1->flowlist_name, 0, 32);
memcpy(key_flowlist1->flowlist_name, "FLOWLIST", 32);
ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist1);
DalDmlIntf *dmi = new DalOdbcMgr();
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT, kDalRcSuccess);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenameInfo(ikey, okey, rename_info, dmi, ctrl, flag));
delete ikey; delete okey; 
DalOdbcMgr::clearStubData();

}
TEST(FlowlistMoMgr,GetRenameInfo_okey_field_renamed_true) {
FlowListMoMgr obj;
bool flag = true;
ConfigKeyVal *rename_info =  new ConfigKeyVal(UNC_KT_FLOWLIST);
key_flowlist_t *key_flowlist = new key_flowlist_t;
memset(key_flowlist->flowlist_name, 0, 32);
memcpy(key_flowlist->flowlist_name, "FLOWLIST", 32);
ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist);
key_flowlist_t *key_flowlist1 = new key_flowlist_t;
memset(key_flowlist1->flowlist_name, 0, 32);
memcpy(key_flowlist1->flowlist_name, "FLOWLIST", 32);
ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist1);

EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetRenameInfo(ikey, okey, rename_info, NULL, NULL, flag));
delete ikey; delete okey; delete rename_info;
}

/*=============================UpdateMo===============================*/
TEST(FlowlistMoMgr, UpdateMoSuccess) {
 FlowListMoMgr obj;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateMo(NULL,NULL,NULL));
}

/*============================UpdateAuditConfigStatus=========================================*/
TEST(FlowlistMoMgr, UpdateAuditConfigStatus_ckv_runningNULL) {
FlowListMoMgr obj;
ConfigKeyVal *ikey =NULL;
DalDmlIntf *dmi= new DalOdbcMgr();
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey, dmi));
 val_flowlist_t *val = reinterpret_cast<val_flowlist_t *>
        (malloc(sizeof(val_flowlist_t)));
  memset(val, 0, sizeof(val_flowlist_t));

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val);

  ikey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey, dmi));
  val_flowlist_t *output = reinterpret_cast<val_flowlist_t *> (GetVal(ikey));
  EXPECT_EQ(UNC_CS_UNKNOWN,output->cs_row_status);
  delete ikey; delete dmi;
}

/*============================MergeValidate=============================*/
/** MergeValidate testcases are handled in flowlist_import_ut */
TEST(FlowListMoMgrUt, MergeValidate_ctrlidNULL) {
 FlowListMoMgr flowlist_obj;
ConfigKeyVal *key=new ConfigKeyVal(UNC_KT_FLOWLIST);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.MergeValidate(UNC_KT_FLOWLIST, NULL, key, NULL, UPLL_IMPORT_TYPE_FULL));
delete key;
}

TEST(FlowListMoMgrUt, MergeValidate_Success) {
 FlowListMoMgr flowlist_obj;
ConfigKeyVal *key=new ConfigKeyVal(UNC_KT_FLOWLIST);
DalDmlIntf *dmi = new DalOdbcMgr();
EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.MergeValidate(UNC_KT_FLOWLIST, "UNC", key, dmi, UPLL_IMPORT_TYPE_FULL));
delete key;
}

/*============================GetRenamedUncKey==================================*/
TEST(FlowListMoMgrUt, GetRenamedUncKey_keyStructIn_ctrlr_keyNULL) {
 FlowListMoMgr flowlist_obj;
  ConfigKeyVal *key=new ConfigKeyVal(UNC_KT_FLOWLIST);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.GetRenamedUncKey(key,UPLL_DT_CANDIDATE,NULL,NULL));
delete key;
}

/*==============================GetParentConfigKey=============================*/
TEST(FlowListMoMgrUt,GetParentConfigKey_Success) {
    FlowListMoMgr flowlist_obj;
   ConfigKeyVal *key = NULL;
   EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.GetParentConfigKey(key,NULL));
}
/*==================================CompareValidValue============================*/
TEST(FlowListMoMgrUt,CompareValidValue_auditTrue) {
 FlowListMoMgr flowlist_obj;
 val_flowlist_t *flowlist_val1 =
    reinterpret_cast<val_flowlist_t *>(malloc(sizeof(val_flowlist_t)));
 flowlist_val1->valid[0] = UNC_VF_INVALID;
 val_flowlist_t *flowlist_val2 =
    reinterpret_cast<val_flowlist_t *>(malloc(sizeof(val_flowlist_t)));
 flowlist_val2->valid[0] = UNC_VF_VALID;
void *v1 = reinterpret_cast<void *>(flowlist_val1);
void *v2 = reinterpret_cast<void *>(flowlist_val2);

flowlist_obj.CompareValidValue(v1, v2, true); 
  EXPECT_EQ(UNC_VF_VALID_NO_VALUE, flowlist_val1->valid[0]);
free(flowlist_val1); free(flowlist_val2);
}

TEST(FlowListMoMgrUt,CompareValidValue_auditfalse) {
 FlowListMoMgr flowlist_obj;
 val_flowlist_t *flowlist_val1 =
    reinterpret_cast<val_flowlist_t *>(malloc(sizeof(val_flowlist_t)));
 memset(flowlist_val1, 0, sizeof(val_flowlist_t));
 flowlist_val1->ip_type = UPLL_FLOWLIST_TYPE_IP;
 flowlist_val1->valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_VALID;
 val_flowlist_t *flowlist_val2 =
    reinterpret_cast<val_flowlist_t *>(malloc(sizeof(val_flowlist_t)));
 memset(flowlist_val2, 0, sizeof(val_flowlist_t));
 flowlist_val2->ip_type = UPLL_FLOWLIST_TYPE_IP;
 flowlist_val2->valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_VALID;
void *v1 = reinterpret_cast<void *>(flowlist_val1);
void *v2 = reinterpret_cast<void *>(flowlist_val2);

flowlist_obj.CompareValidValue(v1, v2, false);
 
  EXPECT_EQ(UNC_VF_INVALID, flowlist_val1->valid[UPLL_IDX_IP_TYPE_FL]);
free(flowlist_val1); free(flowlist_val2);
}

/*============================GetRenameKeyBindInfo=====================*/
TEST(FlowListMoMgrUt, GetRenameKeyBindInfo_InvalidKT) {
 FlowListMoMgr flowlist_obj;
 BindInfo *info = NULL;
  int nattrs;
EXPECT_EQ(PFC_TRUE,  flowlist_obj.GetRenameKeyBindInfo(UNC_KT_FLOWLIST_ENTRY, info, nattrs, MAINTBL));
}

TEST(FlowListMoMgrUt, GetRenameKeyBindInfo_SuccessMAINTBL) {
 FlowListMoMgr flowlist_obj;
 BindInfo *info = NULL;
  int nattrs;
EXPECT_EQ(PFC_TRUE,  flowlist_obj.GetRenameKeyBindInfo(UNC_KT_FLOWLIST, info, nattrs, MAINTBL));
EXPECT_EQ(NUM_KEY_MAIN_COL, nattrs);
}

TEST(FlowListMoMgrUt, GetRenameKeyBindInfo_SuccessCTRLRTBL) {
 FlowListMoMgr flowlist_obj;
 BindInfo *info = NULL;
  int nattrs;
EXPECT_EQ(PFC_TRUE,  flowlist_obj.GetRenameKeyBindInfo(UNC_KT_FLOWLIST, info, nattrs, CTRLRTBL));
EXPECT_EQ(NUM_KEY_CTRL_COL, nattrs);
}

TEST(FlowListMoMgrUt, GetRenameKeyBindInfo_SuccessRENAMETBL) {
 FlowListMoMgr flowlist_obj;
 BindInfo *info = NULL;
  int nattrs;
EXPECT_EQ(PFC_TRUE,  flowlist_obj.GetRenameKeyBindInfo(UNC_KT_FLOWLIST, info, nattrs, RENAMETBL));
EXPECT_EQ(NUM_KEY_RENAME_COL, nattrs);
}

/*================================GetValid========================================================*/
TEST(FlowListMoMgrUt, GetValid_valNULL) {
FlowListMoMgr flowlist_obj;
uint8_t *valid = NULL;
EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.GetValid((void*)valid, (uint64_t )1, valid, UPLL_DT_CANDIDATE, MAINTBL));
}

TEST(FlowListMoMgrUt, GetValid_MAINTBLSuccess) {
FlowListMoMgr flowlist_obj;
uint8_t *valid = NULL;
val_flowlist_t *val_flowlist = new val_flowlist_t;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.GetValid((void*)val_flowlist, (uint64_t )1, valid, UPLL_DT_CANDIDATE, MAINTBL));
delete val_flowlist;
}

TEST(FlowListMoMgrUt, GetValid_CTRLRTBLSuccess) {
FlowListMoMgr flowlist_obj;
uint8_t *valid = NULL;
val_flowlist_ctrl *flowlist_ctrl = new val_flowlist_ctrl;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.GetValid((void*)flowlist_ctrl, (uint64_t )1, valid, UPLL_DT_CANDIDATE, CTRLRTBL));
delete flowlist_ctrl;
}

TEST(FlowListMoMgrUt, GetValid_RENAMETBLSuccess) {
FlowListMoMgr flowlist_obj;
uint8_t *valid = NULL;
val_flowlist_t *val_flowlist = new val_flowlist_t;
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.GetValid((void*)val_flowlist, (uint64_t )0, valid, UPLL_DT_CANDIDATE, RENAMETBL));
delete val_flowlist;
}

TEST(FlowListMoMgrUt, GetValid_Error) {
FlowListMoMgr flowlist_obj;
uint8_t *valid = NULL;
val_flowlist_t *val_flowlist = new val_flowlist_t;
EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.GetValid((void*)val_flowlist, (uint64_t )1, valid, UPLL_DT_CANDIDATE, MAX_MOMGR_TBLS));
delete val_flowlist;
}

/*------------------------------**CompareKey****--------------*/
TEST(FlowListMoMgrUt, CompareKey_key1key2NULL) {
FlowListMoMgr flowlist_obj;
EXPECT_EQ(true, flowlist_obj.CompareKey(NULL,NULL));
}

TEST(FlowListMoMgrUt, CompareKey_key1validkey2NULL) {
FlowListMoMgr flowlist_obj;
key_flowlist_t *flowlist_key1 = new key_flowlist_t;
EXPECT_EQ(false, flowlist_obj.CompareKey(flowlist_key1, NULL));
delete flowlist_key1;
}

TEST(FlowListMoMgrUt, CompareKey_keyMisMatch) {
FlowListMoMgr flowlist_obj;
key_flowlist_t *flowlist_key1 = new key_flowlist_t;
strncpy(reinterpret_cast<char *>(flowlist_key1->flowlist_name),
  "FLOWLIST",kMaxLenFlowListName);
key_flowlist_t *flowlist_key2 = new key_flowlist_t;
strncpy(reinterpret_cast<char *>(flowlist_key2->flowlist_name),
  "FLOWLIST1",kMaxLenFlowListName);

EXPECT_EQ(false, flowlist_obj.CompareKey(flowlist_key1, flowlist_key2));
delete flowlist_key1; delete flowlist_key2;
}

TEST(FlowListMoMgrUt, CompareKey_keyMatch) {
FlowListMoMgr flowlist_obj;
key_flowlist_t *flowlist_key1 = new key_flowlist_t;
strncpy(reinterpret_cast<char *>(flowlist_key1->flowlist_name),
  "FLOWLIST",kMaxLenFlowListName);
key_flowlist_t *flowlist_key2 = new key_flowlist_t;
strncpy(reinterpret_cast<char *>(flowlist_key2->flowlist_name),
  "FLOWLIST",kMaxLenFlowListName);

EXPECT_EQ(true, flowlist_obj.CompareKey(flowlist_key1, flowlist_key2));
delete flowlist_key1; delete flowlist_key2;
}


/*** ValidateMessage UT starts ***************************/

TEST(FlowListMoMgrUt,ValidateMsg1_Validreq_NULLkey) {
FlowListMoMgr flowlist_obj;
ConfigKeyVal *ikey = NULL;
IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, ikey));
free(req);
}

TEST(FlowListMoMgrUt,ValidateMsg2_InvalidKeytype) {
FlowListMoMgr flowlist_obj;
/** set Invalid keytype */
ConfigKeyVal ikey(UNC_KT_FLOWLIST_ENTRY);
IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, &ikey));
free(req);
}


TEST(FlowListMoMgrUt,ValidateMsg4_NullKeyStruct) {
FlowListMoMgr flowlist_obj;
/** set valid keytype, st_num */
ConfigKeyVal ikey(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist);
IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
/** key struct is NULL */
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, &ikey));
free(req);
}

TEST(FlowListMoMgrUt,ValidateMsg4_Invalidoption1) {
  FlowListMoMgr flowlist_obj;
  /** set valid keytype, st_num */
  ConfigKeyVal ikey(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
        malloc(sizeof(IpcReqRespHeader)));
  req->option1 = UNC_OPT1_DETAIL;
  /** key struct is NULL */
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, flowlist_obj.ValidateMessage(req, &ikey));
  free(req);

}

/** valid value structu, invalid st_num */
TEST(FlowListMoMgrUt,ValidateMsg5_InvalidValStNum) {
FlowListMoMgr flowlist_obj;
/** set valid keytype */
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
  malloc(sizeof(val_flowlist_t)));
memset(val_flowlist, 0, sizeof(val_flowlist_t));
ConfigVal *config_val= new ConfigVal(IpctSt::kIpcInvalidStNum, val_flowlist);
ConfigKeyVal ikey(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);

/** default value for st_num is kIpcInvalidStNum*/
IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));
req->operation = UNC_OP_CREATE;
req->datatype = UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, &ikey));
free(req);  

}

/**TODO have to recheck this output */
TEST(FlowListMoMgrUt,ValidateMsg6_NoValueInReqKey) {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
/** set valid keytype, st_num, key-struct */
ConfigKeyVal ikey(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist);
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateMessage(req, &ikey));
 free(req);
}

TEST(FlowListMoMgrUt,ValidateMsg7_ValidKeyStruct_ValRenameNull) {
FlowListMoMgr flowlist_obj;
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_RENAME;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

/** set valid keytype, st_num, key-struct */
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, NULL);

/** Rename value struct and its st_num is not filled*/
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, ikey));
 free(req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateMsg7_ValidKeyStruct_Invalidkeyname) {
FlowListMoMgr flowlist_obj;
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "_$FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_RENAME;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

/** set valid keytype, st_num, key-struct */
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, NULL);
IpctSt::RegisterAll();
/** Rename value struct and its st_num is not filled*/
EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateMsg8_RENAMEwithInvalidDT) {
FlowListMoMgr flowlist_obj;
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_RENAME;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
val_rename_flowlist_t *val_rename_flowlist = reinterpret_cast<val_rename_flowlist_t*>(
  malloc(sizeof(val_rename_flowlist_t)));
memset(val_rename_flowlist, 0, sizeof(val_rename_flowlist_t));
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, val_rename_flowlist);

/** set valid keytype, st_num, key-struct */
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);

/** Rename value struct and its st_num is not filled*/
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, ikey));
 free(req); delete ikey;
}
TEST(FlowListMoMgrUt,ValidateMsg8_RENAMEwithInvalidStnum) {
FlowListMoMgr flowlist_obj;
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_RENAME;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
val_rename_flowlist_t *val_rename_flowlist = reinterpret_cast<val_rename_flowlist_t*>(
  malloc(sizeof(val_rename_flowlist_t)));
memset(val_rename_flowlist, 0, sizeof(val_rename_flowlist_t));
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_rename_flowlist);

/** set valid keytype, st_num, key-struct */
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);

/** Rename value struct and its st_num is not filled*/
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, ikey));
 free(req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateMsg8_RENAMEwithvalstructNone) {
FlowListMoMgr flowlist_obj;
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_RENAME;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist);

/** set valid keytype, st_num, key-struct */
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);

/** Rename value struct and its st_num is not filled*/
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, ikey));
 free(req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateMsg8_RENAMEwithnoNameinValSt) {
FlowListMoMgr flowlist_obj;
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_RENAME;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
val_rename_flowlist_t *val_rename_flowlist = reinterpret_cast<val_rename_flowlist_t*>(
  malloc(sizeof(val_rename_flowlist_t)));
memset(val_rename_flowlist, 0, sizeof(val_rename_flowlist_t));
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, val_rename_flowlist);

/** set valid keytype, st_num, key-struct */
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);

/** Rename value struct and its st_num is not filled*/
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
 free(req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateMsg9_RENAMEValidFlagNotFilled) {
FlowListMoMgr flowlist_obj;
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_RENAME;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_rename_flowlist_t *val_rename_flowlist = reinterpret_cast<val_rename_flowlist_t*>(
  malloc(sizeof(val_rename_flowlist_t)));
memset(val_rename_flowlist, 0, sizeof(val_rename_flowlist_t));
val_rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_INVALID;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, val_rename_flowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey; 

}


TEST(FlowListMoMgrUt,ValidateMsg10_RENAMESuccess) {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_RENAME;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_rename_flowlist_t *val_rename_flowlist = reinterpret_cast<val_rename_flowlist_t*>(
  malloc(sizeof(val_rename_flowlist_t)));
memset(val_rename_flowlist, 0, sizeof(val_rename_flowlist_t));

val_rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL]
      = UNC_VF_VALID;
strncpy(reinterpret_cast<char*>(val_rename_flowlist->flowlist_newname),
  "FLOWLIST" , kMaxLenFlowListName);

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, val_rename_flowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey; 

}
TEST(FlowListMoMgrUt,ValidateMsg10_RENAMESuccess1) {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_RENAME;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_rename_flowlist_t *val_rename_flowlist = reinterpret_cast<val_rename_flowlist_t*>(
  malloc(sizeof(val_rename_flowlist_t)));
memset(val_rename_flowlist, 0, sizeof(val_rename_flowlist_t));

val_rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL]
      = UNC_VF_VALID_NO_VALUE;
strncpy(reinterpret_cast<char*>(val_rename_flowlist->flowlist_newname),
  "FLOWLIST" , kMaxLenFlowListName);

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, val_rename_flowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey;

}

TEST(FlowListMoMgrUt,ValidateMsg11_CREATEInvalidDT) {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist);
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey; 
}

TEST(FlowListMoMgrUt,ValidateMsg12_CREATEValidDTValStructNULL) {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist);
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, flowlist_obj.ValidateMessage(req, ikey));
 free(req); delete ikey; 
}

TEST(FlowListMoMgrUt,ValidateMsg13_CREATESuccess) {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
  malloc(sizeof(val_flowlist_t)));

val_flowlist->ip_type = UPLL_FLOWLIST_TYPE_IPV6;
val_flowlist->valid[UPLL_IDX_IP_TYPE_FL]
      = UNC_VF_VALID;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateMsg14_READ_DT_STATE) {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
 free(req); delete ikey; 
}

TEST(FlowListMoMgrUt,ValidateMsg14_READ_Unsupported_dttype) {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_READ_SIBLING_COUNT;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
 free(req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateMsg15_READ_IMPORT)  {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);
val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(val_flowlist, 0, sizeof(val_flowlist_t));
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_DETAIL;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist,config_val);
EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey; 
}

TEST(FlowListMoMgrUt,ValidateMsg_InvalidOPTION2) {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_READ_NEXT;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_L2DOMAIN;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist);
EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey; 
}

TEST(FlowListMoMgrUt,ValidateMsg17_READSuccess)  {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
  malloc(sizeof(val_flowlist_t)));

val_flowlist->ip_type = UPLL_FLOWLIST_TYPE_IPV6;
val_flowlist->valid[UPLL_IDX_IP_TYPE_FL]
      = UNC_VF_VALID;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateMsg18_READSIBLING_IMPORT)  {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_READ_SIBLING;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey;
}


TEST(FlowListMoMgrUt,ValidateMsg20_InvalidOperation)  {
FlowListMoMgr flowlist_obj;

key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST" , kMaxLenFlowListName);

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_INVALID;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;


ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist);
EXPECT_EQ(UPLL_RC_SUCCESS, flowlist_obj.ValidateMessage(req, ikey));
free(req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateCapa1_reqNULL) {
FlowListMoMgr flowlist_obj;

EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.ValidateCapability(NULL,NULL));
}

TEST(FlowListMoMgrUt,ValidateCapa2_keyNULL) {
FlowListMoMgr flowlist_obj;

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.ValidateCapability(req,NULL));
free(req);
}

TEST(FlowListMoMgrUt,ValidateCapa3_reqkey_Empty) {
FlowListMoMgr flowlist_obj;

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.ValidateCapability(req, ikey));
free(req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateCapa4_max_attrs_check) {
FlowListMoMgr flowlist_obj;

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));
const char * ctrlr_name = "cntrlr_name";                                          
 const char* version("version");                                                 
CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);                              
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));              
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);                               
                                                                                  
                                                                                 
 CapaModuleStub::stub_loadCapaModule();                                           
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
  malloc(sizeof(val_flowlist_t)));
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, NULL,
config_val);
EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
CapaModuleStub::stub_clearStubData();
}


TEST(FlowListMoMgrUt,ValidateCapa_READ_max_attrs_check) {
FlowListMoMgr flowlist_obj;

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
const char * ctrlr_name = "cntrlr_name";
 const char* version("version");

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 CapaModuleStub::stub_loadCapaModule();
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);


val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
  malloc(sizeof(val_flowlist_t)));
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, NULL,
config_val);
EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
CapaModuleStub::stub_clearStubData();
free (req); delete ikey;
}

TEST(FlowListMoMgrUt,ValidateCapa_READ_valstruct_NULL) {
FlowListMoMgr flowlist_obj;

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_READ;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
const char * ctrlr_name = "cntrlr_name";
 const char* version("version");

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 CapaModuleStub::stub_loadCapaModule();
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
val_flowlist_t *val_flowlist = new val_flowlist_t;
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, NULL,
config_val);
EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
CapaModuleStub::stub_clearStubData();
free (req); delete ikey; //delete ctrl1; 
}

TEST(FlowListMoMgrUt,ValidateCapa_READSIBLING_valstruct_NULL) {
FlowListMoMgr flowlist_obj;

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_READ_SIBLING;
req->datatype =  UPLL_DT_STATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
const char * ctrlr_name = "cntrlr_name";
 const char* version("version");

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 CapaModuleStub::stub_loadCapaModule();
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, NULL,
config_val);
EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
CapaModuleStub::stub_clearStubData();
free (req); delete ikey; //delete ctrl1;
}

TEST(FlowListMoMgrUt,ValidateCapa_DELETE) {
FlowListMoMgr flowlist_obj;

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_DELETE;
const char * ctrlr_name = "cntrlr_name";
 const char* version("version");

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 CapaModuleStub::stub_loadCapaModule();
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, NULL,
config_val);
EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
free (req); delete ikey; //delete ctrl1;
CapaModuleStub::stub_clearStubData();
}

TEST(FlowListMoMgrUt,ValidateCapa_InvalidOperation) {
FlowListMoMgr flowlist_obj;

IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));

req->operation = UNC_OP_RENAME;
req->datatype = UPLL_DT_CANDIDATE;
const char * ctrlr_name = "cntrlr_name";
 const char* version("version");

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 CapaModuleStub::stub_loadCapaModule();
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist);
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, NULL,
config_val);
EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, flowlist_obj.ValidateCapability(req, ikey, ctrlr_name));
free (req); delete ikey; ////delete ctrl1;
CapaModuleStub::stub_clearStubData();
}
/*
TEST(FlowListMoMgrUt, SupportCheckSuccess) {
FlowListMoMgr obj;
val_flowlist_t *val_flowlist = static_cast<val_flowlist_t*>(malloc
  (sizeof(val_flowlist_t)));
uint8_t attrs[0];
attrs[0] = 1;
val_flowlist->valid[0] = UNC_VF_VALID;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistAttributeSupportCheck(val_flowlist, attrs));
free (val_flowlist);
}

TEST(FlowListMoMgrUt, SupportCheckFailure) {
FlowListMoMgr obj;
val_flowlist_t *val_flowlist = static_cast<val_flowlist_t*>(malloc
  (sizeof(val_flowlist_t)));
uint8_t attrs[0];
attrs[0] = 0;
val_flowlist->valid[0] = UNC_VF_VALID_NO_VALUE;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValFlowlistAttributeSupportCheck(val_flowlist, attrs));
EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val_flowlist->valid[0]);
free (val_flowlist);
}
*/
/**
 * Invalid testcase, as NULL check for valstruct taken care in code before
 * invoking this function */
#if 0
TEST(FlowListMoMgrUt, SupportCheckNoValStruct) {
FlowListMoMgr obj;
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValFlowlistAttributeSupportCheck(NULL, NULL));
}
#endif
#if 0
TEST(FlowlistMoMgr,TxUpdateProcess_Success) {
FlowListMoMgr obj;
set<string> affected_ctrlr_set;
bool driver_resp = false;
key_flowlist_t *key_flowlist =
    reinterpret_cast<key_flowlist_t *>(malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist));
memcpy(key_flowlist->flowlist_name, "FLOWLIST", kMaxLenFlowListName);
ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist, key_flowlist);
key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
memset(user_data, 0, sizeof(key_user_data_t));
user_data->flags = 1;
key->set_user_data((void*)user_data);

DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE, kDalRcSuccess);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);

DalDmlIntf *dmi = new DalOdbcMgr();
IpcResponse *req= reinterpret_cast<IpcResponse*>(malloc(sizeof(IpcResponse)));
const char * ctrlr_name = "PFC";
 const char* version("version");

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
controller_domain *dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(
  controller_domain_t)));
memset(dom, 0, sizeof(dom));
uint8_t name[3] = {'P','F','C'};
dom->ctrlr = name;

EXPECT_EQ(UPLL_RC_SUCCESS, obj.TxUpdateProcess(key, req, UNC_OP_CREATE, dmi, dom, &affected_ctrlr_set, &driver_resp));
DalOdbcMgr::clearStubData();
delete dmi; delete key;
}
#endif
/*=========================ReadRecord===========================*/
TEST(FlowListMoMgrUt,ReadRecord_req_ikey_NULL) {
FlowListMoMgr obj;
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ReadRecord(NULL,NULL, NULL));
}


TEST(FlowListMoMgrUt,ReadRecord_CANDIDATE_Success1) {
FlowListMoMgr obj;
UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST_NEW1" , kMaxLenFlowListName);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);

req->operation = UNC_OP_CREATE;
req->datatype =  UPLL_DT_CANDIDATE;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
DalOdbcMgr::stub_setSingleRecordExists(true);;
val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
  malloc(sizeof(val_flowlist_t)));
for(uint8_t i =0; i<sizeof(val_flowlist->valid); i++) {
  val_flowlist->valid[i] = UNC_VF_VALID_NO_VALUE;
}
ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
DalDmlIntf *dmi = new DalOdbcMgr();
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadRecord(req, ikey,dmi));
DalOdbcMgr::clearStubData();
delete dmi; free(req); delete ikey; delete upll_obj;
}
#if 0
/* this function is not returning SUCCESS as DB is not filling Rename struct in GetRenamedControllerKey*/
ReadRecord is a dead code no need to check
TEST(FlowListMoMgrUt,ReadRecord_IMPORT_Success1) {
FlowListMoMgr obj;
UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
  malloc(sizeof(key_flowlist_t)));
memset(key_flowlist, 0, sizeof(key_flowlist_t));
strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
  "FLOWLIST_NEW1" , kMaxLenFlowListName);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(
  malloc(sizeof(IpcReqRespHeader)));
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);

req->operation = UNC_OP_RENAME;
req->datatype =  UPLL_DT_IMPORT;
req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
DalOdbcMgr::stub_setSingleRecordExists(true);;
val_rename_flowlist_t *rename_val = NULL;
rename_val = reinterpret_cast<val_rename_flowlist_t *> (malloc(sizeof(val_flowlist_t)));
for(int i =0; i<sizeof(rename_val->valid); i++) {
  rename_val->valid[i] = UNC_VF_VALID;
}
strncpy(reinterpret_cast<char*>(rename_val->flowlist_newname),
  "FLOWLIST1" , kMaxLenFlowListName);

ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, rename_val);

ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST,
  IpctSt::kIpcStKeyFlowlist, key_flowlist, config_val);
key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
memset(user_data, 0, sizeof(key_user_data_t));
user_data->flags = 1;
ikey->set_user_data((void*)user_data);
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
DalDmlIntf *dmi = new DalOdbcMgr();
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadRecord(req, ikey,dmi));
DalOdbcMgr::clearStubData();
delete dmi;free(req);
}
#endif

TEST(FlowListMoMgrUt, ValidateAttributeSuccess) {
FlowListMoMgr obj;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST);
EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey, NULL));
delete ikey;
}

TEST(FlowListMoMgrUt, ValidateAttributeError) {
FlowListMoMgr obj;
ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey, NULL));
delete ikey;
}
/*=============================AddFlowListToController================================*/

/*invalid testcase, ctrlr_id NULL check taken care in caller 
TEST(FlowListMoMgrUt,AddFlowListToControllerctrlid_NULL) {
FlowListMoMgr obj;
upll_keytype_datatype_t dt_type;
DalDmlIntf *dmi = new DalOdbcMgr();
char fl_name[4] = {'F','L','O','W'};
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AddFlowListToController(fl_name,dmi, NULL, dt_type, UNC_OP_READ));
}
*/
TEST(FlowListMoMgrUt,AddFlowListToControllerctrlid_CtrlName) {
FlowListMoMgr obj;
//DalDmlIntf *dmi(getDalDmlIntf());
DalDmlIntf *dmi = new DalOdbcMgr();

upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
char ctrlr_name[] = {'c','n','t','r','l','r','_','n','a','m','e'};
const char* version("version");
CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 CapaModuleStub::stub_loadCapaModule();
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
char fl_name[4] = {'F','L','O','W'};
//UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
TcConfigMode config_mode = TC_CONFIG_VTN;
string vtn_name = "vtn1";

EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AddFlowListToController(fl_name,dmi, ctrlr_name,
         dt_type, UNC_OP_CREATE, config_mode, vtn_name));
CapaModuleStub::stub_clearStubData();
DalOdbcMgr::clearStubData();
}
#if 0
tested in momgr_impl_ut
/* couln't make validatecapbility to return SUCCESS*/
TEST(FlowListMoMgrUt,TxUpdateController_Success) {
  FlowListMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> affected_ctrlr_set;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
		malloc(sizeof(key_flowlist_t)));
  memset(key_flowlist, 0, sizeof(key_flowlist_t));
  strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
		"vtn" , kMaxLenFlowListName);
    DalOdbcMgr::stub_setSingleRecordExists(true);;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
		malloc(sizeof(val_flowlist_t)));
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
  IpcRequest *ipc_req = reinterpret_cast<IpcRequest*>
                                (malloc(sizeof(IpcRequest)));
   IpcResponse *ipc_response = reinterpret_cast<IpcResponse*>
                                      (malloc(sizeof(IpcResponse)));

 ConfigKeyVal *l_err_ckv = NULL;
  const char * ctrlr_name = "PFC";
 const char* version("version");

CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);

 
 DalDmlIntf *dmi = new DalOdbcMgr();
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, 
                obj.TxUpdateController(UNC_KT_FLOWLIST,session_id,config_id,phase,&affected_ctrlr_set,dmi,&l_err_ckv));
 delete dmi;  delete l_err_ckv;
 DalOdbcMgr::clearStubData();
}
#endif
/*
TEST(FlowListMoMgrUt, TxCopyCandidateToRunning_cs_null) {
  FlowListMoMgr obj;
  typedef list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  CtrlrCommitStatusList * l_CtrlrCommitStatusList = NULL;

  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";
 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
           TxCopyCandidateToRunning
          (UNC_KT_FLOWLIST, l_CtrlrCommitStatusList, dmi,
           config_mode, vtn_name));
}

TEST(FlowListMoMgrUt, TxCopyCandidateToRunning_noNULL) {
  FlowListMoMgr obj;
  typedef list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  CtrlrCommitStatusList * l_CtrlrCommitStatusList = NULL;
  l_CtrlrCommitStatusList = new CtrlrCommitStatusList();
  DalDmlIntf *dmi = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  dmi = new DalOdbcMgr();
  CtrlrTxResult *l_CtrlrTxResult = new CtrlrTxResult ();
    l_CtrlrCommitStatusList.insert(l_CtrlrTxResult);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
           TxCopyCandidateToRunning
         (UNC_KT_FLOWLIST, l_CtrlrCommitStatusList, dmi,
          config_mode, vtn_name));
}

TEST(FlowListMoMgrUt, TxUpdateControllerdmiNULL) {
  FlowListMoMgr obj;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (UNC_KT_FLOWLIST,&CtrlrCommitStatusList,dmi,
                    config_mode, vtn_name));
}

TEST(FlowListMoMgrUt, TxUpdateControllerDiffconfigFail) {
  FlowListMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> *affected_ctrlr_set = NULL;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
		malloc(sizeof(key_flowlist_t)));
  memset(key_flowlist, 0, sizeof(key_flowlist_t));
  strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
		"flowlist" , kMaxLenFlowListName);
   key_flowlist_t *l_key_flowlist = reinterpret_cast<key_flowlist_t*>(
		malloc(sizeof(key_flowlist_t)));
  memset(l_key_flowlist, 0, sizeof(key_flowlist_t));
 i_flowlist = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcTxnError);
  //val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
		//malloc(sizeof(val_flowlist_t)));
 //ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowlist);
  //IpcRequest *ipc_req = reinterpret_cast<IpcRequest*>
                                (malloc(sizeof(IpcRequest)));
   //IpcResponse *ipc_response = reinterpret_cast<IpcResponse*>
                                      (malloc(sizeof(IpcResponse)));

 ConfigKeyVal *l_err_ckv = NULL;
 TcConfigMode config_mode = TC_CONFIG_VTN;
 string vtn_name = "vtn1";

 DalDmlIntf *dmi = new DalOdbcMgr();
 TxUpdateUtil tx(4);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, 
                obj.TxUpdateController(UNC_KT_FLOWLIST,session_id,config_id,
                phase,affected_ctrlr_set,dmi,&l_err_ckv, &tx, config_mode,
                vtn_name));
 delete dmi; 
 DalOdbcMgr::clearStubData();
}
*/
TEST(TxUpdateController,diffconfig_default) {
  FlowListMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> *affected_ctrlr_set = NULL;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpUpdate;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
		malloc(sizeof(key_flowlist_t)));
  memset(key_flowlist, 0, sizeof(key_flowlist_t));
  strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
		"Flowlist" , kMaxLenFlowListName);
   key_flowlist_t *l_key_flowlist = reinterpret_cast<key_flowlist_t*>(
		malloc(sizeof(key_flowlist_t)));
  memset(l_key_flowlist, 0, sizeof(key_flowlist_t));
 i_flowlist = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcTxnError);
  //val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
		//malloc(sizeof(val_flowlist_t)));
 //ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
  //IpcRequest *ipc_req = reinterpret_cast<IpcRequest*>
    //                            (malloc(sizeof(IpcRequest)));
   //IpcResponse *ipc_response = reinterpret_cast<IpcResponse*>
      //                                (malloc(sizeof(IpcResponse)));

 ConfigKeyVal *l_err_ckv = NULL;

 DalDmlIntf *dmi = new DalOdbcMgr();
 TxUpdateUtil tx(4);
 EXPECT_EQ(UPLL_RC_SUCCESS, 
                obj.TxUpdateController(UNC_KT_FLOWLIST,session_id,config_id,
                phase,affected_ctrlr_set,dmi,&l_err_ckv, &tx, config_mode, vtn_name));
 delete dmi; 
 DalOdbcMgr::clearStubData();
}
TEST(FlowListMoMgrUt, TxUpdateControllerGetnextRecord_succ) {
  FlowListMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> *affected_ctrlr_set = NULL;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
		malloc(sizeof(key_flowlist_t)));
  memset(key_flowlist, 0, sizeof(key_flowlist_t));
  strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
		"flowlist" , kMaxLenFlowListName);
   key_flowlist_t *l_key_flowlist = reinterpret_cast<key_flowlist_t*>(
		malloc(sizeof(key_flowlist_t)));
  memset(l_key_flowlist, 0, sizeof(key_flowlist_t));
 i_flowlist = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNoMore);
  val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
		malloc(sizeof(val_flowlist_t)));
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowlist);
  IpcRequest *ipc_req = reinterpret_cast<IpcRequest*>
                                (malloc(sizeof(IpcRequest)));
   IpcResponse *ipc_response = reinterpret_cast<IpcResponse*>
                                      (malloc(sizeof(IpcResponse)));

 ConfigKeyVal *l_err_ckv = NULL;

 DalDmlIntf *dmi = new DalOdbcMgr();
 TxUpdateUtil tx(4);
 EXPECT_EQ(UPLL_RC_SUCCESS, 
                obj.TxUpdateController(UNC_KT_FLOWLIST,session_id,config_id,
                phase,affected_ctrlr_set,dmi,&l_err_ckv, &tx, 
                config_mode, vtn_name));
 delete dmi;  free (ipc_response);free(ipc_req);delete config_val;
 DalOdbcMgr::clearStubData();
}
#if 0
// this function tested in momgr_impl_ut file
TEST(FlowListMoMgrUt, TxUpdateControllerUpdateProcessSucc) {
  FlowListMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> affected_ctrlr_set;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
		malloc(sizeof(key_flowlist_t)));
  memset(key_flowlist, 0, sizeof(key_flowlist_t));
  strncpy(reinterpret_cast<char*>(key_flowlist->flowlist_name),
		"vtn" , kMaxLenFlowListName);
   key_flowlist_t *l_key_flowlist = reinterpret_cast<key_flowlist_t*>(
		malloc(sizeof(key_flowlist_t)));
  memset(l_key_flowlist, 0, sizeof(key_flowlist_t));
 i_flowlist = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t*>(
		malloc(sizeof(val_flowlist_t)));
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val_flowlist);
  IpcRequest *ipc_req = reinterpret_cast<IpcRequest*>
                                (malloc(sizeof(IpcRequest)));
   IpcResponse *ipc_response = reinterpret_cast<IpcResponse*>
                                      (malloc(sizeof(IpcResponse)));

 ConfigKeyVal *l_err_ckv = NULL;

 DalDmlIntf *dmi = new DalOdbcMgr();
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, 
                obj.TxUpdateController(UNC_KT_FLOWLIST,session_id,config_id,phase,&affected_ctrlr_set,dmi,&l_err_ckv));
 delete dmi; 
 DalOdbcMgr::clearStubData();
}
#endif
TEST(FlowlistMoMgr, TxCopyCandidateToRunning_CTRLTBL_FAIL) {
  FlowListMoMgr obj;
  DalDmlIntf *dmi = NULL;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  ConfigVal *l_cval = NULL;
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD,kDalRcRecordNoMore);
  CtrlrCommitStatusList.push_back(l_CtrlrTxResult);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc_key_type_t keytype = UNC_KT_FLOWLIST;
  val_flowlist_t* val_flowlist = new val_flowlist_t();
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_flowlist);
  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>(
                malloc(sizeof(key_flowlist_t)));
 dmi = new DalOdbcMgr();
 ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_flowlist,l_cval);
 
  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi,
                    config_mode, vtn_name));
  delete dmi;  delete l_CtrlrTxResult;
}
/*
TEST(FlowListMoMgrUt, TxCopyCandidateToRunning_DELETE_UPDCONFG) {
  FlowListMoMgr obj;
  DalDmlIntf *dmi = NULL;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigVal *l_cval = NULL;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  CtrlrCommitStatusList.push_back(l_CtrlrTxResult); 
  unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  val_flowlist_t* val_flowlist = new val_flowlist_t();
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
                             val_flowlist);
 key_flowlist_t*  key_flowlist = new key_flowlist_t();
 dmi = new DalOdbcMgr();
 ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            key_flowlist,l_cval);
 
  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi,
                     config_mode, vtn_name));
 delete dmi; delete l_CtrlrTxResult;
 CapaModuleStub::stub_clearStubData();
}
*/
/*******PartialConfig New TC*********/

/**************RefCountSemanticCheck*********/
TEST(FlowListMoMgrUt, RefCountSemanticCheck_1) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  const char * flowlist_name = "flowlist_name";
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";
  //EXECUTE_QUERY success 
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.RefCountSemanticCheck(flowlist_name,
            dmi, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, RefCountSemanticCheck_2) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  const char * flowlist_name = "flowlist_name";
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";
  //EXECUTE_QUERY failure
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.RefCountSemanticCheck(flowlist_name,
            dmi, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, RefCountSemanticCheck_3) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  const char * flowlist_name = "flowlist_name";
  //config mode is VIRTUAL
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  std::string vtn_name = "";
  
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.RefCountSemanticCheck(flowlist_name,
            dmi, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, RefCountSemanticCheck_4) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  const char * flowlist_name = "flowlist_name";
  //config mode is GLOBAL
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  std::string vtn_name = "";
  
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.RefCountSemanticCheck(flowlist_name,
            dmi, config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}

/**********UpdateRefCountInScratchTbl***********/
TEST(FlowListMoMgrUt, UpdateRefCountInScratchTbl_1) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  std::string vtn_name = "";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);

  uint32_t count = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  //ikey->user_data is NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateRefCountInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, UpdateRefCountInScratchTbl_2) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  std::string vtn_name = "";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  
  //ikey is NULL
  ConfigKeyVal *ikey = NULL;
  uint32_t count = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateRefCountInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, UpdateRefCountInScratchTbl_3) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;

  //vtn_name is empty
  std::string vtn_name = "";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);
  uint32_t count = 0;

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateRefCountInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, UpdateRefCountInScratchTbl_4) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;

  //vtn?_name is filled
  std::string vtn_name = "VTN1";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);
  uint32_t count = 1;

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateRefCountInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, UpdateRefCountInScratchTbl_5) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;

  std::string vtn_name = "VTN1";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  //operation is DELETE
  unc_keytype_operation_t op = UNC_OP_DELETE;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);
  uint32_t count = 0;

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateRefCountInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, UpdateRefCountInScratchTbl_6) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;

  std::string vtn_name = "VTN1";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  //operation is UPDATE
  unc_keytype_operation_t op = UNC_OP_UPDATE;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);
  uint32_t count = 1;

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateRefCountInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}

/*****************InsertRecInScratchTbl**********/
//Success Case
TEST(FlowListMoMgrUt, InsertRecInScratchTbl_1) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;

  std::string vtn_name = "VTN1";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  unc_keytype_operation_t op = UNC_OP_CREATE;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);

  uint32_t count = 1;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.InsertRecInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, InsertRecInScratchTbl_2) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name = "VTN1";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);

  uint32_t count = 0;

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  //ikey->user_data is NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.InsertRecInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, InsertRecInScratchTbl_3) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name = "VTN1";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  
  uint32_t count = 1;
  //ikey is NULL
  ConfigKeyVal *ikey = NULL;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.InsertRecInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, InsertRecInScratchTbl_4) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;

  //vtn_name is empty
  std::string vtn_name = "";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);
  uint32_t count = 0;

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.InsertRecInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}


TEST(FlowListMoMgrUt, InsertRecInScratchTbl_5) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;

  std::string vtn_name = "VTN1";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  //operation is DELETE
  unc_keytype_operation_t op = UNC_OP_DELETE;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  uint32_t count = 1;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.InsertRecInScratchTbl(ikey,
            dmi, dt_type, op, config_mode, vtn_name, count));
  DalOdbcMgr::clearStubData();
}

/*****************ComputeRefCountInScratchTbl************/
TEST(FlowListMoMgrUt, ComputeRefCountInScratchTbl_1) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;

  std::string vtn_name = "VTN1";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  int ref_cnt = 0;
  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ComputeRefCountInScratchTbl(ikey,
            dmi, dt_type, config_mode, vtn_name, ref_cnt));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, ComputeRefCountInScratchTbl_2) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;

  std::string vtn_name = "VTN1";
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  int ref_cnt = 1;
  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  //ikey->user_data is  NULL
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ComputeRefCountInScratchTbl(ikey,
            dmi, dt_type, config_mode, vtn_name, ref_cnt));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, ComputeRefCountInScratchTbl_3) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;

  std::string vtn_name = "VTN1";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  int ref_cnt = 0;
  ConfigKeyVal *ikey = NULL;
  //ikey is NULL
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ComputeRefCountInScratchTbl(ikey,
            dmi, dt_type, config_mode, vtn_name, ref_cnt));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, ComputeRefCountInScratchTbl_4) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  //config mode is VIRTUAL
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;

  std::string vtn_name = "";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  int ref_cnt = 1;
  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ComputeRefCountInScratchTbl(ikey,
            dmi, dt_type, config_mode, vtn_name, ref_cnt));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, ComputeRefCountInScratchTbl_5) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;

  std::string vtn_name = "VTN1";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  int ref_cnt = 0;
  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
 
   const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ComputeRefCountInScratchTbl(ikey,
            dmi, dt_type, config_mode, vtn_name, ref_cnt));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, ComputeRefCountInScratchTbl_6) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  //CFG_MODE is VIRTUAL
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;

  std::string vtn_name = "";

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  int ref_cnt = 1;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ComputeRefCountInScratchTbl(ikey,
            dmi, dt_type, config_mode, vtn_name, ref_cnt));
  DalOdbcMgr::clearStubData();
}


/**************ClearScratchTbl**************/
TEST(FlowListMoMgrUt, ClearScratchTbl_1) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name = "VTN1";

  //ExecuteAppQuery SUCCESS
  DalOdbcMgr::stub_setSingleRecordExists(true);;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ClearScratchTbl(config_mode,
            vtn_name, dmi));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, ClearScratchTbl_2) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;
  //vtn_name is empty
  std::string vtn_name = "";

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ClearScratchTbl(config_mode,
            vtn_name, dmi));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, ClearScratchTbl_3) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  //cfg_mode is VIRTUAL
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  std::string vtn_name = "";

  DalOdbcMgr::stub_setSingleRecordExists(true);;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ClearScratchTbl(config_mode,
            vtn_name, dmi));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, ClearScratchTbl_4) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  //cfg_mode is VIRTUAL
  TcConfigMode config_mode = TC_CONFIG_VIRTUAL;
  std::string vtn_name = "";

  //ExecuteAppQuery SUCCESS
  DalOdbcMgr::stub_setSingleRecordExists(true);;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ClearScratchTbl(config_mode,
            vtn_name, dmi));
  DalOdbcMgr::clearStubData();
}

TEST(FlowListMoMgrUt, ClearScratchTbl_5) {
  FlowListMoMgr obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name = "VTN1";

  //ExecuteAppQuery SUCCESS
  DalOdbcMgr::stub_setSingleRecordExists(true);;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcGeneralError);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ClearScratchTbl(config_mode,
            vtn_name, dmi));
  DalOdbcMgr::clearStubData();
}

#if 0
/*************CheckRefCountInCtrlrTbl***********/
TEST(FlowlistTest, CheckRefCountInCtrlrTbl_1) {
  FlowlistTest obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name = "VTN1";
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
 
  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));

  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);

  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CheckRefCountInCtrlrTbl(ikey,
           dmi, dt_type, config_mode, vtn_name));
  FlowlistTest::stub_result.clear();
  DalOdbcMgr::clearStubData();
}

TEST(FlowlistTest, CheckRefCountInCtrlrTbl_2) {
  FlowlistTest obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name = "VTN1";
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
 
  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));

  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);

  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.CheckRefCountInCtrlrTbl(ikey,
           dmi, dt_type, config_mode, vtn_name));
  FlowlistTest::stub_result.clear();
  DalOdbcMgr::clearStubData();
}

TEST(FlowlistTest, CheckRefCountInCtrlrTbl_3) {
  FlowlistTest obj;
  DalDmlIntf  *dmi = new DalOdbcMgr();
  TcConfigMode config_mode = TC_CONFIG_VTN;
  std::string vtn_name = "VTN1";
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  
  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));

  memset(flowlist_key1, 0, sizeof(key_flowlist_t));

  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  const char * ctrlr_name = "PFC";
  ikey->set_user_data((void*)ctrlr_name);

  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_GENERIC;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcRecordNotFound);
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CheckRefCountInCtrlrTbl(ikey,
           dmi, dt_type, config_mode, vtn_name, is_commit, count));
  FlowlistTest::stub_result.clear();
  DalOdbcMgr::clearStubData();
}
#endif

