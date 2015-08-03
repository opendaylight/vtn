/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <vbr_if_flowfilter_momgr.hh>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <ctrlr_mgr.hh>
#include <config_mgr.hh>
#include "ut_util.hh"
#include <pfcxx/synch.hh>


using namespace unc::upll::dal;
using namespace unc::capa;
using namespace unc::upll::config_momgr;
using namespace unc::upll::test;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::dal::schema::table;

UpllDbConnMgr *objDb=NULL;
class vbr_if_test :
  public UpllTestEnv {
};
TEST_F(vbr_if_test,AllocVal_outputNull) {
  VbrIfFlowFilterMoMgr obj;
  val_flowfilter_t *vbr_if_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(val, UPLL_DT_IMPORT,MAINTBL));
  delete val;
}

TEST_F(vbr_if_test,AllocVal_Success) {
  VbrIfFlowFilterMoMgr obj;
  ConfigVal *val = NULL; 
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_IMPORT,MAINTBL)); 
}

TEST_F(vbr_if_test,AllocVal_Error) {
  VbrIfFlowFilterMoMgr obj;
  ConfigVal *val = NULL; 
  obj.AllocVal(val, UPLL_DT_STATE,RENAMETBL); 
  EXPECT_EQ(UPLL_RC_SUCCESS,/*ERR_GENERIC*/obj.AllocVal(val, UPLL_DT_STATE,RENAMETBL));           
}

TEST_F(vbr_if_test,GetChildConfigKey) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey)); 
}
/*
TEST_F(vbr_if_test,GetChildConfigKey_pkeyNull) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey =  new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));   
  delete pkey;
}
*/
TEST_F(vbr_if_test,GetChildConfigKey_pkeyNull1) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));    
  delete okey;
  delete pkey;
}

TEST_F(vbr_if_test,GetChildConfigKey_pkeyNull2) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);

  strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
  strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_if->if_key.if_name,"INTERFACENAME",32);
  
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            key_vbr_if, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  key_vbr_if_flowfilter_t *output = reinterpret_cast<key_vbr_if_flowfilter_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->if_key.vbr_key.vtn_key.vtn_name)));
  EXPECT_STREQ("VBR1",(reinterpret_cast<const char *> (output->if_key.vbr_key.vbridge_name)));
  EXPECT_STREQ("INTERFACENAME",(reinterpret_cast<const char *> (output->if_key.if_name)));
  delete pkey;
}

TEST_F(vbr_if_test, DupConfigKeyVal_reqnull) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, NULL);
  ConfigKeyVal *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req,MAINTBL));
  delete okey;
}

TEST_F(vbr_if_test, DupConfigKeyVal_okeynull) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, NULL);

  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
  delete okey;
  delete req;
}

TEST_F(vbr_if_test, DupConfigKeyVal_Req_Invalid) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;

  val_flowfilter_t *vbr_if_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  vbr_if_flowfilter_val->cs_row_status = 1;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_val); 
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, tmp);
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
  delete req;
}

/*Core Dump Case
TEST_F(vbr_if_test, DupConfigKeyVal_Req_Valid) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;

  val_flowfilter_t *vbr_if_flowfilter_val = reinterpret_cast<val_flowfilter_t *>
        (malloc(sizeof(val_flowfilter_t)));
  memset(vbr_if_flowfilter_val, 0, sizeof(val_flowfilter_t));
  vbr_if_flowfilter_val->cs_row_status = 1;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, tmp);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
}
*/

TEST_F(vbr_if_test, DupConfigKeyVal_Req_Valid) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
 
  val_flowfilter_t *vbr_if_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t); 
  vbr_if_flowfilter_val->cs_row_status = 1;
 
  key_vbr_if_flowfilter_t * key_vbr_ff = ZALLOC_TYPE(key_vbr_if_flowfilter_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            key_vbr_ff, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,MAINTBL));
  delete req;
}

TEST_F(vbr_if_test, CopyToConfigkey_Ikey_NUll) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
  delete ikey;
}

TEST_F(vbr_if_test, CopyToConfigkeyVal_InValid) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            key_rename, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  strncpy((char*) key_rename->old_unc_vtn_name,"OLDVTNNAME",32); 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
  delete ikey;
}

