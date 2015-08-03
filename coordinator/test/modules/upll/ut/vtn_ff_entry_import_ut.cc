/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "include/vtn_ff_entry_stub.hh"

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

//int testcase_id;
//std::map<function, upll_rc_t> VtnffEntryTest::stub_result;
std::map<function, upll_rc_t> UpdateFlowTest::stub_result;
std::map<function, upll_rc_t> CompareValTest::stub_result;

//bool fatal_done;
//pfc::core::Mutex  fatal_mutex_lock;

TEST_F(VtnffEntryTest, req_NULL) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = NULL;

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key = 
                                reinterpret_cast<key_vtn_flowfilter_entry_t*>
                                (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, ikey_NULL) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  ConfigKeyVal *ikey = NULL;

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_GENERIC;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, ikey_req_NULL) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = NULL;

  DalDmlIntf  *dmi = new DalOdbcMgr();

  ConfigKeyVal *ikey = NULL;

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_GENERIC;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, GetRen_Generic) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                                 reinterpret_cast<key_vtn_flowfilter_entry_t*>
                                 (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                           reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_GENERIC;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}


TEST_F(VtnffEntryTest, GetRen_DB_Err) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                                  reinterpret_cast<key_vtn_flowfilter_entry_t*>
                                  (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                           reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_DB_ACCESS;
  EXPECT_EQ(UPLL_RC_ERR_DB_ACCESS,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, GetRen_NO_INS) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                                reinterpret_cast<key_vtn_flowfilter_entry_t*>
                                (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val = 
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_DB_ACCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, GetRen_Upd_NO_INS) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                               reinterpret_cast<key_vtn_flowfilter_entry_t*>
                               (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                           reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, Upd_DB_Err) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_DB_ACCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, Upd_NO_INS_Err) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key = 
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, Upd_Success) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                              reinterpret_cast<val_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateControllerTable1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, Upd_INS_EXIST_Success) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffEntryTest::stub_result[CompareValueStructure1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateControllerTable1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, Success_Case2) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffEntryTest::stub_result[CompareValueStructure1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateControllerTable1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, UpdFlow_Success) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key = 
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffEntryTest::stub_result[CompareValueStructure1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateControllerTable1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateFlowListInCtrlTbl1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, Compare_Err_Conflict) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffEntryTest::stub_result[CompareValueStructure1] = 
                                                  UPLL_RC_ERR_MERGE_CONFLICT;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, Compare_Err_Generic) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                               reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val = 
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffEntryTest::stub_result[CompareValueStructure1] = UPLL_RC_ERR_GENERIC;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, Compare_Err_DB_Access) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val = 
                           reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  VtnffEntryTest::stub_result[CompareValueStructure1] = UPLL_RC_ERR_DB_ACCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, GetVtn_Err_INS_EXIST) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                  vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[CompareValueStructure1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[GetVtnControllerSpan1] =
                                             UPLL_RC_ERR_INSTANCE_EXISTS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, GetVtn_Err_DB_Access) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val = 
                             reinterpret_cast<val_vtn_flowfilter_entry_t*>
                             (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[CompareValueStructure1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[GetVtnControllerSpan1] = UPLL_RC_ERR_DB_ACCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, GetVtn_Succ_NO_Inst) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key = 
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                             (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[CompareValueStructure1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[GetVtnControllerSpan1] =
                                           UPLL_RC_ERR_NO_SUCH_INSTANCE;
  VtnffEntryTest::stub_result[UpdateControllerTable1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateFlowListInCtrlTbl1] = UPLL_RC_SUCCESS;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}

TEST_F(VtnffEntryTest, UpdFlow_Err) {
  VtnffEntryTest::stub_result.clear();
  VtnffEntryTest obj;

  //unc_key_type_t keytype = UNC_KT_VTN_FLOWFILTER_ENTRY;
  IpcReqRespHeader *req = ZALLOC_TYPE(IpcReqRespHeader);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                               reinterpret_cast<key_vtn_flowfilter_entry_t*>
                               (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val = 
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  VtnffEntryTest::stub_result[GetRenamedUncKey1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateConfigDB1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[CompareValueStructure1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[GetVtnControllerSpan1] =
                                           UPLL_RC_ERR_NO_SUCH_INSTANCE;
  VtnffEntryTest::stub_result[UpdateControllerTable1] = UPLL_RC_SUCCESS;
  VtnffEntryTest::stub_result[UpdateFlowListInCtrlTbl1] = UPLL_RC_SUCCESS;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CreatePIForVtnPom(req, ikey, dmi, ctrlr_id));
  delete req;
  delete ikey;
  delete dmi;
}
/*
TEST_F(UpdateFlowTest, UpdateFlowList_Gen_Err) {
  UpdateFlowTest::stub_result.clear();
  UpdateFlowTest obj;

  DalDmlIntf  *dmi = new DalOdbcMgr();

  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  TcConfigMode config_mode = TC_CONFIG_VTN;
  string vtn_name = "vtn1";

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));

  val_vtn_flowfilter_entry_t *vtn_ffe_val = 
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  const char *ctrlr_id = {"pfc1"};

  UpdateFlowTest::stub_result[AddFlowListToController1] = UPLL_RC_ERR_GENERIC;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.UpdateFlowListInCtrlTbl(ikey, dt_type, ctrlr_id, dmi,
          config_mode, vtn_name));
  delete ikey;
  delete dmi;
}
*/
TEST_F(CompareValTest, Read_Err_DB_Access) {
  CompareValTest::stub_result.clear();
  CompareValTest obj;

  DalDmlIntf  *dmi = new DalOdbcMgr();

  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                             (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val = 
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  CompareValTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_DB_ACCESS;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CompareValueStructure(ikey, dt_type, dmi));;
  delete ikey;
  delete dmi;
}

TEST_F(CompareValTest, Read_Err_Generic) {
  CompareValTest::stub_result.clear();
  CompareValTest obj;

  DalDmlIntf  *dmi = new DalOdbcMgr();

  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                             (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val = 
                           reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  CompareValTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_GENERIC;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CompareValueStructure(ikey, dt_type, dmi));;
  delete ikey;
  delete dmi;
}

TEST_F(CompareValTest, Read_Err_NO_INS) {
  CompareValTest::stub_result.clear();
  CompareValTest obj;

  DalDmlIntf  *dmi = new DalOdbcMgr();

  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val = 
                           reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  CompareValTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CompareValueStructure(ikey, dt_type, dmi));;
  delete ikey;
  delete dmi;
}

TEST_F(CompareValTest, Compare_Success) {
  CompareValTest::stub_result.clear();
  CompareValTest obj;

  DalDmlIntf  *dmi = new DalOdbcMgr();

  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                             (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val = 
                           reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  CompareValTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  CompareValTest::stub_result[CompareValStructure1] = UPLL_RC_SUCCESS;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CompareValueStructure(ikey, dt_type, dmi));;
  delete ikey;
  delete dmi;
}

TEST_F(CompareValTest, Compare_Conflict) {
  CompareValTest::stub_result.clear();
  CompareValTest obj;

  DalDmlIntf  *dmi = new DalOdbcMgr();

  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;

  key_vtn_flowfilter_entry_t *vtn_ffe_key =
                              reinterpret_cast<key_vtn_flowfilter_entry_t*>
                              (malloc(sizeof(key_vtn_flowfilter_entry_t)));
  memset(vtn_ffe_key, 0, sizeof(key_vtn_flowfilter_entry_t));
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
                            reinterpret_cast<val_vtn_flowfilter_entry_t*>
                           (malloc(sizeof(val_vtn_flowfilter_entry_t)));

  vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                                 vtn_ffe_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                       IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key, val);

  CompareValTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  CompareValTest::stub_result[CompareValStructure1] =
                                              UPLL_RC_ERR_MERGE_CONFLICT;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
          obj.CompareValueStructure(ikey, dt_type, dmi));;
  delete ikey;
  delete dmi;
}

TEST_F(CompareValStructureTest, Fl_name_Invalid1) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_INVALID;

  strncpy((char*) val_ffe1->flowlist_name,"FlowlistName", 32);

  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}


