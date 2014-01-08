/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <limits.h>
#include <gtest/gtest.h>
#include <pfc/util.h>
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

using ::testing::InitGoogleTest;
using ::testing::Test;
using namespace std;
using namespace unc::tclib;
using namespace unc::upll::dal;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::config_momgr;
using namespace unc::upll::test;
using namespace unc::capa;
using namespace pfc::core;
using namespace unc::upll::dal::schema::table;

namespace unc {
namespace upll {
namespace kt_momgr {

class VtnMoMgrTest
  : public UpllTestEnv
{
};

static void GetKeyStruct(key_vtn *&kst)
{
  const char *vtn_name = "VTN_1";

  kst = ZALLOC_TYPE(key_vtn);
  strncpy(reinterpret_cast<char *>(kst->vtn_name),
          vtn_name, strlen(vtn_name)+1);
}

static void GetKeyValStruct(key_vtn *&kst, val_vtn *&vst)
{
  GetKeyStruct(kst);

  const char *desc = "thisisvbridge";
  vst = ZALLOC_TYPE(val_vtn);
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

static void GetKeyValStructSt(key_vtn *&kst, val_vtn_st *&vst)
{
  const char *vtn_name = "VTN_1";
  //const char *desc = "thisisvbridge";
  kst = ZALLOC_TYPE(key_vtn);
  strncpy(reinterpret_cast<char *>(kst->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  vst = ZALLOC_TYPE(val_vtn_st);
  for(unsigned int loop = 0; loop < sizeof(vst->valid)/
          sizeof(vst->valid[0]); ++loop) {
      vst->valid[loop] = UNC_VF_VALID;
  }
}

static void createControllerInfo(const char* cntrlr_name,
                                 upll_keytype_datatype_t data_type,
                                 unc_keytype_ctrtype_t cntrl_type)
{
    const char*  version("version");
    CtrlrMgr::Ctrlr ctrl(cntrlr_name,cntrl_type,version);
    CtrlrMgr::Ctrlr* ctrl1(new CtrlrMgr::Ctrlr(ctrl,data_type));
    CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
}

/* GetRenameKeyBindInfo() */
// Passing NULL
TEST_F(VtnMoMgrTest, GetRenameKeyBindInfo_outputNull) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);

  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;
  //MoMgrTables tbl;

  EXPECT_TRUE(vtn.GetRenameKeyBindInfo(key_type, bin, nattr, MAINTBL));
}

// Verify the nattr is filled with proper value
TEST_F(VtnMoMgrTest, GetRenameKeyBindInfo_nattrFill) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_TRUE(vtn.GetRenameKeyBindInfo(key_type, bin, nattr, MAINTBL));
  EXPECT_EQ(3, nattr);
}

// Passing controller table to the function
TEST_F(VtnMoMgrTest, GetRenameKeyBindInfo_ctrlTbl) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_TRUE(vtn.GetRenameKeyBindInfo(key_type, bin, nattr, CTRLRTBL));
  EXPECT_EQ(3, nattr);
}

// Passing rename table to the function
TEST_F(VtnMoMgrTest, GetRenameKeyBindInfo_renameTbl) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VTN;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_TRUE(vtn.GetRenameKeyBindInfo(key_type, bin, nattr, RENAMETBL));
  EXPECT_EQ(2, nattr);
}

// Passing rename table to the function
TEST_F(VtnMoMgrTest, GetRenameKeyBindInfo_novtnkey) {
  VtnMoMgr vtn;
  unc_key_type_t key_type = UNC_KT_VBRIDGE;
  BindInfo *bin = NULL;
  int nattr = 2;

  EXPECT_EQ(true, vtn.GetRenameKeyBindInfo(key_type, bin, nattr, RENAMETBL));
}

/* ValidateAttribute() */
// Passing null value to the function
TEST_F(VtnMoMgrTest, ValidateAttribute_nullkey) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateAttribute(ikey, dmi)); //Bug 217
}

// Passing vtn key type to the function
TEST_F(VtnMoMgrTest, ValidateAttribute_keyVtn) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateAttribute(ikey, dmi));

  delete ikey;
}

// Passing other key type to the function
TEST_F(VtnMoMgrTest, ValidateAttribute_keyVtep) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtep_t *keyVtep(ZALLOC_TYPE(key_vtep_t));
  val_vtep_t *valVtep(ZALLOC_TYPE(val_vtep_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtep, valVtep);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTEP, IpctSt::kIpcStKeyVtep, keyVtep, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateAttribute(ikey, dmi));

  delete ikey;
}

// Passing NULL key to the function
TEST_F(VtnMoMgrTest, IsValidKey_keyNull) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn = NULL;
  unsigned long index = 1;
  EXPECT_FALSE(vtn.IsValidKey(keyvtn, index));
}

// Passing the vtn name to the function
TEST_F(VtnMoMgrTest, IsValidKey_vtnName) {
  VtnMoMgr vtn;
  unsigned long index = 0;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  EXPECT_EQ(true, vtn.IsValidKey(keyvtn, index));
  free(keyvtn);
}

// To test the maximum length of vtn name using the function
TEST_F(VtnMoMgrTest, IsValidKey_vtnNameMax) {
  VtnMoMgr vtn;
  unsigned long index = 0;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  pfc_strlcpy(reinterpret_cast<char *>(keyvtn->vtn_name),
              "vtn1vtnvtnvtnvtnvtnvtnvtnvtnvtn", sizeof(keyvtn->vtn_name));
  EXPECT_EQ(true, vtn.IsValidKey(keyvtn, index));
  free(keyvtn);
}

// To test the minimum length of vtn name using the function
TEST_F(VtnMoMgrTest, IsValidKey_vtnNameMin) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  unsigned long index = 0;
  strcpy((char *)keyvtn->vtn_name, (const char *)"1");
  EXPECT_EQ(true, vtn.IsValidKey(keyvtn, index));
  free(keyvtn);
}

// To test exceeding the maximum length of vtn name using the function
TEST_F(VtnMoMgrTest, IsValidKey_vtnNameMaxExceeds) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn(ZALLOC_ARRAY(key_vtn_t, 2));
  unsigned long index = 0;
  memcpy(keyvtn->vtn_name, "vtnnkjdsokljhkjdsncvdsjkjdksdjjksd1",
         sizeof(keyvtn->vtn_name));
  EXPECT_FALSE(vtn.IsValidKey(keyvtn, index));
  free(keyvtn);
}

// To test the empty name of vtn name using the function
TEST_F(VtnMoMgrTest, IsValidKey_vtnNameEmpty) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  unsigned long index = 0;
  strcpy((char *)keyvtn->vtn_name, (const char *)"");
  EXPECT_FALSE(vtn.IsValidKey(keyvtn, index));
  free(keyvtn);
}

/* GetValid() */
// Passing NULL to the function
TEST_F(VtnMoMgrTest, GetValid_nullValue) {
  VtnMoMgr vtn;
  void *val = NULL;
  uint64_t index = uudst::vtn::kDbiVtnOperStatus;
  uint8_t *valid = NULL;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  EXPECT_EQ(NULL, valid);
}

// Passing operstatus to the function
TEST_F(VtnMoMgrTest, GetValid_operStatus) {
  VtnMoMgr vtn;
  void *val = NULL;
  uint64_t index = uudst::vtn::kDbiVtnOperStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst(ZALLOC_TYPE(val_vtn_st_t));
  vtn_valst->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  free(vtn_valst);
}

// Passing alarmstatus to the function
TEST_F(VtnMoMgrTest, GetValid_alarmStatus) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnAlarmStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst(ZALLOC_TYPE(val_vtn_st_t));
  vtn_valst->valid[UPLL_IDX_ALARM_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  free(vtn_valst);
}

// Passing creationTime to the function
TEST_F(VtnMoMgrTest, GetValid_creationTime) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnCreationTime;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst(ZALLOC_TYPE(val_vtn_st_t));
  vtn_valst->valid[UPLL_IDX_CREATION_TIME_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  free(vtn_valst);
}

// Passing lastUpdatedTime to the function
TEST_F(VtnMoMgrTest, GetValid_lastUpdatedTime) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnLastUpdatedTime;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_st_t *vtn_valst(ZALLOC_TYPE(val_vtn_st_t));
  vtn_valst->valid[UPLL_IDX_LAST_UPDATE_TIME_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  free(vtn_valst);
}

// Passing vtnName to the function
TEST_F(VtnMoMgrTest, GetValid_vtnName) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnName;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_vtn_t *val_vtn(ZALLOC_TYPE(val_vtn_t));
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  free(val_vtn);
}

// Passing creationTime with DT_CANDIDATE to the function
TEST_F(VtnMoMgrTest, GetValid_creationTimeDtCandidate) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnCreationTime;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_CANDIDATE;

  val_vtn_st_t *vtn_valst(ZALLOC_TYPE(val_vtn_st_t));
  vtn_valst->valid[UPLL_IDX_CREATION_TIME_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(vtn_valst));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  free(vtn_valst);
}

// Passing description to the function
TEST_F(VtnMoMgrTest, GetValid_description) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnDesc;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_t *val_vtn(ZALLOC_TYPE(val_vtn_t));
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, MAINTBL));
  free(val_vtn);
}

// Passing description to the function
TEST_F(VtnMoMgrTest, GetValid_descriptionRenameTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnDesc;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_t *val_vtn(ZALLOC_TYPE(val_vtn_t));
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, RENAMETBL));
  free(val_vtn);
}

// Passing description to the function
TEST_F(VtnMoMgrTest, GetValid_descriptionCtrlrTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnDesc;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_ctrlr_t *val_vtn_ctrlr(ZALLOC_TYPE(val_vtn_ctrlr_t));
  val_vtn_ctrlr->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_vtn_ctrlr));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetValid(val, index, valid, dtType, CTRLRTBL));
  free(val_vtn_ctrlr);
}

// Passing alarmstatus to the function
TEST_F(VtnMoMgrTest, GetValid_alarmStatusCtrlrTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnAlarmStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_st_t *vtn_valst(ZALLOC_TYPE(val_vtn_st_t));
  vtn_valst->valid[UPLL_IDX_ALARM_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(vtn_valst);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, CTRLRTBL));
  free(vtn_valst);
}

// Passing operstatus to the function
TEST_F(VtnMoMgrTest, GetValid_operStatusCtrlrTbl) {
  VtnMoMgr vtn;
  void *val = NULL;
  unsigned long index = uudst::vtn::kDbiVtnAlarmStatus;
  uint8_t *valid;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_rename_vtn_t *val_rename_vtn(ZALLOC_TYPE(val_rename_vtn_t));
  val_rename_vtn->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
  val = reinterpret_cast<void *>(reinterpret_cast<char *>(val_rename_vtn));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetValid(val, index, valid, dtType, CTRLRTBL));
  free(val_rename_vtn);
}

/* AllocVal() */
//Passing empty configval
TEST_F(VtnMoMgrTest, AllocVal_emptyval) {
  VtnMoMgr vtn;
  ConfigVal *cfg_val = NULL;
  upll_keytype_datatype_t dtType = UPLL_DT_CANDIDATE;
  MoMgrTables tbl=MAINTBL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AllocVal(cfg_val, dtType, tbl));
  delete cfg_val;
}

// Passing configval to the function
TEST_F(VtnMoMgrTest, AllocVal_invalidObj) {
  VtnMoMgr vtn;
  val_vtn_t *val_vtn(ZALLOC_TYPE(val_vtn_t));
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtnSt, val_vtn);//Invalid st_num
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, MAINTBL));
  delete cfg_val;
}

// Passing DT_RUNNING to the function
TEST_F(VtnMoMgrTest, AllocVal_valVtnMaintbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_RUNNING;

  val_vtn_t *val_vtn(ZALLOC_TYPE(val_vtn_t));
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, MAINTBL));//Need to analyse
  delete cfg_val;
}

// Passing DT_STATE to the function
TEST_F(VtnMoMgrTest, AllocVal_valVtnStMaintbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;

  val_db_vtn_st *vtn_valst(ZALLOC_TYPE(val_db_vtn_st));
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtnSt, vtn_valst);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, MAINTBL)); //Need to analyse
  delete cfg_val;
}

