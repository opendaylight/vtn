/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <vbr_flowfilter_entry_momgr.hh>
#include <flowlist_momgr.hh>
#include <vbr_momgr.hh>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <ctrlr_mgr.hh>
#include <config_mgr.hh>
#include "ut_util.hh"
#include <pfcxx/synch.hh>


using namespace unc::upll::dal;
using namespace unc::upll::config_momgr;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::test;

#define GET_USER_DATA_TEST(ckey) { \
      void *user_data = (ckey)->get_user_data(); \
      user_data = malloc(sizeof(unc::upll::kt_momgr::key_user_data)); \
      if (user_data) { \
                memset(user_data, 0, sizeof(unc::upll::kt_momgr::key_user_data)); \
                (ckey)->set_user_data(user_data); \
               } \
      }

#define SET_USER_DATA_FLAGS_TEST(ckey, rename) \
    { \
         GET_USER_DATA_TEST(ckey)    \
         unc::upll::kt_momgr::key_user_data_t  *user_data = \
                     reinterpret_cast<unc::upll::kt_momgr::key_user_data_t *>(ckey->get_user_data()); \
         user_data->flags = (rename); \
    }

class VbrFlowFilterEntry
  : public UpllTestEnv
{
};
//U13
TEST(VerifyRedirectDestination,ikeyNull_VBR) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi =  NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_IMPORT ));
}
TEST(VerifyRedirectDestination,ikeynot_null_VBR) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_flowfilter_entry_t)));
  memset(key_vrt_if, 0 ,sizeof(key_vbr_flowfilter_entry_t));

  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);
  val_flowfilter_entry_t *vbr_flowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbr_flowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbr_flowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbr_flowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbr_flowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbr_flowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vrt_if, val);
  DalDmlIntf* dmi = new DalOdbcMgr();

  // EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
    delete (dmi);
    delete (ikey);
    ikey = NULL;
}
TEST(VerifyRedirectDestination,ikeynot_null_1_VBR) {
  VbrFlowFilterEntryMoMgr obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_flowfilter_entry_t)));
  memset(key_vrt_if, 0 ,sizeof(key_vbr_flowfilter_entry_t));
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);
  
  val_flowfilter_entry_t *vbrflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc((sizeof(key_user_data)+ 10)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.VerifyRedirectDestination(ikey, dmi, UPLL_DT_CANDIDATE));
  delete (dmi);
  delete (ikey);
  ikey = NULL ;
}
TEST(VerifyRedirectDestination,SemanticError) {
  VbrFlowFilterEntryMoMgr  obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_flowfilter_entry_t)));
  memset(key_vrt_if, 0 ,sizeof(key_vbr_flowfilter_entry_t));
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);
  val_flowfilter_entry_t *vbrflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);

  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC ,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_IMPORT ));
  delete dmi;
  delete ikey;

}
TEST(VerifyRedirectDestination,DIR_IN_UpdateConfigDB_GEN_ERR_VBR) {
  VbrFlowFilterEntryMoMgr obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_flowfilter_entry_t)));

  memset(key_vrt_if, 0 ,sizeof(key_vbr_flowfilter_entry_t));

  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);
  val_flowfilter_entry_t *vbrflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcNotDisconnected);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC ,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
  delete dmi;
  delete ikey ;
}
TEST(VerifyRedirectDestination,DIR_IN_UpdateConfigDB_INS_EXIST_VBR) {
  VbrFlowFilterEntryMoMgr obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_flowfilter_entry_t)));

  memset(key_vrt_if, 0 ,sizeof(key_vbr_flowfilter_entry_t));

  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);
  val_flowfilter_entry_t *vbrflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcNotDisconnected);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
  delete dmi;
  delete ikey ;
}
TEST(VerifyRedirectDestination,DIR_IN_UpdateConfigDB_NO_SUCH_INS_VBR) {
  VbrFlowFilterEntryMoMgr obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_flowfilter_entry_t)));

  memset(key_vrt_if, 0 ,sizeof(key_vbr_flowfilter_entry_t));

  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);
  val_flowfilter_entry_t *vbrflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));

  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);
  vbrflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcNotDisconnected);
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
  delete dmi;
  delete ikey ;
}
#if 0
TEST(VerifyRedirectDestination,DIR_IN_UpdateConfigDB_ACCESS_ERR_VBR) {
 VbrFlowFilterEntryMoMgr obj;
  controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_flowfilter_entry_t)));

  memset(key_vrt_if, 0 ,sizeof(key_vbr_flowfilter_entry_t));

  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);
  val_flowfilter_entry_t *vbrflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcNotDisconnected);
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcAccessViolation);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_DB_ACCESS,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
  delete dmi;
  delete ikey ;
}
#endif
TEST(ConstructReadDetailResponse , Construct2_keyNULL) {
  VbrFlowFilterEntryMoMgr obj;
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

  //key_vbr_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_flowfilter_entry_t*>
  //                 (malloc(sizeof(key_vbr_flowfilter_entry_t)));
  val_flowfilter_entry_t *l_flowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(l_flowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  ConfigVal *val_1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             l_flowfilter_entry_val);

 // memset(key_vrt_if, 0 ,sizeof(key_vbr_flowfilter_entry_t));

  //strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
 // strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);
  val_flowfilter_entry_t *vbrflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrflowfilter_entry_val);
  //ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
  //                          IpctSt::kIpcStKeyVbrFlowfilterEntry,
  //                          key_vrt_if, NULL);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  //ikey->set_user_data(user_data1);

  drv_ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, val);
  drv_ikey->AppendCfgVal(val_1);   
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::clearStubData();
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcNotDisconnected);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ConstructReadDetailResponse(NULL, drv_ikey,l_ctrlr_dom , dmi , &okey ));
  delete dmi;
  //delete ikey ;
  delete(drv_ikey);
  free(l_ctrlr_dom.ctrlr);
  free(l_ctrlr_dom.domain);
}

TEST(ConstructReadDetailResponse , Construct2_val) {
  VbrFlowFilterEntryMoMgr obj;
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

  key_vbr_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_flowfilter_entry_t)));
  val_flowfilter_entry_t *l_flowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(l_flowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  ConfigVal *val_1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             l_flowfilter_entry_val);

  memset(key_vrt_if, 0 ,sizeof(key_vbr_flowfilter_entry_t));

  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vrt_if->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);
  val_flowfilter_entry_t *vbrflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrflowfilter_entry_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vrt_if, NULL);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);

  drv_ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, val);
  drv_ikey->AppendCfgVal(val_1);   
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::clearStubData();
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcNotDisconnected);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ConstructReadDetailResponse(ikey, drv_ikey,l_ctrlr_dom , dmi , &okey ));
  delete dmi;
  delete ikey ;
  delete(drv_ikey);
  free(l_ctrlr_dom.ctrlr);
  free(l_ctrlr_dom.domain);
}
TEST(vbr_entry_test,Redirect_Field) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val1 =
    reinterpret_cast<val_flowfilter_entry_t *>(malloc(sizeof(val_flowfilter_entry_t)));
  val1->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE] = UNC_VF_VALID;
  val1->redirect_direction = UPLL_IDX_L4_SRC_PORT_ENDPT_FLE ; 

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX , obj.ValidateRedirectField(val1 , UNC_OP_CREATE));
  free (val1);
}
TEST(vbr_entry_test,Redirect_Field_1) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val1 =
    reinterpret_cast<val_flowfilter_entry_t *>(malloc(sizeof(val_flowfilter_entry_t)));
  val1->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE] = UNC_VF_INVALID;
  val1->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID;
  val1->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX , obj.ValidateRedirectField(val1 , UNC_OP_CREATE));
  free (val1);
}
TEST(vbr_entry_test,Redirect_Field_2) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val1 =
    reinterpret_cast<val_flowfilter_entry_t *>(malloc(sizeof(val_flowfilter_entry_t)));
  val1->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID_NO_VALUE;
  val1->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_VALID_NO_VALUE;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateRedirectField(val1 , UNC_OP_CREATE));
  free (val1);
}

//U13 End
TEST_F (VbrFlowFilterEntry, Output_Pos)
{
  VbrFlowFilterEntryMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_FLOWFILTER_ENTRY;
  BindInfo *bin = NULL;
  int nattr;

  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, MAINTBL));
  EXPECT_EQ(5, nattr);
}

#if 0 //Not Needed
TEST_F (VbrFlowFilterEntry, Output_Negtbl)
{
  VbrFlowFilterEntryMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_FLOWFILTER_ENTRY;
  BindInfo *bin = NULL;
  int nattr;

  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, CTRLRTBL));
}

