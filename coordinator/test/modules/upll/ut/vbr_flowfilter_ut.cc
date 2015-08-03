/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <vbr_flowfilter_momgr.hh>
#include <flowlist_momgr.hh>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <ctrlr_mgr.hh>
#include <config_mgr.hh>
#include "ut_util.hh"

using namespace unc;
using namespace upll;
using namespace kt_momgr;
using namespace unc::upll::test;
using namespace unc::capa;
using namespace unc::upll::dal;
using namespace unc::upll::config_momgr;
using namespace unc::tclib;
using namespace std;


class VbrFlowFilterTest : public UpllTestEnv
{
};
//namespace uudst = unc::upll::dal::schema::table;
TEST_F(VbrFlowFilterTest, Output_Pos)
{
  VbrFlowFilterMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_FLOWFILTER;
  BindInfo *bin = NULL;
  int nattr;

  EXPECT_EQ(true, obj.GetRenameKeyBindInfo(key_type, bin, nattr, MAINTBL));
  EXPECT_EQ(5, nattr);
}

#if 0
//Commented as it has a bug with code
TEST_F(VbrFlowFilterTest, Output_Negtbl)
{
  VbrFlowFilterMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_FLOWFILTER;
  BindInfo *bin = NULL;
  int nattr;

  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, CTRLRTBL));
}
TEST_F(VbrFlowFilterTest, Output_Negtbl1)
{
  VbrFlowFilterMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_FLOWFILTER;
  BindInfo *bin = NULL;
  int nattr;

  EXPECT_EQ(false, obj.GetRenameKeyBindInfo(key_type, bin, nattr, RENAMETBL));
}
#endif

TEST_F(VbrFlowFilterTest,AllocVal_outputNull) {
  VbrFlowFilterMoMgr obj;
  val_flowfilter_t *vbr_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_flowfilter_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(val, UPLL_DT_IMPORT,MAINTBL));
  //delete val;
}

TEST_F(VbrFlowFilterTest,AllocVal_Success) {
  VbrFlowFilterMoMgr obj;
  ConfigVal *val = NULL;
 
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_IMPORT,MAINTBL)); 
}

TEST_F(VbrFlowFilterTest,AllocVal_Error) {
  VbrFlowFilterMoMgr obj;
  val_flowfilter_t *vbr_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_flowfilter_val);

  obj.AllocVal(val, UPLL_DT_STATE, RENAMETBL); 

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AllocVal(val, UPLL_DT_STATE,RENAMETBL));           
  //delete val;
}

TEST_F(VbrFlowFilterTest,AllocVal_Error1) {
  VbrFlowFilterMoMgr obj;
  val_flowfilter_t *vbr_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_flowfilter_val);

  obj.AllocVal(val, UPLL_DT_STATE, CTRLRTBL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AllocVal(val, UPLL_DT_STATE,RENAMETBL));
  //delete val;
}

TEST_F(VbrFlowFilterTest,GetChildConfigKey) {
  VbrFlowFilterMoMgr obj;
  key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            key_vbr, NULL);
  
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey)); 
  //delete pkey;
}

TEST_F(VbrFlowFilterTest,GetChildConfigKey_pkeyNull) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey));   
  delete okey;
} 

TEST_F(VbrFlowFilterTest,GetChildConfigKey_pkeyNull1) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));    
  delete okey;
  delete pkey;
}

TEST_F(VbrFlowFilterTest,GetChildConfigKey_pkeyNull10) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
}
  

TEST_F(VbrFlowFilterTest,GetChildConfigKey_pkeyNull2) {
  VbrFlowFilterMoMgr obj;

  key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            key_vbr, NULL);

  strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1", 32);
  
  ConfigKeyVal *pkey = NULL;

  key_vbr_flowfilter_t *output = reinterpret_cast<key_vbr_flowfilter_t *> (okey->get_key());

  std::cout <<"val"<<(reinterpret_cast<const char *> (output->vbr_key.vbridge_name))<<std::endl;

  EXPECT_STREQ("VTN1", (reinterpret_cast<const char *> (output->vbr_key.vtn_key.vtn_name)));
  EXPECT_STREQ("VBR1", (reinterpret_cast<const char *> (output->vbr_key.vbridge_name)));
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey));  
  delete okey;
  delete pkey;
}

