/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "include/vtermif_stub.hh"
#include "ut_util.hh"


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
std::map<function, upll_rc_t> VtermIfMoMgrTest::stub_result;
std::map<function, upll_rc_t> VtnMoMgrTest::stub_result;

//bool fatal_done;
//pfc::core::Mutex  fatal_mutex_lock;

/*=========== GetRenameKeyBindInfoRenameTbl ======================*/
TEST_F(VtermIfMoMgrTest, GetRenameKeyBindInfoMAINTBL) {
  BindInfo     *binfo;
  int          nattr;
  VtermIfMoMgr vtermifmomgr;

  EXPECT_TRUE(vtermifmomgr.GetRenameKeyBindInfo(UNC_KT_VTN, binfo,
                                                nattr, MAINTBL));
}

TEST_F(VtermIfMoMgrTest, GetRenameKeyBindInfoRenameTbl) {
  BindInfo     *binfo;
  int          nattr;
  VtermIfMoMgr vtermifmomgr;

  EXPECT_FALSE(vtermifmomgr.GetRenameKeyBindInfo(UNC_KT_VTN, binfo,
                                                 nattr, RENAMETBL));
}

/*=========== UpdateMo ======================*/
TEST_F(VtermIfMoMgrTest, UpdateMo) {
  VtermIfMoMgrTest obj;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.UpdateMo(NULL, NULL, NULL));
}

/*=========== IsValidKey ======================*/
TEST(VtermIfMoMgrTest_IsValidKey, VTNNameIndex_VTNName_syntax_fail) {
  VtermIfMoMgr     vtermifmomgr;
  key_vterm_if_t   key;

  const char* name = "_VTN";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vtn_key.vtn_name),
  name, strlen(name)+1);
  EXPECT_FALSE(vtermifmomgr.IsValidKey((void *)&key,
               uudst::vterminal_interface::kDbiVtnName));
}

TEST(VtermIfMoMgrTest_IsValidKey, VTERMINALNameIndex_VterminalName_syntax_fail) {
  VtermIfMoMgr     vtermifmomgr;
  key_vterm_if_t   key;

  const char* name = "VTN123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vtn_key.vtn_name),
  name, strlen(name)+1);

  name = "_VTERM";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vterminal_name),
  name, strlen(name)+1);
  EXPECT_FALSE(vtermifmomgr.IsValidKey((void *)&key,
               uudst::vterminal_interface::kDbiVterminalName));
}

TEST(VtermIfMoMgrTest_IsValidKey, VTERM_IFNameIndex_IfName_syntax_fail) {
  VtermIfMoMgr     vtermifmomgr;
  key_vterm_if_t   key;

  const char* name = "VTN123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vtn_key.vtn_name),
  name, strlen(name)+1);

  name = "VTERM123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vterminal_name),
  name, strlen(name)+1);

  name = "_IF12";
  strncpy(reinterpret_cast<char *>(key.if_name),
  name, strlen(name)+1);
  EXPECT_FALSE(vtermifmomgr.IsValidKey((void *)&key,
               uudst::vterminal_interface::kDbiIfName));
}

TEST(VtermIfMoMgrTest_IsValidKey, VTNIndexSuc) {
  VtermIfMoMgr     vtermifmomgr;
  key_vterm_if_t   key;

  const char* name = "VTN123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vtn_key.vtn_name),
  name, strlen(name)+1);

  name = "VTERM123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vterminal_name),
  name, strlen(name)+1);

  name = "IF1234";
  strncpy(reinterpret_cast<char *>(key.if_name),
  name, strlen(name)+1);
  EXPECT_TRUE(vtermifmomgr.IsValidKey((void *)&key,
              uudst::vterminal_interface::kDbiVtnName));
}

TEST(VtermIfMoMgrTest_IsValidKey, VTERMINALIndexSuc) {
  VtermIfMoMgr     vtermifmomgr;
  key_vterm_if_t   key;

  const char* name = "VTN123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vtn_key.vtn_name),
  name, strlen(name)+1);

  name = "VTERM123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vterminal_name),
  name, strlen(name)+1);

  name = "IF1234";
  strncpy(reinterpret_cast<char *>(key.if_name),
  name, strlen(name)+1);
  EXPECT_TRUE(vtermifmomgr.IsValidKey((void *)&key,
              uudst::vterminal_interface::kDbiVterminalName));
}

TEST(VtermIfMoMgrTest_IsValidKey, IfIndexSuc) {
  VtermIfMoMgr     vtermifmomgr;
  key_vterm_if_t   key;

  const char* name = "VTN123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vtn_key.vtn_name),
  name, strlen(name)+1);

  name = "VTERM123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vterminal_name),
  name, strlen(name)+1);

  name = "IF1234";
  strncpy(reinterpret_cast<char *>(key.if_name),
  name, strlen(name)+1);
  EXPECT_TRUE(vtermifmomgr.IsValidKey((void *)&key,
              uudst::vterminal_interface::kDbiIfName));
}

TEST(VtermIfMoMgrTest_IsValidKey, InvalidIndex) {
  VtermIfMoMgr     vtermifmomgr;
  key_vterm_if_t   key;

  const char* name = "VTN123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vtn_key.vtn_name),
  name, strlen(name)+1);

  name = "VTERM123";
  strncpy(reinterpret_cast<char *>(key.vterm_key.vterminal_name),
  name, strlen(name)+1);

  name = "IF1234";
  strncpy(reinterpret_cast<char *>(key.if_name),
  name, strlen(name)+1);
  EXPECT_TRUE(vtermifmomgr.IsValidKey((void *)&key, 124));
}

/* ======================= GetChildConfigKey=================== */
TEST(VtermIfMoMgrTest_GetChildConfigKey, ParentOkeyNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetChildConfigKey(okey, NULL));
  EXPECT_TRUE(okey);
  EXPECT_TRUE(okey->get_key());
  EXPECT_EQ(okey->get_key_type(), UNC_KT_VTERM_IF);
  delete okey;
}

TEST(VtermIfMoMgrTest_GetChildConfigKey, ParentKeyStructNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates Parent key */
  pkey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.GetChildConfigKey(okey, pkey));
  delete pkey;
  delete okey;
}

TEST(VtermIfMoMgrTest_GetChildConfigKey, OkeyExistsKeyTypeInvalid) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates Parent key */
  key_vterm_if_t *vterm_key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));;
  pkey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, vterm_key, NULL);

  /* Populates Existing key */
  okey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.GetChildConfigKey(okey, pkey));
  delete pkey;
  delete okey;
}

TEST(VtermIfMoMgrTest_GetChildConfigKey, OkeyKeyStructNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates Parent key is VTERM_IF */
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));;
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));
  pkey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                          IpctSt::kIpcStKeyVtermIf, key, NULL);

  /* Populates Existing key is VTERM_IF */
  okey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.GetChildConfigKey(okey, pkey));
  delete pkey;
  delete okey;
}
TEST_F(VtermIfMoMgrTest, GetChildConfigKeyOkeyKeyStructvalues_pkey_VTN) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

 /* Populates Parent key is VTN */
  key_vtn_t *key = reinterpret_cast<key_vtn_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));;
  strncpy(reinterpret_cast<char*>(key->vtn_name), "VTN_NEW", strlen("VTN_NEW"));
  pkey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, NULL);

  /* Populates Existing key is VTERM_IF */
  key_vterm_if_t *okey_key = reinterpret_cast<key_vterm_if_t *>
         (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));;
  strncpy(reinterpret_cast<char*>(okey_key->vterm_key.vtn_key.vtn_name),
          "VTN_OLD", strlen("VTN_OLD"));
  strncpy(reinterpret_cast<char*>(okey_key->vterm_key.vterminal_name),
          "VTERM_OLD", strlen("VTERM_OLD"));
  strncpy(reinterpret_cast<char*>(okey_key->if_name),
          "IF_OLD", strlen("IF_OLD"));
  okey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                          IpctSt::kIpcStKeyVtermIf, okey_key, NULL);

  /* Validates return value and key type and key struct */
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetChildConfigKey(okey, pkey));
  EXPECT_TRUE(okey);
  EXPECT_TRUE(okey->get_key());
  EXPECT_EQ(okey->get_key_type(), UNC_KT_VTERM_IF);

  okey_key = reinterpret_cast<key_vterm_if_t *>(okey->get_key());
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vtn_key.vtn_name), "VTN_NEW"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vterminal_name), "VTERM_OLD"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->if_name), "IF_OLD"));
  delete pkey;
  delete okey;
}
TEST_F(VtermIfMoMgrTest, GetChildConfigKeyOkeyKeyStructvalues_pkey_VTERMINAL) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates Parent key is VTERMINAL */
  key_vterm_t *key = reinterpret_cast<key_vterm_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_t)));;
  strncpy(reinterpret_cast<char*>(key->vtn_key.vtn_name),
          "VTN_NEW", strlen("VTN_NEW"));
  strncpy(reinterpret_cast<char*>(key->vterminal_name),
          "VTERM_NEW", strlen("VTERM_NEW"));
  pkey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
                          key, NULL);

  /* Populates Existing key is VTERM_IF */
  key_vterm_if_t *okey_key = reinterpret_cast<key_vterm_if_t *>
         (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));;
  strncpy(reinterpret_cast<char*>(okey_key->vterm_key.vtn_key.vtn_name),
          "VTN_OLD", strlen("VTN_OLD"));
  strncpy(reinterpret_cast<char*>(okey_key->vterm_key.vterminal_name),
          "VTERM_OLD", strlen("VTERM_OLD"));
  strncpy(reinterpret_cast<char*>(okey_key->if_name),
          "IF_OLD", strlen("IF_OLD"));
  okey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                          IpctSt::kIpcStKeyVtermIf, okey_key, NULL);

  /* Validates return value and key type and key struct */
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetChildConfigKey(okey, pkey));
  EXPECT_TRUE(okey);
  EXPECT_TRUE(okey->get_key());
  EXPECT_EQ(okey->get_key_type(), UNC_KT_VTERM_IF);

  okey_key = reinterpret_cast<key_vterm_if_t *>(okey->get_key());
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vtn_key.vtn_name), "VTN_NEW"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vterminal_name), "VTERM_NEW"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->if_name), "IF_OLD"));
  delete pkey;
  delete okey;
}

TEST_F(VtermIfMoMgrTest, GetChildConfigKeyOkeyKeyStructvalues_pkey_VTERMIF) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates Parent key is VTERM_IF */
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));;
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "VTN_NEW", strlen("VTN_NEW"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "VTERM_NEW", strlen("VTERM_NEW"));
  strncpy(reinterpret_cast<char*>(key->if_name), "IF_NEW", strlen("IF_NEW"));
  pkey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                          IpctSt::kIpcStKeyVtermIf, key, NULL);

  /* Populates Existing key is VTERM_IF */
  key_vterm_if_t *okey_key = reinterpret_cast<key_vterm_if_t *>
         (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));;
  strncpy(reinterpret_cast<char*>(okey_key->vterm_key.vtn_key.vtn_name),
          "VTN_OLD", strlen("VTN_OLD"));
  strncpy(reinterpret_cast<char*>(okey_key->vterm_key.vterminal_name),
          "VTERM_OLD", strlen("VTERM_OLD"));
  strncpy(reinterpret_cast<char*>(okey_key->if_name),
          "IF_OLD", strlen("IF_OLD"));
  okey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                          IpctSt::kIpcStKeyVtermIf, okey_key, NULL);

  /* Validates return value and key type and key struct */
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetChildConfigKey(okey, pkey));
  EXPECT_TRUE(okey);
  EXPECT_TRUE(okey->get_key());
  EXPECT_EQ(okey->get_key_type(), UNC_KT_VTERM_IF);

  okey_key = reinterpret_cast<key_vterm_if_t *>(okey->get_key());
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vtn_key.vtn_name), "VTN_NEW"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vterminal_name), "VTERM_NEW"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->if_name), "IF_NEW"));
  delete pkey;
  delete okey;
}

