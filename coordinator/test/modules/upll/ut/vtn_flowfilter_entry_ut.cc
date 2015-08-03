/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <upll/vtn_flowfilter_entry_momgr.hh>
#include <upll/config_mgr.hh>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <dal/dal_odbc_mgr.hh>
#include <config_mgr.hh>
#include <ctrlr_mgr.hh>
#include <list>
#include "../ut_util.hh"
#include <pfcxx/synch.hh>
#include  "tx_update_util.hh"
#include "include/vtn_ff_entry_stub.hh"  


std::map<function, upll_rc_t> VtnffEntryTest::stub_result;

using ::testing::TestWithParam;
using ::testing::Values;
using namespace std;
using namespace unc::upll::dal;
using namespace unc::capa;
using namespace unc::upll::config_momgr;
using namespace unc::upll::test;
using namespace unc::upll::kt_momgr;
using namespace pfc::core;
using namespace unc::upll;
using namespace unc::tclib;
using namespace unc::upll::tx_update_util;

class VtnFlowFilterEntryTest :  public UpllTestEnv
{
};

TEST_F (VtnFlowFilterEntryTest, validate_UNC_KT_VTN_FLOWFILTER_ENTRY)
{
  VtnFlowFilterEntryMoMgr obj;
  BindInfo *bin = NULL ;
  int nattr ;
  
   EXPECT_EQ(PFC_TRUE,obj.GetRenameKeyBindInfo(UNC_KT_VTN_FLOWFILTER_ENTRY, bin, nattr, MAINTBL));
}
TEST_F (VtnFlowFilterEntryTest, validate_CTRLRTBL)
{
  VtnFlowFilterEntryMoMgr obj;
  BindInfo *bin = NULL ;
  int nattr ;
  
  EXPECT_EQ(PFC_TRUE,obj.GetRenameKeyBindInfo(UNC_KT_VTN_FLOWFILTER_ENTRY, bin, nattr, CTRLRTBL));
}
TEST_F (VtnFlowFilterEntryTest, validate_UNC_KT_FLOWLIST)
{
  VtnFlowFilterEntryMoMgr obj;
  BindInfo *bin = NULL ;
  int nattr ;
  
  EXPECT_EQ(PFC_TRUE,obj.GetRenameKeyBindInfo(UNC_KT_FLOWLIST, bin, nattr,MAINTBL ));
}
TEST_F (VtnFlowFilterEntryTest, AllocMaintblSuccess)
{
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *l_ck_val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(l_ck_val, UPLL_DT_STATE, MAINTBL));
}
TEST_F (VtnFlowFilterEntryTest, AllocCtrlSuccess)
{
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *l_ck_val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(l_ck_val, UPLL_DT_STATE,CTRLRTBL));
}
TEST_F (VtnFlowFilterEntryTest, ValidnameSuccess)
{
  VtnFlowFilterEntryMoMgr obj;
  void* l_key = NULL;
  key_vtn_flowfilter_entry_t * key_vtn_flowfilter = 
        ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  l_key = (void*) (key_vtn_flowfilter);
  strcpy((char*)(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),"vtn");
  unsigned long index = uudst::vtn_flowfilter_entry::kDbiVtnName;
  EXPECT_EQ(PFC_TRUE, obj.IsValidKey(l_key, index));
  delete key_vtn_flowfilter;
}
#if 0
TEST_F (VtnFlowFilterEntryTest,Ctrl_ReadMo_DT_STATE_DETAIL) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  key_vtn_flowfilter_controller_t *key_vtn_flowfilter = 
                  ZALLOC_TYPE(key_vtn_flowfilter_controller_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  //key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  //key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_controller_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_controller_t);
  val_flowfilter->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  val_flowfilter->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_INVALID;
  val_flowfilter->direction = 1;
  val_flowfilter->sequence_num = 1;
 
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterController, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
		IpctSt::kIpcStKeyVtnFlowfilterController, key_vtn_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 // EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.ReadRecord(req, ikey, dmi,UNC_OP_READ_SIBLING));
// delete dmi;delete ikey; free(req);
 
}
TEST_F (VtnFlowFilterEntryTest, ReadDetail_valid_data) {
  VtnFlowFilterEntryMoMgr obj;
  key_vtn_flowfilter_controller_t *key_vtn_ctrl = ZALLOC_TYPE(key_vtn_flowfilter_controller_t);
  val_flowfilter_controller_t *val_flowfilter_controller = ZALLOC_TYPE(val_flowfilter_controller_t);
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_flowfilter_controller);
  if(!tmp) {
    std::cout<<"tmp memory allocation failed"<<std::endl;
    exit(0);
  }
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
               IpctSt::kIpcStKeyVtnFlowfilterController,key_vtn_ctrl,tmp);
  IPC_RESPONSE_DECL(ipc_response);
  ipc_response->header.clnt_sess_id = 1;
  ipc_response->header.config_id = 1;
  ipc_response->header.operation = UNC_OP_READ;
  ipc_response->header.rep_count = 1;
  ipc_response->header.option1 = UNC_OPT1_DETAIL;
  ipc_response->header.option2 = UNC_OPT2_NONE;
  ipc_response->header.datatype = UPLL_DT_STATE;
  ipc_response->header.result_code = UPLL_RC_SUCCESS;
  val_vtn_flowfilter_controller_st_t *val_vtn_stflowfilter = 
     new val_vtn_flowfilter_controller_st_t();
  ConfigVal *tmp_st = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterControllerSt,
                             val_vtn_stflowfilter);
  val_vtn_stflowfilter->valid[UPLL_IDX_SEQ_NUM_VFFCS] = UNC_VF_VALID ;
  ipc_response->ckv_data= new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                 IpctSt::kIpcStValFlowfilterEntrySt,NULL,tmp_st);
  if (!ipc_response->ckv_data) {
    std::cout<<"memory allocation failed"<<std::endl;
    exit(0);
  }
  val_flowlist_entry_st_t *val_flowlist_entry_st =
                                new val_flowlist_entry_st_t(); 
  ConfigVal *tmp_fl_st = new ConfigVal(IpctSt::kIpcStValFlowfilterEntrySt,
                             val_flowlist_entry_st);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  ipc_response->return_code = UPLL_RC_SUCCESS;
  ipc_response->ckv_data->AppendCfgVal(tmp_fl_st); 
  unc_keytype_operation_t op = UNC_OP_READ;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t  domain_id[32] = {'u','n','k','n','o','w','n'};
  uint8_t ctrlr_id[4] = {'p', 'f', 'c'};
  std::cout<<"calling ReadDetail function"<<std::endl;
  
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ReadDetailSibling
                ( 
                 ipc_response,
                 ikey,
                 UPLL_DT_STATE, 
                 op,
                 dbop,
                 dmi,
                 domain_id,
                 ctrlr_id));

  
}
#endif
TEST_F (VtnFlowFilterEntryTest, AllocErr)
{
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *l_ck_val;
  val_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_entry_t);
  l_ck_val  = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,val_flowfilter);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AllocVal(l_ck_val, UPLL_DT_STATE, MAINTBL));
  delete l_ck_val;
}
TEST_F (VtnFlowFilterEntryTest, Valid_name_fail)
{
  VtnFlowFilterEntryMoMgr obj;
  void* l_key = NULL;
  key_vtn_flowfilter_entry_t * key_vtn_flowfilter = 
                  ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  l_key = (void*) (key_vtn_flowfilter);
  strcpy((char*)(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name)," ");
  unsigned long index = uudst::vtn_flowfilter_entry::kDbiVtnName;
  EXPECT_EQ(PFC_FALSE, obj.IsValidKey(l_key, index));
  delete key_vtn_flowfilter;
  index = 0;
}
TEST_F (VtnFlowFilterEntryTest, Valid_dir_success)
{
  VtnFlowFilterEntryMoMgr obj;
  void* l_key = NULL;
  key_vtn_flowfilter_entry_t * key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  l_key = (void*) (key_vtn_flowfilter);
  
  key_vtn_flowfilter->sequence_num= 1;
  
  unsigned long index = uudst::vtn_flowfilter_entry::kDbiSequenceNum;
  EXPECT_EQ(PFC_TRUE, obj.IsValidKey(l_key, index));
   delete key_vtn_flowfilter;
}
TEST_F (VtnFlowFilterEntryTest, Request_Null_success)
{
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(l_okey, l_req,MAINTBL));
}
TEST_F (VtnFlowFilterEntryTest, Request_okey_success)
{
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_req = NULL;
  key_vtn_flowfilter_entry_t *vtn_flowfilterkey =
           ZALLOC_TYPE(key_vtn_flowfilter_entry_t);

  key_vtn_flowfilter_entry_t *l_vtn_flowfilterkey =
        ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  
  l_okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtnFlowfilterEntry,
                          l_vtn_flowfilterkey, NULL);
  l_req = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtnFlowfilterEntry,
                          vtn_flowfilterkey, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(l_okey, l_req,MAINTBL));
  delete l_req;
  delete l_okey;
}
TEST_F (VtnFlowFilterEntryTest, Request_key_success)
{
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_req = NULL;
  key_vtn_flowfilter_entry_t *vtn_flowfilterkey =
        ZALLOC_TYPE(key_vtn_flowfilter_entry_t);

  ConfigVal *tmp1 = NULL;
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

  l_req = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY, 
  IpctSt::kIpcStKeyVtnFlowfilterEntry,
          vtn_flowfilterkey, tmp1);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(l_okey, l_req,MAINTBL));
  delete l_req;
}
TEST_F (VtnFlowFilterEntryTest, RequestctrltblSuccess)
{
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_req = NULL;
  key_vtn_flowfilter_entry_t *vtn_flowfilterkey =
        ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  ConfigVal *tmp1 = NULL;
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  l_req = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtnFlowfilterEntry,
                          vtn_flowfilterkey ,tmp1 );
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(l_okey, l_req,CTRLRTBL));
  delete l_req;
}
TEST_F (VtnFlowFilterEntryTest, ReqParentkeyNull)
{
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_parent_key = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(l_okey, l_parent_key));
}
TEST_F (VtnFlowFilterEntryTest, ReqParentkeyNotNull)
{
  VtnFlowFilterEntryMoMgr obj;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_parent_key = NULL;
  key_vtn_flowfilter_entry_t *vtn_flowfilter_key =
                  ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  strncpy((char*)(vtn_flowfilter_key->flowfilter_key.vtn_key.vtn_name),"vtn",10);
  l_parent_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtnFlowfilterEntry,
                          vtn_flowfilter_key, NULL);
 result_code=  obj.GetChildConfigKey(l_okey, l_parent_key); 
  EXPECT_EQ(UPLL_RC_SUCCESS, result_code);
  delete l_parent_key;
}
TEST_F (VtnFlowFilterEntryTest, ReqKeyof_ParentkeyNull)
{
  VtnFlowFilterEntryMoMgr obj;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_parent_key = NULL;
  l_parent_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtnFlowfilterEntry,
                          NULL, NULL);
  result_code=  obj.GetChildConfigKey(l_okey, l_parent_key); 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, result_code);
  delete l_parent_key;
}
TEST_F (VtnFlowFilterEntryTest, CopyToConfigkeyVal_Ikey_NUll) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
  delete ikey;
}

