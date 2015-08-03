/*
 * Copyright (c) 2013-2015 NEC Corporation
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
#include <uncxx/tclib/tclib_defs.hh>

using namespace unc::tclib;

namespace unc {

namespace tc {

#define SESSION_ID 10
#define CONFIG_ID 200

typedef enum {
  TCUTIL_RET_SUCCESS=0,
  TCUTIL_RET_FAILURE,
  TCUTIL_RET_FATAL
} TcUtilRet;

typedef enum {
  LIB_READ_NONE = 0,
  LIB_NOTIFY_SESSION,
  LIB_COMMIT_TRANS_START,
  LIB_COMMIT_TRANS_VOTE_GLOBAL,
  LIB_COMMIT_DRIVER_VOTE_GLOBAL,
  LIB_COMMIT_GLOBAL_ABORT,
  LIB_AUDIT_START,
  LIB_AUDIT_VOTE,
  LIB_AUDIT_DRIVER_VOTE_GLOBAL,
  LIB_AUDIT_GLOBAL_ABORT,
  LIB_COMMON,
  LIB_ABORT_CANDIDATE,
  LIB_AUDIT_CONFIG,
  LIB_UPDATE_KEY_LIST,
  LIB_COMMIT_DRIVER_RESULT,
  LIB_AUDIT_DRIVER_RESULT,
  LIB_WRITE_API,
  LIB_DRIVER_AUDIT
} SessUint32ReadType;

typedef enum {
  RETURN_SUCCESS = 0,
  RETURN_FAILURE,
  RETURN_FAILURE_1,
  RETURN_FAILURE_2,
  RETURN_FAILURE_3,
  RETURN_FAILURE_4,
  RETURN_FAILURE_5,
  RETURN_STRUCT_FAILURE_1,
  RETURN_STRUCT_FAILURE_2
} ReturnType;


class TcServerSessionUtils {
 public:
  TcServerSessionUtils();
  ~TcServerSessionUtils();

  static TcUtilRet get_uint32(
      pfc::core::ipc::ServerSession* ssess,
      uint32_t index,
      uint32_t* data);

  static TcUtilRet get_uint8(
      pfc::core::ipc::ServerSession* ssess,
      uint32_t index,
      uint8_t* data);

  static TcUtilRet get_uint64(
      pfc::core::ipc::ServerSession* ssess,
      uint32_t index,
      uint64_t* data);

  static TcUtilRet get_string(
      pfc::core::ipc::ServerSession* ssess,
      uint32_t index,
      std::string& data);

  static TcUtilRet get_struct(
      pfc::core::ipc::ServerSession* ssess,
      uint32_t index,
      const pfc_ipcstdef_t &def,
      void* data);

  static TcUtilRet set_uint32(
      pfc::core::ipc::ServerSession* ssess,
      uint32_t data);

  static TcUtilRet set_uint64(
      pfc::core::ipc::ServerSession* ssess,
      uint64_t data);

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

  inline static void set_read_type(SessUint32ReadType read_type) {
    read_type_ = read_type;
  }
  static SessUint32ReadType read_type_;

  inline static void set_return_type(ReturnType return_type) {
    return_type_ = return_type;
  }
  static ReturnType return_type_;

  // audit driver result set failure purpose
  inline static void set_return_type_1(ReturnType return_type) {
    return_type_1_ = return_type;
  }
  static ReturnType return_type_1_;

  inline static void set_oper_type(TcMsgOperType oper_type) {
    oper_type_ = oper_type;
  }
  static TcMsgOperType oper_type_;

  inline static void set_ctr_type(unc_keytype_ctrtype_t ctr_type) {
    ctr_type_ = ctr_type;
  }
  static unc_keytype_ctrtype_t ctr_type_;

  inline static void set_numof_errors(uint32_t num_error) {
    num_of_errors_ = num_error;
  }
  static uint32_t num_of_errors_;

  static TcUtilRet update_key_list_handler_uint64(uint32_t index,
                                           uint64_t& data);
  static TcUtilRet update_key_list_handler_uint32(uint32_t index,
                                           uint32_t& data);

  inline static void set_updatekey_failure(pfc_bool_t key_failure) {
    update_key_failure_ = key_failure;
  }
  static pfc_bool_t update_key_failure_;

};

class TcClientSessionUtils {
 public:
  TcClientSessionUtils();
  ~TcClientSessionUtils();

  static pfc::core::ipc::ClientSession& create_tc_client_session(
      std::string channel_name,
      uint32_t service_id);

  static TcUtilRet tc_session_invoke(pfc::core::ipc::ClientSession* csess,
                                     pfc_ipcresp_t& response);

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

  static TcUtilRet set_uint32 (
      pfc::core::ipc::ClientSession* csess,
      uint32_t data);

  static TcUtilRet set_uint8 (
      pfc::core::ipc::ClientSession* csess,
      uint8_t data);

  static TcUtilRet set_string (
      pfc::core::ipc::ClientSession* csess,
      std::string& data);

  inline static void set_return_type(ReturnType return_type) {
    return_type_ = return_type;
  }
  static ReturnType return_type_;

};

}  // namespace tc
}  // namespace unc

#endif
