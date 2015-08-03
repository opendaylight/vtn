/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "include/flowlist_stub.hh"
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
std::map<function, upll_rc_t> FlowlistTest::stub_result;

//bool fatal_done;
//pfc::core::Mutex  fatal_mutex_lock;
//Test cases
TEST_F(FlowlistTest, MergeVal_ikey_NULL) {
  unc_key_type_t keytype = UNC_KT_FLOWLIST;
  const char *ctrl_id = "pfc1";

  ConfigKeyVal *configkey = NULL;
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  DalDmlIntf  *dmi = new DalOdbcMgr();
  FlowListMoMgr flowlist_obj;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.MergeValidate(keytype,
            ctrl_id, configkey, dmi, import_type));
  delete dmi;
}

TEST_F(FlowlistTest, MergeVal_ctr_id_NULL) {
  unc_key_type_t keytype = UNC_KT_FLOWLIST;
  const char *ctrl_id = NULL;

  ConfigKeyVal *configkey = NULL;
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  DalDmlIntf  *dmi = new DalOdbcMgr();
  FlowListMoMgr flowlist_obj;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlist_obj.MergeValidate(keytype,
            ctrl_id, configkey, dmi, import_type));
  delete dmi;
}

TEST_F(FlowlistTest, ValidateImp_DB_ACCESS) {
  FlowlistTest::stub_result.clear();
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  FlowlistTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  FlowlistTest::stub_result[ValidateImportWithRunning1] =
                                                UPLL_RC_ERR_DB_ACCESS;
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlistmomgr.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, ValidateImp_success) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  FlowlistTest::stub_result[UpdateConfigDB1] = UPLL_RC_SUCCESS;
  FlowlistTest::stub_result[ValidateImportWithRunning1] = UPLL_RC_SUCCESS;
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlistmomgr.MergeValidate(keytype,
                                            ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, ValidateImp_NO_INS) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  FlowlistTest::stub_result[UpdateConfigDB1] = UPLL_RC_SUCCESS;
  FlowlistTest::stub_result[ValidateImportWithRunning1] =
                                    UPLL_RC_ERR_NO_SUCH_INSTANCE;
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, flowlistmomgr.MergeValidate(keytype,
                                            ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, ValidateImp_Merge_Conflict) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);

  DalDmlIntf  *dmi = new DalOdbcMgr();

  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  FlowlistTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  FlowlistTest::stub_result[ValidateImportWithRunning1] =
                                           UPLL_RC_ERR_MERGE_CONFLICT;
  
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlistmomgr.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, Read_NO_INS) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  DalDmlIntf  *dmi = new DalOdbcMgr();
  
  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlistmomgr.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, Read_db_error_DB_ACCESS) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);

  DalDmlIntf  *dmi = new DalOdbcMgr();
  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_DB_ACCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlistmomgr.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, Read_INS_EXISTS) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);

  DalDmlIntf  *dmi = new DalOdbcMgr();
  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlistmomgr.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, Read_success) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);

  DalDmlIntf  *dmi = new DalOdbcMgr();
  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlistmomgr.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, MergeVal_Generic) {
  unc_key_type_t keytype = UNC_KT_FLOWLIST;
  const char *ctrl_id = "PFC1";

  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
 
  DalDmlIntf  *dmi = new DalOdbcMgr();
  FlowListMoMgr flowlist_obj;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlist_obj.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  }

TEST_F(FlowlistTest, Update_NO_INS) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  DalDmlIntf  *dmi = new DalOdbcMgr();

  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  FlowlistTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlistmomgr.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, Update_INS_EXISTS) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  DalDmlIntf  *dmi = new DalOdbcMgr();
  
  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  FlowlistTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_INSTANCE_EXISTS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlistmomgr.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, Update_Db_Access) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  DalDmlIntf  *dmi = new DalOdbcMgr();
  
  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  FlowlistTest::stub_result[UpdateConfigDB1] = UPLL_RC_ERR_DB_ACCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlistmomgr.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, Update_success) {
  FlowlistTest flowlistmomgr;
  unc_key_type_t keytype = UNC_KT_FLOWLIST;

  const char *ctrl_id = "pfc1";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
  DalDmlIntf  *dmi = new DalOdbcMgr();

  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  FlowlistTest::stub_result[UpdateConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlistmomgr.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
  FlowlistTest::stub_result.clear();
}

TEST_F(FlowlistTest, MergeVal_ctr_empty) {
  unc_key_type_t keytype = UNC_KT_FLOWLIST;
  const char *ctrl_id = "";
  upll_import_type import_type = UPLL_IMPORT_TYPE_PARTIAL;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));
  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));;

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
 
  DalDmlIntf  *dmi = new DalOdbcMgr();
  FlowListMoMgr flowlist_obj;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            flowlist_obj.MergeValidate(keytype, ctrl_id, ikey, dmi, import_type));
  delete dmi;
  delete ikey;
}

TEST_F(FlowlistTest, ReadCtrlrTbl) {
  FlowlistTest obj;
  DalDmlIntf *dmi = new DalOdbcMgr();
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;

  key_flowlist_t *flowlist_key1 = reinterpret_cast<key_flowlist_t*>
                                  (malloc(sizeof(key_flowlist_t)));

  memset(flowlist_key1, 0, sizeof(key_flowlist_t));
  val_flowlist_t *fl_val = reinterpret_cast<val_flowlist_t *>
                           (malloc(sizeof(val_flowlist_t)));

  ConfigVal *val = new ConfigVal(IpctSt::kIpcStKeyFlowlist, fl_val);

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                       IpctSt::kIpcStKeyFlowlist, flowlist_key1, val);
//  const char * ctrlr_name = "PFC";
//  ikey->set_user_data((void*)ctrlr_name);

  FlowlistTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(UNC_UPLL_RC_ERR_GENERIC, obj.ReadCtrlrTbl(ikey, dmi, dt_type));
  FlowlistTest::stub_result.clear();
}