// Passing RENAMETBL to the function
TEST_F(VtnMoMgrTest, AllocVal_valRenametbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_CANDIDATE;
  MoMgrTables tbl = RENAMETBL;
  ConfigVal *config_val=NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AllocVal(config_val, dtType, tbl));//Need to analyse
  delete config_val;
}

// Passing CTRLRTBL to the function
TEST_F(VtnMoMgrTest, AllocVal_valCtrlrtbl) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_CANDIDATE;
  MoMgrTables tbl = CTRLRTBL;
  val_vtn_ctrlr_t *val_vtn_ctrlr(ZALLOC_TYPE(val_vtn_ctrlr_t));
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val_vtn_ctrlr);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AllocVal(cfg_val, dtType, tbl));
  delete cfg_val;
}

TEST_F(VtnMoMgrTest, AllocVal_Error_defaultcase) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;
  MoMgrTables tbl = MAX_MOMGR_TBLS;
  ConfigVal *cfg_val = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AllocVal(cfg_val, dtType, tbl));
  delete cfg_val;
}

// Passing empty val to the function
TEST_F(VtnMoMgrTest, DupConfigKeyValVtnMapping_EmptyReqval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey;
  ConfigKeyVal *req=NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnMapping(okey, req));//Need to analyse
}

// Passing empty val to the function
TEST_F(VtnMoMgrTest, DupConfigKeyValVtnMapping_EmptyConfigval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey;
  ConfigKeyVal *req=NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnMapping(okey, req));//Need to analyse
}

TEST_F(VtnMoMgrTest, DupConfigKeyValVtnMapping_Configval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;

  key_vtn_controller_t *key(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st *val
      (ZALLOC_TYPE(val_vtn_mapping_controller_st));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyValVtnMapping(okey, ikey));
  delete okey;
  delete ikey;
}

// Passing empty val to the function
TEST_F(VtnMoMgrTest, DupConfigKeyValVtnStation_EmptyReqval) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;
  ConfigKeyVal *req=NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnStation(okey, req));//Need to analyse
  delete okey;
  delete req;
}

TEST_F(VtnMoMgrTest, DupConfigKeyValVtnStation_kIpcStIpv4) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;

  key_vtn_controller_t *key(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtnstation_controller_st *val(ZALLOC_TYPE(val_vtnstation_controller_st));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStIpv4, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.DupConfigKeyValVtnStation(okey, ikey));
  delete okey;
  delete ikey;
}

TEST_F(VtnMoMgrTest, DupConfigKeyValVtnStation_kIpcStIpv6) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;

  key_vtn_controller_t *key(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtnstation_controller_st *val(ZALLOC_TYPE(val_vtnstation_controller_st));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStIpv6, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.DupConfigKeyValVtnStation(okey, ikey));
  delete okey;
  delete ikey;
}

TEST_F(VtnMoMgrTest, DupConfigKeyValVtnStation_okey_NOT_NULL) {
  VtnMoMgr vtn;

  key_vtn_controller_t *key(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtnstation_controller_st *val(ZALLOC_TYPE(val_vtnstation_controller_st));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStIpv4, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key, config_val);

  key_vtn_controller_t *key1(UT_CLONE(key_vtn_controller_t, key));
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key1, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyValVtnStation(okey, ikey));
  delete ikey;
  delete okey;
}

// Passing val2 as NULL to the function
TEST_F(VtnMoMgrTest, FilterAttributes_val2Null) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vtn_t *valVtn1(ZALLOC_TYPE(val_vtn_t));
  valVtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn1->valid[UPLL_IDX_DESC_VTN]);
  free(valVtn1);
}

// Passing audit_status as false to the function
TEST_F(VtnMoMgrTest, FilterAttributes_auditStatus) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = false;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vtn_t *valVtn1(ZALLOC_TYPE(val_vtn_t));
  val_vtn_t *valVtn2(ZALLOC_TYPE(val_vtn_t));
  valVtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn1->valid[UPLL_IDX_DESC_VTN]);
  free(valVtn1);
  free(valVtn2);
}

// Passing valid flag to the function
TEST_F(VtnMoMgrTest, FilterAttributes_val1ValidFlag) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_CREATE;

  val_vtn_t *valVtn1(ZALLOC_TYPE(val_vtn_t));
  val_vtn_t *valVtn2(ZALLOC_TYPE(val_vtn_t));
  valVtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn1->valid[UPLL_IDX_DESC_VTN]);
  free(valVtn1);
  free(valVtn2);
}

// Passing valid flag with delete operation to the function
TEST_F(VtnMoMgrTest, FilterAttributes_val2ValidFlag) {
  VtnMoMgr vtn;
  void *val1 = NULL;
  void *val2 = NULL;
  bool audit_status = true;
  unc_keytype_operation_t op = UNC_OP_DELETE;

  val_vtn_t *valVtn1(ZALLOC_TYPE(val_vtn_t));
  val_vtn_t *valVtn2(ZALLOC_TYPE(val_vtn_t));
  valVtn2->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  val1 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn1));
  val2 = reinterpret_cast<void *>(reinterpret_cast<char *>(valVtn2));

  vtn.FilterAttributes(val1, val2, audit_status, op);
  EXPECT_EQ(UNC_VF_INVALID, valVtn2->valid[UPLL_IDX_DESC_VTN]);
  free(valVtn1);
  free(valVtn2);
}

// Passing NULL to the function
TEST_F(VtnMoMgrTest, ValidateVtnKey_nullVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyVtn = NULL;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnKey(keyVtn));
}

// Testing the vtn name
TEST_F(VtnMoMgrTest, ValidateVtnKey_properVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnKey(keyvtn));
  free(keyvtn);
}

// Testing the vtn name with minimum value
TEST_F(VtnMoMgrTest, ValidateVtnKey_minVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnKey(keyvtn));
  free(keyvtn);
}

// Testing the vtn name with maximum value
TEST_F(VtnMoMgrTest, ValidateVtnKey_maxVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  pfc_strlcpy(reinterpret_cast<char *>(keyvtn->vtn_name),
              "vtnsddfkjlkssdklfjsdkladdassdd1", sizeof(keyvtn->vtn_name));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnKey(keyvtn));
  free(keyvtn);
}

// Testing the vtn name with maximum value exceeds
TEST_F(VtnMoMgrTest, ValidateVtnKey_maxValExceeds) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn(ZALLOC_ARRAY(key_vtn_t, 2));
  memcpy(keyvtn->vtn_name, "vtndfgdfddfrsdklfjsdklflsdsddfdfgdgfd1",
         sizeof(keyvtn->vtn_name));

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnKey(keyvtn));
  free(keyvtn);
}

// Testing the vtn name with empty value
TEST_F(VtnMoMgrTest, ValidateVtnKey_emptyVal) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnKey(keyvtn));
  free(keyvtn);
}

/* ValidateVtnValue() */
// Passing NULL to the function
TEST_F(VtnMoMgrTest, ValidateVtnValue_invalidVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  uint32_t op = UNC_OP_CREATE;
  strcpy((char*)valVtn->description,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnValue(valVtn, op)); //Bug 250
  free(valVtn);
}

// Testing the vtn description
TEST_F(VtnMoMgrTest, ValidateVtnValue_properVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"ashd l1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
  free(valVtn);
}

// Testing the vtn description with minimum value
TEST_F(VtnMoMgrTest, ValidateVtnValue_minVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
  free(valVtn);
}

// Testing the vtn description with maximum value
TEST_F(VtnMoMgrTest, ValidateVtnValue_maxVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  pfc_strlcpy(reinterpret_cast<char *>(valVtn->description),
              "vtnsddfkjlkssdklfjsdkladdassdd1", sizeof(valVtn->description));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
  free(valVtn);
}

// Testing the vtn description with maximum value exceeds
TEST_F(VtnMoMgrTest, ValidateVtnValue_maxValExceeds) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn(ZALLOC_ARRAY(val_vtn_t, 2));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  memcpy(valVtn->description,
         "vtndfgjjj;j;j;j;jjjjjjjjjdfddfrsdklfjsdklflsdsddfdfgdgfd1",
         sizeof(valVtn->description));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
  free(valVtn);
}

// Testing the vtn description with empty value
TEST_F(VtnMoMgrTest, ValidateVtnValue_emptyVal) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  strcpy((char*)valVtn->description,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnValue(valVtn, op));
  free(valVtn);
}

// Testing the vtn description with invalid flag
TEST_F(VtnMoMgrTest, ValidateVtnValue_invalidFlag) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
  free(valVtn);
}

// Testing the vtn description with no value flag
TEST_F(VtnMoMgrTest, ValidateVtnValue_novalueFlagCreate) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  uint32_t op = UNC_OP_CREATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID_NO_VALUE;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
  free(valVtn);
}

// Testing the vtn description with no value flag
TEST_F(VtnMoMgrTest, ValidateVtnValue_novalueFlagUpdate) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  uint32_t op = UNC_OP_UPDATE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID_NO_VALUE;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
  free(valVtn);
}

// Testing the vtn description with invalid flag
TEST_F(VtnMoMgrTest, ValidateVtnValue_novalueFlagDelete) {
  VtnMoMgr vtn;
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  uint32_t op = UNC_OP_DELETE;
  valVtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID_NO_VALUE;
  strcpy((char*)valVtn->description,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnValue(valVtn, op));
  free(valVtn);
}

