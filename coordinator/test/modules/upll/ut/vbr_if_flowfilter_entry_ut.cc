/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <vbr_if_flowfilter_entry_momgr.hh>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <ctrlr_mgr.hh>
#include <config_mgr.hh>
#include "ut_util.hh"
#include <pfcxx/synch.hh>

using namespace unc::upll::config_momgr;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::dal;
using namespace unc::capa;
using namespace unc::upll::test;
class vbr_if_entry_test
  : public UpllTestEnv
{
};

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
//Begin
TEST(VerifyRedirectDestination,ikeyNull_VRT) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi =  NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_IMPORT ));
}
TEST(VerifyRedirectDestination,ikeynot_null_VRT) {
  VbrIfFlowFilterEntryMoMgr obj;
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));
  memset(key_vrt_if, 0 ,sizeof(key_vbr_if_flowfilter_entry_t));

  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            key_vrt_if, val);
  DalDmlIntf* dmi = new DalOdbcMgr();

  // EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
    delete (dmi);
    delete (ikey);
    ikey = NULL;
}
TEST(VerifyRedirectDestination,ikeynot_null_1_VRT) {
  VbrIfFlowFilterEntryMoMgr obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));
  memset(key_vrt_if, 0 ,sizeof(key_vbr_if_flowfilter_entry_t));

  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc((sizeof(key_user_data)+ 10)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
    delete (dmi);
    delete (ikey);
    ikey = NULL ;
}
TEST(VerifyRedirectDestination,key_mergedconflict_VRT) {
  VbrIfFlowFilterEntryMoMgr obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));
  memset(key_vrt_if, 0 ,sizeof(key_vbr_if_flowfilter_entry_t));

  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);

  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_IMPORT ));
  delete dmi;
  delete ikey;

}
#if 0
TEST(VerifyRedirectDestination,key_readconfig_err) {
  VbrIfFlowFilterEntryMoMgr obj;
  controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));
  memset(key_vrt_if, 0 ,sizeof(key_vrt_if_flowfilter_t));

  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC2"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC2"), kMaxLenDomainId+1);
  ikey->get_cfg_val()->set_user_data(user_data1);

  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcNotDisconnected);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_CFG_SEMANTIC ,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
  delete dmi;
}
#endif
TEST(VerifyRedirectDestination,DIR_IN_UpdateConfigDB_GEN_ERR_VRT) {
  VbrIfFlowFilterEntryMoMgr obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));

  memset(key_vrt_if, 0 ,sizeof(key_vrt_if_flowfilter_t));
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
		        reinterpret_cast<const char *>("VTN1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vbridge_name,
		        reinterpret_cast<const char *>("VRT1"), kMaxLenCtrlrId+1);
  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
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
TEST(VerifyRedirectDestination,DIR_IN_UpdateConfigDB_INS_EXIST_VRT) {
  VbrIfFlowFilterEntryMoMgr obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));

  memset(key_vrt_if, 0 ,sizeof(key_vrt_if_flowfilter_t));
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
		        reinterpret_cast<const char *>("VTN1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vbridge_name,
		        reinterpret_cast<const char *>("VRT1"), kMaxLenCtrlrId+1);
  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
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
TEST(VerifyRedirectDestination,DIR_IN_UpdateConfigDB_NO_SUCH_INS_VRT) {
  VbrIfFlowFilterEntryMoMgr obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));

  memset(key_vrt_if, 0 ,sizeof(key_vrt_if_flowfilter_t));
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
		        reinterpret_cast<const char *>("VTN1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vbridge_name,
		        reinterpret_cast<const char *>("VRT1"), kMaxLenCtrlrId+1);
  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
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
//for ReadConfigDB Hide GetControllerDomainID and excute the next 2 TC
#if 0
TEST(VerifyRedirectDestination,DIR_OUT_ReadConDB_NO_SUCH_INS) {
  VbrIfFlowFilterEntryMoMgr obj;
  DalOdbcMgr::clearStubData();
  controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));

  memset(key_vrt_if, 0 ,sizeof(key_vrt_if_flowfilter_t));
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
		        reinterpret_cast<const char *>("VTN1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vbridge_name,
		        reinterpret_cast<const char *>("VRT1"), kMaxLenCtrlrId+1);
  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  DalOdbcMgr::stub_setSingleRecordExists(false);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_CFG_SEMANTIC,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
  delete dmi;
  delete ikey ;
}
TEST(VerifyRedirectDestination,DIR_OUT_ReadConDB_SUCH_INS_EXIST) {
  VbrIfFlowFilterEntryMoMgr obj;
  DalOdbcMgr::clearStubData();
  controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));

  memset(key_vrt_if, 0 ,sizeof(key_vrt_if_flowfilter_t));
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
		        reinterpret_cast<const char *>("VTN1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vbridge_name,
		        reinterpret_cast<const char *>("VRT1"), kMaxLenCtrlrId+1);
  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
  delete dmi;
  delete ikey ;
}
#endif 
TEST(VerifyRedirectDestination,DIR_IN_UpdateConfigDB_ACCESS_ERR_VRT) {
  VbrIfFlowFilterEntryMoMgr obj;
  //controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));

  memset(key_vrt_if, 0 ,sizeof(key_vrt_if_flowfilter_t));
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
		        reinterpret_cast<const char *>("VTN1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vbridge_name,
		        reinterpret_cast<const char *>("VRT1"), kMaxLenCtrlrId+1);
  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
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
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.VerifyRedirectDestination(ikey, dmi  ,UPLL_DT_CANDIDATE ));
  delete dmi;
  delete ikey ;
}
#if 0
TEST(VerifyRedirectDestination,MergeValidate) {
  VbrIfFlowFilterEntryMoMgr obj;
  controller_domain_t ctrlr_dom[2] = {{NULL,NULL},{NULL,NULL}};
  char *ctrl_id =  NULL;
   ctrl_id = (char*)malloc(sizeof(30));
  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));
  memset(key_vrt_if, 0 ,sizeof(key_vrt_if_flowfilter_t));

  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbrifflowfilter_entry_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            key_vrt_if, val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc((sizeof(key_user_data)+ 10)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcRecordNoMore);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  unc_key_type_t keytype = UNC_KT_VBRIF_FLOWFILTER_ENTRY ;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SEMANTIC,obj.MergeValidate( keytype, (const char*)ctrl_id, ikey , dmi));
    delete (dmi);
    delete (ikey);
    ikey = NULL ;
}
#endif
TEST(ConstructReadDetailResponse , Construct2_KEY_NULL) {
  VbrIfFlowFilterEntryMoMgr obj;
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

  val_flowfilter_entry_t *l_flowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(l_flowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  ConfigVal *val_1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             l_flowfilter_entry_val);

    val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrifflowfilter_entry_val);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);

  drv_ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            NULL, val);
  drv_ikey->AppendCfgVal(val_1);   
  DalDmlIntf* dmi = new DalOdbcMgr();
  DalOdbcMgr::clearStubData();
 // DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcNotDisconnected);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS, kDalRcSuccess);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ConstructReadDetailResponse(NULL,drv_ikey,l_ctrlr_dom ,&okey  ,dmi ));
  delete dmi;
  delete(drv_ikey);
  free(l_ctrlr_dom.ctrlr);
  free(l_ctrlr_dom.domain);
}

