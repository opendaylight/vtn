/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_USER_HH_
#define _USESS_USER_HH_

#include "usess_def.hh"
#include "usess_conf_user.hh"
#include "mgmt_database.hh"
#include "usess_session.hh"

namespace unc {
namespace usess {

// ---------------------------------------------------------------------
// definition of define.
// ---------------------------------------------------------------------
// default user name.
const std::string kDefaultUser_CLIAdmin = USESS_USER_CLI_ADMIN;
const std::string kDefaultUser_WebAdmin = USESS_USER_WEB_ADMIN;
const std::string kDefaultUser_WebOper = USESS_USER_WEB_OPER;

// ---------------------------------------------------------------------
// definition of enumerate.
// ---------------------------------------------------------------------
// privilege check mode.
typedef enum {
  kPrivilegeUserPasswd,       // To change the user password.
} user_privilege_e;

// Authenticate mode.
typedef enum {
  kAuthenticateSessAdd,       // add session.
} user_authenticate_e;

// -------------------------------------------------------------
// Class declaration.
// -------------------------------------------------------------
class UsessUser : public UsessBaseCommon
{
 public:
  // -----------------------------
  //  class method.
  // -----------------------------
  UsessUser(mgmtdb::MgmtDatabase& database);
  ~UsessUser(void);

  usess_ipc_err_e Retrieve(const std::string& name);
  usess_ipc_err_e Privilege(const user_privilege_e mode,
                            const usess_ipc_res_sess_info_t& sess);
  usess_ipc_err_e Authenticate(const user_authenticate_e mode,
                               const usess_type_e sess_type,
                               const char* passwd);
  usess_ipc_err_e ChangePassword(const char* passwd);
  usess_ipc_err_e SetConf(UsessConfUser& conf_data);

  // -----------------------------
  //  data member.
  // -----------------------------
  std::string name;
  hash_type_e passwd_type;
  std::string passwd_digest;
  user_type_e type;
  SQL_DATE_STRUCT expiration;
  SQL_TIMESTAMP_STRUCT created;
  SQL_TIMESTAMP_STRUCT modified;

 private:
  // -----------------------------
  //  class method.
  // -----------------------------
  bool CheckUserName(const std::string& name);
  bool CheckPassword(const char* passwd);
  usess_ipc_err_e SetUserData(const mgmtdb::mgmtdb_variant_v& exec_value);

  // -----------------------------
  //  data member.
  // -----------------------------
  UsessConfUser conf_;
  mgmtdb::MgmtDatabase& database_;
};

}  // namespace usess
}  // namespace unc
#endif      // _USESS_USER_HH_