TEST_F (VtnFlowFilterEntryTest, CopyToConfigkeyVal_InValid) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_rename, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  delete ikey;
}
TEST_F (VtnFlowFilterEntryTest, CopyToConfigkey_InValid) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  strncpy((char*) key_rename->old_unc_vtn_name,"OLDVTNNAME",32); 
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_rename, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.CopyToConfigKey(okey,ikey));
  delete ikey;
}
TEST_F (VtnFlowFilterEntryTest, CopyToConfigkey_FLOWLIST) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  strncpy((char*) key_rename->old_flowlist_name,"FLOWLIST",32); 
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            key_rename, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
  delete ikey;
}
TEST_F (VtnFlowFilterEntryTest, CopyToConfigkey_INVALIDKEYTYPE) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  strncpy((char*) key_rename->old_flowlist_name,"FLOWLIST",32); 
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_rename, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
  delete ikey;
}
#if 0
TEST_F (VtnFlowFilterEntryTest, CopyToConfigkey_MAINTBL) {
  VtnFlowFilterEntryMoMgr obj;
  int indx = uudst::vtn_flowfilter_entry::kDbiFlowlistName;
  val_vtn_flowfilter_entry_t *l_val = 
  upll_keytype_datatype_t dt_type = 
  MoMgrTables tbl = MAINTBL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetValid(val,indx,valid,dt_type,tbl));
  delete ikey;
}

TEST_F (VtnFlowFilterEntryTest, CompareValidValue_AuditTrue) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
 val_vtn_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

 for (int loop = 0;loop< sizeof(val_ffe1->valid); loop++) {
   val_ffe1->valid[loop] = UNC_VF_INVALID;
   val_ffe2->valid[loop] = UNC_VF_VALID;
   obj.CompareValidValue((void *&) val_ffe1,(void *&) val_ffe2,true);
   EXPECT_EQ(UNC_VF_VALID_NO_VALUE,val_ffe1->valid[loop]);
 }
 free (val_ffe1);
 free (val_ffe2);
}
#endif
TEST_F (VtnFlowFilterEntryTest,Validate_VAL_NULL) {
  VtnFlowFilterEntryMoMgr obj;
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase  = uuc::kUpllUcpCreate; 
  ConfigKeyVal *ckv_running = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  ckv_running  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStValFlowfilterController,
                            NULL,NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 UpdateAuditConfigStatus
                    (cs_status ,phase,ckv_running, dmi));

  delete ckv_running;
}
TEST_F (VtnFlowFilterEntryTest,Validate_VAL_NOT_NULL) {
  VtnFlowFilterEntryMoMgr obj;
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase  = uuc::kUpllUcpCreate; 
  ConfigKeyVal *ckv_running = NULL;
  val_vtn_flowfilter_entry_t *val_ffe1 = 
            ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi = NULL;
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_ffe1);
  ckv_running  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStValFlowfilterController,
                            NULL,tmp1);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 UpdateAuditConfigStatus
                    (cs_status ,phase,ckv_running, dmi));

  delete ckv_running;
}
#if 0
TEST_F (VtnFlowFilterEntryTest,Validate_KEY_NOT_NULL) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *l_cval = NULL;
  val_vtn_flowfilter_entry_t* val_vtn_flowfilter = 
                       ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_vtn_flowfilter);
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = 
    ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,l_cval);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 SetConsolidatedStatus
                    (l_ikey,dmi));
  delete l_ikey;
}
TEST_F (VtnFlowFilterEntryTest,Validate_KEY_NULL) {
  VtnFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *l_ikey = NULL;
 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 SetConsolidatedStatus
                    (l_ikey,dmi));
}
TEST_F (VtnFlowFilterEntryTest,Validate_KEY_NOT_NULL) {
  VtnFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  char l_vtn[] = "vtn";
   struct  controller_domain *lcontroller =( struct controller_domain*)
                             malloc(sizeof(struct controller_domain));
   lcontroller->ctrlr =(uint8_t*)malloc(sizeof(20));
   strncpy((char*)(lcontroller->ctrlr),"l_ctrl",20);

    DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
   //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 UpdateControllerTableForVtn
                    ((uint8_t*)(l_vtn),lcontroller,UNC_OP_CREATE,dmi));
delete dmi;
 
}
TEST_F (VtnFlowFilterEntryTest,Validate_UpdateConfigDB_SUCCESS) {
  VtnFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  char l_vtn[] = "vtn";
  controller_domain *lcontroller =( struct controller_domain*)
                             malloc(sizeof(struct controller_domain));
   lcontroller->ctrlr =(uint8_t*)malloc(sizeof(30));
   strncpy((char*)(lcontroller->ctrlr),"l_ctrl",20);

    DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
    DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
 
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 UpdateControllerTableForVtn
                    ((uint8_t*)(l_vtn), lcontroller, UNC_OP_CREATE, UNC_DT_CANDIDATE, dmi, (uint8_t)1));
delete dmi;
 
}
#endif
TEST_F (VtnFlowFilterEntryTest,Validate_ctrlckv_NOT_NULL) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ctrlckv  = NULL;
  //char l_ctrl[] = "ctrl";
  ConfigVal *tmp1 = NULL;
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  ival->valid[0] = UNC_VF_VALID;
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
  //strncpy((char*)(key_vtn_flowfilter->vtn_key.vtn_name) ,"vtn",10);
  //key_vtn_flowfilter->input_direction = 1;
  controller_domain *ctrlr_dom = ZALLOC_TYPE(controller_domain );
  uint8_t name[3] = {'P','F','C'};
  ctrlr_dom->ctrlr = name;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 GetControllerKeyval
                    (ctrlckv ,l_ikey,ctrlr_dom));
  delete l_ikey;

}
TEST_F (VtnFlowFilterEntryTest,Validate_UpdateFlowList) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNotFound);
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
   strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
                "vtn" , kMaxLenVtnName);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
 
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 UpdateFlowListInCtrl
                    (l_ikey, UPLL_DT_CANDIDATE, UNC_OP_CREATE ,dmi, config_mode,
                     vtn_name));
  delete l_ikey;
 
}
TEST_F (VtnFlowFilterEntryTest,UpdateFlowList_RestoreFail) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
   strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
                "vtn" , kMaxLenVtnName);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 UpdateFlowListInCtrl
                    (l_ikey, UPLL_DT_CANDIDATE, UNC_OP_CREATE ,dmi,
                     config_mode, vtn_name));
  delete l_ikey;
}
#if 0
TEST_F (VtnFlowFilterEntryTest, ValidateMsg1_Validreq_NULLkey) {
  VtnFlowFilterEntryMoMgr obj;
  /** set Invalid keytype */
  ConfigKeyVal *ikey = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateMessage(req, ikey));
}
#endif
TEST_F (VtnFlowFilterEntryTest,ValidateMsg2_InvalidKeytype) {
  VtnFlowFilterEntryMoMgr obj;
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,NULL);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
  delete ikey;
}

