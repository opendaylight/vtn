/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <gtest/gtest.h>
#include <upll/vtn_flowfilter_momgr.hh>
#include <upll/config_mgr.hh>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <capa_intf.hh>
#include <capa_module_stub.hh>
#include <ctrlr_mgr.hh>
#include <config_mgr.hh>
#include <tclib_module.hh>
#include <list>
#include "ut_util.hh"
#include  "tx_update_util.hh"

//bool fatal_done;
//pfc::core::Mutex  fatal_mutex_lock;

int i_d =0;
using namespace std;
using namespace unc::capa;
using namespace unc::upll::dal;
using namespace unc::upll::config_momgr;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::test;
using namespace unc::upll::tx_update_util;

class VtnFlowfilter : public UpllTestEnv
{

};

TEST_F (VtnFlowfilter, OutputNull)
{
  VtnFlowFilterMoMgr obj;
  BindInfo *bin = NULL ;
  int nattr ;

  obj.GetRenameKeyBindInfo(UNC_KT_VTN_FLOWFILTER, bin, nattr, MAINTBL);
  EXPECT_EQ(3,nattr);
}

TEST_F (VtnFlowfilter, OutputCtrlNull)
{
   VtnFlowFilterMoMgr obj ;
   BindInfo *l_bin = NULL;
   int nattr ;
   EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(UNC_KT_VTN_FLOWFILTER, l_bin, nattr, CTRLRTBL));
}

TEST_F (VtnFlowfilter, OutputRenameNull)
{
  VtnFlowFilterMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VTN_FLOWFILTER;
  BindInfo *bin = NULL;
  int nattr = 2;
  cout<< "Bug For rename Table" << endl;
  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, RENAMETBL));
}
TEST_F (VtnFlowfilter, AllocErr)
{
  VtnFlowFilterMoMgr obj;
  ConfigVal *l_ck_val;
  val_flowfilter_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
  l_ck_val  = new ConfigVal(IpctSt::kIpcStValFlowfilter,val_flowfilter);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AllocVal(l_ck_val, UPLL_DT_STATE, MAINTBL));
}
TEST_F (VtnFlowfilter, AllocMaintblSuccess)
{
  VtnFlowFilterMoMgr obj;
  ConfigVal *l_ck_val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(l_ck_val, UPLL_DT_STATE, MAINTBL));
}
TEST_F (VtnFlowfilter, AllocCtrlSuccess)
{
  VtnFlowFilterMoMgr obj;
  ConfigVal *l_ck_val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(l_ck_val, UPLL_DT_STATE, CTRLRTBL));
}
#if 0
TEST_F (VtnFlowfilter, AllocRenameSuccess)
{
  VtnFlowFilterMoMgr obj;
  ConfigVal *l_ck_val = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(l_ck_val, UPLL_DT_STATE, RENAMETBL));
}
#endif
TEST_F (VtnFlowfilter, ValidnameSuccess)
{
  VtnFlowFilterMoMgr obj;
  void* l_key = NULL;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  l_key = (void*) (key_vtn_flowfilter);
  strcpy((char*)(key_vtn_flowfilter->vtn_key.vtn_name),"vtn");
  unsigned long index = uudst::vtn_flowfilter::kDbiVtnName;
  EXPECT_EQ(PFC_TRUE, obj.IsValidKey(l_key, index));
}
TEST_F (VtnFlowfilter, Valid_name_fail)
{
  VtnFlowFilterMoMgr obj;
  void* l_key = NULL;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  l_key = (void*) (key_vtn_flowfilter);
  strcpy((char*)(key_vtn_flowfilter->vtn_key.vtn_name)," ");
  unsigned long index = uudst::vtn_flowfilter::kDbiVtnName;
  EXPECT_EQ(PFC_FALSE, obj.IsValidKey(l_key, index));
  index = 0;
}
TEST_F (VtnFlowfilter, Valid_dir_success)
{
  VtnFlowFilterMoMgr obj;
  void* l_key = NULL;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  l_key = (void*) (key_vtn_flowfilter);

  key_vtn_flowfilter->input_direction= 'a';

  unsigned long index = uudst::vtn_flowfilter::kDbiInputDirection;
  EXPECT_EQ(PFC_FALSE, obj.IsValidKey(l_key, index));
}
TEST_F (VtnFlowfilter, Default_success)
{
  VtnFlowFilterMoMgr obj;
  void* l_key = NULL;
  unsigned long index = uudst::vtn_flowfilter::kDbiVtnFlowFilterNumCols;
  EXPECT_EQ(PFC_TRUE, obj.IsValidKey(l_key, index));
}
TEST_F (VtnFlowfilter, Request_Null_success)
{
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(l_okey, l_req,MAINTBL));
}