TEST(ConstructReadDetailResponse , Construct2_val_VRT) {
  VbrIfFlowFilterEntryMoMgr obj;
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

  key_vbr_if_flowfilter_entry_t *key_vrt_if = reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
                   (malloc(sizeof(key_vbr_if_flowfilter_entry_t)));
  val_flowfilter_entry_t *l_flowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(l_flowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  ConfigVal *val_1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             l_flowfilter_entry_val);

  memset(key_vrt_if, 0 ,sizeof(key_vrt_if_flowfilter_t));
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
		        reinterpret_cast<const char *>("VTN1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vbr_key.vbridge_name,
		        reinterpret_cast<const char *>("VRT1"), kMaxLenCtrlrId+1);
  val_flowfilter_entry_t *vbrifflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
         (malloc(sizeof(val_flowfilter_entry_t)));
  memset(vbrifflowfilter_entry_val, 0, sizeof(val_flowfilter_entry_t));
  vbrifflowfilter_entry_val->valid[2] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->valid[3] = UNC_VF_VALID;
  vbrifflowfilter_entry_val->redirect_direction = UPLL_FLOWFILTER_DIR_IN;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             vbrifflowfilter_entry_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            key_vrt_if, NULL);
  key_user_data *user_data1 = static_cast<key_user_data *>(malloc(sizeof(key_user_data)));
  uuu::upll_strncpy(user_data1->ctrlr_id, reinterpret_cast<const char *>("PFC_UNC1"), kMaxLenCtrlrId+1);
  uuu::upll_strncpy(user_data1->domain_id, reinterpret_cast<const char *>("DOMAIN_UNC1"), kMaxLenDomainId+1);
  ikey->set_user_data(user_data1);

  drv_ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
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
//End U13
TEST_F(vbr_if_entry_test,AllocVal_outputNull) {
  VbrIfFlowFilterEntryMoMgr obj;
  val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_entry_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(val, UPLL_DT_IMPORT,MAINTBL));
  delete val;
}
TEST_F(vbr_if_entry_test,AllocVal_Success) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigVal *val = NULL; 
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.AllocVal(val, UPLL_DT_IMPORT,MAINTBL));
}

TEST_F(vbr_if_entry_test,AllocVal_Error) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigVal *val = NULL; 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.AllocVal(val, UPLL_DT_STATE,RENAMETBL));           
}
TEST_F(vbr_if_entry_test,GetChildConfigKey) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey)); 
}

TEST_F(vbr_if_entry_test,GetChildConfigKey_pkeyNull) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL; 
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));   
}

TEST_F(vbr_if_entry_test,GetChildConfigKey_pkeyNull1) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL, NULL);
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetChildConfigKey(okey, pkey));    
}


TEST_F(vbr_if_entry_test,GetChildConfigKey_pkeyNull2) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);

  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"INTERFACENAME",32);
  
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            key_vbr_if_entry, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetChildConfigKey(okey, pkey));
  key_vbr_if_flowfilter_entry_t *output = reinterpret_cast<key_vbr_if_flowfilter_entry_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name)));
  EXPECT_STREQ("VBR1",(reinterpret_cast<const char *> (output->flowfilter_key.if_key.vbr_key.vbridge_name)));
  EXPECT_STREQ("INTERFACENAME",(reinterpret_cast<const char *> (output->flowfilter_key.if_key.if_name)));
}

TEST_F(vbr_if_entry_test, DupConfigKeyVal_reqnull) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL, NULL);
  ConfigKeyVal *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey, req,MAINTBL));
}

TEST_F(vbr_if_entry_test, DupConfigKeyVal_okeynull) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL, NULL);

  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
}

TEST_F(vbr_if_entry_test, DupConfigKeyVal_Req_Invalid) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;

  val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  vbr_if_flowfilter_entry_val->cs_row_status = 1;

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_entry_val); 
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            NULL, tmp);
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DupConfigKeyVal(okey,req,MAINTBL));
}

TEST_F(vbr_if_entry_test, DupConfigKeyVal_Req_Valid) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;

  val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
  vbr_if_flowfilter_entry_val->cs_row_status = 1;

  key_vbr_if_flowfilter_entry_t *key_vbr_if_ffe = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_entry_val);

  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            key_vbr_if_ffe, tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS,obj.DupConfigKeyVal(okey,req,MAINTBL));
}

TEST_F(vbr_if_entry_test, CopyToConfigkeyVal_Ikey_NUll) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
  //delete ikey;
}

TEST_F(vbr_if_entry_test, CopyToConfigkeyVal_InValid) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;

  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            key_rename, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));

  strncpy((char*) key_rename->old_unc_vtn_name,"OLDVTNNAME",32); 
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.CopyToConfigKey(okey,ikey));
}
#if 0
TEST_F(vbr_if_entry_test, CopyToConfigkeyVal_Valid) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *key = NULL;
  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            key_rename, NULL);

  strncpy((char*) key_rename->old_unc_vtn_name,"OLDVTNNAME",32);
  strncpy((char*) key_rename->old_unc_vnode_name,"OLDVBRIDGEE",32); 
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.CopyToConfigKey(okey,ikey));

  key_vbr_if_flowfilter_entry_t *output = reinterpret_cast<key_vbr_if_flowfilter_entry_t *> (okey->get_key());
  EXPECT_STREQ("OLDVTNNAME",(reinterpret_cast<const char *> (output->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name)));
  EXPECT_STREQ("OLDVBRIDGEE",(reinterpret_cast<const char *> (output->flowfilter_key.if_key.vbr_key.vbridge_name)));

}

TEST_F(vbr_if_entry_test, CopyToConfigkeyVal_Flowlist) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *key = NULL;
  key_rename_vnode_info *key_rename = ZALLOC_TYPE(key_rename_vnode_info);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                            IpctSt::kIpcStKeyFlowlist,
                            key_rename, NULL);
  strncpy((char*) key_rename->old_flowlist_name,"OLDFLOWLISTNAME",32);
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.CopyToConfigKey(okey,ikey));
  val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (okey->get_cfg_val()->get_val());
  EXPECT_STREQ("OLDFLOWLISTNAME",(reinterpret_cast<const char *> (output->flowlist_name)));
}
#endif
/*
TEST_F(vbr_if_entry_test, RenameMo_NoOperation) {
  VbrIfFlowFilterEntryMoMgr obj;
   IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  const char* ctrlr_id = {0};
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.RenameMo(req,ikey,dmi,ctrlr_id));
}*/

/*
TEST_F(vbr_if_entry_test, UpdateMo_NoOperation) {
  VbrIfEntryFlowFilterMoMgr obj;
   IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.UpdateMo(req,ikey,dmi));
}*/

