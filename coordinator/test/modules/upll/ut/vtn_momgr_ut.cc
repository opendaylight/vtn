 /*
  * Copyright (c) 2012-2013 NEC Corporation
  * All rights reserved.
  *
  * This program and the accompanying materials are made available under the
  * terms of the Eclipse Public License v1.0 which accompanies this
  * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
  */


#include <limits.h>
#include <gtest/gtest.h>
#include "vtn_momgr.hh"
#include "vbr_if_momgr.hh"
#include "momgr_impl.hh"
#include "config_mgr.hh"
#include "momgr_intf.hh"
#include "dal_odbc_mgr.hh"
#include "dal_dml_intf.hh"
#include "capa_intf.hh"
#include "capa_module_stub.hh"
#include "tclib_module.hh"
#include "dal_cursor.hh"

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

namespace unc {
namespace upll {
namespace kt_momgr {

void GetKeyValStruct(key_vtn *&kst, val_vtn *&vst) {

  const char *vtn_name = "VTN_1";
  const char *desc = "thisisvbridge";
  kst = reinterpret_cast<key_vtn *>(malloc
                 (sizeof(key_vtn)));
  memset(kst,0,sizeof(key_vtn));
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
  //const char *desc = "thisisvbridge";
  kst = reinterpret_cast<key_vtn *>(malloc
                 (sizeof(key_vtn)));
  memset(kst,0,sizeof(key_vtn));
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
        CtrlrMgr::Ctrlr ctrl(cntrlr_name,cntrl_type,version);
        CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,data_type));
        CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
}

/* GetRenameKeyBindInfo() */
// Passing NULL
TEST(GetRenameKeyBindInfo, outputNull) {
  VtnMoMgr vtn;
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  TcLibModule::stub_setTCApiCommonRetcode(TcLibModule::REGISTER,unc::tclib::TC_API_COMMON_SUCCESS);
  TcLibModule::stub_loadtcLibModule();
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;
  //MoMgrTables tbl;

  EXPECT_EQ(PFC_TRUE, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, MAINTBL));
}

// Verify the nattr is filled with proper value
TEST(GetRenameKeyBindInfo, nattrFill) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_EQ(PFC_TRUE, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, MAINTBL));
  EXPECT_EQ(3, nattr);
}

// Passing controller table to the function
TEST(GetRenameKeyBindInfo, ctrlTbl) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_EQ(PFC_TRUE, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, CTRLRTBL));
  EXPECT_EQ(3, nattr);
}

// Passing rename table to the function
TEST(GetRenameKeyBindInfo, renameTbl) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_EQ(PFC_TRUE, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, RENAMETBL));
  EXPECT_EQ(2, nattr);
}

// Passing rename table to the function
TEST(GetRenameKeyBindInfo, novtnkey) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VBRIDGE;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_EQ(true, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, RENAMETBL));
}

/* ValidateAttribute() */
// Passing null value to the function
TEST(ValidateAttribute, nullkey) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateAttribute(ikey, dmi)); //Bug 217
}

// Passing vtn key type to the function
TEST(ValidateAttribute, keyVtn) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateAttribute(ikey, dmi));
}

// Passing other key type to the function
TEST(ValidateAttribute, keyVtep) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtep_t *keyVtep = (key_vtep_t *)malloc(sizeof(key_vtep_t));
  val_vtep_t *valVtep = (val_vtep_t *)malloc(sizeof(val_vtep_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtep, valVtep);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTEP, IpctSt::kIpcStKeyVtep, keyVtep, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateAttribute(ikey, dmi));
}

// Passing NULL key to the function
TEST(IsValidKey, keyNull) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = NULL;
  unsigned long index = 1;
  EXPECT_EQ(false, vtn.IsValidKey(keyvtn, index));
}

// Passing the vtn name to the function
TEST(IsValidKey, vtnName) {
  VtnMoMgr vtn;
  unsigned long index = 0;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  EXPECT_EQ(true, vtn.IsValidKey(keyvtn, index));
}

// To test the maximum length of vtn name using the function
TEST(IsValidKey, vtnNameMax) {
  VtnMoMgr vtn;
  unsigned long index = 0;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char *)keyvtn->vtn_name,(const char *)"vtn1vtnvtnvtnvtnvtnvtnvtnvtnvtn");
  EXPECT_EQ(true, vtn.IsValidKey(keyvtn, index));
}

// To test the minimum length of vtn name using the function
TEST(IsValidKey, vtnNameMin) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  unsigned long index = 0;
  strcpy((char *)keyvtn->vtn_name, (const char *)"1");
  EXPECT_EQ(true, vtn.IsValidKey(keyvtn, index));
}

// To test exceeding the maximum length of vtn name using the function
TEST(IsValidKey, vtnNameMaxExceeds) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  unsigned long index = 0;
  strcpy((char *)keyvtn->vtn_name, (const char *)("vtnnkjdsokljhkjdsncvdsjkjdksdjjksd1"));
  EXPECT_EQ(false, vtn.IsValidKey(keyvtn, index));
}

// To test the empty name of vtn name using the function
TEST(IsValidKey, vtnNameEmpty) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  unsigned long index = 0;
  strcpy((char *)keyvtn->vtn_name, (const char *)"");
  EXPECT_EQ(false, vtn.IsValidKey(keyvtn, index));
}
/* GetValid() */
// Passing NULL to the function
TEST(GetValid, nullValue) {
  VtnMoMgr vtn;
  void *val = NULL;
  uint64_t index = uudst::vtn::kDbiVtnOperStatus;
  uint8_t *valid = NULL;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  EXPECT_EQ(NULL, valid);
}

// Passing operstatus to the function
TEST(GetValid, operStatus) {
  VtnMoMgr vtn;
  void *val = NULL;
  uint64_t index = uudst::vtn::kDbiVtnOperStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst = (val_vtn_st_t *)malloc(sizeof(val_vtn_st_t));
  vtn_valst->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing alarmstatus to the function
TEST(GetValid, alarmStatus) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnAlarmStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst = (val_vtn_st_t *)malloc(sizeof(val_vtn_st_t));
  vtn_valst->valid[UPLL_IDX_ALARM_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing creationTime to the function
TEST(GetValid, creationTime) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnCreationTime;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst = (val_vtn_st_t *)malloc(sizeof(val_vtn_st_t));
  vtn_valst->valid[UPLL_IDX_CREATION_TIME_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing lastUpdatedTime to the function
TEST(GetValid, lastUpdatedTime) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnLastUpdatedTime;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst = (val_vtn_st_t *)malloc(sizeof(val_vtn_st_t));
  vtn_valst->valid[UPLL_IDX_LAST_UPDATE_TIME_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing vtnName to the function
TEST(GetValid, vtnName) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnName;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_t *val_vtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing creationTime with DT_CANDIDATE to the function
TEST(GetValid, creationTimeDtCandidate) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnCreationTime;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_CANDIDATE;

  val_vtn_t *val_vtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  val_vtn->valid[UPLL_IDX_CREATION_TIME_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing description to the function
TEST(GetValid, description) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnDesc;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_t *val_vtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
}

// Passing description to the function
TEST(GetValid, descriptionRenameTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnDesc;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_t *val_vtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, RENAMETBL));
}

// Passing description to the function
TEST(GetValid, descriptionCtrlrTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnDesc;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_ctrlr_t *val_vtn_ctrlr = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  val_vtn_ctrlr->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn_ctrlr));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetValid(val, index, valid, dtType, CTRLRTBL));
}

// Passing alarmstatus to the function
TEST(GetValid, alarmStatusCtrlrTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnAlarmStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_rename_vtn_t *val_rename_vtn = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  val_rename_vtn->valid[UPLL_IDX_ALARM_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_rename_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, CTRLRTBL));
}

// Passing operstatus to the function
TEST(GetValid, operStatusCtrlrTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnAlarmStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_rename_vtn_t *val_rename_vtn = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  val_rename_vtn->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_rename_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, CTRLRTBL));
}

/* AllocVal() */
//Passing empty configval
TEST(AllocVal, emptyval) {
  VtnMoMgr vtn;
  ConfigVal *cfg_val = NULL;
  upll_keytype_datatype_t dtType = UPLL_DT_CANDIDATE;
  MoMgrTables tbl=MAINTBL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AllocVal(cfg_val, dtType, tbl));
}

// Passing configval to the function
TEST(AllocVal, invalidObj) {
  VtnMoMgr vtn;
  val_vtn_t *val_vtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtnSt, val_vtn);//Invalid st_num
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, MAINTBL));
}

// Passing DT_RUNNING to the function
TEST(AllocVal, valVtnMaintbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_t *val_vtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, MAINTBL));//Need to analyse
}

// Passing DT_STATE to the function
TEST(AllocVal, valVtnStMaintbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_db_vtn_st *val_vtnst = (val_db_vtn_st *)malloc(sizeof(val_db_vtn_st));
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtnSt, val_vtnst);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, MAINTBL)); //Need to analyse
}

// Passing RENAMETBL to the function
TEST(AllocVal, valRenametbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_CANDIDATE;
  MoMgrTables tbl = RENAMETBL;
  ConfigVal *config_val=NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AllocVal(config_val, dtType, tbl));//Need to analyse
}

// Passing CTRLRTBL to the function
TEST(AllocVal, valCtrlrtbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_CANDIDATE;
  MoMgrTables tbl = CTRLRTBL;
  val_vtn_ctrlr_t *val_vtn_ctrlr = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val_vtn_ctrlr);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, tbl));

}

TEST(AllocVal, Error_defaultcase) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;
  MoMgrTables tbl = MAX_MOMGR_TBLS;
  ConfigVal *cfg_val = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AllocVal(cfg_val, dtType, tbl));

}

// Passing empty val to the function
TEST(DupConfigKeyValVtnMapping, EmptyReqval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey;
  ConfigKeyVal *req=NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnMapping(okey, req));//Need to analyse
}

// Passing empty val to the function
TEST(DupConfigKeyValVtnMapping, EmptyConfigval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey;
  ConfigKeyVal *req=NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnMapping(okey, req));//Need to analyse
}

TEST(DupConfigKeyValVtnMapping, Configval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;

  key_vtn_controller_t *key = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st *val = (val_vtn_mapping_controller_st *)malloc(sizeof(val_vtn_mapping_controller_st));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyValVtnMapping(okey, ikey));
  delete okey;
  delete ikey;
}

