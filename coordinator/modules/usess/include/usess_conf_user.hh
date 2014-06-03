/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_CONF_USER_HH_
#define _USESS_CONF_USER_HH_

#include "usess_def.hh"

namespace unc {
namespace usess {

// -------------------------------------------------------------
// Structure declaration.
// -------------------------------------------------------------

// configuration "user" block data.
typedef struct {
  // Hash type of password.
  hash_type_e hash_type;
  // Valid number of user name characters
  uint32_t user_length;
  // Available characters to user name.
  std::string user_regular;
  // Valid number of password characters.
  uint32_t passwd_length;
  // Available characters to password.
  std::string passwd_regular;
} usess_conf_user_t;


// -------------------------------------------------------------
// Class declaration.
// -------------------------------------------------------------
class UsessConfUser
{
public:
  UsessConfUser(void);
  ~UsessConfUser(void);

  // configuration data load.
  usess_ipc_err_e LoadConf(void);
  // configuration data access.
  const usess_conf_user_t& data(void) const;

 private:
  // -----------------------------
  //  static const member.
  // -----------------------------
  // configuration file data block name.
  static const char* kConfBlockName_;
  // configuration data default value.
  static const usess_conf_user_t kDefaultConf_;

  // -----------------------------
  //  data member.
  // -----------------------------
  // configuration data.
  usess_conf_user_t data_;
};

}  // namespace usess
}  // namespace unc
#endif    // _USESS_CONF_USER_HH_