TEST_F(vbr_if_test, CopyToConfigkeyVal_Valid) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info); 
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            key_rename, NULL);

  strncpy((char*) key_rename->old_unc_vtn_name,"OLDVTNNAME",32);
  strncpy((char*) key_rename->old_unc_vnode_name,"OLDVBRIDGEE",32); 
  strncpy((char*) key_rename->new_unc_vnode_name,"NEWVBRIDGE",32);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.CopyToConfigKey(okey,ikey));
  key_vbr_if_flowfilter_t *output = reinterpret_cast<key_vbr_if_flowfilter_t *> (okey->get_key());
  EXPECT_STREQ("OLDVTNNAME",(reinterpret_cast<const char *> (output->if_key.vbr_key.vtn_key.vtn_name)));
  EXPECT_STREQ("NEWVBRIDGE",(reinterpret_cast<const char *> (output->if_key.vbr_key.vbridge_name)));
  delete ikey;
}

TEST_F(vbr_if_test, RenameMo_NoOperation) {
  VbrIfFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  const char* ctrlr_id = {0};
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.RenameMo(req,ikey,dmi,ctrlr_id));
}

TEST_F(vbr_if_test, UpdateMo_NoOperation) {
  VbrIfFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.UpdateMo(req,ikey,dmi));
}

TEST_F(vbr_if_test, MerGeValidate_Success) {
  VbrIfFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  const char* ctrlr_id = {0};
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.MergeValidate(UNC_KT_VBRIF_FLOWFILTER,ctrlr_id,ikey,dmi,(upll_import_type)0));
} 

TEST_F(vbr_if_test, Update_Audit) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,kUpllUcpCreate, ikey, dmi));
  val_flowfilter_t *val = ZALLOC_TYPE(val_flowfilter_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val);

  ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey,dmi));
  val_flowfilter_t *output = reinterpret_cast<val_flowfilter_t *> (GetVal(ikey));
  EXPECT_EQ(UNC_CS_APPLIED,output->cs_row_status);
  delete ikey;
}

TEST_F(vbr_if_test, ValidateMessage) {
  VbrIfFlowFilterMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req,ikey));
}

TEST_F(vbr_if_test, ValidateMessage_Invalid_KeyType) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req,ikey));
  delete ikey;
}

TEST_F(vbr_if_test, ValidateMessage_InValidMoManagerInstance) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateMessage(req,ikey));
 delete ikey;
}


TEST_F(vbr_if_test, ValidateMessage_ValidMoManagerInstance) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 memset(key_vbr_if, 0 ,sizeof(key_vbr_if_flowfilter_t));
 

 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req,ikey));
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 delete ikey;
}
/* Core Dump Test Case
TEST_F(vbr_if_test, ValidateAttributei_keyNull) {
 VbrIfFlowFilterMoMgr obj;
 ConfigKeyVal *ikey = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey));
}*/


TEST_F(vbr_if_test, ValidateAttribute) {
 VbrIfFlowFilterMoMgr obj;
 DalDmlIntf *dmi(getDalDmlIntf());
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey, dmi));
  delete ikey;
}
TEST_F(vbr_if_test, ValidateAttribute_Success) {
 VbrIfFlowFilterMoMgr obj;
 DalDmlIntf *dmi(getDalDmlIntf());
 IPC_REQ_RESP_HEADER_DECL(req);
 req->datatype = UPLL_DT_STATE;
 //key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER);
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey,dmi,req));
 delete ikey;
}
TEST_F(vbr_if_test, GetRenameBindInfo_Output_Null) {
 VbrIfFlowFilterMoMgr obj;
 BindInfo *info = NULL;
 int nattr = 0;
 EXPECT_EQ(PFC_TRUE,obj.GetRenameKeyBindInfo(UNC_KT_VBRIF_FLOWFILTER, info, nattr, MAINTBL));
 EXPECT_EQ(6,nattr);
}
/*
TEST_F(vbr_if_test, GetRenameBindInfo_InvalidTable) {
 VbrIfFlowFilterMoMgr obj;
 BindInfo *info = NULL;
 int nattr = 0;
 EXPECT_EQ(PFC_FALSE,obj.GetRenameKeyBindInfo(UNC_KT_VBRIF_FLOWFILTER, info, nattr,RENAMETBL));
}
*/

TEST_F(vbr_if_test, UpdateConfigStatus_ValStructureNull) {
 VbrIfFlowFilterMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
}