// Passing empty val to the function
TEST(DupConfigKeyValVtnStation, EmptyReqval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;
  ConfigKeyVal *req=NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnStation(okey, req));//Need to analyse
  delete okey;
  delete req;
}

TEST(DupConfigKeyValVtnStation, Configval_kIpcStIpv4) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;

  key_vtn_controller_t *key = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  val_vtnstation_controller_st *val = (val_vtnstation_controller_st *)malloc(sizeof(val_vtnstation_controller_st));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStIpv4, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.DupConfigKeyValVtnStation(okey, ikey));
  delete okey;
  delete ikey;
}
TEST(DupConfigKeyValVtnStation, Configval_kIpcStIpv6) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;

  key_vtn_controller_t *key = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  val_vtnstation_controller_st *val = (val_vtnstation_controller_st *)malloc(sizeof(val_vtnstation_controller_st));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStIpv6, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.DupConfigKeyValVtnStation(okey, ikey));
  delete okey;
  delete ikey;
}

TEST(DupConfigKeyValVtnStation, okey_NOT_NULL) {
  VtnMoMgr vtn;

  key_vtn_controller_t *key = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  val_vtnstation_controller_st *val = (val_vtnstation_controller_st *)malloc(sizeof(val_vtnstation_controller_st));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStIpv4, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnStation(okey, ikey));
  delete ikey;
}

// Passing val1 as NULL to the function
TEST(FilterAttributes, val1Null) {
  VtnMoMgr vtn;
  void *val2 = NULL;
  val_vtn_t *valVtn2 = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  valVtn2->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  EXPECT_EQ(UNC_VF_VALID, valVtn2->valid[UPLL_IDX_DESC_VTN]);
}

// Passing val2 as NULL to the function
TEST(FilterAttributes, val2Null) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vtn_t *valVtn1 = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  valVtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn1->valid[UPLL_IDX_DESC_VTN]);
}

// Passing audit_status as false to the function
TEST(FilterAttributes, auditStatus) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = false;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vtn_t *valVtn1 = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  val_vtn_t *valVtn2 = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  valVtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn1->valid[UPLL_IDX_DESC_VTN]);
}

// Passing valid flag to the function
TEST(FilterAttributes, val1ValidFlag) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vtn_t *valVtn1 = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  val_vtn_t *valVtn2 = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  valVtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn1->valid[UPLL_IDX_DESC_VTN]);
}

// Passing valid flag with delete operation to the function
TEST(FilterAttributes, val2ValidFlag) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_DELETE;

  val_vtn_t *valVtn1 = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  val_vtn_t *valVtn2 = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  valVtn2->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn2->valid[UPLL_IDX_DESC_VTN]);
}

// Passing NULL to the function
TEST(ValidateVtnKey, nullVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = NULL;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnKey(keyVtn));
}

// Testing the vtn name
TEST(ValidateVtnKey, properVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyVtn->vtn_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnKey(keyVtn));
}

// Testing the vtn name with minimum value
TEST(ValidateVtnKey, minVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyVtn->vtn_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnKey(keyVtn));
}

// Testing the vtn name with maximum value
TEST(ValidateVtnKey, maxVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyVtn->vtn_name,(const char *)"vtnsddfkjlkssdklfjsdkladdassdd1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnKey(keyVtn));
}

// Testing the vtn name with maximum value exceeds
TEST(ValidateVtnKey, maxValExceeds) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyVtn->vtn_name,(const char *)"vtndfgdfddfrsdklfjsdklflsdsddfdfgdgfd1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnKey(keyVtn));
}

// Testing the vtn name with empty value
TEST(ValidateVtnKey, emptyVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyVtn->vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnKey(keyVtn));
}

/* ValidateVtnValue() */
// Passing NULL to the function
TEST(ValidateVtnValue, invalidVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  uint32_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnValue(valVtn, op)); //Bug 250
}

// Testing the vtn description
TEST(ValidateVtnValue, properVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"ashd l1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with minimum value
TEST(ValidateVtnValue, minVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with maximum value
TEST(ValidateVtnValue, maxVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"vtnsddfkjlkssdklfjsdkladdassdd1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with maximum value exceeds
TEST(ValidateVtnValue, maxValExceeds) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"vtndfgjjj;j;j;j;jjjjjjjjjdfddfrsdklfjsdklflsdsddfdfgdgfd1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with empty value
TEST(ValidateVtnValue, emptyVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with invalid flag
TEST(ValidateVtnValue, invalidFlag) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with no value flag
TEST(ValidateVtnValue, novalueFlagCreate) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID_NO_VALUE;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with no value flag
TEST(ValidateVtnValue, novalueFlagUpdate) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  uint32_t op = UNC_OP_UPDATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID_NO_VALUE;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}

// Testing the vtn description with invalid flag
TEST(ValidateVtnValue, novalueFlagDelete) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  uint32_t op = UNC_OP_DELETE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID_NO_VALUE;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
}


// Testing the vtn name
TEST(ValidateVtnRenameValue, properVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
}

// Testing the vtn name with minimum value
TEST(ValidateVtnRenameValue, minVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
}

// Testing the vtn name with maximum value
TEST(ValidateVtnRenameValue, maxVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"vtnsddfkjlkssdklfjsdkladdassdd1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
}

// Testing the vtn name with maximum value exceeds
TEST(ValidateVtnRenameValue, maxValExceeds) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"vtndfgdfddfrsdklfjsdklflsdsddfdfgdgfd1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnRenameValue(valVtnRename));
}

// Testing the vtn name with empty value
TEST(ValidateVtnRenameValue, emptyVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnRenameValue(valVtnRename));
}

// Testing the vtn name with invalid flag
TEST(ValidateVtnRenameValue, invalidFlag) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_INVALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
}


/* ValidateVtnMapCtrlrKey() */
// Passing NULL to the function
TEST(ValidateVtnMapCtrlrKey, nullVal) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = NULL;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the controller name and vtn name
TEST(ValidateVtnMapCtrlrKey, properVal) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the controller name with minimum value and proper vtn name
TEST(ValidateVtnMapCtrlrKey, ctrlNameMin) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the controller name with maximum value and proper vtn name
TEST(ValidateVtnMapCtrlrKey, ctrlNameMax) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"vtnstationcontrollersdklfjsdkl1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the controller name with maximum value exceeds and proper vtn name
TEST(ValidateVtnMapCtrlrKey, ctrlNameMaxExceeds) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"vtnstationcontrollersdklfjsdklflsdsddf1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the controller name with empty value and proper vtn name
TEST(ValidateVtnMapCtrlrKey, ctrlNameempty) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the vtn name with minimum value and proper controller name
TEST(ValidateVtnMapCtrlrKey, vtnNameMin) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the vtn name with maximum value and proper controller name
TEST(ValidateVtnMapCtrlrKey, vtnNameMax) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtnsdaflkjsdfhksdfgdghkshglkas1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the vtn name with maximum value exceeds and proper controller name
TEST(ValidateVtnMapCtrlrKey, vtnNameMaxExceeds) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtnsdaflkjsdfhksdfghkjasdghkshglkask1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the vtn name with empty value and proper controller name
TEST(ValidateVtnMapCtrlrKey, vtnNameEmpty) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = (key_vtn_controller_t *)malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

/* ValidateVtnStnCtrlrKey() */
// Passing NULL to the function
TEST(ValidateVtnStnCtrlrKey, nullVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = NULL;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

// Testing the controller name
TEST(ValidateVtnStnCtrlrKey, properVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)keyVtnStn->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

TEST(ValidateVtnStnCtrlrKey, UNC_OP_READ_SIBLING_COUNT) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)keyVtnStn->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_READ_SIBLING_COUNT));
}

// Testing the controller name with minimum value
TEST(ValidateVtnStnCtrlrKey, minVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)keyVtnStn->controller_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

// Testing the controller name with maximum value
TEST(ValidateVtnStnCtrlrKey, maxVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)keyVtnStn->controller_name,(const char *)"vtnstationcontrollhljhleouuuuuu");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

// Testing the controller name with maximum value exceeds
TEST(ValidateVtnStnCtrlrKey, maxValExceeds) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)keyVtnStn->controller_name,(const char *)"vtnstationcontrollersdklfjsdklflsdsddf1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

// Testing the controller name with empty value
TEST(ValidateVtnStnCtrlrKey, emptyVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)keyVtnStn->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

/* ReadMo() */
TEST(ReadMo, otherKT_detail) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//otherKT when option1 = UNC_OPT1_COUNT
TEST(ReadMo, otherKT_count) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong dt_type
TEST(ReadMo, Mapping_invaliddatatype) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping); 
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong st_num
TEST(ReadMo, Mapping_invalidstnum) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping); 
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no vtn_name
TEST(ReadMo, Mapping_emptyvtnname) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi)); //Bug raised already
  delete ikey;
}

TEST(ReadMo, Mapping_valid) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_mapping->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}
TEST(ReadMo, Mapping_invalid_01) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_mapping->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has domain
TEST(ReadMo, Mapping_withctrl_domain) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

TEST(ReadMo, Station_invalidkeytype) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has wrong st_num
TEST(ReadMo, Station_invalidstnum) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has no ctrlr_name
TEST(ReadMo, Station_emptyctrlrname) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi = new DalOdbcMgr();;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  strcpy((char*)key_station->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi)); //Bug raised already
  delete ikey;
}

TEST(ReadMo, Station_valid_01) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi = new DalOdbcMgr();;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  uuu::upll_strncpy(key_station->controller_name,"pfc1", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  uuu::upll_strncpy(val_station->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->port_name,"port_name", (kMaxLenCtrlrId + 1));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi)); //Bug has to raise
  delete ikey;
}

TEST(ReadMo, Station_valid_02) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi = new DalOdbcMgr();;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  uuu::upll_strncpy(key_station->controller_name,"pfc1", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  uuu::upll_strncpy(val_station->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->port_name,"port_name", (kMaxLenCtrlrId + 1));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}
TEST(ReadMo, Station_valid_03) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  uuu::upll_strncpy(key_station->controller_name,"pfc1", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  memset(val_station, 0, sizeof(val_vtnstation_controller_st_t));
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

TEST(ReadMo, mapping_valid_01) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}

TEST(ReadMo, mapping_valid_02) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}

TEST(ReadMo, mapping_valid_03) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_mapping->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}
TEST(ReadMo, mapping_valid_04) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  createControllerInfo("controller_name1",UPLL_DT_RUNNING,UNC_CT_PFC);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(ReadMo, mapping_valid_05) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
/* ReadSiblingMo() */
//otherKT when option1 = UNC_OPT1_DETAIL
TEST(ReadSiblingMo, KT_VTN_detail) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

