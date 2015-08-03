/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_CONF_COMMON_HH_
#define _USESS_CONF_COMMON_HH_

#include "usess_def.hh"

namespace unc {
namespace usess {

// -------------------------------------------------------------
// Structure declaration.
// -------------------------------------------------------------
// configuration "common" block data.
typedef struct {
  // lock timeout.
  pfc_timespec_t lock_timeout;
  uint32_t auth_retry_count;
} usess_conf_common_t;


// -------------------------------------------------------------
// Class declaration.
// -------------------------------------------------------------
class UsessConfCommon
{
 public:
  UsessConfCommon(void);
  ~UsessConfCommon(void);

  // configuration data load.
  usess_ipc_err_e LoadConf(void);
  // configuration data access.
  const usess_conf_common_t& data(void) const;

 private:
  // -----------------------------
  //  static const member.
  // -----------------------------
  // configuration file data block name.
  static const char* kConfBlockName_;
  // configuration data default value.
  static const usess_conf_common_t kDefaultConf_;

  // -----------------------------
  //  data member.
  // -----------------------------
  // configuration data.
  usess_conf_common_t data_;
};

}  // namespace usess
}  // namespace unc
#endif    // _USESS_CONF_COMMON_HH_
