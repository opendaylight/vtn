/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <uncxx/tc/libtc_common.hh>
#include <unc/upll_svc.h>
#include <unc/uppl_common.h>

#define TCLIB_SERVICE_NAME "tclib"

namespace unc {
namespace tc {

std::map<pfc_ipcconn_t, pfc::core::ipc::ClientSession*>
                                              TcClientSessionUtils::sess_map_;
pthread_mutex_t TcClientSessionUtils::sess_map_lock_ = PTHREAD_MUTEX_INITIALIZER;

pfc_bool_t TcClientSessionUtils::sys_stop_ = PFC_FALSE;
pthread_mutex_t TcClientSessionUtils::sys_stop_lock_ = PTHREAD_MUTEX_INITIALIZER;

/*IPC session creation*/
pfc::core::ipc::ClientSession*
    TcClientSessionUtils::create_tc_client_session(std::string channel_name,
                                                   uint32_t service_id,
                                                   pfc_ipcconn_t &conn,
                                                   pfc_bool_t infinite_timeout,
                                                   pfc_bool_t to_tclib) {
  pfc::core::ipc::ClientSession* result_session = NULL;
  int err = 0;
  /*open an alternate connection*/
  err = pfc_ipcclnt_altopen(channel_name.c_str(), &conn);

  if (err != 0 || conn == 0) {
    pfc_log_fatal("pfc_ipcclnt_altopen failed");
    return NULL;
  }

  std::string service_name;

  if (to_tclib == PFC_FALSE && service_id == UPLL_GLOBAL_CONFIG_SVC_ID) {
    service_name = UPLL_IPC_SERVICE_NAME;
  } else if (to_tclib == PFC_FALSE && service_id == UPPL_SVC_GLOBAL_CONFIG) {
    service_name = UPPL_IPC_SVC_NAME;
  } else { 
    service_name = TCLIB_SERVICE_NAME;
  }
  /*create a client session to TCLIB*/
  result_session=
      new pfc::core::ipc::ClientSession(conn,
                                        service_name,
                                        service_id,
                                        err,
                                        PFC_IPCSSF_CANCELABLE);
  if ( result_session == NULL ) {
    pfc_log_fatal("creating a ClientSession failed");
    pfc_ipcclnt_altclose(conn);
    return NULL;
  }

  /*set infinite timeout for commit/audit operations*/
  if (infinite_timeout) {
    err = result_session->setTimeout(NULL);
    if (err != 0) {
      pfc_log_fatal("pfc_ipcclnt_sess_settimeout failed");
      delete result_session;
      result_session = NULL;
      pfc_ipcclnt_altclose(conn);
      return NULL;
    }
  }

  // Inserting session id and connection id into map
  pthread_mutex_lock(&sess_map_lock_);
  std::map<pfc_ipcconn_t, pfc::core::ipc::ClientSession*>::iterator it;
  it = sess_map_.find(conn);
  if (it != sess_map_.end()) {
    pfc_log_warn("%s Connection-id %u already exists. Overwriting...",
                 __FUNCTION__, conn);
  }

  pfc_log_debug("%s Adding conn[%u] to sess_map_", __FUNCTION__, conn);
  sess_map_[conn] = result_session;
  pthread_mutex_unlock(&sess_map_lock_);

  /*return session pointer*/
  return result_session;
}
/*IPC session invoke*/
TcUtilRet TcClientSessionUtils::tc_session_invoke(
                                pfc::core::ipc::ClientSession* csess,
                                pfc_ipcresp_t& response) {
  uint8_t err = 0;
  if ( csess == NULL ) {
    pfc_log_error("Session param is NULL");
    return TCUTIL_RET_FAILURE;
  }

  if (get_sys_stop()) {
    pfc_log_error("%s SYS_STOP in-progress. Cannot invoke()",
                  __FUNCTION__);
    return TCUTIL_RET_FATAL;
  }

  /*invoke a session*/
  err = csess->invoke(response);
  if ( err != 0 ) {
    if (get_sys_stop() &&
        (err == ESHUTDOWN || err == ECANCELED)) {
      pfc_log_warn("%s SYS_STOP set. Session invoke Cancelled. Errno:%d",
                   __FUNCTION__, err);
    } else {
      pfc_log_fatal("Session invoke failed. Errno:%d", err);
    }
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
  
}
/*IPC session close*/
TcUtilRet
TcClientSessionUtils::tc_session_close(pfc::core::ipc::ClientSession** csess,
                                                 pfc_ipcconn_t conn) {
  int err = 0;
  pfc_log_debug("tc_session_close invoked");
  if (*csess != NULL) {
    delete *csess;
    *csess = NULL;
  }

  /*close the alternate connection*/
  err = pfc_ipcclnt_altclose(conn);
  if (err != 0) {
    pfc_log_fatal("pfc_ipcclnt_altclose failed");
    return TCUTIL_RET_FAILURE;
  }

  // Remove from sess_map_
  pthread_mutex_lock(&sess_map_lock_);

  std::map<pfc_ipcconn_t, pfc::core::ipc::ClientSession*>::iterator it;
  it = sess_map_.find(conn);
  if (it == sess_map_.end()) {
    pfc_log_error("%s Cannot find conn[%u] in sess_map", __FUNCTION__, conn);
  } else {
    pfc_log_debug("%s Removing sess_map_ entry from conn[%u]",
                 __FUNCTION__, conn);
    sess_map_.erase(it);
  }
  pthread_mutex_unlock(&sess_map_lock_);
  return TCUTIL_RET_SUCCESS;
}

/* Cancel all pending sessions during sys_stop event */
TcUtilRet TcClientSessionUtils::tc_session_cancel_all_sessions() {
  // Set the sys_stop flag to true
  set_sys_stop(PFC_TRUE);

  pfc_log_info("%s Cancelling all existing sessions", __FUNCTION__);

  pthread_mutex_lock(&sess_map_lock_);
  std::map<pfc_ipcconn_t, pfc::core::ipc::ClientSession*>::iterator it;

  for (it = sess_map_.begin();
       it != sess_map_.end();
       ++it) {
    pfc_log_info("%s pfc_ipcclnt_sess_cancel for conn[%u]",
                 __FUNCTION__, it->first);
    int ret = it->second->cancel(PFC_TRUE);
    if (ret != 0) {
      pfc_log_error("%s Cannot cancel conn[%u]. Errno=%d",
                    __FUNCTION__, it->first, ret);
    }
  }

  sess_map_.clear();
  pthread_mutex_unlock(&sess_map_lock_);

  return TCUTIL_RET_SUCCESS; 
}

/*retrieve uint64_t data from IPC session*/
TcUtilRet TcClientSessionUtils::get_uint64(
                          pfc::core::ipc::ClientSession* csess,
                          uint32_t index,
                          uint64_t* data) {
  uint8_t err = 0;
  uint32_t arg_count = 0;
  pfc_ipctype_t response;

  if ( csess == NULL || data == NULL ) {
    pfc_log_error("Session/Data param is NULL");
    return TCUTIL_RET_FAILURE;
  }
  /*validate response count*/
  arg_count = csess->getResponseCount();
  if ( index > arg_count ) {
    pfc_log_error("Index Exceeded Response Count");
    return TCUTIL_RET_FAILURE;
  }
  /*validate response type*/
  err = csess->getResponseType(index, response);
  if (err != 0) {
    pfc_log_fatal("Client Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  if (response != PFC_IPCTYPE_UINT64) {
    pfc_log_error("getResponseType failed");
    return TCUTIL_RET_FAILURE;
  }
  /*retrieve data*/
  err = csess->getResponse(index, *data);
  if ( err != 0 ) {
    pfc_log_fatal("getResponse failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*retrieve uint32_t data from IPC session*/
TcUtilRet TcClientSessionUtils::get_uint32(
                          pfc::core::ipc::ClientSession* csess,
                          uint32_t index,
                          uint32_t* data) {
  uint8_t err = 0;
  uint32_t arg_count = 0;
  pfc_ipctype_t response;

  if ( csess == NULL || data == NULL ) {
    pfc_log_error("Session/Data param is NULL");
    return TCUTIL_RET_FAILURE;
  }
  /*validate response count*/
  arg_count = csess->getResponseCount();
  if ( index > arg_count ) {
    pfc_log_error("Index Exceeded Response Count");
    return TCUTIL_RET_FAILURE;
  }
  /*validate response type*/
  err = csess->getResponseType(index, response);
  if (err != 0) {
    pfc_log_fatal("Client Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  if (response != PFC_IPCTYPE_UINT32) {
    pfc_log_error("getResponseType failed");
    return TCUTIL_RET_FAILURE;
  }
  /*retrieve data*/
  err = csess->getResponse(index, *data);
  if ( err != 0 ) {
    pfc_log_fatal("getResponse failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*retrieve uint8_t data from IPC session*/
TcUtilRet TcClientSessionUtils::get_uint8(
                          pfc::core::ipc::ClientSession* csess,
                          uint32_t index,
                          uint8_t* data) {
  uint8_t err = 0;
  uint32_t arg_count = 0;
  pfc_ipctype_t response;

  if ( csess == NULL || data == NULL ) {
    pfc_log_error("Session/Data param is NULL");
    return TCUTIL_RET_FAILURE;
  }
  /*validate response count*/
  arg_count = csess->getResponseCount();
  if ( index > arg_count ) {
    pfc_log_error("Index Exceeded Response Count");
    return TCUTIL_RET_FAILURE;
  }
  /*validate response type*/
  err = csess->getResponseType(index, response);
  if ( err != 0 ) {
    pfc_log_fatal("Client Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  if (response != PFC_IPCTYPE_UINT8) {
    pfc_log_error("getResponseType failed");
    return TCUTIL_RET_FAILURE;
  }
  /*retrieve data*/
  err = csess->getResponse(index, *data);
  if ( err != 0 ) {
    pfc_log_fatal("getResponse failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*retrieve string data from IPC session*/
TcUtilRet TcClientSessionUtils::get_string(
                          pfc::core::ipc::ClientSession* csess,
                          uint32_t index,
                          std::string& data) {
  uint8_t err = 0;
  uint32_t arg_count = 0;
  const char *temp = NULL;
  pfc_ipctype_t response;

  if ( csess == NULL ) {
    pfc_log_error("Session param is NULL");
    return TCUTIL_RET_FAILURE;
  }
  /*validate response count*/
  arg_count = csess->getResponseCount();
  if ( index > arg_count ) {
    pfc_log_error("Index Exceeded Response Count");
    return TCUTIL_RET_FAILURE;
  }
  /*validate response type*/
  err = csess->getResponseType(index, response);
  if ((err != 0) || (response != PFC_IPCTYPE_STRING)) {
    pfc_log_error("getResponseType failed");
    return TCUTIL_RET_FAILURE;
  }
  /*retrieve data*/
  err = csess->getResponse(index, temp);
  if ((err != 0)  || (temp == NULL)) {
    pfc_log_error("getResponse failed");
    return TCUTIL_RET_FATAL;
  }
  data.assign(temp);
  return TCUTIL_RET_SUCCESS;
}

/* Retrieve value for sys_stop_ flag */
pfc_bool_t TcClientSessionUtils::get_sys_stop() {
  pthread_mutex_lock(&sys_stop_lock_);
  pfc_bool_t stop = sys_stop_;
  pthread_mutex_unlock(&sys_stop_lock_);
  return stop;
}

/*set uint64_t data to IPC session*/
TcUtilRet TcClientSessionUtils::set_uint64(
                          pfc::core::ipc::ClientSession* csess,
                          uint64_t data) {
  uint8_t err = 0;
  if ( csess == NULL ) {
    pfc_log_error("Session param is NULL");
    return TCUTIL_RET_FAILURE;
  }
  /*append data to session*/
  err = csess->addOutput(data);
  if ( err != 0 ) {
    pfc_log_fatal("Appending data to ClientSession failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*set uint32_t data to IPC session*/
TcUtilRet TcClientSessionUtils::set_uint32(
                          pfc::core::ipc::ClientSession* csess,
                          uint32_t data) {
  uint8_t err = 0;
  if ( csess == NULL ) {
    pfc_log_error("Session param is NULL");
    return TCUTIL_RET_FAILURE;
  }
  /*append data to session*/
  err = csess->addOutput(data);
  if ( err != 0 ) {
    pfc_log_fatal("Appending data to ClientSession failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*set uint8_t data to IPC session*/
TcUtilRet TcClientSessionUtils::set_uint8(
                          pfc::core::ipc::ClientSession* csess,
                          uint8_t data) {
  uint8_t err = 0;
  if ( csess == NULL ) {
    pfc_log_error("Session param is NULL");
    return TCUTIL_RET_FAILURE;
  }
  /*append data to session*/
  err = csess->addOutput(data);
  if ( err != 0 ) {
    pfc_log_error("Appending data to ClientSession failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*set string data to IPC session*/
TcUtilRet TcClientSessionUtils::set_string(
                          pfc::core::ipc::ClientSession* csess,
                          std::string& data) {
  uint8_t err = 0;
  if ( csess == NULL ) {
    pfc_log_error("Session param is NULL");
    return TCUTIL_RET_FAILURE;
  }
  /*append data to session*/
  err = csess->addOutput(data);
  if ( err != 0 ) {
    pfc_log_error("Appending data to ClientSession failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

TcUtilRet TcClientSessionUtils::set_sys_stop(pfc_bool_t is_sys_stop) {
  pthread_mutex_lock(&sys_stop_lock_);
  sys_stop_ = is_sys_stop;
  pthread_mutex_unlock(&sys_stop_lock_);
  return TCUTIL_RET_SUCCESS;
}

}  // namespace tc
}  // namespace unc

