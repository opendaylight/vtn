/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <limits.h>
#include <gtest/gtest.h>
#include <vtn_momgr.hh>
#include <vbr_if_momgr.hh>
#include <momgr_impl.hh>
#include <config_mgr.hh>
#include <momgr_intf.hh>
#include <dal_odbc_mgr.hh>
#include <dal_dml_intf.hh>
#include <capa_intf.hh>
#include <capa_module_stub.hh>
#include <tclib_module.hh>
#include <dal_cursor.hh>
#include "ut_util.hh"
#include  "tx_update_util.hh"


using namespace unc::upll::test;
using ::testing::InitGoogleTest;
using ::testing::Test;
using namespace std;
using namespace unc::tclib;
using namespace unc::upll::dal;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::config_momgr;
using namespace unc::capa;
using namespace pfc::core;
using namespace unc::upll::dal::schema::table;
using namespace unc::upll::tx_update_util;

/*namespace unc {
namespace upll {
namespace kt_momgr {*/

class VtnMoMgrTest : public UpllTestEnv
{

};

void GetKeyValStruct(key_vtn *&kst, val_vtn *&vst) {

  const char *vtn_name = "VTN_1";
  const char *desc = "thisisvbridge";
  kst = ZALLOC_TYPE(key_vtn);
  strncpy(reinterpret_cast<char *>(kst->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  vst = reinterpret_cast<val_vtn *>(malloc
  (sizeof(val_vtn)));
  memset(vst,0,sizeof(val_vtn));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
  vst->cs_row_status = UNC_VF_VALID;
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->cs_attr[loop] = UNC_CS_APPLIED;
  }
  strncpy(reinterpret_cast<char *>(vst->description), desc,
  strlen(desc)+1);
}

void GetKeyValStructSt(key_vtn *&kst, val_vtn_st *&vst) {

  const char *vtn_name = "VTN_1";
  kst = ZALLOC_TYPE(key_vtn);
  strncpy(reinterpret_cast<char *>(kst->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  vst = reinterpret_cast<val_vtn_st *>(malloc
  (sizeof(val_vtn_st)));
  memset(vst,0,sizeof(val_vtn_st));
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
     sizeof(vst->valid[0]); ++loop) {
    vst->valid[loop] = UNC_VF_VALID;
  }
}

void createControllerInfo(const char* cntrlr_name,upll_keytype_datatype_t data_type,unc_keytype_ctrtype_t cntrl_type)
{
        const char*  version("version");
        CtrlrMgr::Ctrlr ctrl(cntrlr_name,cntrl_type,version,0);
        CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,data_type));
        CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
}

//class VtnMoMgrTest : public UplltestEnv


/* GetRenameKeyBindInfo() */
// Passing NULL

TEST_F(VtnMoMgrTest, outputNull) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_EQ(PFC_TRUE, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, MAINTBL));
}

// Verify the nattr is filled with proper value
TEST_F(VtnMoMgrTest, nattrFill) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_EQ(PFC_TRUE, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, MAINTBL));
  EXPECT_EQ(3, nattr);
}

// Passing controller table to the function
TEST_F(VtnMoMgrTest, ctrlTbl) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_EQ(PFC_TRUE, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, CTRLRTBL));
  EXPECT_EQ(3, nattr);
}

// Passing rename table to the function
TEST_F(VtnMoMgrTest, renameTbl) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_EQ(PFC_TRUE, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, RENAMETBL));
  EXPECT_EQ(2, nattr);
}

// Passing rename table to the function
TEST_F(VtnMoMgrTest, novtnkey) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VBRIDGE;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_EQ(true, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, RENAMETBL));
}

/* ValidateAttribute() */
// Passing null value to the function
TEST_F(VtnMoMgrTest, Test_nullkey) {
  VtnMoMgr vtn;
  //EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateAttribute(ikey, dmi)); //Bug 217
}

// Passing vtn key type to the function
TEST_F(VtnMoMgrTest, keyVtn) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *keyvtn = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *valVtn = ZALLOC_TYPE(val_vtn_t);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateAttribute(ikey, dmi));
}

// Passing other key type to the function
TEST_F(VtnMoMgrTest, keyVtep) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtep_t *keyVtep = ZALLOC_TYPE(key_vtep_t);
  val_vtep_t *valVtep = ZALLOC_TYPE(val_vtep_t);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtep, valVtep);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTEP, IpctSt::kIpcStKeyVtep, keyVtep, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateAttribute(ikey, dmi));
}

/* IsValidKey() */
  // IsValidKey() has to be changed to single parameter
  // till such time the second argument to be passed as dummy
  // to avoid compilation issue

// Passing NULL key to the function
TEST_F(VtnMoMgrTest, keyNull) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = NULL;
  unsigned long index = 1;
  EXPECT_EQ(false, vtn.IsValidKey(keyvtn, index));
}

// Passing the vtn name to the function
TEST_F(VtnMoMgrTest, IsValidKey_vtnName) {
  VtnMoMgr vtn;
  unsigned long index = 0;
  key_vtn_t *keyvtn = ZALLOC_TYPE(key_vtn_t);
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  EXPECT_EQ(true, vtn.IsValidKey(keyvtn, index));
}

// To test the maximum length of vtn name using the function
TEST_F(VtnMoMgrTest, vtnNameMax) {
  VtnMoMgr vtn;
  unsigned long index = 0;
  key_vtn_t *keyvtn = ZALLOC_TYPE(key_vtn_t);
  strcpy((char *)keyvtn->vtn_name,(const char *)"vtn1vtnvtnvtnvtnvtnvtnvtnvtnvtn");
  EXPECT_EQ(true, vtn.IsValidKey(keyvtn, index));
}

// To test the minimum length of vtn name using the function
TEST_F(VtnMoMgrTest, vtnNameMin) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = ZALLOC_TYPE(key_vtn_t);
  unsigned long index = 0;
  strcpy((char *)keyvtn->vtn_name, (const char *)"1");
  EXPECT_EQ(true, vtn.IsValidKey(keyvtn, index));
}

// To test the empty name of vtn name using the function
TEST_F(VtnMoMgrTest, vtnNameEmpty) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = ZALLOC_TYPE(key_vtn_t);
  unsigned long index = 0;
  strcpy((char *)keyvtn->vtn_name, (const char *)"");
  EXPECT_EQ(false, vtn.IsValidKey(keyvtn, index));
}
/* GetValid() */
// Passing NULL to the function
TEST_F(VtnMoMgrTest, nullValue) {
  VtnMoMgr vtn;
  void *val = NULL;
  uint64_t index = uudst::vtn::kDbiVtnOperStatus;
  uint8_t *valid = NULL;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  EXPECT_EQ(NULL, valid);
}

// Passing operstatus to the function
TEST_F(VtnMoMgrTest, operStatus) {
  VtnMoMgr vtn;
  void *val = NULL;
  uint64_t index = uudst::vtn::kDbiVtnOperStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst = ZALLOC_TYPE(val_vtn_st_t);
  vtn_valst->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  // EXPECT_EQ((uint8_t *)UNC_VF_VALID, *valid);
}