TEST_F(vbr_if_entry_test, MerGeValidate_Success) {
  VbrIfFlowFilterEntryMoMgr obj;
   //IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  const char* ctrlr_id = {0};
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.MergeValidate(UNC_KT_VBRIF_FLOWFILTER_ENTRY,ctrlr_id,ikey,dmi, (upll_import_type)0));
} 

TEST_F(vbr_if_entry_test, Update_Audit) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey, dmi));

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);

  ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL,tmp);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpCreate, ikey, dmi));
  val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  EXPECT_EQ(UNC_CS_APPLIED,output->cs_row_status);
}

TEST_F(vbr_if_entry_test, Update_Audit_Update) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;

  val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

  val->valid[0] = UNC_VF_VALID;
  val->valid[1] = UNC_VF_INVALID;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val);
  
  ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL,tmp);

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.UpdateAuditConfigStatus(UNC_CS_APPLIED,uuc::kUpllUcpUpdate, ikey, dmi));
  val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  EXPECT_EQ(UNC_CS_APPLIED,output->cs_attr[0]);
}


// Test Case core Dump
TEST_F(vbr_if_entry_test, ValidateMessage) {
  VbrIfFlowFilterEntryMoMgr obj;
   IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req,ikey));
  delete req;
}

TEST_F(vbr_if_entry_test, ValidateMessage_Invalid_KeyType) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            NULL, NULL);
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req,ikey));
 delete req;
 delete ikey;
}

TEST_F(vbr_if_entry_test, ValidateMessage_Valid_KeyType) {
 VbrIfFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req= NULL;
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY);
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req,ikey));
 delete ikey;
}

TEST_F(vbr_if_entry_test, ValidateMessage_InValidKey) {
 VbrIfFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req= NULL; 
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry);
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req,ikey));
 delete ikey;
}

TEST_F(vbr_if_entry_test, ValidateMessage_InValidMoManagerInstance) {
 VbrIfFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req= NULL;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req,ikey));
 delete ikey;
}

TEST_F(vbr_if_entry_test, ValidateMessage_Success) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_NONE;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req,ikey));
 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, ValidateAttribute_keyNull) {
 VbrIfFlowFilterEntryMoMgr obj;
 ConfigKeyVal *ikey = NULL;
 DalDmlIntf *dmi =  NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey, dmi));
}

TEST_F(vbr_if_entry_test, ValidateAttribute) {
 VbrIfFlowFilterEntryMoMgr obj;
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);
 DalDmlIntf *dmi = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateAttribute(ikey, dmi));
 delete ikey;
}

TEST_F(vbr_if_entry_test, ValidateAttribute_Success) {
 VbrIfFlowFilterEntryMoMgr obj;
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry);
 //ConfigVal *ival = ZALLOC_TYPE(ConfigVal);
 void *key = ZALLOC_TYPE(IpctSt);
 ikey->SetKey(IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key);
 
 void *val= ZALLOC_TYPE(IpctSt);
 ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStKeyVbrIfFlowfilterEntry,val));

 DalDmlIntf *dmi(getDalDmlIntf());
 IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_NONE;
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey, dmi, req));
 delete ikey;
}

TEST_F(vbr_if_entry_test, GetRenameBindInfo_Output_Null) {
 VbrIfFlowFilterEntryMoMgr obj;
 BindInfo *info = NULL;
 int nattr = 0;
 EXPECT_EQ(PFC_TRUE,obj.GetRenameKeyBindInfo(UNC_KT_VBRIF_FLOWFILTER_ENTRY, info, nattr, MAINTBL));
 EXPECT_EQ(6,nattr);
}

TEST_F(vbr_if_entry_test, GetRenameBindInfo_InvalidTable) {
 VbrIfFlowFilterEntryMoMgr obj;
 BindInfo *info = NULL;
 int nattr = 0;
 EXPECT_EQ(PFC_FALSE,obj.GetRenameKeyBindInfo(UNC_KT_VBRIF_FLOWFILTER_ENTRY, info, nattr,RENAMETBL));
}

TEST_F(vbr_if_entry_test, GetRenameBindInfo_FlowlistTbl) {
 VbrIfFlowFilterEntryMoMgr obj;
 BindInfo *info = NULL;
 int nattr = 0;
 EXPECT_EQ(PFC_TRUE,obj.GetRenameKeyBindInfo(UNC_KT_FLOWLIST, info, nattr, MAINTBL));
 EXPECT_EQ(6,nattr);
}

TEST_F(vbr_if_entry_test, UpdateConfigStatus_ValStructureNull) {
 VbrIfFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, NULL);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 0, upd_key, dmi, ctrlr_key));
 delete key;
}

TEST_F(vbr_if_entry_test, UpdateConfigStatus_ValStructure_Valid) {
 VbrIfFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 vbr_if_flowfilter_entry_val->valid[2] = UNC_VF_VALID;

 ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_entry_val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, val);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_CREATE, 1, upd_key, dmi, ctrlr_key));
 val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (key->get_cfg_val()->get_val()); 
 EXPECT_EQ(UNC_CS_NOT_APPLIED,output->cs_attr[2]);
 delete key;
}
TEST_F(vbr_if_entry_test, UpdateConfigStatus_ValStructure_Valid_Update) {
 VbrIfFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 DalOdbcMgr::clearStubData();
 val_flowfilter_entry_t *ffe_val = ZALLOC_TYPE(val_flowfilter_entry_t);

 ffe_val->cs_row_status = UNC_CS_APPLIED;
 for (unsigned int loop = 0;
          loop < sizeof(ffe_val->valid) / sizeof(ffe_val->valid[0]); ++loop) {
   ffe_val->valid[loop] = UNC_VF_VALID;
 }

 ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                             ffe_val);
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL, val);
     void *ival= ZALLOC_TYPE(IpctSt);
 //  key->SetCfgVal(new ConfigVal(IpctSt::kIpcStKeyVbrIfFlowfilterEntry,ival));
 key->SetKey(IpctSt::kIpcStKeyVbrIfFlowfilterEntry,ival);

 val_flowfilter_entry_t *ffe_updval = ZALLOC_TYPE(val_flowfilter_entry_t);
 ConfigVal *val1 = new ConfigVal(IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                                ffe_updval);
 upd_key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                            NULL, val1);
 DalDmlIntf *dmi(getDalDmlIntf());
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.UpdateConfigStatus(key, UNC_OP_UPDATE, 0, upd_key, dmi, ctrlr_key));
 val_flowfilter_entry_t *output = reinterpret_cast<val_flowfilter_entry_t *> (key->get_cfg_val()->get_val());
 EXPECT_EQ(UNC_CS_APPLIED,output->cs_attr[2]);
 DELETE_IF_NOT_NULL(key);
 DELETE_IF_NOT_NULL(upd_key);
 DalOdbcMgr::clearStubData();
}