TEST_F(CompareValStructureTest, Fl_name_mismatch) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                               ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                               ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;

  strncpy((char*) val_ffe1->flowlist_name,"FlowlistName", 32);
  strncpy((char*) val_ffe2->flowlist_name,"Flowlist1", 32);

  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, Fl_name_matched) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;

  strncpy((char*) val_ffe1->flowlist_name,"FlowlistName", 32);
  strncpy((char*) val_ffe2->flowlist_name,"FlowlistName", 32);


  EXPECT_EQ(UPLL_RC_SUCCESS,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, Action_Invalid) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_INVALID;

  val_ffe1->action = 1;
  val_ffe2->action = 1;

  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, Action_mismatch) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;

  val_ffe1->action = 0;
  val_ffe2->action = 1;

  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, Action_match) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                             ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                             ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;

  val_ffe1->action = 1;
  val_ffe2->action = 1;

  EXPECT_EQ(UPLL_RC_SUCCESS,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, nwm_Invalid) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                             ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                             ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_INVALID;

  strncpy((char*) val_ffe1->nwm_name,"nwm", 32);
  strncpy((char*) val_ffe2->nwm_name,"nwm", 32);

  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, nwm_mismatch) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;

  strncpy((char*) val_ffe1->nwm_name,"nwm1", 32);
  strncpy((char*) val_ffe2->nwm_name,"nwm2", 32);

  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, nwm_match) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;

  strncpy((char*) val_ffe1->nwm_name,"nwm", 32);
  strncpy((char*) val_ffe2->nwm_name,"nwm", 32);

  EXPECT_EQ(UPLL_RC_SUCCESS,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, Dscp_Invalid) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_INVALID;

  val_ffe1->dscp = 1;
  val_ffe2->dscp = 1;

  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, Dscp_Mismatch) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID;

  val_ffe1->dscp = 2;
  val_ffe2->dscp = 1;

  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, Dscp_Match) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                             ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                             ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID;

  val_ffe1->dscp = 1;
  val_ffe2->dscp = 1;

  EXPECT_EQ(UPLL_RC_SUCCESS,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, Priority_Invalid) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_INVALID;

  val_ffe1->priority = 1;
  val_ffe2->priority = 1;

  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, Priority_Mismatch) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;

  val_ffe1->priority = 2;
  val_ffe2->priority = 1;

  EXPECT_EQ(UPLL_RC_ERR_MERGE_CONFLICT,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, Priority_Match) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;

  val_ffe1->priority = 1;
  val_ffe2->priority = 1;

  EXPECT_EQ(UPLL_RC_SUCCESS,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

TEST_F(CompareValStructureTest, AllValues_Match) {
  CompareValStructureTest obj;
 
  val_vtn_flowfilter_entry_t *val_ffe1 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);
  val_vtn_flowfilter_entry_t *val_ffe2 =
                              ZALLOC_TYPE(val_vtn_flowfilter_entry_t);

  val_ffe1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;

  strncpy((char*) val_ffe1->flowlist_name,"FlowlistName", 32);
  strncpy((char*) val_ffe2->flowlist_name,"FlowlistName", 32);

  val_ffe1->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;

  val_ffe1->action = 1;
  val_ffe2->action = 1;

  val_ffe1->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;

  strncpy((char*) val_ffe1->nwm_name,"nwm", 32);
  strncpy((char*) val_ffe2->nwm_name,"nwm", 32);

  val_ffe1->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_VALID;

  val_ffe1->dscp = 1;
  val_ffe2->dscp = 1;


  val_ffe1->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;
  val_ffe2->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_VALID;

  val_ffe1->priority = 1;
  val_ffe2->priority = 1;

  EXPECT_EQ(UPLL_RC_SUCCESS,
          obj.CompareValStructure(val_ffe1, val_ffe2));;
  free(val_ffe1);
  free(val_ffe2);
}