TEST_F (VtnFlowFilterEntryTest,ValidateMsg2_NOT_READ_SUPPORTED_DATATYPE) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_AUDIT;
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_NONE;   
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
  delete ikey;
}
TEST_F (VtnFlowFilterEntryTest,ValidateMsg2_NOT_OPEARTION_WITH_VAL_STRUCT_NONE) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_AUDIT;
  req->operation = UNC_OP_CONTROL;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_NONE;   
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
  delete ikey;
}
TEST_F (VtnFlowFilterEntryTest, ValidateMsg1_Validreq_NULLkey) {
  VtnFlowFilterEntryMoMgr obj;
/** set Invalid keytype */
  ConfigKeyVal *ikey = NULL;
  char l_ctrl[]= "ctrlr";
  IPC_REQ_RESP_HEADER_DECL(req);

EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(req, ikey,l_ctrl));
}
TEST_F (VtnFlowFilterEntryTest,ValidateMsg2_UNC_OP_CREATE) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  char ctrlr_name[]= "ctrlr";
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_L2DOMAIN; 

  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey,ctrlr_name));
  delete ikey;
}
TEST_F (VtnFlowFilterEntryTest,ValidateMsg2_UNC_OP_UPDATE) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  char ctrlr_name[]= "ctrlr";
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype =  UPLL_DT_STARTUP;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_L2DOMAIN;  
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);


  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey,ctrlr_name));
  delete ikey;
}
TEST_F (VtnFlowFilterEntryTest,ValidateMsg2_DEFAULT) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  char ctrlr_name[]= "ctrlr";
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype =  UPLL_DT_STARTUP;
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->option1 = UNC_OPT1_DETAIL;   
  req->option2 = UNC_OPT2_NONE;  
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);


  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey,ctrlr_name));
  delete ikey;
}
#if 0
TEST_F (VtnFlowFilterEntryTest,ValidateMsg2_DEFAULT_READ) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  char ctrlr_name[]= "ctrlr";
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype =  UPLL_DT_STARTUP;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;   
  req->option2 = UNC_OPT2_NONE;  
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);


  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateCapability(req, ikey,ctrlr_name));
  delete ikey;
}
TEST_F (VtnFlowFilterEntryTest,ValidateMsg2_INVALID_OPTIONTWO) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  char ctrlr_name[]= "ctrlr";
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype =  UPLL_DT_STARTUP;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;   
  req->option2 = UNC_OPT2_MAC_ENTRY;  
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);


  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateCapability(req, ikey,ctrlr_name));
  delete ikey;
}
TEST_F (VtnFlowFilterEntryTest,ValidateMsg2_NOT_READ_SUPPORTED_DATATYPE) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  char ctrlr_name[]= "ctrlr";
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype =  UPLL_DT_AUDIT;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;   
  req->option2 = UNC_OPT2_MAC_ENTRY;  
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);


  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateCapability(req, ikey,ctrlr_name));
  delete req;
  delete  tmp1;
  delete ival;
  delete ikey;
  delete ctrl1;
  delete key_vtn_flowfilter;
}
TEST_F (VtnFlowFilterEntryTest,Valid_NOT_READ_SUPPORTED_OPERATION) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  char ctrlr_name[]= "ctrlr";
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype =  UPLL_DT_AUDIT;
  req->operation = UNC_OP_READ_BULK;
  req->option1 = UNC_OPT1_DETAIL;   
  req->option2 = UNC_OPT2_MAC_ENTRY;  
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);


  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateCapability(req, ikey,ctrlr_name));
  delete ikey;
}
#endif
TEST_F (VtnFlowFilterEntryTest,Validate_UPLL_LOWLIST) {
   VtnFlowFilterEntryMoMgr obj;
  uint8_t l_attrs[] = "ctrl"; 
  val_vtn_flowfilter_entry_t* val_vtn_flowfilter =
                           ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
  l_attrs[unc::capa::vtn_flowfilter_entry::kCapFlowlistName] = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValVtnFlowfilterEntryAttributeSupportCheck
                    (val_vtn_flowfilter,(const uint8_t*)(l_attrs)));
  delete val_vtn_flowfilter;
}
TEST_F (VtnFlowFilterEntryTest,Validate_UPLL_ACTION) {
  VtnFlowFilterEntryMoMgr obj;
  uint8_t l_attrs[] = "ctrl"; 
  val_vtn_flowfilter_entry_t* val_vtn_flowfilter =
                           ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
  l_attrs[unc::capa::vtn_flowfilter_entry::kCapAction] = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValVtnFlowfilterEntryAttributeSupportCheck
                    (val_vtn_flowfilter,(const uint8_t*)(l_attrs)));
 delete val_vtn_flowfilter;
}
TEST_F (VtnFlowFilterEntryTest,Validate_NWM) {
  VtnFlowFilterEntryMoMgr obj;
  int8_t l_attrs[] = "ctrl"; 
  val_vtn_flowfilter_entry_t* val_vtn_flowfilter =
                           ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
  l_attrs[unc::capa::vtn_flowfilter_entry::kCapNwnName] = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValVtnFlowfilterEntryAttributeSupportCheck
                    (val_vtn_flowfilter,(const uint8_t*)(l_attrs)));
 delete val_vtn_flowfilter;
}
TEST_F (VtnFlowFilterEntryTest,Validate_UPLL_IDX_DSCP_VFFE) {
  VtnFlowFilterEntryMoMgr obj;
  uint8_t l_attrs[] = "ctrl"; 
  val_vtn_flowfilter_entry_t* val_vtn_flowfilter =
                           ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID;
  l_attrs[unc::capa::vtn_flowfilter_entry::kCapDscp] = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValVtnFlowfilterEntryAttributeSupportCheck
                    (val_vtn_flowfilter,(const uint8_t*)(l_attrs)));

 delete val_vtn_flowfilter;
}
TEST_F (VtnFlowFilterEntryTest,Validate_UPLL_IDX_PRIORITY_VFFE) {
  VtnFlowFilterEntryMoMgr obj;
  uint8_t l_attrs[] = "ctrl"; 
  val_vtn_flowfilter_entry_t* val_vtn_flowfilter =
                           ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;
  l_attrs[unc::capa::vtn_flowfilter_entry::kCapPriority] = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValVtnFlowfilterEntryAttributeSupportCheck
                    (val_vtn_flowfilter,(const uint8_t*)(l_attrs)));

 delete val_vtn_flowfilter;
}
TEST_F (VtnFlowFilterEntryTest,Validate_NOT_SUPPORTED) {
  VtnFlowFilterEntryMoMgr obj;
  uint8_t l_attrs[] = "ctrl"; 
  val_vtn_flowfilter_entry_t* val_vtn_flowfilter =
                           ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;
  l_attrs[unc::capa::vtn_flowfilter_entry::kCapPriority] = 1;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValVtnFlowfilterEntryAttributeSupportCheck
                    (val_vtn_flowfilter,(const uint8_t*)(l_attrs)));

 delete val_vtn_flowfilter;
}
TEST_F (VtnFlowFilterEntryTest,Validate_ERR_GENERIC) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t* val_vtn_flowfilter =
                           ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;
  uint8_t l_attrs[] = "ctrl"; 
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValVtnFlowfilterEntryAttributeSupportCheck
                    (val_vtn_flowfilter,(const uint8_t*)(l_attrs)));

 delete val_vtn_flowfilter;
}
TEST_F (VtnFlowFilterEntryTest, CompareValidValue_AuditTrue) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
 val_vtn_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
 
 for (uint8_t loop = 0;loop< sizeof(val_ffe1->valid); loop++) {
   val_ffe1->valid[loop] = UNC_VF_INVALID;
   val_ffe2->valid[loop] = UNC_VF_VALID;
   void *v1 = reinterpret_cast<void *>(val_ffe1);
   void *v2 = reinterpret_cast<void *>(val_ffe2);

   obj.CompareValidValue(v1, v2, true);
   EXPECT_EQ(UNC_VF_VALID_NO_VALUE,val_ffe1->valid[loop]);
 }
 free (val_ffe1);
 free (val_ffe2);
}
TEST_F (VtnFlowFilterEntryTest, CompareValidValue_ValidFlowlist) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
 val_vtn_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
 strncpy((char*) val_ffe1->flowlist_name,"FlowlistName", 32);
 strncpy((char*) val_ffe2->flowlist_name,"FlowlistName", 32);
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false);
 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]);
 free(val_ffe1);
 free(val_ffe2);
}
TEST_F (VtnFlowFilterEntryTest, CompareValidValue_ACTION) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
 val_vtn_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
 //val_ffe1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] == UNC_VF_VALID;
 strncpy((char*) val_ffe1->flowlist_name,"FlowlistName", 32);
 strncpy((char*) val_ffe2->flowlist_name,"FlowlistName", 32);
 val_ffe1->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
 val_ffe1->action = 1;
 val_ffe2->action = 1;
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false);
 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_ACTION_VFFE]);
 free(val_ffe1);
 free(val_ffe2);
}
TEST_F (VtnFlowFilterEntryTest, valid_NWM) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
 val_vtn_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
 strncpy((char*) val_ffe1->nwm_name,"nwm", 32);
 strncpy((char*) val_ffe2->nwm_name,"nwm", 32);
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false); 
 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_NWN_NAME_VFFE]);
 free(val_ffe1);
 free(val_ffe2);
}
TEST_F (VtnFlowFilterEntryTest, valid_DSCP) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
 val_vtn_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID;
 val_ffe1->dscp = 1;
 val_ffe2->dscp = 1 ;
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false); 
 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_DSCP_VFFE]);
 free(val_ffe1);
 free(val_ffe2);
}
TEST_F (VtnFlowFilterEntryTest, valid_priority) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
 val_vtn_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;
 val_ffe1->priority = 1;
 val_ffe2->priority = 1 ;
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false);
 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_PRIORITY_VFFE]);
 free(val_ffe1);
 free(val_ffe2);
}
TEST_F (VtnFlowFilterEntryTest, valid_GetRenamedUncKey) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *ival = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  char l_ctrl[] = "l_ctrl";
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",kMaxLenVtnName);
   
  strncpy(reinterpret_cast< char *>(ival->flowlist_name),
                              "flowlist",kMaxLenFlowListName);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
 
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 GetRenamedUncKey
                    (l_ikey,UPLL_DT_CANDIDATE,dmi,(uint8_t*)l_ctrl));
  
 delete l_ikey;
 
}
TEST_F (VtnFlowFilterEntryTest, valid_MergeValidate) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *ival = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  char l_ctrl[] = "l_ctrl";
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",kMaxLenVtnName);
   
  strncpy(reinterpret_cast< char *>(ival->flowlist_name),
                              "flowlist",kMaxLenFlowListName);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
 
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 MergeValidate
                    (UNC_KT_VTN_FLOWFILTER_ENTRY ,(const char*)l_ctrl,l_ikey,dmi,(upll_import_type)0));
 delete l_ikey; 
}