TEST(VtermIfMoMgrTest_GetChildConfigKey, ParentVTN_OkeyNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates Parent key is VTN */
  key_vtn_t *key = reinterpret_cast<key_vtn_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));;
  strncpy(reinterpret_cast<char*>(key->vtn_name), "vtn1", strlen("vtn1"));
  pkey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key, NULL);

  /* Validates return value and key type and key struct */
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetChildConfigKey(okey, pkey));
  EXPECT_TRUE(okey);
  EXPECT_TRUE(okey->get_key());
  EXPECT_EQ(okey->get_key_type(), UNC_KT_VTERM_IF);
  key_vterm_if_t *okey_key = NULL;
  okey_key = reinterpret_cast<key_vterm_if_t *>(okey->get_key());
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vtn_key.vtn_name), "vtn1"));
  delete pkey;
  delete okey;
}

TEST(VtermIfMoMgrTest_GetChildConfigKey, ParentVterminal_okeyNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates Parent key is VTERMINAL */
  key_vterm_t *key = reinterpret_cast<key_vterm_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_t)));;
  strncpy(reinterpret_cast<char*>(key->vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterminal_name),
          "vterm1", strlen("vterm1"));
  pkey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal, key, NULL);

  /* Validates return value and key type and key struct */
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetChildConfigKey(okey, pkey));
  EXPECT_TRUE(okey);
  EXPECT_TRUE(okey->get_key());
  EXPECT_EQ(okey->get_key_type(), UNC_KT_VTERM_IF);
  key_vterm_if_t *okey_key = NULL;
  okey_key = reinterpret_cast<key_vterm_if_t *>(okey->get_key());
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vtn_key.vtn_name), "vtn1"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vterminal_name), "vterm1"));
  delete pkey;
  delete okey;
}

TEST(VtermIfMoMgrTest_GetChildConfigKey, ParentKeyVtermIf_OkeyNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates Parent key is VTERM_IF */
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));;
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));
  pkey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVterminal,
                          key, NULL);

  /* Validates return value and key type and key struct */
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetChildConfigKey(okey, pkey));
  EXPECT_TRUE(okey);
  EXPECT_TRUE(okey->get_key());
  EXPECT_EQ(okey->get_key_type(), UNC_KT_VTERM_IF);
  key_vterm_if_t *okey_key = NULL;
  okey_key = reinterpret_cast<key_vterm_if_t *>(okey->get_key());
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vtn_key.vtn_name), "vtn1"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vterminal_name), "vterm1"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(okey_key->if_name), "if1"));
  delete pkey;
  delete okey;
}

TEST(VtermIfMoMgrTest_GetChildConfigKey, OkeyKeyStructWithEmptyValues) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates Parent key is VTERM_IF */
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));;
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));
  pkey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                          IpctSt::kIpcStKeyVtermIf, key, NULL);

  /* Populates Existing key is VTERM_IF */
  key_vterm_if_t *okey_key = reinterpret_cast<key_vterm_if_t *>
         (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));;
  okey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                          okey_key, NULL);

  /* Validates return value and key type and key struct */
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetChildConfigKey(okey, pkey));
  EXPECT_TRUE(okey);
  EXPECT_TRUE(okey->get_key());
  EXPECT_EQ(okey->get_key_type(), UNC_KT_VTERM_IF);
  okey_key = reinterpret_cast<key_vterm_if_t *>(okey->get_key());
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vtn_key.vtn_name), "vtn1"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterm_key.vterminal_name), "vterm1"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(okey_key->if_name), "if1"));
  delete pkey;
  delete okey;
}

TEST(VtermIfMoMgrTest_GetChildConfigKey, InvalidParentkeytype) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates Parent key is VTN */
  pkey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                          NULL, NULL);

  /* Validates return value and key type and key struct */
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.GetChildConfigKey(okey, pkey));
  delete pkey;
}

/* ============== GetParentConfigKey ======================= */
TEST(VtermIfMoMgrTest_GetParentConfigKey, ikeyNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtermifmomgr.GetParentConfigKey(okey, NULL));
}

TEST(VtermIfMoMgrTest_GetParentConfigKey, ikeyKeyStructNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates ikey */
  pkey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                          NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.GetParentConfigKey(okey, pkey));

  delete pkey;
}

TEST(VtermIfMoMgrTest_GetParentConfigKey, KeyTypeNotVtermIf) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates ikey with invalid key type */
  pkey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                          IpctSt::kIpcStKeyVtermIf, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.GetParentConfigKey(okey, pkey));

  delete pkey;
}

TEST(VtermIfMoMgrTest_GetParentConfigKey, ikeyVtermIf) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *pkey = NULL;

  /* Populates ikey with keytype is VTERM_IF */
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));;
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  pkey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                          IpctSt::kIpcStKeyVtermIf, key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetParentConfigKey(okey, pkey));
  /* Validates return value and key type and key struct */
  EXPECT_TRUE(okey);
  EXPECT_TRUE(okey->get_key());
  EXPECT_EQ(okey->get_key_type(), UNC_KT_VTERMINAL);
  key_vterm_t *okey_key;
  okey_key = reinterpret_cast<key_vterm_t *>(okey->get_key());
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vtn_key.vtn_name), "vtn1"));
  EXPECT_FALSE(strcmp(reinterpret_cast<char*>(
              okey_key->vterminal_name), "vterm1"));

  delete pkey;
  delete okey;
}

/* ================= AllocVal ====================== */
TEST(VtermIfMoMgrTest_AllocVal, CfgValNotNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigVal        *val = NULL;

  val = new ConfigVal(IpctSt::kIpcStValVtermIf, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtermifmomgr.AllocVal(val, UPLL_DT_STATE, MAINTBL));
  delete val;
}

/* =================== AllocVal =================== */
TEST(VtermIfMoMgrTest_AllocVal, CfgValNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigVal        *val = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS,
            vtermifmomgr.AllocVal(val, UPLL_DT_STATE, MAINTBL));
  EXPECT_TRUE(val);
  EXPECT_TRUE(val->next_cfg_val_);
  delete val;
}

/* ================== DupConfigKeyVal ================ */
TEST(VtermIfMoMgrTest_DupConfigKeyVal, ReqNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigKeyVal     *req  = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtermifmomgr.DupConfigKeyVal(okey, req, MAINTBL));
}

TEST(VtermIfMoMgrTest_DupConfigKeyVal, okeyNotNULL) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                       IpctSt::kIpcStKeyVtermIf, NULL, NULL);
  ConfigKeyVal     *req  = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtermifmomgr.DupConfigKeyVal(okey, req, MAINTBL));
  delete okey;
}

TEST(VtermIfMoMgrTest_DupConfigKeyVal, ReqInvalidKeytype) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;

  /* Populates ikey as a invalid keytype */
  ConfigKeyVal     *req  = new ConfigKeyVal(UNC_KT_VBR_IF,
                IpctSt::kIpcStKeyVtermIf, NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtermifmomgr.DupConfigKeyVal(okey, req, MAINTBL));
  delete req;
}

TEST(VtermIfMoMgrTest_DupConfigKeyVal, ReqKeyStructNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;

  ConfigKeyVal     *req  = new ConfigKeyVal(UNC_KT_VTERM_IF,
                      IpctSt::kIpcStKeyVtermIf, NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtermifmomgr.DupConfigKeyVal(okey, req, MAINTBL));
  delete req;
}

TEST(VtermIfMoMgrTest_DupConfigKeyVal, ReqValStructNULL) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  ConfigVal        *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, NULL);
  ConfigKeyVal     *req  = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, NULL, val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtermifmomgr.DupConfigKeyVal(okey, req, MAINTBL));
  delete req;
}

TEST(VtermIfMoMgrTest_DupConfigKeyVal, ReqValStructInvalidStNum) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  val_vterm_if_t   *val_vterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));

  ConfigVal        *val  = new ConfigVal(IpctSt::kIpcStValVbrIf, val_vterm);
  ConfigKeyVal     *req  = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtermifmomgr.DupConfigKeyVal(okey, req, MAINTBL));
  delete req;
}

TEST(VtermIfMoMgrTest_DupConfigKeyVal, ReqValStStructNULL) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;
  val_vterm_if_t   *val_vterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  ConfigVal        *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, val_vterm);
  ConfigVal        *val_st  = new ConfigVal(IpctSt::kIpcStValVtermIfSt, NULL);
  val->AppendCfgVal(val_st);
  ConfigKeyVal     *req  = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtermifmomgr.DupConfigKeyVal(okey, req, MAINTBL));
  delete req;
}

TEST_F(VtermIfMoMgrTest, DupConfigKeyValSucc) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal     *okey = NULL;

  /* Populate vterm_if ConfigKeyVal */
  val_vterm_if_t   *val_vterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  ConfigVal        *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, val_vterm);
  val_vterm_if_st_t *val_vterm_st = reinterpret_cast<val_vterm_if_st_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_st)));

  ConfigVal        *val_st  = new ConfigVal(IpctSt::kIpcStValVtermIfSt, val_vterm_st);
  val->AppendCfgVal(val_st);
  key_vterm_if_t      *key     =  reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));
  ConfigKeyVal     *req  = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                            IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.DupConfigKeyVal(okey, req, MAINTBL));
  delete okey;
  delete req;
}

/* ================ UpdateConfigStatus ================ */
TEST(VtermIfMoMgrTest_UpdateConfigStatus, ikeyNull) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_CREATE, 0,
                                            upd_key, dmi, ctlr_key));
}
/*
TEST(VtermIfMoMgrTest_UpdateConfigStatus, OpCreate_DriverResultSucc_Valid) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  // Populate DAL interface //
  dmi = new DalOdbcMgr();

  // Populate vterm_if ConfigKeyVal //
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_CREATE,
            0, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_APPLIED);

  val_db_vterm_if_st *vtermif_valdbst = reinterpret_cast<val_db_vterm_if_st *>(
      ikey->get_cfg_val()->get_next_cfg_val()->get_val());
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.oper_status,UPLL_OPER_STATUS_UNINIT);
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.valid[UPLL_IDX_OPER_STATUS_VTERMIS],
            UNC_VF_VALID);

  delete ikey;
  delete upd_key;
  delete dmi;
}
*/
TEST(VtermIfMoMgrTest_UpdateConfigStatus, OpCreate_DriverResultSucc_ValidNoValue) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populate vterm_if ConfigKeyVal */
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_CREATE,
            0, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_APPLIED);
  val_db_vterm_if_st *vtermif_valdbst = reinterpret_cast<val_db_vterm_if_st *>(
      ikey->get_cfg_val()->get_next_cfg_val()->get_val());
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.oper_status,UPLL_OPER_STATUS_UNINIT);
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.valid[UPLL_IDX_OPER_STATUS_VTERMIS],
            UNC_VF_VALID);

  delete ikey;
  delete upd_key;
  delete dmi;
}