TEST_F(vbr_if_entry_test, UpdateConfigStatus_Delete_Operation) {
 VbrIfFlowFilterEntryMoMgr obj;
 ConfigKeyVal *upd_key = NULL;
 ConfigKeyVal *ctrlr_key= NULL;

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 vbr_if_flowfilter_entry_val->valid[2] = UNC_VF_VALID;

 ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_entry_val);

 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            NULL, val);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateConfigStatus(key, UNC_OP_DELETE, 1, upd_key, dmi, ctrlr_key));
 delete key;
}
TEST_F(vbr_if_entry_test, IsValid_keyNull ) {
 VbrIfFlowFilterEntryMoMgr obj;
 EXPECT_EQ(true, obj.IsValidKey(NULL, uudst::vbr_if_flowfilter_entry::kDbiVtnName));
}

TEST_F(vbr_if_entry_test, IsValid_InvalidVtnName ) {
 VbrIfFlowFilterEntryMoMgr obj;
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);

 EXPECT_EQ(false,obj.IsValidKey(key, uudst::vbr_if_flowfilter_entry::kDbiVtnName));
 delete key;
}

TEST_F(vbr_if_entry_test, IsValid_InvalidVbrName ) {
 VbrIfFlowFilterEntryMoMgr obj;
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);

 EXPECT_EQ(false,obj.IsValidKey(key, uudst::vbr_if_flowfilter_entry::kDbiVbrName));
 delete key;
}

TEST_F(vbr_if_entry_test, IsValid_InvalidIfName ) {
 VbrIfFlowFilterEntryMoMgr obj;
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);

 EXPECT_EQ(false,obj.IsValidKey(key, uudst::vbr_if_flowfilter_entry::kDbiVbrIfName));
 delete key;
}

TEST_F(vbr_if_entry_test, IsValid_InvalidSequenceNum) {
 VbrIfFlowFilterEntryMoMgr obj;
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 ConfigKeyVal *key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);
 key_vbr_if->sequence_num = 0;
 EXPECT_EQ(false,obj.IsValidKey(key_vbr_if, uudst::vbr_if_flowfilter_entry::kDbiSequenceNum));
 delete key;
}

TEST_F(vbr_if_entry_test, ValidateCapability_InvalidKey) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 ConfigKeyVal *key = NULL;
 const char * ctrlr_name = "NULL";
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.ValidateCapability(req,key,ctrlr_name));
 delete req;
}

TEST_F(vbr_if_entry_test, ValidateCapability) {
 VbrIfFlowFilterEntryMoMgr obj1;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_STATE;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;
 
 const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version, true);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.clear();
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);

 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);
 ikey->set_user_data((void *)ctrlr_name);
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,obj1.ValidateCapability(req,ikey,ctrlr_name));

 delete req;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 //CapaModuleStub::stub_clearStubData();
}

/*
TEST_F(vbr_if_entry_test, ReadMo_NoInstance) {
 VbrIfFlowFilterEntryMoMgr obj;
 IpcReqRespHeader* req = new IpcReqRespHeader;
 req->datatype = UPLL_DT_STATE;
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE,obj.ReadMo(req,ikey,dmi));
 delete req;
 delete ikey;
}*/

TEST_F(vbr_if_entry_test, GetValid_ValNull) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t * val = NULL;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetValid(val,0, valid, UPLL_DT_STATE, MAINTBL));
}

TEST_F(vbr_if_entry_test, GetValid_Action) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);
 
 val->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetValid((void *) val,uudst::vbr_if_flowfilter_entry::kDbiAction, valid, UPLL_DT_STATE, MAINTBL));
 EXPECT_EQ(UNC_VF_VALID,*valid);
}

TEST_F(vbr_if_entry_test, GetValid_Flowlist) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetValid((void *) val,uudst::vbr_if_flowfilter_entry::kDbiFlowlistName, valid, UPLL_DT_STATE, MAINTBL));
 EXPECT_EQ(UNC_VF_VALID,*valid);
}

TEST_F(vbr_if_entry_test, GetValid_RedirectNode) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_INVALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetValid((void *) val,uudst::vbr_if_flowfilter_entry::kDbiRedirectNode, valid, UPLL_DT_STATE, MAINTBL));
 EXPECT_EQ(UNC_VF_INVALID,*valid);
}

TEST_F(vbr_if_entry_test, GetValid_RedirectPort) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_VALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetValid((void *) val,uudst::vbr_if_flowfilter_entry::kDbiRedirectPort, valid, UPLL_DT_STATE, MAINTBL));
 EXPECT_EQ(UNC_VF_VALID,*valid);
}

TEST_F(vbr_if_entry_test, GetValid_ModifyDstmac) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = UNC_VF_INVALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetValid((void *) val,uudst::vbr_if_flowfilter_entry::kDbiModifyDstMac, valid, UPLL_DT_STATE, MAINTBL));
 EXPECT_EQ(UNC_VF_INVALID,*valid);
}

TEST_F(vbr_if_entry_test, GetValid_ModifySrcmac) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = UNC_VF_INVALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetValid((void *) val,uudst::vbr_if_flowfilter_entry::kDbiModifySrcMac, valid, UPLL_DT_STATE, MAINTBL));
 EXPECT_EQ(UNC_VF_INVALID,*valid);
}

TEST_F(vbr_if_entry_test, GetValid_NwmName) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);
 
 val->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_INVALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetValid((void *) val,uudst::vbr_if_flowfilter_entry::kDbiNwmName, valid, UPLL_DT_STATE, MAINTBL));
 EXPECT_EQ(UNC_VF_INVALID,*valid);
}

TEST_F(vbr_if_entry_test, GetValid_Dscp) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);
 
 val->valid[UPLL_IDX_DSCP_FFE] = UNC_VF_INVALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetValid((void *) val,uudst::vbr_if_flowfilter_entry::kDbiDscp, valid, UPLL_DT_STATE, MAINTBL));
 EXPECT_EQ(UNC_VF_INVALID,*valid);
}

TEST_F(vbr_if_entry_test, GetValid_Priority) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->valid[UPLL_IDX_PRIORITY_FFE] = UNC_VF_VALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetValid((void *) val,uudst::vbr_if_flowfilter_entry::kDbiPriority, valid, UPLL_DT_STATE, MAINTBL));
 EXPECT_EQ(UNC_VF_VALID,*valid);
}

TEST_F(vbr_if_entry_test, GetValid_InvalidIndex) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);
 
 val->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_INVALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetValid((void *) val,33, valid, UPLL_DT_STATE, MAINTBL));
}

TEST_F(vbr_if_entry_test, GetValid_InValidTable) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val = ZALLOC_TYPE(val_flowfilter_entry_t);

 val->valid[0] = UNC_VF_VALID;
 uint8_t *valid = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetValid(val,0, valid, UPLL_DT_STATE, CTRLRTBL));

}

TEST_F(vbr_if_entry_test, CompareValidValue_AuditTrue) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_flowfilter_entry_t);
 val_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_flowfilter_entry_t);

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

TEST_F(vbr_if_entry_test, CompareValidValue_ValidFlowlist) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_flowfilter_entry_t);
 val_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
 strncpy((char*) val_ffe1->flowlist_name,"FlowlistName", 32);
 strncpy((char*) val_ffe2->flowlist_name,"FlowlistName", 32);
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false);
 
 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_FLOWLIST_NAME_FFE]);
 free(val_ffe1);
 free(val_ffe2);
}