// Passing alarmstatus to the function
TEST_F(VtnMoMgrTest, alarmStatus) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnAlarmStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst = ZALLOC_TYPE(val_vtn_st_t);
  vtn_valst->valid[UPLL_IDX_ALARM_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing creationTime to the function
TEST_F(VtnMoMgrTest, creationTime) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnCreationTime;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst = ZALLOC_TYPE(val_vtn_st_t);
  vtn_valst->valid[UPLL_IDX_CREATION_TIME_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing lastUpdatedTime to the function
TEST_F(VtnMoMgrTest, lastUpdatedTime) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnLastUpdatedTime;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst = ZALLOC_TYPE(val_vtn_st_t);
  vtn_valst->valid[UPLL_IDX_LAST_UPDATE_TIME_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing vtnName to the function
TEST_F(VtnMoMgrTest, GetValid_vtnName) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnName;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_t *val_vtn = ZALLOC_TYPE(val_vtn_t);
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing creationTime with DT_CANDIDATE to the function
TEST_F(VtnMoMgrTest, creationTimeDtCandidate) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnCreationTime;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_CANDIDATE;

  val_vtn_t *val_vtn = ZALLOC_TYPE(val_vtn_t);
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing description to the function
TEST_F(VtnMoMgrTest, description) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnDesc;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_t *val_vtn = ZALLOC_TYPE(val_vtn_t);
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  // EXPECT_EQ(UNC_VF_VALID, valid);
}

// Passing description to the function
TEST_F(VtnMoMgrTest, descriptionRenameTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnDesc;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_t *val_vtn = ZALLOC_TYPE(val_vtn_t);
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, RENAMETBL));
}

// Passing description to the function
TEST_F(VtnMoMgrTest, descriptionCtrlrTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnDesc;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_ctrlr_t *val_vtn_ctrlr = ZALLOC_TYPE(val_vtn_ctrlr_t);
  val_vtn_ctrlr->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn_ctrlr));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetValid(val, index, valid, dtType, CTRLRTBL));
  // EXPECT_EQ(UNC_VF_VALID, valid);
}

// Passing alarmstatus to the function
TEST_F(VtnMoMgrTest, alarmStatusCtrlrTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnAlarmStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_rename_vtn_t *val_rename_vtn = ZALLOC_TYPE(val_rename_vtn_t);
  val_rename_vtn->valid[UPLL_IDX_ALARM_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_rename_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, CTRLRTBL));
  // EXPECT_EQ(UNC_VF_VALID, valid);
}

// Passing operstatus to the function
TEST_F(VtnMoMgrTest, operStatusCtrlrTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnAlarmStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_rename_vtn_t *val_rename_vtn = ZALLOC_TYPE(val_rename_vtn_t);
  val_rename_vtn->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_rename_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, CTRLRTBL));
  // EXPECT_EQ(UNC_VF_VALID, valid);
}

/* AllocVal() */
//Passing empty configval
TEST_F(VtnMoMgrTest, emptyval) {
  VtnMoMgr vtn;
  ConfigVal *cfg_val = NULL;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;
  MoMgrTables tbl = RENAMETBL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AllocVal(cfg_val, dtType, tbl));
}

// Passing configval to the function
TEST_F(VtnMoMgrTest, invalidObj) {
  VtnMoMgr vtn;
  val_vtn_t *val_vtn = ZALLOC_TYPE(val_vtn_t);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtnSt, val_vtn);//Invalid st_num
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, MAINTBL));
}

// Passing DT_RUNNING to the function
TEST_F(VtnMoMgrTest, valVtnMaintbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_t *val_vtn = ZALLOC_TYPE(val_vtn_t);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, MAINTBL));//Need to analyse
}

// Passing DT_STATE to the function
TEST_F(VtnMoMgrTest, valVtnStMaintbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_db_vtn_st *val_vtnst = ZALLOC_TYPE(val_db_vtn_st);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtnSt, val_vtnst);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, MAINTBL)); //Need to analyse
}

// Passing RENAMETBL to the function
TEST_F(VtnMoMgrTest, valRenametbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;
  MoMgrTables tbl = RENAMETBL;
  //val_rename_vtn_t *val_rename_Vtn = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));

  //ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValRenameVtn, val_rename_Vtn);
  ConfigVal *config_val=NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AllocVal(config_val, dtType, tbl));//Need to analyse
}

// Passing CTRLRTBL to the function
TEST_F(VtnMoMgrTest, valCtrlrtbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;
  MoMgrTables tbl = CTRLRTBL;
  val_vtn_ctrlr_t *val_vtn_ctrlr = ZALLOC_TYPE(val_vtn_ctrlr_t);
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val_vtn_ctrlr);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, tbl));

}

TEST_F(VtnMoMgrTest, Error_defaultcase) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;
  MoMgrTables tbl = MAX_MOMGR_TBLS;
  ConfigVal *cfg_val = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AllocVal(cfg_val, dtType, tbl));

}

// Passing empty val to the function
TEST_F(VtnMoMgrTest, DupConfigKeyVal_EmptyReqval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey;
  ConfigKeyVal *req=NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnMapping(okey, req));//Need to analyse
}

// Passing empty val to the function
TEST_F(VtnMoMgrTest, EmptyConfigval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnMapping(okey, req));//Need to analyse
}

TEST_F(VtnMoMgrTest, DupConfigmapping_Configval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;

  key_vtn_controller_t *key = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st *val = ZALLOC_TYPE(val_vtn_mapping_controller_st);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key, config_val);


  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyValVtnMapping(okey, ikey));
}

// Passing empty val to the function
TEST_F(VtnMoMgrTest, EmptyReqval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;
  ConfigKeyVal *req=NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnStation(okey, req));//Need to analyse
}

TEST_F(VtnMoMgrTest,DupConfigKeyValVtnstation_kIpcStIpv4) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;

  key_vtn_controller_t *key = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtnstation_controller_st *val = ZALLOC_TYPE(val_vtnstation_controller_st);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStIpv4, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.DupConfigKeyValVtnStation(okey, ikey));
}
TEST_F(VtnMoMgrTest, Configval_kIpcStIpv6) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;

  key_vtn_controller_t *key = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtnstation_controller_st *val = ZALLOC_TYPE(val_vtnstation_controller_st);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStIpv6, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.DupConfigKeyValVtnStation(okey, ikey));
}

TEST_F(VtnMoMgrTest, okey_NOT_NULL) {
  VtnMoMgr vtn;

  key_vtn_controller_t *key = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtnstation_controller_st *val = ZALLOC_TYPE(val_vtnstation_controller_st);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStIpv4, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnStation(okey, ikey));
}
/* FilterAttributes() */ //Function would be removed from vtn_momgr.cc
// Passing val1 as NULL to the function
TEST_F(VtnMoMgrTest, val1Null) {
  VtnMoMgr vtn;
  //void *val2 = NULL;

  val_vtn_t *valVtn2 = ZALLOC_TYPE(val_vtn_t);
  valVtn2->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  //val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  EXPECT_EQ(UNC_VF_VALID, valVtn2->valid[UPLL_IDX_DESC_VTN]);
}

