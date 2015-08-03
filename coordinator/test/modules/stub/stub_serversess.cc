/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <pfcxx/module.hh>
#include <pfcxx/synch.hh>
#include "tclib_module/libtc_common.hh"
#include <unc/tc/external/tc_services.h>

int pfc_ipcsrv_getargtype(pfc_ipcsrv_t *PFC_RESTRICT srv,
                          uint32_t index,
                          pfc_ipctype_t *PFC_RESTRICT typep) {
  if (index == 4)
    *typep = PFC_IPCTYPE_STRING;
  if (index == 7)
    *typep = PFC_IPCTYPE_UINT32;
  return 0;
}

namespace unc {
namespace tc {

SessUint32ReadType TcServerSessionUtils::read_type_ = LIB_READ_NONE;
ReturnType TcServerSessionUtils::return_type_ = RETURN_SUCCESS;
TcMsgOperType TcServerSessionUtils::oper_type_ = MSG_NONE;
unc_keytype_ctrtype_t TcServerSessionUtils::ctr_type_ = UNC_CT_UNKNOWN;
uint32_t TcServerSessionUtils::num_of_errors_ = 0;
pfc_bool_t TcServerSessionUtils::update_key_failure_ = PFC_FALSE;
ReturnType TcServerSessionUtils::return_type_1_ = RETURN_SUCCESS;

TcServerSessionUtils::TcServerSessionUtils() {
  read_type_ = LIB_READ_NONE;
  return_type_ = RETURN_SUCCESS;
  oper_type_ = MSG_NONE;
  ctr_type_ = UNC_CT_UNKNOWN;
  num_of_errors_ = 0;
  update_key_failure_ = PFC_FALSE;
}

TcServerSessionUtils::~TcServerSessionUtils() {
  read_type_ = LIB_READ_NONE;
  return_type_ = RETURN_SUCCESS;
  oper_type_ = MSG_NONE;
  num_of_errors_ = 0;
  ctr_type_ = UNC_CT_UNKNOWN;
  update_key_failure_ = PFC_FALSE;
}

int getArgType(uint32_t index, pfc_ipctype_t &type) {
  if (index == 4)
    type = PFC_IPCTYPE_STRING;
  if (index == 7)
    type = PFC_IPCTYPE_UINT32;

  return 0;
}

TcUtilRet
TcServerSessionUtils::update_key_list_handler_uint64(uint32_t index,
                                              uint64_t& data) {
  if (return_type_ == RETURN_SUCCESS) {
    if (( index == 5) || (index == 8))
      data = 0;  // resp_code
    if (index == 6)
      data = num_of_errors_;
    if (index == 9)
      data = 1;

    if (index == 7)
      data = 20;
    if (index == 10)
      data = 21;
    return TCUTIL_RET_SUCCESS;
  }

  if ((return_type_ == RETURN_FAILURE_1) && (index == 5)) {
    return TCUTIL_RET_FAILURE;
  }

  if ((return_type_ == RETURN_FAILURE_2) && (index == 6)) {
    return TCUTIL_RET_FAILURE;
  }

  if ((return_type_ == RETURN_FAILURE_3) && (index == 7)) {
    return TCUTIL_RET_FAILURE;
  }

  if ((return_type_ == RETURN_FAILURE_3) && (index == 10)) {
    data = 21;
    return TCUTIL_RET_FAILURE;
  }
  return TCUTIL_RET_SUCCESS;
}

TcUtilRet
TcServerSessionUtils::update_key_list_handler_uint32(uint32_t index,
                                              uint32_t& data) {
  if (return_type_ == RETURN_SUCCESS) {
    if (( index == 5) || (index == 8))
      data = 0;  // resp_code
    if (index == 6)
      data = num_of_errors_;
    if (index == 9)
      data = 1;

    if (index == 7)
      data = 20;
    if (index == 10)
      data = 21;
    return TCUTIL_RET_SUCCESS;
  }

  if ((return_type_ == RETURN_FAILURE_1) && (index == 5)) {
    return TCUTIL_RET_FAILURE;
  }

  if ((return_type_ == RETURN_FAILURE_2) && (index == 6)) {
    return TCUTIL_RET_FAILURE;
  }

  if ((return_type_ == RETURN_FAILURE_3) && (index == 7)) {
    return TCUTIL_RET_FAILURE;
  }

  if ((return_type_ == RETURN_FAILURE_3) && (index == 10)) {
    data = 21;
    return TCUTIL_RET_FAILURE;
  }
  return TCUTIL_RET_SUCCESS;
}

TcUtilRet TcServerSessionUtils::get_uint32(
    pfc::core::ipc::ServerSession* ssess,
    uint32_t index,
    uint32_t* data) {
  TcUtilRet ret = TCUTIL_RET_SUCCESS; 
  uint32_t temp_data = 0;
  switch (read_type_) {
    case LIB_NOTIFY_SESSION:
    case LIB_ABORT_CANDIDATE:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 0)
          *data = SESSION_ID;
        if (index == 1)
          *data = CONFIG_ID;
        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE_1) && (index == 0))
        return TCUTIL_RET_FAILURE;