TEST_F(vbr_if_entry_test, CompareValidValue_ValidAction) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_flowfilter_entry_t);
 val_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;

 val_ffe1->action = UPLL_IDX_ACTION_FFE;
 val_ffe2->action = UPLL_IDX_ACTION_FFE;
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false);
 
 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_ACTION_FFE]);

 free (val_ffe1);
 free (val_ffe2);
}

TEST_F(vbr_if_entry_test, CompareValidValue_ValidRedirectNode) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_flowfilter_entry_t);
 val_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID;
 strncpy((char*) val_ffe1->redirect_node,"ReDirectNode", 32);
 strncpy((char*) val_ffe2->redirect_node,"ReDirectNode", 32);
 void *ffeval1 = reinterpret_cast<void *>(val_ffe1);
 void *ffeval2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(ffeval1,ffeval2,true);
 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_REDIRECT_NODE_FFE]);
 free(val_ffe1);
 free(val_ffe2);
}

TEST_F(vbr_if_entry_test, CompareValidValue_ValidRedirectPort) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_flowfilter_entry_t);
 val_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_VALID;
 strncpy((char*) val_ffe1->redirect_port,"ReDirectport", 32);
 strncpy((char*) val_ffe2->redirect_port,"ReDirectport", 32);
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, true);

 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_REDIRECT_PORT_FFE]);
 free(val_ffe1);
 free(val_ffe2);
}

TEST_F(vbr_if_entry_test, CompareValidValue_ValidDstMac) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_flowfilter_entry_t);
 val_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = UNC_VF_VALID;
 strncpy((char*) val_ffe1->modify_dstmac,"MAC", 6);
 strncpy((char*) val_ffe2->modify_dstmac,"MAC", 6);
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false);

 EXPECT_EQ(UNC_VF_VALUE_NOT_MODIFIED,val_ffe1->valid[UPLL_IDX_MODIFY_DST_MAC_FFE]);
 free(val_ffe1);
 free(val_ffe2);
}

TEST_F(vbr_if_entry_test, CompareValidValue_ValidSrcMac) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_flowfilter_entry_t);
 val_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = UNC_VF_VALID;
 strncpy((char*) val_ffe1->modify_srcmac,"MAC", 6);
 strncpy((char*) val_ffe2->modify_srcmac,"MAC", 6);
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false);
 
 EXPECT_EQ(UNC_VF_VALUE_NOT_MODIFIED,val_ffe1->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE]);
 free(val_ffe1);
 free(val_ffe2);
}

TEST_F(vbr_if_entry_test, CompareValidValue_ValidNwmName) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_flowfilter_entry_t);
 val_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_VALID;
 strncpy((char*) val_ffe1->nwm_name,"NwmName", 32);
 strncpy((char*) val_ffe2->nwm_name,"NwmName", 32);
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false);

 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_NWM_NAME_FFE]);
 free(val_ffe1);
 free(val_ffe2);
}

TEST_F(vbr_if_entry_test, CompareValidValue_ValidDscp) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_flowfilter_entry_t);
 val_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_DSCP_FFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_DSCP_FFE] = UNC_VF_VALID;

 val_ffe1->dscp = UPLL_IDX_NWM_NAME_FFE;
 val_ffe2->dscp = UPLL_IDX_NWM_NAME_FFE;
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, false);
 
 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_DSCP_FFE]);

 free (val_ffe1);
 free (val_ffe2);
}

TEST_F(vbr_if_entry_test, CompareValidValue_ValidPriority) {
 VbrIfFlowFilterEntryMoMgr obj;
 val_flowfilter_entry_t *val_ffe1 = ZALLOC_TYPE(val_flowfilter_entry_t);
 val_flowfilter_entry_t *val_ffe2 = ZALLOC_TYPE(val_flowfilter_entry_t);

 val_ffe1->valid[UPLL_IDX_PRIORITY_FFE] = UNC_VF_VALID;
 val_ffe2->valid[UPLL_IDX_PRIORITY_FFE] = UNC_VF_VALID;

 val_ffe1->priority = UPLL_IDX_PRIORITY_FFE;
 val_ffe2->priority = UPLL_IDX_PRIORITY_FFE;
 void *v1 = reinterpret_cast<void *>(val_ffe1);
 void *v2 = reinterpret_cast<void *>(val_ffe2);

 obj.CompareValidValue(v1, v2, true);

 EXPECT_EQ(UNC_VF_INVALID,val_ffe1->valid[UPLL_IDX_PRIORITY_FFE]);

 free (val_ffe1);
 free (val_ffe2);
}

TEST_F(vbr_if_entry_test, DeleteMo) {
 VbrIfFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req= NULL;
 ConfigKeyVal *ikey = NULL;
 DalDmlIntf* dmi = NULL;
 
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.DeleteMo(req,ikey,dmi));
} 

TEST_F(vbr_if_entry_test, DeleteMo1) {
 VbrIfFlowFilterEntryMoMgr obj;
 DalDmlIntf* dmi = NULL;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);
 EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2,obj.DeleteMo(req,ikey,dmi));
}

TEST_F(vbr_if_entry_test, GetRenamedUncKey_CtlrInValid) {
 VbrIfFlowFilterEntryMoMgr obj;
 uint8_t * ctrlr_id = NULL;
 DalDmlIntf *dmi = NULL;

 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedUncKey(ikey, UPLL_DT_STATE,dmi,ctrlr_id));

 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, GetRenamedUncKey_Valid) {
 VbrIfFlowFilterEntryMoMgr obj;
 uint8_t ctrlr_id[4] = {'p', 'f', 'c'};
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"OLDVTNNAME",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vbridge_name,"OLDVBRNAME",32);

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_if_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

  ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                vbr_if_flowfilter_entry_val);
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,val);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetRenamedUncKey(ikey, UPLL_DT_STATE,dmi,ctrlr_id));

 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, GetRenamedControllerKey_VtnInvalidInstance) {
 VbrIfFlowFilterEntryMoMgr obj;
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);

 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.if_name,"IFNAME1",32);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

 controller_domain *ctrl_domain = new controller_domain();
 DalDmlIntf *dmi(getDalDmlIntf());
 SET_USER_DATA_FLAGS_TEST(ikey, 0x01);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_STATE,dmi,ctrl_domain));

 delete ikey;
}


TEST_F(vbr_if_entry_test, GetRenamedControllerKey_ValidNotSet) {
 VbrIfFlowFilterEntryMoMgr obj;
 controller_domain *ctrl_domain = new controller_domain();
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);

 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.if_name,"IFNAME1",32);
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS_TEST(ikey, 0x01);
 EXPECT_EQ(UPLL_RC_SUCCESS,obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));

 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 delete ikey;
}