TEST_F (VtnFlowfilter, Request_okey_success)
{
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_req = NULL;
  key_vtn_flowfilter_t *vtn_flowfilterkey = ZALLOC_TYPE(key_vtn_flowfilter_t);
  key_vtn_flowfilter_t *l_vtn_flowfilterkey = ZALLOC_TYPE(key_vtn_flowfilter_t);
  
  l_okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVtnFlowfilter,
                          l_vtn_flowfilterkey, NULL);
  l_req = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::kIpcStKeyVtnFlowfilter,
                          vtn_flowfilterkey, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(l_okey, l_req,MAINTBL));
}
TEST_F (VtnFlowfilter, Request_key_success)
{
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_req = NULL;
  key_vtn_flowfilter_t *vtn_flowfilterkey = ZALLOC_TYPE(key_vtn_flowfilter_t);


  l_okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtnFlowfilter,
          vtn_flowfilterkey, NULL);
  l_req = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
IpctSt::kIpcStKeyVtnFlowfilterEntry,
          vtn_flowfilterkey, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(l_okey, l_req,MAINTBL));
}
TEST_F (VtnFlowfilter, RequestmaintblSuccess)
{
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_req = NULL;
  key_vtn_flowfilter_t *vtn_flowfilterkey = ZALLOC_TYPE(key_vtn_flowfilter_t);

  val_flowfilter_t *ival = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal *tmp1 = NULL;
  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             ival);
  l_req = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::kIpcStKeyVtnFlowfilter,
                          vtn_flowfilterkey ,tmp1 );
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(l_okey, l_req,MAINTBL));
}
TEST_F (VtnFlowfilter, RequestctrltblSuccess)
{
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_req = NULL;
  key_vtn_flowfilter_t *vtn_flowfilterkey = ZALLOC_TYPE(key_vtn_flowfilter_t);
  
  val_flowfilter_t *ival = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal *tmp1 = NULL;
  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             ival);
  l_req = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::kIpcStKeyVtnFlowfilter,
                          vtn_flowfilterkey ,tmp1 );
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(l_okey, l_req,CTRLRTBL));
}
TEST_F (VtnFlowfilter, ReqParentkeyNull)
{
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_parent_key = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(l_okey, l_parent_key));
}
TEST_F (VtnFlowfilter, ReqParentkeyNotNull)
{
  VtnFlowFilterMoMgr obj;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_parent_key = NULL;
  key_vtn_flowfilter_t *vtn_flowfilter_key = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy((char*)(vtn_flowfilter_key->vtn_key.vtn_name),"vtn",10);
  l_parent_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::kIpcStKeyVtnFlowfilter,
                          vtn_flowfilter_key, NULL);
 result_code=  obj.GetChildConfigKey(l_okey, l_parent_key);
  EXPECT_EQ(UPLL_RC_SUCCESS, result_code);
}
TEST_F (VtnFlowfilter, ReqKeyof_ParentkeyNull)
{
  VtnFlowFilterMoMgr obj;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *l_okey = NULL;
  ConfigKeyVal *l_parent_key = NULL;
  l_parent_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::kIpcStKeyVtnFlowfilter,
                          NULL, NULL);
  result_code=  obj.GetChildConfigKey(l_okey, l_parent_key);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, result_code);
}
TEST_F(VtnFlowfilter, CopyToConfigkeyVal_Ikey_NUll) {
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
}
TEST_F(VtnFlowfilter, CopyToConfigkeyVal_InValid) {
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_rename, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

}
TEST_F(VtnFlowfilter, CopyToConfigkey_InValid) {
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  strncpy((char*) key_rename->old_unc_vtn_name,"OLDVTNNAME",32);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_rename, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.CopyToConfigKey(okey,ikey));
}
TEST_F(VtnFlowfilter, MerGeValidate_Success) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  const char* ctrlr_id = {0};
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.MergeValidate(UNC_KT_VTN_FLOWFILTER,
               ctrlr_id,ikey,dmi,(upll_import_type)0));
}