TEST(ReadSiblingMo, KT_VBRIDGE) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vbr_t *keyvbr = (key_vbr_t *)malloc(sizeof(key_vbr_t));
  strcpy((char*)keyvbr->vbridge_name,(const char *)"vbr1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, keyvbr, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}
//otherKT when option1 = UNC_OPT1_COUNT
TEST(ReadSiblingMo, otherKT_count) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong dt_type
TEST(ReadSiblingMo, Mapping_invaliddatatype) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong st_num
TEST(ReadSiblingMo, Mapping_invalidstnum) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no vtn_name
TEST(ReadSiblingMo, Mapping_emptyvtnname) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already/
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no ctrlr name
TEST(ReadSiblingMo, Mapping_noctrlrname_nodomain) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}
//UNC_KT_VTNSTATION_CONTROLLER has wrong dt_type
TEST(ReadSiblingMo, station_invaliddatatype) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has wrong st_num
TEST(ReadSiblingMo, Station_invalidstnum) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has no ctrlr_name
TEST(ReadSiblingMo, Station_emptyctrlrname) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  strcpy((char*)key_station->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has invalid datatype
TEST(ReadSiblingMo, Station_invaliddatatype) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug has to raise
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has invalid operation
TEST(ReadSiblingMo, Station_invalidop) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug has to raise
  delete ikey;
}


//otherKT when option1 = UNC_OPT1_DETAIL
TEST(ReadSiblingMo, otherKT_detail_begin) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi = new DalOdbcMgr();;
  bool begin = true;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//otherKT when option1 = UNC_OPT1_COUNT
TEST(ReadSiblingMo, otherKT_count_begin) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong dt_type
TEST(ReadSiblingMo, Mapping_invaliddatatype_begin) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong st_num
TEST(ReadSiblingMo, Mapping_invalidstnum_begin) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no vtn_name
TEST(ReadSiblingMo, Mapping_emptyvtnname_begin) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already/
  delete ikey;
}

TEST(ReadSiblingMo, Mapping_01) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

TEST(ReadSiblingMo, Mapping_READ_SIBLING_BEGIN) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(ReadSiblingMo, Mapping_READ) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(ReadSiblingMo, station_READ_SIBLING_BEGIN) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has wrong st_num
TEST(ReadSiblingMo, Station_invalidstnum_begin) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi = new DalOdbcMgr();;
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

TEST(ReadSiblingMo, Station_READ_SIBLING_BEGIN_01) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc11");
  strcpy((char*)val_station->vtn_name,(const char *)"vtn1");
  strcpy((char*)val_station->domain_id,(const char *)"dom1");
  strcpy((char*)val_station->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_station->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_station->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_station->port_name,(const char *)"port1");

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

TEST(ReadSiblingMo, Station_READ_SIBLING_BEGIN_02) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  //unc_keytype_ctrtype_t ctrlrtype = UNC_CT_PFC;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc11");
  strcpy((char*)val_station->vtn_name,(const char *)"vtn1");
  strcpy((char*)val_station->domain_id,(const char *)"dom1");
  strcpy((char*)val_station->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_station->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_station->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_station->port_name,(const char *)"port1");

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

TEST(ReadSiblingMo, Station_READ_SIBLING_BEGIN_03) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc11");
  strcpy((char*)val_station->vtn_name,(const char *)"vtn1");
  strcpy((char*)val_station->domain_id,(const char *)"dom1");
  strcpy((char*)val_station->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_station->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_station->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_station->port_name,(const char *)"port1");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl("controller_name1",UNC_CT_VNP,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}
//UNC_KT_VTNSTATION_CONTROLLER has no ctrlr_name
TEST(ReadSiblingMo, Station_emptyctrlrname_begin) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  strcpy((char*)key_station->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has invalid operation
TEST(ReadSiblingMo, Station_invalidop_begin) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug has to raise
  delete ikey;
}


TEST(ReadSiblingCount, vtn) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi=new unc::upll::dal::DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *key_vtn = (key_vtn_t*) malloc(sizeof(key_vtn_t));
  val_vtn_t *val_vtn = (val_vtn_t*)malloc(sizeof(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_vtn, config_val);
  strcpy((char*)key_vtn->vtn_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingCount(req, ikey, dmi));
  delete ikey;
}
TEST(ReadSiblingCount, vtnStation_01) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi=new unc::upll::dal::DalOdbcMgr();

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  strcpy((char*)key_station->controller_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ReadSiblingCount(req, ikey, dmi));
  delete ikey;
}
/* MappingvExtTovBr() */

TEST(MappingvExtTovBr, vtnstation_02) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  strcpy((char*)val_station->vtn_name,(const char *)"vtn1");
  strcpy((char*)val_station->domain_id,(const char *)"dom1");
  strcpy((char*)val_station->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_station->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_station->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_station->port_name,(const char *)"port1");

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  val_station->map_type = UPLL_IF_VLAN_MAP;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
}
TEST(MappingvExtTovBr, vtnstation_03) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  uuu::upll_strncpy(val_station->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->port_name,"port_name", (kMaxLenCtrlrId + 1));
  val_station->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  val_station->valid[UPLL_IDX_VBR_NAME_VSCS] = UNC_VF_VALID;
  val_station->valid[UPLL_IDX_VBR_IF_NAME_VSCS] = UNC_VF_VALID;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  val_station->map_type = UPLL_IF_VLAN_MAP;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  val_station->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  val_station->valid[UPLL_IDX_VBR_IF_NAME_VSCS] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.MappingvExtTovBr(ikey, req, dmi, count));

  delete ikey;
}
TEST(MappingvExtTovBr, vtnstation_val_NULL) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));

  delete ikey;
}
TEST(MappingvExtTovBr, vtnstation_04) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  uuu::upll_strncpy(val_station->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->port_name,"port_name", (kMaxLenCtrlrId + 1));
  val_station->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  val_station->valid[UPLL_IDX_VBR_NAME_VSCS] = UNC_VF_VALID;
  val_station->valid[UPLL_IDX_VBR_IF_NAME_VSCS] = UNC_VF_VALID;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  val_station->map_type = UPLL_IF_VLAN_MAP;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  val_station->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  val_station->valid[UPLL_IDX_VBR_IF_NAME_VSCS] = UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.MappingvExtTovBr(ikey, req, dmi, count));

  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(MappingvExtTovBr, vtnstation_05) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  uuu::upll_strncpy(val_station->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->port_name,"port_name", (kMaxLenCtrlrId + 1));
  val_station->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  val_station->valid[UPLL_IDX_VBR_NAME_VSCS] = UNC_VF_VALID;
  val_station->valid[UPLL_IDX_VBR_IF_NAME_VSCS] = UNC_VF_VALID;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));

  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(MappingvExtTovBr, vtnstation_06) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  uuu::upll_strncpy(val_station->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_station->port_name,"port_name", (kMaxLenCtrlrId + 1));
  val_station->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  val_station->valid[UPLL_IDX_VBR_NAME_VSCS] = UNC_VF_VALID;
  val_station->valid[UPLL_IDX_VBR_IF_NAME_VSCS] = UNC_VF_VALID;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));

  delete ikey;
  DalOdbcMgr::clearStubData();
}
//when the key is UNC_KT_VTN_MAPPING_CONTROLLER and has invalid key st_num
TEST(MappingvExtTovBr, mapping_invalidkeynum) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
}

TEST(MappingvExtTovBr, ikey_No_value) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  ConfigKeyVal *ikey =NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
}
//when the key is UNC_KT_VTN_MAPPING_CONTROLLER and without setting vbr valid flag
TEST(MappingvExtTovBr, mapping_withoutvbrvalidflag) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
}

//when the key is UNC_KT_VTN_MAPPING_CONTROLLER and with setting vbr valid flag
TEST(MappingvExtTovBr, mapping_withoutvbrifvalidflag) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  val_mapping->valid[UPLL_IDX_VBR_NAME_VMCS] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
}
TEST(MappingvExtTovBr, mapping_withvbrifvalidflag) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_mapping->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  val_mapping->valid[UPLL_IDX_VBR_NAME_VMCS] = UNC_VF_VALID;
  val_mapping->valid[UPLL_IDX_VBR_IF_NAME_VMCS] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
}
TEST(MappingvExtTovBr, mapping_01) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_mapping->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  val_mapping->valid[UPLL_IDX_VBR_NAME_VMCS] = UNC_VF_VALID;
  val_mapping->valid[UPLL_IDX_VBR_IF_NAME_VMCS] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  val_mapping->map_type = UPLL_IF_VLAN_MAP;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(MappingvExtTovBr, mapping_02) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_mapping->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  val_mapping->valid[UPLL_IDX_VBR_NAME_VMCS] = UNC_VF_VALID;
  val_mapping->valid[UPLL_IDX_VBR_IF_NAME_VMCS] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(MappingvExtTovBr, mapping_03) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  strcpy((char*)val_mapping->switch_id,(const char *)"switch_id1");
  strcpy((char*)val_mapping->port_name,(const char *)"port_name1");
  strcpy((char*)val_mapping->vbr_name,(const char *)"vbr_name1");
  strcpy((char*)val_mapping->vbrif_name,(const char *)"vbrif_name1");
  strcpy((char*)val_mapping->logical_port_id,(const char *)"logical_port_id1");
  val_mapping->valid[UPLL_IDX_VBR_NAME_VMCS] = UNC_VF_VALID;
  val_mapping->valid[UPLL_IDX_VBR_IF_NAME_VMCS] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
  DalOdbcMgr::clearStubData();
}

/* ValidateMessageForVtnStnCtrlr */
//when the configkeyval is null
TEST(ValidateMessageForVtnStnCtrlr, ikeynull) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateMessageForVtnStnCtrlr(req, ikey)); // Code has the fix now
  delete ikey;
}

//when the IpcReqRespHeader is null
TEST(ValidateMessageForVtnStnCtrlr, reqnull) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = NULL;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateMessageForVtnStnCtrlr(req, ikey)); //Bug 404
  delete ikey;
}

//when invalid st_num
TEST(ValidateMessageForVtnStnCtrlr, invalid_stnum) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
  delete ikey;
}

TEST(ValidateMessageForVtnStnCtrlr, Invalid_option1) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1=(unc_keytype_option1_t)3;
  req->option2 = UNC_OPT2_NEIGHBOR;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
  delete ikey;
}

TEST(ValidateMessageForVtnStnCtrlr, Invalid_option2) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;

  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
  delete ikey;
}