TEST_F (VbrFlowFilterEntry, Output_Negtbl1)
{
  VbrFlowFilterEntryMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_FLOWFILTER_ENTRY;
  BindInfo *bin = NULL;
  int nattr;

  EXPECT_EQ(PFC_FALSE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, RENAMETBL));
}
#endif

TEST_F (VbrFlowFilterEntry, Output_KeyType)
{
  VbrFlowFilterEntryMoMgr obj;
  unc_key_type_t key_type = UNC_KT_FLOWLIST;
  BindInfo *bin = NULL;
  int nattr;

  EXPECT_EQ(PFC_TRUE, obj.GetRenameKeyBindInfo(key_type, bin, nattr, MAINTBL));
  EXPECT_EQ(5, nattr);
}

TEST_F(VbrFlowFilterEntry, AllocVal_ckvalnotNull) {
  VbrFlowFilterEntryMoMgr obj;
  
  val_flowfilter_entry_t *vbr_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbr_flowfilter_entry_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AllocVal(val, UPLL_DT_IMPORT, MAINTBL));
  std::cout<<"for null o/p"<<std::endl;
  //delete val;
}

TEST_F(VbrFlowFilterEntry, AllocVal_ckvalNull) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigVal* val = NULL;
                   
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(val, UPLL_DT_IMPORT, MAINTBL));
}

TEST_F(VbrFlowFilterEntry, AllocVal_valNull) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *vbr_flowfilter_entry_val = NULL; 
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbr_flowfilter_entry_val); 
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AllocVal(val, UPLL_DT_IMPORT, MAINTBL));           
  //delete val;
}

TEST_F(VbrFlowFilterEntry, AllocVal_tblerror) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *vbr_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbr_flowfilter_entry_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AllocVal(val, UPLL_DT_STATE, RENAMETBL));
  //delete val;
}

TEST_F(VbrFlowFilterEntry, GetChildConfigKey) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr, NULL);
  
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey)); 
  //delete pkey;
}

TEST_F(VbrFlowFilterEntry, GetChildConfigKey_pkeyNull) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, NULL);
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, pkey));   
  delete okey;
}

TEST_F(VbrFlowFilterEntry,GetChildConfigKey_pkeyNull1) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));    
  delete okey;
  delete pkey;
}

TEST_F(VbrFlowFilterEntry,GetChildConfigKey_pkeyNull2) {
  VbrFlowFilterEntryMoMgr obj;

  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr, NULL);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1", 32);

  //ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
    //                        IpctSt::kIpcStKeyVbrFlowfilterEntry,
     //                       key_vbr, NULL);

  key_vbr_flowfilter_entry_t *output = reinterpret_cast<key_vbr_flowfilter_entry_t *> (okey->get_key());

  std::cout <<"val"<<(reinterpret_cast<const char *> (output->flowfilter_key.vbr_key.vbridge_name))<<std::endl;

  EXPECT_STREQ("VTN1", (reinterpret_cast<const char *> (output->flowfilter_key.vbr_key.vtn_key.vtn_name)));
  EXPECT_STREQ("VBR1", (reinterpret_cast<const char *> (output->flowfilter_key.vbr_key.vbridge_name)));
  //delete pkey;
  delete okey;
}

TEST_F(VbrFlowFilterEntry, DupConfigKeyVal_reqnull) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, NULL);
  ConfigKeyVal *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));
  //delete okey;
} 
  
TEST_F(VbrFlowFilterEntry, DupConfigKeyVal_okeynull) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, NULL);

  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));
  //delete okey;
  //delete req;
}

TEST_F(VbrFlowFilterEntry, DupConfigKeyVal_Req_Invalid) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  
  val_flowfilter_entry_t *vbr_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 
  vbr_flowfilter_entry_val->cs_row_status = 1;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbr_flowfilter_entry_val); 
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, tmp);
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, MAINTBL));
  //delete req;
} 

 
TEST_F(VbrFlowFilterEntry, DupConfigKeyVal_Req_Valid) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr_flowfilter_entry_t *vbr_flowfilter_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  val_flowfilter_entry_t *vbr_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  vbr_flowfilter_entry_val->cs_row_status = 1;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbr_flowfilter_entry_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            vbr_flowfilter_entry, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(okey, req, MAINTBL));
  //delete req;
} 


TEST_F(VbrFlowFilterEntry, CopyToConfigkey_Ikey_NUll) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterEntry, CopyToConfigkey_valid) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;
  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  free (key_rename);
}

TEST_F(VbrFlowFilterEntry, CopyToConfigkey_keyvbrNull) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;
  key_vbr_flowfilter_entry_t *key_vbr_ff_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char *) key_vbr_ff_entry->flowfilter_key.vbr_key.vbridge_name,"VBR1", 32);

  //key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  EXPECT_STREQ("VBR1", (reinterpret_cast<const char *> (key_vbr_ff_entry->flowfilter_key.vbr_key.vbridge_name)));
  //free (key_rename);
}

TEST_F(VbrFlowFilterEntry, CopyToConfigkey_old_name) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;
  
  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  strncpy(reinterpret_cast<char *>(key_rename->old_unc_vtn_name), "OLDVTN1", 32);
  strncpy(reinterpret_cast<char *>(key_rename->old_unc_vnode_name), "OLDVNODE1", 32);
  strncpy(reinterpret_cast<char *>(key_rename->new_unc_vnode_name), "NEWVNODE1", 32);

 
  ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                         IpctSt::kIpcStKeyVbrFlowfilterEntry,
                         key_rename, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));

  key_vbr_flowfilter_entry_t *key_vbr_ff_entry = reinterpret_cast< key_vbr_flowfilter_entry_t *>(okey->get_key());
  //key_vbr_flowfilter_entry_t *key_vbr_ff_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  EXPECT_STREQ("OLDVTN1", (reinterpret_cast<const char *> (key_vbr_ff_entry->flowfilter_key.vbr_key.vtn_key.vtn_name)));
  EXPECT_STREQ("NEWVNODE1", (reinterpret_cast<const char *> (key_vbr_ff_entry->flowfilter_key.vbr_key.vbridge_name)));
  //delete ikey;
}

TEST_F(VbrFlowFilterEntry, CopyToConfigkey_KEYTYPE) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);


  ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                         IpctSt::kIpcStKeyVbrFlowfilterEntry,
                         key_rename, NULL);
  strncpy(reinterpret_cast<char *>(key_rename->old_unc_vtn_name), "OLDVTN1", 32);
  strncpy(reinterpret_cast<char *>(key_rename->old_unc_vnode_name), "OLDVNODE1", 32);
  strncpy(reinterpret_cast<char *>(key_rename->new_unc_vnode_name), "NEWVNODE1", 32);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterEntry, CopyToConfigkey_invalid_renametktype) {
  VbrFlowFilterEntryMoMgr obj;
  //key_vbr_flowfilter_entry_t *key_vbr_ff_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  ConfigKeyVal* okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);

  ConfigKeyVal *ikey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);

  ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                         IpctSt::kIpcStKeyFlowlist,
                         key_rename, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
  delete okey;
  delete ikey;
} 
#if 0
TEST_F(VbrFlowFilterEntry, CopyToConfigkey_valNull) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  strncpy(reinterpret_cast<char *>(key_rename->old_flowlist_name), "old_flowlist_name", 32);


  ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                         IpctSt::kIpcStKeyFlowlist,
                         key_rename, NULL);
  

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));

  val_flowfilter_entry_t *val = reinterpret_cast<val_flowfilter_entry_t*> (GetVal(okey));
  EXPECT_STREQ("old_flowlist_name", (reinterpret_cast<const char *> (val->flowlist_name)));
  
  //delete ikey;
}  
#endif

TEST_F(VbrFlowFilterEntry, RenameMo_NoOperation) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  const char* ctrlr_id = {0};

  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.RenameMo(req, ikey, dmi, ctrlr_id));
  //delete req;
}

TEST_F(VbrFlowFilterEntry, UpdateConfigStatus_ValStructureNull) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, NULL);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
 //delete key;
}

TEST_F(VbrFlowFilterEntry, UpdateConfigStatus_Success) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->cs_row_status = UNC_CS_NOT_APPLIED;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, tmp);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
 
 val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (GetVal(key));
 EXPECT_EQ(UNC_CS_APPLIED, output->cs_row_status);
 //delete key;
}

TEST_F(VbrFlowFilterEntry, UpdateConfigStatus_Success1) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->cs_row_status = UNC_CS_APPLIED;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, tmp);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 1, upd_key, dmi, ctrlr_key));
 
 val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (GetVal(key));
 EXPECT_EQ(UNC_CS_NOT_APPLIED, output->cs_row_status);
 //delete key;
}

