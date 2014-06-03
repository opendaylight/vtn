/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_USERS_HH_
#define _USESS_USERS_HH_


#include "usess_def.hh"
#include "usess_conf_user.hh"
#include "usess_user.hh"

namespace unc {
namespace usess {

// -------------------------------------------------------------
// Class declaration.
// -------------------------------------------------------------
class UsessUsers
{
 public:
  UsessUsers(mgmtdb::MgmtDatabase& database);
  ~UsessUsers(void);

  // initialize, finish function.
  bool Init(void);
  bool Fini(void);

  usess_ipc_err_e GetUser(const std::string& user_name, UsessUser& user);
  usess_ipc_err_e LoadConf(void);

 private:
  // -----------------------------
  //  data member.
  // -----------------------------
  UsessConfUser conf_;
  mgmtdb::MgmtDatabase& database_;
};

}  // namespace usess
}  // namespace unc

#endif      // _USESS_USERS_HH_