TEST_F(VbrFlowFilterTest,GetChildConfigKey_pkeyNull3) {
  VbrFlowFilterMoMgr obj;

  key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            key_vbr, NULL);

  strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1", 32);

  //ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
   //                         IpctSt::kIpcStKeyVbrFlowfilter,
    //                        key_vbr, NULL);

  key_vbr_flowfilter_t *output = reinterpret_cast<key_vbr_flowfilter_t *> (okey->get_key());

  std::cout <<"val"<<(reinterpret_cast<const char *> (output->vbr_key.vbridge_name))<<std::endl;

  EXPECT_STREQ("VTN1", (reinterpret_cast<const char *> (output->vbr_key.vtn_key.vtn_name)));
  EXPECT_STREQ("VBR1", (reinterpret_cast<const char *> (output->vbr_key.vbridge_name)));
  //delete pkey;
} 
TEST_F(VbrFlowFilterTest, DupConfigKeyVal_reqnull) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));
  //delete okey;
} 
  
TEST_F(VbrFlowFilterTest, DupConfigKeyVal_okeynull) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);

  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
  //delete req;
}

TEST_F(VbrFlowFilterTest, DupConfigKeyVal_Req_Invalid) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;

  val_flowfilter_t *vbr_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  vbr_flowfilter_val->cs_row_status = 1;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_flowfilter_val); 
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, tmp);
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));
  //delete req;
}

TEST_F(VbrFlowFilterTest, DupConfigKeyVal_val_Null) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;

  val_flowfilter_t *vbr_flowfilter_val = NULL; 

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_flowfilter_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));
  //delete req;
}

TEST_F(VbrFlowFilterTest, DupConfigKeyVal_Req_Valid) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  val_flowfilter_t *vbr_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  vbr_flowfilter_val->cs_row_status = 1;
 
  key_vbr_flowfilter_t *vbr_flowfilter = ZALLOC_TYPE(key_vbr_flowfilter_t);
 
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_flowfilter_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            vbr_flowfilter, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(okey, req, MAINTBL));
}

TEST_F(VbrFlowFilterTest, DupConfigKeyVal_key_null) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  val_flowfilter_t *vbr_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  vbr_flowfilter_val->cs_row_status = 1;

  key_vbr_flowfilter_t *vbr_flowfilter = NULL; 

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_flowfilter_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            vbr_flowfilter, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));
}

TEST_F(VbrFlowFilterTest, DupConfigKeyVal_InValidKT) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  val_flowfilter_t *vbr_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  vbr_flowfilter_val->cs_row_status = 1;

  key_vbr_flowfilter_t *vbr_flowfilter = ZALLOC_TYPE(key_vbr_flowfilter_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_flowfilter_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            vbr_flowfilter, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));
}

TEST_F(VbrFlowFilterTest, CopyToConfigkey_Ikey_NUll) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey, ikey));

  ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey,ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterTest, CopyToConfigkey_valid) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  //key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  //free (key_rename);
}

TEST_F(VbrFlowFilterTest, CopyToConfigkey_keyvbrNull) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;
  key_vbr_flowfilter_t *key_vbr_ff = ZALLOC_TYPE(key_vbr_flowfilter_t);
  strncpy((char *) key_vbr_ff->vbr_key.vbridge_name,"VBR1", 32);

  //key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  EXPECT_STREQ("VBR1", (reinterpret_cast<const char *> (key_vbr_ff->vbr_key.vbridge_name)));
  //free (key_rename);
}