// Passing val2 as NULL to the function
TEST_F(VtnMoMgrTest, val2Null) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vtn_t *valVtn1 = ZALLOC_TYPE(val_vtn_t);
  valVtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn1->valid[UPLL_IDX_DESC_VTN]);
}

// Passing audit_status as false to the function
TEST_F(VtnMoMgrTest, auditStatus) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = false;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vtn_t *valVtn1 = ZALLOC_TYPE(val_vtn_t);
  val_vtn_t *valVtn2 = ZALLOC_TYPE(val_vtn_t);
  valVtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn1->valid[UPLL_IDX_DESC_VTN]);
}

// Passing valid flag to the function
TEST_F(VtnMoMgrTest, val1ValidFlag) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vtn_t *valVtn1 = ZALLOC_TYPE(val_vtn_t);
  val_vtn_t *valVtn2 = ZALLOC_TYPE(val_vtn_t);
  valVtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn1->valid[UPLL_IDX_DESC_VTN]);
}

// Passing valid flag with delete operation to the function
TEST_F(VtnMoMgrTest, val2ValidFlag) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_DELETE;

  val_vtn_t *valVtn1 = ZALLOC_TYPE(val_vtn_t);
  val_vtn_t *valVtn2 = ZALLOC_TYPE(val_vtn_t);
  valVtn2->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn2->valid[UPLL_IDX_DESC_VTN]);
}

/* ValidateVtnKey() */
// Passing NULL to the function
TEST_F(VtnMoMgrTest, Key_nullVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = NULL;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnKey(keyVtn));
}

// Testing the vtn name
TEST_F(VtnMoMgrTest, properVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = ZALLOC_TYPE(key_vtn_t);
  strcpy((char*)keyVtn->vtn_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnKey(keyVtn));
}

// Testing the vtn name with minimum value
TEST_F(VtnMoMgrTest, minVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = ZALLOC_TYPE(key_vtn_t);
  strcpy((char*)keyVtn->vtn_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnKey(keyVtn));
}

// Testing the vtn name with maximum value
TEST_F(VtnMoMgrTest,ValidateKey_maxVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = ZALLOC_TYPE(key_vtn_t);
  strcpy((char*)keyVtn->vtn_name,(const char *)"vtnsddfkjlkssdklfjsdkladdassdd1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnKey(keyVtn));
}


// Testing the vtn name with empty value
TEST_F(VtnMoMgrTest, Validate_emptyVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = ZALLOC_TYPE(key_vtn_t);
  strcpy((char*)keyVtn->vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnKey(keyVtn));
}

/* ValidateVtnValue() */
// Passing NULL to the function
TEST_F(VtnMoMgrTest, invalidVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = ZALLOC_TYPE(val_vtn_t);
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  uint32_t op = UNC_OP_CREATE;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnValue(valVtn, op)); //Bug 250
}

// Testing the vtn description
TEST_F(VtnMoMgrTest, ValidateVtnValue_properVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = ZALLOC_TYPE(val_vtn_t);
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"ashd l1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with minimum value
TEST_F(VtnMoMgrTest, ValidateVtnValue_minVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = ZALLOC_TYPE(val_vtn_t);
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with maximum value
TEST_F(VtnMoMgrTest, Validate_maxVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = ZALLOC_TYPE(val_vtn_t);
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"vtnsddfkjlkssdklfjsdkladdassdd1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}


// Testing the vtn description with empty value
TEST_F(VtnMoMgrTest, Vtn_emptyVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = ZALLOC_TYPE(val_vtn_t);
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with invalid flag
TEST_F(VtnMoMgrTest, invalidFlag) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = ZALLOC_TYPE(val_vtn_t);
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with no value flag
TEST_F(VtnMoMgrTest, novalueFlagCreate) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = ZALLOC_TYPE(val_vtn_t);
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID_NO_VALUE;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
  //EXPECT_EQ(NULL, valVtn->description);
}

// Testing the vtn description with no value flag
TEST_F(VtnMoMgrTest, novalueFlagUpdate) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = ZALLOC_TYPE(val_vtn_t);
  uint32_t op = UNC_OP_UPDATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID_NO_VALUE;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
  //EXPECT_EQ(NULL, valVtn->description);
}

// Testing the vtn description with invalid flag
TEST_F(VtnMoMgrTest, novalueFlagDelete) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = ZALLOC_TYPE(val_vtn_t);
  uint32_t op = UNC_OP_DELETE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID_NO_VALUE;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

/* ValidateVtnRenameValue() */
// Passing NULL to the function
TEST_F(VtnMoMgrTest, Vtn_nullVal) {
  VtnMoMgr vtn;

  //EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateVtnRenameValue(valVtnRename)); // Bug 251
}

// Testing the vtn name
TEST_F(VtnMoMgrTest, VtnRename_properVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename = ZALLOC_TYPE(val_rename_vtn_t);
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
}

// Testing the vtn name with minimum value
TEST_F(VtnMoMgrTest, VtnRename_minVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename = ZALLOC_TYPE(val_rename_vtn_t);
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
}

// Testing the vtn name with maximum value
TEST_F(VtnMoMgrTest, maxVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename = ZALLOC_TYPE(val_rename_vtn_t);
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"vtnsddfkjlkssdklfjsdkladdassdd1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
}

// Testing the vtn name with empty value
TEST_F(VtnMoMgrTest, ValidateVtnRename_emptyVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename = ZALLOC_TYPE(val_rename_vtn_t);
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnRenameValue(valVtnRename));
}

#if 0
// Testing the controller name with minimum value and proper vtn name
TEST_F(VtnMoMgrTest, ctrlNameMin) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the controller name with maximum value and proper vtn name
TEST_F(VtnMoMgrTest, ctrlNameMax) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"vtnstationcontrollersdklfjsdkl1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the controller name with maximum value exceeds and proper vtn name
TEST_F(VtnMoMgrTest, ctrlNameMaxExceeds) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"vtnstationcontrollersdklfjsdklflsdsddf1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the controller name with empty value and proper vtn name
TEST_F(VtnMoMgrTest, ctrlNameempty) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the vtn name with minimum value and proper controller name
TEST_F(VtnMoMgrTest, VtnMapCtrlr_vtnNameMin) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the vtn name with maximum value and proper controller name
TEST_F(VtnMoMgrTest,VtnMapCtrlr_vtnNameMax) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtnsdaflkjsdfhksdfgdghkshglkas1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the vtn name with maximum value exceeds and proper controller name
TEST_F(VtnMoMgrTest, MapCtrlr_vtnNameMaxExceeds) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtnsdaflkjsdfhksdfghkjasdghkshglkask1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the vtn name with empty value and proper controller name
TEST_F(VtnMoMgrTest,MapCtrlrvtnNameEmpty) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

/* ValidateVtnStnCtrlrKey() */
// Passing NULL to the function
TEST_F(VtnMoMgrTest, StnCtrlrnullVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = NULL;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

// Testing the controller name
TEST_F(VtnMoMgrTest, StnCtrlr_properVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = ZALLOC_TYPE(key_vtnstation_controller_t);
  strcpy((char*)keyVtnStn->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

TEST_F(VtnMoMgrTest, UNC_OP_READ_SIBLING_COUNT) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = ZALLOC_TYPE(key_vtnstation_controller_t);
  strcpy((char*)keyVtnStn->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_READ_SIBLING_COUNT));
}

// Testing the controller name with minimum value
TEST_F(VtnMoMgrTest, StnCtrlr_minVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = ZALLOC_TYPE(key_vtnstation_controller_t);
  strcpy((char*)keyVtnStn->controller_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

// Testing the controller name with maximum value
TEST_F(VtnMoMgrTest, StnCtrlr_maxVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = ZALLOC_TYPE(key_vtnstation_controller_t);
  strcpy((char*)keyVtnStn->controller_name,(const char *)"vtnstationcontrollhljhleouuuuuu");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

// Testing the controller name with maximum value exceeds
TEST_F(VtnMoMgrTest, StnCtrlrmaxValExceeds) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = ZALLOC_TYPE(key_vtnstation_controller_t);
  strcpy((char*)keyVtnStn->controller_name,(const char *)"vtnstationcontrollersdklfjsdkldf1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

// Testing the controller name with empty value
TEST_F(VtnMoMgrTest, ValidateStnemptyVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = ZALLOC_TYPE(key_vtnstation_controller_t);
  strcpy((char*)keyVtnStn->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}



//UNC_KT_VTN_MAPPING_CONTROLLER has wrong dt_type
TEST_F(VtnMoMgrTest, MappingReadMo_invaliddatatype) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UNC_UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, vtn.ReadMo(req, ikey, dmi));
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, ReadMo_Mapping_invalidstnum) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UNC_UPLL_RC_ERR_BAD_REQUEST, vtn.ReadMo(req, ikey, dmi));
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no vtn_name
TEST_F(VtnMoMgrTest, Read_Mapping_emptyvtnname) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi)); //Bug raised already
}

TEST_F(VtnMoMgrTest, Mapping_valid) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vnode_name,(const char *)"vnode_name1");
  strcpy((char*)val_mapping->vnode_if_name,(const char *)"vnode_if_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
}
TEST_F(VtnMoMgrTest, Mapping_invalid_01) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vnode_name,(const char *)"vnode_name1");
  strcpy((char*)val_mapping->vnode_if_name,(const char *)"vnode_if_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");

  EXPECT_EQ(UNC_UPLL_RC_ERR_BAD_REQUEST, vtn.ReadMo(req, ikey, dmi));
}