TEST_F(VbrFlowFilterEntry, UpdateConfigStatus_Failure) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->cs_row_status = UNC_CS_NOT_APPLIED;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, tmp);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(key, UNC_OP_DELETE, 0, upd_key, dmi, ctrlr_key));
 //delete key; 
}

TEST_F(VbrFlowFilterEntry, UpdateConfigStatus_Success2) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->cs_row_status = UNC_CS_APPLIED;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, tmp);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(key, UNC_OP_UPDATE, 1, upd_key, dmi, ctrlr_key));

 val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (GetVal(key));
 EXPECT_EQ(UNC_CS_APPLIED, output->cs_row_status);
 delete key;
}

TEST_F (VbrFlowFilterEntry, UpdateConfigStatus_Failure1) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->cs_row_status = UNC_CS_NOT_APPLIED;
 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, tmp);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateConfigStatus(key, UNC_OP_UPDATE, 1, upd_key, dmi, ctrlr_key));
 //delete key; 
}

TEST_F(VbrFlowFilterEntry, UpdateConfigStatus_ValStructure_Valid) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowfilter_entry_t *vbr_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 vbr_flowfilter_entry_val->valid[2] = UNC_VF_VALID;

 ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbr_flowfilter_entry_val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, val);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 1, upd_key, dmi, ctrlr_key));
 val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (key->get_cfg_val()->get_val()); 
 EXPECT_EQ(UNC_CS_NOT_APPLIED,output->cs_attr[2]);
 //delete key;
}

TEST_F(VbrFlowFilterEntry, IsValid_InvalidVtnName ) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, 
                                     IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                     key_vbr_entry,NULL);

  EXPECT_EQ(false, obj.IsValidKey(key, uudst::vbr_flowfilter_entry::kDbiVtnName));
  //delete key;                  
}

TEST_F(VbrFlowFilterEntry, IsValid_InvalidVbrName ) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_entry, NULL);

  EXPECT_EQ(false, obj.IsValidKey(key, uudst::vbr_flowfilter_entry::kDbiVbrName));
  //delete key;
}
/*
TEST_F(VbrFlowFilterEntry, IsValid_InvalidSeqNum ) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_entry, NULL);

  EXPECT_EQ(false, obj.IsValidKey(key, uudst::vbr_flowfilter_entry::kDbiSequenceNum));
  delete key;
}

TEST_F(VbrFlowFilterEntry, IsValid_InvalidInputDir) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_entry, NULL);

  EXPECT_EQ(false, obj.IsValidKey(key, uudst::vbr_flowfilter_entry::kDbiInputDirection));
  delete key;
} 

TEST_F(VbrFlowFilterEntry, IsValid_ValidVtnName ) {
 VbrFlowFilterEntryMoMgr obj;
 key_vbr_flowfilter_entry_t *key_vbr_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
 strncpy((char *) key_vbr_entry->flowfilter_key.vbr_key.vtn_key.vtn_name, "VTN1", 32);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_entry, NULL);

 EXPECT_EQ(true, obj.IsValidKey(key, uudst::vbr_flowfilter_entry::kDbiVtnName));    
 //delete key;
}

TEST_F(VbrFlowFilterEntry, IsValid_ValidVbrName) {
 VbrFlowFilterEntryMoMgr obj;
 key_vbr_flowfilter_entry_t *key_vbr_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
 strncpy((char*) key_vbr_entry->flowfilter_key.vbr_key.vbridge_name, "VBR1", 32);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_entry, NULL);

 EXPECT_EQ(true, obj.IsValidKey(key, uudst::vbr_flowfilter_entry::kDbiVbrName));
 //delete key;
}
*/
TEST_F(VbrFlowFilterEntry, IsValid_InvalidIndex ) {
 VbrFlowFilterEntryMoMgr obj;
 key_vbr_flowfilter_entry_t *key_vbr_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_entry, NULL);

 EXPECT_EQ(false, obj.IsValidKey(key, 23)); 
 free (key_vbr_entry);
}

TEST_F(VbrFlowFilterEntry, ValidateAttribute_invalid_keytyp) {
 VbrFlowFilterEntryMoMgr obj;
 DalDmlIntf *dmi = NULL;
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER);

 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey, dmi));
 delete ikey;
}

TEST_F(VbrFlowFilterEntry, ValidateAttribute_Success) {
 VbrFlowFilterEntryMoMgr obj;
 DalDmlIntf *dmi(getDalDmlIntf());
 IPC_REQ_RESP_HEADER_DECL(req);
 ConfigKeyVal *ikey = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey, dmi, req));
 delete ikey;
}
TEST_F(VbrFlowFilterEntry,CompareValidValue_auditTrue) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val1 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val_flowfilter_entry_t *val2 = ZALLOC_TYPE(val_flowfilter_entry_t);
  for (uint8_t loop = 0; loop < sizeof(val1->valid); loop++) {
    val1->valid[loop] = UNC_VF_INVALID;
    val2->valid[loop] = UNC_VF_VALID;
    void *v1 = reinterpret_cast<void *>(val1);
    void *v2 = reinterpret_cast<void *>(val2);

    obj.CompareValidValue(v1, v2, true);
    EXPECT_EQ(UNC_VF_VALID_NO_VALUE, val1->valid[loop]);
  }
  //ee(val1); 
  //free(val2);
}

TEST_F(VbrFlowFilterEntry,CompareValidValue_auditfalse1) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val1 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val1->valid[0] = UNC_VF_VALID;
  strncpy((char *)val1->flowlist_name, "FLOWLISTNAME", 32);
  val_flowfilter_entry_t *val2 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val2->valid[0] = UNC_VF_VALID;
  strncpy((char *)val2->flowlist_name, "FLOWLISTNAME", 32);  
  
  void *v1 = reinterpret_cast<void *>(val1);
  void *v2 = reinterpret_cast<void *>(val2);

  obj.CompareValidValue(v1, v2, false);
  EXPECT_EQ(UNC_VF_INVALID, val1->valid[0]); 
  
  free (val1); 
  free (val2);
}

TEST_F(VbrFlowFilterEntry,CompareValidValue_auditfalse2) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val1 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val1->valid[1] = UNC_VF_VALID;
  val_flowfilter_entry_t *val2 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val2->valid[1] = UNC_VF_VALID;
 
  val1->action = UPLL_IDX_ACTION_FFE;
  val2->action = UPLL_IDX_ACTION_FFE; 
  void *v1 = reinterpret_cast<void *>(val1);
  void *v2 = reinterpret_cast<void *>(val2);

  obj.CompareValidValue(v1, v2, false);
  EXPECT_EQ(UNC_VF_INVALID, val1->valid[1]);

  free(val1);
  free(val2);
}

TEST_F(VbrFlowFilterEntry,CompareValidValue_auditfalse3) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val1 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val1->valid[2] = UNC_VF_VALID;
  strncpy((char *)val1->redirect_node, "REDNODE", 32);
  val_flowfilter_entry_t *val2 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val2->valid[2] = UNC_VF_VALID;
  strncpy((char *)val2->redirect_node, "REDNODE", 32);

  void *v1 = reinterpret_cast<void *>(val1);
  void *v2 = reinterpret_cast<void *>(val2);

  obj.CompareValidValue(v1, v2, false);
  EXPECT_EQ(UNC_VF_VALUE_NOT_MODIFIED, val1->valid[2]);

  free(val1);
  free(val2);
}

TEST_F(VbrFlowFilterEntry,CompareValidValue_auditfalse7) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val1 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val1->valid[7] = UNC_VF_VALID;
  strncpy((char *)val1->nwm_name, "NWMNAME", 32);
  val_flowfilter_entry_t *val2 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val2->valid[7] = UNC_VF_VALID;
  strncpy((char *)val2->nwm_name, "NWMNAME", 32);

  void *v1 = reinterpret_cast<void *>(val1);
  void *v2 = reinterpret_cast<void *>(val2);

  obj.CompareValidValue(v1, v2, false);
  EXPECT_EQ(UNC_VF_INVALID, val1->valid[6]);

  free (val1);
  free (val2);
}

TEST_F(VbrFlowFilterEntry,CompareValidValue_auditfalse8) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val1 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val1->valid[7] = UNC_VF_VALID;
  val_flowfilter_entry_t *val2 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val2->valid[7] = UNC_VF_VALID;

  val1->dscp = UPLL_IDX_DSCP_FFE;
  val2->dscp = UPLL_IDX_DSCP_FFE;
  void *v1 = reinterpret_cast<void *>(val1);
  void *v2 = reinterpret_cast<void *>(val2);

  obj.CompareValidValue(v1, v2, false);
  EXPECT_EQ(UNC_VF_INVALID, val1->valid[7]);

  free(val1);
  free(val2);
}