TEST_F(vbr_if_test, UpdateConfigStatus_Success) {
 VbrIfFlowFilterMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;
 DalDmlIntf *dmi(getDalDmlIntf());
 val_flowfilter_t *val = ZALLOC_TYPE(val_flowfilter_t);

 val->cs_row_status = UNC_CS_NOT_APPLIED;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, tmp);

 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
 val_flowfilter_t *output = reinterpret_cast<val_flowfilter_t *> (GetVal(key));
 EXPECT_EQ(UNC_CS_APPLIED,output->cs_row_status);
 delete key;
}


TEST_F(vbr_if_test, UpdateConfigStatus_Success1) {
 VbrIfFlowFilterMoMgr obj;
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
  delete key;
}

TEST_F(vbr_if_test, UpdateConfigStatus_Failure) {
 VbrIfFlowFilterMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;
 DalDmlIntf *dmi(getDalDmlIntf());
 val_flowfilter_t *val = ZALLOC_TYPE(val_flowfilter_t);

 val->cs_row_status = UNC_CS_NOT_APPLIED;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, tmp);

 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(key, UNC_OP_DELETE, 0, upd_key, dmi, ctrlr_key));
 delete key;
}
TEST_F(vbr_if_test, IsValid_keyNull ) {
 VbrIfFlowFilterMoMgr obj;
 ConfigKeyVal *key = NULL;
 EXPECT_EQ(false,obj.IsValidKey(key, 1)); 
}

TEST_F(vbr_if_test, IsValid_InvalidVtnName ) {
 VbrIfFlowFilterMoMgr obj;
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);

 EXPECT_EQ(false,obj.IsValidKey(key, uudst::vbr_if_flowfilter::kDbiVtnName));
 delete key;
}

TEST_F(vbr_if_test, IsValid_InvalidVbrName ) {
 VbrIfFlowFilterMoMgr obj;
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);

 EXPECT_EQ(false,obj.IsValidKey(key, uudst::vbr_if_flowfilter::kDbiVbrName));
 delete key;
}

TEST_F(vbr_if_test, IsValid_InvalidIfnName ) {
  VbrIfFlowFilterMoMgr obj;
   key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 
  ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);
 
  EXPECT_EQ(false,obj.IsValidKey(key, uudst::vbr_if_flowfilter::kDbiVbrIfName));
  delete key;
}
/*
 VbrIfFlowFilterMoMgr obj;
 key_vbr_if_flowfilter_t *key_vbr_if = reinterpret_cast<key_vbr_if_flowfilter_t*>
                  (malloc(sizeof(key_vbr_if_flowfilter_t)));
 memset(key_vbr_if, 0 ,sizeof(key_vbr_if_flowfilter_t));

 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);

 EXPECT_EQ(false,obj.IsValidKey(key, 0));   
 delete key; 
}*/

TEST_F(vbr_if_test, IsValid_InvalidIndex ) {
 VbrIfFlowFilterMoMgr obj;
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);

 EXPECT_EQ(false,obj.IsValidKey(key, 23));
 delete key;
}

TEST_F(vbr_if_test, ValidateCapability_InvalidReq) {
 VbrIfFlowFilterMoMgr obj;
 IpcReqRespHeader *req= NULL;
 ConfigKeyVal *key = NULL;
 const char * ctrlr_name = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ValidateCapability(req,key,ctrlr_name));
}

TEST_F(vbr_if_test, ValidateCapability_InvalidKey) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 ConfigKeyVal *key = NULL;
 const char * ctrlr_name = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ValidateCapability(req,key,ctrlr_name));
}
#if 0
VALIDATE API CRASH

TEST_F(vbr_if_test, ValidateCapability_Invalid_Create) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_STATE;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER);
 const char * ctrlr_name = "cntrlr_name";
 EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,obj.ValidateCapability(req,key,ctrlr_name));
 delete key;
}
TEST_F(vbr_if_test, ValidateCapability_Invalid_Update) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 req->operation = UNC_OP_UPDATE;
 req->datatype = UPLL_DT_STATE;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER);
 const char * ctrlr_name = "cntrlr_name";
 EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,obj.ValidateCapability(req,key,ctrlr_name));
 delete key;
}
TEST_F(vbr_if_test, ValidateCapability_Invalid_Delete) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 req->operation = UNC_OP_DELETE;
 req->datatype = UPLL_DT_STATE;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER);
 const char * ctrlr_name = "cntrlr_name";
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ValidateCapability(req,key,ctrlr_name));
 delete key;
}

