/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_tcdbhandler.hh"
#include <uncxx/tc/libtc_common.hh>

extern uint32_t resp_count;
extern int stub_srv_uint8;
extern int stub_srv_uint32;
extern int stub_srv_string;
extern int stub_opertype;

TEST(TcDbHandler, TestUpdateRecoveryTbl_GlobalModeDirtyTrue) {

  std::string dsn_name_test = "UNC_DB_DSN";
  pfc_bool_t global_mode_dirty_test = PFC_FALSE;

  TcDbHandler* db_handler = new TcDbHandler(dsn_name_test);
  TcOperRet result;
  result = db_handler->UpdateRecoveryTableGlobalModeDirty(global_mode_dirty_test);
  EXPECT_EQ(TCOPER_RET_SUCCESS,result);
}

TEST(TcDbHandler, TestUpdateRecoveryTbl_GlobalModeDirtyFalse) {

  std::string dsn_name_test = "UNC_DB_DSN";
  pfc_bool_t global_mode_dirty_test = PFC_FALSE;

  TcDbHandler* db_handler = new TcDbHandler(dsn_name_test);
  TcOperRet result;
  result = db_handler->UpdateRecoveryTableGlobalModeDirty(global_mode_dirty_test);
  EXPECT_EQ(TCOPER_RET_SUCCESS,result);
}

TEST(TcDbHandler, TestGetRecoveryTblGlobalModeDirty_GlobalModeDirtyTrue) {

  std::string dsn_name_test = "UNC_DB_DSN";
  pfc_bool_t global_mode_dirty_test = PFC_TRUE;

  TcDbHandler* db_handler = new TcDbHandler(dsn_name_test);
  TcOperRet result;
  result = db_handler->GetRecoveryTableGlobalModeDirty(global_mode_dirty_test);
  EXPECT_EQ(TCOPER_RET_SUCCESS,result);
}

TEST(TcDbHandler, TestGetRecoveryTblGlobalModeDirty_GlobalModeDirtyFalse) {

  std::string dsn_name_test = "UNC_DB_DSN";
  pfc_bool_t global_mode_dirty_test = PFC_FALSE;

  TcDbHandler* db_handler = new TcDbHandler(dsn_name_test);
  TcOperRet result;
  result = db_handler->GetRecoveryTableGlobalModeDirty(global_mode_dirty_test);
  EXPECT_EQ(TCOPER_RET_SUCCESS,result);
}
#if 0
TEST(TcDbHandler, TestGetRecoveryTblGlobalModeDirty_GlobalModeDirty_SqlAlloc_Fail) {

  std::string dsn_name_test = "UNC_DB_DSN";
  pfc_bool_t global_mode_dirty_test = PFC_FALSE;

  TcDbHandler* db_handler = new TcDbHandler(dsn_name_test);
  TcOperRet result;
  result = db_handler->GetRecoveryTableGlobalModeDirty(global_mode_dirty_test);
  EXPECT_EQ(TCOPER_RET_SUCCESS,result);
}
#endif

TEST(TcDbHandler, TestSetDefaultRecoveryTable) {

 TcOperRet result;
 std::string dsn_name_test = "UNC_DB_DSN";
 TcDbHandler* db_handler = new TcDbHandler(dsn_name_test);
 result = db_handler->SetDefaultRecoveryTable();
 EXPECT_EQ(TCOPER_RET_SUCCESS,result);
}