//when valid stnum with ctrlr name
TEST(ValidateMessageForVtnStnCtrlr, valid_stnum_ctrlname) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  req->datatype = UPLL_DT_STATE;
  key_vtnstation_controller_t *key_station = (key_vtnstation_controller_t*) malloc(sizeof(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station = (val_vtnstation_controller_st_t*)malloc(sizeof(val_vtnstation_controller_st_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
  delete ikey;
}

//when the configkeyval is null
TEST(ValidateMessageForVtnMapCtrlr, ikeynull) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateMessageForVtnMapCtrlr(req, ikey)); // Code has the fix now
  delete ikey;
}

//when the IpcReqRespHeader is null
TEST(ValidateMessageForVtnMapCtrlr, reqnull) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = NULL;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateMessageForVtnMapCtrlr(req, ikey)); // Code has the fix now
  delete ikey;
}

//when invalid st_num
TEST(ValidateMessageForVtnMapCtrlr, invalid_stnum) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when valid stnum with empty ctrlr name and vtn name
TEST(ValidateMessageForVtnMapCtrlr, valid_stnum_emptyctrlvtnname) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when valid stnum with empty ctrlr name
TEST(ValidateMessageForVtnMapCtrlr, valid_stnum_emptyctrlname) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when valid stnum with ctrlr name
TEST(ValidateMessageForVtnMapCtrlr, valid_stnum_ctrlname) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when valid stnum with empty vtn name
TEST(ValidateMessageForVtnMapCtrlr, valid_stnum_emptyvtnname) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

TEST(ValidateMessageForVtnMapCtrlr, invalid_option1) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

TEST(ValidateMessageForVtnMapCtrlr, invalid_option2) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when invalid datatype
TEST(ValidateMessageForVtnMapCtrlr, invalid_datatype) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when invalid operation
TEST(ValidateMessageForVtnMapCtrlr, invalid_operation) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr = (key_vtn_controller_t*) malloc(sizeof(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping = (val_vtn_mapping_controller_st_t*)malloc(sizeof(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

/* DupConfigKeyVal */

TEST(DupConfigKeyVal, nullkey) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = MAINTBL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

TEST(DupConfigKeyVal, nonullokey) {
  VtnMoMgr vtn;
  MoMgrTables tbl=MAINTBL;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
}

TEST(DupConfigKeyVal, novtnKT) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl=MAINTBL;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

TEST(DupConfigKeyVal, MAINTBL) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = MAINTBL;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

TEST(DupConfigKeyVal, MAINTBL_withST) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = MAINTBL;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  val_vtn_st_t *val_vtnst = (val_vtn_st_t *)malloc(sizeof(val_vtn_st_t));
  ConfigVal *config_valst = new ConfigVal(IpctSt::kIpcStValVtnSt, val_vtnst);
  config_val->AppendCfgVal(config_valst);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

TEST(DupConfigKeyVal, RENAMETBL) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = RENAMETBL;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_rename_vtn_t *val_rename_vtn = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  ConfigVal *rename_val = new ConfigVal(IpctSt::kIpcStValRenameVtn, val_rename_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStValRenameVtn, keyvtn, rename_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

//Negative test case
TEST(DupConfigKeyVal, CTRLRTBL) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = CTRLRTBL;

  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

/* MergeValidateChildren */

TEST(MergeValidateChildren, nullikey) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  const char *ctrlr_id = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MergeValidateChildren(ikey, ctrlr_id, ikey, dmi));
}
TEST(MergeValidateChildren, ikey_VTN) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  const char *ctrlr_id = "pfc1";
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MergeValidateChildren(ikey, ctrlr_id, ikey, dmi));
}
TEST(MergeValidateChildren, ikey_UNC_KT_VBRIDGE) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  const char *ctrlr_id = "pfc1";
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MergeValidateChildren(ikey, ctrlr_id, ikey, dmi));
}

TEST(AdaptValToVtnService, ikey_NULL) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AdaptValToVtnService(ikey));
}
TEST(AdaptValToVtnService, ikey) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);

  val_vtn_st *valst;
  valst = reinterpret_cast<val_vtn_st *>(malloc
  (sizeof(val_vtn_st)));

  memset(valst,0,sizeof(val_vtn_st));
  valst->valid[0] = UNC_VF_VALID;
  valst->oper_status = UPLL_OPER_STATUS_UP;


  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, valst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AdaptValToVtnService(ikey));
  delete ikey;
}
TEST(AdaptValToVtnService, ConfigVal_NULL) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);

  val_vtn_st *valst;
  valst = reinterpret_cast<val_vtn_st *>(malloc
  (sizeof(val_vtn_st)));

  memset(valst,0,sizeof(val_vtn_st));
  valst->valid[0] = UNC_VF_VALID;
  valst->oper_status = UPLL_OPER_STATUS_UP;


  ConfigVal *config_val= NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AdaptValToVtnService(ikey));
  delete ikey;
}

TEST(GetControllerDomainSpan, ikey) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi= new DalOdbcMgr();
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;
  std::list<controller_domain_t> list_ctrlr_dom;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetControllerDomainSpan(ikey,dtType,dmi,list_ctrlr_dom));
  delete ikey;
}
TEST(GetControllerDomainSpan, datatype_STATE) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi= new DalOdbcMgr();
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  std::list<controller_domain_t> list_ctrlr_dom;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetControllerDomainSpan(ikey,dt_type,dmi,list_ctrlr_dom));
  delete ikey;
}

TEST(GetControllerDomainSpan, ikey_01) {

  VtnMoMgr vtn;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;
  std::list<controller_domain_t> list_ctrlr_dom;
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.GetControllerDomainSpan(ikey,dtType,dmi,list_ctrlr_dom));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetControllerDomainSpan, ikey_02) {

  DalDmlIntf *dmi= new DalOdbcMgr();
  VtnMoMgr vtn;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;
  std::list<controller_domain_t> list_ctrlr_dom;
  key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
  uuu::upll_strncpy(user_data->ctrlr_id,"ctr_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetControllerDomainSpan(ikey,dtType,dmi,list_ctrlr_dom));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetControllerDomainSpan,default ) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi= new DalOdbcMgr();
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetControllerDomainSpan(ikey,dt_type,dmi));
  delete ikey;
}
TEST(ReadSingleCtlrlStation, ikey_NULL) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
}
TEST(ReadSingleCtlrlStation, ikey_proper) {

  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
}
TEST(ReadSingleCtlrlStation, val_vtnstation_valid) {

  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi= new DalOdbcMgr();

  key_vtnstation_controller_t *keyvtnsta = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));

 uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st *valVtnsta = (val_vtnstation_controller_st *)malloc(sizeof(val_vtnstation_controller_st));

ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, valVtnsta);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, keyvtnsta, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
}
TEST(ReadSingleCtlrlStation, val_vtnstation_ALL_VALID_01) {

  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi=new unc::upll::dal::DalOdbcMgr();

  key_vtnstation_controller_t *keyvtnsta = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));

  uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st *valVtnsta = (val_vtnstation_controller_st *)malloc(sizeof(val_vtnstation_controller_st));

  uuu::upll_strncpy(valVtnsta->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(valVtnsta->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(valVtnsta->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  valVtnsta->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  valVtnsta->valid[UPLL_IDX_VBR_NAME_VSCS] = UNC_VF_VALID;
  valVtnsta->valid[UPLL_IDX_VBR_IF_NAME_VSCS] = UNC_VF_VALID;
ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, valVtnsta);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, keyvtnsta, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
}
TEST(ReadSingleCtlrlStation, val_vtnstation_ALL_VALID_02) {

  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi=new unc::upll::dal::DalOdbcMgr();

  key_vtnstation_controller_t *keyvtnsta = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));

  uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st *valVtnsta = (val_vtnstation_controller_st *)malloc(sizeof(val_vtnstation_controller_st));

  uuu::upll_strncpy(valVtnsta->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(valVtnsta->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(valVtnsta->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  valVtnsta->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  valVtnsta->valid[UPLL_IDX_VBR_NAME_VSCS] = UNC_VF_VALID;
  valVtnsta->valid[UPLL_IDX_VBR_IF_NAME_VSCS] = UNC_VF_VALID;
ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, valVtnsta);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, keyvtnsta, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(ReadSingleCtlrlStation, vtn_valid_01) {

  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi=new unc::upll::dal::DalOdbcMgr();

  key_vtnstation_controller_t *keyvtnsta = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));
  uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st *valVtnsta = (val_vtnstation_controller_st *)malloc(sizeof(val_vtnstation_controller_st));

  uuu::upll_strncpy(valVtnsta->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  valVtnsta->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  uuu::upll_strncpy(valVtnsta->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(valVtnsta->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, valVtnsta);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, keyvtnsta, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
}
TEST(ReadSingleCtlrlStation, vtn_valid_02) {

  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi=new unc::upll::dal::DalOdbcMgr();

  key_vtnstation_controller_t *keyvtnsta = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));
  uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st *valVtnsta = (val_vtnstation_controller_st *)malloc(sizeof(val_vtnstation_controller_st));

  uuu::upll_strncpy(valVtnsta->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(valVtnsta->domain_id,"domain_id1", (kMaxLenCtrlrId + 1));
  valVtnsta->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  valVtnsta->valid[UPLL_IDX_DOMAIN_ID_VSCS] = UNC_VF_VALID;
  uuu::upll_strncpy(valVtnsta->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(valVtnsta->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, valVtnsta);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, keyvtnsta, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
}
TEST(ReadSingleCtlrlStation, vtn_valid_03) {

  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi=new unc::upll::dal::DalOdbcMgr();

  key_vtnstation_controller_t *keyvtnsta = (key_vtnstation_controller_t *)malloc(sizeof(key_vtnstation_controller_t));
  uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st *valVtnsta = (val_vtnstation_controller_st *)malloc(sizeof(val_vtnstation_controller_st));

  uuu::upll_strncpy(valVtnsta->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(valVtnsta->domain_id,"domain_id1", (kMaxLenCtrlrId + 1));
  valVtnsta->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  valVtnsta->valid[UPLL_IDX_DOMAIN_ID_VSCS] = UNC_VF_VALID;
  uuu::upll_strncpy(valVtnsta->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(valVtnsta->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, valVtnsta);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, keyvtnsta, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(ReadSingleCtlrlVtnMapping, ikey) {

  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  uint32_t ckv_count1=1;
  uint32_t *ckv_count=&ckv_count1;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi=new unc::upll::dal::DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlVtnMapping(req,ikey,dmi,ckv_count));
  delete ikey;
}
TEST(ReadSingleCtlrlVtnMapping, ikey_Proper) {

  VtnMoMgr vtn;
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  uint32_t ckv_count1=1;
  uint32_t *ckv_count=&ckv_count1;
  req->rep_count = 2;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi=new unc::upll::dal::DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlVtnMapping(req,ikey,dmi,ckv_count));
  delete ikey;
}

TEST(ReadSingleCtlrlVtnMapping, ikey_Proper_03) {

  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);
  IpcReqRespHeader *req = reinterpret_cast<IpcReqRespHeader*>(malloc(sizeof(IpcReqRespHeader)));
  uint32_t ckv_count1=1;
  uint32_t *ckv_count=&ckv_count1;
  req->rep_count = 2;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi=new unc::upll::dal::DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSingleCtlrlVtnMapping(req,ikey,dmi,ckv_count));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(MergeValidate, MergeValidate_Error) {

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  const char *ctrlr_id = "Controller1";
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.MergeValidate(UNC_KT_VTN, ctrlr_id,
                                              ikey, dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(MergeValidate, MergeValidate_success) {

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  VtnMoMgr vtn;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  const char *ctrlr_id = "Controller1";
  GetKeyValStruct(key,val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.MergeValidate(UNC_KT_VTN, ctrlr_id,
                                              ikey, dmi));
  DalOdbcMgr::clearStubData();
  delete ikey;
}
TEST(ControllerStatusHandler, ControllerStatusHandler_01) {
  bool operstatus=true;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();;

  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.ControllerStatusHandler( ctr_id1,
                                              dmi, operstatus));
  DalOdbcMgr::clearStubData();
  delete ikey;
}
TEST(ControllerStatusHandler, ControllerStatusHandler_02) {
  bool operstatus=true;
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();;
  uint8_t *ctrlr_id=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.ControllerStatusHandler( ctrlr_id,
                                              dmi, operstatus));
  delete ikey;
}
TEST(ControllerStatusHandler, ControllerStatusHandler_03) {
  bool operstatus=true;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();;
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.ControllerStatusHandler( ctr_id1,
                                              dmi, operstatus));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(IsReferenced, IsReferenced_01) {

  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsReferenced(ikey,dt_type,dmi));
  delete ikey;
}
TEST(IsReferenced, IsReferenced_02) {

  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsReferenced(ikey,dt_type,dmi));
  delete ikey;
}
TEST(IsReferenced, IsReferenced_03) {

  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi=new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_CFG_SEMANTIC, vtn.IsReferenced(ikey,dt_type,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}

TEST(ControllerStatusHandler, ControllerStatusHandler_04) {
  bool operstatus=true;
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();;
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.ControllerStatusHandler( ctr_id1,
                                              dmi, operstatus));
  delete ikey;
}
TEST(UpdateOperStatus, UpdateOperStatus_01) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();;
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateOperStatus( ikey,
                                              dmi, notification, true));
  delete ikey;
}
TEST(UpdateOperStatus, UpdateOperStatus_02) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  state_notification notification=kPortFaultReset;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();;
  key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
  uuu::upll_strncpy(user_data->ctrlr_id,"Controller1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateOperStatus( ikey,
                                              dmi, notification, true));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(UpdateOperStatus, UpdateOperStatus_03) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  state_notification notification=kPortFaultReset;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();;
  key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
  uuu::upll_strncpy(user_data->ctrlr_id,"Controller1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE,vtn.UpdateOperStatus( ikey, dmi, notification, true));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetRenamedUncKey, GetRenamedUncKey_01) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetRenamedUncKey(ikey,dt_type,dmi,ctr_id1));
  delete ikey;
  DalOdbcMgr::clearStubData();
}

