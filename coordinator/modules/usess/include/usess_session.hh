/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_SESSION_HH_
#define _USESS_SESSION_HH_

#include "usess_def.hh"
#include "usess_base_common.hh"
#include "usess_conf_session.hh"
#include "tc/include/tc_module.hh"

namespace unc {
namespace usess {

// ---------------------------------------------------------------------
// definition of enumerate.
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// Definition of type.
// ---------------------------------------------------------------------
// -------------------------------------------------------------
// Class declaration.
// -------------------------------------------------------------
class UsessSession : public UsessBaseCommon
{
 public:
  UsessSession(const UsessConfSession& conf);
  UsessSession(const UsessConfSession& conf,
               const usess_ipc_res_sess_info_t& sess);
  ~UsessSession(void);

  usess_ipc_err_e TransitMode(const usess_mode_e sess_mode);
  const usess_ipc_res_sess_info_t& sess(void);

 private:
  // -----------------------------
  //  data member.
  // -----------------------------
  usess_ipc_res_sess_info_t sess_;
  const UsessConfSession& conf_;
};

}  // namespace usess
}  // namespace unc
#endif      // _USESS_SESSION_HH_