TEST_F(VbrFlowFilterEntry,CompareValidValue_auditfalse9) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val1 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val1->valid[8] = UNC_VF_VALID;
  val_flowfilter_entry_t *val2 = ZALLOC_TYPE(val_flowfilter_entry_t);
  val2->valid[8] = UNC_VF_VALID;

  val1->priority = UPLL_IDX_PRIORITY_FFE;
  val2->priority = UPLL_IDX_PRIORITY_FFE;
  void *v1 = reinterpret_cast<void *>(val1);
  void *v2 = reinterpret_cast<void *>(val2);

  obj.CompareValidValue(v1, v2, false);
  EXPECT_EQ(UNC_VF_INVALID, val1->valid[8]);

  free(val1);
  free(val2);
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_Null) {
  VbrFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_NullChk1) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_Invalid_KeyType) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req,ikey));
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_Valid_KeyType) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY); 

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_InValidKeyST) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilter);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterEntry,ValidateMsgNullKeyStruct) {
  VbrFlowFilterEntryMoMgr obj;
  /** set valid keytype, st_num */
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry);
  IPC_REQ_RESP_HEADER_DECL(req);
  /** key struct is NULL */
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_ValidMoManagerInstance) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  
  req->operation = UNC_OP_CREATE;
  req->datatype =  UPLL_DT_CANDIDATE;

  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = UPLL_FLOWFILTER_DIR_IN;
  key_vbr->sequence_num = 1;
   
  val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  entry_val->valid[UPLL_IDX_ACTION_FFE] = 1;
  entry_val->action = 1; 
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                entry_val);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
} 


TEST_F(VbrFlowFilterEntry, ValidateMessage_valStruct) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = UPLL_FLOWFILTER_DIR_IN;
  key_vbr->sequence_num = 1;

  val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  entry_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID_NO_VALUE;
  memset(entry_val->flowlist_name, 0,
             sizeof(entry_val->flowlist_name));
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                 entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                       IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr, val);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
    UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_UPDATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;


  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
  //delete ikey;
}


TEST_F(VbrFlowFilterEntry, ValidateMessage_valStruct_flowlistname) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = UPLL_FLOWFILTER_DIR_IN;
  key_vbr->sequence_num = 1;

  val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  strncpy(reinterpret_cast<char *>(entry_val->flowlist_name), "flowlistname1", 32);
  strncpy(reinterpret_cast<char *>(entry_val->nwm_name), "nwmname1", 32);

  //strncpy(reinterpret_cast<char *>(entry_val->redirect_node), "redirectnode1", 32);
  //strncpy(reinterpret_cast<char *>(entry_val->redirect_port), "redirectport1", 32);

  //entry_val->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID;
  //entry_val->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_VALID;


  entry_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  entry_val->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_VALID;
  entry_val->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
  entry_val->action = 1;

  entry_val->valid[UPLL_IDX_DSCP_FFE] = UNC_VF_VALID;
  entry_val->dscp = 1;
  entry_val->valid[UPLL_IDX_PRIORITY_FFE] = UNC_VF_VALID;
  entry_val->priority = 1;
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                 entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                       IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr, val);
  

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_valStruct_flowlistname1) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = UPLL_FLOWFILTER_DIR_IN;
  key_vbr->sequence_num = 1;

  val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  strncpy(reinterpret_cast<char *>(entry_val->flowlist_name), "flowlistname1", 32);
  strncpy(reinterpret_cast<char *>(entry_val->nwm_name), "nwmname1", 32);

  entry_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  entry_val->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_VALID;
  entry_val->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
  entry_val->action = 1;

  entry_val->valid[UPLL_IDX_DSCP_FFE] = UNC_VF_VALID;
  entry_val->dscp = 1;
  entry_val->valid[UPLL_IDX_PRIORITY_FFE] = UNC_VF_VALID;
  entry_val->priority = 1;
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                 entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                       IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr, val);


  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
   UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option1 = UNC_OPT1_DETAIL;
	
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
  delete ikey;
}


TEST_F(VbrFlowFilterEntry, ValidateMessage_valStruct_action) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = UPLL_FLOWFILTER_DIR_IN;
  key_vbr->sequence_num = 1;

  val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);

  entry_val->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
  entry_val->action = 1;
   
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                 entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                       IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr, val);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;


  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_valStruct_action_updatecase) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = UPLL_FLOWFILTER_DIR_IN;
  key_vbr->sequence_num = 1;

  val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);

  entry_val->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID_NO_VALUE;
  entry_val->action = 0;

  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                 entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                       IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr, val);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_UPDATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_valStruct_ActionCoditions) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = UPLL_FLOWFILTER_DIR_IN;
  key_vbr->sequence_num = 1;

  val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);

  entry_val->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_INVALID;
  entry_val->action = 0;
  entry_val->valid[UPLL_IDX_REDIRECT_PORT_FFE] = 1;
  entry_val->valid[UPLL_IDX_REDIRECT_NODE_FFE] = 1;
  entry_val->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = 1;
  entry_val->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = 1;
  entry_val->dscp =1;
  entry_val->priority = 1;
  entry_val->cs_row_status = 1;

  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                 entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                       IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr, val);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_valStruct_ActionCoditions1) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = UPLL_FLOWFILTER_DIR_IN; 
  key_vbr->sequence_num = 1;

  val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  strncpy(reinterpret_cast<char *>(entry_val->flowlist_name), "flowlistname1", 32);
  entry_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;


  entry_val->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
  entry_val->action = UPLL_FLOWFILTER_ACT_REDIRECT; 
  //entry_val->action = 1;

  entry_val->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_INVALID;
  strncpy(reinterpret_cast<char*>(entry_val->redirect_node), "_$node1", 32);
  entry_val->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_INVALID;
  strncpy(reinterpret_cast<char*>(entry_val->redirect_port), "_$port1", 32);


  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                 entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                       IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr, val);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
  //delete ikey;
}

TEST_F(VbrFlowFilterEntry, ValidateMessage_valStruct_action_updatecase1) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = UPLL_FLOWFILTER_DIR_IN;
  key_vbr->sequence_num = 1;

  val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  
  entry_val->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID_NO_VALUE;
  entry_val->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID_NO_VALUE;

  entry_val->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = UNC_VF_VALID_NO_VALUE;
  memset(entry_val->modify_dstmac, 0,
         sizeof(entry_val->modify_dstmac));

  entry_val->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = UNC_VF_VALID_NO_VALUE;
  memset(entry_val->modify_srcmac, 0,
         sizeof(entry_val->modify_srcmac));
  entry_val->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_VALID_NO_VALUE;
  memset(entry_val->nwm_name, 0,
         sizeof(entry_val->nwm_name));
  entry_val->valid[UPLL_IDX_DSCP_FFE] = UNC_VF_VALID_NO_VALUE;
  entry_val->dscp = 0;
  entry_val->valid[UPLL_IDX_PRIORITY_FFE] = UNC_VF_VALID_NO_VALUE;
  entry_val->priority = 0;

  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                 entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                       IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr, val);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_UPDATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
  //delete ikey;
}


TEST_F(VbrFlowFilterEntry, UpdateMo_NullChcek) {
  VbrFlowFilterEntryMoMgr obj;
  //ConfigKeyVal* okey = NULL;
  IpcReqRespHeader *req= NULL;
  DalDmlIntf *dmi = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY); 

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.UpdateMo(req, ikey , dmi));
  delete ikey;
} 


TEST_F(VbrFlowFilterEntry, ValidateCapa_NullChk) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  const char * ctrlr_name = NULL;
  ConfigKeyVal *ikey = NULL;
 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(req, ikey, ctrlr_name));
  delete req;
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_InvalidKey) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= NULL;
  ConfigKeyVal *key = NULL;
  const char * ctrlr_name = NULL;
 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ValidateCapability(req,key,ctrlr_name));
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_Invalid_Create) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_STATE;
  const char * ctrlr_name = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(req, ikey, ctrlr_name));
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_Invalid_Update) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  req->operation = UNC_OP_UPDATE;
  req->datatype = UPLL_DT_STATE;
  ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);
  const char * ctrlr_name = NULL;
 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ValidateCapability(req,key,ctrlr_name));
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_Invalid_Delete) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  req->operation = UNC_OP_DELETE;
  req->datatype = UPLL_DT_STATE;
  ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);
  const char * ctrlr_name = NULL;
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ValidateCapability(req,key,ctrlr_name));
}  

TEST_F(VbrFlowFilterEntry, IsReferenced_Invalid_key) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *key = NULL;
 DalDmlIntf *dmi(getDalDmlIntf());
 IPC_REQ_RESP_HEADER_DECL(req);
 req->datatype =  UPLL_DT_CANDIDATE;

 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, key, dmi));
}