// Testing the vtn name
TEST_F(VtnMoMgrTest, ValidateVtnRenameValue_properVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename(ZALLOC_TYPE(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
  free(valVtnRename);
}

// Testing the vtn name with minimum value
TEST_F(VtnMoMgrTest, ValidateVtnRenameValue_minVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename(ZALLOC_TYPE(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
  free(valVtnRename);
}

// Testing the vtn name with maximum value
TEST_F(VtnMoMgrTest, ValidateVtnRenameValue_maxVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename(ZALLOC_TYPE(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  pfc_strlcpy(reinterpret_cast<char *>(valVtnRename->new_name),
              "vtnsddfkjlkssdklfjsdkladdassdd1",
              sizeof(valVtnRename->new_name));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
  free(valVtnRename);
}

// Testing the vtn name with maximum value exceeds
TEST_F(VtnMoMgrTest, ValidateVtnRenameValue_maxValExceeds) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename(ZALLOC_ARRAY(val_rename_vtn_t, 2));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  memcpy(valVtnRename->new_name,
         "vtndfgdfddfrsdklfjsdklflsdsddfdfgdgfd1",
         sizeof(valVtnRename->new_name));

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnRenameValue(valVtnRename));
  free(valVtnRename);
}

// Testing the vtn name with empty value
TEST_F(VtnMoMgrTest, ValidateVtnRenameValue_emptyVal) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename(ZALLOC_TYPE(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnRenameValue(valVtnRename));
  free(valVtnRename);
}

// Testing the vtn name with invalid flag
TEST_F(VtnMoMgrTest, ValidateVtnRenameValue_invalidFlag) {
  VtnMoMgr vtn;
  val_rename_vtn_t *valVtnRename(ZALLOC_TYPE(val_rename_vtn_t));
  valVtnRename->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_INVALID;
  strcpy((char*)valVtnRename->new_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnRenameValue(valVtnRename));
  free(valVtnRename);
}


/* ValidateVtnMapCtrlrKey() */
// Passing NULL to the function
TEST_F(VtnMoMgrTest, ValidateVtnMapCtrlrKey_nullVal) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap = NULL;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
}

// Testing the controller name and vtn name
TEST_F(VtnMoMgrTest, ValidateVtnMapCtrlrKey_properVal) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap(ZALLOC_TYPE(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
  free(keyVtnMap);
}

// Testing the controller name with minimum value and proper vtn name
TEST_F(VtnMoMgrTest, ValidateVtnMapCtrlrKey_ctrlNameMin) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap(ZALLOC_TYPE(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
  free(keyVtnMap);
}

// Testing the controller name with maximum value and proper vtn name
TEST_F(VtnMoMgrTest, ValidateVtnMapCtrlrKey_ctrlNameMax) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap(ZALLOC_TYPE(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  pfc_strlcpy(reinterpret_cast<char *>(keyVtnMap->controller_name),
              "vtnstationcontrollersdklfjsdkl1",
              sizeof(keyVtnMap->controller_name));

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
  free(keyVtnMap);
}

// Testing the controller name with maximum value exceeds and proper vtn name
TEST_F(VtnMoMgrTest, ValidateVtnMapCtrlrKey_ctrlNameMaxExceeds) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap(ZALLOC_ARRAY(key_vtn_controller_t, 2));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  memcpy(keyVtnMap->controller_name,
         "vtnstationcontrollersdklfjsdklflsdsddf1",
         sizeof(keyVtnMap->controller_name));

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
  free(keyVtnMap);
}

// Testing the controller name with empty value and proper vtn name
TEST_F(VtnMoMgrTest, ValidateVtnMapCtrlrKey_ctrlNameempty) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap(ZALLOC_TYPE(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
  free(keyVtnMap);
}

// Testing the vtn name with minimum value and proper controller name
TEST_F(VtnMoMgrTest, ValidateVtnMapCtrlrKey_vtnNameMin) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap(ZALLOC_TYPE(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"1");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
  free(keyVtnMap);
}

// Testing the vtn name with maximum value and proper controller name
TEST_F(VtnMoMgrTest, ValidateVtnMapCtrlrKey_vtnNameMax) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap(ZALLOC_TYPE(key_vtn_controller_t));
  pfc_strlcpy(reinterpret_cast<char *>(keyVtnMap->vtn_key.vtn_name),
              "vtnsdaflkjsdfhksdfgdghkshglkas1",
              sizeof(keyVtnMap->vtn_key.vtn_name));
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
  free(keyVtnMap);
}

// Testing the vtn name with maximum value exceeds and proper controller name
TEST_F(VtnMoMgrTest, ValidateVtnMapCtrlrKey_vtnNameMaxExceeds) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap(ZALLOC_ARRAY(key_vtn_controller_t, 2));
  memcpy(keyVtnMap->vtn_key.vtn_name, "vtnsdaflkjsdfhksdfghkjasdghkshglkask1",
         sizeof(keyVtnMap->vtn_key.vtn_name));
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
  free(keyVtnMap);
}

// Testing the vtn name with empty value and proper controller name
TEST_F(VtnMoMgrTest, ValidateVtnMapCtrlrKey_vtnNameEmpty) {
  VtnMoMgr vtn;
  key_vtn_controller_t *keyVtnMap(ZALLOC_TYPE(key_vtn_controller_t));
  strcpy((char*)keyVtnMap->vtn_key.vtn_name,(const char *)"");
  strcpy((char*)keyVtnMap->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnMapCtrlrKey(keyVtnMap,UNC_OP_CREATE));
  free(keyVtnMap);
}

/* ValidateVtnStnCtrlrKey() */
// Passing NULL to the function
TEST_F(VtnMoMgrTest, ValidateVtnStnCtrlrKey_nullVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn = NULL;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
}

// Testing the controller name
TEST_F(VtnMoMgrTest, ValidateVtnStnCtrlrKey_properVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  strcpy((char*)keyVtnStn->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
  free(keyVtnStn);
}

TEST_F(VtnMoMgrTest, ValidateVtnStnCtrlrKey_READ_SIBLING_COUNT) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  strcpy((char*)keyVtnStn->controller_name,(const char *)"pfc1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_READ_SIBLING_COUNT));
  free(keyVtnStn);
}

// Testing the controller name with minimum value
TEST_F(VtnMoMgrTest, ValidateVtnStnCtrlrKey_minVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  keyVtnStn->controller_name[0] = 'a';

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
  free(keyVtnStn);
}

// Testing the controller name with maximum value
TEST_F(VtnMoMgrTest, ValidateVtnStnCtrlrKey_maxVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  pfc_strlcpy(reinterpret_cast<char *>(keyVtnStn->controller_name),
              "vtnstationcontrollhljhleouuuuuu",
              sizeof(keyVtnStn->controller_name));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
  free(keyVtnStn);
}

// Testing the controller name with maximum value exceeds
TEST_F(VtnMoMgrTest, ValidateVtnStnCtrlrKey_maxValExceeds) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn
      (ZALLOC_ARRAY(key_vtnstation_controller_t, 2));
  memcpy(keyVtnStn->controller_name, "vtnstationcontrollersdklfjsdklflsdsddf1",
         sizeof(keyVtnStn->controller_name));

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
  free(keyVtnStn);
}

// Testing the controller name with empty value
TEST_F(VtnMoMgrTest, ValidateVtnStnCtrlrKey_emptyVal) {
  VtnMoMgr vtn;
  key_vtnstation_controller_t *keyVtnStn
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  strcpy((char*)keyVtnStn->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateVtnStnCtrlrKey(keyVtnStn,UNC_OP_CREATE));
  free(keyVtnStn);
}

/* ReadMo() */
TEST_F(VtnMoMgrTest, ReadMo_otherKT_detail) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//otherKT when option1 = UNC_OPT1_COUNT
TEST_F(VtnMoMgrTest, ReadMo_otherKT_count) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong dt_type
TEST_F(VtnMoMgrTest, ReadMo_mapping_invaliddatatype) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping); 
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, ReadMo_mapping_invalidstnum) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping); 
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no vtn_name
TEST_F(VtnMoMgrTest, ReadMo_mapping_emptyvtnname) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi)); //Bug raised already
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadMo_mapping_valid) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
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

TEST_F(VtnMoMgrTest, ReadMo_mapping_invalid_01) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
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
TEST_F(VtnMoMgrTest, ReadMo_mapping_withctrl_domain) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadMo_Station_invalidkeytype) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, ReadMo_Station_invalidstnum) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has no ctrlr_name
TEST_F(VtnMoMgrTest, ReadMo_Station_emptyctrlrname) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  strcpy((char*)key_station->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadMo(req, ikey, dmi)); //Bug raised already
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadMo_Station_valid_01) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  uuu::upll_strncpy(key_station->controller_name,"pfc1", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
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

TEST_F(VtnMoMgrTest, ReadMo_Station_valid_02) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  uuu::upll_strncpy(key_station->controller_name,"pfc1", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
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

TEST_F(VtnMoMgrTest, ReadMo_Station_valid_03) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  uuu::upll_strncpy(key_station->controller_name,"pfc1", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  memset(val_station, 0, sizeof(val_vtnstation_controller_st_t));
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadMo_mapping_valid_01) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  createControllerInfo("controller_name1", UPLL_DT_RUNNING, UNC_CT_PFC);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadMo_mapping_valid_02) {
  VtnMoMgr vtn;
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

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadMo_mapping_valid_03) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  strcpy((char*)key_ctrlr->domain_id,(const char *)"dom1");
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
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

TEST_F(VtnMoMgrTest, ReadMo_mapping_valid_04) {
  VtnMoMgr vtn;
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

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
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
}

TEST_F(VtnMoMgrTest, ReadMo_mapping_valid_05) {
  VtnMoMgr vtn;
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

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadMo(req, ikey, dmi));
  delete ikey;
}

/* ReadSiblingMo() */
//otherKT when option1 = UNC_OPT1_DETAIL
TEST_F(VtnMoMgrTest, ReadSiblingMo_KT_VTN_detail) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSiblingMo_KT_VBRIDGE) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vbr_t *keyvbr(ZALLOC_TYPE(key_vbr_t));
  strcpy((char*)keyvbr->vbridge_name,(const char *)"vbr1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, keyvbr, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//otherKT when option1 = UNC_OPT1_COUNT
TEST_F(VtnMoMgrTest, ReadSiblingMo_otherKT_count) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong dt_type
TEST_F(VtnMoMgrTest, ReadSiblingMo_Mapping_invaliddatatype) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, ReadSiblingMo_Mapping_invalidstnum) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no vtn_name
TEST_F(VtnMoMgrTest, ReadSiblingMo_Mapping_emptyvtnname) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already/
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no ctrlr name
TEST_F(VtnMoMgrTest, ReadSiblingMo_Mapping_noctrlrname_nodomain) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has wrong dt_type
TEST_F(VtnMoMgrTest, ReadSiblingMo_station_invaliddatatype) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, ReadSiblingMo_Station_invalidstnum) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has no ctrlr_name
TEST_F(VtnMoMgrTest, ReadSiblingMo_Station_emptyctrlrname) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  strcpy((char*)key_station->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has invalid datatype
TEST_F(VtnMoMgrTest, ReadSiblingMo_Station_invaliddatatype) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug has to raise
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has invalid operation
TEST_F(VtnMoMgrTest, ReadSiblingMo_Station_invalidop) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = false;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug has to raise
  delete ikey;
}

//otherKT when option1 = UNC_OPT1_DETAIL
TEST_F(VtnMoMgrTest, ReadSiblingMo_otherKT_detail_begin) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//otherKT when option1 = UNC_OPT1_COUNT
TEST_F(VtnMoMgrTest, ReadSiblingMo_otherKT_count_begin) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_COUNT;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  strcpy((char*)keyvtn->vtn_name,(const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong dt_type
TEST_F(VtnMoMgrTest, ReadSiblingMo_Mapping_invaliddatatype_begin) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, ReadSiblingMo_Mapping_invalidstnum_begin) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTN_MAPPING_CONTROLLER has no vtn_name
TEST_F(VtnMoMgrTest, ReadSiblingMo_Mapping_emptyvtnname_begin) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug raised already/
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSiblingMo_Mapping_01) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSiblingMo_Mapping_READ_SIBLING_BEGIN) {
  VtnMoMgr vtn;
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

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE,
            vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSiblingMo_Mapping_READ) {
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  uuu::upll_strncpy(val_mapping->logical_port_id,"logical_port_id1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->switch_id,"switch_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(val_mapping->port_name,"port_name", (kMaxLenCtrlrId + 1));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE,
            vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSiblingMo_station_READ_SIBLING_BEGIN) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has wrong st_num
TEST_F(VtnMoMgrTest, ReadSiblingMo_Station_invalidstnum_begin) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSiblingMo_Station_READ_SIBLING_BEGIN_01) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
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

TEST_F(VtnMoMgrTest, ReadSiblingMo_Station_READ_SIBLING_BEGIN_02) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  //unc_keytype_ctrtype_t ctrlrtype = UNC_CT_PFC;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
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

TEST_F(VtnMoMgrTest, ReadSiblingMo_Station_READ_SIBLING_BEGIN_03) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
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
TEST_F(VtnMoMgrTest, ReadSiblingMo_Station_emptyctrlrname_begin) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE,
            vtn.ReadSiblingMo(req, ikey, begin, dmi));
  delete ikey;
}

//UNC_KT_VTNSTATION_CONTROLLER has invalid operation
TEST_F(VtnMoMgrTest, ReadSiblingMo_Station_invalidop_begin) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  bool begin = true;

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSiblingMo(req, ikey, begin, dmi)); //Bug has to raise
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSiblingCount_vtn) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_t *key_vtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *val_vtn(ZALLOC_TYPE(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_vtn, config_val);
  strcpy((char*)key_vtn->vtn_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSiblingCount(req, ikey, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSiblingCount_vtnStation_01) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  strcpy((char*)key_station->controller_name,(const char *)"vtn1");

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ReadSiblingCount(req, ikey, dmi));
  delete ikey;
}

/* MappingvExtTovBr() */
TEST_F(VtnMoMgrTest, MappingvExtTovBr_vtnstation_02) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  val_vtnstation_controller_st_t *val_station(ZALLOC_TYPE(val_vtnstation_controller_st_t));
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

TEST_F(VtnMoMgrTest, MappingvExtTovBr_vtnstation_03) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  val_vtnstation_controller_st_t *val_station(ZALLOC_TYPE(val_vtnstation_controller_st_t));
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

TEST_F(VtnMoMgrTest, MappingvExtTovBr_vtnstation_val_NULL) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));

  delete ikey;
}