TEST_F(VtnFlowfilter, UpdateMo_NoOperation) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.UpdateMo(req,ikey,dmi));
}
TEST_F(VtnFlowfilter, RenameMo_NoOperation) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  const char* ctrlr_id = {0};
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.RenameMo(req,ikey,dmi,ctrlr_id));
}
TEST_F(VtnFlowfilter, UpdateConfigStatusval_null) {
  VtnFlowFilterMoMgr obj;

  ConfigKeyVal *l_vtn_flow_filter_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            NULL, NULL);
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 2;
  ConfigKeyVal *upd_key =NULL;
  DalDmlIntf *dmi(getDalDmlIntf());

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(l_vtn_flow_filter_key,
                                        op, driver_result, upd_key, dmi,ctrlr_key));
}
TEST_F(VtnFlowfilter, UpdateConfigStatusval_Success) {
  VtnFlowFilterMoMgr obj;
  val_flowfilter_t *vtn_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  val_vtn_flowfilter_ctrlr_t *val_vtn_flowfilter_ctrlr =ZALLOC_TYPE(val_vtn_flowfilter_ctrlr_t);
  vtn_flowfilter_val->cs_row_status = UNC_CS_UNKNOWN;
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vtn_flowfilter_val);
  val_vtn_flowfilter_ctrlr->cs_row_status = UNC_CS_PARTIALLY_APPLIED;
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
TEST_F(VtnFlowfilter, UpdateConfigSUncPartiallyApplied) {
  VtnFlowFilterMoMgr obj;
  val_flowfilter_t *vtn_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  val_vtn_flowfilter_ctrlr_t *val_vtn_flowfilter_ctrlr =ZALLOC_TYPE(val_vtn_flowfilter_ctrlr_t);
  vtn_flowfilter_val->cs_row_status = UNC_CS_PARTIALLY_APPLIED;
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
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
TEST_F(VtnFlowfilter, UpdateConfigSCSApplied) {
  VtnFlowFilterMoMgr obj;
  val_flowfilter_t *vtn_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  val_vtn_flowfilter_ctrlr_t *val_vtn_flowfilter_ctrlr =ZALLOC_TYPE(val_vtn_flowfilter_ctrlr_t);
  vtn_flowfilter_val->cs_row_status = UNC_CS_APPLIED;
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
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
TEST_F(VtnFlowfilter, UpdateConfigSCSDefault) {
  VtnFlowFilterMoMgr obj;
  val_flowfilter_t *vtn_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  val_vtn_flowfilter_ctrlr_t *val_vtn_flowfilter_ctrlr =ZALLOC_TYPE(val_vtn_flowfilter_ctrlr_t); 
  vtn_flowfilter_val->cs_row_status = UNC_DT_AUDIT; 
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
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
TEST_F(VtnFlowfilter, UpdateConfigS_Unc_Op_Update) {
  VtnFlowFilterMoMgr obj;
  val_flowfilter_t *vtn_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  val_vtn_flowfilter_ctrlr_t *val_vtn_flowfilter_ctrlr =ZALLOC_TYPE(val_vtn_flowfilter_ctrlr_t); 
  vtn_flowfilter_val->cs_row_status = UNC_DT_AUDIT; 
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vtn_flowfilter_val);
  val_vtn_flowfilter_ctrlr->cs_row_status = UNC_CS_NOT_APPLIED;
  ConfigKeyVal *l_vtn_flow_filter_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            NULL, cval);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            NULL, l_cval);
  unc_keytype_operation_t op = UNC_OP_UPDATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key =NULL;
  DalDmlIntf *dmi(getDalDmlIntf());

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(l_vtn_flow_filter_key, 
                                        op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F(VtnFlowfilter, UpdateConfigS_U_Op_Update_not_supported) {
  VtnFlowFilterMoMgr obj;
  val_flowfilter_t *vtn_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  val_vtn_flowfilter_ctrlr_t *val_vtn_flowfilter_ctrlr =ZALLOC_TYPE(val_vtn_flowfilter_ctrlr_t); 
  vtn_flowfilter_val->cs_row_status = UNC_DT_AUDIT; 
  ConfigVal *cval = NULL;
  ConfigVal *l_cval = NULL;
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter_ctrlr);
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vtn_flowfilter_val);
  val_vtn_flowfilter_ctrlr->cs_row_status = UNC_CS_NOT_SUPPORTED;
  ConfigKeyVal *l_vtn_flow_filter_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            NULL, cval);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            NULL, l_cval);
  unc_keytype_operation_t op = UNC_OP_UPDATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key =NULL;
  DalDmlIntf *dmi(getDalDmlIntf());

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(l_vtn_flow_filter_key, 
                                        op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F(VtnFlowfilter,ValidateMsg_UPLL_RC_ERR_CFG_SYNTAX) {
  VtnFlowFilterMoMgr obj;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStValFlowfilter,
                           key_vtn_flowfilter , NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, l_key));
}
TEST_F(VtnFlowfilter,ValidateMsg_KEY_VTN_FF_NOT_NULL) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            NULL ,NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, l_key));
}
TEST_F(VtnFlowfilter,ValidateMsg_NOT_VALID_KT) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            NULL ,NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, l_key));
}
TEST_F(VtnFlowfilter,ValidateMsg_NUMERICRANGE_FAIL) {
  VtnFlowFilterMoMgr obj;

  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  IPC_REQ_RESP_HEADER_DECL(req);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  strncpy((char*)(key_vtn_flowfilter->vtn_key.vtn_name),"vtn",10);
  key_vtn_flowfilter->input_direction = 1;
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                           key_vtn_flowfilter , NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, l_key));
}
TEST_F(VtnFlowfilter,ValidateMsg_KT_VTN_FF_CTRL_SUCCESS) {
  VtnFlowFilterMoMgr obj;
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller =ZALLOC_TYPE(key_vtn_flowfilter_controller_t);
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                           key_vtn_flowfilter_controller , NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, l_key));
  l_key = NULL;
}
TEST_F(VtnFlowfilter,ValidateMsg_KT_VTN_FF_SUCCESS) {
  VtnFlowFilterMoMgr obj;
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller =ZALLOC_TYPE(key_vtn_flowfilter_controller_t);
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                           key_vtn_flowfilter_controller , NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, l_key));
  l_key = NULL;
}
TEST_F(VtnFlowfilter,ValidateMsg_dt_type_UNC_DT_CANDIDATE) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller =ZALLOC_TYPE(key_vtn_flowfilter_controller_t);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  strncpy((char*)(key_vtn_flowfilter_controller->vtn_key.vtn_name),"vtn",10);
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_ARP_ENTRY;
  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            key_vtn_flowfilter_controller ,NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, l_key));
}
TEST_F(VtnFlowfilter,ValidateMsg_REQ_KEY_NULL) {
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  char l_ctrl[] = "ctrl";
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(req,
                                         ikey,(const char*)(l_ctrl)));

}
TEST_F(VtnFlowfilter,ValidateMsg_REQ_KEY_NOT_NULL) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  char l_ctrl[] = "ctrl";
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);

  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter , NULL);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req,
                                         l_key,(const char*)(l_ctrl)));

}
TEST_F(VtnFlowfilter,Validate_READ_SUPPORTED_OPERATION) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  char ctrlr_name[] = "ctrl";
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller = ZALLOC_TYPE(key_vtn_flowfilter_controller_t);
  val_flowfilter_controller_t *val_flowfilter_controller =ZALLOC_TYPE(val_flowfilter_controller_t);
  ConfigVal *cval = NULL;
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_flowfilter_controller);
 req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_COUNT;
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);


  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            key_vtn_flowfilter_controller , cval);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req,
                                         l_key,(const char*)(ctrlr_name)));

}
TEST_F(VtnFlowfilter,Validate_VAL_NULL) {
  VtnFlowFilterMoMgr obj;
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase  = uuc::kUpllUcpCreate;
  ConfigKeyVal *ckv_running = NULL;
 ckv_running  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStValFlowfilterController,
                            NULL,NULL);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 UpdateAuditConfigStatus
                    (cs_status ,phase,ckv_running,dmi));

}
TEST_F(VtnFlowfilter,Validate_KEY_NULL) {
  VtnFlowFilterMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStValFlowfilterController,
                            NULL,NULL);

  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
            IsReferenced(req, l_ikey, dmi));
}

