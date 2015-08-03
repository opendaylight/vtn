/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_HH_
#define _USESS_HH_

#include "pfcxx/synch.hh"
#include "usess_conf_common.hh"
#include "mgmt_database.hh"
#include "usess_sessions.hh"
#include "usess_users.hh"
#include "usess_enable.hh"
#include "usess_def.hh"


namespace unc {
namespace usess {

// -------------------------------------------------------------
// Macro definition.
// -------------------------------------------------------------

// ReadLock
#define USESS_RLOCK(LABEL, RTN) \
{ \
  RTN = USESS_E_OK; \
  int E_CODE = lock_.timedrdlock(conf_.data().lock_timeout); \
  GOTO_CODESET_DETAIL_IF2((E_CODE != 0), LABEL, RTN, USESS_E_NG, \
      E_CODE, "%s", "Failed to read lock.") \
}

// WriteLock
#define USESS_WLOCK(LABEL, RTN) \
{ \
  RTN = USESS_E_OK; \
  int E_CODE = lock_.timedwrlock(conf_.data().lock_timeout); \
  GOTO_CODESET_DETAIL_IF2((E_CODE != 0), LABEL, RTN, USESS_E_NG, \
      E_CODE, "%s", "Failed to write lock."); \
}

// UnLock
#define USESS_UNLOCK() \
{ \
  lock_.unlock(); \
}

#define CONNECT(LABEL, RTN) \
{ \
  RTN = USESS_E_OK; \
  mgmtdb::db_err_e db_rtn = database_.Connect(); \
  if (db_rtn != mgmtdb::DB_E_OK) { \
    L_ERROR("Failed database connect. err=%d", db_rtn); \
    RTN = USESS_E_NG; \
    goto LABEL; \
  } \
}

#define DISCONNECT(LABEL, RTN) \
{ \
  RTN = USESS_E_OK; \
  mgmtdb::db_err_e db_rtn = database_.Disconnect(); \
  if (db_rtn != mgmtdb::DB_E_OK) { \
    L_ERROR("Failed database disconnect. err=%d", db_rtn); \
    RTN = USESS_E_NG; \
    goto LABEL; \
  } \
}


// -------------------------------------------------------------
// Class definition
// -------------------------------------------------------------
class Usess : public pfc::core::Module
{
 public:
  Usess(const pfc_modattr_t *attr);
  ~Usess(void);
  pfc_bool_t init(void);
  pfc_bool_t fini(void);

  // IPC service handler.
  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession& ipcsess,
                             pfc_ipcid_t service);

  // Module specific events handler.
  void ReloadConfEventHandler(pfc::core::Event* event);


 private:
  static usess_ipc_err_e (Usess::*IpcHandler[])(
                      pfc::core::ipc::ServerSession&);

  // Processing function of IPC Service.
  usess_ipc_err_e UsessSessAddHandler(pfc::core::ipc::ServerSession& ipcsess);
  usess_ipc_err_e UsessSessDelHandler(pfc::core::ipc::ServerSession& ipcsess);
  usess_ipc_err_e UsessSessTypeDelHandler(
                      pfc::core::ipc::ServerSession& ipcsess);
  usess_ipc_err_e UsessEnableHandler(
                      pfc::core::ipc::ServerSession& ipcsess);
  usess_ipc_err_e UsessDisableHandler(pfc::core::ipc::ServerSession& ipcsess);
  usess_ipc_err_e UsessSessCountHandler(
                      pfc::core::ipc::ServerSession& ipcsess);
  usess_ipc_err_e UsessSessListHandler(pfc::core::ipc::ServerSession& ipcsess);
  usess_ipc_err_e UsessSessDetailHandler(
                      pfc::core::ipc::ServerSession& ipcsess);
  usess_ipc_err_e UserUserPasswdHandler(
                      pfc::core::ipc::ServerSession& ipcsess);
  usess_ipc_err_e UserEnablePasswdHandler(
                      pfc::core::ipc::ServerSession& ipcsess);

  // Config file data.
  UsessConfCommon conf_;

  // Caution: The constructor's initializer declaration order.
  // requires a "database_" in the constructor of the "users_" and "enable_".
  // "database_ " need to be defined before the  "users_" and "enable_" .

  // Database management class.
  mgmtdb::MgmtDatabase database_;

  // Sessions management class.
  UsessSessions sessions_;

  // Users management class.
  UsessUsers users_;

  // Enable management class.
  UsessEnable enable_;

  // Lock function class.
  pfc::core::ReadWriteLock lock_;

  // Module specific event handler ID.
  pfc_evhandler_t event_id_conf_reload_;
};

}  // namespace usess
}  // namespace unc
#endif  // _USESS_HH_