TEST_F(VtnMoMgrTest, MappingvExtTovBr_vtnstation_04) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  val_vtnstation_controller_st_t *val_station(ZALLOC_TYPE(val_vtnstation_controller_st_t));
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
}

TEST_F(VtnMoMgrTest, MappingvExtTovBr_vtnstation_05) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  val_vtnstation_controller_st_t *val_station(ZALLOC_TYPE(val_vtnstation_controller_st_t));
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
}

TEST_F(VtnMoMgrTest, MappingvExtTovBr_vtnstation_06) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  val_vtnstation_controller_st_t *val_station(ZALLOC_TYPE(val_vtnstation_controller_st_t));
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
}

//when the key is UNC_KT_VTN_MAPPING_CONTROLLER and has invalid key st_num
TEST_F(VtnMoMgrTest, MappingvExtTovBr_mapping_invalidkeynum) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
}

TEST_F(VtnMoMgrTest, MappingvExtTovBr_ikey_No_value) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  ConfigKeyVal *ikey =NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
}

//when the key is UNC_KT_VTN_MAPPING_CONTROLLER and without setting vbr valid flag
TEST_F(VtnMoMgrTest, MappingvExtTovBr_mapping_withoutvbrvalidflag) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
}

//when the key is UNC_KT_VTN_MAPPING_CONTROLLER and with setting vbr valid flag
TEST_F(VtnMoMgrTest, MappingvExtTovBr_mapping_withoutvbrifvalidflag) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  val_mapping->valid[UPLL_IDX_VBR_NAME_VMCS] = UNC_VF_VALID;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MappingvExtTovBr(ikey, req, dmi, count));
  delete ikey;
}

TEST_F(VtnMoMgrTest, MappingvExtTovBr_mapping_withvbrifvalidflag) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
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

TEST_F(VtnMoMgrTest, MappingvExtTovBr_mapping_01) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
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
}

TEST_F(VtnMoMgrTest, MappingvExtTovBr_mapping_02) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
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

TEST_F(VtnMoMgrTest, MappingvExtTovBr_mapping_03) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  IPC_REQ_RESP_HEADER_DECL(req);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t count1=0;
  uint32_t *count=&count1;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
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

/* ValidateMessageForVtnStnCtrlr */
//when the configkeyval is null
TEST_F(VtnMoMgrTest, ValidateMessageForVtnStnCtrlr_ikeynull) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateMessageForVtnStnCtrlr(req, ikey)); // Code has the fix now
  delete ikey;
}

//when the IpcReqRespHeader is null
TEST_F(VtnMoMgrTest, ValidateMessageForVtnStnCtrlr_reqnull) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = NULL;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station(ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateMessageForVtnStnCtrlr(req, ikey)); //Bug 404
  delete ikey;
}

//when invalid st_num
TEST_F(VtnMoMgrTest, ValidateMessageForVtnStnCtrlr_invalid_stnum) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station(ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtn, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ValidateMessageForVtnStnCtrlr_Invalid_option1) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1=(unc_keytype_option1_t)3;
  req->option2 = UNC_OPT2_NEIGHBOR;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station(ZALLOC_TYPE(val_vtnstation_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ValidateMessageForVtnStnCtrlr_Invalid_option2) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;

  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station(ZALLOC_TYPE(val_vtnstation_controller_st_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
  delete ikey;
}

//when valid stnum with ctrlr name
TEST_F(VtnMoMgrTest, ValidateMessageForVtnStnCtrlr_valid_stnum_ctrlname) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_STATE;
  key_vtnstation_controller_t *key_station
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  val_vtnstation_controller_st_t *val_station
      (ZALLOC_TYPE(val_vtnstation_controller_st_t));
  strcpy((char*)key_station->controller_name,(const char *)"pfc1");
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, val_station);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_station, config_val);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ValidateMessageForVtnStnCtrlr(req, ikey));
  delete ikey;
}

//when the configkeyval is null
TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_ikeynull) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateMessageForVtnMapCtrlr(req, ikey)); // Code has the fix now
  delete ikey;
}

//when the IpcReqRespHeader is null
TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_reqnull) {
  VtnMoMgr vtn;
  IpcReqRespHeader *req = NULL;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateMessageForVtnMapCtrlr(req, ikey)); // Code has the fix now
  delete ikey;
}

//when invalid st_num
TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_invalid_stnum) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnstationController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when valid stnum with empty ctrlr name and vtn name
TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_valid_stnum_emptyctrlvtnname) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when valid stnum with empty ctrlr name
TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_valid_stnum_emptyctrlname) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when valid stnum with ctrlr name
TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_valid_stnum_ctrlname) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  pfc_strlcpy(reinterpret_cast<char *>(key_ctrlr->domain_id), "dom1",
              sizeof(key_ctrlr->domain_id));
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when valid stnum with empty vtn name
TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_valid_stnum_emptyvtnname) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_invalid_option1) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_invalid_option2) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_STATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NEIGHBOR;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when invalid datatype
TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_invalid_datatype) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_READ;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

//when invalid operation
TEST_F(VtnMoMgrTest, ValidateMessageForVtnMapCtrlr_invalid_operation) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);

  req->datatype = UPLL_DT_CANDIDATE;
  req->operation = UNC_OP_CREATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  val_vtn_mapping_controller_st_t *val_mapping
      (ZALLOC_TYPE(val_vtn_mapping_controller_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, val_mapping);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER, IpctSt::kIpcStKeyVtnController, key_ctrlr, config_val);
  strcpy((char*)key_ctrlr->vtn_key.vtn_name,(const char *)"vtn1");
  strcpy((char*)key_ctrlr->controller_name,(const char *)"pfc1");
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ValidateMessageForVtnMapCtrlr(req, ikey));
  delete ikey;
}

/* DupConfigKeyVal */
TEST_F(VtnMoMgrTest, DupConfigKeyVal_nullkey) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = MAINTBL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

TEST_F(VtnMoMgrTest, DupConfigKeyVal_nonullokey) {
  VtnMoMgr vtn;
  MoMgrTables tbl=MAINTBL;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  key_vtn_t *key1(UT_CLONE(key_vtn_t, keyvtn));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key1,
                                      NULL));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

TEST_F(VtnMoMgrTest, DupConfigKeyVal_novtnKT) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl=MAINTBL;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

TEST_F(VtnMoMgrTest, DupConfigKeyVal_MAINTBL) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = MAINTBL;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

TEST_F(VtnMoMgrTest, DupConfigKeyVal_MAINTBL_withST) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = MAINTBL;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  val_db_vtn_st *val_dbvtn(ZALLOC_TYPE(val_db_vtn_st));
  ConfigVal *config_valst = new ConfigVal(IpctSt::kIpcStValVtnSt, val_dbvtn);
  config_val->AppendCfgVal(config_valst);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

TEST_F(VtnMoMgrTest, DupConfigKeyVal_RENAMETBL) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = RENAMETBL;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_rename_vtn_t *val_rename_vtn(ZALLOC_TYPE(val_rename_vtn_t));
  ConfigVal *rename_val = new ConfigVal(IpctSt::kIpcStValRenameVtn, val_rename_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStValRenameVtn, keyvtn, rename_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

//Negative test case
TEST_F(VtnMoMgrTest, DupConfigKeyVal_CTRLRTBL) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  MoMgrTables tbl = CTRLRTBL;

  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.DupConfigKeyVal(okey, ikey, tbl));
  delete ikey;
  delete okey;
}

/* MergeValidateChildren */
TEST_F(VtnMoMgrTest, MergeValidateChildren_nullikey) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  const char *ctrlr_id = NULL;
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MergeValidateChildren(ikey, ctrlr_id, ikey, dmi));
}

TEST_F(VtnMoMgrTest, MergeValidateChildren_ikey_VTN) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  const char *ctrlr_id = "pfc1";
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.MergeValidateChildren(ikey, ctrlr_id, ikey, dmi));

  delete ikey;
}

TEST_F(VtnMoMgrTest, MergeValidateChildren_ikey_UNC_KT_VBRIDGE) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  const char *ctrlr_id = "pfc1";
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  key_vbr_t *vbrkey(ZALLOC_TYPE(key_vbr_t));
  vbrkey->vtn_key = *key;
  free(key);
  pfc_strlcpy(reinterpret_cast<char *>(vbrkey->vbridge_name), "VBR_1",
              sizeof(vbrkey->vbridge_name));
  ConfigVal *config_val(new ConfigVal(IpctSt::kIpcStValVtnSt, val));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn,
                                      vbrkey, config_val));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.MergeValidateChildren(ikey, ctrlr_id, ikey, dmi));
}

TEST_F(VtnMoMgrTest, AdaptValToVtnService_ikey_NULL) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AdaptValToVtnService(ikey));
}

TEST_F(VtnMoMgrTest, AdaptValToVtnService_ikey) {
  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;

  GetKeyValStruct(key, val);

  val_vtn_st *valst(ZALLOC_TYPE(val_vtn_st));
  valst->valid[0] = UNC_VF_VALID;
  valst->oper_status = UPLL_OPER_STATUS_UP;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, valst);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.AdaptValToVtnService(ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, AdaptValToVtnService_ConfigVal_NULL) {
  VtnMoMgr vtn;
  key_vtn *key;
  GetKeyStruct(key);

  ConfigVal *config_val= NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.AdaptValToVtnService(ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, GetControllerDomainSpan_ikey) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;
  std::list<controller_domain_t> list_ctrlr_dom;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetControllerDomainSpan(ikey,dtType,dmi,list_ctrlr_dom));
  delete ikey;
}

TEST_F(VtnMoMgrTest, GetControllerDomainSpan_datatype_STATE) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  std::list<controller_domain_t> list_ctrlr_dom;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetControllerDomainSpan(ikey,dt_type,dmi,list_ctrlr_dom));
  delete ikey;
}

TEST_F(VtnMoMgrTest, GetControllerDomainSpan_ikey_01) {
  VtnMoMgr vtn;
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::EXECUTE_QUERY,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  upll_keytype_datatype_t dtType = UPLL_DT_STATE;
  std::list<controller_domain_t> list_ctrlr_dom;
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.GetControllerDomainSpan(ikey,dtType,dmi,list_ctrlr_dom));
  delete ikey;
}

TEST_F(VtnMoMgrTest, GetControllerDomainSpan_ikey_02) {
  DalDmlIntf *dmi(getDalDmlIntf());
  VtnMoMgr vtn;
  key_vtn_t *key;
  val_vtn_t *val;
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
  key_user_data_t *user_data(ZALLOC_TYPE(key_user_data_t));
  uuu::upll_strncpy(user_data->ctrlr_id,"ctr_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetControllerDomainSpan(ikey,dtType,dmi,list_ctrlr_dom));
  delete ikey;
}

TEST_F(VtnMoMgrTest, GetControllerDomainSpan_default) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetControllerDomainSpan(ikey,dt_type,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSingleCtlrlStation_ikey_NULL) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSingleCtlrlStation_ikey_proper) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSingleCtlrlStation_val_vtnstation_valid) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtnstation_controller_t *keyvtnsta
      (ZALLOC_TYPE(key_vtnstation_controller_t));
 uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));

  val_vtnstation_controller_st *valVtnsta
      (ZALLOC_TYPE(val_vtnstation_controller_st));
ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, valVtnsta);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, keyvtnsta, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSingleCtlrlStation_val_vtnstation_ALL_VALID_01) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtnstation_controller_t *keyvtnsta
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));

  val_vtnstation_controller_st *valVtnsta
      (ZALLOC_TYPE(val_vtnstation_controller_st));
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

TEST_F(VtnMoMgrTest, ReadSingleCtlrlStation_val_vtnstation_ALL_VALID_02) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtnstation_controller_t *keyvtnsta
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));

  val_vtnstation_controller_st *valVtnsta
      (ZALLOC_TYPE(val_vtnstation_controller_st));
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
}