TEST_F(VtnFlowfilter,Validate_KEY_NOT_NULL) {
  VtnFlowFilterMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *l_cval = NULL;
  val_flowfilter_t *val_vtn_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter);
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_CANDIDATE;

  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,l_cval);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 IsReferenced
                    (req, l_ikey,dmi));

}
TEST_F(VtnFlowfilter,Consolidated_KEY_NOT_NULL) {
  VtnFlowFilterMoMgr obj;
  ConfigVal *l_cval = NULL;
  val_flowfilter_t *val_vtn_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter);
  DalDmlIntf *dmi(getDalDmlIntf());
  i_d = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                           key_vtn_flowfilter,l_cval);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 SetConsolidatedStatus
                    (l_ikey,dmi));

}
TEST_F(VtnFlowfilter,Validate_KEY_UPDATE) {
  VtnFlowFilterMoMgr obj;
  ConfigVal *l_cval = NULL;
  val_flowfilter_t *val_vtn_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter);
  DalDmlIntf *dmi(getDalDmlIntf());
  i_d = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,l_cval);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 SetConsolidatedStatus
                    (l_ikey,dmi));

}

TEST_F(VtnFlowfilter,Consolidated_KEY_NULL) {
  VtnFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *l_ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 SetConsolidatedStatus
                    (l_ikey,dmi));
}
TEST_F(VtnFlowfilter,Validate_DELETE_UPDCONFG) {
  VtnFlowFilterMoMgr obj;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;
 // strncpy()
  ConfigVal *l_cval = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  CtrlrCommitStatusList.push_back(l_CtrlrTxResult);
  unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  val_flowfilter_t *val_vtn_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter);
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  DalDmlIntf *dmi(getDalDmlIntf());
 ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,l_cval);
 
  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi, config_mode,
                      vtn_name));
}
TEST_F(VtnFlowfilter,Validate_option2_fail) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  char ctrlr_name[] = "ctrl";
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller = ZALLOC_TYPE(key_vtn_flowfilter_controller_t);
  val_flowfilter_controller_t *val_flowfilter_controller = ZALLOC_TYPE(val_flowfilter_controller_t);
  ConfigVal *cval = NULL;
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_flowfilter_controller);
 req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
 CapaModuleStub::stub_loadCapaModule();
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            key_vtn_flowfilter_controller , cval);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req,
                                         l_key,(const char*)(ctrlr_name)));
}
TEST_F(VtnFlowfilter,UPDATE_pass) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  char ctrlr_name[] = "ctrl";
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller = ZALLOC_TYPE(key_vtn_flowfilter_controller_t);
  val_flowfilter_controller_t *val_flowfilter_controller = ZALLOC_TYPE(val_flowfilter_controller_t);
  ConfigVal *cval = NULL;
  cval =  new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                             val_flowfilter_controller);
 req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);


  ConfigKeyVal *l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                            IpctSt::kIpcStKeyVtnFlowfilterController,
                            key_vtn_flowfilter_controller , cval);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req,
                                         l_key,(const char*)(ctrlr_name)));
}
TEST_F(VtnFlowfilter, valid_GetRenamedUncKey) {
 VtnFlowFilterMoMgr obj;
  val_flowfilter_t *ival = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  char l_ctrl[] = "l_ctrl";
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          vtn_key.vtn_name),"VTN",kMaxLenVtnName);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             ival);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 GetRenamedUncKey
                    (l_ikey,UPLL_DT_CANDIDATE,dmi,(uint8_t*)l_ctrl));

}