TEST_F(vbr_if_entry_test, GetRenamedControllerKey_VbrInvalidInstance) {
 VbrIfFlowFilterEntryMoMgr obj;
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);

 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.if_name,"IFNAME1",32);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

 controller_domain *ctrl_domain = new controller_domain();
 DalDmlIntf *dmi(getDalDmlIntf());
 SET_USER_DATA_FLAGS_TEST(ikey, 0x10);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_STATE,dmi,ctrl_domain));

 delete ikey;
}

TEST_F(vbr_if_entry_test, GetRenamedControllerKey_SUCCESS) {
 VbrIfFlowFilterEntryMoMgr obj;
 controller_domain *ctrl_domain = new controller_domain();
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);

 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vbridge_name,"VB R1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.if_name,"IFNAME1",32);
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS_TEST(ikey, 0x10);
 EXPECT_EQ(UPLL_RC_SUCCESS , obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));

 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 delete ikey;
}
/*
TEST_F(vbr_if_entry_test, GetRenamedControllerKey_FlowlistInvalidInstance) {
 VbrIfFlowFilterEntryMoMgr obj;
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);

 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.if_name,"IFNAME1",32);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

 controller_domain *ctrl_domain = new controller_domain();
 DalDmlIntf *dmi(getDalDmlIntf());
 SET_USER_DATA_FLAGS_TEST(ikey, 0x04);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_STATE,dmi,ctrl_domain));

 delete ikey;

}

TEST_F(vbr_if_entry_test, GetRenamedControllerKey_FlowlistValidNotSet) {
 VbrIfFlowFilterEntryMoMgr obj;
 controller_domain *ctrl_domain = new controller_domain();
 key_vbr_if_flowfilter_entry_t *key_vbr_if = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);

 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if->flowfilter_key.if_key.if_name,"IFNAME1",32);

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_if_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                vbr_if_flowfilter_entry_val);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if,val);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());

 SET_USER_DATA_FLAGS_TEST(ikey, 0x11);
 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.GetRenamedControllerKey(ikey,UPLL_DT_CANDIDATE,dmi,ctrl_domain));

 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
 delete ikey;
}
*/

TEST_F(vbr_if_entry_test, ReadMo_NoInstance) {
 VbrIfFlowFilterEntryMoMgr obj;
 IpcReqRespHeader* req = NULL;
 ConfigKeyVal *ikey = NULL;

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.ReadMo(req,ikey,dmi));
}

TEST_F(vbr_if_entry_test, ReadMo_Instance) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UNC_UPLL_RC_ERR_INVALID_OPTION2,obj.ReadMo(req,ikey,dmi));

 delete req;
 delete ikey;
}

TEST_F(vbr_if_entry_test, ReadMo_Instance1) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;
 
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UNC_UPLL_RC_ERR_INVALID_OPTION2,obj.ReadMo(req,ikey,dmi));

 delete req;
 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, ReadSiblingMo_NoInstance) {
 VbrIfFlowFilterEntryMoMgr obj;
 IpcReqRespHeader* req = NULL;
 ConfigKeyVal *ikey = NULL;

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,obj.ReadSiblingMo(req,ikey,true,dmi));
}

TEST_F(vbr_if_entry_test, ReadSiblingMo_Instance) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ReadSiblingMo(req,ikey,true,dmi));

 delete req;
 delete ikey;
}

TEST_F(vbr_if_entry_test, ReadSiblingMo_Instance1) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UNC_UPLL_RC_ERR_INVALID_OPTION2,obj.ReadSiblingMo(req,ikey,true,dmi));

 delete req;
 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, ReadiSiblingMo_Instance2) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option1 = UNC_OPT1_DETAIL;
 req->option2 = UNC_OPT2_NONE;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;

 DalOdbcMgr::clearStubData();
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);
 SET_USER_DATA_FLAGS_TEST(ikey,0x04);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,obj.ReadSiblingMo(req,ikey,true,dmi));

 delete req;
 delete ikey;
 DalOdbcMgr::clearStubData();
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, MergeValidate_Read_Cfg_Err) {
  VbrIfFlowFilterEntryMoMgr obj;
  key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vbr_if_entry->sequence_num = 1;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);
  char ctrl_id[]={'p', 'f', 'c'};
  unc_key_type_t keytype = UNC_KT_VBRIF_FLOWFILTER_ENTRY;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.MergeValidate(keytype,ctrl_id,ikey,dmi, (upll_import_type)0));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, MergeValidate_Success) {
  VbrIfFlowFilterEntryMoMgr obj;
  key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vbr_if_entry->sequence_num = 1;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);
  //ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER);
  char ctrl_id[]={'p', 'f', 'c'};
  unc_key_type_t keytype = UNC_KT_VBRIF_FLOWFILTER_ENTRY;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT, kDalRcRecordNoMore);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS,obj.MergeValidate(keytype,ctrl_id,ikey,dmi, (upll_import_type)0));
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, MergeValidate_Update_DB_Err) {
  VbrIfFlowFilterEntryMoMgr obj;
  key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
  val_flowfilter_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_t);
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                             vbr_if_flowfilter_entry_val);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vbr_if_entry->sequence_num = 1;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,tmp);
  char ctrl_id[]={'p', 'f', 'c'};
  unc_key_type_t keytype = UNC_KT_VBRIF_FLOWFILTER_ENTRY;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.MergeValidate(keytype,ctrl_id,ikey,dmi, (upll_import_type)0));
  UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, DeleteMo_ReqNull) {
 VbrIfFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = NULL;
 ConfigKeyVal *key = NULL;
 DalDmlIntf *dmi = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteMo(req,key,dmi));
}

TEST_F(vbr_if_entry_test, DeleteMo_KeyNull) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 ConfigKeyVal *key = NULL;
 DalDmlIntf *dmi = NULL;
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.DeleteMo(req,key,dmi));
}

TEST_F(vbr_if_entry_test, Valid_data) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_if_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                vbr_if_flowfilter_entry_val);
 
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,val);
 const char ctrlr_name[] = "ctrlr_name";
 ikey->set_user_data((void*)ctrlr_name);
 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UNC_UPLL_RC_ERR_INVALID_OPTION2,obj.DeleteMo(req,ikey,dmi));

// delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, DeleteMo_Error) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_if_flowfilter_entry_val->flowlist_name,"FLOW",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                vbr_if_flowfilter_entry_val);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
// DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD,kDalRcSuccess);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,val);
 const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version, true);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.clear();
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);

 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 ikey->set_user_data((void*)ctrlr_name);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UNC_UPLL_RC_ERR_INVALID_OPTION2, obj.DeleteMo(req,ikey,dmi));

 //delete ikey;
 //CapaModuleStub::stub_clearStubData();
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}


TEST_F(vbr_if_entry_test, UpdateMo_ReqNull) {
 VbrIfFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = NULL;
 ConfigKeyVal *key = NULL;
 DalDmlIntf *dmi = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateMo(req,key,dmi));
}

TEST_F(vbr_if_entry_test, UpdateMo_KeyNull) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 ConfigKeyVal *key = NULL;
 DalDmlIntf *dmi = NULL;
 EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.UpdateMo(req,key,dmi));
}