TEST_F (VtnFlowFilterEntryTest,Validate_commit_status_null) {
  VtnFlowFilterEntryMoMgr obj;
  typedef list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  CtrlrCommitStatusList * l_CtrlrCommitStatusList = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_UPDATED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcRecordNoMore);


  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                TxCopyCandidateToRunning
                    (UNC_KT_VTN_FLOWFILTER, l_CtrlrCommitStatusList, dmi,
                     config_mode, vtn_name));
}
TEST_F (VtnFlowFilterEntryTest,Validate_KEY_NULL) {
  VtnFlowFilterEntryMoMgr obj;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";
                               
  ConfigVal *l_cval = NULL;
   l_CtrlrTxResult->ctrlr_id.assign("ctrl",5);
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  CtrlrCommitStatusList.push_back((CtrlrCommitStatus*)l_CtrlrTxResult); 
  unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  l_cval = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  key_vtn_flowfilter_entry_t*  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,l_cval);
 
  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi, config_mode,
                     vtn_name));
}
/* commenting it as it is tested in momgr_impl_ut with some DB data
TEST_F (VtnFlowFilterEntryTest,Validate_TxUpdateController) {
  VtnFlowFilterEntryMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> affected_ctrlr_set;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter_entry_t *l_key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
//  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  val_vtn_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val_flowfilter);
  IpcRequest *ipc_req = ZALLOC_TYPE(IpcRequest);
  IPC_RESPONSE_DECL(ipc_response);

 ConfigKeyVal *l_err_ckv = NULL;

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, 
                obj.TxUpdateController(UNC_KT_VTN_FLOWFILTER_ENTRY,session_id,config_id,phase,&affected_ctrlr_set,dmi,&l_err_ckv));
 
}

TEST_F (VtnFlowFilterEntryTest,Validate_Delete) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_DELETE;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_NONE; 
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
   ikey->set_user_data((void*)ctrlr_name);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteMo(NULL, ikey,dmi));
 
}

TEST_F (VtnFlowFilterEntryTest,Validate_Update) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
   ikey->set_user_data((void*)ctrlr_name);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateMo(NULL, ikey,dmi));
}
*/
TEST_F (VtnFlowFilterEntryTest,Validate_KEY) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *l_cval = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  val_vtn_flowfilter_entry_t* val_vtn_flowfilter = 
                        ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_vtn_flowfilter);

  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
   DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->flowfilter_key.
                     vtn_key.vtn_name),
                "vtn" , kMaxLenVtnName);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,l_cval);
  std::list<controller_domain_t> list_ctrlr_dom;
  DalDmlIntf *dmi(getDalDmlIntf());
  
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 UpdateControllerTable
                    (l_ikey,UNC_OP_CREATE,UPLL_DT_CANDIDATE ,dmi,
                     list_ctrlr_dom, config_mode, vtn_name));

  //delete l_ikey;
 // delete dmi;
 
}
#if 0
TEST_F (VtnFlowFilterEntryTest, validate_UNC_KT_VTN_FLOWFILTER_ENTRY)
{
  VtnFlowFilterEntryMoMgr obj;
  char ctrl[] = "ctrl"; 
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,NULL);
 
   EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT,obj.RenameMo(req,l_ikey,dmi,(const char*)(ctrl)));
}
#endif
TEST_F (VtnFlowFilterEntryTest,Validate1_CreateCandidate) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req = NULL;
  DalDmlIntf *dmi = NULL;

EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey,dmi));
}
TEST_F (VtnFlowFilterEntryTest,Validate3_CreateCandidate) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  IPC_REQ_RESP_HEADER_DECL(req);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(NULL, ikey,dmi));
 
}
#if 0
TEST_F (VtnFlowFilterEntryTest,Validate4_CreateCandidate) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_STARTUP;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_NONE; 
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.CreateCandidateMo(req, ikey,dmi));
 
}
TEST_F (VtnFlowFilterEntryTest,Validate5_CreateCandidate) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_NONE; 
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
   ikey->set_user_data((void*)ctrlr_name);
EXPECT_EQ(UPLL_RC_ERR_INSTANCE_EXISTS, obj.CreateCandidateMo(req, ikey,dmi));
 
}
TEST_F (VtnFlowFilterEntryTest,Validate6_CreateCandidate) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_NONE; 
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcInvalidCursor);
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcGeneralError);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  //DalOdbcMgr::stub_setSingleRecordExists(false);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
   ikey->set_user_data((void*)ctrlr_name);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey,dmi));
}
/*
TEST_F (VtnFlowFilterEntryTest,Validate7_CreateCandidate) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
 // DalOdbcMgr::stub_setSingleRecordExists(true);
//  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_NONE; 
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
   ikey->set_user_data((void*)ctrlr_name);
EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.CreateCandidateMo(req, ikey,dmi));
 
}
*/
#endif

TEST_F (VtnFlowFilterEntryTest,Validate1_Delete) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req = NULL;
  DalDmlIntf *dmi = NULL;

EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteMo(req, ikey,dmi));
}
TEST_F (VtnFlowFilterEntryTest,Validate3_DeleteCandidate) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  IPC_REQ_RESP_HEADER_DECL(req);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.DeleteMo(req, ikey,dmi));
 
}
/*
TEST_F (VtnFlowFilterEntryTest,Validate4_Delete) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_STARTUP;
  req->operation = UNC_OP_DELETE;
  req->option1 = UNC_OPT1_DETAIL;   
  req->option2 = UNC_OPT2_MAC_ENTRY; 
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.DeleteMo(req, ikey,dmi));
 
}

TEST_F (VtnFlowFilterEntryTest,Validate7_Delete) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_STARTUP;
  req->operation = UNC_OP_DELETE;
  req->option1 = UNC_OPT1_DETAIL;   
  req->option2 = UNC_OPT2_MAC_ENTRY; 
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
   ikey->set_user_data((void*)ctrlr_name);
EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.DeleteMo(req, ikey,dmi));
 
}
*/
TEST_F (VtnFlowFilterEntryTest,Validate1_Update) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req = NULL;
  DalDmlIntf *dmi = NULL;

EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateMo(req, ikey,dmi));
}
#if 0
TEST_F (VtnFlowFilterEntryTest,Validate3_DeleteCandidate) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  IPC_REQ_RESP_HEADER_DECL(req);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateMo(req, ikey,dmi));
 
}
TEST_F (VtnFlowFilterEntryTest,Validate4_Delete) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_DELETE;
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.UpdateMo(req, ikey,dmi));
 
}
#endif
/*
TEST_F (VtnFlowFilterEntryTest,Validate7) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
   
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcNotDisconnected);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
   ikey->set_user_data((void*)ctrlr_name);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateMo(req, ikey,dmi));
}

TEST_F (VtnFlowFilterEntryTest,Validate8_) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  const char* version("version");
   const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
 
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcNotDisconnected);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);

  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
   ikey->set_user_data((void*)ctrlr_name);
EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateMo(req, ikey,dmi));
}
*/
TEST_F (VtnFlowFilterEntryTest, valid2_MergeValidate) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *ival = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  char l_ctrl[] = "l_ctrl";
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",kMaxLenVtnName);
   
  strncpy(reinterpret_cast< char *>(ival->flowlist_name),
                              "flowlist",kMaxLenFlowListName);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
 
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 MergeValidate
                    (UNC_KT_VTN_FLOWFILTER_ENTRY ,(const char*)l_ctrl,l_ikey,dmi,(upll_import_type)0));
  
 
}
TEST_F (VtnFlowFilterEntryTest, UpdateConfigStatusval_null) {
  VtnFlowFilterEntryMoMgr obj;

  ConfigKeyVal *l_vtn_flow_filter_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, NULL);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, NULL);
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 2;
  ConfigKeyVal *upd_key =NULL;
  DalDmlIntf *dmi(getDalDmlIntf());

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(l_vtn_flow_filter_key, 
                                        op, driver_result, upd_key, dmi,ctrlr_key));
}
TEST_F (VtnFlowFilterEntryTest, UpdateConfigStatusval_Success) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t * vtn_flowfilter_val = NULL; 
  vtn_flowfilter_val = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_ctrlr_t *val_vtn_flowfilter_ctrlr = 
                   new val_vtn_flowfilter_entry_ctrlr_t();
  vtn_flowfilter_val->cs_row_status = UNC_CS_UNKNOWN; 
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             vtn_flowfilter_val);
  val_vtn_flowfilter_ctrlr->cs_row_status = UNC_CS_UNKNOWN;
  ConfigKeyVal *l_vtn_flow_filter_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, cval);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, l_cval);
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key =NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(l_vtn_flow_filter_key, 
                                        op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F (VtnFlowFilterEntryTest, UpdateConfigSUncPartiallyApplied) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t * vtn_flowfilter_val = NULL; 
  vtn_flowfilter_val = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_ctrlr_t *val_vtn_flowfilter_ctrlr = 
                   new val_vtn_flowfilter_entry_ctrlr_t();
  vtn_flowfilter_val->cs_row_status = UNC_CS_PARTIALLY_APPLIED; 
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             vtn_flowfilter_val);
  val_vtn_flowfilter_ctrlr->cs_row_status = UNC_CS_NOT_APPLIED;
  ConfigKeyVal *l_vtn_flow_filter_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, cval);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStValVtnFlowfilterEntry,
                            NULL, l_cval);
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key =NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(l_vtn_flow_filter_key, 
                                        op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F (VtnFlowFilterEntryTest, UpdateConfigSCSApplied) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t * vtn_flowfilter_val = NULL; 
  vtn_flowfilter_val = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_ctrlr_t *val_vtn_flowfilter_ctrlr = 
                   new val_vtn_flowfilter_entry_ctrlr_t();
  vtn_flowfilter_val->cs_row_status = UNC_CS_APPLIED; 
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             vtn_flowfilter_val);
  val_vtn_flowfilter_ctrlr->cs_row_status = UNC_CS_NOT_APPLIED;
  ConfigKeyVal *l_vtn_flow_filter_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            NULL, cval);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            NULL, l_cval);
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key =NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(l_vtn_flow_filter_key, 
                                        op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F (VtnFlowFilterEntryTest, UpdateConfigSCSDefault) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t * vtn_flowfilter_val = NULL; 
  vtn_flowfilter_val = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_ctrlr_t *val_vtn_flowfilter_ctrlr = 
                   new val_vtn_flowfilter_entry_ctrlr_t();
  vtn_flowfilter_val->cs_row_status = UNC_DT_AUDIT; 
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  vtn_flowfilter_val->valid[0] = UNC_VF_NOT_SUPPORTED;
  vtn_flowfilter_val->valid[1] = UNC_VF_VALID;
  // val_vtn_flowfilter_ctrlr->valid[1] = UNC_VF_NOT_SUPPORTED;
  l_cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             vtn_flowfilter_val);
  val_vtn_flowfilter_ctrlr->cs_row_status = UNC_CS_NOT_APPLIED;
  ConfigKeyVal *l_vtn_flow_filter_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, cval);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, l_cval);
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key =NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(l_vtn_flow_filter_key, 
                                        op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F (VtnFlowFilterEntryTest, UpdateConfigS_Unc_Op_Update) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t * vtn_flowfilter_val = NULL; 
  vtn_flowfilter_val = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t * vtn_flowfilter_val2 = NULL; 
  vtn_flowfilter_val2 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_ctrlr_t *val_vtn_flowfilter_ctrlr = 
                   new val_vtn_flowfilter_entry_ctrlr_t();
  vtn_flowfilter_val->cs_row_status = UNC_DT_AUDIT; 
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  ConfigVal *l_cval2 = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             vtn_flowfilter_val);
  l_cval2 =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             vtn_flowfilter_val2);
  val_vtn_flowfilter_ctrlr->cs_row_status = UNC_CS_NOT_APPLIED;
  ConfigKeyVal *l_vtn_flow_filter_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, cval);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, l_cval);
  unc_keytype_operation_t op = UNC_OP_UPDATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key =NULL;
  upd_key  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, l_cval2);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(l_vtn_flow_filter_key, 
                                        op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F (VtnFlowFilterEntryTest, UpdateConfigS_U_Op_Update_not_supported) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t * vtn_flowfilter_val = NULL; 
  vtn_flowfilter_val = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_ctrlr_t *val_vtn_flowfilter_ctrlr = 
                   new val_vtn_flowfilter_entry_ctrlr_t();
  vtn_flowfilter_val->cs_row_status = UNC_DT_AUDIT; 
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  val_vtn_flowfilter_entry_t * vtn_flowfilter_val2 = NULL; 
  vtn_flowfilter_val2 = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_ctrlr->valid[0] = UNC_VF_NOT_SUPPORTED; 
  l_cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             vtn_flowfilter_val);
  val_vtn_flowfilter_ctrlr->cs_row_status = UNC_CS_NOT_SUPPORTED;
  ConfigKeyVal *l_vtn_flow_filter_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, cval);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, l_cval);
  unc_keytype_operation_t op = UNC_OP_UPDATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key =NULL;
  ConfigVal *l_cval2 = NULL;
  l_cval2 =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             vtn_flowfilter_val2);
  upd_key  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            NULL, l_cval2);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(l_vtn_flow_filter_key, 
                                        op, driver_result, upd_key, dmi, ctrlr_key));
}
#if 0
TEST_F (VtnFlowFilterEntryTest,Validate_KEY_NOT_NULL) {
  VtnFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *l_cval = NULL;
  val_vtn_flowfilter_entry_t* val_vtn_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_vtn_flowfilter);

  val_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,l_cval);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValidateAttribute
                    (l_ikey ,dmi));

  delete l_ikey;
  delete dmi;
}
TEST_F (VtnFlowFilterEntryTest,Validate_KEY) {
  VtnFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *l_cval = NULL;
  val_vtn_flowfilter_entry_t* val_vtn_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             val_vtn_flowfilter);

  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,l_cval);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 ValidateAttribute
                    (l_ikey ,dmi));

  delete l_ikey;
  delete dmi;
}
#endif
TEST_F (VtnFlowFilterEntryTest,ValidateMsg8_InvalidKeytype) {
  VtnFlowFilterEntryMoMgr obj;
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,NULL);

EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}
TEST_F (VtnFlowFilterEntryTest,ValidateMsg9_InvalidKeytype) {
  VtnFlowFilterEntryMoMgr obj;
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,NULL);

EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}
TEST_F (VtnFlowFilterEntryTest,Validate1_InvalidKeytype) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t *val_vtn_flowfilter = NULL;
  val_vtn_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID_NO_VALUE;
 strncpy((char*)val_vtn_flowfilter->flowlist_name,"FlowlistName", 32);
   val_vtn_flowfilter->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID ;
  val_vtn_flowfilter->action  = UPLL_FLOWFILTER_ACT_DROP; 
  ConfigVal *l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_vtn_flowfilter);
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            NULL ,l_cval);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.
   ValidateVtnFlowfilterEntryValue(l_key, req, dmi));
}
#if 0
TEST_F (VtnFlowFilterEntryTest,Validate2_InvalidKeytype) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t *val_vtn_flowfilter = NULL;
  val_vtn_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_INVALID; 
  val_vtn_flowfilter->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID_NO_VALUE;
 strncpy((char*)val_vtn_flowfilter->flowlist_name,"FlowlistName", 32);
   val_vtn_flowfilter->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_INVALID ;
  val_vtn_flowfilter->action  = UNC_VF_VALID_NO_VALUE;
  val_vtn_flowfilter->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
  val_vtn_flowfilter->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID ; 
  ConfigVal *l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_vtn_flowfilter);
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            NULL ,l_cval);
  DalDmlIntf *dmi(getDalDmlIntf());

EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.
   ValidateVtnFlowfilterEntryValue(l_key, req, dmi));
}
#endif
TEST_F (VtnFlowFilterEntryTest,Validate3_InvalidKeytype) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t *val_vtn_flowfilter = NULL;
  val_vtn_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_INVALID; 
  val_vtn_flowfilter->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID_NO_VALUE;
 strncpy((char*)val_vtn_flowfilter->flowlist_name,"FlowlistName", 32);
   val_vtn_flowfilter->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
  val_vtn_flowfilter->action  = UPLL_FLOWFILTER_ACT_PASS;
  val_vtn_flowfilter->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
  val_vtn_flowfilter->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID ;
  val_vtn_flowfilter->dscp = 70;   
  ConfigVal *l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_vtn_flowfilter);
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            NULL ,l_cval);
  DalDmlIntf *dmi(getDalDmlIntf());

EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.
   ValidateVtnFlowfilterEntryValue(l_key, req, dmi));
}
TEST_F (VtnFlowFilterEntryTest,Validate4_InvalidKeytype) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t *val_vtn_flowfilter = NULL;
  val_vtn_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_INVALID; 
  val_vtn_flowfilter->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID_NO_VALUE;
 strncpy((char*)val_vtn_flowfilter->flowlist_name,"FlowlistName", 32);
   val_vtn_flowfilter->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
  val_vtn_flowfilter->action  = UPLL_FLOWFILTER_ACT_PASS;
  val_vtn_flowfilter->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
  val_vtn_flowfilter->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID ;
  val_vtn_flowfilter->dscp = 0;  
  val_vtn_flowfilter->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID_NO_VALUE; 
   ConfigVal *l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_vtn_flowfilter);
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            NULL ,l_cval);
  DalDmlIntf *dmi(getDalDmlIntf());

EXPECT_EQ(UNC_UPLL_RC_ERR_CFG_SYNTAX, obj.
   ValidateVtnFlowfilterEntryValue(l_key, req, dmi));
}
TEST_F (VtnFlowFilterEntryTest,Validate1_TxUpdateController) {
  VtnFlowFilterEntryMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> affected_ctrlr_set;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpUpdate;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
   //key_vtn_flowfilter_entry_t *l_key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcDataError);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  //val_vtn_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  //ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val_flowfilter);
  //IpcRequest *ipc_req = ZALLOC_TYPE(IpcRequest);
  IPC_RESPONSE_DECL(ipc_response);

 ConfigKeyVal *l_err_ckv = NULL;

 DalDmlIntf *dmi(getDalDmlIntf());
 TxUpdateUtil tx(4);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, 
                obj.TxUpdateController(UNC_KT_VTN_FLOWFILTER_ENTRY,session_id,config_id,phase,
                &affected_ctrlr_set,dmi,&l_err_ckv, &tx,
                config_mode, vtn_name));
 
}
TEST_F (VtnFlowFilterEntryTest,Validate_ReadMo) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->datatype = UPLL_DT_STATE;
  req->option2 = UNC_OPT2_NONE;   
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->
                                   flowfilter_key.vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  //val_vtn_flowfilter_entry_t *val_flowfilter = 
                ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
 //ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val_flowfilter);

 //ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
		//IpctSt::kIpcStKeyVtnFlowfilterEntry, key_vtn_flowfilter, config_val);
 //DalDmlIntf *dmi(getDalDmlIntf());

// delete dmi;delete ikey; free(req);
}
/*
TEST_F (VtnFlowFilterEntryTest,Validate_ReadMo_Candidate) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter =
                  ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_vtn_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val_flowfilter);

  ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
		IpctSt::kIpcStKeyVtnFlowfilterEntry, key_vtn_flowfilter, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

   EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
   delete dmi;
   delete ikey;
   //free(req);
}

TEST_F (VtnFlowFilterEntryTest,Validate_ReadMo_UPLL_DT_STARTUP) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_STARTUP;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t );
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
   DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_vtn_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t );
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
		IpctSt::kIpcStKeyVtnFlowfilterEntry, key_vtn_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
 delete ikey; 
 
}
*/
TEST_F (VtnFlowFilterEntryTest,Validate5_InvalidKeytype) {
  VtnFlowFilterEntryMoMgr obj;
  val_vtn_flowfilter_entry_t *val_vtn_flowfilter = NULL;
  val_vtn_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter->dscp = 0;
  val_vtn_flowfilter->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID_NO_VALUE;
  //strncpy((char*)val_vtn_flowfilter->flowlist_name,"FlowlistName", 32);
   val_vtn_flowfilter->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID_NO_VALUE ;
  val_vtn_flowfilter->action  = UPLL_FLOWFILTER_ACT_DROP;
  val_vtn_flowfilter->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID_NO_VALUE;
  val_vtn_flowfilter->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID_NO_VALUE; 
  val_vtn_flowfilter->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID; 
 ConfigVal *l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_vtn_flowfilter);
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            NULL ,l_cval);
  DalDmlIntf *dmi(getDalDmlIntf());
 
EXPECT_EQ(UPLL_RC_SUCCESS, obj.
   ValidateVtnFlowfilterEntryValue(l_key, req, dmi));
}
TEST_F (VtnFlowFilterEntryTest,KT_CTRLR_UPLL_DT_STATE) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_controller_t );
 // key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
 // key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  val_vtn_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t );
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
		IpctSt::kIpcStKeyVtnFlowfilterEntry, key_vtn_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ReadMo(req, ikey, dmi));
 delete ikey; 
 
}
TEST_F (VtnFlowFilterEntryTest,Validate_UPLL_IDX_DIRECTION_FFC) {
  VtnFlowFilterEntryMoMgr obj;
  uint8_t l_attrs[] = "ctrl"; 
  val_flowfilter_controller_t* val_flowfilter_controller =
                           new val_flowfilter_controller_t();
  val_flowfilter_controller->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  l_attrs[unc::capa::vtn_flowfilter_controller::kCapDirection] = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValVtnFlowfilterCtrlAttributeSupportCheck
                    (val_flowfilter_controller,(const uint8_t*)(l_attrs)));

}
TEST_F (VtnFlowFilterEntryTest,Validate_UPLL_IDX_SEQ_NUM_FFC) {
  VtnFlowFilterEntryMoMgr obj;
  uint8_t l_attrs[] = "ctrl"; 
  val_flowfilter_controller_t* val_flowfilter_controller =
                           new val_flowfilter_controller_t();
  val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_VALID;
  l_attrs[unc::capa::vtn_flowfilter_controller::kCapSeqNum] = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValVtnFlowfilterCtrlAttributeSupportCheck
                    (val_flowfilter_controller,(const uint8_t*)(l_attrs)));

}
#if 0
TEST_F (VtnFlowFilterEntryTest,Validate_VAL_NULL) {
  VtnFlowFilterEntryMoMgr obj;
  uint8_t l_attrs[] = "ctrl"; 
  val_flowfilter_controller_t* val_flowfilter_controller = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 ValVtnFlowfilterCtrlAttributeSupportCheck
                    (val_flowfilter_controller,(const uint8_t*)(l_attrs)));

}
#endif