TEST(VtermIfMoMgrTest_UpdateConfigStatus, OpCreate_DriverResultSucc_InValid) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populate vterm_if ConfigKeyVal */
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_CREATE,
            0, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_APPLIED);
  val_db_vterm_if_st *vtermif_valdbst = reinterpret_cast<val_db_vterm_if_st *>(
      ikey->get_cfg_val()->get_next_cfg_val()->get_val());
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.oper_status,UPLL_OPER_STATUS_UNINIT);
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.valid[UPLL_IDX_OPER_STATUS_VTERMIS],
            UNC_VF_VALID);

  delete ikey;
  delete upd_key;
  delete dmi;
}
/*
TEST(VtermIfMoMgrTest_UpdateConfigStatus, OpCreateDriverResultFail_Valid) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  // Populate DAL interface //
  dmi = new DalOdbcMgr();

  // Populate vterm_if ConfigKeyVal //
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_CREATE,
            1, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_NOT_APPLIED);
  val_db_vterm_if_st *vtermif_valdbst = reinterpret_cast<val_db_vterm_if_st *>(
      ikey->get_cfg_val()->get_next_cfg_val()->get_val());
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.oper_status,UPLL_OPER_STATUS_UNINIT);
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.valid[UPLL_IDX_OPER_STATUS_VTERMIS],
            UNC_VF_VALID);

  delete ikey;
  delete upd_key;
  delete dmi;
}
*/
TEST(VtermIfMoMgrTest_UpdateConfigStatus, OperationCreateDriverResultFailValidNoValue) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populate vterm_if ConfigKeyVal */
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_CREATE,
            1, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_NOT_APPLIED);
  val_db_vterm_if_st *vtermif_valdbst = reinterpret_cast<val_db_vterm_if_st *>(
      ikey->get_cfg_val()->get_next_cfg_val()->get_val());
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.oper_status,UPLL_OPER_STATUS_UNINIT);
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.valid[UPLL_IDX_OPER_STATUS_VTERMIS],
            UNC_VF_VALID);

  delete ikey;
  delete upd_key;
  delete dmi;
}

TEST(VtermIfMoMgrTest_UpdateConfigStatus, OperationCreateDriverResultFailInValid) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populate vterm_if ConfigKeyVal */
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_CREATE,
            1, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_APPLIED);
  val_db_vterm_if_st *vtermif_valdbst = reinterpret_cast<val_db_vterm_if_st *>(
      ikey->get_cfg_val()->get_next_cfg_val()->get_val());
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.oper_status,UPLL_OPER_STATUS_UNINIT);
  EXPECT_EQ(vtermif_valdbst->vterm_if_val_st.valid[UPLL_IDX_OPER_STATUS_VTERMIS],
            UNC_VF_VALID);

  delete ikey;
  delete upd_key;
  delete dmi;
}
/*
TEST(VtermIfMoMgrTest_UpdateConfigStatus, OperationUpdateDriverResultSucc_Valid) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  // Populate DAL interface //
  dmi = new DalOdbcMgr();

  // Populate vterm_if ConfigKeyVal //
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  u_vterm_val->cs_row_status = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_PM_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM]= UNC_CS_NOT_APPLIED;
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
            0, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(vterm_val->cs_row_status, UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_APPLIED);
  delete ikey;
  delete upd_key;
  delete dmi;
}
*/
TEST(VtermIfMoMgrTest_UpdateConfigStatus, OperationUpdateDriverResultSucc_InValid_RunnApllied) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populate vterm_if ConfigKeyVal */
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  u_vterm_val->cs_row_status = UNC_CS_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_CS_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI] = UNC_CS_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_PM_VTERMI] = UNC_CS_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_CS_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM] = UNC_CS_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM]= UNC_CS_APPLIED;
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
            0, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(vterm_val->cs_row_status, UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_APPLIED);
  delete ikey;
  delete upd_key;
  delete dmi;
}
TEST(VtermIfMoMgrTest_UpdateConfigStatus, OpUpdateDriverResultSucc_InValid_RunningNotApllied) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populate vterm_if ConfigKeyVal */
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  u_vterm_val->cs_row_status = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_PM_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM]= UNC_CS_NOT_APPLIED;
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
            0, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(vterm_val->cs_row_status, UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_NOT_APPLIED);
  delete ikey;
  delete upd_key;
  delete dmi;
}
/*
TEST(VtermIfMoMgrTest_UpdateConfigStatus, OperationUpdateDriverResultSucc_ValidNovalue) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  // Populate DAL interface //
  dmi = new DalOdbcMgr();

  // Populate vterm_if ConfigKeyVal //
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  u_vterm_val->cs_row_status = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_PM_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM]= UNC_CS_NOT_APPLIED;
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
            0, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(vterm_val->cs_row_status, UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_APPLIED);
  delete ikey;
  delete upd_key;
  delete dmi;
}

TEST(VtermIfMoMgrTest_UpdateConfigStatus, OperationUpdateDriverResultFail) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  // Populate DAL interface //
  dmi = new DalOdbcMgr();

  // Populate vterm_if ConfigKeyVal //
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);
  u_vterm_val->cs_row_status = UNC_CS_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_PM_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM]= UNC_CS_NOT_APPLIED;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
            1, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_NOT_APPLIED);
  delete ikey;
  delete upd_key;
  delete dmi;
}

TEST(VtermIfMoMgrTest_UpdateConfigStatus, OperationUpdateDriverResultFailInvalid) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  // Populate DAL interface //
  dmi = new DalOdbcMgr();

  / Populate vterm_if ConfigKeyVal /
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);
  u_vterm_val->cs_row_status = UNC_CS_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_PM_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM]= UNC_CS_NOT_APPLIED;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_UPDATE,
            1, upd_key, dmi, ctlr_key));
  vterm_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_NOT_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_NOT_APPLIED);
  delete ikey;
  delete upd_key;
  delete dmi;
}
*/
TEST(VtermIfMoMgrTest_UpdateConfigStatus, InvalidOperation) {
  VtermIfMoMgr      vtermifmomgr;
  ConfigKeyVal      *ikey     = NULL;
  ConfigKeyVal      *upd_key  = NULL;
  ConfigKeyVal      *ctlr_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populate vterm_if ConfigKeyVal */
  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
  ConfigVal *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf, NULL, val);

  val_vterm_if_t   *u_vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, u_vterm_val);
  upd_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);
  u_vterm_val->cs_row_status = UNC_CS_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->cs_attr[UPLL_IDX_PM_VTERMI] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM] = UNC_CS_NOT_APPLIED;
  u_vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM]= UNC_CS_NOT_APPLIED;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.UpdateConfigStatus(ikey, UNC_OP_DELETE,
            1, upd_key, dmi, ctlr_key));
  delete ikey;
  delete upd_key;
  delete dmi;
}

/* ================ UpdateAuditConfigStatus ================ */
TEST(VtermIfMoMgrTest_UpdateAuditConfigStatus, Null) {
  VtermIfMoMgr      obj;
  ConfigKeyVal      *runn_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  EXPECT_EQ(obj.UpdateAuditConfigStatus(UNC_CS_APPLIED, uuc::kUpllUcpCreate,
                                        runn_key, dmi), UPLL_RC_ERR_GENERIC);
}

TEST(VtermIfMoMgrTest_UpdateAuditConfigStatus, CreatePhaseCsInvalid) {
  VtermIfMoMgr      obj;
  ConfigKeyVal      *runn_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  val_vterm_if_t   *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  runn_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);
 vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;

  EXPECT_EQ(obj.UpdateAuditConfigStatus(UNC_CS_INVALID, uuc::kUpllUcpCreate,
                                        runn_key, dmi), UPLL_RC_SUCCESS);
  vterm_val =  reinterpret_cast<val_vterm_if_t *>(GetVal(runn_key));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_INVALID);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_INVALID);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_INVALID);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_INVALID);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_INVALID);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_INVALID);
  delete runn_key;
}

TEST(VtermIfMoMgrTest_UpdateAuditConfigStatus, CreatePhaseCsApllied) {
  VtermIfMoMgr      obj;
  ConfigKeyVal      *runn_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  val_vterm_if_t   *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  runn_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);
 vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;

  EXPECT_EQ(obj.UpdateAuditConfigStatus(UNC_CS_APPLIED, uuc::kUpllUcpCreate,
                                        runn_key, dmi), UPLL_RC_SUCCESS);
  vterm_val =  reinterpret_cast<val_vterm_if_t *>(GetVal(runn_key));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_APPLIED);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_APPLIED);
  delete runn_key;
}

TEST(VtermIfMoMgrTest_UpdateAuditConfigStatus, UpdatePhaseCsNotApplied) {
  VtermIfMoMgr      obj;
  ConfigKeyVal      *runn_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  val_vterm_if_t   *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  runn_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);
  vterm_val->cs_row_status = UNC_CS_INVALID;
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;

  EXPECT_EQ(obj.UpdateAuditConfigStatus(UNC_CS_NOT_APPLIED, uuc::kUpllUcpUpdate,
                                        runn_key, dmi), UPLL_RC_SUCCESS);
  vterm_val =  reinterpret_cast<val_vterm_if_t *>(GetVal(runn_key));
  delete runn_key;
}

TEST(VtermIfMoMgrTest_UpdateAuditConfigStatus, UpdatePhaseCsInValid) {
  VtermIfMoMgr      obj;
  ConfigKeyVal      *runn_key = NULL;
  DalDmlIntf        *dmi      = NULL;

  val_vterm_if_t   *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal  *u_val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  runn_key  =  new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, u_val);
  vterm_val->cs_row_status = UNC_CS_INVALID;
  vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;

  EXPECT_EQ(obj.UpdateAuditConfigStatus(UNC_CS_INVALID, uuc::kUpllUcpUpdate,
                                        runn_key, dmi), UPLL_RC_SUCCESS);
  vterm_val =  reinterpret_cast<val_vterm_if_t *>(GetVal(runn_key));

  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_CS_INVALID);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_DESC_VTERMI], UNC_CS_INVALID);
  EXPECT_EQ(vterm_val->cs_attr[UPLL_IDX_PM_VTERMI], UNC_CS_INVALID);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM], UNC_CS_INVALID);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_VLAN_ID_PM], UNC_CS_INVALID);
  EXPECT_EQ(vterm_val->portmap.cs_attr[UPLL_IDX_TAGGED_PM], UNC_CS_INVALID);
  delete runn_key;
}

/* ================ ValidateMessage ================ */
TEST_F(VtermIfMoMgrTest, ValidateMessageCkvReqNull) {
  VtermIfMoMgr  vtermifmomgr;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.ValidateMessage(NULL, NULL));
}

TEST_F(VtermIfMoMgrTest, ValidateMessageCkvNull) {
  VtermIfMoMgr  vtermifmomgr;
  IpcReqRespHeader  req;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.ValidateMessage(&req, NULL));
}
TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyNull) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigKeyVal      *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                            IpctSt::kIpcStKeyVtermIf, NULL, NULL);

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyInvalidStNum) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));
  
  ConfigKeyVal      *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                   IpctSt::kIpcStKeyVterminal, key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyInvalidKeytype) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));
  
  ConfigKeyVal      *ckv = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                     IpctSt::kIpcStKeyVtermIf, key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyInvalidOperation) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));
 
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  
  req.operation = (unc_keytype_operation_t)12;
  ConfigKeyVal      *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                            IpctSt::kIpcStKeyVtermIf, key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,
            vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyValidateFail) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));
 
  req.operation = (unc_keytype_operation_t)12;
  ConfigKeyVal      *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                   IpctSt::kIpcStKeyVtermIf, key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyCreateOperationSucc) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));
 
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_CREATE;
  req.datatype = UPLL_DT_CANDIDATE;
  ConfigKeyVal      *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                            IpctSt::kIpcStKeyVtermIf, key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyDeleteOperationSucc) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));
 
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_DELETE;
  req.datatype = UPLL_DT_CANDIDATE;
  ConfigKeyVal      *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                  IpctSt::kIpcStKeyVtermIf, key, NULL);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyUpdateOperationNoValStruct) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));
 
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_UPDATE;
  req.datatype = UPLL_DT_CANDIDATE;
  ConfigKeyVal      *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                      IpctSt::kIpcStKeyVtermIf, key, NULL);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyCreateOperationValStructFail) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
 
  valvterm->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
 
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  
  req.operation = UNC_OP_CREATE;
  req.datatype = UPLL_DT_CANDIDATE;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyCreateOperationValStructSucc) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
 
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
 
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  
  req.operation = UNC_OP_CREATE;
  req.datatype = UPLL_DT_CANDIDATE;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                              IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}


TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyUpdateOperationValStructFail) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
 
  valvterm->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
 
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name), "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name), "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  
  req.operation = UNC_OP_UPDATE;
  req.datatype = UPLL_DT_CANDIDATE;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyUpdateOperationValStructSucc) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
 
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
 
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name), "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name), "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  
  req.operation = UNC_OP_UPDATE;
  req.datatype = UPLL_DT_CANDIDATE;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, CkvKeyUpdateOperationInvalidValSt) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
 
  val = new ConfigVal(IpctSt::kIpcStValVbrIf, valvterm);
 
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_UPDATE;
  req.datatype = UPLL_DT_CANDIDATE;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadOperationInvalidOption1) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_READ;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option1 = (unc_keytype_option1_t)2;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}


TEST(VtermIfMoMgrTest_ValidateMessage, ReadOperationWithoutVal) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_READ;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadOperationWithInvalidVal) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  valvterm->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
  req.operation = UNC_OP_READ;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadOperationSucc) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
  req.operation = UNC_OP_READ;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibOperationInvalidOption1) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_READ_SIBLING;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option1 = (unc_keytype_option1_t)2;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}


TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibOperationWithoutVal) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_READ_SIBLING;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibOperationWithInvalidVal) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  valvterm->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
  req.operation = UNC_OP_READ_SIBLING;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibOperationSucc) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
  req.operation = UNC_OP_READ_SIBLING;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibBeginOperationInvalidOption1) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_READ_SIBLING_BEGIN;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option1 = (unc_keytype_option1_t)2;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}


TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibBeginOperationWithoutVal) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_READ_SIBLING_BEGIN;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibBeginOperationWithInvalidVal) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  valvterm->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
  req.operation = UNC_OP_READ_SIBLING_BEGIN;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibBeginOperationSucc) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
  req.operation = UNC_OP_READ_SIBLING_BEGIN;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibCountOperationInvalidOption1) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;
   
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_READ_SIBLING_COUNT;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option1 = (unc_keytype_option1_t)2;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, vtermifmomgr.ValidateMessage(&req, ckv));
  delete ckv;
}


TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibCountOperationWithoutVal) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  req.operation = UNC_OP_READ_SIBLING_COUNT;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibCountOperationWithInvalidVal) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  valvterm->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
  req.operation = UNC_OP_READ_SIBLING_COUNT;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}

TEST(VtermIfMoMgrTest_ValidateMessage, ReadSibCountOperationSucc) {
  VtermIfMoMgr      vtermifmomgr;
  IpcReqRespHeader  req;
  ConfigVal         *val = NULL;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
  req.operation = UNC_OP_READ_SIBLING_COUNT;
  req.datatype = UPLL_DT_CANDIDATE;
  req.option2  = UNC_OPT2_NONE;
  req.option1  = UNC_OPT1_NORMAL;
  ConfigKeyVal *ckv = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                       IpctSt::kIpcStKeyVtermIf, key, val);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateMessage(&req, ckv));

  delete ckv;
}
/* ================================= ValidateVtermIfKey ==========*/
TEST_F(VtermIfMoMgrTest, ValidateVtermIfKeySuccessCreateOp) {
  VtermIfMoMgr  vtermifmomgr;
  key_vterm_if  key;
  const char    *vtn_name       = "VTN_1";
  const char    *vterminal_name = "VTERM_1";
  const char    *if_name        = "IF_1";
  memset(&key,0,sizeof(key_vterm_if));

  strncpy(reinterpret_cast<char *>(key.vterm_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key.vterm_key.vterminal_name),
  vterminal_name, strlen(vterminal_name)+1);

  strncpy(reinterpret_cast<char *>(key.if_name),
  if_name, strlen(if_name)+1);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateVtermIfKey(&key, UNC_OP_CREATE));
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfKeySuccessReadSibilingBeginOp) {
  VtermIfMoMgr  vtermifmomgr;
  key_vterm_if  key;
  const char    *vtn_name       = "VTN_1";
  const char    *vterminal_name = "VTERM_1";
  memset(&key,0,sizeof(key_vterm_if));

  strncpy(reinterpret_cast<char *>(key.vterm_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key.vterm_key.vterminal_name),
  vterminal_name, strlen(vterminal_name)+1);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateVtermIfKey(&key,
            UNC_OP_READ_SIBLING_BEGIN));
  EXPECT_STREQ(reinterpret_cast<char *>(key.if_name), "");
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfKeySuccessReadSibilingCountOp) {
  VtermIfMoMgr  vtermifmomgr;
  key_vterm_if  key;
  const char    *vtn_name       = "VTN_1";
  const char    *vterminal_name = "VTERM_1";
  memset(&key,0,sizeof(key_vterm_if));

  strncpy(reinterpret_cast<char *>(key.vterm_key.vtn_key.vtn_name),
  vtn_name, strlen(vtn_name)+1);
  strncpy(reinterpret_cast<char *>(key.vterm_key.vterminal_name),
  vterminal_name, strlen(vterminal_name)+1);

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateVtermIfKey(&key,
            UNC_OP_READ_SIBLING_COUNT));
  EXPECT_STREQ(reinterpret_cast<char *>(key.if_name), "");
}

/* =========================== ValidateVtermIfVal ============ */
TEST_F(VtermIfMoMgrTest, ValidateVtermIfValValidNoValueOpCreate) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  const char *des  = "des";
  const char *logical_port_id = "PP-000-000-00";
  memset(&val, 0, sizeof(val_vterm_if));
 
  strncpy(reinterpret_cast<char *>(val.description), des, strlen(des)+1);
  strncpy(reinterpret_cast<char *>(val.portmap.logical_port_id), logical_port_id,
         strlen(logical_port_id)+1);
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_VALID_NO_VALUE;
  val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  val.portmap.valid[UPLL_IDX_TAGGED_PM]  = UNC_VF_VALID_NO_VALUE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));
  EXPECT_STREQ(" ", reinterpret_cast<char *>(val.description));
  
  EXPECT_EQ(val.admin_status, UPLL_ADMIN_ENABLE);
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValValidNoValueOpUpdate) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  const char *des  = "des";
  const char *logical_port_id = "PP-000-000-00";
  memset(&val, 0, sizeof(val_vterm_if));
 
  strncpy(reinterpret_cast<char *>(val.description), des, strlen(des)+1);
  strncpy(reinterpret_cast<char *>(val.portmap.logical_port_id), logical_port_id,
         strlen(logical_port_id)+1);
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_VALID_NO_VALUE;
  val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  val.portmap.valid[UPLL_IDX_TAGGED_PM]  = UNC_VF_VALID_NO_VALUE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_UPDATE));
  EXPECT_STREQ(" ", reinterpret_cast<char *>(val.description));
  
  EXPECT_EQ(val.admin_status, UPLL_ADMIN_ENABLE);
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValDescValidFail) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  memset(&val, 0, sizeof(val_vterm_if));
 
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,
            vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValAdminStatusInvalid) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  const char *des  = "des";
  memset(&val, 0, sizeof(val_vterm_if));
 
  strncpy(reinterpret_cast<char *>(val.description), des, strlen(des)+1);
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));
 
  EXPECT_EQ(val.admin_status, UPLL_ADMIN_ENABLE);
  EXPECT_EQ(val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_VF_VALID_NO_VALUE);
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValAdminStatusFail) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  const char *des  = "des";
  memset(&val, 0, sizeof(val_vterm_if));
 
  strncpy(reinterpret_cast<char *>(val.description), des, strlen(des)+1);
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  val.admin_status = 3;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValPMValidMembersValidNoValue) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  const char *des  = "des";
  memset(&val, 0, sizeof(val_vterm_if));

  strncpy(reinterpret_cast<char *>(val.description), des, strlen(des)+1);
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_VALID;
  val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  val.portmap.valid[UPLL_IDX_TAGGED_PM]  = UNC_VF_VALID_NO_VALUE;

  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));

  EXPECT_STREQ(" ", reinterpret_cast<char*>(val.portmap.logical_port_id));
  EXPECT_EQ(0, val.portmap.vlan_id);
  EXPECT_EQ(UPLL_VLAN_UNTAGGED, val.portmap.tagged);
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValLogicalPortIdFail) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  const char *des  = "des";
  memset(&val, 0, sizeof(val_vterm_if));

  strncpy(reinterpret_cast<char *>(val.description), des, strlen(des)+1);
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_VALID;
  val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  val.portmap.valid[UPLL_IDX_TAGGED_PM]  = UNC_VF_VALID_NO_VALUE;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,
            vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValLogicalPortIdSwitch) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  const char *des  = "des";
  memset(&val, 0, sizeof(val_vterm_if));

  strncpy(reinterpret_cast<char *>(val.description), des, strlen(des)+1);
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_VALID;
  val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val.portmap.logical_port_id), "SW-");
  val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  val.portmap.valid[UPLL_IDX_TAGGED_PM]  = UNC_VF_VALID_NO_VALUE;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,
            vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValVlanIdInvalid) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  const char *des  = "des";
  memset(&val, 0, sizeof(val_vterm_if));

  strncpy(reinterpret_cast<char *>(val.description), des, strlen(des)+1);
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_VALID;
  val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val.portmap.logical_port_id), "PP-");
  val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val.portmap.vlan_id = 5000;
  val.portmap.valid[UPLL_IDX_TAGGED_PM]  = UNC_VF_VALID_NO_VALUE;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,
            vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValVlanTagInvalid) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  const char *des  = "des";
  memset(&val, 0, sizeof(val_vterm_if));

  strncpy(reinterpret_cast<char *>(val.description), des, strlen(des)+1);
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID_NO_VALUE;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_VALID;
  val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val.portmap.logical_port_id), "PP-");
  val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val.portmap.vlan_id = 10;
  val.portmap.valid[UPLL_IDX_TAGGED_PM]  = UNC_VF_VALID;
  val.portmap.tagged = 2;

  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX,
            vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValSuccValid) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  const char *des  = "des";
  memset(&val, 0, sizeof(val_vterm_if));

  strncpy(reinterpret_cast<char *>(val.description), des, strlen(des)+1);
  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  val.admin_status = 1;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_VALID;
  val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val.portmap.logical_port_id), "PP-");
  val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val.portmap.vlan_id = 10;
  val.portmap.valid[UPLL_IDX_TAGGED_PM]  = UNC_VF_VALID;
  val.portmap.tagged = 1;

  EXPECT_EQ(UPLL_RC_SUCCESS,
            vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));
}

TEST_F(VtermIfMoMgrTest, ValidateVtermIfValSuccInValid) {
  VtermIfMoMgr     vtermifmomgr;
  val_vterm_if_t   val;

  memset(&val, 0, sizeof(val_vterm_if));

  val.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  val.admin_status = 1;
  val.valid[UPLL_IDX_PM_VTERMI]   = UNC_VF_INVALID;
  val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
  strcpy(reinterpret_cast<char*>(val.portmap.logical_port_id), "PP-");
  val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
  val.portmap.vlan_id = 10;
  val.portmap.valid[UPLL_IDX_TAGGED_PM]  = UNC_VF_INVALID;
  val.portmap.tagged = 1;

  EXPECT_EQ(UPLL_RC_SUCCESS,
            vtermifmomgr.ValidateVtermIfValue(&val, UNC_OP_CREATE));
}
/* ====================== GetValid ================================== */
TEST(VtermIfMoMgrTest_GetValid, Null) {
  VtermIfMoMgr obj;

  uint8_t *valid = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetValid(NULL,
        (uint64_t)uudst::vterminal_interface::kDbiOperStatus,
        valid, UPLL_DT_STATE, MAINTBL));
 // EXPECT_EQ(valid, NULL);
}

TEST(VtermIfMoMgrTest_GetValid, oper_status_index) {
  VtermIfMoMgr obj;

  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  uint8_t *valid = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(reinterpret_cast<void *>(vterm_val),
        (uint64_t)uudst::vterminal_interface::kDbiOperStatus,
        valid, UPLL_DT_STATE, MAINTBL));
  EXPECT_EQ(valid, &(vterm_val->valid[UPLL_IDX_OPER_STATUS_VTERMIS]));

  FREE_IF_NOT_NULL(vterm_val);

}

