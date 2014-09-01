/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <uncxx/tc/libtc_common.hh>

namespace unc {
namespace tc {

TcUtilRet
    TcServerSessionUtils::set_srv_timeout(pfc::core::ipc::ServerSession* ssess,
                                          const pfc_timespec_t *timeout) {
      int err = 0;
      if (ssess == NULL)
        return TCUTIL_RET_FAILURE;

      err = ssess->setTimeout(timeout);
      if (err != 0) {
        pfc_log_fatal("setting timeout to server session failed");
        return TCUTIL_RET_FAILURE;
      }
      return TCUTIL_RET_SUCCESS;
}

/*retrive the int32 data from the IPC session*/
TcUtilRet TcServerSessionUtils::get_int32(
                          pfc::core::ipc::ServerSession* ssess,
                          uint32_t index,
                          int32_t * data) {
  uint8_t err = 0;
  uint32_t arg_count = 0;
  pfc_ipctype_t response;
  if ( ssess == NULL ) {
    return TCUTIL_RET_FAILURE;
  }

  if ( data == NULL ) {
    return TCUTIL_RET_FAILURE;
  }

  /*validate response count*/
  arg_count = ssess->getArgCount();
  if ( index > arg_count ) {
    pfc_log_error("Index Exceeded Argument Count");
    return TCUTIL_RET_FAILURE;
  }

  /*validate response type*/
  err = ssess->getArgType(index, response);
  if ( err != 0 ) {
    pfc_log_error("GetArgType Session Corrupted");
    return TCUTIL_RET_FATAL;
  }

  if (response != PFC_IPCTYPE_INT32) {
    return TCUTIL_RET_FAILURE;
  }

  /*retrieve data*/
  err = ssess->getArgument(index, *data);
  if ( err != 0 ) {
    pfc_log_error("GetArg Session Corrupted");
    return TCUTIL_RET_FATAL;
  }

  return TCUTIL_RET_SUCCESS;
}

/*retrieve uint64_t data from IPC session*/
TcUtilRet TcServerSessionUtils::get_uint64(
                          pfc::core::ipc::ServerSession* ssess,
                          uint32_t index,
                          uint64_t* data) {
  uint8_t err = 0;
  uint32_t arg_count = 0;
  pfc_ipctype_t response;
  if ( ssess == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*validate response count*/
  arg_count = ssess->getArgCount();
  if ( index > arg_count ) {
    pfc_log_error("Index Exceeded Argument Count");
    return TCUTIL_RET_FAILURE;
  }
  if ( data == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*validate response type*/
  err = ssess->getArgType(index, response);
  if ( err != 0 ) {
    pfc_log_error("Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  if (response != PFC_IPCTYPE_UINT64) {
    return TCUTIL_RET_FAILURE;
  }
  /*retrieve data*/
  err = ssess->getArgument(index, *data);
  if ( err != 0 ) {
    pfc_log_error("Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*retrieve uint32_t data from IPC session*/
TcUtilRet TcServerSessionUtils::get_uint32(
                          pfc::core::ipc::ServerSession* ssess,
                          uint32_t index,
                          uint32_t* data) {
  uint8_t err = 0;
  uint32_t arg_count = 0;
  pfc_ipctype_t response;
  if ( ssess == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*validate response count*/
  arg_count = ssess->getArgCount();
  if ( index > arg_count ) {
    pfc_log_error("Index Exceeded Argument Count");
    return TCUTIL_RET_FAILURE;
  }
  if ( data == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*validate response type*/
  err = ssess->getArgType(index, response);
  if ( err != 0 ) {
    pfc_log_error("Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  if (response != PFC_IPCTYPE_UINT32) {
    return TCUTIL_RET_FAILURE;
  }
  /*retrieve data*/
  err = ssess->getArgument(index, *data);
  if ( err != 0 ) {
    pfc_log_error("Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*retrieve uint8_t data from IPC session*/
TcUtilRet TcServerSessionUtils::get_uint8(
                          pfc::core::ipc::ServerSession* ssess,
                          uint32_t index,
                          uint8_t* data) {
  uint8_t err = 0;
  uint32_t arg_count = 0;
  pfc_ipctype_t response;
  if ( ssess == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*validate response count*/
  arg_count = ssess->getArgCount();
  if ( index > arg_count ) {
    pfc_log_error("Index Exceeded Argument Count");
    return TCUTIL_RET_FAILURE;
  }
  if ( data == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*validate response type*/
  err = ssess->getArgType(index, response);
  if ( err != 0 ) {
    pfc_log_error("Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  if (response != PFC_IPCTYPE_UINT8) {
    return TCUTIL_RET_FAILURE;
  }
  /*retrieve data*/
  err = ssess->getArgument(index, *data);
  if ( err != 0 ) {
    pfc_log_error("Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*retrieve string data from IPC session*/
TcUtilRet TcServerSessionUtils::get_string(
                          pfc::core::ipc::ServerSession* ssess,
                          uint32_t index,
                          std::string& data) {
  uint8_t err = 0;
  uint32_t arg_count = 0;
  const char *temp = NULL;
  pfc_ipctype_t response;
  if ( ssess == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*validate response count*/
  arg_count = ssess->getArgCount();
  if ( index > arg_count ) {
    pfc_log_error("Index Exceeded Argument Count");
    return TCUTIL_RET_FAILURE;
  }
  /*validate response type*/
  err = ssess->getArgType(index, response);
  if ( err != 0 ) {
    pfc_log_error("Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  if ( response != PFC_IPCTYPE_STRING ) {
    return TCUTIL_RET_FAILURE;
  }
  /*retrieve data*/
  err = ssess->getArgument(index, temp);
  if ( err != 0 ) {
    pfc_log_error("Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  if ( temp == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  data.assign(temp);
  return TCUTIL_RET_SUCCESS;
}
/*retrieve structure from IPC session*/
TcUtilRet TcServerSessionUtils::get_struct(
                          pfc::core::ipc::ServerSession* ssess,
                          uint32_t index,
                          const pfc_ipcstdef_t &def,
                          void* data) {
  uint8_t err = 0;
  uint32_t arg_count = 0;
  pfc_ipctype_t response;
  if ( ssess == NULL || data == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*validate response count*/
  arg_count = ssess->getArgCount();
  if ( index > arg_count ) {
    pfc_log_error("Index Exceeded Argument Count");
    return TCUTIL_RET_FAILURE;
  }
  /*validate response type*/
  err = ssess->getArgType(index, response);
  if ( err != 0 ) {
    pfc_log_error("Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  if (response != PFC_IPCTYPE_STRUCT) {
    return TCUTIL_RET_FAILURE;
  }
  /*retrieve data*/
  err = ssess->getArgument(index, def, data);
  if ( err != 0 ) {
    pfc_log_error("Session Corrupted");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*set uint64_t data to IPC session*/
TcUtilRet TcServerSessionUtils::set_uint64(
                          pfc::core::ipc::ServerSession* ssess,
                          uint64_t data) {
  uint8_t err = 0;
  if ( ssess == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*append data to session*/
  err = ssess->addOutput(data);
  if ( err != 0 ) {
    pfc_log_error("Write to Session Failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*set uint32_t data to IPC session*/
TcUtilRet TcServerSessionUtils::set_uint32(
                          pfc::core::ipc::ServerSession* ssess,
                          uint32_t data) {
  uint8_t err = 0;
  if ( ssess == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*append data to session*/
  err = ssess->addOutput(data);
  if ( err != 0 ) {
    pfc_log_error("Write to Session Failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*set uint8_t data to IPC session*/
TcUtilRet TcServerSessionUtils::set_uint8(
                          pfc::core::ipc::ServerSession* ssess,
                          uint8_t data) {
  uint8_t err = 0;
  if ( ssess == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  /*append data to session*/
  err = ssess->addOutput(data);
  if ( err != 0 ) {
    pfc_log_error("Write to Session Failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}

/*set string data to IPC session*/
TcUtilRet TcServerSessionUtils::set_string(
                          pfc::core::ipc::ServerSession* ssess,
                          std::string& data) {
  uint8_t err = 0;
  if (ssess == NULL) {
    return TCUTIL_RET_FAILURE;
  }
  /*append data to session*/
  err = ssess->addOutput(data);
  if ( err != 0 ) {
    pfc_log_error("Write to Session Failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}
/*set structure to IPC session*/
TcUtilRet TcServerSessionUtils::set_struct(
                          pfc::core::ipc::ServerSession* ssess,
                          const pfc_ipcstdef_t &def,
                          const void* data) {
  uint8_t err = 0;
  if ( data == NULL || ssess == NULL ) {
    return TCUTIL_RET_FAILURE;
  }
  err = ssess->addOutput(def, data);
  if ( err != 0 ) {
    pfc_log_error("Write to Session Failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}
}  // namespace tc
}  // namespace unc
