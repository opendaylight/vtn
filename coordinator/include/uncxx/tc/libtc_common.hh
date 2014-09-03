/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _INCLUDE_LIB_TC_COMMON_HH_
#define _INCLUDE_LIB_TC_COMMON_HH_

#include <pfcxx/ipc_server.hh>
#include <pfcxx/ipc_client.hh>
#include <pfc/log.h>
#include <string>

namespace unc {
namespace tc {

/*response value*/
typedef enum {
  TCUTIL_RET_SUCCESS = 0,
  TCUTIL_RET_FAILURE,
  TCUTIL_RET_FATAL
} TcUtilRet;

/*provides methods to access IPC server session data*/
class TcServerSessionUtils {
 public:
  /*method to set timeout for server session*/
  static TcUtilRet
      set_srv_timeout(pfc::core::ipc::ServerSession* ssess,
                      const pfc_timespec_t *timeout);
  /*methods to retrieve data*/
  static TcUtilRet get_int32(
               pfc::core::ipc::ServerSession* ssess,
               uint32_t index,
               int32_t * data);

  static TcUtilRet get_uint64(
               pfc::core::ipc::ServerSession* ssess,
               uint32_t index,
               uint64_t* data);

  static TcUtilRet get_uint32(
               pfc::core::ipc::ServerSession* ssess,
               uint32_t index,
               uint32_t* data);

  static TcUtilRet get_uint8(
               pfc::core::ipc::ServerSession* ssess,
               uint32_t index,
               uint8_t* data);

  static TcUtilRet get_string(
               pfc::core::ipc::ServerSession* ssess,
               uint32_t index,
               std::string& data);

  static TcUtilRet get_struct(
               pfc::core::ipc::ServerSession* ssess,
               uint32_t index,
               const pfc_ipcstdef_t &def,
               void* data);
  /*methods to set data*/
  static TcUtilRet set_uint64(
               pfc::core::ipc::ServerSession* ssess,
               uint64_t data);

  static TcUtilRet set_uint32(
               pfc::core::ipc::ServerSession* ssess,
               uint32_t data);

  static TcUtilRet set_uint8(
               pfc::core::ipc::ServerSession* ssess,
               uint8_t data);

  static TcUtilRet set_string(
               pfc::core::ipc::ServerSession* ssess,
               std::string& data);

  static TcUtilRet set_struct(
               pfc::core::ipc::ServerSession* ssess,
               const pfc_ipcstdef_t &def,
               const void* data);
};

/*provides methods to access IPC client session data*/
class TcClientSessionUtils {
 public:
  static pfc::core::ipc::ClientSession* create_tc_client_session(
                                            std::string channel_name,
                                            uint32_t service_id,
                                            pfc_ipcconn_t &conn,
                                            pfc_bool_t infinite_timeout = PFC_TRUE);

  static TcUtilRet tc_session_invoke(pfc::core::ipc::ClientSession* csess,
                                     pfc_ipcresp_t& response);

  static TcUtilRet tc_session_close(pfc::core::ipc::ClientSession** csess,
                                    pfc_ipcconn_t conn);
  /*methods to retrieve data*/

  static TcUtilRet get_uint64(
               pfc::core::ipc::ClientSession* csess,
               uint32_t index,
               uint64_t* data);

  static TcUtilRet get_uint32(
               pfc::core::ipc::ClientSession* csess,
               uint32_t index,
               uint32_t* data);

  static TcUtilRet get_uint8(
               pfc::core::ipc::ClientSession* csess,
               uint32_t index,
               uint8_t* data);

  static TcUtilRet get_string(
               pfc::core::ipc::ClientSession* csess,
               uint32_t index,
               std::string& data);
  /*methods to set data*/
  static TcUtilRet set_uint64(
               pfc::core::ipc::ClientSession* csess,
               uint64_t data);

  static TcUtilRet set_uint32(
               pfc::core::ipc::ClientSession* csess,
               uint32_t data);

  static TcUtilRet set_uint8(
               pfc::core::ipc::ClientSession* csess,
               uint8_t data);

  static TcUtilRet set_string(
               pfc::core::ipc::ClientSession* csess,
               std::string& data);
};

}  // namespace tc
}  // namespace unc
#endif