TEST_F (VtnFlowFilterEntryTest,Validate_UPLL_RC_SUCCESS) {
  VtnFlowFilterEntryMoMgr obj;
  uint8_t l_attrs[] = "ctrl"; 
  val_flowfilter_controller_t* val_flowfilter_controller =
                           new val_flowfilter_controller_t();
  val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_VALID;
  l_attrs[unc::capa::vtn_flowfilter_controller::kCapSeqNum] = 1;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValVtnFlowfilterCtrlAttributeSupportCheck
                    (val_flowfilter_controller,(const uint8_t*)(l_attrs)));

}
#if 0
TEST_F (VtnFlowFilterEntryTest,Validate_READ_SUPPORTED_OPERATION) {
  VtnFlowFilterEntryMoMgr obj;
  
  IPC_REQ_RESP_HEADER_DECL(req);
  char ctrlr_name[] = "ctrl"; 
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller = NULL;
  key_vtn_flowfilter_controller =(key_vtn_flowfilter_controller_t*)
              malloc(sizeof(key_vtn_flowfilter_controller_t));
  val_flowfilter_controller_t *val_flowfilter_controller = NULL;
  val_flowfilter_controller = new val_flowfilter_controller_t(); 
  ConfigVal *cval = NULL; 
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_flowfilter_controller);
 req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->option1 = UNC_OPT1_COUNT;   
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            key_vtn_flowfilter_controller , cval);
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateCapability(req, 
                                         l_key,(const char*)(ctrlr_name)));
  delete l_key;

}
TEST_F (VtnFlowFilterEntryTest,Validate_option2_pass) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  char ctrlr_name[] = "ctrl"; 
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller = NULL;
  key_vtn_flowfilter_controller =(key_vtn_flowfilter_controller_t*)
              malloc(sizeof(key_vtn_flowfilter_controller_t));
  val_flowfilter_controller_t *val_flowfilter_controller = NULL;
  val_flowfilter_controller = new val_flowfilter_controller_t(); 
  ConfigVal *cval = NULL; 
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_flowfilter_controller);
 req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_L2DOMAIN;   
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            key_vtn_flowfilter_controller , cval);
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateCapability(req, 
                                         l_key,(const char*)(ctrlr_name)));
  delete l_key;
}
TEST_F (VtnFlowFilterEntryTest,Validate_option2_ctrl_NULL) {
  VtnFlowFilterEntryMoMgr obj;
  
  IPC_REQ_RESP_HEADER_DECL(req);
  char ctrlr_name[] = "ctrl"; 
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller = NULL;
  key_vtn_flowfilter_controller =(key_vtn_flowfilter_controller_t*)
              malloc(sizeof(key_vtn_flowfilter_controller_t));
 req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_NONE;   
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            key_vtn_flowfilter_controller , NULL);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateCapability(req, 
                                         l_key,(const char*)(ctrlr_name)));
  delete l_key;
}
#endif
TEST_F (VtnFlowFilterEntryTest,_READ_SUPPORTED_FAIL) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  char ctrlr_name[] = "ctrl"; 
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller = NULL;
  key_vtn_flowfilter_controller =(key_vtn_flowfilter_controller_t*)
              malloc(sizeof(key_vtn_flowfilter_controller_t));
 req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_DELETE;
  req->option1 = UNC_OPT1_NORMAL;   
  req->option2 = UNC_OPT2_NONE;   
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            key_vtn_flowfilter_controller , NULL);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, 
                                         l_key,(const char*)(ctrlr_name)));
  delete l_key;
}
TEST_F (VtnFlowFilterEntryTest,Validate_Ctrl) {
  VtnFlowFilterEntryMoMgr obj;
  //ConfigKeyVal *ikey = NULL;
  char ctrlr_name[] = "ctrl"; 
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller = NULL;
  key_vtn_flowfilter_controller = new key_vtn_flowfilter_controller_t(); 
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                           key_vtn_flowfilter_controller , NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(req, l_key,(const char*)(ctrlr_name)));
  delete l_key;
}
TEST_F (VtnFlowFilterEntryTest,ValidateMsg_KT_VTN_FF_REQ_SUCCESS) {
  VtnFlowFilterEntryMoMgr obj;
  //ConfigKeyVal *ikey = NULL;
  val_flowfilter_controller_t *val_flowfilter_controller = NULL;
  val_flowfilter_controller = new val_flowfilter_controller_t(); 
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller = NULL;
  key_vtn_flowfilter_controller = new key_vtn_flowfilter_controller_t(); 
  strncpy((char*)(key_vtn_flowfilter_controller->vtn_key.vtn_name),"vtn",10);
  strncpy((char*)(key_vtn_flowfilter_controller->controller_name),"controller",20);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  strncpy((char*)(key_vtn_flowfilter_controller->domain_id),"domain",20);
  ConfigVal *l_cval = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_flowfilter_controller);
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL; 
  req->option2 = UNC_OPT2_NONE; 
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            key_vtn_flowfilter_controller ,l_cval);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, l_key));
 delete l_key;
}
 TEST_F (VtnFlowFilterEntryTest,KT_VTN_FF_REQ_SUCCESS) {
  VtnFlowFilterEntryMoMgr obj;
  val_flowfilter_controller_t *val_flowfilter_controller = NULL;
  val_flowfilter_controller = new val_flowfilter_controller_t(); 
  val_flowfilter_controller->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_VALID;
  val_flowfilter_controller->direction = 1;
  val_flowfilter_controller->sequence_num = 1;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
  ValidateVtnFlowfilterControllerValue(val_flowfilter_controller,(uint32_t)UNC_OP_UPDATE));
}
 TEST_F (VtnFlowFilterEntryTest,KT_VTN_FF_) {
  VtnFlowFilterEntryMoMgr obj;
  val_flowfilter_controller_t *val_flowfilter_controller = NULL;
  val_flowfilter_controller = new val_flowfilter_controller_t(); 
  val_flowfilter_controller->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_INVALID;
  val_flowfilter_controller->direction = 1;
  val_flowfilter_controller->sequence_num = 1;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
  ValidateVtnFlowfilterControllerValue(val_flowfilter_controller,(uint32_t)UNC_OP_UPDATE));
}
 TEST_F (VtnFlowFilterEntryTest,KT_VTN_FF_NUM_FAIL_SEQ) {
  VtnFlowFilterEntryMoMgr obj;
  val_flowfilter_controller_t *val_flowfilter_controller = NULL;
  val_flowfilter_controller = new val_flowfilter_controller_t(); 
  val_flowfilter_controller->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_INVALID;
  val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_VALID;
  val_flowfilter_controller->direction = 1;
  val_flowfilter_controller->sequence_num = 0;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.
  ValidateVtnFlowfilterControllerValue(val_flowfilter_controller,(uint32_t)UNC_OP_UPDATE));
}
 TEST_F (VtnFlowFilterEntryTest,KT_VTN_FF_NUM_FAIL_DIR) {
  VtnFlowFilterEntryMoMgr obj;
  val_flowfilter_controller_t *val_flowfilter_controller = NULL;
  val_flowfilter_controller = new val_flowfilter_controller_t(); 
  val_flowfilter_controller->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_VALID;
  val_flowfilter_controller->direction = 0;
  val_flowfilter_controller->sequence_num = 0;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.
  ValidateVtnFlowfilterControllerValue(val_flowfilter_controller,(uint32_t)UNC_OP_UPDATE));
}
TEST_F (VtnFlowFilterEntryTest,Ctrl_ReadMo_DT_STATE) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter = 
                  ZALLOC_TYPE(key_vtn_flowfilter_controller_t );
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  //key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  //key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_controller_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_controller_t);
  val_flowfilter->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  val_flowfilter->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_INVALID;
  val_flowfilter->direction = 1;
  val_flowfilter->sequence_num = 1;
 
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterController, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
		IpctSt::kIpcStKeyVtnFlowfilterController, key_vtn_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UNC_UPLL_RC_ERR_CFG_SYNTAX, obj.ReadMo(req, ikey, dmi));
// delete dmi;delete ikey; free(req);
 
}
TEST_F (VtnFlowFilterEntryTest,Ctrl_ReadMo_DT_STATE_GetreadPass) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter = 
                  ZALLOC_TYPE(key_vtn_flowfilter_controller_t );
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->controller_name),
		"ctrl" , kMaxLenVtnName);
  //key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  //key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_controller_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_controller_t);
  val_flowfilter->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  val_flowfilter->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_INVALID;
  val_flowfilter->direction = 1;
  val_flowfilter->sequence_num = 1;
  char ctrlr_name[]= "ctrlr";
   const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);

  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterController, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
		IpctSt::kIpcStKeyVtnFlowfilterController, key_vtn_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UNC_UPLL_RC_ERR_CFG_SYNTAX, obj.ReadMo(req, ikey, dmi));