TEST_F(VbrFlowFilterTest, CopyToConfigkey_old_name) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  strncpy(reinterpret_cast<char *>(key_rename->old_unc_vtn_name), "OLDVTN1", 32);
  strncpy(reinterpret_cast<char *>(key_rename->old_unc_vnode_name), "OLDVNODE1", 32);


  ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                         IpctSt::kIpcStKeyVbrFlowfilter,
                         key_rename, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));

  //key_vbr_flowfilter_t *key_vbr_ff = reinterpret_cast<key_vbr_flowfilter_t *>(okey->get_key());

  //EXPECT_STREQ("OLDVTN1", (reinterpret_cast<const char *> (key_vbr_ff->vbr_key.vtn_key.vtn_name)));
  //EXPECT_STREQ("OLDVNODE1", (reinterpret_cast<const char *> (key_vbr_ff->vbr_key.vbridge_name)));
  //delete ikey;
}

TEST_F(VbrFlowFilterTest, CopyToConfigkey_WRONG_KEYTYPE) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);


  ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                         IpctSt::kIpcStKeyVbrFlowfilter,
                         key_rename, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterTest, MerGeValidate_Success) {
  VbrFlowFilterMoMgr obj;
  //IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  const char* ctrlr_id = {0};
  
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.MergeValidate(UNC_KT_VBRIF_FLOWFILTER,ctrlr_id,ikey,dmi,(upll_import_type)0));
}

TEST_F(VbrFlowFilterTest, RenameMo_NoOperation) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  const char* ctrlr_id = {0};
  
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.RenameMo(req,ikey,dmi,ctrlr_id));
}

TEST_F(VbrFlowFilterTest, UpdateMo_NoOperation) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.UpdateMo(req,ikey,dmi));
}

TEST_F(VbrFlowFilterTest, UpdateConfigStatus_ValStructureNull) {
 VbrFlowFilterMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;
 DalDmlIntf *dmi(getDalDmlIntf());
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);

 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
 //delete key;
}

TEST_F(VbrFlowFilterTest, UpdateConfigStatus_Success) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *upd_key = NULL;
  ConfigKeyVal *ctrlr_key= NULL;
  DalDmlIntf *dmi(getDalDmlIntf());

  val_flowfilter_t *val = ZALLOC_TYPE(val_flowfilter_t);

  val->cs_row_status = UNC_CS_NOT_APPLIED;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val);

  ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
  val_flowfilter_t *output = reinterpret_cast<val_flowfilter_t *> (GetVal(key));
 
  EXPECT_EQ(UNC_CS_APPLIED,output->cs_row_status);
  //delete key;
}

TEST_F(VbrFlowFilterTest, UpdateConfigStatus_Success1) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *upd_key = NULL;
  ConfigKeyVal *ctrlr_key= NULL;
  DalDmlIntf *dmi(getDalDmlIntf());

  val_flowfilter_t *val = ZALLOC_TYPE(val_flowfilter_t);

  val->cs_row_status = UNC_CS_APPLIED;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val);

  ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 1, upd_key, dmi, ctrlr_key));
  val_flowfilter_t *output = reinterpret_cast<val_flowfilter_t *> (GetVal(key));
  
  EXPECT_EQ(UNC_CS_NOT_APPLIED,output->cs_row_status);
  //delete key;
}

TEST_F(VbrFlowFilterTest, UpdateConfigStatus_Failure) {
 VbrFlowFilterMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;
 DalDmlIntf *dmi(getDalDmlIntf());

 val_flowfilter_t *val = ZALLOC_TYPE(val_flowfilter_t);

 val->cs_row_status = UNC_CS_NOT_APPLIED;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, tmp);

 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(key, UNC_OP_DELETE, 0, upd_key, dmi, ctrlr_key));
 //delete key;
}

TEST_F(VbrFlowFilterTest, ValidateAttribute_Success) {
  VbrFlowFilterMoMgr obj;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);
  
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey,dmi));
}

TEST_F(VbrFlowFilterTest, ValidateMessage_NullChk) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrFlowFilterTest, ValidateMessage_NullChk1) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrFlowFilterTest, ValidateMessage_Invalid_KeyType) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req,ikey));
}

