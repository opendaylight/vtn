/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef SRC_MODULES_TC_INCLUDE_TC_DB_HANDLER_H_
#define SRC_MODULES_TC_INCLUDE_TC_DB_HANDLER_H_

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <cstdlib>
#include <map>
#include <string>
#include "tcmsg.hh"
#include "tc_module_data.hh"

namespace unc {
namespace tc {
/*TC DB tables*/
#define SAVE_CONF_TABLE "TC_UNC_CONF_TABLE"
#define RECOVERY_TABLE "TC_RECOVERY_TABLE"
/*for SQL APIs*/
#define STATUS_RECORD 1
#define BUFF_LEN 100

/*class to handle all database related operations*/
class TcDbHandler {
  public:
    TcDbHandler(std::string dsn_name);
    TcDbHandler(const TcDbHandler&);
    ~TcDbHandler() {
      CloseDBConnection();
    }
    /*Initialises DB connection and sets the default values*/
    TcOperRet InitDB();
    /*methods to access TC_UNC_CONF_TABLE*/
    TcOperRet UpdateConfTable(pfc_bool_t auto_save);
    TcOperRet GetConfTable(pfc_bool_t* auto_save);
    /*methods to access TC_RECOVERY_TABLE*/
    TcOperRet UpdateRecoveryTable(unc_keytype_datatype_t data_base,
                                  TcServiceType operation,
                                  TcConfigMode config_mode = TC_CONFIG_INVALID,
                                  std::string vtn_name = "",
                                  uint32_t failover_instance = 0);
    TcOperRet UpdateRecoveryTableAbortVersion(uint64_t abort_version);
    TcOperRet UpdateRecoveryTableSaveVersion(uint64_t save_version);
    TcOperRet GetRecoveryTable(unc_keytype_datatype_t* db,
                               TcServiceType* oper,
                               TcConfigMode *config_mode,
                               std::string *vtn_name,
                               uint32_t* failover_instance);
    TcOperRet GetRecoveryTableAbortVersion(uint64_t & abort_version);
    TcOperRet GetRecoveryTableSaveVersion(uint64_t & save_version);
    /*checks whether the row exists with a value*/
    TcOperRet IsRowExists(std::string table_name, std::string attribute);

    TcOperRet UpdateRecoveryTableGlobalModeDirty(pfc_bool_t global_mode_dirty);
    TcOperRet GetRecoveryTableGlobalModeDirty(pfc_bool_t&  global_mode_dirty);
  private:
    /*DB connection methods*/
    TcOperRet SetConnectionEnv();
    TcOperRet OpenDBConnection();
    TcOperRet CloseDBConnection();
    /*methods to initialize TC DB tables*/
    TcOperRet InitTcDbTables();
    TcOperRet SetDefaultConfTable();
    TcOperRet SetDefaultRecoveryTable();
    /*method to retrieve SQLDriverConnect connection string*/
    std::string GetDBConnectString();
    /*methods to print readable strings in logs*/
    std::string ConvertOptoString(SQLINTEGER op);
    std::string ConvertDbasetoString(SQLINTEGER dbase);
    std::string ConvertAutoSaveString(pfc_bool_t autosave);
    std::string ConvertConfigModeString(SQLINTEGER config_mode);
    /*method to log SQL error msg*/
    void GetErrorReason(SQLRETURN sqlret,
                        SQLSMALLINT handletype,
                        SQLHANDLE handle);

    std::string dsn_name_;   /*DB DSN name*/
    SQLHENV db_env_;         /*Environment handle for application*/
    SQLHDBC db_conn_handle_; /*Connection handle */
};

}  // namespace tc
}  // namespace unc

#endif  // SRC_MODULES_TC_INCLUDE_TC_DB_HANDLER_H_