TEST(GetRenamedUncKey, GetRenamedUncKey_02) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedUncKey(ikey,dt_type,dmi,ctr_id1));
  delete ikey;
}
TEST(GetRenamedUncKey, GetRenamedUncKey_03) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetRenamedUncKey(ikey,dt_type,dmi,ctr_id1));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetRenamedUncKey, GetRenamedUncKey_04) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedUncKey(ikey,dt_type,dmi,ctr_id1));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetRenamedControllerKey, GetRenamedControllerKey_01) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetRenamedControllerKey(ikey,dt_type,dmi,ctrlr_dom));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetRenamedControllerKey, GetRenamedControllerKey_02) {
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetRenamedControllerKey(ikey,dt_type,dmi,ctrlr_dom));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetRenamedControllerKey, GetRenamedControllerKey_03) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.GetRenamedControllerKey(ikey,dt_type,dmi,ctrlr_dom));
  delete ikey;
  DalOdbcMgr::clearStubData();
}

TEST(GetRenamedControllerKey, GetRenamedControllerKey_05) {
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain *ctrlr_dom = reinterpret_cast<controller_domain_t *>(malloc(sizeof(controller_domain_t)));
  ctrlr_dom->ctrlr = ctr_id1;
  ctrlr_dom->domain = dom_id1;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedControllerKey(ikey,dt_type,dmi,ctrlr_dom));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetRenamedControllerKey, GetRenamedControllerKey_04) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;
  DalDmlIntf *dmi = new DalOdbcMgr();
  controller_domain *ctrlr_dom=NULL;
  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedControllerKey(ikey,dt_type,dmi,ctrlr_dom));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetRenamedControllerKey, ikey_NULL) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *valVtn = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctr_id1;
  ctrlr_dom.domain = dom_id1;
  controller_domain ctrlr_dom1;
  ctrlr_dom1.ctrlr = ctr_id1;
  ctrlr_dom1.domain = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedControllerKey(ikey,dt_type,dmi,&ctrlr_dom1));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(GetRenamedControllerKey, ikey_01) {
  DalOdbcMgr::clearStubData();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VtnMoMgr vtn;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, NULL, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = (uint8_t *)malloc(32);
  memset(dom_id1, '\0', 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctr_id1;
  ctrlr_dom.domain = dom_id1;
  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedControllerKey(ikey,dt_type,dmi,&ctrlr_dom));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(UpdateVtnConfigStatus, UpdateVtnConfigStatus_Update) {
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;

  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            keyvtn, config_val);
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_UPDATE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
}
TEST(UpdateVtnConfigStatus, UpdateVtnConfigStatus_Update_01) {

  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            keyvtn, config_val);
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_UPDATE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(UpdateVtnConfigStatus, UpdateVtnConfigStatus_Create) {

  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            keyvtn, config_val);
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_CREATE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
}
TEST(UpdateVtnConfigStatus, UpdateVtnConfigStatus_DElete) {

  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            keyvtn, config_val);
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_DELETE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
}
TEST(UpdateVtnConfigStatus, UpdateVtnConfigStatus_Read) {

  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            keyvtn, config_val);
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_READ,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
}
TEST(UpdateVtnConfigStatus, Update_ikey_Err) {

  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();
  ConfigKeyVal *upd_key = NULL;
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_UPDATE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
}
TEST(UpdateVtnConfigStatus, Update_Val_NULL) {

  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();
  ConfigKeyVal *upd_key = NULL;
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_UPDATE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
}
TEST(UpdateVtnConfigStatus, Delete_ikey_Err) {

  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi = new DalOdbcMgr();
  ConfigKeyVal *upd_key = NULL;
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_DELETE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
}

TEST(CopyToConfigkey, CopyToConfigkey_ikeyokeyNull) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.CopyToConfigKey(okey,ikey));

  ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.CopyToConfigKey(okey,ikey));
  delete ikey;
  delete okey;
}

TEST(CopyToConfigkey, CopyToConfigkey_01) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            keyvtn, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.CopyToConfigKey(okey,ikey));
  delete ikey;
  delete okey;
}
TEST(CopyToConfigkey, CopyToConfigkey_02) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            keyvtn, config_val);
  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  uuu::upll_strncpy(keyvtn->vtn_name, key_rename->old_unc_vtn_name,(kMaxLenVtnName + 1));


  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.CopyToConfigKey(okey,ikey));
  delete ikey;
  delete okey;
}
TEST(UpdateConfigStatus, UpdateConfigStatus_Create) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn *key;
  val_vtn *val;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  ConfigKeyVal *ctrlr_run = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ctrlr_run));
  delete ikey;
}
TEST(UpdateConfigStatus, UpdateConfigStatus_ValVTN_NULL) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn *key;
  key = reinterpret_cast<key_vtn *>(malloc
                 (sizeof(key_vtn)));
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  val_vtn *val=NULL;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
  delete ikey;
}
TEST(UpdateConfigStatus, UpdateConfigStatus_ConfigKeyVal_NULL) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn *key;
  val_vtn *val;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  ConfigKeyVal *upd_key = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
  delete ikey;
}
TEST(UpdateConfigStatus, UpdateConfigStatus_PFC) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
  delete ikey;
}