TEST_F(VbrFlowFilterTest, ValidateMessage_Valid_KeyType) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
  ikey->set_key_type((unc_key_type_t)UNC_KT_VBR_FLOWFILTER); 
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrFlowFilterTest, ValidateMessage_InValidKeyST) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader); 
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter);
  void *key = NULL;
  ikey->SetKey(IpctSt::kIpcStKeyVbrFlowfilter, key);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterTest, ValidateMessage_ValidMoManagerInstance) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);
  strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name, "VTN", 32);
  strncpy((char*) key_vbr->vbr_key.vbridge_name, "VBR1", 32);
  key_vbr->direction = 1;
  req->option2 = UNC_OPT2_NONE;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter, key_vbr, NULL);
 
   if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ikey->SetKey(IpctSt::kIpcStKeyVbrFlowfilter, key_vbr);
 
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterTest, ValidateMessage_keystructNull) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  //key_vbr_flowfilter_t *key_vbr = NULL;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter, NULL, NULL);


  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterTest, Update_Audit) {
  VbrFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,kUpllUcpCreate, ikey, dmi));

  val_flowfilter_t *val = ZALLOC_TYPE(val_flowfilter_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val);

  ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED, kUpllUcpCreate, ikey, dmi));
  val_flowfilter_t *output = reinterpret_cast<val_flowfilter_t *> (GetVal(ikey));
  EXPECT_EQ(UNC_CS_APPLIED, output->cs_row_status);
  //delete ikey;
}


TEST_F(VbrFlowFilterTest, ValidateMessage_ValidMoManagerInsInvalidkey) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);
  strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name, "_$VTN", 32);
  strncpy((char*) key_vbr->vbr_key.vbridge_name, "_$VBR1", 32);
  key_vbr->direction = -2;
  
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter, key_vbr, NULL);
  
   if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
  //delete ikey;
}
 
TEST_F(VbrFlowFilterTest, IsValid_keyNull ) {
 VbrFlowFilterMoMgr obj;
 ConfigKeyVal *key = NULL;
 
  EXPECT_EQ(false, obj.IsValidKey(key, 1)); 
}

TEST_F(VbrFlowFilterTest, IsValid_InvalidVtnName ) {
 VbrFlowFilterMoMgr obj;
 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);

 EXPECT_EQ(false, obj.IsValidKey(key, uudst::vbr_flowfilter::kDbiVtnName));
 //delete key;
}

TEST_F(VbrFlowFilterTest, IsValid_InvalidVbrName ) {
 VbrFlowFilterMoMgr obj;
 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);

 EXPECT_EQ(false,obj.IsValidKey(key, uudst::vbr_flowfilter::kDbiVbrName));
 //delete key;
}


TEST_F(VbrFlowFilterTest, GetRenamedUncKey_CtlrInValid) {
 VbrFlowFilterMoMgr obj;
 uint8_t * ctrlr_id = NULL;
 DalDmlIntf *dmi(getDalDmlIntf());
 
 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);
 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"OLDVTNNAME",32);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"OLDVBRNAME",32);

   if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedUncKey(ikey, UPLL_DT_STATE,dmi,ctrlr_id));

 //delete ikey;
 //UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(VbrFlowFilterTest, GetRenamedUncKey_Valid) {
 VbrFlowFilterMoMgr obj;
 uint8_t ctrlr_id[4] = {'p', 'f', 'c'};
 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);
 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"OLDVTNNAME",32);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"OLDVBRNAME",32);

   if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter, key_vbr, NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE, kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetRenamedUncKey(ikey, UPLL_DT_STATE, dmi, ctrlr_id));
 
 //key_vbr_flowfilter_t *output = reinterpret_cast<key_vbr_flowfilter_t *> (ikey->get_key());
 
 //delete dmi;
 //delete ikey;
 //UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(VbrFlowFilterTest, GetRenamedControllerKey_ValidSet) {
 VbrFlowFilterMoMgr obj;
 controller_domain *ctrl_domain = new controller_domain();
 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",32);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS(ikey, 0x01);
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetRenamedControllerKey(ikey, UPLL_DT_CANDIDATE, dmi, ctrl_domain));

 //UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 //DalOdbcMgr::clearStubData();
}