TEST_F(vbr_if_test, ValidateCapability_Success) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_STATE;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER);

 const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_STATE_CAPABILITY, true);

 EXPECT_EQ(UPLL_RC_SUCCESS,obj.ValidateCapability(req,key,ctrlr_name));
 delete key;
}
#endif

TEST_F(vbr_if_test, GetRenamedUncKey_CtlrInValid) {
 VbrIfFlowFilterMoMgr obj;
 uint8_t * ctrlr_id = NULL;
 DalDmlIntf *dmi(getDalDmlIntf());
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"OLDVTNNAME",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"OLDVBRNAME",32);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedUncKey(ikey, UPLL_DT_STATE,dmi,ctrlr_id));

 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_test, GetRenamedUncKey_Valid) {
 VbrIfFlowFilterMoMgr obj;
 uint8_t ctrlr_id[4] = {'p', 'f', 'c'};
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"OLDVTNNAME",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"OLDVBRNAME",32);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetRenamedUncKey(ikey, UPLL_DT_STATE,dmi,ctrlr_id));
 
 //key_vbr_if_flowfilter_t *output = reinterpret_cast<key_vbr_if_flowfilter_t *> (ikey->get_key());

 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_test, ReadMo_NoInstance) {
 VbrIfFlowFilterMoMgr obj;
 IpcReqRespHeader* req = NULL;
 ConfigKeyVal *ikey = NULL;

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UNC_UPLL_RC_ERR_BAD_REQUEST,obj.ReadMo(req,ikey,dmi));
}

TEST_F(vbr_if_test, ReadMo_Instance) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 req->option1 = UNC_OPT1_NORMAL;
 req->datatype = UPLL_DT_STATE;
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();


 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ReadMo(req,ikey,dmi));

 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_test, ReadSiblingMo_NoInstance) {
 VbrIfFlowFilterMoMgr obj;
 IpcReqRespHeader* req = NULL;
 ConfigKeyVal *ikey = NULL;
 bool begin = false;

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UNC_UPLL_RC_ERR_BAD_REQUEST,obj.ReadSiblingMo(req,ikey,begin,dmi));
}

TEST_F(vbr_if_test, ReadMo_Instance1) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 req->option1 = UNC_OPT1_NORMAL;
 req->datatype = UPLL_DT_CANDIDATE;
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);

 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UNC_UPLL_RC_ERR_GENERIC,obj.ReadMo(req,ikey,dmi));

 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}
TEST_F(vbr_if_test, ReadSiblingMo_Instance) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 bool begin = false;
 req->operation = UNC_OP_READ;
 req->option1 = UNC_OPT1_NORMAL;
 req->datatype = UPLL_DT_STATE;
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.ReadSiblingMo(req,ikey,begin,dmi));

 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}
TEST_F(vbr_if_test, ReadSiblingMo_Instance1) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 bool begin = false;
 req->option1 = UNC_OPT1_NORMAL;
 DalOdbcMgr::clearStubData();
 req->datatype = UPLL_DT_CANDIDATE;
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);

 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ReadSiblingMo(req,ikey,begin,dmi));

 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}


TEST_F(vbr_if_test, GetRenamedControllerKey_VtnInvalidInstance) {
 VbrIfFlowFilterMoMgr obj;
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

 controller_domain *ctrl_domain = ZALLOC_TYPE(controller_domain);
 DalDmlIntf *dmi(getDalDmlIntf()); 
 SET_USER_DATA_FLAGS(ikey, 0x01);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_STATE,dmi,ctrl_domain));

 delete ikey;
}

TEST_F(vbr_if_test, GetRenamedControllerKey_ValidNotSet) {
 VbrIfFlowFilterMoMgr obj;
 controller_domain *ctrl_domain = ZALLOC_TYPE(controller_domain);
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);
 //UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS(ikey, 0x01);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));
 
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 delete ikey;
}

TEST_F(vbr_if_test, GetRenamedControllerKey_VbrInvalidInstance) {
 VbrIfFlowFilterMoMgr obj;
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

 controller_domain *ctrl_domain = ZALLOC_TYPE(controller_domain);
 DalDmlIntf *dmi(getDalDmlIntf());     
 SET_USER_DATA_FLAGS(ikey, 0x10);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_STATE,dmi,ctrl_domain));
 delete ikey;
}