TEST(UpdateConfigStatus, OP_Update_01) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  key_vtn_controller_t *key_ctrlr = reinterpret_cast<key_vtn_controller_t *>(malloc
                 (sizeof(key_vtn_controller_t)));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr = reinterpret_cast<val_vtn_ctrlr_t *>(malloc
                 (sizeof(val_vtn_ctrlr_t)));
  val_ctrlr->ref_count=1;
  val_ctrlr->down_count=1;

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVtn, val_ctrlr);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtnController,
                            key_ctrlr, cfgval1);
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl("controller_name1",UNC_CT_PFC,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_CANDIDATE));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
  uuu::upll_strncpy(user_data->ctrlr_id,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  upd_key->set_user_data((void*)user_data);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,upd_key));
  delete ikey;
}
TEST(UpdateConfigStatus, OP_Update_02) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  key_vtn_controller_t *key_ctrlr = reinterpret_cast<key_vtn_controller_t *>(malloc
                 (sizeof(key_vtn_controller_t)));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr = reinterpret_cast<val_vtn_ctrlr_t *>(malloc
                 (sizeof(val_vtn_ctrlr_t)));
  val_ctrlr->ref_count=3;
  val_ctrlr->down_count=1;

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVtn, val_ctrlr);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtnController,
                            key_ctrlr, cfgval1);
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl("controller_name1",UNC_CT_PFC,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_CANDIDATE));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
  uuu::upll_strncpy(user_data->ctrlr_id,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  upd_key->set_user_data((void*)user_data);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);
  val_vtn_ctrlr_t *val_ctrlr1 = reinterpret_cast<val_vtn_ctrlr_t *>(malloc
                 (sizeof(val_vtn_ctrlr_t)));
  val_ctrlr1->ref_count=1;
  val_ctrlr1->down_count=0;
  ConfigVal *cfgval2 = new ConfigVal(IpctSt::kIpcStValVtn, val_ctrlr1);
  ConfigKeyVal *upd_key1 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtnController,
                            key_ctrlr, cfgval2);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,upd_key1));
  delete ikey;
}
TEST(UpdateConfigStatus, UpdateConfigStatus_invalidOP) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_READ,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
  delete ikey;
}
TEST(UpdateConfigStatus, UpdateConfigStatus_OP_Create) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  key_vtn_controller_t *key_ctrlr = reinterpret_cast<key_vtn_controller_t *>(malloc
                 (sizeof(key_vtn_controller_t)));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr = reinterpret_cast<val_vtn_ctrlr_t *>(malloc
                 (sizeof(val_vtn_ctrlr_t)));
  val_ctrlr->ref_count=1;
  val_ctrlr->down_count=1;

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVtn, val_ctrlr);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtnController,
                            key_ctrlr, cfgval1);
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl("controller_name1",UNC_CT_PFC,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_CANDIDATE));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
  uuu::upll_strncpy(user_data->ctrlr_id,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  upd_key->set_user_data((void*)user_data);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,upd_key));
  delete ikey;
}
TEST(UpdateConfigStatus, UpdateConfigStatus_OP_Create_01) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);
  val->cs_row_status=UNC_CS_NOT_APPLIED;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  key_vtn_controller_t *key_ctrlr = reinterpret_cast<key_vtn_controller_t *>(malloc
                 (sizeof(key_vtn_controller_t)));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr = reinterpret_cast<val_vtn_ctrlr_t *>(malloc
                 (sizeof(val_vtn_ctrlr_t)));
  val_ctrlr->ref_count=1;
  val_ctrlr->down_count=1;

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfgval1 = new ConfigVal(IpctSt::kIpcStValVtn, val_ctrlr);
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtnController,
                            key_ctrlr, cfgval1);
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl("controller_name1",UNC_CT_PFC,version);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl,UPLL_DT_CANDIDATE));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  key_user_data_t *user_data = reinterpret_cast<key_user_data_t *>(malloc(sizeof(key_user_data_t)));
  uuu::upll_strncpy(user_data->ctrlr_id,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  upd_key->set_user_data((void*)user_data);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,upd_key));
  val->cs_row_status=UNC_CS_PARTIALLY_APPLIED;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,upd_key));
  val->cs_row_status=UNC_CS_INVALID;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,upd_key));
  val->cs_row_status=UNC_CS_UNKNOWN;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,upd_key));
  val->cs_row_status=UNC_CS_NOT_SUPPORTED;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,upd_key));
  delete ikey;
}
TEST(ValidateCapability, ValidateCapability_Success) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);


  CtrlrMgr::Ctrlr ctrlrobj("CTR_1", UNC_CT_PFC, "5.0");
  CtrlrMgr::GetInstance()->Add(ctrlrobj, UPLL_DT_CANDIDATE);

  uint8_t attrs[3];
  attrs[unc::capa::vtn::kCapDesc] = 1;

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vtn.ValidateCapability(req, ikey, "CTR_1"));
  val_vbr* no_val = NULL;
  cfgval = new ConfigVal(IpctSt::kIpcStValVbr, no_val);
  ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, vtn.ValidateCapability(req, ikey, "CTR_1"));

  req->operation = UNC_OP_DELETE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateCapability(req, ikey, "CTR_1"));

  CtrlrMgr::GetInstance()->Delete("CTR_1", UPLL_DT_CANDIDATE);
  delete ikey;
}
TEST(ValidateCapability, ValidateCapability_ikey_NULL) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req=NULL;
  const char *ctrlr_name="ctr1";
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateCapability(req, ikey, ctrlr_name));
}
TEST(ValidateCapability, ValidateCapability_ctrName_NULL) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));

  const char *ctrlr_name=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateCapability(req, ikey, ctrlr_name));
}
TEST(GetChildConfigKey, GetChildConfigKey_SuccessNullObjs) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,vtn.GetChildConfigKey(okey, pkey));
  delete okey;
  delete pkey;
}
TEST(GetChildConfigKey, GetChildConfigKey_pkeyNull) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.GetChildConfigKey(okey, pkey));
  delete okey;
  delete pkey;
}
TEST(GetChildConfigKey, pkeyNull_okey_NotNull) {
  VtnMoMgr vtn;
  key_vbr_t *key = reinterpret_cast<key_vbr_t *>
                  (malloc(sizeof(key_vbr_t)));
  memset(key, 0 ,sizeof(key_vtn_t));

  strncpy((char*) key->vbridge_name,"VTN1",32);

  ConfigKeyVal *pkey = NULL;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,key,NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,vtn.GetChildConfigKey(okey, pkey));
  delete okey;
}
TEST(GetChildConfigKey, GetChildConfigKey_Vtn) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  key_vtn_t *key = reinterpret_cast<key_vtn_t *>
                  (malloc(sizeof(key_vtn_t)));
  memset(key, 0 ,sizeof(key_vtn_t));

  strncpy((char*) key->vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,vtn.GetChildConfigKey(okey, pkey));
  key_vtn_t *output = reinterpret_cast<key_vtn_t *> (okey->get_key());

  EXPECT_STREQ("VTN1",(reinterpret_cast<const char *> (output->vtn_name)));
  delete okey;
  delete pkey;
}
TEST(GetChildConfigKey, okey_Not_NULL) {
  VtnMoMgr vtn;
  key_vtn_t *key = reinterpret_cast<key_vtn_t *>
                  (malloc(sizeof(key_vtn_t)));
  memset(key, 0 ,sizeof(key_vtn_t));

  strncpy((char*) key->vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, NULL);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,vtn.GetChildConfigKey(okey, pkey));

  delete okey;
}
TEST(GetChildConfigKey, GetChildConfigKey_Vbr) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  key_vtn_t *key = reinterpret_cast<key_vtn_t *>
                  (malloc(sizeof(key_vtn_t)));
  memset(key, 0 ,sizeof(key_vtn_t));

  strncpy((char*) key->vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVtn,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.GetChildConfigKey(okey, pkey));
  delete okey;
  delete pkey;
}
TEST(IsKeyInUse, ikey_Error) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);
  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  DalDmlIntf *dmi=new DalOdbcMgr();
  bool in_use;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsKeyInUse(dt_type,ikey,&in_use,dmi));
  delete ikey;
}
TEST(IsKeyInUse, invalid_ctrlrName) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  const char *controller_name = "";
  GetKeyValStruct(key, val);
  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  DalDmlIntf *dmi=new DalOdbcMgr();
  bool in_use;
  key = reinterpret_cast<key_vtn *>(malloc
                 (sizeof(key_vtn)));
  memset(key,0,sizeof(key_vtn));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  key_ctr_t *key_ctr =
           reinterpret_cast<key_ctr_t *>(ikey->get_key());
  memset(key_ctr,0,sizeof(key_ctr));
  strncpy(reinterpret_cast<char *>(key_ctr->controller_name),
  controller_name, strlen(controller_name)+1);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsKeyInUse(dt_type,ikey,&in_use,dmi));
  delete ikey;
}

TEST(IsKeyInUse, ikey_NULL) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);
  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  DalDmlIntf *dmi=new DalOdbcMgr();
  bool in_use;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsKeyInUse(dt_type,ikey,&in_use,dmi));
  delete ikey;
}
TEST(ValidateMessage, ValidateMessage_01) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_CREATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey));

  ConfigVal *cfgval2 = NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval2);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey2));
  val_rename_vtn *renameval;
  renameval = reinterpret_cast<val_rename_vtn *>(malloc
  (sizeof(val_rename_vbr)));
  memset(renameval,0,sizeof(val_rename_vtn));
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

  ConfigVal *cfgval3 = NULL;
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval3);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessage(req, ikey3));

  val_rename_vtn *vtn_rename =NULL;
  ConfigVal *cfgval4 = new ConfigVal(IpctSt::kIpcStValVtn, vtn_rename);
  ConfigKeyVal *ikey4 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval4);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey4));

  req->operation = UNC_OP_READ_SIBLING;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, rename_ikey));

  ConfigVal *invrename_cfgval = new ConfigVal(IpctSt::kIpcStValRenameVtn, NULL);
  ConfigKeyVal *invrename_ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, invrename_cfgval);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, invrename_ikey));

  const char *vtn_name1 = " ";
  key_vtn *key5;
  val_vtn *val5;
  key5 = reinterpret_cast<key_vtn *>(malloc
                 (sizeof(key_vtn)));
  memset(key5,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key5->vtn_name),
  vtn_name1, strlen(vtn_name1)+1);
  val5 = reinterpret_cast<val_vtn *>(malloc
  (sizeof(val_vtn)));
  memset(val5,0,sizeof(val_vtn));
  ConfigVal *cfgval5 = new ConfigVal(IpctSt::kIpcStValVtn, val5);
  ConfigKeyVal *ikey5 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key5, cfgval5);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessage(req, ikey5));

  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_RUNNING;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey));


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

}

TEST(ValidateMessage, ValidateMessage_02) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
  req->clnt_sess_id = 5;
  req->config_id = 14;
  req->operation = UNC_OP_UPDATE;
  req->rep_count = 100;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  req->datatype = UPLL_DT_CANDIDATE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey));
  req->operation = UNC_OP_CONTROL;
  ConfigKeyVal *ikey4 = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
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

}
TEST(ValidateMessage, ValidateMessage_03) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
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

}
TEST(ValidateMessage, ValidateMessage_04) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

  IpcReqRespHeader *req;
  req = reinterpret_cast<IpcReqRespHeader *>(malloc
                 (sizeof(IpcReqRespHeader)));
  memset(req,0,sizeof(IpcReqRespHeader));
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
}
TEST(ValVtnAttributeSupportCheck, ValVtnAttributeSupportCheck_01) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);
  uint32_t operation= UNC_OP_CREATE;
  uint8_t ckv_count1=1;
  uint8_t *attrs=&ckv_count1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValVtnAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}