#if 0
TEST_F(VbrFlowFilterTest, GetRenamedControllerKey_VbrValidNotSet) {
 VbrFlowFilterMoMgr obj;
 controller_domain *ctrl_domain = new controller_domain();
 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",32);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS(ikey, 0x10);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));

 //UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 //DalOdbcMgr::clearStubData();
 //delete dmi;
}

TEST_F(VbrFlowFilterTest, GetRenamedControllerKey_VbrValidNotSet1) {
 VbrFlowFilterMoMgr obj;
 controller_domain *ctrl_domain = new controller_domain();
 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",32);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalOdbcMgr::stub_setSingleRecordExists(false);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS(ikey, 0x10);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));

 //DalOdbcMgr::clearStubData();
 //delete dmi;
}

TEST_F(VbrFlowFilterTest, GetRenamedControllerKey_VbrValidNotSet2) {
 VbrFlowFilterMoMgr obj;
 controller_domain *ctrl_domain = new controller_domain();
 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",32);

 if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalOdbcMgr::stub_setSingleRecordExists(false);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS(ikey, 0x10);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));

//UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
// DalOdbcMgr::clearStubData();
  //delete dmi;
}
#endif

TEST_F(VbrFlowFilterTest, ValidateCapability_InvalidReq) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= NULL;
 ConfigKeyVal *key = NULL;
 const char * ctrlr_name = NULL;
 
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ValidateCapability(req,key,ctrlr_name));
}

TEST_F(VbrFlowFilterTest, ValidateCapability_InvalidKey) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 ConfigKeyVal *key = NULL;
 const char * ctrlr_name = NULL;
 
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ValidateCapability(req,key,ctrlr_name));
 //delete req;
}

TEST_F(VbrFlowFilterTest, ValidateCapability_Success) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_RUNNING;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);

 const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 //CapaModuleStub::stub_loadCapaModule();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

 EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,obj.ValidateCapability(req,key,ctrlr_name));
 delete req;
 delete key;
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterTest, ValidateCapability_Success1) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_UPDATE;
 req->datatype = UPLL_DT_RUNNING;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);

 const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 //CapaModuleStub::stub_loadCapaModule();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);

 EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,obj.ValidateCapability(req,key,ctrlr_name));
 delete req;
 delete key;
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterTest, ValidateCapability_Success2) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_UPDATE;
 req->datatype = UPLL_DT_RUNNING;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);

 const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version,0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 //CapaModuleStub::stub_loadCapaModule();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,obj.ValidateCapability(req,key,ctrlr_name));
 delete req;
 delete key;
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterTest, IsReferenced_Invalid_key) {
 VbrFlowFilterMoMgr obj;
 ConfigKeyVal *key = NULL;
 DalDmlIntf *dmi(getDalDmlIntf());
 IPC_REQ_RESP_HEADER_DECL(req);
 req->datatype = UPLL_DT_CANDIDATE;

 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, key, dmi));
}
	
TEST_F(VbrFlowFilterTest, IsReferenced_Invalid_dmi) {
 VbrFlowFilterMoMgr obj;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);
 DalDmlIntf *dmi = NULL;
 IPC_REQ_RESP_HEADER_DECL(req);
 req->datatype = UPLL_DT_CANDIDATE;

 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, key, dmi));
 //delete key;
}

TEST_F(VbrFlowFilterTest, IsReferenced_valid_update) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_CANDIDATE;

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, key, dmi));
 //DalOdbcMgr::clearStubData();
 //delete req;
 //delete key;
}

TEST_F(VbrFlowFilterTest, IsReferenced_valid_update1) {
 VbrFlowFilterMoMgr obj;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 IPC_REQ_RESP_HEADER_DECL(req);
 req->datatype = UPLL_DT_CANDIDATE;

 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, key, dmi));
 //DalOdbcMgr::clearStubData();
 //delete key;
}

TEST_F(VbrFlowFilterTest, IsReferenced_valid_update2) {
 VbrFlowFilterMoMgr obj;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 IPC_REQ_RESP_HEADER_DECL(req);
 req->datatype = UPLL_DT_CANDIDATE;

 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, key, dmi));
 //DalOdbcMgr::clearStubData();
 //delete key;
}