//UNC_KT_VTN_MAPPING_CONTROLLER has domain
TEST_F(VtnMoMgrTest, Mapping_withctrl_domain) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
}

TEST_F(VtnMoMgrTest, Station_invalidkeytype) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
}

//UNC_KT_VTNSTATION_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, Read_Station_invalidstnum) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);

  EXPECT_EQ(UNC_UPLL_RC_ERR_BAD_REQUEST, vtn.ReadMo(req, ikey, dmi));
}

//UNC_KT_VTNSTATION_CONTROLLER has no ctrlr_name
TEST_F(VtnMoMgrTest, Station_ReadMoemptyctrlrname) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  strcpy((char*)key_station->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi)); //Bug raised already
}

TEST_F(VtnMoMgrTest, Station_valid_01) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  uuu::upll_strncpy(key_station->controller_name,"pfc1", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  uuu::upll_strncpy(val_station->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vnode_name,"vnode_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vnode_if_name,"vnode_if_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->port_name,"port_name", (kMaxLenCtrlrId + 1));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi)); //Bug has to raise
}

TEST_F(VtnMoMgrTest, Station_valid_02) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  uuu::upll_strncpy(key_station->controller_name,"pfc1", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  uuu::upll_strncpy(val_station->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vnode_name,"vnode_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vnode_if_name,"vnode_if_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->port_name,"port_name", (kMaxLenCtrlrId + 1));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi)); //Bug has to raise
}
TEST_F(VtnMoMgrTest, Station_valid_03) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  uuu::upll_strncpy(key_station->controller_name,"pfc1", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
}


TEST_F(VtnMoMgrTest, mapping_valid_02) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vnode_name,"vnode_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vnode_if_name,"vnode_if_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
}

TEST_F(VtnMoMgrTest, mapping_valid_03) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  //uint8_t domain_id = 1;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
 // strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vnode_name,(const char *)"vnode_name1");
  strcpy((char*)val_mapping->vnode_if_name,(const char *)"vnode_if_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadMo(req, ikey, dmi));
}
TEST_F(VtnMoMgrTest, mapping_valid_04) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vnode_name,"vnode_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vnode_if_name,"vnode_if_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  createControllerInfo("controller_name1",UPLL_DT_RUNNING,UNC_CT_PFC);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
}
TEST_F(VtnMoMgrTest, mapping_valid_05) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vnode_name,"vnode_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vnode_if_name,"vnode_if_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
}
/* ReadSiblingMo() */

//-------------UNC_OP_READ_SIBLING--------------

//otherKT when option1 = UNC_OPT1_DETAIL
TEST_F(VtnMoMgrTest, KT_VTN_detail) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn = ZALLOC_TYPE(key_vtn_t);
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

TEST_F(VtnMoMgrTest, KT_VBRIDGE) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vbr_t *keyvbr = ZALLOC_TYPE(key_vbr_t);
  strcpy((char*)keyvbr->vbridge_name,(const char *)"vbr1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, keyvbr, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}
//otherKT when option1 = UNC_OPT1_COUNT
TEST_F(VtnMoMgrTest, otherKT_count) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn = ZALLOC_TYPE(key_vtn_t);
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong dt_type
TEST_F(VtnMoMgrTest, Mapping_invaliddatatype) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, Mapping_invalidstnum) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no vtn_name
TEST_F(VtnMoMgrTest, Mapping_emptyvtnname) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already/
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no ctrlr name
TEST_F(VtnMoMgrTest, Mapping_noctrlrname_nodomain) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}
//UNC_KT_VTNSTATION_CONTROLLER has wrong dt_type
TEST_F(VtnMoMgrTest, station_invaliddatatype) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//UNC_KT_VTNSTATION_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, Station_invalidstnum) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//UNC_KT_VTNSTATION_CONTROLLER has no ctrlr_name
TEST_F(VtnMoMgrTest, Station_emptyctrlrname) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  strcpy((char*)key_station->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already
}

//UNC_KT_VTNSTATION_CONTROLLER has invalid datatype
TEST_F(VtnMoMgrTest, Station_invaliddatatype) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug has to raise
}

//UNC_KT_VTNSTATION_CONTROLLER has invalid operation
TEST_F(VtnMoMgrTest, Station_invalidop) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug has to raise
}


//otherKT when option1 = UNC_OPT1_DETAIL
TEST_F(VtnMoMgrTest, otherKT_detail_begin) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn = ZALLOC_TYPE(key_vtn_t);
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//otherKT when option1 = UNC_OPT1_COUNT
TEST_F(VtnMoMgrTest, otherKT_count_begin) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn = ZALLOC_TYPE(key_vtn_t);
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong dt_type
TEST_F(VtnMoMgrTest, Mapping_invaliddatatype_begin) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, Mapping_invalidstnum_begin) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no vtn_name
TEST_F(VtnMoMgrTest, Mapping_emptyvtnname_begin) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already/
}