TEST(VtermIfMoMgrTest_GetValid, down_count_index) {
  VtermIfMoMgr obj;

  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  uint8_t *valid = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(reinterpret_cast<void *>(vterm_val),
        (uint64_t)uudst::vterminal_interface::kDbiDownCount,
        valid, UPLL_DT_STATE, MAINTBL));
 // EXPECT_EQ(valid, NULL);

  FREE_IF_NOT_NULL(vterm_val);

}
TEST(VtermIfMoMgrTest_GetValid, admin_status) {
  VtermIfMoMgr obj;

  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  uint8_t *valid = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(reinterpret_cast<void *>(vterm_val),
        (uint64_t)uudst::vterminal_interface::kDbiAdminStatus,
        valid, UPLL_DT_STATE, MAINTBL));
  EXPECT_EQ(valid, &(vterm_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI]));

  FREE_IF_NOT_NULL(vterm_val);

}
TEST(VtermIfMoMgrTest_GetValid, Desc_index) {
  VtermIfMoMgr obj;

  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  uint8_t *valid = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(reinterpret_cast<void *>(vterm_val),
        (uint64_t)uudst::vterminal_interface::kDbiDesc,
        valid, UPLL_DT_STATE, MAINTBL));
  EXPECT_EQ(valid, &(vterm_val->valid[UPLL_IDX_DESC_VTERMI]));

  FREE_IF_NOT_NULL(vterm_val);

}
TEST(VtermIfMoMgrTest_GetValid, Portmap_index) {
  VtermIfMoMgr obj;

  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  uint8_t *valid = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(reinterpret_cast<void *>(vterm_val),
        (uint64_t)uudst::vterminal_interface::kDbiValidPortMap,
        valid, UPLL_DT_STATE, MAINTBL));
  EXPECT_EQ(valid, &(vterm_val->valid[UPLL_IDX_PM_VTERMI]));

  FREE_IF_NOT_NULL(vterm_val);

}
TEST(VtermIfMoMgrTest_GetValid, logical_prt_id_index) {
  VtermIfMoMgr obj;

  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  uint8_t *valid = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(reinterpret_cast<void *>(vterm_val),
        (uint64_t)uudst::vterminal_interface::kDbiLogicalPortId,
        valid, UPLL_DT_STATE, MAINTBL));
  EXPECT_EQ(valid, &(vterm_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]));

  FREE_IF_NOT_NULL(vterm_val);

}
TEST(VtermIfMoMgrTest_GetValid, Vlanid_index) {
  VtermIfMoMgr obj;

  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  uint8_t *valid = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(reinterpret_cast<void *>(vterm_val),
        (uint64_t)uudst::vterminal_interface::kDbiVlanId,
        valid, UPLL_DT_STATE, MAINTBL));
  EXPECT_EQ(valid, &(vterm_val->portmap.valid[UPLL_IDX_VLAN_ID_PM]));

  FREE_IF_NOT_NULL(vterm_val);

}
TEST(VtermIfMoMgrTest_GetValid, vlantagg_index) {
  VtermIfMoMgr obj;

  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  uint8_t *valid = NULL;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(reinterpret_cast<void *>(vterm_val),
        (uint64_t)uudst::vterminal_interface::kDbiTagged,
        valid, UPLL_DT_STATE, MAINTBL));
  EXPECT_EQ(valid, &(vterm_val->portmap.valid[UPLL_IDX_TAGGED_PM]));

  FREE_IF_NOT_NULL(vterm_val);

}
TEST(VtermIfMoMgrTest_GetValid, invalid_index) {
  VtermIfMoMgr obj;

  val_vterm_if_t  *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  uint8_t *valid = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            obj.GetValid(reinterpret_cast<void *>(vterm_val),
        (uint64_t)100,
        valid, UPLL_DT_STATE, MAINTBL));

  FREE_IF_NOT_NULL(vterm_val);
}

/* ========================= AdaptValToVtnServce =================== */
TEST(VtermIfMoMgrTest_AdaptValToVtnService, ikeyNull) {
  VtermIfMoMgr obj;

  ConfigKeyVal *ikey = NULL;
  AdaptType adaptType = ADAPT_ALL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey, adaptType));
}

TEST(VtermIfMoMgrTest_AdaptValToVtnService, CfgValNull) {
  VtermIfMoMgr obj;
  AdaptType adaptType = ADAPT_ALL;

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                        IpctSt::kIpcStKeyVtermIf);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey, adaptType));
  delete ikey;
}

TEST(VtermIfMoMgrTest_AdaptValToVtnService, CfgValValStructNull) {
  VtermIfMoMgr obj;

  val_vterm_if_t *valvterm = NULL;
  AdaptType adaptType = ADAPT_ALL;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, cfg_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey, adaptType));
  delete ikey;
}
/*
TEST(VtermIfMoMgrTest_AdaptValToVtnService, CfgValDbValStructNull) {
  VtermIfMoMgr obj;
  val_db_vterm_if_st *db_vterm_if_val_st = NULL;
  AdaptType adaptType = ADAPT_ALL;
  ConfigVal *cfg_db_val = new ConfigVal(IpctSt::kIpcStValVtermIfSt,
                                        db_vterm_if_val_st);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, cfg_db_val);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AdaptValToVtnService(ikey, adaptType));
  delete ikey;
}
*/
TEST(VtermIfMoMgrTest_AdaptValToVtnService, Succ) {
  VtermIfMoMgr obj;
  AdaptType adaptType = ADAPT_ALL;

  val_vterm_if_t *valvterm = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  valvterm->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtermIf, valvterm);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, cfg_val);

  val_db_vterm_if_st *db_vterm_if_val_st = reinterpret_cast<val_db_vterm_if_st *>
                           (ConfigKeyVal::Malloc(sizeof(val_db_vterm_if_st)));
  ConfigVal *cfg_db_val = new ConfigVal(IpctSt::kIpcStValVtermIfSt,
                                        db_vterm_if_val_st);
  ConfigKeyVal *ckv_next = new ConfigKeyVal(UNC_KT_VTERM_IF,
                               IpctSt::kIpcStKeyVtermIf, NULL, cfg_db_val);
  ikey->AppendCfgKeyVal(ckv_next);

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AdaptValToVtnService(ikey, adaptType));
  valvterm = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(valvterm->valid[UPLL_IDX_ADMIN_STATUS_VTERMI],
            UNC_VF_VALID_NO_VALUE);

  delete ikey;
}

/* =========================== IsReferenced ======================= */
TEST(VtermIfMoMgrTest_IsReferenced, Null) {
  VtermIfMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->datatype = UPLL_DT_STATE;

  EXPECT_EQ(UPLL_RC_SUCCESS, obj.IsReferenced(req, NULL, NULL));
}

/* =========================== GetPortMap ========================= */
TEST_F(VtermIfMoMgrTest, GetPortMapikeyNull) {
  VtermIfMoMgr     vtermifmomgr;
  uint8_t          valid_pm;
  uint8_t          valid_admin;
  uint8_t          admin;
  val_port_map_t   *pm = NULL;
  ConfigKeyVal     *ikey = NULL;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.GetPortMap(ikey, valid_pm, pm,
                                                         valid_admin, admin));
}

TEST_F(VtermIfMoMgrTest, GetPortMapikeyValNull) {
  VtermIfMoMgr     vtermifmomgr;
  uint8_t          valid_pm;
  uint8_t          valid_admin;
  uint8_t          admin;
  val_port_map_t   *pm = NULL;
  ConfigKeyVal     *ikey = NULL;

  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.GetPortMap(ikey, valid_pm, pm,
                                                         valid_admin, admin));
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, GetPortMapValid) {
  VtermIfMoMgr     vtermifmomgr;
  uint8_t          valid_pm;
  uint8_t          valid_admin;
  uint8_t          admin;
  val_port_map_t   *pm = NULL;
  ConfigKeyVal     *ikey = NULL;

  val_vterm_if_t   *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal        *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  vterm_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetPortMap(ikey, valid_pm, pm,
                                                         valid_admin, admin));
  EXPECT_TRUE(pm);
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, GetPortMapNotValid) {
  VtermIfMoMgr     vtermifmomgr;
  uint8_t          valid_pm;
  uint8_t          valid_admin;
  uint8_t          admin;
  val_port_map_t   *pm = NULL;
  ConfigKeyVal     *ikey = NULL;

  val_vterm_if_t   *vterm_val =  reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal        *val   =  new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val);
  ikey  =  new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.GetPortMap(ikey, valid_pm, pm,
                                                         valid_admin, admin));
  EXPECT_FALSE(pm);
  delete ikey;
}

/* ============================== CopyToConfigKey ================== */
TEST_F(VtermIfMoMgrTest, CopyToConfigKeyIkeyNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal*    okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.CopyToConfigKey(okey, NULL));
}

TEST_F(VtermIfMoMgrTest, CopyToConfigKeyIkeykeystructNull) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal*    okey = NULL;
  ConfigKeyVal*    ikey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                           IpctSt::kIpcStKeyVtermIf, NULL, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.CopyToConfigKey(okey, ikey));
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, CopyToConfigKeyOldVtnnameEmpty) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal*    okey = NULL;
  key_rename_vnode_info   *key =  reinterpret_cast<key_rename_vnode_info *>
                      (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));


  ConfigKeyVal*    ikey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                           IpctSt::kIpcStKeyVtermIf, key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.CopyToConfigKey(okey, ikey));
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, CopyToConfigKeyVterminalKeyEmptyName) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal*    okey = NULL;
  key_rename_vnode_info   *key =  reinterpret_cast<key_rename_vnode_info *>
                         (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));

  strncpy(reinterpret_cast<char*>(key->old_unc_vtn_name), "vtn1", strlen("vtn1"));
  ConfigKeyVal*    ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                           IpctSt::kIpcStKeyVtermIf, key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.CopyToConfigKey(okey, ikey));
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, CopyToConfigKeyVterminalIfKeyEmptyName) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal*    okey = NULL;
  key_rename_vnode_info   *key =  reinterpret_cast<key_rename_vnode_info *>
                         (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));

  strncpy(reinterpret_cast<char*>(key->old_unc_vtn_name), "vtn1", strlen("vtn1"));
  ConfigKeyVal*    ikey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                           IpctSt::kIpcStKeyVtermIf, key, NULL);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, vtermifmomgr.CopyToConfigKey(okey, ikey));
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, CopyToConfigKeySuccVTERMIF) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal*    okey = NULL;
  key_rename_vnode_info   *key =  reinterpret_cast<key_rename_vnode_info *>
                         (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));

  strncpy(reinterpret_cast<char*>(key->old_unc_vtn_name), "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->new_unc_vnode_name), "vterm1", strlen("vterm1"));
  ConfigKeyVal*    ikey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                           IpctSt::kIpcStKeyVtermIf, key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.CopyToConfigKey(okey, ikey));

  key_vterm_if_t *vtermif_key =
      reinterpret_cast<key_vterm_if_t *>(okey->get_key());
  EXPECT_EQ(0, strcmp(reinterpret_cast<char*>(
              vtermif_key->vterm_key.vtn_key.vtn_name), "vtn1"));
  EXPECT_EQ(0, strcmp(reinterpret_cast<char*>(
              vtermif_key->vterm_key.vterminal_name), "vterm1"));
  delete okey;
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, CopyToConfigKeySuccVTERMINAL) {
  VtermIfMoMgr     vtermifmomgr;
  ConfigKeyVal*    okey = NULL;
  key_rename_vnode_info   *key =  reinterpret_cast<key_rename_vnode_info *>
                         (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));

  strncpy(reinterpret_cast<char*>(key->old_unc_vtn_name), "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->old_unc_vnode_name), "vterm1", strlen("vterm1"));
  strncpy(reinterpret_cast<char*>(key->new_unc_vnode_name), "vterm2", strlen("vterm2"));
  ConfigKeyVal*    ikey = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                           IpctSt::kIpcStKeyVtermIf, key, NULL);
  EXPECT_EQ(UPLL_RC_SUCCESS, vtermifmomgr.CopyToConfigKey(okey, ikey));

  key_vterm_if_t *vtermif_key =
      reinterpret_cast<key_vterm_if_t *>(okey->get_key());
  EXPECT_EQ(0, strcmp(reinterpret_cast<char*>
                      (vtermif_key->vterm_key.vtn_key.vtn_name), "vtn1"));
  EXPECT_EQ(0, strcmp(reinterpret_cast<char*>(
              vtermif_key->vterm_key.vterminal_name), "vterm1"));
  delete okey;
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, FilterAttributesNotCreateOp) {
  VtermIfMoMgr     vtermifmomgr;
  void   *val1 = ConfigKeyVal::Malloc(sizeof(val_vterm_if_t));
  val_vterm_if_t   *val2 = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));

  EXPECT_TRUE(vtermifmomgr.FilterAttributes(val1,
              (void *)val2, false, UNC_OP_UPDATE));
  free(val1);
  free(val2);
}