TEST_F(VbrFlowFilterTest, IsReferenced_valid_update3_fail) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_CANDIDATE;

 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

 
 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",32);
 key_vbr->direction = 1;

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);

 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 DalOdbcMgr::stub_setSingleRecordExists(false); 

 EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.IsReferenced(req, key, dmi));
 
 //DalOdbcMgr::clearStubData();
 //delete req;
 //delete key;
}

TEST_F(VbrFlowFilterTest, IsReferenced_valid_update3_Success) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_CANDIDATE;

 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",32);
 key_vbr->direction = 1;

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);

 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 DalOdbcMgr::stub_setSingleRecordExists(true);
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.IsReferenced(req, key, dmi));
 
 //DalOdbcMgr::clearStubData();
 //delete req;
 //delete key;
}

TEST_F(VbrFlowFilterTest, ReadMo_NoInstance) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader* req = NULL;
 ConfigKeyVal *ikey = NULL;

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.ReadMo(req,ikey,dmi));
}
TEST_F(VbrFlowFilterTest, ReadMo_Instance) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->option1 = UNC_OPT1_NORMAL;
 req->datatype = UPLL_DT_STATE;
 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);


 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",32);
 key_vbr->direction = UPLL_FLOWFILTER_DIR_IN;
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 DalOdbcMgr::stub_setSingleRecordExists(false);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadMo(req, ikey, dmi));

 //delete req;
 //delete ikey;
 //delete dmi;
 //UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(VbrFlowFilterTest, ReadMo_Instance1) {
  VbrFlowFilterMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_STATE;
  
  //`	DalOdbcMgr::clearStubData();
  req->datatype = UPLL_DT_CANDIDATE;
  key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);
  
  strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",32);
  key_vbr->direction = UPLL_FLOWFILTER_DIR_IN;
  
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalDmlIntf *dmi(getDalDmlIntf());
  
  DalOdbcMgr::stub_setSingleRecordExists(false);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ReadMo(req,ikey,dmi));

  //delete req;
  //delete ikey;
 // DalOdbcMgr::clearStubData();
 // UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(VbrFlowFilterTest, ReadMo_Instance2) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
 req->datatype = UPLL_DT_STATE;

 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",32);
 key_vbr->direction = UPLL_FLOWFILTER_DIR_IN;
 DalOdbcMgr::stub_setSingleRecordExists(true);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);


 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req,ikey,dmi));

 //delete req;
 //delete ikey;
}

TEST_F(VbrFlowFilterTest, ReadSiblingMo_NoInstance) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader* req = NULL;
 ConfigKeyVal *ikey = NULL;
 bool begin = false;

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.ReadSiblingMo(req,ikey,begin,dmi));
 //delete dmi;
}

TEST_F(VbrFlowFilterTest, ReadSiblingMo_Instance) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 bool begin = false;
 req->operation = UNC_OP_READ;
 req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
 req->datatype = UPLL_DT_STATE;

 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 
 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",32);
 key_vbr->direction = UPLL_FLOWFILTER_DIR_IN;
 DalOdbcMgr::stub_setSingleRecordExists(true);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);


 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.ReadSiblingMo(req,ikey,begin,dmi));
}


TEST_F(VbrFlowFilterTest, ReadSiblingMo_Instance1) {
 VbrFlowFilterMoMgr obj;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 bool begin = false;
 req->operation = UNC_OP_READ;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;
 req->datatype = UPLL_DT_STARTUP;

 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",32);
 key_vbr->direction = UPLL_FLOWFILTER_DIR_IN;
 DalOdbcMgr::stub_setSingleRecordExists(true);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);


 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.ReadSiblingMo(req,ikey,begin,dmi));
}

