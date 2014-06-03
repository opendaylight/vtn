/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_SESSIONS_HH_
#define _USESS_SESSIONS_HH_

#include "usess_def.hh"
#include "usess_conf_session.hh"
#include "usess_session.hh"
#include "usess_user.hh"


namespace unc {
namespace usess {

// ---------------------------------------------------------------------
// definition of macro.
// ---------------------------------------------------------------------
// get configuration setting session type ID.
#define CONF_LOCAL_ID(TYPE) \
    ((TYPE == USESS_TYPE_CLI || TYPE == USESS_TYPE_CLI_DAEMON) ? \
        ID_CLI : (TYPE == USESS_TYPE_WEB_API) ? ID_WEB_API : ID_WEB_UI)

// ---------------------------------------------------------------------
// definition of enumerate.
// ---------------------------------------------------------------------
// Privilege check mode.
typedef enum {
  kPrivilegeSessDel,          // delete the session.
  kPrivilegeSessTypeDel,      // delete session of specified session type.
  kPrivilegeSessCount,        // gets number of sessions.
  kPrivilegeSessList,         // gets list of session information.
  kPrivilegeSessDetail,       // gets detailed Session information.
} session_privilege_e;

// ---------------------------------------------------------------------
// Definition of type.
// ---------------------------------------------------------------------
// Session data table type.
typedef std::map<uint32_t, UsessSession> usess_session_table_t;

// Type of session data list.
typedef std::vector<usess_ipc_res_sess_info_t> usess_session_list_v;

// -------------------------------------------------------------
// Class declaration.
// -------------------------------------------------------------
class UsessSessions
{
 public:
  UsessSessions(void);
  ~UsessSessions(void);

  // initialize, finish function.
  bool Init(void);
  bool Fini(void);

  // session access function.
  usess_ipc_err_e Add(const usess_ipc_req_sess_add_t& add_sess,
                      UsessUser user, usess_ipc_sess_id_t& sess_id);
  usess_ipc_err_e Del(const usess_ipc_sess_id_t& target_id);
  usess_ipc_err_e Del(const usess_type_e target_sess_type);
  usess_ipc_err_e Del(void);
  usess_ipc_err_e GetSession(const usess_ipc_sess_id_t& id,
                              UsessSession** sess);
  uint32_t GetCount(void);
  usess_ipc_err_e GetList(const usess_ipc_sess_id_t& target_id,
                          usess_session_list_v& info_list);
  usess_ipc_err_e GetList(usess_session_list_v& info_list);
  usess_ipc_err_e Privilege(const session_privilege_e mode,
                            const usess_ipc_sess_id_t& current,
                            const usess_ipc_sess_id_t& target);

  usess_ipc_err_e LoadConf(void);

 private:
  const usess_ipc_sess_id_t GetNewSessionId(usess_type_e sess_type);
  bool CheckSessTypeCount(usess_type_e sess_type);
  bool IsSessType(usess_type_e type) const;
  bool IsUserType(user_type_e type) const;

  // -----------------------------
  //  data member.
  // -----------------------------
  usess_session_table_t table_;
  UsessConfSession conf_;
  // allocated session id.
  uint32_t allocated_sess_id_[ID_NUM];

};

}  // namespace usess
}  // namespace unc
#endif      // _USESS_SESSIONS_HH_