TEST_F(VtermIfMoMgrTest, FilterAttributesCreateOperation) {
  VtermIfMoMgr     vtermifmomgr;
  void   *val1 = ConfigKeyVal::Malloc(sizeof(val_vterm_if_t));;

  EXPECT_FALSE(vtermifmomgr.FilterAttributes(val1,
               NULL, false, UNC_OP_CREATE));
  free(val1);
}

TEST_F(VtermIfMoMgrTest, CompareValidValueInvalidPortMapAttr) {
  VtermIfMoMgr     vtermifmomgr;

  void   *val1 = (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  val_vterm_if_t   *val2 = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));

  val_vterm_if_t *vterm_if_val1 = reinterpret_cast<val_vterm_if_t *>(val1);

  vterm_if_val1->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;

  val2->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val2->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  val2->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;


  EXPECT_FALSE(vtermifmomgr.CompareValidValue(val1, (void *)val2, false));
  free(val1);
  free(val2);
}

TEST_F(VtermIfMoMgrTest, CompareValidValuePortMapNotModified) {
  VtermIfMoMgr     vtermifmomgr;

  void   *val1 = (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  val_vterm_if_t   *val2 = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));

  val_vterm_if_t *vterm_if_val1 = reinterpret_cast<val_vterm_if_t *>(val1);

  vterm_if_val1->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;

  val2->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val2->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  val2->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;


  EXPECT_FALSE(vtermifmomgr.CompareValidValue(val1, (void *)val2, false));
  free(val1);
  free(val2);
}

TEST_F(VtermIfMoMgrTest, CompareValidValuePortMapLogicalPortIdModified) {
  VtermIfMoMgr     vtermifmomgr;

  void   *val1 = (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  val_vterm_if_t   *val2 = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));

  val_vterm_if_t *vterm_if_val1 = reinterpret_cast<val_vterm_if_t *>(val1);

  vterm_if_val1->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char *>(vterm_if_val1->portmap.logical_port_id), "PP-0000",
         strlen("PP-0000")+1);
  vterm_if_val1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;

  val2->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val2->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  val2->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;


  EXPECT_FALSE(vtermifmomgr.CompareValidValue(val1, (void *)val2, false));
  EXPECT_EQ(4, UNC_VF_VALUE_NOT_MODIFIED);
  EXPECT_EQ(4, UNC_VF_VALUE_NOT_MODIFIED);
  free(val1);
  free(val2);
}

TEST_F(VtermIfMoMgrTest, CompareValidValuePortMapVlaniddModified) {
  VtermIfMoMgr     vtermifmomgr;

  void   *val1 = (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  val_vterm_if_t   *val2 = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));

  val_vterm_if_t *vterm_if_val1 = reinterpret_cast<val_vterm_if_t *>(val1);

  vterm_if_val1->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_if_val1->portmap.vlan_id = 10;
  vterm_if_val1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;

  val2->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val2->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  val2->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;


  EXPECT_FALSE(vtermifmomgr.CompareValidValue(val1, (void *)val2, false));
  EXPECT_EQ(4,
              UNC_VF_VALUE_NOT_MODIFIED);
  EXPECT_EQ(4,
            UNC_VF_VALUE_NOT_MODIFIED);
  free(val1);
  free(val2);
}

TEST_F(VtermIfMoMgrTest, CompareValidValuePortMapVlanTagModified) {
  VtermIfMoMgr     vtermifmomgr;

  void   *val1 = (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  val_vterm_if_t   *val2 = reinterpret_cast<val_vterm_if_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));

  val_vterm_if_t *vterm_if_val1 = reinterpret_cast<val_vterm_if_t *>(val1);

  vterm_if_val1->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  vterm_if_val1->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  vterm_if_val1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  vterm_if_val1->portmap.tagged = 1;
  vterm_if_val1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;

  val2->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  val2->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  val2->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val2->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;


  EXPECT_FALSE(vtermifmomgr.CompareValidValue(val1, (void *)val2, false));
  EXPECT_EQ(4,
            UNC_VF_VALUE_NOT_MODIFIED);
  EXPECT_EQ(4,
            UNC_VF_VALUE_NOT_MODIFIED);
  free(val1);
  free(val2);
}

/* =================== IsVtermIfAlreadyExists ============= */
TEST(VtermIfMoMgrTest_IsVtermIfAlreadyExists, NoEntry) {
  VtermIfMoMgrTest vtermifmomgr;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf   *dmi      = NULL;
  IpcReqRespHeader req;

  memset(&req, 0, sizeof(IpcReqRespHeader));

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, key);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  EXPECT_EQ(vtermifmomgr.IsVtermIfAlreadyExists(ikey, dmi, &req), UPLL_RC_ERR_GENERIC);

  delete dmi;
  delete ikey;
  VtermIfMoMgrTest::stub_result.clear();
}
TEST(VtermIfMoMgrTest_IsVtermIfAlreadyExists, EntryPresent) {
  VtermIfMoMgrTest vtermifmomgr;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf   *dmi      = NULL;
  IpcReqRespHeader req;

  memset(&req, 0, sizeof(IpcReqRespHeader));

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, key);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(vtermifmomgr.IsVtermIfAlreadyExists(ikey, dmi, &req), UPLL_RC_ERR_GENERIC);

  delete dmi;
  delete ikey;
  VtermIfMoMgrTest::stub_result.clear();
}
TEST(VtermIfMoMgrTest_IsVtermIfAlreadyExists, DB_Error) {
  VtermIfMoMgrTest vtermifmomgr;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf   *dmi      = NULL;
  IpcReqRespHeader req;

  memset(&req, 0, sizeof(IpcReqRespHeader));

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, key);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_GENERIC;
  EXPECT_EQ(vtermifmomgr.IsVtermIfAlreadyExists(ikey, dmi, &req), UPLL_RC_ERR_GENERIC);

  delete dmi;
  delete ikey;
  VtermIfMoMgrTest::stub_result.clear();
}
TEST(VtermIfMoMgrTest_IsVtermIfAlreadyExists, GetChildConfigKeyError) {
  VtermIfMoMgrTest vtermifmomgr;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf   *dmi      = NULL;
  IpcReqRespHeader req;

  memset(&req, 0, sizeof(IpcReqRespHeader));

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, key);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_GENERIC;
  EXPECT_EQ(vtermifmomgr.IsVtermIfAlreadyExists(ikey, dmi, &req), UPLL_RC_ERR_GENERIC);

  delete dmi;
  delete ikey;
  VtermIfMoMgrTest::stub_result.clear();
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_ikeyNull) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetPortmapInfo(ikey, UPLL_DT_CANDIDATE,
                                                    dmi, iftype));
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_ikeyKeyStructNull) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  /* Populates Parent key is VTERM_IF */
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, NULL);

  /* verifies result code and output param */
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetPortmapInfo(ikey,
                                 UPLL_DT_CANDIDATE, dmi, iftype));
  /* Clean up*/
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_InvalidDatatype) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  /* Populates Parent key is VTERM_IF */
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf, key);

  /* verifies result code and output param */
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetPortmapInfo(ikey,
                        (upll_keytype_datatype_t)10, dmi, iftype));
  /* Clean up*/
  delete ikey;
}
/*
TEST_F(VtermIfMoMgrTest, GetPortMapInfo_DTCandidate_PortmapValid) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  // Populates Parent key is VTERM_IF //
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, key);
  testcase_id = GetPortmapValid;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  // verifies result code and output param //
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetPortmapInfo(ikey, UPLL_DT_CANDIDATE,
                                                    dmi, iftype));
  EXPECT_EQ(iftype, kPortMapConfigured);

  // clean up //
  delete ikey;
  testcase_id = 0;
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_DTImport_PortmapValid) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  // Populates Parent key is VTERM_IF //
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, key);
  testcase_id = GetPortmapValid;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  // verifies result code and output param //
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetPortmapInfo(ikey, UPLL_DT_IMPORT,
                                                    dmi, iftype));
  EXPECT_EQ(iftype, kPortMapConfigured);

  // clean up //
  delete ikey;
  testcase_id = 0;
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_DTRunning_PortmapValid) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  // Populates Parent key is VTERM_IF //
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, key);
  testcase_id = GetPortmapValid;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  // verifies result code and output param //
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetPortmapInfo(ikey, UPLL_DT_RUNNING,
                                                    dmi, iftype));
  EXPECT_EQ(iftype, kPortMapConfigured);

  // clean up //
  delete ikey;
  testcase_id = 0;
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_DTAudit_PortmapValid) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  // Populates Parent key is VTERM_IF //
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, key);
  testcase_id = GetPortmapValid;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  // verifies result code and output param //
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetPortmapInfo(ikey, UPLL_DT_AUDIT,
                                                    dmi, iftype));
  EXPECT_EQ(iftype, kPortMapConfigured);

  // clean up //
  delete ikey;
  testcase_id = 0;
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_DTCandidate_NoPortMap) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  // Populates Parent key is VTERM_IF //
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, key);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  // verifies result code and output param //
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetPortmapInfo(ikey, UPLL_DT_CANDIDATE,
                                                    dmi, iftype));
  EXPECT_EQ(iftype, kVlinkPortMapNotConfigured);

  // clean up//
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_DTRunning_DBNoEntry) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  // Populates Parent key is VTERM_IF /
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, key);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  // verifies result code and output param //
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetPortmapInfo(ikey, UPLL_DT_RUNNING,
                                                    dmi, iftype));
  EXPECT_EQ(iftype, kVlinkPortMapNotConfigured);

  // clean up//
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_DTImport_DBNoEntry) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  // Populates Parent key is VTERM_IF //
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, key);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  // verifies result code and output param //
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetPortmapInfo(ikey, UPLL_DT_IMPORT,
                                                    dmi, iftype));
  EXPECT_EQ(iftype, kVlinkPortMapNotConfigured);

  // clean up //
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_DTAudit_DBNoEntry) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  // Populates Parent key is VTERM_IF //
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, key);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  // verifies result code and output param //
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetPortmapInfo(ikey, UPLL_DT_AUDIT,
                                                    dmi, iftype));
  EXPECT_EQ(iftype, kVlinkPortMapNotConfigured);

  // clean up //
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, GetPortMapInfo_DBError) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  DalDmlIntf            *dmi   = NULL;
  InterfacePortMapInfo  iftype;

  // Populates Parent key is VTERM_IF //
  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, key);
  testcase_id = GetPortmapValid;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_GENERIC;

  // verifies result code and output param //
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetPortmapInfo(ikey, UPLL_DT_CANDIDATE,
                                                    dmi, iftype));

  // clean up//
  delete ikey;
}
*/
/* ============= ValVtermIfAttributeSupportCheck ============= */
TEST_F(VtermIfMoMgrTest, ValVtermIfAttributeSupportCheck_ikeyNull) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
               UNC_OP_CREATE, UPLL_DT_STATE), UPLL_RC_ERR_GENERIC);
}