TEST_F(VbrFlowFilterEntry, IsReferenced_Invalid_dmi) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);
 DalDmlIntf *dmi = NULL;
 IPC_REQ_RESP_HEADER_DECL(req);
 req->datatype =  UPLL_DT_CANDIDATE;

 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, key, dmi));
}

TEST_F(VbrFlowFilterEntry, IsReferenced_valid_update) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_CANDIDATE;

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());


 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, key, dmi));
 //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, IsReferenced_valid_update1) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 IPC_REQ_RESP_HEADER_DECL(req);
 req->datatype =  UPLL_DT_CANDIDATE;

 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, key, dmi));
 //DalOdbcMgr::clearStubData();
 //delete key;
}

TEST_F(VbrFlowFilterEntry, IsReferenced_valid_update2) {
 VbrFlowFilterEntryMoMgr obj;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 IPC_REQ_RESP_HEADER_DECL(req);
 req->datatype =  UPLL_DT_CANDIDATE;

 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.IsReferenced(req, key, dmi));
 //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, IsReferenced_valid_update3_fail) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_CANDIDATE;

 key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);


 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
 key_vbr->flowfilter_key.direction = 1;
 key_vbr->sequence_num = 1;
 
 val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy(reinterpret_cast<char *>(entry_val->flowlist_name), "flowlistname1", 32); 
 entry_val->valid[UPLL_IDX_ACTION_FFE] = 1;
 entry_val->action = 1;
 entry_val->valid[UPLL_IDX_REDIRECT_PORT_FFE] = 1;
 entry_val->valid[UPLL_IDX_REDIRECT_NODE_FFE] = 1;
 entry_val->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = 1;
 entry_val->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = 1;
 entry_val->dscp =1;
 entry_val->priority = 1;
 entry_val->cs_row_status = 1;
 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                 entry_val);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                       IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr, val);
 

 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 DalOdbcMgr::stub_setSingleRecordExists(false); 

 EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.IsReferenced(req, ikey, dmi));
 //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, IsReferenced_valid_update3_pass) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_CANDIDATE;

 key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);


 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
 key_vbr->flowfilter_key.direction = 1;
 key_vbr->sequence_num = 1;

 val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy(reinterpret_cast<char *>(entry_val->flowlist_name), "flowlistname1", 32);
 entry_val->valid[UPLL_IDX_ACTION_FFE] = 1;
 entry_val->action = 1;
 entry_val->valid[UPLL_IDX_REDIRECT_PORT_FFE] = 1;
 entry_val->valid[UPLL_IDX_REDIRECT_NODE_FFE] = 1;
 entry_val->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = 1;
 entry_val->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = 1;
 entry_val->dscp =1;
 entry_val->priority = 1;
 entry_val->cs_row_status = 1;
 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                 entry_val);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                       IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr, val);

 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 DalOdbcMgr::stub_setSingleRecordExists(true);

 EXPECT_EQ(UPLL_RC_ERR_INSTANCE_EXISTS, obj.IsReferenced(req, ikey, dmi));
 //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, GetRenamedUncKey_CtlrInValid) {
 VbrFlowFilterEntryMoMgr obj;
 uint8_t * ctrlr_id = NULL;
 DalDmlIntf *dmi = NULL;

 key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"OLDVTNNAME",32);
 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"OLDVBRNAME",32);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, NULL);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedUncKey(ikey, UPLL_DT_STATE,dmi,ctrlr_id));

// UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(VbrFlowFilterEntry, GetRenamedUncKey_Valid) {
 VbrFlowFilterEntryMoMgr obj;
 uint8_t ctrlr_id[4] = {'p', 'f', 'c'};
 key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"OLDVTNNAME",32);
 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"OLDVBRNAME",32);

 val_flowfilter_entry_t *vbr_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                vbr_flowfilter_entry_val); 
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, val);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetRenamedUncKey(ikey, UPLL_DT_STATE,dmi,ctrlr_id));

// UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(VbrFlowFilterEntry, GetRenamedControllerKey_VtnInvalidInstance) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, NULL);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  controller_domain *ctrl_domain = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  SET_USER_DATA_FLAGS_TEST(ikey, 0x01);
 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));

//  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
  //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, GetRenamedControllerKey_ValidNotSet) {
 VbrFlowFilterEntryMoMgr obj;
 controller_domain *ctrl_domain = NULL;
 key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry,key_vbr,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS_TEST(ikey, 0x10);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));

// UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, GetRenamedControllerKey_VbrInvalidInstance) {
 VbrFlowFilterEntryMoMgr obj;
 key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry,key_vbr,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 controller_domain *ctrl_domain = NULL;
 DalDmlIntf *dmi(getDalDmlIntf());
 SET_USER_DATA_FLAGS_TEST(ikey, 0x10);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));

// UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear(); 
 //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, GetRenamedControllerKey_VbrValidNotSet) {
 VbrFlowFilterEntryMoMgr obj;
 controller_domain *ctrl_domain = NULL;
 key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS_TEST(ikey, 0x10);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_STATE,dmi,ctrl_domain));

// UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
// DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, GetRenamedControllerKey_FlowlistInvalidInstance) {
 VbrFlowFilterEntryMoMgr obj;
 key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

 val_flowfilter_entry_t *vbr_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                vbr_flowfilter_entry_val);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, val);

 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

 controller_domain *ctrl_domain = NULL;
 DalDmlIntf *dmi(getDalDmlIntf());
 SET_USER_DATA_FLAGS_TEST(ikey, 0x11);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_STATE,dmi,ctrl_domain));
// UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 //DalOdbcMgr::clearStubData();
}
TEST_F(VbrFlowFilterEntry, Update_Audit) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey, dmi));

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

  ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey,dmi));
  val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  EXPECT_EQ(UNC_CS_APPLIED,output->cs_row_status);
}

TEST_F(VbrFlowFilterEntry, Update_Audit_Update) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  val->valid[0] = UNC_VF_VALID;
  val->valid[1] = UNC_VF_INVALID;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);
  
  ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL,tmp);

 DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpUpdate, ikey,dmi));
  val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  EXPECT_EQ(UNC_CS_APPLIED,output->cs_attr[0]);
}

TEST_F(VbrFlowFilterEntry, MerGeValidate_Success) {
  VbrFlowFilterEntryMoMgr obj;
  //IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  const char* ctrlr_id = {0};
  
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.MergeValidate(UNC_KT_VBR_FLOWFILTER_ENTRY, ctrlr_id, ikey, dmi, (upll_import_type)0));
}

TEST_F (VbrFlowFilterEntry, MerGeValidate_Failure) {
  VbrFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  char ctrlr_id[] = "PFC";
  val_flowfilter_entry_t  *entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);

  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  strncpy((char*) entry_val->flowlist_name,"FLOWLISTNAME",32);

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                entry_val);
  
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.MergeValidate(UNC_KT_VBR_FLOWFILTER_ENTRY, ctrlr_id, ikey, dmi, (upll_import_type)0));
  //DalOdbcMgr::clearStubData();
 // UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
} 

TEST_F(VbrFlowFilterEntry, ValidateCapability_SuccessInvalid1) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_RUNNING;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;

 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version, 0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL,tmp);
  ikey->set_user_data((void*)ctrlr_name);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_SuccessInvalid2) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_UPDATE;
 req->datatype = UPLL_DT_RUNNING;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;

 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version, 0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL,tmp);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
}
 
TEST_F(VbrFlowFilterEntry, ValidateCapability_SuccessUpdatevalNull) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_UPDATE;
 req->datatype = UPLL_DT_RUNNING;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;

 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version, 0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);

 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, tmp);
 EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
} 
 