TEST_F(VbrFlowFilterTest, ReadSiblingMo_Instance2) {
 VbrFlowFilterMoMgr obj;
 bool begin = false;
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;
 req->datatype = UPLL_DT_STARTUP;

 key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 strncpy((char*) key_vbr->vbr_key.vtn_key.vtn_name,"VTN1",7);
 strncpy((char*) key_vbr->vbr_key.vbridge_name,"VBR1",7);
 key_vbr->direction = UPLL_FLOWFILTER_DIR_IN;
 DalOdbcMgr::stub_setSingleRecordExists(false);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);


 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER, IpctSt::kIpcStKeyVbrFlowfilter,key_vbr,NULL);
  //void *key = ZALLOC_TYPE(IpctSt);
        //key_vbr_flowfilter_t *vbr_ff_key;
          //      vbr_ff_key = reinterpret_cast<key_vbr_flowfilter_t*>
            //                    (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_t)));

 //strncpy((char*) key->vbr_key.vtn_key.vtn_name,"VTN1",31);
  ikey->SetKey(IpctSt::kIpcStKeyVbrFlowfilter, key_vbr);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ReadSiblingMo(req,ikey,begin,dmi));
}

/*
TEST_F(VbrFlowFilterTest, ReadDetail_valid_data) {
  VbrFlowFilterMoMgr obj;
  key_vbr_flowfilter_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_t);
  val_flowfilter_t *val_flowfilter_controller = reinterpret_cast<val_flowfilter_t *>
        (malloc(sizeof(val_flowfilter_t)));
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val_flowfilter_controller);
  //  val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_VFFCS] = UNC_VF_VALID ;
  if(!tmp) {
    std::cout<<"tmp memory allocation failed"<<std::endl;
    exit(0);
  }
  //ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
 //               IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,tmp);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
               IpctSt::kIpcStKeyVbrFlowfilter, key_vbr, tmp);
 //ikey->SetCfgVal(tmp);
  IpcResponse *ipc_response =  reinterpret_cast<IpcResponse *>
               (malloc(sizeof(IpcResponse)));
  ipc_response->header.clnt_sess_id = 1;
  ipc_response->header.config_id = 1;
  ipc_response->header.operation = UNC_OP_READ;
  ipc_response->header.rep_count = 1;
  ipc_response->header.option1 = UNC_OPT1_DETAIL;
  ipc_response->header.option2 = UNC_OPT2_NONE;
  ipc_response->header.datatype = UPLL_DT_STATE;
  ipc_response->header.result_code = UPLL_RC_SUCCESS;
  //ipc_response->ckv_data= new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
//                 IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,tmp);
  val_flowfilter_entry_st_t *val_vbr_stflowfilter = 
     new val_flowfilter_entry_st_t();
  ConfigVal *tmp_st = new ConfigVal(IpctSt::kIpcStValFlowfilterEntrySt,
                             val_vbr_stflowfilter);
  val_vbr_stflowfilter->valid[UPLL_IDX_SEQ_NUM_FFES] = UNC_VF_VALID ;
  ipc_response->ckv_data= new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                 IpctSt::kIpcStValFlowfilterEntrySt,NULL,tmp_st);
  if (!ipc_response->ckv_data) {
    std::cout<<"memory allocation failed"<<std::endl;
    exit(0);
  }
  
  val_flowlist_entry_st_t *val_flowlist_entry_st =
                                new val_flowlist_entry_st_t(); 
  ConfigVal *tmp_fl_st = new ConfigVal(IpctSt::kIpcStValFlowlist,
                             val_flowlist_entry_st);
  //ipc_response->ckv_data->SetCfgVal(tmp); 
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  ipc_response->return_code = UPLL_RC_SUCCESS;
  ipc_response->ckv_data->AppendCfgVal(tmp_fl_st); 
  //upll_keytype_datatype_t dt_type = UPLL_DT_STATE;
   if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
    UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
    UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  unc_keytype_operation_t op = UNC_OP_READ;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t  domain_id[32] = {'u','n','k','n','o','w','n'};
  uint8_t ctrlr_id[4] = {'p', 'f', 'c'};
  std::cout<<"calling ReadDetail function"<<std::endl;
  
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadDetail
                ( 
                 ikey,
                 dup_key,
                 ipc_response,
                 UPLL_DT_STATE, 
                 op,
                 dbop,
                 dmi,
                 domain_id,
                 ctrlr_id));
}

*/