      if ((return_type_ == RETURN_FAILURE_2) && (index == 1))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_COMMIT_TRANS_START:
    case LIB_COMMIT_DRIVER_VOTE_GLOBAL:
    case LIB_COMMIT_GLOBAL_ABORT:
    case LIB_COMMIT_DRIVER_RESULT:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 1)
          *data = SESSION_ID;
        if (index == 2)
          *data = CONFIG_ID;
        /* below two fields are for driver result alone */
        if (index == 5)
          *data = 0;  // resp_code
        if (index == 6)
          *data = 0;  // num of errors
        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE_1) && (index == 1))
        return TCUTIL_RET_FAILURE;

      if ((return_type_ == RETURN_FAILURE_2) && (index == 2))
        return TCUTIL_RET_FAILURE;

      if ((return_type_ == RETURN_FAILURE_4) && (index == 5))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_AUDIT_START:
    case LIB_AUDIT_GLOBAL_ABORT:
    case LIB_AUDIT_DRIVER_RESULT:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 1)
          *data = SESSION_ID;
        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE_1) && (index == 1))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_COMMON:
      if (return_type_ == RETURN_SUCCESS) {
        return TCUTIL_RET_SUCCESS;
      }
      if (return_type_ == RETURN_FAILURE)
        return TCUTIL_RET_FAILURE;

    case LIB_UPDATE_KEY_LIST:
      ret = update_key_list_handler_uint32(index, temp_data);
      *data = temp_data;
      return ret;

    case LIB_AUDIT_DRIVER_VOTE_GLOBAL:
      if (return_type_ == RETURN_FAILURE_1 ||
          return_type_ == RETURN_FAILURE_3)
        return TCUTIL_RET_FAILURE;

      return TCUTIL_RET_SUCCESS;
    default:
      return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::get_uint64(
    pfc::core::ipc::ServerSession* ssess,
    uint32_t index,
    uint64_t* data) {

  uint64_t temp_data = 0;
  TcUtilRet ret = TCUTIL_RET_SUCCESS; 
  switch (read_type_) {
    case LIB_NOTIFY_SESSION:
    case LIB_ABORT_CANDIDATE:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 0)
          *data = SESSION_ID;
        if (index == 1)
          *data = CONFIG_ID;
        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE_1) && (index == 0))
        return TCUTIL_RET_FAILURE;

      if ((return_type_ == RETURN_FAILURE_2) && (index == 1))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_COMMIT_TRANS_START:
    case LIB_COMMIT_DRIVER_VOTE_GLOBAL:
    case LIB_COMMIT_GLOBAL_ABORT:
    case LIB_COMMIT_DRIVER_RESULT:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 1)
          *data = SESSION_ID;
        if (index == 2)
          *data = CONFIG_ID;
        /* below two fields are for driver result alone */
        if (index == 5)
          *data = 0;  // resp_code
        if (index == 6)
          *data = 0;  // num of errors
        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE_1) && (index == 1))
        return TCUTIL_RET_FAILURE;

      if ((return_type_ == RETURN_FAILURE_2) && (index == 2))
        return TCUTIL_RET_FAILURE;

      if ((return_type_ == RETURN_FAILURE_4) && (index == 5))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_AUDIT_START:
    case LIB_AUDIT_GLOBAL_ABORT:
    case LIB_AUDIT_DRIVER_RESULT:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 1)
          *data = SESSION_ID;
        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE_1) && (index == 1))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_COMMON:
      if (return_type_ == RETURN_SUCCESS) {
        return TCUTIL_RET_SUCCESS;
      }
      if (return_type_ == RETURN_FAILURE)
        return TCUTIL_RET_FAILURE;

    case LIB_UPDATE_KEY_LIST:
      ret = update_key_list_handler_uint64(index, temp_data);
      *data = temp_data;
      return ret;

    case LIB_AUDIT_DRIVER_VOTE_GLOBAL:
      if (return_type_ == RETURN_FAILURE_1 ||
          return_type_ == RETURN_FAILURE_3)
        return TCUTIL_RET_FAILURE;

      return TCUTIL_RET_SUCCESS;
    default:
      return TCUTIL_RET_SUCCESS;
  }
}