TEST_F(VtermIfMoMgrTest, ValVtermIfAttributeSupportCheck_CfgValNull) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, NULL);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
               UNC_OP_CREATE, UPLL_DT_STATE), UPLL_RC_SUCCESS);

  /* cleanup */
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, ValVtermIfAttributeSupportCheck_ValStNull) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, NULL);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
               UNC_OP_CREATE, UPLL_DT_STATE), UPLL_RC_ERR_GENERIC);

  /* cleanup */
  delete ikey;
}

TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_DescAttValid_CreateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapDesc] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_CREATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  /* cleanup */
  delete ikey;
  free(attrs);
}

TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_DescAttValid_UpdateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapDesc] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_UPDATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  /* cleanup */
  delete ikey;
  free(attrs);
}

TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_AdminAttValid_CreateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapAdminStatus] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_CREATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  /* cleanup */
  delete ikey;
  free(attrs);
}

TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_AdminStatusAttValid_UpdateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapAdminStatus] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_UPDATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  /* cleanup */
  delete ikey;
  free(attrs);
}

TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_LogicalPortIdValid_CreateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vtermif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_CREATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  /* cleanup */
  delete ikey;
  free(attrs);
}

TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_LogicalPortIdAttValid_UpdateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vtermif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_UPDATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  /* cleanup */
  delete ikey;
  free(attrs);
}

TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_LogicalPortIdAttValid_DeleteOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vtermif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_DELETE, UPLL_DT_STATE), UPLL_RC_SUCCESS);
  vtermif_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(vtermif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM],
            UNC_VF_INVALID);

  /* cleanup */
  delete ikey;
  free(attrs);
}
/*
TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_VlanTagValid_CreateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vtermif_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_CREATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  delete ikey;
  free(attrs);
}

TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_VlanTagAttValid_UpdateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vtermif_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_UPDATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  delete ikey;
  free(attrs);
}

TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_VlanTagAttValid_DeleteOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
  vtermif_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_DELETE, UPLL_DT_STATE), UPLL_RC_SUCCESS);
  vtermif_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(vtermif_val->portmap.valid[UPLL_IDX_TAGGED_PM],
            UNC_VF_INVALID);

  delete ikey;
  free(attrs);
}
*/
TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_LogicalPortIdAttValidNoValue_UpdateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vtermif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_UPDATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  /* cleanup */
  delete ikey;
  free(attrs);
}
#if 0
TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_VlanTagAttValidNoValue_UpdateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vtermif_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_UPDATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  /* cleanup */
  delete ikey;
  free(attrs);
}

TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_VlanIdAttValidNoValue_UpdateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vtermif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_UPDATE, UPLL_DT_STATE), UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR);

  /* cleanup */
  delete ikey;
  free(attrs);
}
#endif
TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_LogicalPortIdAttValidNoValue_DeleteOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vtermif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_DELETE, UPLL_DT_STATE), UPLL_RC_SUCCESS);
  vtermif_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(vtermif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM],
            UNC_VF_INVALID);

  /* cleanup */
  delete ikey;
  free(attrs);
}
#if 0
TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_VlanIdAttValidNoValue_DeleteOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vtermif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_DELETE, UPLL_DT_STATE), UPLL_RC_SUCCESS);
  vtermif_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(vtermif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM],
            UNC_VF_INVALID);

  /* cleanup */
  delete ikey;
  free(attrs);
}
TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_VlanTagAttValidNoValue_DeleteOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID_NO_VALUE;
  vtermif_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_DELETE, UPLL_DT_STATE), UPLL_RC_SUCCESS);
  vtermif_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(vtermif_val->portmap.valid[UPLL_IDX_TAGGED_PM],
            UNC_VF_INVALID);

  /* cleanup */
  delete ikey;
  free(attrs);
}
#endif
TEST_F(VtermIfMoMgrTest,
       ValVtermIfAttributeSupportCheck_ValAttInValid_UpdateOp) {
  VtermIfMoMgrTest      obj;
  ConfigKeyVal          *ikey  = NULL;
  uint8_t               *attrs = NULL;

  attrs = (uint8_t *)malloc(6 * sizeof(uint8_t));
  attrs[unc::capa::vterm_if::kCapDesc] = 0;
  attrs[unc::capa::vterm_if::kCapAdminStatus] = 0;
  attrs[unc::capa::vterm_if::kCapLogicalPortId] = 0;
  attrs[unc::capa::vterm_if::kCapVlanId] = 0;
  attrs[unc::capa::vterm_if::kCapTagged] = 0;

  /* Populates VTERM_IF ConfigKeyVal */
  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if_t)));
  ConfigVal *val  = new ConfigVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  ikey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, val);

  /* Validates output values */
  EXPECT_EQ(obj.ValVtermIfAttributeSupportCheck(attrs, ikey,
         UNC_OP_UPDATE, UPLL_DT_STATE), UPLL_RC_SUCCESS);
  vtermif_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  EXPECT_EQ(vtermif_val->valid[UPLL_IDX_DESC_VTERMI], UNC_VF_INVALID);
  EXPECT_EQ(vtermif_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI], UNC_VF_INVALID);
  EXPECT_EQ(vtermif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM],
            UNC_VF_INVALID);
  EXPECT_EQ(vtermif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM],
            UNC_VF_INVALID);
  EXPECT_EQ(vtermif_val->portmap.valid[UPLL_IDX_TAGGED_PM],
            UNC_VF_INVALID);

  /* cleanup */
  delete ikey;
  free(attrs);
}

TEST_F(VtermIfMoMgrTest, ValidateCapability_ikeyNull) {
  VtermIfMoMgrTest        obj;
  ConfigKeyVal            *ikey  = NULL;
  IpcReqRespHeader        req;

  EXPECT_EQ(obj.ValidateCapability(&req, ikey, NULL), UPLL_RC_ERR_GENERIC);
}

TEST_F(VtermIfMoMgrTest, ValidateCapability_reqNull) {
  VtermIfMoMgrTest        obj;
  ConfigKeyVal            *ikey  =
      new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, NULL);

  EXPECT_EQ(obj.ValidateCapability(NULL, ikey, NULL), UPLL_RC_ERR_GENERIC);

  delete ikey;
}

TEST_F(VtermIfMoMgrTest, ValidateCapability_req_ikey_Null) {
  VtermIfMoMgrTest        obj;
  ConfigKeyVal            *ikey  = NULL;

  EXPECT_EQ(obj.ValidateCapability(NULL, ikey, NULL), UPLL_RC_ERR_GENERIC);
}

TEST_F(VtermIfMoMgrTest, ValidateCapability_ikey_userdata_ControllerId_Null) {
  VtermIfMoMgrTest        obj;
  IpcReqRespHeader        req;
  key_user_data_t *user_data =
      (key_user_data_t *)malloc(sizeof(key_user_data_t));
  memset(user_data, 0, sizeof(key_user_data_t));

  ConfigKeyVal            *ikey  =
      new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVtermIf, NULL, NULL);
  ikey->user_data_ = user_data;

  EXPECT_EQ(obj.ValidateCapability(&req, ikey, NULL), UPLL_RC_ERR_GENERIC);

  delete ikey;
}


TEST_F(VtermIfMoMgrTest, TranslateVbrIfToVtermIfError_ErrorCkv_Null) {
  VtermIfMoMgrTest  obj;
  ConfigKeyVal      *translated_ckv = NULL;
  ConfigKeyVal      *ckv_drv        = NULL;
  DalDmlIntf        *dmi            = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  uint8_t* ctrlr_id = NULL;
  EXPECT_EQ(obj.TranslateVbrIfToVtermIfError(translated_ckv, ckv_drv,
                  UPLL_DT_CANDIDATE, dmi, ctrlr_id), UPLL_RC_ERR_GENERIC);

  /* Clean up */
  delete dmi;
}


TEST_F(VtermIfMoMgrTest, TranslateVbrIfToVtermIfError_ErrorCkvKeyStruct_Null) {
  VtermIfMoMgrTest  obj;
  ConfigKeyVal      *translated_ckv = NULL;
  ConfigKeyVal      *ckv_drv        = NULL;
  DalDmlIntf        *dmi            = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populates driver error ConfigKeyVal */

  ckv_drv = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                             NULL, NULL);
  uint8_t* ctrlr_id = NULL;
  EXPECT_EQ(obj.TranslateVbrIfToVtermIfError(translated_ckv, ckv_drv,
                  UPLL_DT_CANDIDATE, dmi, ctrlr_id), UPLL_RC_ERR_GENERIC);

  /* Clean up */
  delete ckv_drv;
  delete dmi;
}

TEST_F(VtermIfMoMgrTest, TranslateVbrIfToVtermIfError_ErrorCkvValStruct_Null) {
  VtermIfMoMgrTest  obj;
  ConfigKeyVal      *translated_ckv = NULL;
  ConfigKeyVal      *ckv_drv        = NULL;
  DalDmlIntf        *dmi            = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populates driver error ConfigKeyVal */

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));

  ckv_drv = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVtermIf,
                             key, NULL);
  uint8_t* ctrlr_id = NULL;
  EXPECT_EQ(obj.TranslateVbrIfToVtermIfError(translated_ckv, ckv_drv,
                  UPLL_DT_CANDIDATE, dmi, ctrlr_id), UPLL_RC_ERR_NO_SUCH_INSTANCE);

  /* Clean up */
  delete ckv_drv;
  delete dmi;
}

TEST_F(VtermIfMoMgrTest, TranslateVbrIfToVtermIfError_ErrorConversionSucc) {
  VtermIfMoMgrTest  obj;
  ConfigKeyVal      *translated_ckv = NULL;
  ConfigKeyVal      *ckv_drv        = NULL;
  DalDmlIntf        *dmi            = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populates driver error ConfigKeyVal */

  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  val_drv_vbr_if *val = reinterpret_cast<val_drv_vbr_if *>
                            (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->vex_name), "vexternal");
  val->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->vex_if_name), "vexternal_if");
  val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->vbr_if_val.portmap.logical_port_id),
         "PP-0000-0000-0000-0001");
  val->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val->vbr_if_val.portmap.vlan_id = 10;
  val->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  val->vbr_if_val.portmap.tagged = 1;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);
  ckv_drv = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                             key, cfg_val);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  uint8_t* ctrlr_id = NULL;
  EXPECT_EQ(obj.TranslateVbrIfToVtermIfError(translated_ckv, ckv_drv,
                  UPLL_DT_CANDIDATE, dmi, ctrlr_id), UPLL_RC_ERR_GENERIC);
  VtermIfMoMgrTest::stub_result.clear();
  /* Clean up */
  delete translated_ckv;
  delete ckv_drv;
  delete dmi;
}

TEST_F(VtermIfMoMgrTest, TranslateVbrIfToVtermIfError_NoEntry) {
  VtermIfMoMgrTest  obj;
  ConfigKeyVal      *translated_ckv = NULL;
  ConfigKeyVal      *ckv_drv        = NULL;
  DalDmlIntf        *dmi            = NULL;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  /* Populates driver error ConfigKeyVal */

  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  val_drv_vbr_if *val = reinterpret_cast<val_drv_vbr_if *>
                            (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->vex_name), "vexternal");
  val->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->vex_if_name), "vexternal_if");
  val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->vbr_if_val.portmap.logical_port_id),
         "PP-0000-0000-0000-0001");
  val->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val->vbr_if_val.portmap.vlan_id = 10;
  val->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  val->vbr_if_val.portmap.tagged = 1;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);
  ckv_drv = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                             key, cfg_val);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  uint8_t* ctrlr_id = NULL;
  EXPECT_EQ(obj.TranslateVbrIfToVtermIfError(translated_ckv, ckv_drv,
                  UPLL_DT_CANDIDATE, dmi, ctrlr_id), UPLL_RC_ERR_GENERIC);
  VtermIfMoMgrTest::stub_result.clear();
  /* Clean up */
  delete ckv_drv;
  delete dmi;
}