TEST_F(VtnFlowfilter, valid_CreateCandidateMo) {
 VtnFlowFilterMoMgr obj;
  i_d =1;
  val_flowfilter_t *ival = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal *tmp1 = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_CREATE;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          vtn_key.vtn_name),"VTN",kMaxLenVtnName);

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  strncpy((char*)(key_vtn_flowfilter->vtn_key.vtn_name),"vtn",10);
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcParentNotFound);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNoMore);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
  //DalOdbcMgr::stub_setSingleRecordExists(false);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  key_vtn_flowfilter->input_direction = 1;
  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             ival);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                           key_vtn_flowfilter,tmp1);

  EXPECT_EQ(UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID, obj.
                 CreateCandidateMo
                    (req,l_ikey,dmi));
DalOdbcMgr::clearStubData();

}
/*
TEST_F(VtnFlowfilter,Validate_ReadMo) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_RUNNING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  key_vtn_flowfilter_t *vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  vtn_flowfilter->input_direction = 1;
  //i_d = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
		IpctSt::kIpcStKeyVtnFlowfilter, vtn_flowfilter, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
}

TEST_F(VtnFlowfilter,Validate_ReadMo_DTSTATE) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter->input_direction = 1;
  //i_d = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
		IpctSt::kIpcStKeyVtnFlowfilter, key_vtn_flowfilter, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
}
*/
TEST_F(VtnFlowfilter,Validate_ReadMo_UPLL_DT_STATE) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter->input_direction = 1;
  //i_d = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  val_flowfilter_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
		IpctSt::kIpcStKeyVtnFlowfilter, key_vtn_flowfilter, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ReadMo(req, ikey, dmi));

}
/*
TEST_F(VtnFlowfilter,Validate_ReadSibling_UPLL_DT_STATE) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  bool begin = true;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter->input_direction = 1;
  //i_d = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SIBLING_BEGIN,kDalRcSuccess);
  val_flowfilter_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
		IpctSt::kIpcStKeyVtnFlowfilter, key_vtn_flowfilter, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                obj.ReadSiblingMo(req, ikey ,begin,dmi));

}

TEST_F(VtnFlowfilter,Validate_ReadMo_UPLL_BEGIN_FALSE) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ_SIBLING;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  bool begin = false;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter->input_direction = 1;
  //i_d = 0;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SIBLING_COUNT,kDalRcSuccess);
  val_flowfilter_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
		IpctSt::kIpcStKeyVtnFlowfilter, key_vtn_flowfilter, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                obj.ReadSiblingMo(req, ikey ,begin,dmi));

}

TEST_F(VtnFlowfilter,Validate_ReadMo_UPLL_IMPORT) {
  VtnFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ_SIBLING;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  bool begin = false;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  key_vtn_flowfilter->input_direction = 1;
 // i_d = 0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SIBLING_COUNT,kDalRcSuccess);
  val_flowfilter_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
 ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
		IpctSt::kIpcStKeyVtnFlowfilter, key_vtn_flowfilter, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                obj.ReadSiblingMo(req, ikey ,begin,dmi));

}
*/
TEST_F(VtnFlowfilter,Attribute_KEY_NOT_NULL) {
  VtnFlowFilterMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *l_cval = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  val_flowfilter_t *val_vtn_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter);

  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,l_cval);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 ValidateAttribute
                    (l_ikey ,dmi,req));
  DELETE_IF_NOT_NULL(l_ikey);
}
TEST_F(VtnFlowfilter,ValidateTest_KEY) {
  VtnFlowFilterMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *l_cval = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  val_flowfilter_t *val_vtn_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,l_cval);
  std::list<controller_domain_t> list_ctrlr_dom;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 UpdateControllerTable
                    (l_ikey,UNC_OP_CREATE,UPLL_DT_CANDIDATE ,dmi, list_ctrlr_dom,
                     config_mode, vtn_name));

}
TEST_F(VtnFlowfilter, valid2_CreateCandidateMo) {
 VtnFlowFilterMoMgr obj;
  val_flowfilter_t *ival = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal *tmp1 = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          vtn_key.vtn_name),"VTN",kMaxLenVtnName);
  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  strncpy((char*)(key_vtn_flowfilter->vtn_key.vtn_name),"vtn",10);

  const char * ctrlr_name = "PFC";
  const char* version("version");

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_CANDIDATE));

  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  DalOdbcMgr::clearStubData();
  CapaModuleStub::stub_clearStubData();

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcRecordNotFound);
  // DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNoMore);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
  //DalOdbcMgr::stub_setSingleRecordExists(false);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  key_vtn_flowfilter->input_direction = 1;

  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             ival);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,tmp1);
 
  EXPECT_EQ(UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID, obj.
                 CreateCandidateMo
                    (req,l_ikey,dmi));