TcUtilRet TcServerSessionUtils::get_uint8(
    pfc::core::ipc::ServerSession* ssess,
    uint32_t index,
    uint8_t* data) {

  switch (read_type_) {
    case LIB_COMMIT_TRANS_START:
    case LIB_COMMIT_GLOBAL_ABORT:
      if (index == 0)
        *data = oper_type_;
      if (index == 3)
        *data = 0;  // end_result

      if ((return_type_ == RETURN_FAILURE) && (index == 0))
        return TCUTIL_RET_FAILURE;
      if ((return_type_ == RETURN_FAILURE_3) && (index == 3))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_COMMIT_DRIVER_VOTE_GLOBAL:
      if (index == 0)
        *data = oper_type_;
      if (index == 3)
        *data = 1;  // ctrl_count
      if ((return_type_ == RETURN_FAILURE) && (index == 0))
        return TCUTIL_RET_FAILURE;
      if ((return_type_ == RETURN_FAILURE_3) && (index == 3))
        return TCUTIL_RET_FAILURE;

      return TCUTIL_RET_SUCCESS;

    case LIB_COMMIT_DRIVER_RESULT:
    case LIB_AUDIT_DRIVER_RESULT:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 0)
          *data = oper_type_;
        if (index == 3)
          *data = 3;  // driver result phase

        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE_4) && (index == 0))
        return TCUTIL_RET_FAILURE;
      if ((return_type_ == RETURN_FAILURE_3) && (index == 3))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_AUDIT_VOTE:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 0)
          *data = oper_type_;
        if (index == 2)
          *data = ctr_type_;  // ctr_type number
        if (index == 4)
          *data = 0;  // audit_result
        if (index == 5)
          *data = 0;  // end_result
        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE) && (index == 0))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_AUDIT_DRIVER_VOTE_GLOBAL:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 0)
          *data = oper_type_;
        if (index == 3)
          *data = 1;  // ctr_count
        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE) && (index == 0))
        return TCUTIL_RET_FAILURE;
      if ((return_type_ == RETURN_FAILURE_1) && (index == 0))
        return TCUTIL_RET_FAILURE;
      if ((return_type_ == RETURN_FAILURE_2) && (index == 3))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_AUDIT_GLOBAL_ABORT:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 0)
          *data = oper_type_;
        if (index == 2)
          *data = 1;  // ctr_type
        if (index == 4)
          *data = 1;  // phase

        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE_2) && (index == 0))
        return TCUTIL_RET_FAILURE;
      if ((return_type_ == RETURN_FAILURE_3) && (index == 2))
        return TCUTIL_RET_FAILURE;
      if ((return_type_ == RETURN_FAILURE_4) && (index == 4))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_AUDIT_CONFIG:
      if (return_type_ == RETURN_SUCCESS) {
        if (index == 0)
          *data = 0;
        if (index == 1)
          *data = 0;
        return TCUTIL_RET_SUCCESS;
      }
      if ((return_type_ == RETURN_FAILURE_1) && (index == 0))
        return TCUTIL_RET_FAILURE;

      if ((return_type_ == RETURN_FAILURE_2) && (index == 1))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    case LIB_AUDIT_START:
      if (index == 0)
        *data = oper_type_;
      if (index == 2)
        *data = ctr_type_;  // ctr_type number
      if (index == 4)
        *data = 0;  // audit_result
      if (index == 5)
        *data = 0;  // end_result
      if ((return_type_ == RETURN_FAILURE_2) && (index == 0))
        return TCUTIL_RET_FAILURE;
      if ((return_type_ == RETURN_FAILURE_3) && (index == 2))
        return TCUTIL_RET_FAILURE;
      if ((return_type_ == RETURN_FAILURE_4) && (index == 4))
        return TCUTIL_RET_FAILURE;
      if ((return_type_ == RETURN_FAILURE_5) && (index == 4))
        return TCUTIL_RET_FAILURE;
      return TCUTIL_RET_SUCCESS;

    default:
      return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::get_string(pfc::core::ipc::ServerSession* ssess,
                                           uint32_t index,
                                           std::string& data) {
  if ((read_type_ ==  LIB_COMMIT_DRIVER_VOTE_GLOBAL) &&
      (return_type_ == RETURN_FAILURE_4) && (index == 4))
    return TCUTIL_RET_FAILURE;

  if (return_type_ == RETURN_FAILURE)
    return TCUTIL_RET_FAILURE;

  if ((read_type_ ==  LIB_AUDIT_DRIVER_VOTE_GLOBAL) &&
      (return_type_ == RETURN_FAILURE_4) && (index == 2))
    return TCUTIL_RET_FAILURE;

  if ((read_type_ ==  LIB_AUDIT_DRIVER_VOTE_GLOBAL) &&
      (return_type_ == RETURN_FAILURE_5) && (index == 4))
    return TCUTIL_RET_FAILURE;

  if ((return_type_ == RETURN_SUCCESS) &&
      (read_type_ == LIB_COMMIT_DRIVER_RESULT) &&
      (update_key_failure_ == PFC_TRUE) &&
      (index == 4))
    return TCUTIL_RET_FAILURE;

  if ((return_type_ == RETURN_SUCCESS) &&
      (read_type_ == LIB_AUDIT_DRIVER_RESULT) &&
      (update_key_failure_ == PFC_TRUE) &&
      (index == 2))
    return TCUTIL_RET_FAILURE;

  if (return_type_ == RETURN_FAILURE && read_type_ == LIB_UPDATE_KEY_LIST)
    return TCUTIL_RET_FAILURE;

  if (index == 7) {
    const char *temp2= "openflow2";
    data.assign(temp2);
    return TCUTIL_RET_SUCCESS;
  }
  const char *temp1= "openflow1";
  data.assign(temp1);
  return TCUTIL_RET_SUCCESS;
}