TEST_F(VtnMoMgrTest, ReadSingleCtlrlStation_vtn_valid_01) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtnstation_controller_t *keyvtnsta
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));
  val_vtnstation_controller_st *valVtnsta
      (ZALLOC_TYPE(val_vtnstation_controller_st));
  uuu::upll_strncpy(valVtnsta->vtn_name,"vtn_name", (kMaxLenCtrlrId + 1));
  valVtnsta->valid[UPLL_IDX_VTN_NAME_VSCS] = UNC_VF_VALID;
  uuu::upll_strncpy(valVtnsta->vbr_name,"vbr_name", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(valVtnsta->vbrif_name,"vbrif_name", (kMaxLenCtrlrId + 1));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, valVtnsta);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER, IpctSt::kIpcStValVtnstationControllerSt, keyvtnsta, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlStation(req,ikey,dmi,rec_count));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSingleCtlrlStation_vtn_valid_02) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtnstation_controller_t *keyvtnsta
      (ZALLOC_TYPE(key_vtnstation_controller_t));
  uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId + 1));

  val_vtnstation_controller_st *valVtnsta
      (ZALLOC_TYPE(val_vtnstation_controller_st));
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

TEST_F(VtnMoMgrTest, ReadSingleCtlrlStation_vtn_valid_03) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  IPC_REQ_RESP_HEADER_DECL(req);
  uint32_t ckv_count1=1;
  uint32_t *rec_count=&ckv_count1;
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtnstation_controller_t *keyvtnsta
      (ZALLOC_TYPE(key_vtnstation_controller_t));
uuu::upll_strncpy(keyvtnsta->controller_name,"controller_name", (kMaxLenCtrlrId
                                                                   + 1));
  val_vtnstation_controller_st *valVtnsta
      (ZALLOC_TYPE(val_vtnstation_controller_st));
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

TEST_F(VtnMoMgrTest, ReadSingleCtlrlVtnMapping_ikey) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  uint32_t ckv_count1=1;
  uint32_t *ckv_count=&ckv_count1;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));
  req->rep_count = 2;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlVtnMapping(req,ikey,dmi,ckv_count));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSingleCtlrlVtnMapping_ikey_Proper) {
  VtnMoMgr vtn;
  IPC_REQ_RESP_HEADER_DECL(req);
  uint32_t ckv_count1=1;
  uint32_t *ckv_count=&ckv_count1;
  req->rep_count = 2;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ReadSingleCtlrlVtnMapping(req,ikey,dmi,ckv_count));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ReadSingleCtlrlVtnMapping_ikey_Proper_03) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);
  IPC_REQ_RESP_HEADER_DECL(req);
  uint32_t ckv_count1=1;
  uint32_t *ckv_count=&ckv_count1;
  req->rep_count = 2;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE, vtn.ReadSingleCtlrlVtnMapping(req,ikey,dmi,ckv_count));
  delete ikey;
}

TEST_F(VtnMoMgrTest, MergeValidate_Error) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  const char *ctrlr_id = "Controller1";
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.MergeValidate(UNC_KT_VTN, ctrlr_id,
                                              ikey, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, MergeValidate_success) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  VtnMoMgr vtn;
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);
  const char *ctrlr_id = "Controller1";

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.MergeValidate(UNC_KT_VTN, ctrlr_id,
                                              ikey, dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ControllerStatusHandler_01) {
  bool operstatus=true;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.ControllerStatusHandler( ctr_id1,
                                              dmi, operstatus));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, ControllerStatusHandler_02) {
  bool operstatus=true;
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctrlr_id=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.ControllerStatusHandler( ctrlr_id,
                                              dmi, operstatus));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ControllerStatusHandler_03) {
  bool operstatus=true;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.ControllerStatusHandler( ctr_id1,
                                              dmi, operstatus));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, ControllerStatusHandler_04) {
  bool operstatus=true;
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.ControllerStatusHandler( ctr_id1,
                                              dmi, operstatus));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, IsReferenced_01) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsReferenced(ikey,dt_type,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, IsReferenced_02) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsReferenced(ikey,dt_type,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, IsReferenced_03) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  upll_keytype_datatype_t dt_type=UPLL_DT_STATE;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_CFG_SEMANTIC, vtn.IsReferenced(ikey,dt_type,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateOperStatus_01) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateOperStatus( ikey,
                                              dmi, notification, true));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, UpdateOperStatus_02) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  state_notification notification=kPortFaultReset;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  key_user_data_t *user_data(ZALLOC_TYPE(key_user_data_t));
  uuu::upll_strncpy(user_data->ctrlr_id,"Controller1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateOperStatus( ikey,
                                              dmi, notification, true));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateOperStatus_03) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcRecordNoMore);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  state_notification notification=kPortFaultReset;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  key_user_data_t *user_data(ZALLOC_TYPE(key_user_data_t));
  uuu::upll_strncpy(user_data->ctrlr_id,"Controller1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  ikey->set_user_data((void*)user_data);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE,vtn.UpdateOperStatus( ikey, dmi, notification, true));
  delete ikey;
}

TEST_F(VtnMoMgrTest, GetRenamedUncKey_01) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetRenamedUncKey(ikey,dt_type,dmi,ctr_id1));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, GetRenamedUncKey_02) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedUncKey(ikey,dt_type,dmi,ctr_id1));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, GetRenamedUncKey_03) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetRenamedUncKey(ikey,dt_type,dmi,ctr_id1));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, GetRenamedUncKey_04) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedUncKey(ikey,dt_type,dmi,ctr_id1));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, GetRenamedControllerKey_01) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctr_id1;
  ctrlr_dom.domain = dom_id1;
  EXPECT_EQ(UPLL_RC_SUCCESS,
            vtn.GetRenamedControllerKey(ikey, dt_type, dmi, &ctrlr_dom));
  delete ikey;
  free(ctr_id1);
  free(dom_id1);
}

TEST_F(VtnMoMgrTest, GetRenamedControllerKey_02) {
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctr_id1;
  ctrlr_dom.domain = dom_id1;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_SUCCESS,
            vtn.GetRenamedControllerKey(ikey, dt_type, dmi, &ctrlr_dom));
  delete ikey;
  free(ctr_id1);
  free(dom_id1);
}

TEST_F(VtnMoMgrTest, GetRenamedControllerKey_03) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setSingleRecordExists(true);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_EXISTS,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::RECORD_COUNT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcRecordNoMore);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctr_id1;
  ctrlr_dom.domain = dom_id1;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_ERR_NO_SUCH_INSTANCE,
            vtn.GetRenamedControllerKey(ikey, dt_type, dmi, &ctrlr_dom));
  delete ikey;
  free(ctr_id1);
  free(dom_id1);
}

TEST_F(VtnMoMgrTest, GetRenamedControllerKey_04) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;
  DalDmlIntf *dmi(getDalDmlIntf());
  controller_domain *ctrlr_dom=NULL;
  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedControllerKey(ikey,dt_type,dmi,ctrlr_dom));
  delete ikey;
}

TEST_F(VtnMoMgrTest, GetRenamedControllerKey_05) {
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
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctr_id1;
  ctrlr_dom.domain = dom_id1;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.GetRenamedControllerKey(ikey, dt_type, dmi, &ctrlr_dom));
  delete ikey;
  free(ctr_id1);
  free(dom_id1);
}

TEST_F(VtnMoMgrTest, GetRenamedControllerKey_ikey_NULL) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VtnMoMgr vtn;
  key_vtn_t *keyvtn(ZALLOC_TYPE(key_vtn_t));
  val_vtn_t *valVtn(ZALLOC_TYPE(val_vtn_t));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain ctrlr_dom1;
  ctrlr_dom1.ctrlr = ctr_id1;
  ctrlr_dom1.domain = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedControllerKey(ikey,dt_type,dmi,&ctrlr_dom1));
  delete ikey;
  free(ctr_id1);
  free(dom_id1);
}

TEST_F(VtnMoMgrTest, GetRenamedControllerKey_ikey_01) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE ,kDalRcSuccess);
  VtnMoMgr vtn;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, NULL, config_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  uint8_t *dom_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(dom_id1, "Domain1", 7);
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctr_id1;
  ctrlr_dom.domain = dom_id1;
  SET_USER_DATA_FLAGS(ikey, 0x01);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenamedControllerKey(ikey,dt_type,dmi,&ctrlr_dom));
  delete ikey;
  free(ctr_id1);
  free(dom_id1);
}

TEST_F(VtnMoMgrTest, UpdateVtnConfigStatus_Update) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtn_t *key1(UT_CLONE(key_vtn_t, keyvtn));
  val_vtn_t *val1(UT_CLONE(val_vtn_t, valVtn));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                         key1, cfgval1));
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.UpdateVtnConfigStatus(ikey, UNC_OP_UPDATE, driver_result,
                                      upd_key, dmi));
  delete ikey;
  delete upd_key;
}

TEST_F(VtnMoMgrTest, UpdateVtnConfigStatus_Update_01) {
  VtnMoMgr vtn;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtn_t *key1(UT_CLONE(key_vtn_t, keyvtn));
  val_vtn_t *val1(UT_CLONE(val_vtn_t, valVtn));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                         key1, cfgval1));
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.UpdateVtnConfigStatus(ikey, UNC_OP_UPDATE, driver_result,
                                      upd_key, dmi));
  delete ikey;
  delete upd_key;
}

TEST_F(VtnMoMgrTest, UpdateVtnConfigStatus_Create) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtn_t *key1(UT_CLONE(key_vtn_t, keyvtn));
  val_vtn_t *val1(UT_CLONE(val_vtn_t, valVtn));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                         key1, cfgval1));
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_CREATE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
  delete upd_key;
}

TEST_F(VtnMoMgrTest, UpdateVtnConfigStatus_Delete) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtn_t *key1(UT_CLONE(key_vtn_t, keyvtn));
  val_vtn_t *val1(UT_CLONE(val_vtn_t, valVtn));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                         key1, cfgval1));
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.UpdateVtnConfigStatus(ikey, UNC_OP_DELETE, driver_result,
                                      upd_key, dmi));
  delete ikey;
  delete upd_key;
}

TEST_F(VtnMoMgrTest, UpdateVtnConfigStatus_Read) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  strcpy((char *)keyvtn->vtn_name, (const char *)"vtn1");
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vtn_t *key1(UT_CLONE(key_vtn_t, keyvtn));
  val_vtn_t *val1(UT_CLONE(val_vtn_t, valVtn));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                         key1, cfgval1));
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.UpdateVtnConfigStatus(ikey, UNC_OP_READ, driver_result,
                                      upd_key, dmi));
  delete ikey;
  delete upd_key;
}

TEST_F(VtnMoMgrTest, UpdateVtnConfigStatus_Update_ikey_Err) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *upd_key = NULL;
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_UPDATE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateVtnConfigStatus_Update_Val_NULL) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *upd_key = NULL;
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_UPDATE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateVtnConfigStatus_Delete_ikey_Err) {
  VtnMoMgr vtn;
  key_vtn_t *keyvtn;
  val_vtn_t *valVtn;
  GetKeyValStruct(keyvtn,valVtn);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, valVtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, keyvtn, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *upd_key = NULL;
  uint32_t driver_result=UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateVtnConfigStatus(ikey, UNC_OP_DELETE,
                                                   driver_result,
                                                   upd_key,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, CopyToConfigkey_ikeyokeyNull) {
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

TEST_F(VtnMoMgrTest, CopyToConfigkey_01) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;
  key_rename_vnode_info *key_rename(ZALLOC_TYPE(key_rename_vnode_info));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                      key_rename, NULL));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.CopyToConfigKey(okey, ikey));
  delete ikey;
  delete okey;
}

TEST_F(VtnMoMgrTest, CopyToConfigkey_02) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey=NULL;
  key_rename_vnode_info *key_rename(ZALLOC_TYPE(key_rename_vnode_info));
  pfc_strlcpy(reinterpret_cast<char *>(key_rename->old_unc_vtn_name),
              "VTN_1", sizeof(key_rename->old_unc_vtn_name));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                      key_rename, NULL));
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.CopyToConfigKey(okey, ikey));
  delete ikey;
  delete okey;
}