TEST_F(vbr_if_entry_test, UpdateMo_Valid_datai1) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_if_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                vbr_if_flowfilter_entry_val);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);

 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,val);
 const char ctrlr_name[] = "ctrlr_name";
 ikey->set_user_data((void*)ctrlr_name);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UNC_UPLL_RC_ERR_INVALID_OPTION2, obj.UpdateMo(req,ikey,dmi));
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, UpdateMo_Valid_data2) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_DELETE;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_if_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                vbr_if_flowfilter_entry_val);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,val);
 const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version, true);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.clear();
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);

 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 ikey->set_user_data((void*)ctrlr_name);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UNC_UPLL_RC_ERR_INVALID_OPTION2,obj.UpdateMo(req,ikey,dmi));
 //CapaModuleStub::stub_clearStubData();
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

/*TEST_F(vbr_if_entry_test, UpdateMo_Success) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_DELETE;
 req->datatype = UPLL_DT_CANDIDATE;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_if_flowfilter_entry_t *key = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_if_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                vbr_if_flowfilter_entry_val);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,val);
 const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.clear();
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);

 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
 CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY, true);

 ikey->set_user_data((void*)ctrlr_name);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_GENERIC,obj.UpdateMo(req,ikey,dmi));
 CapaModuleStub::stub_clearStubData();
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}*/

TEST_F(vbr_if_entry_test, CreateCandidateMo_ReqNull) {
 VbrIfFlowFilterEntryMoMgr obj;
 IpcReqRespHeader *req = NULL;
 ConfigKeyVal *key = NULL;
 DalDmlIntf *dmi = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req,key,dmi));
}

TEST_F(vbr_if_entry_test, CreateCandidateMo_KeyNull) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 ConfigKeyVal *key = NULL;
 DalDmlIntf *dmi = NULL;
 EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CreateCandidateMo(req,key,dmi));
}

TEST_F(vbr_if_entry_test, CreateCandidateMo_Valid_data1) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_CREATE;
 req->datatype = UPLL_DT_CANDIDATE;
 req->option1 = UNC_OPT1_NORMAL;
 req->option2 = UNC_OPT2_NONE;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",31);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",31);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",31);
 key_vbr_if_entry->sequence_num = kMinFlowFilterSeqNum;

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_if_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",31);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                vbr_if_flowfilter_entry_val);

 DalOdbcMgr::clearStubData();
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcRecordNotFound);

 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,val);
 const char ctrlr_name[] = "ctrlr_name";
 void *ival = ZALLOC_TYPE(IpctSt);
 ikey->set_user_data((void*)ctrlr_name);
 ikey->SetKey(IpctSt::kIpcStKeyVbrIfFlowfilterEntry, ival);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.CreateCandidateMo(req,ikey,dmi));
 DalOdbcMgr::clearStubData();
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, CreateCandidateMo_Valid_data2) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_DELETE;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_NONE;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = kMinFlowFilterSeqNum;

 val_flowfilter_entry_t *vbr_if_flowfilter_entry_val = ZALLOC_TYPE(val_flowfilter_entry_t);
 strncpy((char*) vbr_if_flowfilter_entry_val->flowlist_name,"FLOWLISTNAME",32);

 ConfigVal* val = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                vbr_if_flowfilter_entry_val);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,val);
 const char * ctrlr_name = "cntrlr_name";
 const char* version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name,UNC_CT_PFC,version, true);
 CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
 CtrlrMgr::GetInstance()->ctrlrs_.clear();
 CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);

 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);

 ikey->set_user_data((void*)ctrlr_name);
 DalDmlIntf *dmi(getDalDmlIntf());

 EXPECT_EQ(UNC_UPLL_RC_ERR_BAD_REQUEST,obj.CreateCandidateMo(req,ikey,dmi));
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, ReadMo_Detail1) {
 VbrIfFlowFilterEntryMoMgr obj;

  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option2 = UNC_OPT2_NONE;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = kMinFlowFilterSeqNum;
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);


 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 SET_USER_DATA_FLAGS_TEST(ikey, 0x10);

 EXPECT_EQ(UPLL_RC_SUCCESS,obj.ReadMo(req,ikey,dmi));

 delete req;
 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}

TEST_F(vbr_if_entry_test, ReadMo_Detail2) {
 VbrIfFlowFilterEntryMoMgr obj;

  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option1 = UNC_OPT1_DETAIL;
 req->option2 = UNC_OPT2_MAC_ENTRY;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;
 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);


 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 DalDmlIntf *dmi(getDalDmlIntf());
 SET_USER_DATA_FLAGS_TEST(ikey, 0x10);

 EXPECT_EQ(UNC_UPLL_RC_ERR_INVALID_OPTION2,obj.ReadMo(req,ikey,dmi));

 delete req;
 delete ikey;
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}
TEST_F(vbr_if_entry_test, ReadMo_Success) {
 VbrIfFlowFilterEntryMoMgr obj;
  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 req->operation = UNC_OP_READ;
 req->datatype = UPLL_DT_STATE;
 req->option1 = UNC_OPT1_DETAIL;
 req->option2 = UNC_OPT2_NONE;
 key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(key_vbr_if_flowfilter_entry_t);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
 strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
 key_vbr_if_entry->sequence_num = 1;
 const char * ctrlr_name = "pfc1";
 const char * version("version");
 CtrlrMgr::Ctrlr ctrl(ctrlr_name, UNC_CT_PFC, version, true);
 CtrlrMgr::GetInstance()->Add(ctrl, UPLL_DT_RUNNING);

 UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
 ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY, IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,NULL);
 SET_USER_DATA_FLAGS_TEST(ikey,0x04);
 ikey->set_key_type(UNC_KT_VBRIF_FLOWFILTER_ENTRY);
 SET_USER_DATA_CTRLR(ikey,ctrlr_name);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
 DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 //CapaModuleStub::stub_clearStubData();
 //CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_STATE_CAPABILITY,true);
 //CapaModuleStub::stub_loadCapaModule();

 DalDmlIntf *dmi(getDalDmlIntf());
 EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,obj.ReadMo(req,ikey,dmi));

 delete req;
 delete ikey;
 //CapaModuleStub::stub_clearStubData();
 DalOdbcMgr::clearStubData();
 UpllConfigMgr::GetUpllConfigMgr()->upll_kt_momgrs_.clear();
}
TEST_F(vbr_if_entry_test, RenameMo_NoOperation) {
  VbrIfFlowFilterEntryMoMgr obj;
   IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf* dmi = NULL;
  const char* ctrlr_id = {0};
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, obj.RenameMo(req,ikey,dmi,ctrlr_id));
}

TEST_F(vbr_if_entry_test, ValidateAttribute_1) {
  VbrIfFlowFilterEntryMoMgr obj;

  key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(
               key_vbr_if_flowfilter_entry_t);  
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vbr_if_entry->sequence_num = 1;

  val_flowfilter_entry_t *val =
    reinterpret_cast<val_flowfilter_entry_t *>(malloc(sizeof(val_flowfilter_entry_t)));
  //val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID;

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
      IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,cv);  

  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  //op is CREATE 
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->clnt_sess_id = 0;
  req->config_id = 1;

  DalDmlIntf *dmi(getDalDmlIntf());
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  
  //GetConfigMode success
  //unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
    //                                          unc::tclib::TC_API_COMMON_SUCCESS);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey, dmi, req));
  ikey = NULL;
  delete ikey;
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

