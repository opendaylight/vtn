/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_ENABLE_H_
#define _USESS_ENABLE_H_

#include "usess_conf_enable.hh"
#include "usess_base_common.hh"
#include "mgmt_database.hh"
#include "usess_session.hh"
#include "usess_user.hh"

namespace unc {
namespace usess {

// ---------------------------------------------------------------------
// definition of enumerate.
// ---------------------------------------------------------------------

// privilege check mode.
typedef enum {
  kPrivilegeEnable,           // authenticate enable.
  kPrivilegeDisable,          // to cancel the state of the enable.
  kPrivilegeEnablePasswd,     // To change the enable password.
} enable_privilege_e;

// Authenticate mode.
typedef enum {
  kAuthenticateEnable,        // authenticate enable.
} enable_authenticate_e;

// -------------------------------------------------------------
// Class declaration.
// -------------------------------------------------------------
class UsessEnable : public UsessBaseCommon
{
 public:
  UsessEnable(mgmtdb::MgmtDatabase& database);
  ~UsessEnable(void);

  // initialize, finish function.
  bool Init(void);
  bool Fini(void);

  // privilege check.
  usess_ipc_err_e Privilege(const enable_privilege_e mode,
                            const usess_ipc_res_sess_info_t& sess);
  // do enable password authentication.
  usess_ipc_err_e Authenticate(const enable_authenticate_e mode,
                               const usess_ipc_res_sess_info_t& sess,
                               const char* passwd);
  usess_ipc_err_e ChangePassword(const char* passwd);
  usess_ipc_err_e LoadConf(void);

 private:
  bool CheckPassword(const char* passwd);

  // -----------------------------
  //  data member.
  // -----------------------------
  UsessConfEnable conf_;
  mgmtdb::MgmtDatabase& database_;
};

}  // namespace usess
}  // namespace unc

#endif      // _USESS_ENABLE_H_