// delete dmi;delete ikey; free(req);
 
}
#if 0
TEST_F (VtnFlowFilterEntryTest,Validate_ReadMo_UPLL_DT_STATE) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t );
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SIBLING_BEGIN,kDalRcSuccess);
  val_vtn_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t );
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
		IpctSt::kIpcStKeyVtnFlowfilterEntry, key_vtn_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadSiblingMo(req, ikey,true,dmi));
// delete dmi;delete ikey; free(req);
 
}
#endif
TEST_F (VtnFlowFilterEntryTest,Ctrl_Readbegin_DT_STATE) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter =
                  ZALLOC_TYPE(key_vtn_flowfilter_controller_t );
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  //key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  //key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SIBLING_BEGIN,kDalRcSuccess);
  val_flowfilter_controller_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_controller_t);
  val_flowfilter->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  val_flowfilter->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_INVALID;
  val_flowfilter->direction = 1;
  val_flowfilter->sequence_num = 1;
 
 //ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterController, val_flowfilter);

 //ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
	//	IpctSt::kIpcStKeyVtnFlowfilterController, key_vtn_flowfilter, config_val);
 //DalDmlIntf *dmi(getDalDmlIntf());

// delete dmi;delete ikey; free(req);
 
}
TEST_F (VtnFlowFilterEntryTest,ReadMo_For_Ctrl_UPLL_DT_STATE) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ_SIBLING;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t );
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SIBLING_COUNT,kDalRcSuccess);
  //val_vtn_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t );
 //ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val_flowfilter);

 //ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
	//	IpctSt::kIpcStKeyVtnFlowfilterEntry, key_vtn_flowfilter, config_val);
 //DalDmlIntf *dmi(getDalDmlIntf());

// delete dmi;delete ikey; free(req);
 
}
#if 0
TEST_F (VtnFlowFilterEntryTest,Ctrl_DT_STATE_OPT_DETAIL) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter = 
                  ZALLOC_TYPE(key_vtn_flowfilter_controller_t );
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  //key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  //key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SIBLING_BEGIN,kDalRcSuccess);
  val_flowfilter_controller_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_controller_t);
    //val_flowfilter->valid[UPLL_IDX_SEQ_NUM_VFFCS] = UNC_VF_VALID ;

   val_flowfilter->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  val_flowfilter->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_INVALID;
  val_flowfilter->direction = 1;
  val_flowfilter->sequence_num = 1;
 
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterController, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
		IpctSt::kIpcStKeyVtnFlowfilterController, key_vtn_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadSiblingMo(req, ikey,true, dmi));
// delete dmi;delete ikey; free(req);
 
}

TEST_F (VtnFlowFilterEntryTest,Ctrl_ReadMo_DT_STATE_DETAIL) {
  VtnFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter = 
                  ZALLOC_TYPE(key_vtn_flowfilter_controller_t );
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  //key_vtn_flowfilter->flowfilter_key.input_direction = UPLL_FLOWFILTER_DIR_IN;
  //key_vtn_flowfilter->sequence_num = 1;
  //i = 0;
 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_controller_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_controller_t);
  val_flowfilter->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  val_flowfilter->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_INVALID;
  val_flowfilter->direction = 1;
  val_flowfilter->sequence_num = 1;
 
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterController, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
		IpctSt::kIpcStKeyVtnFlowfilterController, key_vtn_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 std::cout<<"***raka***********\n";
 // EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
//TODO: obj.ReadRecord(req, ikey, dmi,UNC_OP_READ_SIBLING);
// delete dmi;delete ikey; free(req);
 
}
#endif
TEST_F (VtnFlowFilterEntryTest, ctrl_id_Fail) {
 VtnFlowFilterEntryMoMgr obj;
 val_vtn_flowfilter_entry_t *ival = ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  //char l_ctrl[] = "l_ctrl";
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = NULL;
  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          flowfilter_key.vtn_key.vtn_name),"VTN",kMaxLenVtnName);
   
  strncpy(reinterpret_cast< char *>(ival->flowlist_name),
                              "flowlist",kMaxLenFlowListName);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,tmp1);
 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 GetRenamedUncKey
                    (l_ikey,UPLL_DT_CANDIDATE,dmi,NULL));
  
 
}
TEST_F (VtnFlowFilterEntryTest, KEY_TYPE_FAIL_)
{
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_req = NULL;
  key_vtn_flowfilter_entry_t *vtn_flowfilterkey =
        ZALLOC_TYPE(key_vtn_flowfilter_entry_t);

  ConfigVal *tmp1 = NULL;
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);

  l_req = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, 
IpctSt::kIpcStKeyVtnFlowfilterEntry,
          vtn_flowfilterkey, tmp1);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(l_okey, l_req,MAINTBL));
  delete l_req;
}
TEST_F (VtnFlowFilterEntryTest,Invalid_op) {
  VtnFlowFilterEntryMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> affected_ctrlr_set;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpInvalid;
  //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
  key_vtn_flowfilter_entry_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t );
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
   //key_vtn_flowfilter_entry_t *l_key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t );
//  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  //val_vtn_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_vtn_flowfilter_entry_t );
  //ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val_flowfilter);
  //IpcRequest *ipc_req = ZALLOC_TYPE(IpcRequest);
  IPC_RESPONSE_DECL(ipc_response);

 ConfigKeyVal *l_err_ckv = NULL;

 DalDmlIntf *dmi(getDalDmlIntf());
 TxUpdateUtil tx(4);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, 
                obj.TxUpdateController(UNC_KT_VTN_FLOWFILTER_ENTRY,session_id,config_id,phase,&affected_ctrlr_set,dmi,&l_err_ckv, &tx,
                                       config_mode, vtn_name));
 
}
TEST_F (VtnFlowFilterEntryTest,DELETE_FAIL) {
  VtnFlowFilterEntryMoMgr obj;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  CtrlrCommitStatusList.clear();
  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  ConfigVal *l_cval = NULL;
  // l_CtrlrTxResult->ctrlr_id.assign("ctrl",5);
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  //CtrlrCommitStatusList.push_back((CtrlrCommitStatus*)l_CtrlrTxResult); 
  unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  l_cval = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  key_vtn_flowfilter_entry_t*  key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_entry_t);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,l_cval);
 
  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi, config_mode, vtn_name));
}
TEST_F (VtnFlowFilterEntryTest,DELETE_Pass) {
  VtnFlowFilterEntryMoMgr obj;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";
                                
  ConfigVal *l_cval = NULL;
   l_CtrlrTxResult->ctrlr_id.assign("ctrl",5);
   DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcRecordNoMore);

  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  CtrlrCommitStatusList.push_back((CtrlrCommitStatus*)l_CtrlrTxResult); 
  unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  val_vtn_flowfilter_entry_t *ival =
                         ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  l_cval = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             ival);
  key_vtn_flowfilter_entry_t*  key_vtn_flowfilter = new key_vtn_flowfilter_entry_t();
  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            key_vtn_flowfilter,l_cval);
 
  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi, config_mode, vtn_name));
}

/*********PC*******/

TEST_F(VtnFlowFilterEntryTest, DeleteChildrenPOM_1) {
  VtnFlowFilterEntryMoMgr obj;
  key_vtn_flowfilter_entry_t *vtn_ffe_key =
      reinterpret_cast<key_vtn_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vtn_ffe_key->flowfilter_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  vtn_ffe_key->flowfilter_key.input_direction = 0xFE;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVtnFlowfilterEntry,
                                        vtn_ffe_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalDmlIntf *dmi(getDalDmlIntf());
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST_F(VtnFlowFilterEntryTest, DeleteChildrenPOM_2) {
  VtnFlowFilterEntryMoMgr obj;
  key_vtn_flowfilter_entry_t *vtn_ffe_key =
      reinterpret_cast<key_vtn_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vtn_ffe_key->flowfilter_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  vtn_ffe_key->flowfilter_key.input_direction = 0x0;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVtnFlowfilterEntry,
                                        vtn_ffe_key, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalDmlIntf *dmi(getDalDmlIntf());
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST_F(VtnFlowFilterEntryTest, DeleteChildrenPOM_3) {
  VtnFlowFilterEntryMoMgr obj;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVtnFlowfilterEntry,
                                        NULL, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE,
                                 kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY, kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalDmlIntf *dmi(getDalDmlIntf());
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST_F(VtnFlowFilterEntryTest, DeleteChildrenPOM_4) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST_F(VtnFlowFilterEntryTest, DeleteChildrenPOM_5) {
  VtnFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVtnFlowfilterEntry,
                                        NULL, NULL);
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
