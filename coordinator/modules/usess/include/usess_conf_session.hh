/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_CONF_SESSION_HH_
#define _USESS_CONF_SESSION_HH_

#include "usess_def.hh"

namespace unc {
namespace usess {

// -------------------------------------------------------------
//  Structure declaration.
// -------------------------------------------------------------
typedef enum {
  ID_CLI,        // CLI connect.
  ID_WEB_API,    // WEB API connect.
  ID_WEB_UI,     // WEB UI connect.
  ID_FIXED,      // connect to fixed session id.
  ID_NUM         // number of enum member.
} usess_conf_session_map_type_e;

// -------------------------------------------------------------
//  Structure declaration.
// -------------------------------------------------------------
// configuration:session connectin limited.
typedef struct {
  // Judging of session connection limit.
  bool limited;
  // Maximum number of simultaneous connections.
  uint32_t max_session;
} usess_conf_session_connect_num_t;

// configuration:session id range..
typedef struct {
  // Start of range to session ID.
  uint32_t start;
  // End of range to session ID.
  uint32_t end;
} usess_conf_session_id_range_t;

// configuration:each session type.
typedef struct {
  // configuration:session id range.
  usess_conf_session_id_range_t range;
  // configuration:session connectin limited.
  usess_conf_session_connect_num_t connect;
} usess_conf_session_parameter_t;

// configuration data.
typedef struct {
  usess_conf_session_connect_num_t global;
  // configuration:each session type.
  usess_conf_session_parameter_t local[ID_NUM];
} usess_conf_session_t;

// -------------------------------------------------------------
//  Class declaration.
// -------------------------------------------------------------
class UsessConfSession
{
 public:
  UsessConfSession(void);
  ~UsessConfSession(void);

  // configuration data load.
  usess_ipc_err_e LoadConf(void);
  // configuration data access.
  const usess_conf_session_t& data(void) const;

 private:
  // -----------------------------
  //  static const member.
  // -----------------------------
  // configuration file data block name.
  static const char* kConfBlockName_;
  // configuration file data map name, key.
  static const char* kConfMapName_;
  static const char* kConfMapKey_[ID_NUM];
  // configuration data default value.
  static const usess_conf_session_t kDefaultConf_;

  // -----------------------------
  //  data member.
  // -----------------------------
  // configuration data.
  usess_conf_session_t data_;
};

}  // namespace usess
}  // namespace unc
#endif    // _USESS_CONF_SESSION_HH_