TEST_F(VbrFlowFilterEntry, ValidateCapability_Success1) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_CANDIDATE;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);

 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version, 0);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_CANDIDATE));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);


  //ikey->set_user_data((void*)ctrlr_name);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, key, ctrlr_name));
 //  CapaModuleStub::stub_clearStubData();
}
/*
TEST_F(VbrFlowFilterEntry, ValidateCapability_Success2) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_UPDATE;
 req->datatype = UPLL_DT_CANDIDATE;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);

 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_CANDIDATE));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);


 //ikey->set_user_data((void*)ctrlr_name);
 EXPECT_EQ(true, obj.ValidateCapability(req, key, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_Success3) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_RUNNING;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);

 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);


 //key->set_user_data((void*)ctrlr_name);
 EXPECT_EQ(false, obj.ValidateCapability(req, key, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_Successval1) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_RUNNING;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);

 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL,tmp);
 //ikey->set_user_data((void*)ctrlr_name);
 EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateCapability(req, ikey, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_SuccessREAD_InvalidOption2) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ_SIBLING_BEGIN;
 req->datatype = UPLL_DT_STATE;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_L2DOMAIN;

 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL,tmp);
 //ikey->set_user_data((void*)ctrlr_name);
 EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateCapability(req, ikey, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_SuccessREAD_MaxAttrsChk) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ_SIBLING;
 req->datatype = UPLL_DT_STATE;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;

 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL,tmp);
 //ikey->set_user_data((void*)ctrlr_name);
 EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateCapability(req, ikey, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_SuccessREAD_ValNullChk) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ_SIBLING;
 req->datatype = UPLL_DT_STATE;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;

 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL,tmp);
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateCapability(req, ikey, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_Delete) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_DELETE;
 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_CANDIDATE));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL,tmp);
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateCapability(req, ikey, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateCapability_InValidOperation) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_RENAME;
 const char * ctrlr_name = "cntrlr_name";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_CANDIDATE));
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);


 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL,tmp);
 EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateCapability(req, ikey, ctrlr_name));
 //CapaModuleStub::stub_clearStubData();
}

*/
TEST_F(VbrFlowFilterEntry, SupportCheck_flname_Null) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  uint8_t attrs[9];
  
  for(int i= 0; i < 9; i++) {
    attrs[i] = 1;
    val->valid[i] = UNC_VF_VALID;
  } 
  attrs[0] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValFlowFilterEntryAttributeSupportCheck(val, attrs));
  EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val->valid[UPLL_IDX_FLOWLIST_NAME_FFE]);
}

TEST_F(VbrFlowFilterEntry, SupportCheck_Action_Null) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  uint8_t attrs[9];
  for(int i= 0; i < 9; i++) {
    attrs[i] = 1;
    val->valid[i] = UNC_VF_VALID;
  }
  attrs[1] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValFlowFilterEntryAttributeSupportCheck(val, attrs));
  EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val->valid[UPLL_IDX_ACTION_FFE]);
}

TEST_F(VbrFlowFilterEntry, SupportCheck_rednode_Null) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  uint8_t attrs[9];
  for(int i= 0; i < 9; i++) {
    attrs[i] = 1;
    val->valid[i] = UNC_VF_VALID;
  }
  attrs[2] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValFlowFilterEntryAttributeSupportCheck(val, attrs));
  EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val->valid[UPLL_IDX_REDIRECT_NODE_FFE]);
}

TEST_F(VbrFlowFilterEntry, SupportCheck_redport_Null) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  uint8_t attrs[9];
  for(int i= 0; i < 9; i++) {
    attrs[i] = 1;
    val->valid[i] = UNC_VF_VALID;
  }
  attrs[3] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValFlowFilterEntryAttributeSupportCheck(val, attrs));
  EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val->valid[UPLL_IDX_REDIRECT_PORT_FFE]);
}
TEST_F(VbrFlowFilterEntry, SupportCheck_dstmac_Null) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  uint8_t attrs[9];
  for(int i= 0; i < 9; i++) {
    attrs[i] = 1;
    val->valid[i] = UNC_VF_VALID;
  }
  attrs[5] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValFlowFilterEntryAttributeSupportCheck(val, attrs));
  EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val->valid[UPLL_IDX_MODIFY_DST_MAC_FFE]);
}
TEST_F(VbrFlowFilterEntry, SupportCheck_srcmac_Null) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  uint8_t attrs[9];
  for(int i= 0; i < 9; i++) {
    attrs[i] = 1;
    val->valid[i] = UNC_VF_VALID;
  }
  attrs[6] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValFlowFilterEntryAttributeSupportCheck(val, attrs));
  EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE]);
}
TEST_F(VbrFlowFilterEntry, SupportCheck_nwmname_Null) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  uint8_t attrs[9];
  for(int i= 0; i < 9; i++) {
    attrs[i] = 1;
    val->valid[i] = UNC_VF_VALID;
  }
  attrs[7] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValFlowFilterEntryAttributeSupportCheck(val, attrs));
  EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val->valid[UPLL_IDX_NWM_NAME_FFE]);
}
/*
TEST_F(VbrFlowFilterEntry, SupportCheck_dscp_Null) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  uint8_t attrs[9];
  for(int i= 0; i < 9; i++) {
    attrs[i] = 1;
    val->valid[i] = UNC_VF_VALID;
  }
  attrs[7] = 64;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValFlowFilterEntryAttributeSupportCheck(val, attrs));
  EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val->valid[UPLL_IDX_DSCP_FFE]);
  free (val);
}

TEST_F(VbrFlowFilterEntry, SupportCheck_priority_Null) {
  VbrFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  uint8_t attrs[9];
  for(int i= 0; i < 9; i++) {
    attrs[i] = 1;
    val->valid[i] = UNC_VF_VALID;
  }
  attrs[8] = 8;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValFlowFilterEntryAttributeSupportCheck(val, attrs));
  EXPECT_EQ(UNC_VF_NOT_SUPPORTED, val->valid[UPLL_IDX_PRIORITY_FFE]);
  //free (val);
}
*/
/**********************CREATE CANDIDATE MO *************************/
/*
TEST_F(CreateCandidateMo,Validate_CreateCandidate) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf()); 
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = 1;
  key_vbr->sequence_num = 1;
  

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);
  strncpy(reinterpret_cast<char *>(val->flowlist_name), "flowlistname1", 32);
  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  IPC_REQ_RESP_HEADER_DECL(req);
  
  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;   
  //req->option2 = UNC_OPT2_MAC_ENTRY; 
  
  const char* version("version");
  const char * ctrlr_name = "PFC";

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_CANDIDATE));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);

  strncpy(reinterpret_cast< char *>(key_vbr->
                          flowfilter_key.vbr_key.vtn_key.vtn_name),"VTN1", 33);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr, tmp1);

  //ikey->set_user_data((void*)ctrlr_name);
  EXPECT_EQ(UPLL_RC_ERR_INSTANCE_EXISTS, obj.CreateCandidateMo(req, ikey,dmi));
  //DalOdbcMgr::clearStubData();
}
*/

TEST_F(VbrFlowFilterEntry, CanddidateMo_validateMessage) {
  VbrFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  val_flowfilter_entry_t *val =
                         new val_flowfilter_entry_t();

  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = 1;
  key_vbr->sequence_num = 1;
  val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(val->flowlist_name), "flowlistname1", 32);\
  val->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(val->nwm_name), "nwmname1", 32);
  val->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
  val->valid[UPLL_IDX_ACTION_FFE] = UPLL_FLOWFILTER_ACT_REDIRECT;
  
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
 
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr, tmp1);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.CreateCandidateMo(req, ikey,dmi));
  //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterEntry, CANDIDATEmO_validateMessage1) {
  VbrFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  key_vbr_flowfilter_entry_t *key_vbr = NULL;
  key_vbr = new key_vbr_flowfilter_entry_t();
  val_flowfilter_entry_t *val =
                         new val_flowfilter_entry_t();

  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr->flowfilter_key.direction = 1;
  key_vbr->sequence_num = 1;
  val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(val->flowlist_name), "flowlistname1", 32);\
  val->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(val->nwm_name), "nwmname1", 32);
  val->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
  val->valid[UPLL_IDX_ACTION_FFE] = UPLL_FLOWFILTER_ACT_REDIRECT;

  val->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_INVALID;
  strncpy(reinterpret_cast<char*>(val->redirect_node), "_$node1", 32);
  val->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_INVALID;
  strncpy(reinterpret_cast<char*>(val->redirect_port), "_$port1", 32);
  val->dscp =1;
  val->priority = 1;
  val->cs_row_status = 1;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::COPY_MATCHING,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr, tmp1);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.CreateCandidateMo(req, ikey,dmi));
  //CapaModuleStub::stub_clearStubData();
}

TEST_F(VbrFlowFilterEntry, CreateCandidateMo_ReqNull) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = NULL;
 ConfigKeyVal *key = NULL;
 DalDmlIntf *dmi = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req,key,dmi));
}

TEST_F(VbrFlowFilterEntry, CreateCandidateMo_KeyNull) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 ConfigKeyVal *key = NULL;
 DalDmlIntf *dmi = NULL;
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.CreateCandidateMo(req,key,dmi));
}
/*
TEST_F(VbrFlowFilterEntry, CreateCandidateMo_Valid_datai1) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_flowfilter_entry_t *key_vbr_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
 strncpy((char*) key_vbr_entry->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_entry->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
 key_vbr_entry->sequence_num = 1;

 val_flowfilter_entry_t *vbr_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                vbr_flowfilter_entry_val);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry,key_vbr_entry,val);
 const char ctrlr_name[] = "ctrlr_name";
 ikey->set_user_data((void*)ctrlr_name);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.CreateCandidateMo(req,ikey,dmi));
 //lOdbcMgr::clearStubData();
 //llConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(VbrFlowFilterEntry, CreateCandidateMo_Valid_data2) {
 VbrFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_DELETE;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_flowfilter_entry_t *key_vbr_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
 strncpy((char*) key_vbr_entry->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_entry->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

 val_flowfilter_entry_t *vbr_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                vbr_flowfilter_entry_val);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry,key_vbr_entry,val);
 const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.clear();
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);

 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 ikey->set_user_data((void*)ctrlr_name);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_INSTANCE_EXISTS,obj.CreateCandidateMo(req,ikey,dmi));
 //DalOdbcMgr::clearStubData();
 //UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}
*/

