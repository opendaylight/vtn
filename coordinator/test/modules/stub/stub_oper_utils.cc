/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <uncxx/tc/libtc_common.hh>
#include <pfcxx/module.hh>
#include "tcmsg.hh"
#include "tc_module.hh"
#include "tc_db_handler.hh"
#include "tc_operations.hh"
#include "tc_lock.hh"

int stub_srv_uint8;
int stub_srv_uint32;
int stub_srv_string;
int stub_opertype;

#define CNTRL_COUNT 3
#define DRIVER_COUNT 2

using namespace std;
using namespace unc::tc;
using namespace unc::tclib;

namespace unc {
namespace tc {

TcDbHandler::TcDbHandler(std::string dsn_name) { }
TcDbHandler::TcDbHandler(const TcDbHandler&) { }
/*Initialises DB connection and sets the default values*/
TcOperRet TcDbHandler::InitDB() {
  return TCOPER_RET_SUCCESS;
}
/*methods to access TC_UNC_CONF_TABLE*/
TcOperRet TcDbHandler::UpdateConfTable(pfc_bool_t auto_save) {
  return TCOPER_RET_SUCCESS;
}
TcOperRet TcDbHandler::GetConfTable(pfc_bool_t* auto_save) {
  return TCOPER_RET_SUCCESS;
}
/*methods to access TC_RECOVERY_TABLE*/
TcOperRet TcDbHandler::UpdateRecoveryTable(unc_keytype_datatype_t data_base,
                                           TcServiceType operation, uint32_t failover_instance) {
  return TCOPER_RET_SUCCESS;
}
TcOperRet TcDbHandler::GetRecoveryTable(unc_keytype_datatype_t* db,
                                        TcServiceType* oper, uint32_t* failover_instance) {
  return TCOPER_RET_SUCCESS;
}
/*checks whether the row exists with a
 *                                           * value*/
TcOperRet TcDbHandler::IsRowExists(std::string table_name,
                                   std::string attribute) {
  return TCOPER_RET_SUCCESS;
}

/*DB connection methods*/
TcOperRet TcDbHandler::SetConnectionEnv() {
  return TCOPER_RET_SUCCESS;
}
TcOperRet TcDbHandler::OpenDBConnection() {
  return TCOPER_RET_SUCCESS;
}
TcOperRet TcDbHandler::CloseDBConnection() {
  return TCOPER_RET_SUCCESS;
}
/*methods to initialize
 *                                                           * TC DB tables*/
TcOperRet TcDbHandler::InitTcDbTables() {
  return TCOPER_RET_SUCCESS;
}
TcOperRet TcDbHandler::SetDefaultConfTable() {
  return TCOPER_RET_SUCCESS;
}
TcOperRet TcDbHandler::SetDefaultRecoveryTable() {
  return TCOPER_RET_SUCCESS;
}
/*method to retrieve SQLDriverConnect connection string*/
std::string TcDbHandler::GetDBConnectString() {
  return "test";
}
/*methods to print readable strings in logs*/
std::string TcDbHandler::ConvertOptoString(SQLINTEGER op) {
  return "test";
}
std::string TcDbHandler::ConvertDbasetoString(SQLINTEGER dbase) {
  return "test";
}

std::string TcDbHandler::ConvertAutoSaveString(pfc_bool_t autosave) {
  return "test";
}
/*method to log SQL error msg*/
void TcDbHandler::GetErrorReason(SQLRETURN sqlret,
                                 SQLSMALLINT handletype,
                                 SQLHANDLE handle) {
}

TcTaskqUtil::~TcTaskqUtil() {}

TcTaskqUtil::TcTaskqUtil(uint32_t concurrency) {}
int TcTaskqUtil::PostReadTimer(uint32_t session_id,
                               uint32_t timeout,
                               TcLock* tclock,
                               TcChannelNameMap& unc_map) {
  return 0;
}

int TcTaskqUtil::DispatchAuditDriverRequest(std::string controller_id,
                                            TcDbHandler* tc_db_hdlr,
                                            TcLock* tclock,
                                            TcChannelNameMap& unc_map,
                                            unc_keytype_ctrtype_t driver_id) {
  return 0;
}

TcUtilRet
TcServerSessionUtils::set_srv_timeout(pfc::core::ipc::ServerSession* ssess,
                                      const pfc_timespec_t *timeout) {
  if (stub_srv_uint32 == 0) {
    pfc_log_info("get_uint32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else {
    return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::get_uint8(
    pfc::core::ipc::ServerSession* ssess,
    uint32_t index,
    uint8_t* data) {
  if (stub_srv_uint8) {
    pfc_log_info("get_uint8::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else {
    if (stub_opertype)
      *data = 1;
    return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::get_string(
    pfc::core::ipc::ServerSession* ssess,
    uint32_t index,
    std::string& data) {
  if (stub_srv_string) {
    pfc_log_info("get_string::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else {
    return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::get_uint32(pfc::core::ipc::ServerSession* ssess,
                                           uint32_t index,
                                           uint32_t* data) {
  if ( stub_srv_uint32 == 0 ) {
    pfc_log_info("get_uint32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else if (stub_srv_uint32 == -1) {
    pfc_log_info("get_uint32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FATAL;
  } else {
    return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::get_uint64(pfc::core::ipc::ServerSession* ssess,
                                           uint32_t index,
                                           uint64_t* data) {
  if ( stub_srv_uint32 == 0 ) {
    pfc_log_info("get_uint32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else if (stub_srv_uint32 == -1) {
    pfc_log_info("get_uint32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FATAL;
  } else {
        return TCUTIL_RET_SUCCESS;
      }
    }


TcUtilRet TcServerSessionUtils::set_uint32(pfc::core::ipc::ServerSession* ssess,
                                           uint32_t audit_result ) {
  if ( stub_srv_uint32 == 0 ) {
    pfc_log_info("set_uint32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else if (stub_srv_uint32 == -1) {
    pfc_log_info("set_uint32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FATAL;
  } else {
    return TCUTIL_RET_SUCCESS;
  }
}
}  // namespace tc
}  // namespace unc

