/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <uncxx/tc/libtc_common.hh>

#define TCLIB_SERVICE_NAME "tclib"

namespace unc {
namespace tc {

/*IPC session creation*/
pfc::core::ipc::ClientSession*
    TcClientSessionUtils::create_tc_client_session(std::string channel_name,
                                                   uint32_t service_id,
                                                   pfc_ipcconn_t &conn,
                                                   pfc_bool_t infinite_timeout) {
  pfc::core::ipc::ClientSession* result_session = NULL;
  int err = 0;
  /*open an alternate connection*/
  err = pfc_ipcclnt_altopen(channel_name.c_str(), &conn);
  if (err != 0 || conn == 0) {
    pfc_log_fatal("pfc_ipcclnt_altopen failed");
    return NULL;
  }
  /*create a client session to TCLIB*/
  result_session=
      new pfc::core::ipc::ClientSession(conn,
                                        TCLIB_SERVICE_NAME,
                                        service_id,
                                        err);
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
  /*invoke a session*/
  err = csess->invoke(response);
  if ( err != 0 ) {
    pfc_log_fatal("Session invoke failed. Errno:%d", err);
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

}  // namespace tc
}  // namespace unc