TEST_F(vbr_if_entry_test, ValidateAttribute_2) {
  VbrIfFlowFilterEntryMoMgr obj;

  key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(
               key_vbr_if_flowfilter_entry_t);  
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vbr_if_entry->sequence_num = 1;

  val_flowfilter_entry_t *val =
    reinterpret_cast<val_flowfilter_entry_t *>(malloc(sizeof(val_flowfilter_entry_t)));
  //val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID;

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
      IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,cv);  

  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 
  //op is CREATE 
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->clnt_sess_id = 0;
  req->config_id = 1;

  DalDmlIntf *dmi(getDalDmlIntf());
  //GetConfigMode failure

  unc::tclib::TcLibModule::stub_loadtcLibModule();
  //unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
    //                                          unc::tclib::TC_INVALID_SESSION_ID);

  EXPECT_EQ(UPLL_RC_SUCCESS, 
          obj.ValidateAttribute(ikey, dmi, req));
  ikey = NULL;
  delete ikey;
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

TEST_F(vbr_if_entry_test, ValidateAttribute_3) {
  VbrIfFlowFilterEntryMoMgr obj;

  key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(
               key_vbr_if_flowfilter_entry_t);  
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vbr_if_entry->sequence_num = 1;

  val_flowfilter_entry_t *val =
    reinterpret_cast<val_flowfilter_entry_t *>(malloc(sizeof(val_flowfilter_entry_t)));
  //val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID;

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
      IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,cv);  

  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
  //op is CREATE 
  req->operation = UNC_OP_UPDATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->clnt_sess_id = 0;
  req->config_id = 1;

  DalDmlIntf *dmi(getDalDmlIntf());
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  
  //GetConfigMode success
  //unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
    //                                          unc::tclib::TC_API_COMMON_SUCCESS);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey, dmi, req));
  ikey = NULL;
  delete ikey;
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

TEST_F(vbr_if_entry_test, ValidateAttribute_4) {
  VbrIfFlowFilterEntryMoMgr obj;

  key_vbr_if_flowfilter_entry_t *key_vbr_if_entry = ZALLOC_TYPE(
               key_vbr_if_flowfilter_entry_t);  
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,"VTN1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.vbr_key.vbridge_name,"VBR1",32);
  strncpy((char*) key_vbr_if_entry->flowfilter_key.if_key.if_name,"IF1",32);
  key_vbr_if_entry->sequence_num = 1;

  val_flowfilter_entry_t *val =
    reinterpret_cast<val_flowfilter_entry_t *>(malloc(sizeof(val_flowfilter_entry_t)));
  //val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID;

  ConfigVal* cv = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                                val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
      IpctSt::kIpcStKeyVbrIfFlowfilterEntry,key_vbr_if_entry,cv);  

  IpcReqRespHeader *req= ZALLOC_TYPE(IpcReqRespHeader);
 
  //op is CREATE 
  req->operation = UNC_OP_UPDATE;
  req->datatype = UPLL_DT_CANDIDATE;
  req->clnt_sess_id = 0;
  req->config_id = 1;

  DalDmlIntf *dmi(getDalDmlIntf());
  //GetConfigMode failure
  unc::tclib::TcLibModule::stub_loadtcLibModule();
  //unc::tclib::TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::GET_CONFIG_MODE,
    //                                          unc::tclib::TC_INVALID_SESSION_ID);

  EXPECT_EQ(UPLL_RC_SUCCESS, 
          obj.ValidateAttribute(ikey, dmi, req));
  ikey = NULL;
  delete ikey;
  unc::tclib::TcLibModule::stub_clearTcLibStubData();
}

TEST_F(vbr_if_entry_test, DeleteChildrenPOM_1) {
  VbrIfFlowFilterEntryMoMgr obj;
  key_vbr_if_flowfilter_entry_t *vbrif_ffe_key =
      reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vbrif_ffe_key->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  vbrif_ffe_key->flowfilter_key.direction = 0xFE;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                                        vbrif_ffe_key, NULL);
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
TEST_F(vbr_if_entry_test, DeleteChildrenPOM_2) {
  VbrIfFlowFilterEntryMoMgr obj;
  key_vbr_if_flowfilter_entry_t *vbrif_ffe_key =
      reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vbrif_ffe_key->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  strncpy(reinterpret_cast<char*>
          (vbrif_ffe_key->flowfilter_key.if_key.vbr_key.vbridge_name),
          "vbr1", 32);
  vbrif_ffe_key->flowfilter_key.direction = 0xFE;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                                        vbrif_ffe_key, NULL);
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
TEST_F(vbr_if_entry_test, DeleteChildrenPOM_3) {
  VbrIfFlowFilterEntryMoMgr obj;
  key_vbr_if_flowfilter_entry_t *vbrif_ffe_key =
      reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vbrif_ffe_key->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  strncpy(reinterpret_cast<char*>
          (vbrif_ffe_key->flowfilter_key.if_key.vbr_key.vbridge_name),
          "vbr1", 32);
  strncpy(reinterpret_cast<char*>
          (vbrif_ffe_key->flowfilter_key.if_key.if_name),
          "if1", 32);
  vbrif_ffe_key->flowfilter_key.direction = 0xFE;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                                        vbrif_ffe_key, NULL);
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
TEST_F(vbr_if_entry_test, DeleteChildrenPOM_4) {
  VbrIfFlowFilterEntryMoMgr obj;
  key_vbr_if_flowfilter_entry_t *vbrif_ffe_key =
      reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_entry_t)));

  strncpy(reinterpret_cast<char*>
          (vbrif_ffe_key->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name),
          "vtn1", kMaxLenVtnName);
  strncpy(reinterpret_cast<char*>
          (vbrif_ffe_key->flowfilter_key.if_key.vbr_key.vbridge_name),
          "vbr1", 32);
  strncpy(reinterpret_cast<char*>
          (vbrif_ffe_key->flowfilter_key.if_key.if_name),
          "if1", 32);
  vbrif_ffe_key->flowfilter_key.direction = 0x0;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                                        vbrif_ffe_key, NULL);
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
TEST_F(vbr_if_entry_test, DeleteChildrenPOM_5) {
  VbrIfFlowFilterEntryMoMgr obj;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
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
TEST_F(vbr_if_entry_test, DeleteChildrenPOM_6) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
TEST_F(vbr_if_entry_test, DeleteChildrenPOM_7) {
  VbrIfFlowFilterEntryMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                                        IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                                        NULL, NULL);
  DalDmlIntf *dmi = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DeleteChildrenPOM(ikey, UPLL_DT_CANDIDATE,
                                                       dmi, TC_CONFIG_GLOBAL,
                                                       "vtn1"));
}