TEST_F(VtnMoMgrTest, Mapping_01) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

TEST_F(VtnMoMgrTest, Mapping_READ_SIBLING_BEGIN) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vnode_name,"vnode_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vnode_if_name,"vnode_if_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}
TEST_F(VtnMoMgrTest, Mapping_READ) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vnode_name,"vnode_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vnode_if_name,"vnode_if_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}
TEST_F(VtnMoMgrTest, station_READ_SIBLING_BEGIN) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//UNC_KT_VTNSTATION_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, Station_invalidstnum_begin) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

TEST_F(VtnMoMgrTest, Station_READ_SIBLING_BEGIN_01) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  strcpy((char*)key_station->controller_name,(const char *)"pfc11");
  strcpy((char*)val_station->vtn_name,(const char *)"vtn1");
  strcpy((char*)val_station->domain_id,(const char *)"dom1");
  strcpy((char*)val_station->vnode_name,(const char *)"vnode_name1");
  strcpy((char*)val_station->vnode_if_name,(const char *)"vnode_if_name1");
  strcpy((char*)val_station->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_station->port_name,(const char *)"port1");

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

TEST_F(VtnMoMgrTest, Station_READ_SIBLING_BEGIN_02) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  strcpy((char*)key_station->controller_name,(const char *)"pfc11");
  strcpy((char*)val_station->vtn_name,(const char *)"vtn1");
  strcpy((char*)val_station->domain_id,(const char *)"dom1");
  strcpy((char*)val_station->vnode_name,(const char *)"vnode_name1");
  strcpy((char*)val_station->vnode_if_name,(const char *)"vnode_if_name1");
  strcpy((char*)val_station->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_station->port_name,(const char *)"port1");

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
}

//UNC_KT_VTNSTATION_CONTROLLER has no ctrlr_name
TEST_F(VtnMoMgrTest, Station_emptyctrlrname_begin) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  strcpy((char*)key_station->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already
}

//UNC_KT_VTNSTATION_CONTROLLER has invalid operation
TEST_F(VtnMoMgrTest, Station_invalidop_begin) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug has to raise
}

TEST_F(VtnMoMgrTest, vtnStation_01) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  strcpy((char*)key_station->controller_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingCount(req, ikey, dmi));
}

TEST_F(VtnMoMgrTest, vtnstation_val_NULL) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));

}

//when the key is UNC_KT_VTN_MAPPING_CONTROLLER and has invalid key st_num
TEST_F(VtnMoMgrTest, mapping_invalidkeynum) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
}

TEST_F(VtnMoMgrTest, ikey_No_value) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  ConfigKeyVal *ikey =NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
}
//when the key is UNC_KT_VTN_MAPPING_CONTROLLER and without setting vbr valid flag
TEST_F(VtnMoMgrTest, mapping_withoutvbrvalidflag) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
}

//when the key is UNC_KT_VTN_MAPPING_CONTROLLER and with setting vbr valid flag
TEST_F(VtnMoMgrTest, mapping_withoutvbrifvalidflag) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  val_mapping->valid[UPLL_IDX_VNODE_NAME_VMCS] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
}
TEST_F(VtnMoMgrTest, mapping_withvbrifvalidflag) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vnode_name,(const char *)"vnode_name1");
  strcpy((char*)val_mapping->vnode_if_name,(const char *)"vnode_if_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  val_mapping->valid[UPLL_IDX_VNODE_NAME_VMCS] = UNC_VF_VALID;
  val_mapping->valid[UPLL_IDX_VNODE_IF_NAME_VMCS] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
}
TEST_F(VtnMoMgrTest, mapping_02) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
 IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vnode_name,(const char *)"vnode_name1");
  strcpy((char*)val_mapping->vnode_if_name,(const char *)"vnode_if_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  val_mapping->valid[UPLL_IDX_VNODE_NAME_VMCS] = UNC_VF_VALID;
  val_mapping->valid[UPLL_IDX_VNODE_IF_NAME_VMCS] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  val_mapping->map_type = UPLL_IF_VLAN_MAP;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
}
/* ValidateMessageForVtnStnCtrlr */

//when the configkeyval is null
TEST_F(VtnMoMgrTest, ValidateMsg_ikeynull) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnStnCtrlr(req, ikey)); // Code has the fix now
}

//when the IpcReqRespHeader is null
TEST_F(VtnMoMgrTest, ValidateMessage_reqnull) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = new IpcReqRespHeader;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnStnCtrlr(req, ikey)); //Bug 404
}

//when invalid st_num
TEST_F(VtnMoMgrTest, invalidValidate_stnum) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
}

TEST_F(VtnMoMgrTest, Invalid_option1) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1=(unc_keytype_option1_t)3;
  req->option2 = UNC_OPT2_NEIGHBOR;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
}

TEST_F(VtnMoMgrTest, Invalid_option2) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;

  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
}

//when valid stnum with ctrlr name
TEST_F(VtnMoMgrTest, valid_Messages_tnum_ctrlname) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
//  req->datatype == UPLL_DT_STATE;
  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
}

/* ValidateMessageForVtnMapCtrlr */

//when the configkeyval is null
TEST_F(VtnMoMgrTest, ikeynull) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);
  key_vtnstation_controller_t *key_station = ZALLOC_TYPE(key_vtnstation_controller_t);
  val_vtnstation_controller_st_t *val_station = ZALLOC_TYPE(val_vtnstation_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessageForVtnMapCtrlr(req, ikey)); // Code has the fix now
}

//when the IpcReqRespHeader is null
TEST_F(VtnMoMgrTest, reqnull) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = new IpcReqRespHeader;;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, vtn.ValidateMessageForVtnMapCtrlr(req, ikey)); // Code has the fix now
}

//when invalid st_num
TEST_F(VtnMoMgrTest, invalid_stnum) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
}

//when valid stnum with empty ctrlr name and vtn name
TEST_F(VtnMoMgrTest, valid_stnum_emptyctrlvtnname) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
}

//when valid stnum with empty ctrlr name
TEST_F(VtnMoMgrTest, valid_stnum_emptyctrlname) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
}

//when valid stnum with ctrlr name
TEST_F(VtnMoMgrTest, valid_stnum_ctrlname) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
}

//when valid stnum with empty vtn name
TEST_F(VtnMoMgrTest, valid_stnum_emptyvtnname) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
}

TEST_F(VtnMoMgrTest, invalid_option1) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
}

TEST_F(VtnMoMgrTest, invalid_option2) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
}

//when invalid datatype
TEST_F(VtnMoMgrTest, invalid_datatype) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
}