TEST(ValVtnAttributeSupportCheck, ValVtnAttributeSupportCheck_02) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);
  uint32_t operation= UNC_OP_CREATE;
  val->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  uint8_t ckv_count1=1;
  uint8_t *attrs=&ckv_count1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValVtnAttributeSupportCheck(val,attrs,operation));
  delete ikey;
}
TEST(SetOperStatus, ikey_NULL) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  DalDmlIntf *dmi= new DalOdbcMgr();

  GetKeyValStruct(key, val);
  state_notification notification=kCtrlrReconnect;
  ConfigKeyVal *ikey =NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetOperStatus(ikey,notification,dmi));
  delete ikey;
}
TEST(SetConsolidatedStatus, invalid_key) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_db_vtn_st_t *val = (val_db_vtn_st_t *)malloc(sizeof(val_db_vtn_st_t));
  memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val->down_count=0;
  DalDmlIntf *dmi= new DalOdbcMgr();

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
  delete ikey;
}
TEST(SetConsolidatedStatus, valid_key) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_db_vtn_st_t *val = (val_db_vtn_st_t *)malloc(sizeof(val_db_vtn_st_t));
  memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val->down_count=0;
  DalDmlIntf *dmi= new DalOdbcMgr();

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
  delete ikey;
}
TEST(SetOperStatus, notification_kCtrlrReconnectIfUp) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_db_vtn_st_t *val = (val_db_vtn_st_t *)malloc(sizeof(val_db_vtn_st_t));
  memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val->down_count=0;
  DalDmlIntf *dmi= new DalOdbcMgr();

  state_notification notification=kCtrlrReconnectIfUp;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetOperStatus(ikey,notification,dmi));
  delete ikey;
}
TEST(SetOperStatus, notification_kCtrlrDisconnect) {

  const char *vtn_name = "VTN_1";
  VtnMoMgr vtn;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_db_vtn_st_t *val = (val_db_vtn_st_t *)malloc(sizeof(val_db_vtn_st_t));
  memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val->down_count=0;
  DalDmlIntf *dmi= new DalOdbcMgr();

  state_notification notification=kCtrlrDisconnect;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetOperStatus(ikey,notification,dmi));
  delete ikey;
}
TEST(SetCtrlrOperStatus, ikey_NULL) {

  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  DalDmlIntf *dmi= new DalOdbcMgr();

  GetKeyValStruct(key, val);
  state_notification notification=kCtrlrReconnect;
  ConfigKeyVal *ikey =NULL;
  bool oper_change;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));
  delete ikey;
}
TEST(SetCtrlrOperStatus, kCtrlrReconnectIfUp) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key = (key_vtn_controller *)malloc(sizeof(key_vtn_controller));
  memset(key,0,sizeof(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();

  state_notification notification=kCtrlrReconnectIfUp;
  bool oper_change;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));

  val_vtn->oper_status = UPLL_OPER_STATUS_DOWN;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));
  delete ikey;
}

TEST(SetCtrlrOperStatus, kCtrlrReconnectIfDown) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key = (key_vtn_controller *)malloc(sizeof(key_vtn_controller));
  memset(key,0,sizeof(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();

  state_notification notification=kCtrlrReconnectIfDown;
  bool oper_change;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));

  notification=kCtrlrReconnect;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));

  notification=kCtrlrDisconnect;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));

  notification=kBoundaryFault;
  val_vtn->down_count = 0;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));

  notification=kBoundaryFaultReset;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));

  val_vtn->down_count = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));
  delete ikey;
}
TEST(VtnSetOperStatus, default) {

  VtnMoMgr vtn;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  DalDmlIntf *dmi= new DalOdbcMgr();

  state_notification notification=kCtrlrReconnect;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  uint8_t *vtn_name_o=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.VtnSetOperStatus(vtn_name_o,dmi,notification));
  delete ikey;
}
TEST(VtnSetOperStatus, Valid) {

  VtnMoMgr vtn;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  DalDmlIntf *dmi= new DalOdbcMgr();

  state_notification notification=kCtrlrReconnect;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.VtnSetOperStatus(ctr_id1,dmi,notification));
  delete ikey;
}
TEST(VtnSetOperStatus, Valid_01) {

  VtnMoMgr vtn;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  DalDmlIntf *dmi= new DalOdbcMgr();

  state_notification notification=kCtrlrReconnectIfUp;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.VtnSetOperStatus(ctr_id1,dmi,notification));
  delete ikey;
}
TEST(TxUpdateDtState,UNC_KT_VTN_01 ) {

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);

  VtnMoMgr vtn;
  unc_key_type_t ktype=UNC_KT_VTN;
  uint32_t session_id=1;
  uint32_t config_id=2;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateDtState(ktype,session_id,config_id,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(TxUpdateDtState,UNC_KT_VTN_02 ) {

  VtnMoMgr vtn;
  unc_key_type_t ktype=UNC_KT_VTN;
  uint32_t session_id=1;
  uint32_t config_id=2;
  DalDmlIntf *dmi= new DalOdbcMgr();

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateDtState(ktype,session_id,config_id,dmi));
}
TEST(TxUpdateDtState,UNC_KT_VBRIDGE ) {

  VtnMoMgr vtn;
  unc_key_type_t ktype=UNC_KT_VBRIDGE;
  uint32_t session_id=1;
  uint32_t config_id=2;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_st_t *val = (val_vtn_st_t *)malloc(sizeof(val_vtn_st_t));
  GetKeyValStructSt(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateDtState(ktype,session_id,config_id,dmi));
  delete ikey;
}
TEST(UpdateVnodeOperStatus,UNC_KT_VBRIDGE ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool skip=true;
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
}
TEST(UpdateVnodeOperStatus,kCtrlrDisconnect ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrDisconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool skip=true;
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
}
TEST(UpdateVnodeOperStatus,val_NULL ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();

  key_vbr_t *key = (key_vbr_t *)malloc(sizeof(key_vbr_t));
  val_db_vbr_st *val=NULL;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  bool skip=true;
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
  delete ikey;
}
TEST(UpdateVnodeOperStatus,ikey_NULL ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();

  ConfigKeyVal *ikey = NULL;
  bool skip=true;
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
  delete ikey;
}
TEST(UpdateVnodeOperStatus,UNC_KT_VBR_IF ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool skip=true;
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
}
TEST(UpdateVnodeOperStatus,UNC_KT_VROUTER ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool skip=true;
  key_vrt_t *key = (key_vrt_t *)malloc(sizeof(key_vrt_t));
  val_db_vrt_st_t *val = (val_db_vrt_st_t *)malloc(sizeof(val_db_vrt_st_t));
  val->down_count=0;
  val->fault_count=0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVrtSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, key, config_val);
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
  delete ikey;
}
TEST(UpdateVnodeIfOperStatus,UNC_KT_VBR_IF ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vbr_if_t *key = (key_vbr_if_t *)malloc(sizeof(key_vbr_if_t));
  val_db_vbr_if_st_t *val = (val_db_vbr_if_st_t *)malloc(sizeof(val_db_vbr_if_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIfSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  bool skip=true;
  int if_type=kLinkedInterface;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.UpdateVnodeIfOperStatus(ikey,dmi,notification,skip,if_type));
  delete ikey;
}
TEST(UpdateVnodeIfOperStatus,UNC_KT_VRT_IF ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool skip=true;
  int if_type=kLinkedInterface;
  key_vrt_if_t *key = (key_vrt_if_t *)malloc(sizeof(key_vrt_if_t));
  val_db_vrt_if_st_t *val = (val_db_vrt_if_st_t *)malloc(sizeof(val_db_vrt_if_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVrtIfSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.UpdateVnodeIfOperStatus(ikey,dmi,notification,skip,if_type));
  delete ikey;
}
TEST(UpdateVnodeIfOperStatus,UNC_KT_VROUTER ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool skip=true;
  int if_type=kLinkedInterface;
  key_vrt_t *key = (key_vrt_t *)malloc(sizeof(key_vrt_t));
  val_db_vrt_st_t *val = (val_db_vrt_st_t *)malloc(sizeof(val_db_vrt_st_t));
  val->down_count=0;
  val->fault_count=0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVrtSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.UpdateVnodeIfOperStatus(ikey,dmi,notification,skip,if_type));
  delete ikey;
}
TEST(RestoreVtnOperStatus,invalid_keytype ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrDisconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
}
TEST(RestoreVtnOperStatus,VALID ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrDisconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_db_vtn_st_t *val = NULL;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
}
TEST(RestoreVtnOperStatus,VALID_01 ) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrDisconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val;
  GetKeyValStruct(key,val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(RestoreVtnOperStatus,VALID_02 ) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val;
  GetKeyValStruct(key,val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(RestoreVtnOperStatus,VALID_03 ) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrDisconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val;
  GetKeyValStruct(key,val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(RestoreVtnOperStatus,ikey_NULL ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
}
TEST(RestoreVtnCtrlrOperStatus,ikey_NULL ) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctrlr_id=NULL;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnCtrlrOperStatus(ctrlr_id,dmi,notification));
  delete ikey;
}
TEST(RestoreVtnCtrlrOperStatus,valid_01 ) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctrlr_id=NULL;
  key_vtn_controller_t *key_ctrlr = reinterpret_cast<key_vtn_controller_t *>(malloc
                 (sizeof(key_vtn_controller_t)));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr = reinterpret_cast<val_vtn_ctrlr_t *>(malloc
                 (sizeof(val_vtn_ctrlr_t)));
  val_ctrlr->ref_count=1;
  val_ctrlr->down_count=1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val_ctrlr);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnCtrlrOperStatus(ctrlr_id,dmi,notification));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(RestoreVtnCtrlrOperStatus,valid_02 ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  key_vtn_controller_t *key_ctrlr = reinterpret_cast<key_vtn_controller_t *>(malloc
                 (sizeof(key_vtn_controller_t)));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr = reinterpret_cast<val_vtn_ctrlr_t *>(malloc
                 (sizeof(val_vtn_ctrlr_t)));
  val_ctrlr->ref_count=1;
  val_ctrlr->down_count=1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val_ctrlr);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnCtrlrOperStatus(ctr_id1,dmi,notification));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(RestoreVtnCtrlrOperStatus,valid_03 ) {

  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctrlr_id=NULL;
  key_vtn_controller_t *key_ctrlr = reinterpret_cast<key_vtn_controller_t *>(malloc
                 (sizeof(key_vtn_controller_t)));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr = reinterpret_cast<val_vtn_ctrlr_t *>(malloc
                 (sizeof(val_vtn_ctrlr_t)));
  val_ctrlr->ref_count=1;
  val_ctrlr->down_count=1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val_ctrlr);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.RestoreVtnCtrlrOperStatus(ctrlr_id,dmi,notification));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(CreateVtunnelKey,ikey_NULL ) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.CreateVtunnelKey(ikey,okey));
  delete ikey;
}
TEST(CreateVtunnelKey,IpctSt_invalid ) {

  VtnMoMgr vtn;
  key_vtunnel_t *key = (key_vtunnel_t *)malloc(sizeof(key_vtunnel_t));
  val_vtunnel_t *val = (val_vtunnel_t *)malloc(sizeof(val_vtunnel_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.CreateVtunnelKey(ikey,okey));
  delete ikey;
}
TEST(CreateVtunnelKey,IpctSt_valid ) {

  VtnMoMgr vtn;
  key_vtunnel_t *key = (key_vtunnel_t *)malloc(sizeof(key_vtunnel_t));
  val_vtunnel_t *val = (val_vtunnel_t *)malloc(sizeof(val_vtunnel_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtunnelSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel, key, config_val);
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.CreateVtunnelKey(ikey,okey));
  delete ikey;
}
TEST(SwapKeyVal,ikey_NULL ) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctrlr=NULL;
  bool no_rename;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey,okey,dmi,ctrlr,no_rename));
  delete okey;
  delete ikey;
}

