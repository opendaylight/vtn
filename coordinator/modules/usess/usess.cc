/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <boost/bind.hpp>
#include <crypt.h>
#include "usess.hh"

namespace unc {
namespace usess {

#define CLASS_NAME "Usess"

extern struct crypt_data usess_crypt_data;

// IPC handler function table.
usess_ipc_err_e (Usess::*Usess::IpcHandler[])(pfc::core::ipc::ServerSession&) =
{
    &Usess::UsessSessAddHandler,
    &Usess::UsessSessDelHandler,
    &Usess::UsessSessTypeDelHandler,
    &Usess::UsessEnableHandler,
    &Usess::UsessDisableHandler,
    &Usess::UsessSessCountHandler,
    &Usess::UsessSessListHandler,
    &Usess::UsessSessDetailHandler,
    &Usess::UserUserPasswdHandler,
    &Usess::UserEnablePasswdHandler
};


/*
 * @brief   Constructor.
 * @param   attr  : module attribute.
 * @return  nothing.
 * @note    
 */
Usess::Usess(const pfc_modattr_t *attr)
  : pfc::core::Module(attr), users_(database_), enable_(database_)
{
  event_id_conf_reload_ = EVHANDLER_ID_INVALID;
}


/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
Usess::~Usess(void)
{
}


/*
 * @brief   Initialization.
 * @param   nothing.
 * @return  PFC_TRUE  : Success.
 *          PFC_FALSE : Failure.
 * @note    
 */
pfc_bool_t Usess::init(void)
{
  usess_crypt_data.initialized = 0;
  // event mask value.
  pfc::core::EventMask reload_mask(PFC_MODEVENT_TYPE_RELOAD);
  // event handler.
  pfc::core::event_handler_t event_handler = NULL;
  // area of return value.
  int add_handler_rtn = -1;
  usess_ipc_err_e rtn = USESS_E_NG;


  L_FUNCTION_START();

  // -------------------------------------------------------------
  // Data load of configuration file.
  // -------------------------------------------------------------
  rtn = conf_.LoadConf();
  GOTO_IF2((rtn != USESS_E_OK), unlock_end,
      "Failed to data load of configuration file. err=%d", rtn);

  // write lock.
  USESS_WLOCK(proc_end, rtn);

  // -------------------------------------------------------------
  // Initialization of the data management class.
  // -------------------------------------------------------------
  GOTO_IF2((database_.Init() != true), unlock_end,
      "%s", "Failed to database class initialization.");
  GOTO_IF2((sessions_.Init() != true), unlock_end,
      "%s", "Failed to sessions class initialization.");
  GOTO_IF2((users_.Init() != true), unlock_end,
      "%s", "Failed to users class initialization.");
  GOTO_IF2((enable_.Init() != true), unlock_end,
      "%s", "Failed to enable class initialization.");

  // -------------------------------------------------------------
  // Register event handler.
  // -------------------------------------------------------------
  // Config file reload event.
  if (event_id_conf_reload_ == EVHANDLER_ID_INVALID) {
    event_handler = boost::bind(&Usess::ReloadConfEventHandler, this, _1);
    add_handler_rtn = addEventHandler(event_id_conf_reload_,
                                      event_handler, reload_mask);
    GOTO_CODESET_DETAIL_IF2((add_handler_rtn != 0), unlock_end,
        event_id_conf_reload_, EVHANDLER_ID_INVALID, add_handler_rtn,
        "%s", "Failed to add reload event handler.")
  }

  USESS_UNLOCK();

  // Process success.
  L_FUNCTION_COMPLETE();
  return PFC_TRUE;

unlock_end:
  USESS_UNLOCK();

proc_end:
  fini();
  return PFC_FALSE;
}


/*
 * @brief   Finalization.
 * @param   nothing.
 * @return  PFC_TRUE  : Success.
 *          PFC_FALSE : Failure.
 * @note    
 */
pfc_bool_t Usess::fini(void)
{
  // area of return value.
  int remove_handler_rtn = -1;
  pfc_bool_t err_code = PFC_TRUE;
  int lock_rtn = -1;


  L_FUNCTION_START();

  // -------------------------------------------------------------
  // Register event handler.
  // -------------------------------------------------------------
  // Config file reload event.
  if (event_id_conf_reload_ != EVHANDLER_ID_INVALID) {
    remove_handler_rtn = removeEventHandler(event_id_conf_reload_);
    WARN_CODESET_DETAIL_IF((remove_handler_rtn != 0), err_code, PFC_TRUE,
        remove_handler_rtn, "%s", "Failed to remove reload event handler.");
    if (remove_handler_rtn == 0) {
      event_id_conf_reload_ = EVHANDLER_ID_INVALID;
    }
  }

  // write lock.
  lock_rtn = lock_.timedwrlock(conf_.data().lock_timeout);
  WARN_CODESET_DETAIL_IF((lock_rtn != 0), err_code, PFC_FALSE,
      lock_rtn, "%s", "not write lock.");

  // -------------------------------------------------------------
  // finalization of the data management class.
  // -------------------------------------------------------------
  WARN_CODESET_IF((database_.Fini() != true), err_code, PFC_FALSE,
      "%s class finalization abnormal end.", "database");
  WARN_CODESET_IF((sessions_.Fini() != true), err_code, PFC_FALSE,
      "%s class finalization abnormal end.", "sessions");
  WARN_CODESET_IF((users_.Fini() != true), err_code, PFC_FALSE,
      "%s class finalization abnormal end.", "users");
  WARN_CODESET_IF((enable_.Fini() != true), err_code, PFC_FALSE,
      "%s class finalization abnormal end.", "enable");

  if (lock_rtn == 0) USESS_UNLOCK();
  if (err_code == PFC_TRUE) L_FUNCTION_COMPLETE();
  return err_code;
}


/*
 * @brief   IPC service handler.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 *          service : [IN]     IPC service ID.
 * @return  USESS_E_OK                : Success.
 *          USESS_E_NG                : Error.
 *          USESS_E_INVALID_SESSID    : Invalid current session ID.
 *          USESS_E_NO_SUCH_SESSID    : Invalid target session ID.
 *          USESS_E_INVALID_PRIVILEGE : Invalid privileges
 *          USESS_E_INVALID_MODE      : Invalid mode.
 *          USESS_E_INVALID_SESSTYPE  : Invalid session type.
 *          USESS_E_INVALID_USER      : Invalid user name.
 *          USESS_E_INVALID_PASSWD    : Invalid password.
 *          USESS_E_SESS_OVER         : Over the number of user sessions.
 * @note    
 */
pfc_ipcresp_t Usess::ipcService(pfc::core::ipc::ServerSession &ipcsess,
        pfc_ipcid_t service)
{
  // area of return value.
  usess_ipc_err_e rtn = USESS_E_NG;


  L_FUNCTION_START();

  // check service ID.
  GOTO_CODESET_IF2((service >= kUsessIpcNipcs), proc_end, rtn, USESS_E_NG,
      "Invalid service ID. Service ID=%d", service);

  // service ID process execution.
  rtn = (this->*IpcHandler[service])(ipcsess);
  GOTO_IF2((rtn != USESS_E_OK), proc_end,
      "Failed to IPC handler process. Service ID=%d", service);

  // Process success.
  L_FUNCTION_COMPLETE();
  rtn = USESS_E_OK;

proc_end:
  return static_cast<pfc_ipcresp_t>(rtn);
}


/*
 * @brief   Receive of configuration reload event.
 * @param   event : [IN] event object.
 * @return  nothing.
 * @note    
 */
void Usess::ReloadConfEventHandler(pfc::core::Event* event)
{
  pfc_evtype_t event_type = 0;
  // area of return value.
  usess_ipc_err_e err_code = USESS_E_OK;
  usess_ipc_err_e func_rtn = USESS_E_NG;
  int int_rtn = -1;


  L_FUNCTION_START();

  // Check event type.
  event_type = event->getType();
  GOTO_IF2((event_type != PFC_MODEVENT_TYPE_RELOAD), proc_end,
      "Invalid event type. event type=%hu", event_type);

  // -------------------------------------------------------------
  // Configuration file reload.
  // -------------------------------------------------------------
  int_rtn = reloadConf();
  GOTO_IF2((int_rtn != 0), proc_end, 
      "Failed reload configuration. err=%d (%s)",
      int_rtn, strerror(int_rtn));

  // write lock.
  USESS_WLOCK(proc_end, err_code);

  func_rtn = conf_.LoadConf();
  WARN_CODESET_IF((func_rtn != USESS_E_OK), err_code, func_rtn,
      "Failed %s config reload. err=%d", "common", func_rtn);

  func_rtn = sessions_.LoadConf();
  WARN_CODESET_IF((func_rtn != USESS_E_OK), err_code, func_rtn,
      "Failed %s config reload. err=%d", "session", func_rtn);

  func_rtn = users_.LoadConf();
  WARN_CODESET_IF((func_rtn != USESS_E_OK), err_code, func_rtn,
      "Failed %s config reload. err=%d", "user", func_rtn);

  func_rtn = enable_.LoadConf();
  WARN_CODESET_IF((func_rtn != USESS_E_OK), err_code, func_rtn,
      "Failed %s config reload. err=%d", "enable", func_rtn);

  USESS_UNLOCK();

  if (err_code == USESS_E_OK) {
    L_FUNCTION_COMPLETE();
  }
  return;

proc_end:
  return;
}


/*
 * @brief   Called from IPC service handler, and add the session.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e Usess::UsessSessAddHandler(
      pfc::core::ipc::ServerSession& ipcsess)
{
  uint32_t try_cnt = 0;
  // area of IPC send/receive data.
  usess_ipc_req_sess_add_t receive_data;
  usess_ipc_sess_id_t send_data;

  // user class data.
  UsessUser user(database_);
  std::string uname;
  char passwd[73] = {'\0'};

  // area of return value.
  int ipc_rtn = -1;
  usess_ipc_err_e err_code = USESS_E_NG;


  L_FUNCTION_START();

  // set IPC time out.
  ipcsess.setTimeout(NULL);

  // receive ipc client send data.
  ipc_rtn = ipcsess.getArgument(0, receive_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), proc_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc receive.");

retry:
  try_cnt++;
  // write lock.
  USESS_WLOCK(proc_end, err_code);

  // database connect.
  CONNECT(unlock_end, err_code);

  // get user information.
  uname = CAST_IPC_STRING(receive_data.sess_uname);
  err_code = users_.GetUser(uname, user);
  GOTO_IF2((err_code != USESS_E_OK), disconnect_end,
      "Failed get user information. user=%s err=%d", uname.c_str(), err_code);

  // database disconnect.
  DISCONNECT(unlock_end, err_code);

  // unlock.
  USESS_UNLOCK();

  // check user password authenticate.
  strncpy(passwd, (char*)receive_data.sess_passwd, sizeof(passwd) - 1);
  err_code = user.Authenticate(kAuthenticateSessAdd,
            static_cast<usess_type_e>(receive_data.sess_type), passwd);
  GOTO_IF2(((err_code == USESS_E_INVALID_PASSWD) && (try_cnt <= conf_.data().auth_retry_count)),
    retry, "Failed check user password authenticate. err=%d", err_code);
  GOTO_IF2((err_code != USESS_E_OK), proc_end,
      "Failed check user password authenticate. err=%d", err_code);

  // write lock.
  USESS_WLOCK(proc_end, err_code);

  // add session.
  err_code = sessions_.Add(receive_data, user, send_data);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed add session. err=%d", err_code);

  // send ipc data.
  ipc_rtn = ipcsess.addOutput(send_data);
  if (ipc_rtn != 0) {
    sessions_.Del(send_data);
  }
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), unlock_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc data send.");

  // erase of password data area.
  memset(receive_data.sess_passwd, 0x00, sizeof(receive_data.sess_passwd));
  memset(passwd, 0x00, sizeof(passwd));

  // Process success.
  USESS_UNLOCK();
  L_FUNCTION_COMPLETE();
  return USESS_E_OK;

disconnect_end:
  database_.Disconnect();
unlock_end:
  USESS_UNLOCK();

proc_end:
  // erase of password data area.
  memset(receive_data.sess_passwd, 0x00, sizeof(receive_data.sess_passwd));
  memset(passwd, 0x00, sizeof(passwd));

  return err_code;
}


/*
 * @brief   Called from IPC service handler, and delete the session.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e Usess::UsessSessDelHandler(
    pfc::core::ipc::ServerSession& ipcsess)
{
  usess_ipc_req_sess_del_t receive_data;

  int ipc_rtn = -1;
  usess_ipc_err_e err_code = USESS_E_NG;


  L_FUNCTION_START();

  // receive ipc client send data.
  ipc_rtn = ipcsess.getArgument(0, receive_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), proc_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc receive.");

  // write lock.
  USESS_WLOCK(proc_end, err_code);

  // check privilege of delete session.
  err_code = sessions_.Privilege(kPrivilegeSessDel,
                receive_data.current, receive_data.delsess);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed privilege. err=%d", err_code);

  // del session.
  err_code = sessions_.Del(receive_data.delsess);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed del session. err=%d", err_code);

  // Process success.
  USESS_UNLOCK();
  L_FUNCTION_COMPLETE();
  return USESS_E_OK;

unlock_end:
  USESS_UNLOCK();

proc_end:
  return err_code;
}


/*
 * @brief   Called from IPC service handler,
 *          and delete the session of Specified session type.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e Usess::UsessSessTypeDelHandler(
    pfc::core::ipc::ServerSession& ipcsess)
{
  usess_ipc_req_sess_type_del_t receive_data;

  int ipc_rtn = -1;
  usess_ipc_err_e err_code = USESS_E_NG;


  L_FUNCTION_START();

  // receive ipc client send data.
  ipc_rtn = ipcsess.getArgument(0, receive_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), proc_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc receive.");

  // write lock.
  USESS_WLOCK(proc_end, err_code);

  // del session.
  err_code = sessions_.Del(static_cast<usess_type_e>(receive_data.sess_type));
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed del session. err=%d", err_code);

  // Process success.
  USESS_UNLOCK();
  L_FUNCTION_COMPLETE();
  return USESS_E_OK;

unlock_end:
  USESS_UNLOCK();

proc_end:
  return err_code;
}

/*
 * @brief   Called from IPC service handler, perform enable authentication.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e Usess::UsessEnableHandler(
    pfc::core::ipc::ServerSession& ipcsess)
{
  uint32_t try_cnt = 0;
  usess_ipc_req_sess_enable_t receive_data;
  UsessSession *sess = NULL;
  char passwd[73] = {'\0'};

  int ipc_rtn = -1;
  usess_ipc_err_e err_code = USESS_E_NG;


  L_FUNCTION_START();

  // receive ipc client send data.
  ipc_rtn = ipcsess.getArgument(0, receive_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), proc_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc receive.");

  // write lock.
  USESS_WLOCK(proc_end, err_code);

retry:
  try_cnt++;
  // database connect.
  CONNECT(unlock_end, err_code);

  // get session.
  err_code = sessions_.GetSession(receive_data.current, &sess);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed get session information. err=%d", err_code);

  // check privilege of enable.
  err_code = enable_.Privilege(kPrivilegeEnable, sess->sess());
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed check enable privilege. err=%d", err_code);

  // check enable authenticate.
  strncpy(passwd, (char*)receive_data.enable_passwd, sizeof(passwd) - 1);
  err_code = enable_.Authenticate(kAuthenticateEnable, sess->sess(), passwd);
  if ((err_code == USESS_E_INVALID_PASSWD) && (try_cnt <= conf_.data().auth_retry_count)) {
    pfc_log_error("Failed check enable authenticate. err=%d", err_code);
    DISCONNECT(unlock_end, err_code);
    goto retry;
  }
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed check enable authenticate. err=%d", err_code);

  // session enable.
  err_code = sess->TransitMode(USESS_MODE_ENABLE);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed change enable mode. err=%d", err_code);

  // database disconnect.
  DISCONNECT(unlock_end, err_code);

  // erase of password data area.
  memset(receive_data.enable_passwd, 0x00, sizeof(receive_data.enable_passwd));
  memset(passwd, 0x00, sizeof(passwd));

  // Process success.
  USESS_UNLOCK();
  L_FUNCTION_COMPLETE();
  return USESS_E_OK;

unlock_end:
  database_.Disconnect();
  USESS_UNLOCK();

proc_end:
  // erase of password data area.
  memset(receive_data.enable_passwd, 0x00, sizeof(receive_data.enable_passwd));
  memset(passwd, 0x00, sizeof(passwd));

  return err_code;
}


/*
 * @brief   Called from IPC service handler, perform disable authentication.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e Usess::UsessDisableHandler(
    pfc::core::ipc::ServerSession& ipcsess)
{
  usess_ipc_sess_id_t receive_data;
  UsessSession *sess = NULL;

  int ipc_rtn = -1;
  usess_ipc_err_e err_code = USESS_E_NG;

  L_FUNCTION_START();

  // receive ipc client send data.
  ipc_rtn = ipcsess.getArgument(0, receive_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), proc_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc receive.");

  // write lock.
  USESS_WLOCK(proc_end, err_code);

  // get session.
  err_code = sessions_.GetSession(receive_data, &sess);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed get session information. err=%d", err_code);

  // check privilege of disable.
  err_code = enable_.Privilege(kPrivilegeDisable, sess->sess());
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed check disable privilege. err=%d", err_code);

  // session disable.
  err_code = sess->TransitMode(USESS_MODE_OPER);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed change enable mode. err=%d", err_code);

  // Process success.
  USESS_UNLOCK();
  L_FUNCTION_COMPLETE();
  return USESS_E_OK;

unlock_end:
  USESS_UNLOCK();

proc_end:
  return err_code;
}


/*
 * @brief   Called from IPC service handler, send session count.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e Usess::UsessSessCountHandler(
    pfc::core::ipc::ServerSession& ipcsess)
{
  usess_ipc_sess_id_t receive_data;
  uint32_t send_data;

  int ipc_rtn = -1;
  usess_ipc_err_e err_code = USESS_E_NG;


  L_FUNCTION_START();

  // receive ipc client send data.
  ipc_rtn = ipcsess.getArgument(0, receive_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), proc_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc receive.");

  // read lock.
  USESS_RLOCK(proc_end, err_code);

  // check privilege of get session count.
  err_code = sessions_.Privilege(kPrivilegeSessCount,
                receive_data, receive_data);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed privilege. err=%d", err_code);

  // get session count.
  send_data = sessions_.GetCount();

  // send ipc data.
  ipc_rtn = ipcsess.addOutput(send_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), unlock_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc data send.");

  // Process success.
  USESS_UNLOCK();
  L_FUNCTION_COMPLETE();
  return USESS_E_OK;

unlock_end:
  USESS_UNLOCK();

proc_end:
  return err_code;
}


/*
 * @brief   Called from IPC service handler, send session list.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e Usess::UsessSessListHandler(
    pfc::core::ipc::ServerSession& ipcsess)
{
  usess_ipc_sess_id_t receive_data;
  usess_session_list_v info_list;

  int ipc_rtn = -1;
  usess_ipc_err_e err_code = USESS_E_NG;


  L_FUNCTION_START();

  // receive ipc client send data.
  ipc_rtn = ipcsess.getArgument(0, receive_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), proc_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc receive.");

  // read lock.
  USESS_RLOCK(proc_end, err_code);

  // check privilege of session list.
  err_code = sessions_.Privilege(kPrivilegeSessList,
                receive_data, receive_data);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed privilege. err=%d", err_code);

  // get session list.
  err_code = sessions_.GetList(info_list);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed get session list. err=%d", err_code);

  // send ipc data.
  for (unsigned int loop = 0; loop < info_list.size(); ++loop) {
    ipc_rtn = ipcsess.addOutput(info_list[loop]);
    GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), unlock_end, err_code, USESS_E_NG,
        ipc_rtn, "Failed ipc data send. count=%d", loop+1);
  }

  // Process success.
  USESS_UNLOCK();
  L_FUNCTION_COMPLETE();
  return USESS_E_OK;

unlock_end:
  USESS_UNLOCK();

proc_end:
  return err_code;
}


/*
 * @brief   Called from IPC service handler, send session detail.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e Usess::UsessSessDetailHandler(
    pfc::core::ipc::ServerSession& ipcsess)
{
  usess_ipc_req_sess_detail_t receive_data;
  usess_session_list_v info_list;

  int ipc_rtn = -1;
  usess_ipc_err_e err_code = USESS_E_NG;


  L_FUNCTION_START();

  // receive ipc client send data.
  ipc_rtn = ipcsess.getArgument(0, receive_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), proc_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc receive.");

  // read lock.
  USESS_RLOCK(proc_end, err_code);

  // check privilege of session detail.
  err_code = sessions_.Privilege(kPrivilegeSessDetail,
                receive_data.current, receive_data.detail);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed privilege. err=%d", err_code);

  // get session Detail.
  err_code = sessions_.GetList(receive_data.detail, info_list);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed get session list. err=%d", err_code);

  // send ipc data.
  ipc_rtn = ipcsess.addOutput(info_list[0]);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), unlock_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc data send.");

  // Process success.
  USESS_UNLOCK();
  L_FUNCTION_COMPLETE();
  return USESS_E_OK;

unlock_end:
  USESS_UNLOCK();

proc_end:
  return err_code;
}


/*
 * @brief   Called from IPC service handler, change user password.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e Usess::UserUserPasswdHandler(
    pfc::core::ipc::ServerSession& ipcsess)
{
  usess_ipc_req_user_passwd_t receive_data;
  UsessUser user(database_);
  UsessSession *sess = NULL;

  std::string uname;
  char passwd[73] = {'\0'};

  int ipc_rtn = -1;
  usess_ipc_err_e err_code = USESS_E_NG;


  L_FUNCTION_START();

  // receive ipc client send data.
  ipc_rtn = ipcsess.getArgument(0, receive_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), proc_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc receive.");

  // write lock.
  USESS_WLOCK(proc_end, err_code);

  // database connect.
  CONNECT(unlock_end, err_code);

  // get current session.
  err_code = sessions_.GetSession(receive_data.current, &sess);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
        "Failed get session information. err=%d", err_code);

  // get target user.
  uname = CAST_IPC_STRING(receive_data.sess_uname);
  err_code = users_.GetUser(uname, user);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed get user. user=%s err=%d", uname.c_str(), err_code);

  // check privilege of change user password.
  strncpy(passwd, (char*)receive_data.sess_passwd, sizeof(passwd) - 1);
  err_code = user.Privilege(kPrivilegeUserPasswd, sess->sess());
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed check change user password privilege. err=%d", err_code);

  // change user password.
  err_code = user.ChangePassword(passwd);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed change user password. user=%s err=%d", uname.c_str(), err_code);

  // database disconnect.
  DISCONNECT(unlock_end, err_code);

  // erase of password data area.
  memset(receive_data.sess_passwd, 0x00, sizeof(receive_data.sess_passwd));
  memset(passwd, 0x00, sizeof(passwd));

  // Process success.
  USESS_UNLOCK();
  L_FUNCTION_COMPLETE();
  return USESS_E_OK;

unlock_end:
  database_.Disconnect();
  USESS_UNLOCK();

proc_end:
  // erase of password data area.
  memset(receive_data.sess_passwd, 0x00, sizeof(receive_data.sess_passwd));
  memset(passwd, 0x00, sizeof(passwd));

  return err_code;
}


/*
 * @brief   Called from IPC service handler, change enable password.
 * @param   ipcsess : [IN/OUT] IPC service handler context.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e Usess::UserEnablePasswdHandler(
    pfc::core::ipc::ServerSession& ipcsess)
{
  usess_ipc_req_enable_passwd_t receive_data;
  UsessSession *sess = NULL;
  char passwd[73] = {'\0'};

  int ipc_rtn = -1;
  usess_ipc_err_e err_code = USESS_E_NG;


  L_FUNCTION_START();

  // receive ipc client send data.
  ipc_rtn = ipcsess.getArgument(0, receive_data);
  GOTO_CODESET_DETAIL_IF2((ipc_rtn != 0), proc_end, err_code, USESS_E_NG,
      ipc_rtn, "%s", "Failed ipc receive.");

  // write lock.
  USESS_WLOCK(proc_end, err_code);

  // database connect.
  CONNECT(unlock_end, err_code);

  // get session.
  err_code = sessions_.GetSession(receive_data.current, &sess);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed get session information. err=%d", err_code);

  // check privilege of enable password change.
  err_code = enable_.Privilege(kPrivilegeEnablePasswd, sess->sess());
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed check privilege of enable password change. err=%d", err_code);

  // change enable password.
  strncpy(passwd, (char*)receive_data.enable_passwd, sizeof(passwd) - 1);
  err_code = enable_.ChangePassword(passwd);
  GOTO_IF2((err_code != USESS_E_OK), unlock_end,
      "Failed change enable password. err=%d", err_code);

  // database disconnect.
  DISCONNECT(unlock_end, err_code);

  // erase of password data area.
  memset(receive_data.enable_passwd, 0x00, sizeof(receive_data.enable_passwd));
  memset(passwd, 0x00, sizeof(passwd));

  // Process success.
  USESS_UNLOCK();
  L_FUNCTION_COMPLETE();
  return USESS_E_OK;

unlock_end:
  database_.Disconnect();
  USESS_UNLOCK();

proc_end:
  // erase of password data area.
  memset(receive_data.enable_passwd, 0x00, sizeof(receive_data.enable_passwd));
  memset(passwd, 0x00, sizeof(passwd));

  return err_code;
}


} // namespace usess
} // namespace unc

PFC_MODULE_IPC_DECL(unc::usess::Usess, kUsessIpcNipcs);