TEST_F(VtnMoMgrTest, UpdateConfigStatus_Create) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn *key;
  val_vtn *val;
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

  key_vtn_t *key1(UT_CLONE(key_vtn_t, key));
  val_vtn_t *val1(UT_CLONE(val_vtn_t, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                         key1, cfgval1));

  key_vtn_t *key2(UT_CLONE(key_vtn_t, key));
  val_vtn_t *val2(UT_CLONE(val_vtn_t, val));
  ConfigVal *cfgval2(new ConfigVal(IpctSt::kIpcStValVtn, val2));
  ConfigKeyVal *ctrlr_run(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                           key2, cfgval2));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ctrlr_run));
  delete ikey;
  delete upd_key;
  delete ctrlr_run;
}

TEST_F(VtnMoMgrTest, UpdateConfigStatus_ValVTN_NULL) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn *key(ZALLOC_TYPE(key_vtn));
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  ConfigVal *cfgval(new ConfigVal(IpctSt::kIpcStValVtn, NULL));
  ConfigKeyVal *ikey(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                      key, cfgval));

  key_vtn_t *key1(UT_CLONE(key_vtn_t, key));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, NULL));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                         key1, cfgval1));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE, UPLL_RC_SUCCESS,
                                   upd_key, dmi, ikey));
  delete ikey;
  delete upd_key;
}

TEST_F(VtnMoMgrTest, UpdateConfigStatus_ConfigKeyVal_NULL) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
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

TEST_F(VtnMoMgrTest, UpdateConfigStatus_PFC) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

  key_vtn_t *key1(UT_CLONE(key_vtn_t, key));
  val_vtn_t *val1(UT_CLONE(val_vtn_t, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                         key1, cfgval1));
  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE, UPLL_RC_SUCCESS,
                                   upd_key, dmi, ikey));
  delete ikey;
  delete upd_key;
}

TEST_F(VtnMoMgrTest, UpdateConfigStatus_OP_Update_01) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr(ZALLOC_TYPE(val_vtn_ctrlr_t));
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
  key_user_data_t *user_data(ZALLOC_TYPE(key_user_data_t));
  uuu::upll_strncpy(user_data->ctrlr_id,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  upd_key->set_user_data((void*)user_data);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,upd_key));
  delete ikey;
  delete upd_key;
}

TEST_F(VtnMoMgrTest, UpdateConfigStatus_OP_Update_02) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr(ZALLOC_TYPE(val_vtn_ctrlr_t));
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
  key_user_data_t *user_data(ZALLOC_TYPE(key_user_data_t));
  uuu::upll_strncpy(user_data->ctrlr_id,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  upd_key->set_user_data((void*)user_data);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);
  key_vtn_controller_t *key_ctrlr2(UT_CLONE(key_vtn_controller_t, key_ctrlr));
  val_vtn_ctrlr_t *val_ctrlr2(ZALLOC_TYPE(val_vtn_ctrlr_t));
  val_ctrlr2->ref_count=1;
  val_ctrlr2->down_count=0;
  ConfigVal *cfgval2(new ConfigVal(IpctSt::kIpcStValVtn, val_ctrlr2));
  ConfigKeyVal *upd_key2(new ConfigKeyVal(UNC_KT_VTN,
                                          IpctSt::kIpcStKeyVtnController,
                                          key_ctrlr2, cfgval2));

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.UpdateConfigStatus(ikey, UNC_OP_UPDATE, UPLL_RC_SUCCESS,
                                   upd_key, dmi, upd_key2));
  delete ikey;
  delete upd_key;
  delete upd_key2;
}

TEST_F(VtnMoMgrTest, UpdateConfigStatus_invalidOP) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);

  key_vtn_t *key1(UT_CLONE(key_vtn_t, key));
  val_vtn_t *val1(UT_CLONE(val_vtn_t, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, val1));
  ConfigKeyVal *upd_key(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                         key1, cfgval1));

  UpllConfigMgr::GetUpllConfigMgr()->CreateMoManagers();
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_READ,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,ikey));
  delete ikey;
  delete upd_key;
}

TEST_F(VtnMoMgrTest, UpdateConfigStatus_OP_Create) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr(ZALLOC_TYPE(val_vtn_ctrlr_t));
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
  key_user_data_t *user_data(ZALLOC_TYPE(key_user_data_t));
  uuu::upll_strncpy(user_data->ctrlr_id,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(user_data->domain_id,"dom_id", (kMaxLenCtrlrId + 1));
  upd_key->set_user_data((void*)user_data);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY, true);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.UpdateConfigStatus(ikey, UNC_OP_CREATE,
                                                   UPLL_RC_SUCCESS,
                                                   upd_key,dmi,upd_key));
  delete ikey;
  delete upd_key;
}

TEST_F(VtnMoMgrTest, UpdateConfigStatus_OP_Create_01) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  val->cs_row_status=UNC_CS_NOT_APPLIED;
  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr(ZALLOC_TYPE(val_vtn_ctrlr_t));
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
  key_user_data_t *user_data(ZALLOC_TYPE(key_user_data_t));
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
  delete upd_key;
}

TEST_F(VtnMoMgrTest, ValidateCapability_Success) {
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

  IPC_REQ_RESP_HEADER_DECL(req);
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

TEST_F(VtnMoMgrTest, ValidateCapability_ikey_NULL) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req=NULL;
  const char *ctrlr_name="ctr1";
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateCapability(req, ikey, ctrlr_name));
}

TEST_F(VtnMoMgrTest, ValidateCapability_ctrName_NULL) {
  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  ConfigVal *cfgval = new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            key, cfgval);
  IPC_REQ_RESP_HEADER_DECL(req);

  const char *ctrlr_name=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateCapability(req, ikey, ctrlr_name));

  delete ikey;
}

TEST_F(VtnMoMgrTest, GetChildConfigKey_SuccessNullObjs) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS,vtn.GetChildConfigKey(okey, pkey));
  delete okey;
  delete pkey;
}

TEST_F(VtnMoMgrTest, GetChildConfigKey_pkeyNull) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VTN,
                            IpctSt::kIpcStKeyVtn,
                            NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.GetChildConfigKey(okey, pkey));
  delete okey;
  delete pkey;
}

TEST_F(VtnMoMgrTest, GetChildConfigKey_pkeyNull_okey_NotNull) {
  VtnMoMgr vtn;
  key_vbr_t *key(ZALLOC_TYPE(key_vbr_t));
  strncpy((char*) key->vbridge_name,"VTN1",32);

  ConfigKeyVal *pkey = NULL;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVbr,key,NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS,vtn.GetChildConfigKey(okey, pkey));
  delete okey;
}

TEST_F(VtnMoMgrTest, GetChildConfigKey_Vtn) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
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

TEST_F(VtnMoMgrTest, GetChildConfigKey_okey_Not_NULL) {
  VtnMoMgr vtn;
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  strncpy((char*) key->vtn_name,"VTN1",32);
  key_vtn_t *key1(UT_CLONE(key_vtn_t, key));

  ConfigKeyVal *pkey(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                      key, NULL));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                      key1, NULL));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.GetChildConfigKey(okey, pkey));

  delete okey;
  delete pkey;
}

TEST_F(VtnMoMgrTest, GetChildConfigKey_Vbr) {
  VtnMoMgr vtn;
  ConfigKeyVal *okey = NULL;
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  strncpy((char*) key->vtn_name,"VTN1",32);

  ConfigKeyVal *pkey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                            IpctSt::kIpcStKeyVtn,
                            key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,vtn.GetChildConfigKey(okey, pkey));
  delete okey;
  delete pkey;
}

TEST_F(VtnMoMgrTest, IsKeyInUse_ikey_Error) {
  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool in_use;

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsKeyInUse(dt_type,ikey,&in_use,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, IsKeyInUse_invalid_ctrlrName) {
  VtnMoMgr vtn;
  key_vtn *key;
  val_vtn *val;
  GetKeyValStruct(key, val);

  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool in_use;
  memset(key,0,sizeof(key_vtn));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsKeyInUse(dt_type,ikey,&in_use,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, IsKeyInUse_ikey_NULL) {
  VtnMoMgr vtn;
  upll_keytype_datatype_t dt_type=UPLL_DT_RUNNING;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool in_use;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.IsKeyInUse(dt_type,ikey,&in_use,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, ValidateMessage_01) {
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

  key_vtn *key1(UT_CLONE(key_vtn, key));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key1, NULL));
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey1));
  val_rename_vtn *renameval(ZALLOC_TYPE(val_rename_vtn));
  for(unsigned int loop = 0; loop < sizeof(renameval->valid)/
          sizeof(renameval->valid[0]); ++loop) {
      renameval->valid[loop] = UNC_VF_VALID;
  }
  strncpy(reinterpret_cast<char *>(renameval->new_name),
          "renamed", strlen("renamed")+1);

  key_vtn *key2(UT_CLONE(key_vtn, key));
  ConfigVal *rename_cfgval(new ConfigVal(IpctSt::kIpcStValRenameVtn,
                                         renameval));
  ConfigKeyVal *rename_ikey(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                             key2, rename_cfgval));

  req->operation = UNC_OP_RENAME;
  req->datatype = UPLL_DT_IMPORT;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, rename_ikey));

  key_vtn *key3(UT_CLONE(key_vtn, key));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key3, NULL));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessage(req, ikey3));

  key_vtn *key4(UT_CLONE(key_vtn, key));
  ConfigVal *cfgval4(new ConfigVal(IpctSt::kIpcStValVtn, NULL));
  ConfigKeyVal *ikey4(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key4, cfgval4));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey4));

  req->operation = UNC_OP_READ_SIBLING;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, rename_ikey));

  key_vtn *key5(UT_CLONE(key_vtn, key));
  ConfigVal *invrename_cfgval(new ConfigVal(IpctSt::kIpcStValRenameVtn, NULL));
  ConfigKeyVal *invrename_ikey(new ConfigKeyVal(UNC_KT_VTN,
                                                IpctSt::kIpcStKeyVtn,
                                                key5, invrename_cfgval));

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, invrename_ikey));

  const char *vtn_name1 = " ";
  key_vtn *key6(ZALLOC_TYPE(key_vtn));
  val_vtn *val6(ZALLOC_TYPE(val_vtn));
  strncpy(reinterpret_cast<char *>(key6->vtn_name),
          vtn_name1, strlen(vtn_name1)+1);
  ConfigVal *cfgval6(new ConfigVal(IpctSt::kIpcStValVtn, val6));
  ConfigKeyVal *ikey6(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key6, cfgval6));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessage(req, ikey6));

  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_RUNNING;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey));

  ikey=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.ValidateMessage(req, ikey));

  key_vtn *key7(UT_CLONE(key_vtn, key));
  val_vtn *val7(UT_CLONE(val_vtn, val));
  ConfigVal *cfgval7(new ConfigVal(IpctSt::kIpcStValVtn, val7));
  ConfigKeyVal *ikey7( new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVbr,
                                     key7, cfgval7));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey7));

  key_vtn *key8(UT_CLONE(key_vtn, key));
  val_vtn *val8(UT_CLONE(val_vtn, val));
  ConfigVal *cfgval8(new ConfigVal(IpctSt::kIpcStValVtn, val8));
  ConfigKeyVal *ikey8(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn,
                                       key8, cfgval8));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey8));

  delete ikey;
  delete ikey1;
  delete rename_ikey;
  delete ikey3;
  delete ikey4;
  delete invrename_ikey;
  delete ikey6;
  delete ikey7;
  delete ikey8;
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

  key_vtn *key1(UT_CLONE(key_vtn, key));
  val_vtn *val1(UT_CLONE(val_vtn, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key1, cfgval1));
  req->operation = UNC_OP_CONTROL;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,
            vtn.ValidateMessage(req, ikey1));

  req->operation = UNC_OP_UPDATE;
  key_vtn *key2(UT_CLONE(key_vtn, key));
  val_vtn *val2(UT_CLONE(val_vtn, val));
  ConfigVal *cfgval2(new ConfigVal(IpctSt::kIpcStValVbr, val2));
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key2, cfgval2));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey2));

  key_vtn *key3(UT_CLONE(key_vtn, key));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key3, NULL));
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtn.ValidateMessage(req, ikey3));

  key_vtn *key4(UT_CLONE(key_vtn, key));
  ConfigVal *cfgval4(new ConfigVal(IpctSt::kIpcStValVtn, NULL));
  ConfigKeyVal *ikey4(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key4, cfgval4));
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey4));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
  delete ikey4;
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

  key_vtn *key1(UT_CLONE(key_vtn, key));
  val_vtn *val1(UT_CLONE(val_vtn, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbr, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey1));

  key_vtn *key2(UT_CLONE(key_vtn, key));
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key2, NULL));
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey2));

  key_vtn *key3(UT_CLONE(key_vtn, key));
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValVtn, NULL));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey3));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
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

  key_vtn *key1(UT_CLONE(key_vtn, key));
  val_vtn *val1(UT_CLONE(val_vtn, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVbr, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtn.ValidateMessage(req, ikey1));

  key_vtn *key2(UT_CLONE(key_vtn, key));
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key2, NULL));
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey2));

  key_vtn *key3(UT_CLONE(key_vtn, key));
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValVtn, NULL));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.ValidateMessage(req, ikey3));

  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
}