TEST(SwapKeyVal,IpctSt_valid ) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "VTN_1";
  bool no_rename;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));

  ConfigVal *config_val1= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey1 = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val1);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.SwapKeyVal(ikey1,okey,dmi,ctr_id1,no_rename));

  ConfigVal *config_val2= NULL;
  ConfigKeyVal *ikey2 = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val2);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.SwapKeyVal(ikey2,okey,dmi,ctr_id1,no_rename));

  val_rename_vtn_t *val3 = NULL;
  ConfigVal *config_val3= new ConfigVal(IpctSt::kIpcStValVtnSt, val3);
  ConfigKeyVal *ikey3 = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val3);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey3,okey,dmi,ctr_id1,no_rename));

  delete okey;
  delete ikey;
}
TEST(SwapKeyVal,IpctSt_valid_01 ) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "VTN_1";
  bool no_rename;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
  delete ikey;
}

TEST(SwapKeyVal,same_newName ) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "vtn1";
  const char *new_name = "vtn1";
  bool no_rename;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  memset(val,0,sizeof(val_rename_vtn));
  strncpy(reinterpret_cast<char *>(val->new_name),
  new_name, strlen(new_name)+1);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
  delete okey;
  delete ikey;
}
TEST(SwapKeyVal,EmptyNewName ) {

  VtnMoMgr vtn;
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "vtn1";
  const char *new_name = " ";
  bool no_rename;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  memset(key,0,sizeof(key_vtn));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val = (val_rename_vtn_t *)malloc(sizeof(val_rename_vtn_t));
  memset(val,0,sizeof(val_rename_vtn));
  strncpy(reinterpret_cast<char *>(val->new_name),
  new_name, strlen(new_name)+1);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
  delete okey;
  delete ikey;
}

TEST(UpdateCtrlrConfigStatus,valid ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  key_vtn_controller *key = (key_vtn_controller *)malloc(sizeof(key_vtn_controller));
  memset(key,0,sizeof(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
  delete ikey;
}

TEST(UpdateCtrlrConfigStatus,phase_kUpllUcpCreate ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  key_vtn_controller *key = (key_vtn_controller *)malloc(sizeof(key_vtn_controller));
  memset(key,0,sizeof(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
  delete ikey;
}
TEST(UpdateCtrlrConfigStatus,phase_kUpllUcpUpdate ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
  key_vtn_controller *key = (key_vtn_controller *)malloc(sizeof(key_vtn_controller));
  memset(key,0,sizeof(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  val_vtn->cs_row_status = UNC_CS_INVALID;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
  delete ikey;
}
TEST(UpdateCtrlrConfigStatus,phase_kUpllUcpUpdate_invalid ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status=UNC_CS_INVALID;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
  key_vtn_controller *key = (key_vtn_controller *)malloc(sizeof(key_vtn_controller));
  memset(key,0,sizeof(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
  delete ikey;
}
TEST(UpdateAuditConfigStatus,valid ) {

  VtnMoMgr vtn;
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateAuditConfigStatus(cs_status,phase,ikey));
  delete ikey;
}
TEST(UpdateAuditConfigStatus, phase_kUpllUcpCreate ) {

  VtnMoMgr vtn;
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateAuditConfigStatus(cs_status,phase,ikey));
  delete ikey;
}
TEST(SetVtnConsolidatedStatus,ikey_NULL ) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctrlr=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctrlr,dmi));
  delete ikey;
}
TEST(SetVtnConsolidatedStatus,ctrlr_valid ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key = (key_vtn_controller *)malloc(sizeof(key_vtn_controller));
  memset(key,0,sizeof(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  val_vtn->down_count=0;
  val_vtn->ref_count=0;
  val_vtn->cs_row_status=UNC_CS_APPLIED;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctr_id1,dmi));
  delete ikey;
}
TEST(SetVtnConsolidatedStatus,ctrlr_valid_01 ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key = (key_vtn_controller *)malloc(sizeof(key_vtn_controller));
  memset(key,0,sizeof(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  val_vtn->down_count=0;
  val_vtn->ref_count=0;
  val_vtn->cs_row_status=UNC_CS_APPLIED;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctr_id1,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}

TEST(SetVtnConsolidatedStatus,keytype_invalid ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key = (key_vtn_controller *)malloc(sizeof(key_vtn_controller));
  memset(key,0,sizeof(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi= new DalOdbcMgr();
  uint8_t *ctr_id1 = (uint8_t *)malloc(32);
  memset(ctr_id1, '\0', 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctr_id1,dmi));
  delete ikey;
}
TEST(SetConsolidatedStatus,ikey_valid ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key = (key_vtn_controller *)malloc(sizeof(key_vtn_controller));
  memset(key,0,sizeof(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
  vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
  vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
  delete ikey;
}
TEST(SetConsolidatedStatus,ikey_proper ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  memset(key,0,sizeof(key_vtn_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  val_vtn->down_count=0;
  val_vtn->ref_count=0;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
  delete ikey;
}
TEST(SetConsolidatedStatus,ikey_proper_01 ) {

  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  memset(key,0,sizeof(key_vtn_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
  vtn_name, strlen(vtn_name)+1);
  val_vtn_ctrlr_t *val_vtn = (val_vtn_ctrlr_t *)malloc(sizeof(val_vtn_ctrlr_t));
  val_vtn->down_count=0;
  val_vtn->ref_count=0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
  delete ikey;
  DalOdbcMgr::clearStubData();
}
TEST(TxCopyCandidateToRunning,ikey_NULL ) {

  VtnMoMgr vtn;
  unc_key_type_t keytype = UNC_KT_VTN;
  CtrlrCommitStatusList *ctrlr_commit_status=NULL;
  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxCopyCandidateToRunning(keytype,ctrlr_commit_status,dmi));
}

TEST(TxCopyCandidateToRunning,valid_01 ) {

  VtnMoMgr vtn;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;

  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_SUCCESS;

  CtrlrCommitStatusList.push_back(l_CtrlrTxResult);
  unc_key_type_t keytype = UNC_KT_VTN;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  l_CtrlrTxResult->err_ckv = ikey;

  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxCopyCandidateToRunning(keytype,&CtrlrCommitStatusList,dmi));
  delete ikey;
}

TEST(TxCopyCandidateToRunning,valid_02 ) {

  VtnMoMgr vtn;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;

  struct CtrlrTxResult * l_CtrlrTxResult =  new  CtrlrTxResult("vtn",(upll_rc_t)1,1);
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_GENERIC;

  CtrlrCommitStatusList.push_back(l_CtrlrTxResult);
  unc_key_type_t keytype = UNC_KT_VTN;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  l_CtrlrTxResult->err_ckv = ikey;

  DalDmlIntf *dmi= new DalOdbcMgr();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxCopyCandidateToRunning(keytype,&CtrlrCommitStatusList,dmi));
  delete ikey;
}
TEST(GetRenameInfo, ikey_NULL ) {

  VtnMoMgr vtn;
  ConfigKeyVal *ikey=NULL;
  ConfigKeyVal *okey=NULL;
  ConfigKeyVal *rename_info=NULL;
  const char *ctrlr_id = "Controller1";
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool renamed;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenameInfo(ikey,okey,rename_info,dmi,ctrlr_id,renamed));
  delete ikey;
  delete rename_info;
}

TEST(GetRenameInfo, key_NULL ) {

  VtnMoMgr vtn;
  const char *ctrlr_id = "Controller1";
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool renamed=true;
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  memset(key, '\0', sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key,val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *rename_info = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenameInfo(ikey,okey,rename_info,dmi,ctrlr_id,renamed));
  delete ikey;
}
TEST(GetRenameInfo, renamed_false ) {

  VtnMoMgr vtn;
  const char *ctrlr_id = "Controller1";
  DalDmlIntf *dmi= new DalOdbcMgr();
  bool renamed=false;
  key_rename_vnode_info_t *key1 = (key_rename_vnode_info_t *)malloc(sizeof(key_rename_vnode_info_t));
  val_rename_vnode_t *val1 = (val_rename_vnode_t *)malloc(sizeof(val_rename_vnode_t));
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigVal *config_val1= new ConfigVal(IpctSt::kIpcStValVtnSt, val1);
  ConfigKeyVal *rename_info = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key1, config_val1);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenameInfo(ikey,okey,rename_info,dmi,ctrlr_id,renamed));
  delete ikey;
}

TEST(TxUpdateProcess,Create_01 ) {

  VtnMoMgr vtn;
  IpcResponse *req = reinterpret_cast<IpcResponse*>(malloc(sizeof(IpcResponse)));
  unc_keytype_operation_t op=UNC_OP_CREATE;
  controller_domain *ctrlr_dom=NULL;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrSt, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, NULL, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateProcess(ikey,req,op,dmi,ctrlr_dom));

  delete ikey;
}
TEST(TxUpdateProcess,Create_02 ) {

  VtnMoMgr vtn;
  IpcResponse *req = reinterpret_cast<IpcResponse*>(malloc(sizeof(IpcResponse)));
  unc_keytype_operation_t op=UNC_OP_CREATE;
  controller_domain *ctrlr_dom=NULL;
  DalDmlIntf *dmi= new DalOdbcMgr();
  key_vtn_t *key = (key_vtn_t *)malloc(sizeof(key_vtn_t));
  val_vtn_t *val = (val_vtn_t *)malloc(sizeof(val_vtn_t));
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateProcess(ikey,req,op,dmi,ctrlr_dom));

  delete ikey;
}
}
}
}
