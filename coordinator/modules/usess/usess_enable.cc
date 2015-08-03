/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "usess_enable.hh"

namespace unc {
namespace usess {

#define CLASS_NAME "UsessEnable"

// -------------------------------------------------------------
//  Class method definitions.
// -------------------------------------------------------------
/*
 * @brief   Constructor.
 * @param   database  : database instance.
 * @return  nothing.
 * @note    
 */
UsessEnable::UsessEnable(mgmtdb::MgmtDatabase& database) : database_(database)
{
}


/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessEnable::~UsessEnable(void)
{
}


/*
 * @brief   Initialize.
 * @param   nothing.
 * @return  true  : success
 *          false : failure
 * @note    
 */
bool UsessEnable::Init(void)
{
  usess_ipc_err_e rtn = USESS_E_NG;


  // configuration data load.
  rtn = conf_.LoadConf();
  RETURN_IF2((rtn != USESS_E_OK), false,
      "Failure configuration data load. err=%d", rtn);

  return true;
}


/*
 * @brief   Finalization.
 * @param   nothing.
 * @return  true  : success
 *          false : failure
 * @note    
 */
bool UsessEnable::Fini(void)
{
  return true;
}


/*
 * @brief   Privilege check.
 * @param   mode    :[IN] privilege mode.
 *          sess    :[IN] target session data.
 * @return  USESS_E_OK                : Privilege success
 *          USESS_E_INVALID_PRIVILEGE : Invalid privilege.
 *          USESS_E_NG                : Error
 * @note    
 */
usess_ipc_err_e UsessEnable::Privilege(const enable_privilege_e mode,
                                       const usess_ipc_res_sess_info_t& sess)
{
  L_FUNCTION_START();

  switch(mode) {

  // Enable authority.
  case kPrivilegeEnable:

    RETURN_IF2((sess.user_type != USER_TYPE_ADMIN), USESS_E_INVALID_PRIVILEGE,
        "Is not an administrative user. [%d]", sess.user_type);
    break;

  // Disable authentication.
  case kPrivilegeDisable:

    // In the case of disable, authentication is OK.
    break;

  // Change enable password.
  case kPrivilegeEnablePasswd:

    // check session mode.
    RETURN_IF2((sess.sess_mode != USESS_MODE_ENABLE), USESS_E_INVALID_MODE,
        "Is not enable session mode. [%d]", sess.sess_mode);
    break;

  default:
    RETURN_IF2(true, USESS_E_NG, "%s", "Invalid privilege mode.");
    break;
  }

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Enable password authentication.
 * @param   mode    :[IN] authenticate mode.
 *          sess    :[IN] target session data.
 *          passwd  :[IN] enable password.
 * @return  USESS_E_OK                : Authentication success
 *          USESS_E_INVALID_PASSWD    : Invalid password
 *          USESS_E_NG                : Error
 * @note    
 */
usess_ipc_err_e UsessEnable::Authenticate(const enable_authenticate_e mode,
        const usess_ipc_res_sess_info_t& sess, const char* passwd)
{
  int16_t fetch_type[] = {SQL_INTEGER, SQL_VARCHAR};
  std::string uname;
  std::string sql_statement;
  mgmtdb::mgmtdb_variant_v exec_value;
  mgmtdb::db_err_e db_rtn = mgmtdb::DB_E_NG;
  char primary_key[8] = {0};

  L_FUNCTION_START();

  switch(mode) {

  // Enable password Authentication.
  case kAuthenticateEnable:

    // if In UNC_WEB_ADMIN, WEB_API or WEB_UI,
    // not require password authentication
    uname = CAST_IPC_STRING(sess.sess_uname);
    if (uname == kDefaultUser_WebAdmin &&
         (sess.sess_type == USESS_TYPE_WEB_API ||
          sess.sess_type == USESS_TYPE_WEB_UI)) {
      break;
    }

    // Check password string.
    RETURN_IF2((CheckPassword(passwd) != true), USESS_E_INVALID_PASSWD,
                "%s", "Invalid password string.");

    // Get enable table record.
    sprintf(primary_key, "%d", USESS_MODE_ENABLE);
    sql_statement = sql_statement.erase() +
                    "SELECT passwd_hash, passwd " +
                    "   FROM tbl_unc_usess_enable" +
                    "   WHERE mode ='" + primary_key +"'";

    db_rtn = database_.Exec(sql_statement, false,
        sizeof(fetch_type)/sizeof(fetch_type[0]), fetch_type, exec_value);
    RETURN_IF2((db_rtn != mgmtdb::DB_E_OK || exec_value.size() == 0),
        USESS_E_NG, "Failure select sql exec(tbl_unc_usess_enable). err=%d",
        db_rtn);

    // Error, if count of columns and exec_value.size() is not equal.
    RETURN_IF2((exec_value.size() != 1 || exec_value[0].size() != 2),
        USESS_E_NG, "Abnormal column counts. count=%" PFC_PFMT_SIZE_T ", %"
        PFC_PFMT_SIZE_T,
        exec_value.size(), exec_value[0].size());

    // Compare password.
    RETURN_IF2((CheckDigest(passwd, exec_value[0][1].string_val(),
                             exec_value[0][1].string_val()) != true),
        USESS_E_INVALID_PASSWD, "%s", "Invalid password.");

    break;

  default:
    RETURN_IF2(true, USESS_E_NG, "%s", "Invalid authenticate mode.");
    break;
  }

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Change the enable password.
 * @param   passwd  : [IN] enable password
 * @return  USESS_E_OK             : Change password success
 *          USESS_E_INVALID_PASSWD : Invalid password
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessEnable::ChangePassword(const char* passwd)
{
  mgmtdb::db_err_e db_rtn = mgmtdb::DB_E_NG;
  std::string sql_statement;
  std::string hash_passwd;
  pfc_timespec_t now_time;
  mgmtdb::mgmtdb_variant_v exec_value;
  char hash_type[8] = {0};
  char primary_key[8] = {0};


  L_FUNCTION_START();

  // Check password string.
    RETURN_IF2((CheckPassword(passwd) != true), USESS_E_INVALID_PASSWD,
        "%s", "Invalid modify password string");

  // password hash.
  pfc_clock_get_realtime(&now_time);

  Hash(passwd, now_time, conf_.data().hash_type, hash_passwd);
  RETURN_IF2((hash_passwd.empty() != false), USESS_E_INVALID_PASSWD,
        "%s", "Failure modify password hash");

  // Update enable table record.
  sprintf(primary_key, "%d", USESS_MODE_ENABLE);
  snprintf(hash_type, sizeof(hash_type) - 1, "%d", conf_.data().hash_type);
  sql_statement = sql_statement.erase() +
                  "UPDATE tbl_unc_usess_enable"
                  " SET passwd = '" + hash_passwd + "'," +
                  "     passwd_hash = '" + hash_type + "',"
                  "     modified = CURRENT_TIMESTAMP" +
                    "   WHERE mode ='" + primary_key +"'";

  db_rtn = database_.Exec(sql_statement, true, 0, NULL, exec_value);
  RETURN_IF2((db_rtn != mgmtdb::DB_E_OK), USESS_E_NG,
        "Failure update sql exec(tbl_unc_usess_enable). err=%d", db_rtn);

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Load configuration data.
 * @param   nothing.
 * @return  USESS_E_OK             : Success
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessEnable::LoadConf(void)
{
  usess_ipc_err_e func_rtn;


  L_FUNCTION_START();

  func_rtn = conf_.LoadConf();
  RETURN_IF2((func_rtn != USESS_E_OK), func_rtn,
      "Failure configuration data load. err=%d", func_rtn);

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Check password string.
 * @param   nothing.
 * @return  true  : check ok.
 *          false : abnormal password.
 * @note    
 */
bool UsessEnable::CheckPassword(const char* passwd)
{
  // length check.
  if (strlen(passwd) > conf_.data().passwd_length) return false;
  // character code check.
  if (!CheckRegular(passwd, conf_.data().passwd_regular)) return false;
  // check ok.
  return true;
}

}  // namespace usess
}  // namespace unc