TEST_F(VtnMoMgrTest, ValVtnAttributeSupportCheck_01) {
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

TEST_F(VtnMoMgrTest, ValVtnAttributeSupportCheck_02) {
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

TEST_F(VtnMoMgrTest, SetOperStatus_ikey_NULL) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());

  state_notification notification=kCtrlrReconnect;
  ConfigKeyVal *ikey =NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetOperStatus(ikey,notification,dmi));
}

TEST_F(VtnMoMgrTest, SetConsolidatedStatus_invalid_key) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  val_db_vtn_st_t *val(ZALLOC_TYPE(val_db_vtn_st_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  val->down_count=0;
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SetConsolidatedStatus_valid_key) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  val_db_vtn_st_t *val(ZALLOC_TYPE(val_db_vtn_st_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  val->down_count=0;
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SetOperStatus_notification_kCtrlrReconnectIfUp) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  val_db_vtn_st_t *val(ZALLOC_TYPE(val_db_vtn_st_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  val->down_count=0;
  DalDmlIntf *dmi(getDalDmlIntf());

  state_notification notification=kCtrlrReconnectIfUp;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetOperStatus(ikey,notification,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SetOperStatus_notification_kCtrlrDisconnect) {
  const char *vtn_name = "VTN_1";
  VtnMoMgr vtn;
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  val_db_vtn_st_t *val(ZALLOC_TYPE(val_db_vtn_st_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  val->down_count=0;
  DalDmlIntf *dmi(getDalDmlIntf());

  state_notification notification=kCtrlrDisconnect;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetOperStatus(ikey,notification,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SetCtrlrOperStatus_ikey_NULL) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  state_notification notification=kCtrlrReconnect;
  ConfigKeyVal *ikey =NULL;
  bool oper_change;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SetCtrlrOperStatus_kCtrlrReconnectIfUp) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key(ZALLOC_TYPE(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
          vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
          vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

  state_notification notification=kCtrlrReconnectIfUp;
  bool oper_change;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));

  val_vtn->oper_status = UPLL_OPER_STATUS_DOWN;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SetCtrlrOperStatus(ikey,notification,dmi,oper_change));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SetCtrlrOperStatus_kCtrlrReconnectIfDown) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key(ZALLOC_TYPE(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
          vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
          vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

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

TEST_F(VtnMoMgrTest, VtnSetOperStatus_default) {
  VtnMoMgr vtn;
  key_vtn_t *key;
  val_vtn_t *val;
  DalDmlIntf *dmi(getDalDmlIntf());
  GetKeyValStruct(key, val);

  state_notification notification=kCtrlrReconnect;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  uint8_t *vtn_name_o=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.VtnSetOperStatus(vtn_name_o,dmi,notification));
  delete ikey;
}

TEST_F(VtnMoMgrTest, VtnSetOperStatus_Valid) {
  VtnMoMgr vtn;
  key_vtn_t *key;
  val_vtn_t *val;
  DalDmlIntf *dmi(getDalDmlIntf());
  GetKeyValStruct(key, val);

  state_notification notification=kCtrlrReconnect;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.VtnSetOperStatus(ctr_id1,dmi,notification));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, VtnSetOperStatus_Valid_01) {
  VtnMoMgr vtn;
  key_vtn_t *key;
  val_vtn_t *val;
  DalDmlIntf *dmi(getDalDmlIntf());
  GetKeyValStruct(key, val);

  state_notification notification=kCtrlrReconnectIfUp;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.VtnSetOperStatus(ctr_id1,dmi,notification));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, TxUpdateDtState_UNC_KT_VTN_01) {
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);

  VtnMoMgr vtn;
  unc_key_type_t ktype=UNC_KT_VTN;
  uint32_t session_id=1;
  uint32_t config_id=2;
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateDtState(ktype,session_id,config_id,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, TxUpdateDtState_UNC_KT_VTN_02) {
  VtnMoMgr vtn;
  unc_key_type_t ktype=UNC_KT_VTN;
  uint32_t session_id=1;
  uint32_t config_id=2;
  DalDmlIntf *dmi(getDalDmlIntf());

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateDtState(ktype,session_id,config_id,dmi));
}

TEST_F(VtnMoMgrTest, TxUpdateDtState_UNC_KT_VBRIDGE) {
  VtnMoMgr vtn;
  unc_key_type_t ktype=UNC_KT_VBRIDGE;
  uint32_t session_id=1;
  uint32_t config_id=2;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key;
  val_vtn_st_t *val;
  GetKeyValStructSt(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxUpdateDtState(ktype,session_id,config_id,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateVnodeOperStatus_UNC_KT_VBRIDGE) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool skip=true;
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
}

TEST_F(VtnMoMgrTest, UpdateVnodeOperStatus_kCtrlrDisconnect) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrDisconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool skip=true;
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
}

TEST_F(VtnMoMgrTest, UpdateVnodeOperStatus_val_NULL) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());

  key_vbr_t *key(ZALLOC_TYPE(key_vbr_t));
  val_db_vbr_st *val=NULL;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key, config_val);
  bool skip=true;
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateVnodeOperStatus_ikey_NULL) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigKeyVal *ikey = NULL;
  bool skip=true;
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateVnodeOperStatus_UNC_KT_VBR_IF) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool skip=true;
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
}

TEST_F(VtnMoMgrTest, UpdateVnodeOperStatus_UNC_KT_VROUTER) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool skip=true;
  key_vrt_t *key(ZALLOC_TYPE(key_vrt_t));
  val_db_vrt_st_t *val(ZALLOC_TYPE(val_db_vrt_st_t));
  val->down_count=0;
  val->fault_count=0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVrtSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, key, config_val);
  uint8_t *ctrlr_id=NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateVnodeOperStatus(ctrlr_id,dmi,notification,skip));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateVnodeIfOperStatus_UNC_KT_VBR_IF) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vbr_if_t *key(ZALLOC_TYPE(key_vbr_if_t));
  val_db_vbr_if_st_t *val(ZALLOC_TYPE(val_db_vbr_if_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrIfSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key, config_val);
  bool skip=true;
  int if_type=kLinkedInterface;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.UpdateVnodeIfOperStatus(ikey,dmi,notification,skip,if_type));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateVnodeIfOperStatus_UNC_KT_VRT_IF) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool skip=true;
  int if_type=kLinkedInterface;
  key_vrt_if_t *key(ZALLOC_TYPE(key_vrt_if_t));
  val_db_vrt_if_st_t *val(ZALLOC_TYPE(val_db_vrt_if_st_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVrtIfSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.UpdateVnodeIfOperStatus(ikey,dmi,notification,skip,if_type));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateVnodeIfOperStatus_UNC_KT_VROUTER) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  bool skip=true;
  int if_type=kLinkedInterface;
  key_vrt_t *key(ZALLOC_TYPE(key_vrt_t));
  val_db_vrt_st_t *val(ZALLOC_TYPE(val_db_vrt_st_t));
  val->down_count=0;
  val->fault_count=0;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVrtSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.UpdateVnodeIfOperStatus(ikey,dmi,notification,skip,if_type));
  delete ikey;
}

TEST_F(VtnMoMgrTest, RestoreVtnOperStatus_invalid_keytype) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrDisconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
}

TEST_F(VtnMoMgrTest, RestoreVtnOperStatus_VALID) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrDisconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  val_db_vtn_st_t *val = NULL;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
}

TEST_F(VtnMoMgrTest, RestoreVtnOperStatus_VALID_01) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrDisconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key,val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
}

TEST_F(VtnMoMgrTest, RestoreVtnOperStatus_VALID_02) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key,val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
}

TEST_F(VtnMoMgrTest, RestoreVtnOperStatus_VALID_03) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrDisconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key,val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::CREATE_RECORD,kDalRcSuccess);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
}

TEST_F(VtnMoMgrTest, RestoreVtnOperStatus_ikey_NULL) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnOperStatus(ikey,dmi,notification));
  delete ikey;
}

TEST_F(VtnMoMgrTest, RestoreVtnCtrlrOperStatus_ikey_NULL) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctrlr_id=NULL;
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnCtrlrOperStatus(ctrlr_id,dmi,notification));
  delete ikey;
}

TEST_F(VtnMoMgrTest, RestoreVtnCtrlrOperStatus_valid_01) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctrlr_id=NULL;
  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr(ZALLOC_TYPE(val_vtn_ctrlr_t));
  val_ctrlr->ref_count=1;
  val_ctrlr->down_count=1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val_ctrlr);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::SINGLE,kDalRcSuccess);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnCtrlrOperStatus(ctrlr_id,dmi,notification));
  delete ikey;
}

