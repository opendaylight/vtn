/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "include/vtn_ff_stub.hh"

using ::testing::Test;
using ::testing::TestWithParam;
using ::testing::Values;
using namespace std;
using namespace unc::upll;
using namespace unc::tclib;
using namespace unc::upll::dal;
using namespace unc::upll::kt_momgr;
using namespace unc::upll::config_momgr;
using namespace unc::capa;
using namespace pfc::core;
using namespace unc::upll::dal::schema::table;

int testcase_id;
std::map<function, upll_rc_t> VtnffTest::stub_result;

//bool fatal_done;
//pfc::core::Mutex  fatal_mutex_lock;
//Test cases
TEST_F(VtnffTest, GetRen_Generic_Err) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_GENERIC;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, GetRen_NO_INS_DB_Err) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_DB_ACCESS;
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, GetRen_Upd_NO_INS) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, GetRen_DB_Err) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_DB_ACCESS;
  EXPECT_EQ(UPLL_RC_ERR_DB_ACCESS,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, UPD_DB_Err) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_DB_ACCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, UPD_NO_INS_Err) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, UPD_Success) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;
  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateControllerTable1] = UPLL_RC_SUCCESS;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, UPD_INS_EXIST_Err) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;
  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateControllerTable1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, Success_Case2) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;
  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateControllerTable1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, GetVtn_Err_GENERIC) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;
  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_ERR_GENERIC;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, GetVtn_Err_DB) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;
  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_ERR_DB_ACCESS;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, GetVtn_NO_INS) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;
  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, UpdateCtr_Err_DB) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;
  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateControllerTable1] = UPLL_RC_ERR_DB_ACCESS;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, UpdateCtr_Err_Generic) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;
  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateControllerTable1] = UPLL_RC_ERR_GENERIC;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffTest, UpdateCtr_NO_INS) {
  VtnffTest::stub_result.clear();
  VtnffTest obj;
  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_t *ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_t)));
  memset(ff_key, 0, sizeof(key_vtn_flowfilter_t));
  val_flowfilter_t *ff_val = reinterpret_cast<val_flowfilter_t*>
                           (malloc(sizeof(val_flowfilter_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValFlowfilter, ff_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcStKeyVtnFlowfilter, ff_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_SUCCESS;
  VtnffTest::stub_result[UpdateControllerTable1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}