TcUtilRet TcServerSessionUtils::get_struct(
    pfc::core::ipc::ServerSession* ssess,
    uint32_t index,
    const pfc_ipcstdef_t &def,
    void* data) {
  if ((return_type_1_ == RETURN_STRUCT_FAILURE_1) &&
      (index == 8 || index == 11)) {
    return TCUTIL_RET_FAILURE;
  } else if ((return_type_1_ == RETURN_STRUCT_FAILURE_2) &&
             (index == 9 || index ==12)) {
    return TCUTIL_RET_FAILURE;
  }
  return TCUTIL_RET_SUCCESS;
}

TcUtilRet TcServerSessionUtils::set_uint8(
    pfc::core::ipc::ServerSession* ssess,
    uint8_t data) {
  switch (read_type_) {
    case LIB_COMMIT_TRANS_VOTE_GLOBAL:
      if (return_type_ == RETURN_SUCCESS)
        return TCUTIL_RET_SUCCESS;

      // driver info empty and set failure in that case
      if (return_type_ == RETURN_FAILURE_5)
        return TCUTIL_RET_FAILURE;

      if ((return_type_ == RETURN_FAILURE_1)  /*&& (data == 3)*/) {
        return TCUTIL_RET_FAILURE;
      } else if ((return_type_ == RETURN_FAILURE_2) && (data == 1)) {
        return TCUTIL_RET_FAILURE;
      } else if ((return_type_ == RETURN_FAILURE_3) && (data == 4)) {
        return TCUTIL_RET_FAILURE;
      } else if ((return_type_ == RETURN_FAILURE_4) &&
                 (data == tclib::TC_AUDIT_FAILURE)) {
        return TCUTIL_RET_FAILURE;
      }
      return TCUTIL_RET_SUCCESS;

    case LIB_AUDIT_DRIVER_RESULT:
      if (return_type_1_ == RETURN_SUCCESS)
        return TCUTIL_RET_SUCCESS;

      if (return_type_1_ == RETURN_FAILURE) {
        return TCUTIL_RET_FAILURE;
      }
      return TCUTIL_RET_SUCCESS;

    case LIB_DRIVER_AUDIT:
      if ((return_type_ == RETURN_FAILURE_2) && (data == 1)) {
        return TCUTIL_RET_FAILURE;
      }
      return TCUTIL_RET_SUCCESS;
    default:
      return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::set_string(
    pfc::core::ipc::ServerSession* ssess,
    std::string& data) {
  switch (read_type_) {
    case LIB_COMMIT_TRANS_VOTE_GLOBAL:
    case LIB_WRITE_API:
    case LIB_DRIVER_AUDIT:
      if (return_type_ == RETURN_SUCCESS)
        return TCUTIL_RET_SUCCESS;
      if (return_type_ == RETURN_FAILURE)
        return TCUTIL_RET_FAILURE;
      break;

    default:
      return TCUTIL_RET_SUCCESS;
  }
  return TCUTIL_RET_SUCCESS;
}

TcUtilRet TcServerSessionUtils::set_uint32(
    pfc::core::ipc::ServerSession* ssess,
    uint32_t data) {

  if ((return_type_ == RETURN_FAILURE_1) && (data == 0))
    return TCUTIL_RET_FAILURE;

  if ((return_type_ == RETURN_FAILURE_1) && (data == 1))
    return TCUTIL_RET_FAILURE;

  if ((return_type_ == RETURN_FAILURE_1) && (data == TC_OP_DRIVER_AUDIT))
    return TCUTIL_RET_FAILURE;

  return TCUTIL_RET_SUCCESS;
}

TcUtilRet TcServerSessionUtils::set_uint64(
    pfc::core::ipc::ServerSession* ssess,
    uint64_t data) {

  if ((return_type_ == RETURN_FAILURE_1) && (data == 0))
    return TCUTIL_RET_FAILURE;

  if ((return_type_ == RETURN_FAILURE_1) && (data == 1))
    return TCUTIL_RET_FAILURE;

  if ((return_type_ == RETURN_FAILURE_1) && (data == TC_OP_DRIVER_AUDIT))
    return TCUTIL_RET_FAILURE;

  return TCUTIL_RET_SUCCESS;
}


TcUtilRet TcServerSessionUtils::set_struct(
    pfc::core::ipc::ServerSession* ssess,
    const pfc_ipcstdef_t &def,
    const void* data) {

  if ((return_type_ == RETURN_FAILURE_1)&&(!strcmp(def.ist_name, "key4"))) {
    return TCUTIL_RET_FAILURE;
  }
  if ((return_type_ == RETURN_FAILURE_1)&&(!strcmp(def.ist_name, "val1"))) {
    return TCUTIL_RET_FAILURE;
  }

  return TCUTIL_RET_SUCCESS;
}

ReturnType TcClientSessionUtils::return_type_ = RETURN_SUCCESS;
TcClientSessionUtils::TcClientSessionUtils() {
  return_type_ = RETURN_SUCCESS;
}

TcClientSessionUtils::~TcClientSessionUtils() {
  return_type_ = RETURN_SUCCESS;
}

TcUtilRet TcClientSessionUtils::set_uint32(
    pfc::core::ipc::ClientSession* csess,
    uint32_t data) {
  if (return_type_ == RETURN_FAILURE_1)
    return TCUTIL_RET_FAILURE;

  return TCUTIL_RET_SUCCESS;
}

TcUtilRet TcClientSessionUtils::set_uint8(
    pfc::core::ipc::ClientSession* csess,
    uint8_t data) {
  if (return_type_ == RETURN_FAILURE_3)
    return TCUTIL_RET_FAILURE;
  if (return_type_ == RETURN_FAILURE && data ==0)
    return TCUTIL_RET_FAILURE;
  return TCUTIL_RET_SUCCESS;
}

TcUtilRet TcClientSessionUtils::set_string(
    pfc::core::ipc::ClientSession* csess,
    std::string& data) {
  if (return_type_ == RETURN_FAILURE_2)
    return TCUTIL_RET_FAILURE;
  return TCUTIL_RET_SUCCESS;
}

}  // namespace tc
}  // namespace unc