//when invalid operation
TEST_F(VtnMoMgrTest, invalid_operation) {
  VtnMoMgr vtn;
 IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = ZALLOC_TYPE(key_vtn_controller_t);
  val_vtn_mapping_controller_st_t *val_mapping = ZALLOC_TYPE(val_vtn_mapping_controller_st_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
}

/* DupConfigKeyVal */

TEST_F(VtnMoMgrTest, nullkey) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = RENAMETBL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
}
TEST_F(VtnMoMgrTest, invalid_ctrlrName) {

  VtnMoMgr vtn;
  val_vtn *val;

  const char *controller_name = "";
  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool in_use;
  key_vtn *key = ZALLOC_TYPE(key_vtn);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  key_ctr_t *key_ctr_test =
           reinterpret_cast<key_ctr_t *>(ikey->get_key());
  memset(key_ctr_test,0,sizeof(key_ctr));
  strncpy(reinterpret_cast<char *>(key_ctr_test->controller_name),
  controller_name, strlen(controller_name)+1);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsKeyInUse(dt_type,ikey,&in_use,dmi));
}

TEST_F(VtnMoMgrTest, KeyInUse_ikey_NULL) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);
  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool in_use;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsKeyInUse(dt_type,ikey,&in_use,dmi));
}
/*TEST_F(VtnMoMgrTest, ValidateMessage_01) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

 IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey));
  cout<<"TEST_F: Positive: ValidCREATE"<<endl;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval2 = new ConfigVal(IpctSt::kIpcStValVtn, val);

  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval2);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey2));
  val_rename_vtn *renameval = ZALLOC_TYPE(val_rename_vtn);
  for(unsigned int loop = 0; loop < sizeof(renameval->valid)/
     sizeof(renameval->valid[0]); ++loop) {
    renameval->valid[loop] = UNC_VF_VALID;
  }
  strncpy(reinterpret_cast<char *>(renameval->new_name),
  "renamed", strlen("renamed")+1);


  ConfigVal *rename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVtn, renameval);
  ConfigKeyVal *rename_ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, rename_cfgval);

  req->operation = UNC_OP_RENAME;
  req->datatype = UPLL_DT_IMPORT;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, rename_ikey));
  cout<<"TEST_F: Positive: ValidRENAME"<<endl;

  ConfigVal *cfgval3 = NULL;
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval3);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessage(req, ikey3));

  val_rename_vtn *vtn_rename =ZALLOC_TYPE(val_rename_vtn_t);

  ConfigVal *cfgval4 = new ConfigVal(IpctSt::kIpcStValVtn, vtn_rename);
  ConfigKeyVal *ikey4 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval4);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey4));

  req->operation = UNC_OP_READ_SIBLING;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, rename_ikey));
  cout<<"TEST_F: Positive: ValidREAD_SIBLING_DT_IMPORT"<<endl;

  ConfigVal *invrename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVtn, NULL);
  ConfigKeyVal *invrename_ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, invrename_cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, invrename_ikey));
  cout<<"TEST_F: Positive: ValidValRenameNullData"<<endl;

  const char *vtn_name1 = " ";
  key_vtn *key5 = ZALLOC_TYPE(key_vtn);
  val_vtn *val5 = ZALLOC_TYPE(val_vtn);
  strncpy(reinterpret_cast<char *>(key5->vtn_name),
  vtn_name1, strlen(vtn_name1)+1);
  ConfigVal *cfgval5 = new ConfigVal(IpctSt::kIpcStValVtn, val5);
  ConfigKeyVal *ikey5 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key5, cfgval5);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessage(req, ikey5));

  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_RUNNING;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey));
  cout<<"TEST_F: Positive: ValidREAD_SIBLING_COUNT_DT_RUNNING"<<endl;


  ikey=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateMessage(req, ikey));


  ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVbr,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey));

  ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey));

  delete ikey;
  delete ikey3;
  delete ikey2;
  delete ikey4;
  delete ikey5;
  delete rename_ikey;

}

TEST_F(VtnMoMgrTest, ValidateMessage_02) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

 IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_UPDATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey));
  req->operation = UNC_OP_CONTROL;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, vtn.ValidateMessage(req, ikey));

  req->operation = UNC_OP_UPDATE;
  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey1));

  ConfigVal *cfgval2 = NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval2);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessage(req, ikey2));

  val = NULL;
  ConfigVal *cfgval3 = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval3);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey3));
  delete ikey;
  delete ikey3;
  delete ikey2;
  delete ikey1;


}
TEST_F(VtnMoMgrTest, ValidateMessage_03) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

 IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_IMPORT;
  req->operation = UNC_OP_READ;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ValidateMessage(req, ikey));

  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vtn.ValidateMessage(req, ikey));

  req->option2 = UNC_OPT2_NONE;
  req->option1 = UNC_OPT1_NORMAL;

  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey1));

  ConfigVal *cfgval2 = NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval2);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey2));

  val_vtn *val1=NULL;
  ConfigVal *cfgval3 = new ConfigVal(IpctSt::kIpcStValVtn, val1);
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval3);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey3));
  delete ikey;
  delete ikey3;
  delete ikey2;
  delete ikey1;

}

TEST_F(VtnMoMgrTest, ValidateMessage_04) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

 IPC_REQ_RESP_HEADER_DECL(req);
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->rep_count = 100;
  req->datatype = UPLL_DT_IMPORT;
  req->operation = UNC_OP_READ_NEXT;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey));

  req->operation = UNC_OP_DELETE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey));


  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vtn.ValidateMessage(req, ikey));

  req->option2 = UNC_OPT2_NONE;
  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVbr, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval1);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey1));

  ConfigVal *cfgval2 = NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval2);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey2));

  val_vtn *val1=NULL;
  ConfigVal *cfgval3 = new ConfigVal(IpctSt::kIpcStValVtn, val1);
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval3);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey3));
  delete ikey;
  delete ikey3;
  delete ikey2;
  delete ikey1;
}

TEST_F(VtnMoMgrTest, ValVtnAttributeSupportCheck_01) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);
  uint32_t operation= UNC_OP_CREATE;
  const uint8_t *attrs = ;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValVtnAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}
TEST_F(VtnMoMgrTest, ValVtnAttributeSupportCheck_02) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);
  uint32_t operation = UNC_OP_CREATE;
  //val->valid[UPLL_IDX_DESC_VTN] == UNC_VF_VALID;
  const uint8_t *attrs = 0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValVtnAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}
*/
TEST_F(VtnMoMgrTest, SetOperStatus_ikey_NULL) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  DalDmlIntf *dmi(getDalDmlIntf());

  GetKeyValStruct(key, val);
  state_notification notification = kCommit;
  ConfigKeyVal *ikey =NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetOperStatus(ikey,notification,dmi,true));
  delete ikey;
}
TEST_F(VtnMoMgrTest, invalid_key) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_db_vtn_st_t *val = ZALLOC_TYPE(val_db_vtn_st_t);
  //memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val->down_count=0;
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
}
TEST_F(VtnMoMgrTest, valid_key) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_db_vtn_st_t *val = ZALLOC_TYPE(val_db_vtn_st_t);
  //memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val->down_count=0;
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
}
TEST_F(VtnMoMgrTest,CreateVtunnelKey_ikey_NULL ) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.CreateVtunnelKey(ikey,okey));
  delete ikey;
}
TEST_F(VtnMoMgrTest,IpctSt_invalid ) {

  VtnMoMgr vtn;
  key_vtunnel_t *key = ZALLOC_TYPE(key_vtunnel_t);
  val_vtunnel_t *val = ZALLOC_TYPE(val_vtunnel_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.CreateVtunnelKey(ikey,okey));
}
TEST_F(VtnMoMgrTest,IpctSt_valid ) {

  VtnMoMgr vtn;
  key_vtunnel_t *key = ZALLOC_TYPE(key_vtunnel_t);
  val_vtunnel_t *val = ZALLOC_TYPE(val_vtunnel_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtunnelSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel, key, config_val);
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.CreateVtunnelKey(ikey,okey));
}
TEST_F(VtnMoMgrTest,SwapKey_ikey_NULL ) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctrlr=NULL;
  bool no_rename;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey,okey,dmi,ctrlr,no_rename));
  delete ikey;
  delete okey;
}
/*
TEST_F(VtnMoMgrTest,SwapKey_IpctSt_valid ) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "VTN_1";
  bool no_rename;
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val = ZALLOC_TYPE(val_rename_vtn_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));

  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));

  ConfigVal *config_val1= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val1);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.SwapKeyVal(ikey1,okey,dmi,ctr_id1,no_rename));

//  ConfigVal *config_val2= NULL;
//  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val2);
//  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.SwapKeyVal(ikey2,okey,dmi,ctr_id1,no_rename));

//  val_rename_vtn_t *val3 = ZALLOC_TYPE(val_rename_vtn_t);;
//  ConfigVal *config_val3= new ConfigVal(IpctSt::kIpcStValVtnSt, val3);
//  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val3);
//  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey3,okey,dmi,ctr_id1,no_rename));

  delete ikey;
  delete ikey1;
//  delete ikey2;
  delete okey;
}*/
TEST_F(VtnMoMgrTest,IpctSt_valid_01 ) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "VTN_1";
  bool no_rename;
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  //memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val = ZALLOC_TYPE(val_rename_vtn_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
  delete ikey;
  delete okey;
}