TEST_F(VtnMoMgrTest, RestoreVtnCtrlrOperStatus_valid_02) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::INIT,kDalRcSuccess);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);
  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);

  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr(ZALLOC_TYPE(val_vtn_ctrlr_t));
  val_ctrlr->ref_count=1;
  val_ctrlr->down_count=1;
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val_ctrlr);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_ctrlr, config_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.RestoreVtnCtrlrOperStatus(ctr_id1,dmi,notification));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, RestoreVtnCtrlrOperStatus_valid_03) {
  VtnMoMgr vtn;
  state_notification notification=kCtrlrReconnect;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctrlr_id=NULL;
  key_vtn_controller_t *key_ctrlr(ZALLOC_TYPE(key_vtn_controller_t));
  uuu::upll_strncpy(key_ctrlr->controller_name,"controller_name1", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->domain_id,"domain_id", (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(key_ctrlr->vtn_key.vtn_name,"vtn_name1", (kMaxLenCtrlrId + 1));
  val_vtn_ctrlr_t *val_ctrlr(ZALLOC_TYPE(val_vtn_ctrlr_t));
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
}

TEST_F(VtnMoMgrTest, CreateVtunnelKey_ikey_NULL) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.CreateVtunnelKey(ikey,okey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, CreateVtunnelKey_IpctSt_invalid) {
  VtnMoMgr vtn;
  key_vtunnel_t *key(ZALLOC_TYPE(key_vtunnel_t));
  val_vtunnel_t *val(ZALLOC_TYPE(val_vtunnel_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.CreateVtunnelKey(ikey,okey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, CreateVtunnelKey_IpctSt_valid) {
  VtnMoMgr vtn;
  key_vtunnel_t *key(ZALLOC_TYPE(key_vtunnel_t));
  val_vtunnel_t *val(ZALLOC_TYPE(val_vtunnel_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtunnelSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel, key, config_val);
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.CreateVtunnelKey(ikey,okey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SwapKeyVal_ikey_NULL) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  ConfigKeyVal *okey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctrlr=NULL;
  bool no_rename;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey,okey,dmi,ctrlr,no_rename));
  delete okey;
  delete ikey;
}

TEST_F(VtnMoMgrTest, SwapKeyVal_IpctSt_valid) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "VTN_1";
  bool no_rename;
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val(ZALLOC_TYPE(val_rename_vtn_t));
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.SwapKeyVal(ikey, okey, dmi, ctr_id1, no_rename));
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
  delete okey;
  okey = NULL;

  key_vtn_t *key1(UT_CLONE(key_vtn_t, key));
  val_rename_vtn_t *val1(UT_CLONE(val_rename_vtn_t, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtnSt, val1));
  ConfigKeyVal *ikey1(new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn,
                                       key1, cfgval1));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,
            vtn.SwapKeyVal(ikey1, okey, dmi, ctr_id1, no_rename));
  delete okey;
  okey = NULL;

  key_vtn_t *key2(UT_CLONE(key_vtn_t, key));
  ConfigKeyVal *ikey2(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key2, NULL));
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST,
            vtn.SwapKeyVal(ikey2, okey, dmi, ctr_id1, no_rename));
  delete okey;
  okey = NULL;

  key_vtn_t *key3(UT_CLONE(key_vtn_t, key));
  ConfigVal *cfgval3(new ConfigVal(IpctSt::kIpcStValVtnSt, NULL));
  ConfigKeyVal *ikey3(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                       key3, cfgval3));
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.SwapKeyVal(ikey3, okey, dmi, ctr_id1, no_rename));

  delete okey;
  delete ikey;
  delete ikey1;
  delete ikey2;
  delete ikey3;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, SwapKeyVal_IpctSt_valid_01) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "VTN_1";
  bool no_rename;
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val(ZALLOC_TYPE(val_rename_vtn_t));
  pfc_strlcpy(reinterpret_cast<char *>(val->new_name), "VTN_2",
              sizeof(val->new_name));

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
  delete okey;
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, SwapKeyVal_same_newName) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "vtn1";
  const char *new_name = "vtn1";
  bool no_rename;
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val(ZALLOC_TYPE(val_rename_vtn_t));
  strncpy(reinterpret_cast<char *>(val->new_name),
          new_name, strlen(new_name)+1);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
  delete okey;
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, SwapKeyVal_EmptyNewName) {
  VtnMoMgr vtn;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);

  const char *vtn_name = "vtn1";
  const char *new_name = " ";
  bool no_rename;
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  val_rename_vtn_t *val(ZALLOC_TYPE(val_rename_vtn_t));
  strncpy(reinterpret_cast<char *>(val->new_name),
          new_name, strlen(new_name)+1);
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  ConfigKeyVal *okey = NULL;
  val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.SwapKeyVal(ikey,okey,dmi,ctr_id1,no_rename));
  delete okey;
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, UpdateCtrlrConfigStatus_valid) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  key_vtn_controller *key(ZALLOC_TYPE(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
          vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
          vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
  delete ikey;
}


TEST_F(VtnMoMgrTest, UpdateCtrlrConfigStatus_phase_kUpllUcpCreate) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  key_vtn_controller *key(ZALLOC_TYPE(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
          vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
          vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateCtrlrConfigStatus_phase_kUpllUcpUpdate) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
  key_vtn_controller *key(ZALLOC_TYPE(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
          vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
          vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  val_vtn->cs_row_status = UNC_CS_INVALID;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateCtrlrConfigStatus_phase_kUpllUcpUpdate_invalid) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  unc_keytype_configstatus_t cs_status=UNC_CS_INVALID;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpUpdate;
  key_vtn_controller *key(ZALLOC_TYPE(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
          vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
          vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  val_vtn->valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateCtrlrConfigStatus(cs_status,phase,ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateAuditConfigStatus_valid) {
  VtnMoMgr vtn;
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateAuditConfigStatus(cs_status,phase,ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, UpdateAuditConfigStatus_phase_kUpllUcpCreate) {
  VtnMoMgr vtn;
  unc_keytype_configstatus_t cs_status=UNC_CS_APPLIED;
  uuc::UpdateCtrlrPhase phase=uuc::kUpllUcpCreate;
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtn.UpdateAuditConfigStatus(cs_status,phase,ikey));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SetVtnConsolidatedStatus_ikey_NULL) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctrlr=NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctrlr,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SetVtnConsolidatedStatus_ctrlr_valid) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key(ZALLOC_TYPE(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
          vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
          vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  val_vtn->down_count=0;
  val_vtn->ref_count=0;
  val_vtn->cs_row_status=UNC_CS_APPLIED;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctr_id1,dmi));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, SetVtnConsolidatedStatus_ctrlr_valid_01) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key(ZALLOC_TYPE(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
          vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
          vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  val_vtn->down_count=0;
  val_vtn->ref_count=0;
  val_vtn->cs_row_status=UNC_CS_APPLIED;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctr_id1,dmi));
  delete ikey;
  free(ctr_id1);
}


TEST_F(VtnMoMgrTest, SetVtnConsolidatedStatus_keytype_invalid) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key(ZALLOC_TYPE(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
          vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
          vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVtn, key, config_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  uint8_t *ctr_id1 = ZALLOC_ARRAY(uint8_t, 32);
  memcpy(ctr_id1, "Controller1", 11);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetVtnConsolidatedStatus(ikey,ctr_id1,dmi));
  delete ikey;
  free(ctr_id1);
}

TEST_F(VtnMoMgrTest, SetConsolidatedStatus_ikey_valid) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  const char *controller_name = "pfc1";
  const char *domain_id = "dom1";
  key_vtn_controller *key(ZALLOC_TYPE(key_vtn_controller));
  strncpy(reinterpret_cast<char *>(key->controller_name),
          vtn_name, strlen(controller_name)+1);
  strncpy(reinterpret_cast<char *>(key->domain_id),
          vtn_name, strlen(domain_id)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SetConsolidatedStatus_ikey_proper) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  val_vtn->down_count=0;
  val_vtn->ref_count=0;
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, SetConsolidatedStatus_ikey_proper_01) {
  VtnMoMgr vtn;
  const char *vtn_name = "VTN_1";
  key_vtn_t *key(ZALLOC_TYPE(key_vtn_t));
  strncpy(reinterpret_cast<char *>(key->vtn_name),
          vtn_name, strlen(vtn_name)+1);
  val_vtn_ctrlr_t *val_vtn(ZALLOC_TYPE(val_vtn_ctrlr_t));
  val_vtn->down_count=0;
  val_vtn->ref_count=0;
  DalOdbcMgr::stub_setResultcode(DalOdbcMgr::MULTIPLE,kDalRcSuccess);

  std::map<uint8_t,DalResultCode> map;
  map.insert(std::make_pair(1,kDalRcRecordNoMore));
  map.insert(std::make_pair(0,kDalRcSuccess));
  DalOdbcMgr::stub_setNextRecordResultCodes(map);
  ConfigVal *config_val = new ConfigVal(IpctSt::kIpcStValVtn, val_vtn);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.SetConsolidatedStatus(ikey,dmi));
  delete ikey;
}

TEST_F(VtnMoMgrTest, TxCopyCandidateToRunning_ikey_NULL) {
  VtnMoMgr vtn;
  unc_key_type_t keytype = UNC_KT_VTN;
  CtrlrCommitStatusList *ctrlr_commit_status=NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxCopyCandidateToRunning(keytype,ctrlr_commit_status,dmi));
}

TEST_F(VtnMoMgrTest, TxCopyCandidateToRunning_valid_01) {
  VtnMoMgr vtn;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;

  struct CtrlrTxResult *l_CtrlrTxResult
    (new CtrlrTxResult("vtn",(upll_rc_t)1, 1));
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_SUCCESS;

  CtrlrCommitStatusList.push_back(l_CtrlrTxResult);
  unc_key_type_t keytype = UNC_KT_VTN;
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  l_CtrlrTxResult->err_ckv = ikey;

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxCopyCandidateToRunning(keytype,&CtrlrCommitStatusList,dmi));
  delete l_CtrlrTxResult;
}

TEST_F(VtnMoMgrTest, TxCopyCandidateToRunning_valid_02) {
  VtnMoMgr vtn;
  std::list<CtrlrCommitStatus *> CtrlrCommitStatusList;

  struct CtrlrTxResult * l_CtrlrTxResult
    (new CtrlrTxResult("vtn",(upll_rc_t)1, 1));
  l_CtrlrTxResult->upll_ctrlr_result = UPLL_RC_ERR_GENERIC;

  CtrlrCommitStatusList.push_back(l_CtrlrTxResult);
  unc_key_type_t keytype = UNC_KT_VTN;
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);
  l_CtrlrTxResult->err_ckv = ikey;

  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.TxCopyCandidateToRunning(keytype,&CtrlrCommitStatusList,dmi));
  delete l_CtrlrTxResult;
}

TEST_F(VtnMoMgrTest, GetRenameInfo_ikey_NULL) {
  VtnMoMgr vtn;
  ConfigKeyVal *ikey=NULL;
  ConfigKeyVal *okey=NULL;
  ConfigKeyVal *rename_info=NULL;
  const char *ctrlr_id = "Controller1";
  DalDmlIntf *dmi(getDalDmlIntf());
  bool renamed;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenameInfo(ikey,okey,rename_info,dmi,ctrlr_id,renamed));
  delete ikey;
  delete rename_info;
}

TEST_F(VtnMoMgrTest, GetRenameInfo_key_NULL) {
  VtnMoMgr vtn;
  const char *ctrlr_id = "Controller1";
  DalDmlIntf *dmi(getDalDmlIntf());
  bool renamed=true;
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key,val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  key_vtn_t *key1(UT_CLONE(key_vtn_t, key));
  val_vtn_t *val1(UT_CLONE(val_vtn_t, val));
  ConfigVal *cfgval1(new ConfigVal(IpctSt::kIpcStValVtn, val1));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                      key1, cfgval1));
  ConfigKeyVal *rename_info = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenameInfo(ikey,okey,rename_info,dmi,ctrlr_id,renamed));
  delete ikey;
  delete okey;
}

TEST_F(VtnMoMgrTest, GetRenameInfo_renamed_false) {
  VtnMoMgr vtn;
  const char *ctrlr_id = "Controller1";
  DalDmlIntf *dmi(getDalDmlIntf());
  bool renamed=false;
  key_rename_vnode_info_t *key1(ZALLOC_TYPE(key_rename_vnode_info_t));
  val_rename_vnode_t *val1(ZALLOC_TYPE(val_rename_vnode_t));
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtnSt, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  key_vtn_t *key_o(UT_CLONE(key_vtn_t, key));
  val_vtn_t *val_o(UT_CLONE(val_vtn_t, val));
  ConfigVal *cfg_o(new ConfigVal(IpctSt::kIpcStValVtn, val_o));
  ConfigKeyVal *okey(new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_o,
                                      cfg_o));

  ConfigVal *config_val1= new ConfigVal(IpctSt::kIpcStValVtnSt, val1);
  ConfigKeyVal *rename_info = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key1, config_val1);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtn.GetRenameInfo(ikey,okey,rename_info,dmi,ctrlr_id,renamed));
  delete ikey;
  delete okey;
  delete rename_info;
}

TEST_F(VtnMoMgrTest, TxUpdateProcess_Create_01) {
  VtnMoMgr vtn;
  unc_keytype_operation_t op=UNC_OP_CREATE;
  controller_domain *ctrlr_dom=NULL;
  DalDmlIntf *dmi(getDalDmlIntf());

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVbrSt, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, NULL, config_val);

  IPC_RESPONSE_DECL(resp);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.TxUpdateProcess(ikey, resp, op, dmi, ctrlr_dom));

  delete ikey;
}

TEST_F(VtnMoMgrTest, TxUpdateProcess_Create_02) {
  VtnMoMgr vtn;
  unc_keytype_operation_t op=UNC_OP_CREATE;
  controller_domain *ctrlr_dom=NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  key_vtn_t *key;
  val_vtn_t *val;
  GetKeyValStruct(key, val);

  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVtn, val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, config_val);

  IPC_RESPONSE_DECL(resp);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtn.TxUpdateProcess(ikey, resp, op, dmi, ctrlr_dom));

  delete ikey;
}

}
}
}