TEST_F(VbrFlowFilterEntry,Validate1_CreateCandidate) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req = NULL;
  DalDmlIntf *dmi = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req, ikey,dmi));
}
TEST_F(VbrFlowFilterEntry,Validate1_Delete) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req = NULL;
  DalDmlIntf *dmi = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteMo(req, ikey,dmi));
}

TEST_F(VbrFlowFilterEntry,Validate2_Delete) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = NULL;
  key_vbr_flowfilter = new key_vbr_flowfilter_entry_t();
  IPC_REQ_RESP_HEADER_DECL(req);
  val_flowfilter_entry_t *ival =
                         new val_flowfilter_entry_t();
  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             ival);

 
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr_flowfilter,tmp1);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.DeleteMo(req, ikey,dmi));
  //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, Validate4_Delete) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = NULL;
  key_vbr_flowfilter = new key_vbr_flowfilter_entry_t();

  val_flowfilter_entry_t *ival =
                         new val_flowfilter_entry_t();
  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             ival);
  ival->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(ival->flowlist_name), "flowlistname1", 32);
 
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;


  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_DELETE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  const char * ctrlr_name = "PFC";

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(false);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr_flowfilter,tmp1);
  ikey->set_user_data((void*)ctrlr_name);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.DeleteMo(req, ikey,dmi));
 // DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, Validate5_Delete) {
  VbrFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);

  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  const char ctrlr_name[] = "PFC";
  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_DELETE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = NULL;
  key_vbr_flowfilter = new key_vbr_flowfilter_entry_t();

  val_flowfilter_entry_t *ival =
                         new val_flowfilter_entry_t();
  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             ival);
  ival->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(ival->flowlist_name), "flowlistname1", 32);

  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr_flowfilter,tmp1);
  ikey->set_user_data((void*)ctrlr_name);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.DeleteMo(req, ikey, dmi));
  //DalOdbcMgr::clearStubData();
}
 
TEST_F(VbrFlowFilterEntry, UpdatemO_validationMessCapa) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  
  const char* version("version");
  const char * ctrlr_name = "PFC";
 
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
    //UpllConfigMgr *upll_obj = UpllConfigMgr::GetUpllConfigMgr();
    UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = NULL;
  key_vbr_flowfilter = new key_vbr_flowfilter_entry_t();
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

  val_flowfilter_entry_t *ival =
                         new val_flowfilter_entry_t();
  ival->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(ival->flowlist_name), "flowlistname1", 32);

  
  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             ival);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr_flowfilter, tmp1);
  ikey->set_user_data((void*)ctrlr_name);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.UpdateMo(req, ikey,dmi));
  
  //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, UpdatemO_validationMessCapa1) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  //req->option2 = UNC_OPT2_L2DOMAIN;

  const char* version("version");
  const char * ctrlr_name = "PFC";

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = NULL;
  key_vbr_flowfilter = new key_vbr_flowfilter_entry_t();
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"_$VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"_$VBR1",32);
  key_vbr_flowfilter->flowfilter_key.direction = -1;
  key_vbr_flowfilter->sequence_num = -3;

  val_flowfilter_entry_t *ival =
                         new val_flowfilter_entry_t();
  ival->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(ival->flowlist_name), "_$flowlistname1", 32);


  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             ival);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr_flowfilter, tmp1);
  ikey->set_user_data((void*)ctrlr_name);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.UpdateMo(req, ikey,dmi));

  //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, UpdatemO_validationMessCapa2) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_L2DOMAIN;

  const char* version("version");
  const char * ctrlr_name = "PFC";

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = NULL;
  key_vbr_flowfilter = new key_vbr_flowfilter_entry_t();
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

  val_flowfilter_entry_t *ival =
                         new val_flowfilter_entry_t();
  ival->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(ival->flowlist_name), "flowlistname1", 32);

  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             ival);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr_flowfilter, tmp1);
  ikey->set_user_data((void*)ctrlr_name);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.UpdateMo(req, ikey,dmi));

  //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, UpdatemO_validationMessCapa3) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigVal *tmp1 = NULL;
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype =  UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_UPDATE;
  req->option1 = UNC_OPT1_NORMAL;
  //req->option2 = UNC_OPT2_L2DOMAIN;

  const char* version("version");
  const char * ctrlr_name = "PFC";

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version,0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = NULL;
  key_vbr_flowfilter = new key_vbr_flowfilter_entry_t();
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);
  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

  val_flowfilter_entry_t *ival =
                         new val_flowfilter_entry_t();
  ival->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(ival->flowlist_name), "flowlistname1", 32);

  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             ival);
  ConfigKeyVal *ikey  = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            key_vbr_flowfilter, tmp1);
  ikey->set_user_data((void*)ctrlr_name);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.UpdateMo(req, ikey,dmi));

  //DalOdbcMgr::clearStubData();
}

/*
TEST_F(VbrFlowFilterEntry, Validate_ReadMo) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->datatype = UPLL_DT_STATE;
  
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = ZALLOC_TYPE(key_vbr_flowfilter_entry_t); 
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;
 
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_entry_t); 
  strncpy(reinterpret_cast<char *>(val_flowfilter->flowlist_name), "flowlistname1", 32);


  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val_flowfilter);

  ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
		IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_flowfilter, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
  //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, Validate_ReadMo_DTSTATE1) {
  VbrFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
 
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_entry_t); 
  strncpy(reinterpret_cast<char *>(val_flowfilter->flowlist_name), "flowlistname1", 32);

  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
}

TEST_F(VbrFlowFilterEntry, Validate_ReadMo_DTSTATE2) {
  VbrFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_entry_t); 
  strncpy(reinterpret_cast<char *>(val_flowfilter->flowlist_name), "flowlistname1", 32);

  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.ReadMo(req, ikey, dmi));
}

TEST_F(VbrFlowFilterEntry, Validate_ReadSiblingMo) {
  VbrFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->datatype = UPLL_DT_STATE;
  req->option2 = UNC_OPT2_NONE;

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = ZALLOC_TYPE(key_vbr_flowfilter_entry_t); 
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_entry_t); 
  strncpy(reinterpret_cast<char *>(val_flowfilter->flowlist_name), "flowlistname1", 32);


  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val_flowfilter);

  ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_flowfilter, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadSiblingMo(req, ikey, dmi));
  //DalOdbcMgr::clearStubData();
}

TEST_F(VbrFlowFilterEntry, Validate_ReadSiblingMo_DTSTATE) {
  VbrFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_entry_t); 
  strncpy(reinterpret_cast<char *>(val_flowfilter->flowlist_name), "flowlistname1", 32);

  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadSiblingMo(req, ikey, dmi));
}

TEST_F(VbrFlowFilterEntry, Validate_ReadSiblingMo_DTSTATE1) {
  VbrFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_entry_t);
  strncpy(reinterpret_cast<char *>(val_flowfilter->flowlist_name), "flowlistname1", 32);

  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());


 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadSiblingMo(req, ikey, dmi));
}

TEST_F(VbrFlowFilterEntry, Validate_ReadSiblingMo_DTSTATE2) {
  VbrFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
req->option2 = UNC_OPT2_NONE;
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  val_flowfilter_entry_t *val_flowfilter = ZALLOC_TYPE(val_flowfilter_entry_t); 
  strncpy(reinterpret_cast<char *>(val_flowfilter->flowlist_name), "flowlistname1", 32);

  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val_flowfilter);

 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_flowfilter, config_val);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, obj.ReadSiblingMo(req, ikey, dmi));
}
*/
TEST_F(VbrFlowFilterEntry, GetControllerId_pass) {
  VbrFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);

  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_CANDIDATE;
  //req->option1 = UNC_OPT1_NORMAL;
  //req->option2 = UNC_OPT2_NONE;
  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  const char *controller_id = NULL;
  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name, "VTNNAME", 32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name, "VBRNAME", 32);

  val_vbr_t *entry_val = ZALLOC_TYPE(val_vbr_t);
  //strncpy(reinterpret_cast<char *>(entry_val->flowlist_name), "flowlistname1", 32);
  strncpy((char *) entry_val->controller_id, "PFC", 32);
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, val);
  ikey->set_user_data((void*)controller_id);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetControllerId(ikey, dmi));
}