TEST_F(VtermIfMoMgrTest, ValidateAttribute_ikeynull) {
  VtermIfMoMgrTest     obj;
  ConfigKeyVal         *ikey = NULL;
  DalDmlIntf           *dmi  = NULL;
  IpcReqRespHeader     req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();
  req.operation = UNC_OP_READ;

  EXPECT_EQ(obj.ValidateAttribute(ikey, dmi, &req), UPLL_RC_ERR_GENERIC);
  delete dmi;
}

TEST_F(VtermIfMoMgrTest, ValidateAttribute_OpCreate_valstructnull) {
  VtermIfMoMgrTest     obj;
  ConfigKeyVal         *ikey = NULL;
  DalDmlIntf           *dmi  = NULL;
  IpcReqRespHeader     req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();
  req.operation = UNC_OP_CREATE;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                             key, NULL);
  EXPECT_EQ(obj.ValidateAttribute(ikey, dmi, &req), UPLL_RC_SUCCESS);
  delete dmi;
  delete ikey;
}


TEST_F(VtermIfMoMgrTest, ValidateAttribute_OpDelete_valstructnull) {
  VtermIfMoMgrTest     obj;
  ConfigKeyVal         *ikey = NULL;
  DalDmlIntf           *dmi  = NULL;
  IpcReqRespHeader     req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();
  req.operation = UNC_OP_DELETE;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                             key, NULL);
  EXPECT_EQ(obj.ValidateAttribute(ikey, dmi, &req), UPLL_RC_SUCCESS);
  delete dmi;
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, ValidateAttribute_OpUpdate_ValStructNull) {
  VtermIfMoMgrTest     obj;
  ConfigKeyVal         *ikey = NULL;
  DalDmlIntf           *dmi  = NULL;
  IpcReqRespHeader     req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();
  req.operation = UNC_OP_UPDATE;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                          key, NULL);
  testcase_id = VtermLogicalPortIdSame;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  EXPECT_EQ(obj.ValidateAttribute(ikey, dmi, &req), UPLL_RC_ERR_GENERIC);
  delete dmi;
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, ValidateAttribute_OpUpdate_PortmapInfoSameVtermIf) {
  VtermIfMoMgrTest     obj;
  ConfigKeyVal         *ikey = NULL;
  DalDmlIntf           *dmi  = NULL;
  IpcReqRespHeader     req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();
  req.operation = UNC_OP_UPDATE;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));

  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vbr1", strlen("vbr1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if *val = reinterpret_cast<val_vterm_if *>
                            (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->portmap.logical_port_id),
         "PP-0000-0000-0000-0001");
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val->portmap.vlan_id = 10;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  val->portmap.tagged = 1;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtermIf, val);
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                          key, cfg_val);
  testcase_id = VtermLogicalPortIdSame;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  EXPECT_EQ(obj.ValidateAttribute(ikey, dmi, &req), UPLL_RC_ERR_GENERIC);
  delete dmi;
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, ValidateAttribute_OpUpdate_PortmapInfoSameVbrIf) {
  VtermIfMoMgrTest     obj;
  ConfigKeyVal         *ikey = NULL;
  DalDmlIntf           *dmi  = NULL;
  IpcReqRespHeader     req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();
  req.operation = UNC_OP_UPDATE;

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));

  val_vterm_if *val = reinterpret_cast<val_vterm_if *>
                            (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->portmap.logical_port_id),
         "PP-0000-0000-0000-0001");
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val->portmap.vlan_id = 10;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  val->portmap.tagged = 1;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtermIf, val);
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                          key, cfg_val);
  testcase_id = VtermLogicalPortIdSame;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;

  EXPECT_EQ(obj.ValidateAttribute(ikey, dmi, &req), UPLL_RC_ERR_GENERIC);
  delete dmi;
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, IsLogicalPortAndVlanIdInUse_ikey_null) {
  VtermIfMoMgrTest       obj;
  ConfigKeyVal           *ikey = NULL;
  DalDmlIntf             *dmi  = NULL;
  IpcReqRespHeader       req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  EXPECT_EQ(obj.IsLogicalPortAndVlanIdInUse<val_vbr_if_t>(ikey, dmi, &req),
            UPLL_RC_ERR_GENERIC);
  delete dmi;
}

TEST_F(VtermIfMoMgrTest, IsLogicalPortAndVlanIdInUse_ikey_cfg_val_null) {
  VtermIfMoMgrTest       obj;
  ConfigKeyVal           *ikey = NULL;
  DalDmlIntf             *dmi  = NULL;
  IpcReqRespHeader       req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                          NULL, NULL);
  EXPECT_EQ(obj.IsLogicalPortAndVlanIdInUse<val_vbr_if_t>(ikey, dmi, &req),
            UPLL_RC_ERR_GENERIC);
  delete dmi;
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, IsLogicalPortAndVlanIdInUse_ValStructNull) {
  VtermIfMoMgrTest       obj;
  ConfigKeyVal           *ikey = NULL;
  DalDmlIntf             *dmi  = NULL;
  IpcReqRespHeader       req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, NULL);
  ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                          key, cfg_val);
  testcase_id = LogicalPortIdSamekey;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(obj.IsLogicalPortAndVlanIdInUse<val_vbr_if_t>(ikey, dmi, &req),
            UPLL_RC_SUCCESS);
  delete dmi;
  delete ikey;
  VtermIfMoMgrTest::stub_result.clear();
}

TEST_F(VtermIfMoMgrTest, IsLogicalPortAndVlanIdInUse_Vbrif_DbAndReq_samekey) {
  VtermIfMoMgrTest       obj;
  ConfigKeyVal           *ikey = NULL;
  DalDmlIntf             *dmi  = NULL;
  IpcReqRespHeader       req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  val_drv_vbr_if *val = reinterpret_cast<val_drv_vbr_if *>
                            (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->vbr_if_val.portmap.logical_port_id),
         "PP-0000-0000-0000-0001");
  val->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val->vbr_if_val.portmap.vlan_id = 10;
  val->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  val->vbr_if_val.portmap.tagged = 1;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);
  ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                          key, cfg_val);
  testcase_id = LogicalPortIdSamekey;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(obj.IsLogicalPortAndVlanIdInUse<val_vbr_if_t>(ikey, dmi, &req),
            UPLL_RC_ERR_GENERIC);
  delete dmi;
  delete ikey;
  VtermIfMoMgrTest::stub_result.clear();
}

TEST_F(VtermIfMoMgrTest, IsLogicalPortAndVlanIdInUse_Vbrif_DbAndReq_diffkey) {
  VtermIfMoMgrTest       obj;
  ConfigKeyVal           *ikey = NULL;
  DalDmlIntf             *dmi  = NULL;
  IpcReqRespHeader       req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  strncpy(reinterpret_cast<char*>(key->vbr_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vbr_key.vbridge_name),
          "vbr1", strlen("vbr1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_drv_vbr_if *val = reinterpret_cast<val_drv_vbr_if *>
                            (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->vbr_if_val.portmap.logical_port_id),
         "PP-0000-0000-0000-0001");
  val->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val->vbr_if_val.portmap.vlan_id = 10;
  val->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  val->vbr_if_val.portmap.tagged = 1;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);
  ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                          key, cfg_val);
  testcase_id = VtermLogicalPortIdSame;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(obj.IsLogicalPortAndVlanIdInUse<val_vbr_if_t>(ikey, dmi, &req),
            UPLL_RC_ERR_GENERIC);
  VtermIfMoMgrTest::stub_result.clear();
  delete dmi;
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, IsLogicalPortAndVlanIdInUse_Vtermif_DbAndReq_diffkey) {
  VtermIfMoMgrTest       obj;
  ConfigKeyVal           *ikey = NULL;
  DalDmlIntf             *dmi  = NULL;
  IpcReqRespHeader       req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vbr1", strlen("vbr1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if *val = reinterpret_cast<val_vterm_if *>
                            (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->portmap.logical_port_id),
         "PP-0000-0000-0000-0001");
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val->portmap.vlan_id = 10;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  val->portmap.tagged = 1;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtermIf, val);
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                          key, cfg_val);
  testcase_id = VtermLogicalPortIdSame;
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(obj.IsLogicalPortAndVlanIdInUse<val_vterm_if_t>(ikey, dmi, &req),
            UPLL_RC_ERR_GENERIC);
  VtermIfMoMgrTest::stub_result.clear();
  delete dmi;
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, IsLogicalPortAndVlanIdInUse_NoEntry) {
  VtermIfMoMgrTest       obj;
  ConfigKeyVal           *ikey = NULL;
  DalDmlIntf             *dmi  = NULL;
  IpcReqRespHeader       req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vbr1", strlen("vbr1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  val_vterm_if *val = reinterpret_cast<val_vterm_if *>
                            (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  strcpy(reinterpret_cast<char*>(val->portmap.logical_port_id),
         "PP-0000-0000-0000-0001");
  val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
  val->portmap.vlan_id = 10;
  val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  val->portmap.tagged = 1;

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtermIf, val);
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                          key, cfg_val);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  EXPECT_EQ(obj.IsLogicalPortAndVlanIdInUse<val_vterm_if_t>(ikey, dmi, &req),
            UPLL_RC_ERR_GENERIC);
  VtermIfMoMgrTest::stub_result.clear();
  delete dmi;
  delete ikey;
}

TEST_F(VtermIfMoMgrTest, IsLogicalPortAndVlanIdInUse_ValueStructNull) {
  VtermIfMoMgrTest       obj;
  ConfigKeyVal           *ikey = NULL;
  DalDmlIntf             *dmi  = NULL;
  IpcReqRespHeader       req;

  /* Populate DAL interface */
  dmi = new DalOdbcMgr();

  key_vterm_if_t *key = reinterpret_cast<key_vterm_if_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vtn_key.vtn_name),
          "vtn1", strlen("vtn1"));
  strncpy(reinterpret_cast<char*>(key->vterm_key.vterminal_name),
          "vbr1", strlen("vbr1"));
  strncpy(reinterpret_cast<char*>(key->if_name), "if1", strlen("if1"));

  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVtermIf, NULL);
  ikey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                          key, cfg_val);
  VtermIfMoMgrTest::stub_result[ReadConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  EXPECT_EQ(obj.IsLogicalPortAndVlanIdInUse<val_vterm_if_t>(ikey, dmi, &req),
            UPLL_RC_SUCCESS);
  VtermIfMoMgrTest::stub_result.clear();
  delete dmi;
  delete ikey;
}

TEST_F(VtnMoMgrTest, MappingvExtTovBr_ikeyNull) {
  VtnMoMgrTest           obj;
  ConfigKeyVal           *ikey = NULL;
  DalDmlIntf             *dmi  = NULL;
  uint32_t               *rec_count = NULL;
  IpcReqRespHeader       req;

  EXPECT_EQ(obj.MappingvExtTovBr(ikey, &req, dmi, rec_count),
            UPLL_RC_ERR_GENERIC);
}

TEST_F(VtnMoMgrTest, MappingvExtTovBr_ValStructNull) {
  VtnMoMgrTest           obj;
  ConfigKeyVal           *ikey = NULL;
  DalDmlIntf             *dmi  = NULL;
  uint32_t               *rec_count = NULL;
  IpcReqRespHeader       req;

  ikey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER,
                          IpctSt::kIpcStKeyVtn,
                          NULL, NULL);
  EXPECT_EQ(obj.MappingvExtTovBr(ikey, &req, dmi, rec_count),
            UPLL_RC_ERR_GENERIC);
  delete ikey;
}