DalOdbcMgr::clearStubData();
}
TEST_F(VtnFlowfilter, valid3_CreateCandidateMo) {
 VtnFlowFilterMoMgr obj;
  val_flowfilter_t *ival = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal *tmp1 = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          vtn_key.vtn_name),"VTN",kMaxLenVtnName);
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  strncpy((char*)(key_vtn_flowfilter->vtn_key.vtn_name),"vtn",10);

  const char * ctrlr_name = "PFC";
  const char* version("version");

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
    DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
 CapaModuleStub::stub_loadCapaModule();
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  key_vtn_flowfilter->input_direction = 1;

  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             ival);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,tmp1);

  l_ikey->set_user_data((void*)ctrlr_name);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 CreateCandidateMo
                    (req,l_ikey,dmi));


}
TEST_F(VtnFlowfilter, valid4_CreateCandidateMo) {
 VtnFlowFilterMoMgr obj;
  val_flowfilter_t *ival = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal *tmp1 = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast< char *>(key_vtn_flowfilter->
                          vtn_key.vtn_name),"VTN",kMaxLenVtnName);
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  strncpy((char*)(key_vtn_flowfilter->vtn_key.vtn_name),"vtn",10);

  const char * ctrlr_name = "PFC";
  const char* version("version");

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));

  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
    DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
 CapaModuleStub::stub_loadCapaModule();
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  key_vtn_flowfilter->input_direction = 1;

  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             ival);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,tmp1);

  l_ikey->set_user_data((void*)ctrlr_name);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.
                 CreateCandidateMo
                    (req,l_ikey,dmi));


}
TEST_F(VtnFlowfilter,DELETE_CTRLTBL_FAIL) {
  VtnFlowFilterMoMgr obj;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;

  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  ConfigVal *l_cval = NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_MERGE_CONFLICT;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD,kDalRcRecordNoMore);
  CtrlrCommitStatusList.push_back(l_CtrlrTxResult);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  val_flowfilter_t *val_vtn_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter);
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  DalDmlIntf *dmi(getDalDmlIntf());
 ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,l_cval);

  l_CtrlrTxResult->err_ckv = l_ikey;
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.
                 TxCopyCandidateToRunning
                    (keytype,&CtrlrCommitStatusList,dmi, config_mode,
                     vtn_name));
}
TEST_F(VtnFlowfilter,Validate_req_NULL) {
  VtnFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req = NULL;
  DalDmlIntf *dmi = NULL;

EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey,dmi));
}
TEST_F(VtnFlowfilter,Last_UpdateConfigDB) {
  VtnFlowFilterMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigVal *l_cval = NULL;
  val_flowfilter_t *val_vtn_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_vtn_flowfilter);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";


 DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  ConfigKeyVal *l_ikey  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            key_vtn_flowfilter,l_cval);
  std::list<controller_domain_t> list_ctrlr_dom;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 UpdateControllerTable
                    (l_ikey,UNC_OP_CREATE,UPLL_DT_CANDIDATE ,dmi, list_ctrlr_dom,
                     config_mode, vtn_name));

}
TEST_F(VtnFlowfilter,Diffconfig_fail) {
  VtnFlowFilterMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> *affected_ctrlr_set=NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcTxnError);
   IPC_RESPONSE_DECL(ipc_response);
 ConfigKeyVal *l_err_ckv = NULL;

  DalDmlIntf *dmi(getDalDmlIntf());
  TxUpdateUtil tx(4);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                obj.TxUpdateController(UNC_KT_VTN_FLOWFILTER,session_id,
                                       config_id,phase,affected_ctrlr_set,dmi,
                                       &l_err_ckv, &tx,
                                       config_mode, vtn_name));

}
TEST_F(VtnFlowfilter,Diffconfig_Default) {
  VtnFlowFilterMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> *affected_ctrlr_set=NULL;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpUpdate;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcTxnError);
   IPC_RESPONSE_DECL(ipc_response);

 ConfigKeyVal *l_err_ckv = NULL;

  DalDmlIntf *dmi(getDalDmlIntf());
  TxUpdateUtil tx(4);
 EXPECT_EQ(UPLL_RC_SUCCESS,
                obj.TxUpdateController(UNC_KT_VTN_FLOWFILTER,session_id,config_id,phase,affected_ctrlr_set,dmi,&l_err_ckv, &tx,
                                       config_mode, vtn_name));

}
/*
TEST_F(VtnFlowfilter,TxUpdateProcess_Pass_) {
  VtnFlowFilterMoMgr obj;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  set<string> affected_ctrlr_set;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  key_vtn_flowfilter_t *key_vtn_flowfilter = ZALLOC_TYPE(key_vtn_flowfilter_t);
  strncpy(reinterpret_cast<char*>(key_vtn_flowfilter->vtn_key.vtn_name),
		"vtn" , kMaxLenVtnName);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_CREATED_RECORDS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
   IPC_RESPONSE_DECL(ipc_response);

 ConfigKeyVal *l_err_ckv = NULL;

  DalDmlIntf *dmi(getDalDmlIntf());
  TxUpdateUtil tx(4);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, 
                obj.TxUpdateController(UNC_KT_VTN_FLOWFILTER,session_id,config_id,phase,&affected_ctrlr_set,dmi,&l_err_ckv, &tx,
                                       config_mode, vtn_name));
}
*/
TEST_F(VtnFlowfilter,Validate_VAL) {
  VtnFlowFilterMoMgr obj;
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase  = uuc::kUpllUcpCreate; 
  ConfigKeyVal *ckv_running = NULL;
  ConfigVal *l_cval = NULL;
  val_flowfilter_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_t);
  l_cval =  new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_flowfilter);
 ckv_running  = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStValFlowfilterController,
                            NULL,l_cval);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.
                 UpdateAuditConfigStatus
                    (cs_status ,phase,ckv_running,dmi));

}

