/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <stdlib.h>
#include "usess_user.hh"

namespace unc {
namespace usess {

#define CLASS_NAME "UsessUser"

// -------------------------------------------------------------
//  Static member definitions.
// -------------------------------------------------------------


// -------------------------------------------------------------
//  Class method definitions.
// -------------------------------------------------------------
/*
 * @brief   Constructor.
 * @param   database  : database instance.
 * @return  nothing.
 * @note    
 */
UsessUser::UsessUser(mgmtdb::MgmtDatabase& database) : name(), passwd_digest(),
        expiration(), created(), modified(), database_(database)
{
  // initialize class data member.
  passwd_type = HASH_TYPE_UNKNOWN;
  type = USER_TYPE_UNKNOWN;
}


/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessUser::~UsessUser(void)
{
}

/*
 * @brief   Retrieve user data for table from the database.
 * @param   passwd  : [IN] user name.
 * @return  USESS_E_OK             : Success.
 *          USESS_E_INVALID_USER   : Invalid user name.
 *          USESS_E_NG             : Error.
 * @note    
 */
usess_ipc_err_e UsessUser::Retrieve(const std::string& name)
{
  int16_t fetch_type[] = {SQL_VARCHAR, SQL_INTEGER, SQL_VARCHAR, SQL_INTEGER};
  usess_ipc_err_e func_rtn = USESS_E_NG;
  mgmtdb::db_err_e db_rtn = mgmtdb::DB_E_NG;
  std::string sql_statement;
  std::string hash_passwd;
  mgmtdb::mgmtdb_variant_v exec_value;


  L_FUNCTION_START();

  // Check user name character codes.
  RETURN_IF2((CheckUserName(name) != true), USESS_E_INVALID_USER,
      "%s", "Invalid user name string.");

  sql_statement = sql_statement.erase() +
                  "SELECT uname, passwd_hash, passwd, usertype" +
                  "  FROM tbl_unc_usess_user" +
                  "  WHERE uname = '" + name + "'" +
                  "  AND (expiration >= CURRENT_DATE OR expiration IS NULL)";

  db_rtn = database_.Exec(sql_statement, false,
        sizeof(fetch_type)/sizeof(fetch_type[0]), fetch_type, exec_value);
  RETURN_IF2((db_rtn != mgmtdb::DB_E_OK || exec_value.size() == 0),
      USESS_E_INVALID_USER, "Failure select sql exec(tbl_unc_usess_user). err=%d",
      db_rtn);

  // Save the user data to class members.
  func_rtn = SetUserData(exec_value);
  RETURN_IF2((func_rtn != USESS_E_OK), func_rtn, "%s", "Failure DB data set.");

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   User privilege.
 * @param   sess    :[IN] current session data.
 *          mode    :[IN] privilege mode.
 * @return  USESS_E_OK                : Authentication success.
 *          USESS_E_INVALID_MODE      : Invalid session mode.
 *          USESS_E_INVALID_USER      : Invalid user.
 *          USESS_E_NG                : Error
 * @note    
 */
usess_ipc_err_e UsessUser::Privilege(const user_privilege_e mode,
                                     const usess_ipc_res_sess_info_t& sess)
{
  std::string uname;

  L_FUNCTION_START();

  switch(mode) {

  // Change UNC user password.
  case kPrivilegeUserPasswd:

    // check session mode.
    RETURN_IF2(((sess.sess_mode != USESS_MODE_ENABLE) && 
              ((sess.sess_mode != USESS_MODE_OPER))), USESS_E_INVALID_MODE,
        "Invalid session mode. [%d]", sess.sess_mode);

    // check user name.
    uname = CAST_IPC_STRING(sess.sess_uname);
    RETURN_IF2((name.compare(uname) != 0), USESS_E_INVALID_USER,
        "Invalid user name. [%s]", uname.c_str());
    break;

  default:
    RETURN_IF2(true, USESS_E_NG, "%s", "Invalid privilege mode.");
    break;
  }

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   User authentication.
 * @param   sess_type :[IN] current session type.
 *          passwd    :[IN] user password
 *          mode      :[IN] authenticate mode.
 * @return  USESS_E_OK                : Authentication success.
 *          USESS_E_INVALID_PASSWD    : Invalid password.
 *          USESS_E_INVALID_PRIVILEGE : Invalid privilege.
 *          USESS_E_NG                : Error
 * @note    
 */
usess_ipc_err_e UsessUser::Authenticate(const user_authenticate_e mode,
                                        const usess_type_e sess_type,
                                        const char* passwd)
{
  L_FUNCTION_START();

  switch(mode) {

  // UNC user authority.
  case kAuthenticateSessAdd:

    // If sessio type is "USESS_TYPE_CLI" or "USESS_TYPE_CLI_DAEMON",
    // If user name is "UNC_CLI_ADMIN",
    // password authentication is unnecessary.
    if (name.compare(kDefaultUser_CLIAdmin) == 0 ||
        sess_type == USESS_TYPE_CLI || sess_type == USESS_TYPE_CLI_DAEMON) {
      break;
    }

    // Check password character codes.
    RETURN_IF2((CheckPassword(passwd) != true), USESS_E_INVALID_PASSWD,
        "%s", "Invalid password string.");

    // Compare password.
    RETURN_IF2((CheckDigest(passwd, passwd_digest, passwd_digest) != true),
        USESS_E_INVALID_PASSWD, "%s", "Invalid password.");
    // Success.
    break;

  default:
    RETURN_IF2(true, USESS_E_NG, "%s", "Invalid authenticate mode.");
    break;
  }

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Change the user password.
 * @param   passwd  : [IN] user password
 * @return  USESS_E_OK             : Change password success.
 *          USESS_E_INVALID_PASSWD : Invalid password.
 *          USESS_E_NG             : Error.
 * @note    
 */
usess_ipc_err_e UsessUser::ChangePassword(const char* passwd)
{
  mgmtdb::db_err_e db_rtn = mgmtdb::DB_E_NG;
  std::string sql_statement;
  std::string hash_passwd;
  pfc_timespec_t now_time;
  mgmtdb::mgmtdb_variant_v exec_value;
  char hash_type[8] = {0};


  L_FUNCTION_START();

  // Check password character codes.
  RETURN_IF2((CheckPassword(passwd) != true), USESS_E_INVALID_PASSWD,
      "%s", "Invalid modify password string");

  // password hash.
  pfc_clock_get_realtime(&now_time);

  Hash(passwd, now_time, conf_.data().hash_type, hash_passwd);
  RETURN_IF2((hash_passwd.empty() != false), USESS_E_NG,
        "%s", "Failure modify password hash");

  // Update enable table record.
  snprintf(hash_type, sizeof(hash_type)-1, "%d", conf_.data().hash_type);
  sql_statement = sql_statement.erase() +
                  "UPDATE tbl_unc_usess_user" +
                  " SET passwd = '" + hash_passwd + "'," +
                  "   passwd_hash = '" + hash_type + "'," +
                  "   expiration = NULL," +
                  "   modified = CURRENT_TIMESTAMP" +
                  "   WHERE uname = '" + name + "'";

  db_rtn = database_.Exec(sql_statement, true, 0, NULL, exec_value);
  RETURN_IF2((db_rtn != mgmtdb::DB_E_OK), USESS_E_NG,
      "Failure update sql exec(tbl_unc_usess_user). err=%d", db_rtn);

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}

/*
 * @brief   Save configuration data.
 * @param   conf_data : configuration data.
 * @return  USESS_E_OK             : Success.
 *          USESS_E_NG             : Error.
 * @note    
 */
usess_ipc_err_e UsessUser::SetConf(UsessConfUser& conf_data)
{
  conf_ = conf_data;
  return USESS_E_OK;
}


/*
 * @brief   Check user name string.
 * @param   nothing.
 * @return  true  : check ok.
 *          false : abnormal user name.
 * @note    
 */
bool UsessUser::CheckUserName(const std::string& name)
{
  // length check.
  if (name.length() > conf_.data().user_length) return false;
  // character code check.
  if (!CheckRegular(name.c_str(), conf_.data().user_regular)) return false;
  // check ok.
  return true;
}


/*
 * @brief   Check password string.
 * @param   nothing.
 * @return  true  : check ok.
 *          false : abnormal password.
 * @note    
 */
bool UsessUser::CheckPassword(const char* passwd)
{
  // length check.
  if (strlen(passwd) > conf_.data().passwd_length) return false;
  // character code check.
  if (!CheckRegular(passwd, conf_.data().passwd_regular)) return false;
  // check ok.
  return true;
}


/*
 * @brief   Set user data to class members.
 * @param   passwd  : [IN] user name.
 * @return  USESS_E_OK             : Success.
 *          USESS_E_INVALID_USER   : Invalid user name.
 *          USESS_E_NG             : Error.
 * @note    
 */
usess_ipc_err_e UsessUser::SetUserData(
          const mgmtdb::mgmtdb_variant_v& exec_value)
{
  // Error, if count of columns and exec_value.size() is not equal.
  RETURN_IF2((exec_value.size() != 1 || exec_value[0].size() != 4),
    USESS_E_NG, "abnormal column counts. count=%" PFC_PFMT_SIZE_T ", %"
    PFC_PFMT_SIZE_T,
    exec_value.size(), exec_value[0].size());

  // Save to class data member.
  name = name.erase() + exec_value[0][0].string_val();
  passwd_type = static_cast<hash_type_e>(exec_value[0][1].u_val.v_int32);
  passwd_digest = passwd_digest.erase() + exec_value[0][2].string_val();
  type = static_cast<user_type_e>(exec_value[0][3].u_val.v_int32);
//  expiration = exec_value[0][4].u_val.v_date;
//  created = exec_value[0][5].u_val.v_timestamp;
//  modified = exec_value[0][6].u_val.v_timestamp;

  return USESS_E_OK;
}

}  // namespace so
}  // namespace usess