TEST_F(VtnMoMgrTest,same_newName ) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "vtn1";
  const char *new_name = "vtn1";
  bool no_rename;
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  //memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val = ZALLOC_TYPE(val_rename_vtn_t);
 // memset(val,0,sizeof(val_rename_vtn));
  strncpy(reinterpret_cast<char *>(val->new_name),
  new_name, strlen(new_name)+1);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
  delete ikey;
  delete okey;
}
TEST_F(VtnMoMgrTest,EmptyNewName ) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "vtn1";
  const char *new_name = " ";
  bool no_rename;
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  //memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val = ZALLOC_TYPE(val_rename_vtn_t);
  //memset(val,0,sizeof(val_rename_vtn));
  strncpy(reinterpret_cast<char *>(val->new_name),
  new_name, strlen(new_name)+1);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
}

TEST_F(VtnMoMgrTest,valid ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  key_vtn_controller *key = ZALLOC_TYPE(key_vtn_controller);
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = ZALLOC_TYPE(val_vtn_ctrlr);
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
}

TEST_F(VtnMoMgrTest,phase_kUpllUcpCreate ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  key_vtn_controller *key = ZALLOC_TYPE(key_vtn_controller);
  //key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = ZALLOC_TYPE(val_vtn_ctrlr);
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
}
TEST_F(VtnMoMgrTest,phase_kUpllUcpUpdate ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
  key_vtn_controller *key = ZALLOC_TYPE(key_vtn_controller);
  //key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = ZALLOC_TYPE(val_vtn_ctrlr);
  val_vtn->cs_row_status = UNC_CS_INVALID;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
}
TEST_F(VtnMoMgrTest,phase_kUpllUcpUpdate_invalid ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status=UNC_CS_INVALID;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
  key_vtn_controller *key = ZALLOC_TYPE(key_vtn_controller);
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = ZALLOC_TYPE(val_vtn_ctrlr);
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
}
TEST_F(VtnMoMgrTest,UpdateAudit_valid ) {

  VtnMoMgr vtn;
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateAuditConfigStatus(cs_status,phase,ikey,dmi));
}
TEST_F(VtnMoMgrTest, phaseUpdateAudit_kUpllUcpCreate ) {

  VtnMoMgr vtn;
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateAuditConfigStatus(cs_status,phase,ikey,dmi));
}
TEST_F(VtnMoMgrTest,SetConsolidateikey_NULL ) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctrlr=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctrlr,dmi));
  delete ikey;
}
TEST_F(VtnMoMgrTest,ctrlr_valid ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key = ZALLOC_TYPE(key_vtn_controller);
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = ZALLOC_TYPE(val_vtn_ctrlr_t);
  val_vtn->down_count=0;
//  val_vtn->ref_count=0;
  val_vtn->cs_row_status=UNC_CS_APPLIED;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctr_id1,dmi));
}
TEST_F(VtnMoMgrTest,keytype_valid ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key = ZALLOC_TYPE(key_vtn_controller);
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = ZALLOC_TYPE(val_vtn_ctrlr_t);
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctr_id1,dmi));
}
TEST_F(VtnMoMgrTest,ikey_valid ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key = ZALLOC_TYPE(key_vtn_controller);
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = ZALLOC_TYPE(val_vtn_ctrlr_t);
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
}
TEST_F(VtnMoMgrTest,Consolidate_ikey_proper ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_vtn_ctrlr_t *val_vtn = ZALLOC_TYPE(val_vtn_ctrlr_t);
  val_vtn->down_count=0;
  //val_vtn->ref_count=0;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
}
TEST_F(VtnMoMgrTest,TxCopy_ikey_NULL ) {

  VtnMoMgr vtn;
  unc_key_type_t keytype = UNC_KT_VTN;
  CtrlrCommitStatusList *ctrlr_commit_status=NULL;
  string vtn_name = "vtn1";
  TcConfigMode config_mode = TC_CONFIG_VTN;

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxCopyCandidateToRunning
            (keytype,ctrlr_commit_status,dmi, config_mode, vtn_name));
}
TEST_F(VtnMoMgrTest,valid_01 ) {

  VtnMoMgr vtn;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;

  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_SUCCESS;
  DalOdbcMgr::clearStubData();
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD,kDalRcRecordNoMore);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD, kDalRcRecordNoMore);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD, kDalRcRecordNoMore);

  CtrlrCommitStatusList.push_back(l_CtrlrTxResult);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc_key_type_t keytype = UNC_KT_VTN;
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  l_CtrlrTxResult->err_ckv = ikey;
  string vtn_name = "vtn1";
  TcConfigMode config_mode = TC_CONFIG_VTN;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.TxCopyCandidateToRunning
            (keytype,&CtrlrCommitStatusList,dmi,config_mode, vtn_name));
}
TEST_F(VtnMoMgrTest,valid_02 ) {

  VtnMoMgr vtn;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;

  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_GENERIC;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::GET_DELETED_RECORDS,kDalRcSuccess);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::DELETE_RECORD,kDalRcRecordNoMore);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD, kDalRcRecordNoMore);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::UPDATE_RECORD, kDalRcRecordNoMore);

  CtrlrCommitStatusList.push_back(l_CtrlrTxResult);
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  unc_key_type_t keytype = UNC_KT_VTN;
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  l_CtrlrTxResult->err_ckv = ikey;
  string vtn_name = "vtn1";
  TcConfigMode config_mode = TC_CONFIG_VTN;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxCopyCandidateToRunning
            (keytype,&CtrlrCommitStatusList,dmi,config_mode, vtn_name));
  DalOdbcMgr::clearStubData();
}
TEST_F(VtnMoMgrTest, GetRenameInfo_ikey_NULL ) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey=NULL;
  ConfigKeyVal *okey=NULL;
  ConfigKeyVal *rename_info=NULL;
  const char *ctrlr_id = "Controller1";
  DalDmlIntf *dmi(getDalDmlIntf());
  bool renamed;
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenameInfo(ikey,okey,rename_info,dmi,ctrlr_id,renamed));
}
TEST_F(VtnMoMgrTest, key_NULL ) {

  VtnMoMgr vtn;
  const char *ctrlr_id = "Controller1";
  DalDmlIntf *dmi(getDalDmlIntf());
  bool renamed;
  key_rename_vnode_info_t *key1 = ZALLOC_TYPE(key_rename_vnode_info_t);
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *rename_info = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key1, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenameInfo(ikey,okey,rename_info,dmi,ctrlr_id,renamed));
}
TEST_F(VtnMoMgrTest, renamed_false ) {

  VtnMoMgr vtn;
  const char *ctrlr_id = "Controller1";
  DalDmlIntf *dmi(getDalDmlIntf());
  bool renamed=false;
  key_rename_vnode_info_t *key1 = ZALLOC_TYPE(key_rename_vnode_info_t);
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *rename_info = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key1, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenameInfo(ikey,okey,rename_info,dmi,ctrlr_id,renamed));
}
TEST_F(VtnMoMgrTest, renamed_true ) {

  VtnMoMgr vtn;
  const char *ctrlr_id = "Controller1";
  DalDmlIntf *dmi(getDalDmlIntf());
  bool renamed=true;
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *rename_info = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenameInfo(ikey,okey,rename_info,dmi,ctrlr_id,renamed));
}
/*
TEST_F(VtnMoMgrTest,Create_01 ) {

  VtnMoMgr vtn;
      IPC_RESPONSE_DECL(req);
  unc_keytype_operation_t op=UNC_OP_CREATE;
  controller_domain *ctrlr_dom=NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrSt, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, NULL, config_val);
  IpcResponse resp;
  memset(&resp, 0, sizeof(resp));
  uint32_t session_id = 1;
  uint32_t config_id = 1;

  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  bool driver_resp = false;
  set<string> *affected_ctrlr_set;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateProcess(ikey,&resp,op,dmi,ctrlr_dom, affected_ctrlr_set, &driver_resp));

}
TEST_F(VtnMoMgrTest,Create_02 ) {

  VtnMoMgr vtn;
      IPC_RESPONSE_DECL(req);
  unc_keytype_operation_t op=UNC_OP_CREATE;
  controller_domain *ctrlr_dom=NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  IpcResponse resp;
  memset(&resp, 0, sizeof(resp));
  uint32_t session_id = 1;
  uint32_t config_id = 1;

  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  bool driver_resp = false;
  set<string> *affected_ctrlr_set;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateProcess(ikey,&resp,op,dmi,ctrlr_dom, affected_ctrlr_set, &driver_resp));

}
TEST_F(VtnMoMgrTest,Create_03 ) {

  VtnMoMgr vtn;
      IPC_RESPONSE_DECL(req);
  unc_keytype_operation_t op=UNC_OP_CREATE;
  controller_domain *ctrlr_dom=NULL;
  //DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  IpcResponse resp;
  memset(&resp, 0, sizeof(resp));
  uint32_t session_id = 1;
  uint32_t config_id = 1;

  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  bool driver_resp = false;
  set<string> *affected_ctrlr_set;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateProcess(ikey,&resp,op,dmi,ctrlr_dom, affected_ctrlr_set, &driver_resp));

}
TEST_F(VtnMoMgrTest,Update ) {

  VtnMoMgr vtn;
      IPC_RESPONSE_DECL(req);
  unc_keytype_operation_t op=UNC_OP_UPDATE;
  controller_domain *ctrlr_dom=NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  IpcResponse resp;
  memset(&resp, 0, sizeof(resp));
  uint32_t session_id = 1;
  uint32_t config_id = 1;

  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  bool driver_resp = false;
  set<string> *affected_ctrlr_set;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.TxUpdateProcess(ikey,&resp,op,dmi,ctrlr_dom, affected_ctrlr_set, &driver_resp));
}
TEST_F(VtnMoMgrTest,Delete ) {

  VtnMoMgr vtn;
      IPC_RESPONSE_DECL(req);
  unc_keytype_operation_t op=UNC_OP_DELETE;
  controller_domain *ctrlr_dom=NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  IpcResponse resp;
  memset(&resp, 0, sizeof(resp));
  uint32_t session_id = 1;
  uint32_t config_id = 1;

  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  bool driver_resp = false;
  set<string> *affected_ctrlr_set;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateProcess(ikey,&resp,op,dmi,ctrlr_dom, affected_ctrlr_set, &driver_resp));
}
*/
TEST_F(VtnMoMgrTest,TxUpdate_default ) {

  VtnMoMgr vtn;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  set<string> *affected_ctrlr_set = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateController(UNC_KT_VTN,session_id,config_id,
                                                        phase,affected_ctrlr_set,dmi,&ikey,
                                                        &tx, config_mode, vtn_name));
}
TEST_F(VtnMoMgrTest,Create ) {

  VtnMoMgr vtn;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::NEXT,kDalRcSuccess);
  set<string> *affected_ctrlr_set = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateController(UNC_KT_VTN,session_id,config_id,
                                                        phase,affected_ctrlr_set,dmi,&ikey,
                                                        &tx, config_mode, vtn_name));
}
TEST_F(VtnMoMgrTest,TxController_Update ) {

  VtnMoMgr vtn;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
  set<string> *affected_ctrlr_set = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateController(UNC_KT_VTN,session_id,config_id,
                                                        phase,affected_ctrlr_set,dmi,&ikey,
                                                        &tx, config_mode, vtn_name));
}
TEST_F(VtnMoMgrTest,TxController_Delete ) {

  VtnMoMgr vtn;
  uint32_t session_id = 1;
  uint32_t config_id = 1;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpDelete;
  set<string> *affected_ctrlr_set = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key = ZALLOC_TYPE(key_vtn_t);
  val_vtn_t *val = ZALLOC_TYPE(val_vtn_t);
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";
  TxUpdateUtil tx(4);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateController(UNC_KT_VTN,session_id,config_id,
                                                        phase,affected_ctrlr_set,dmi,&ikey,
                                                        &tx, config_mode, vtn_name));
}
#endif
//}
//}
//}