/*
TEST_F (VbrFlowFilterEntry, OptDetail) {
  VbrFlowFilterEntryMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_STATE;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_MAC_ENTRY;
  
  const char * ctrlr_name = "PFC";

  if (UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.empty())
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

  key_vbr_flowfilter_entry_t *key_vbr_flowfilter = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_flowfilter->flowfilter_key.vbr_key.vbridge_name,"VBR1",32);

  key_vbr_flowfilter->flowfilter_key.direction = 1;
  key_vbr_flowfilter->sequence_num = 1;

 controller_domain *ctrl_domain = new controller_domain();

  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  
 ConfigKeyVal *ikey =  new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr_flowfilter, NULL);
 //SET_USER_DATA_FLAGS_TEST(ikey, 0x10);
  ikey->set_user_data((void*)ctrlr_name);
  DalDmlIntf *dmi(getDalDmlIntf());
  val_flowfilter_entry_t *val = reinterpret_cast<val_flowfilter_entry_t*> (GetVal(ikey)); 
  IpcRequest *ipc_req = new IpcRequest();
  IpcResponse *ipc_response = ZALLOC_TYPE(IpcResponse);
  req->clnt_sess_id = 1;
  req->operation = UNC_OP_READ;
  ConfigKeyVal *pkey = NULL;
  pkey = reinterpret_cast<ConfigKeyVal *>(ipc_response->ckv_data);
  
  //IpcReqRespHeader *req1 = reinterpret_cast<IpcReqRespHeader *>((ipc_response)->header);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ReadMo(req, ikey, dmi));
} */
#if 0

TEST_F(VbrFlowFilterEntry, ReadDetailEntry_NullKeyCheck) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal* ikey = NULL;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  DalDmlIntf *dmi = NULL;
  dmi = new DalOdbcMgr();

  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  /* char* version("version");*/
  uint8_t *ctrlr_id;
  uint8_t *ctrlr_dom;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadDetailEntry(ikey,
                                                    req->datatype,
                                                    dbop,
                                                    dmi,
                                                    ctrlr_dom,
                                                    ctrlr_id));
}

TEST_F(VbrFlowFilterEntry, ReadDetailEntry_ErrorCase) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal* ikey = NULL;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  /* char* version("version");*/
  uint8_t *ctrlr_id;
  uint8_t *ctrlr_dom;
  
  key_vbr_flowfilter_entry_t *key_vbr_ff_entry = ZALLOC_TYPE(key_vbr_flowfilter_entry_t); 

  strncpy((char*) key_vbr_ff_entry->flowfilter_key.vbr_key.vtn_key.vtn_name,"VTN1",33);
  strncpy((char*) key_vbr_ff_entry->flowfilter_key.vbr_key.vbridge_name,"VBR1",33);

  ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                       key_vbr_ff_entry, NULL);
  /*DalOdbcMgr::stub_setSingleRecordExists(true);*/
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.ReadDetailEntry(ikey,
                                                req->datatype,
                                                dbop,
                                                dmi, 
                                                ctrlr_dom,
                                                ctrlr_id));
  //DalOdbcMgr::clearStubData();
}
#endif

/**************PC************/
/*******ValidateAttribute*********/
TEST_F(VbrFlowFilterEntry, ValidateAttribute_1) {
  VbrFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());
  IPC_REQ_RESP_HEADER_DECL(req);

  req->operation = UNC_OP_CREATE;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->clnt_sess_id = 1;
  req->config_id = 0;

  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name, "VTNNAME", 32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name, "VBRNAME", 32);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);
  val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
         IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, cv);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  //unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
    //                                          unc::tclib::TC_API_COMMON_SUCCESS);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey, dmi, req));
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateAttribute_2) {
  VbrFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());

  IPC_REQ_RESP_HEADER_DECL(req);

  req->operation = UNC_OP_CREATE;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->clnt_sess_id = 1;
  req->config_id = 0;

  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name, "VTNNAME", 32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name, "VBRNAME", 32);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);
  val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
         IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, cv);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  //unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
    //                                          unc::tclib::TC_INVALID_CONFIG_ID);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
       obj.ValidateAttribute(ikey, dmi, req));
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateAttribute_3) {
  VbrFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());

  IPC_REQ_RESP_HEADER_DECL(req);

  req->operation = UNC_OP_CREATE;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->clnt_sess_id = 1;
  req->config_id = 0;

  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name, "VTNNAME", 32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name, "VBRNAME", 32);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
         IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, cv);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  //unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
    //                                          unc::tclib::TC_INVALID_SESSION_ID);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
       obj.ValidateAttribute(ikey, dmi, req));
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateAttribute_4) {
  VbrFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());

  IPC_REQ_RESP_HEADER_DECL(req);

  req->operation = UNC_OP_UPDATE;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->clnt_sess_id = 1;
  req->config_id = 0;

  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name, "VTNNAME", 32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name, "VBRNAME", 32);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);
  val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
         IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, cv);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc::tclib::TcLibModule::stub_loadtcLibModule();
//  unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
  //                                            unc::tclib::TC_API_COMMON_SUCCESS);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey, dmi, req));
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateAttribute_5) {
  VbrFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());

  IPC_REQ_RESP_HEADER_DECL(req);

  req->operation = UNC_OP_UPDATE;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->clnt_sess_id = 1;
  req->config_id = 0;

  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name, "VTNNAME", 32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name, "VBRNAME", 32);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);
  val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
         IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, cv);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  //unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
    //                                          unc::tclib::TC_INVALID_CONFIG_ID);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
       obj.ValidateAttribute(ikey, dmi, req));
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateAttribute_6) {
  VbrFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());

  IPC_REQ_RESP_HEADER_DECL(req);

  req->operation = UNC_OP_UPDATE;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->clnt_sess_id = 1;
  req->config_id = 0;

  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name, "VTNNAME", 32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name, "VBRNAME", 32);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);
  val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
         IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, cv);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  //unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
    //                                          unc::tclib::TC_INVALID_SESSION_ID);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
       obj.ValidateAttribute(ikey, dmi, req));
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

TEST_F(VbrFlowFilterEntry, ValidateAttribute_7) {
  VbrFlowFilterEntryMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());

  IPC_REQ_RESP_HEADER_DECL(req);

  req->operation = UNC_OP_UPDATE;
  req->datatype =  UPLL_DT_IMPORT;
  req->clnt_sess_id = 1;
  req->config_id = 0;

  key_vbr_flowfilter_entry_t *key_vbr = ZALLOC_TYPE(key_vbr_flowfilter_entry_t);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vtn_key.vtn_name, "VTNNAME", 32);
  strncpy((char*) key_vbr->flowfilter_key.vbr_key.vbridge_name, "VBRNAME", 32);

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
         IpctSt::kIpcStKeyVbrFlowfilterEntry, key_vbr, cv);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey, dmi, req));
}

TEST_F(VbrFlowFilterEntry, DeleteChildrenPOM_1) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *vbr_ffe_key =
      reinterpret_cast<key_vbr_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  vbr_ffe_key->flowfilter_key.direction = 0xFE;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                        vbr_ffe_key, NULL);
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
TEST_F(VbrFlowFilterEntry, DeleteChildrenPOM_2) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *vbr_ffe_key =
      reinterpret_cast<key_vbr_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  strncpy(reinterpret_cast<char*>
          (vbr_ffe_key->flowfilter_key.vbr_key.vbridge_name),
          "vbr1", 32);
  vbr_ffe_key->flowfilter_key.direction = 0xFE;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                        vbr_ffe_key, NULL);
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
TEST_F(VbrFlowFilterEntry, DeleteChildrenPOM_3) {
  VbrFlowFilterEntryMoMgr obj;
  key_vbr_flowfilter_entry_t *vbr_ffe_key =
      reinterpret_cast<key_vbr_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  strncpy(reinterpret_cast<char*>
          (vbr_ffe_key->flowfilter_key.vbr_key.vbridge_name),
          "vbr1", 32);
  vbr_ffe_key->flowfilter_key.direction = 0x0;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                        vbr_ffe_key, NULL);
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
TEST_F(VbrFlowFilterEntry, DeleteChildrenPOM_4) {
  VbrFlowFilterEntryMoMgr obj;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrFlowfilterEntry,
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
TEST_F(VbrFlowFilterEntry, DeleteChildrenPOM_5) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST_F(VbrFlowFilterEntry, DeleteChildrenPOM_6) {
  VbrFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrFlowfilterEntry,
                                        NULL, NULL);
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