TEST_F(vbr_if_test, GetRenamedControllerKey_VbrValidNotSet) {
 VbrIfFlowFilterMoMgr obj;
 controller_domain *ctrl_domain = ZALLOC_TYPE(controller_domain);
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);


 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS(ikey, 0x10);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));

 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 delete ikey;
}

TEST_F(vbr_if_test, ReadMo_Detail_Val_NotValid) {
 VbrIfFlowFilterMoMgr obj;
 IPC_REQ_RESP_HEADER_DECL(req);
 req->option1 = UNC_OPT1_DETAIL;
 DalOdbcMgr::clearStubData();
 req->datatype = UPLL_DT_STATE;
 key_vbr_if_flowfilter_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->if_key.if_name,"IFNAME1",32);
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 val_flowfilter_t *vbr_if_flowfilter_val = ZALLOC_TYPE(val_flowfilter_t);
  vbr_if_flowfilter_val->cs_row_status = 1;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_val);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER, IpctSt::kIpcStKeyVbrIfFlowfilter,key_vbr_if,tmp);

 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.ReadMo(NULL,ikey,dmi));

 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_test,IsReferenced_InputNull)
{
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *ikey= NULL;
  DalDmlIntf *dmi = NULL;
  
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_STATE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, ikey, dmi));
}
/*
TEST_F(vbr_if_test,IsReferenced_ValidInput)
{
  VbrIfFlowFilterMoMgr obj;
  key_vbr_if_flowfilter_t *key_vrt_if = ZALLOC_TYPE(key_vbr_if_flowfilter_t);
  strncpy((char*) key_vrt_if->if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vrt_if->if_key.vbr_key.vbridge_name,"VRT1",32);
  strncpy((char*) key_vrt_if->if_key.if_name,"IFNAME1",32);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
    const char * ctrlr_name = "cntrlr_name";
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_POLICINGMAP, IpctSt::kIpcStKeyVrtIfFlowfilter,key_vrt_if,NULL);
  DalDmlIntf *dmi(getDalDmlIntf());
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, ikey, dmi));
  delete ikey;
}

TEST(ConstructReadDetailResponse , Construct2_val) {
  VbrIfFlowFilterMoMgr obj;
  ConfigKeyVal *drv_ikey = NULL , *okey = NULL;
  controller_domain_t l_ctrlr_dom ;
  l_ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
            (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
  l_ctrlr_dom.domain = reinterpret_cast <uint8_t*>
            (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));
  uuu::upll_strncpy(l_ctrlr_dom.ctrlr,
                          "CTRLR",
                          (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(l_ctrlr_dom.domain,
                          "DOMAIN",
                          (kMaxLenDomainId + 1));

  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};

  key_vbr_if_flowfilter_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_t)));
  val_flowfilter_entry_t *l_flowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(l_flowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  ConfigVal *val_1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             l_flowfilter_entry_val);

  memset(key_vrt_if, 0 ,sizeof(key_vrt_if_flowfilter_t));
  uuu::upll_strncpy(key_vrt_if->if_key.vbr_key.vtn_key.vtn_name,
		        reinterpret_cast<const char *>("VTN1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(key_vrt_if->if_key.vbr_key.vbridge_name,
		        reinterpret_cast<const char *>("VRT1"), kMaxLenCtrlrId+1);
  val_flowfilter_entry_t *vrt_if_flowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vrt_if_flowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vrt_if_flowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vrt_if_flowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vrt_if_flowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vrt_if_flowfilter_entry_val);
  val_flowfilter_t *tmp_val_ff = reinterpret_cast<val_flowfilter_t *>
	                         (malloc(sizeof(val_flowfilter_t)));

  ConfigVal *val_ff = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             tmp_val_ff);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            key_vrt_if, val_ff);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);

  drv_ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, val);
  drv_ikey->AppendCfgVal(val_1);   
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::clearStubData();
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcNotDisconnected);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ConstructReadDetailResponse(ikey,drv_ikey,l_ctrlr_dom ,&okey  ,dmi ));
  delete dmi;
  delete ikey ;
  delete(drv_ikey);
  free(l_ctrlr_dom.ctrlr);
  free(l_ctrlr_dom.domain);
}
*/
