/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include <uncxx/tc/libtc_common.hh>
#include "tcmsg.hh"

extern int stub_create_session;
extern int stub_session_invoke;
extern int stub_response;
extern int stub_set_arg;
extern int stub_clnt_forward;
extern int stub_same_driverid;

#define CNTRL_COUNT 3
#define DRIVER_COUNT 2

#define SUCCESS 0
#define FAILURE 1

uint32_t resp_count = 0;

uint32_t
pfc_ipcclnt_getrescount(pfc_ipcsess_t *sess) {
  pfc_log_debug("resp_count:%d", resp_count);
  return resp_count;
}

int
pfc_ipcclnt_altopen(const char *PFC_RESTRICT name,
                    pfc_ipcconn_t *PFC_RESTRICT connp) {
  return 0;
}

int
pfc_ipcclnt_altclose(pfc_ipcconn_t conn) {
  return 0;
}


int
pfc_ipcclnt_forward_tosrv(pfc_ipcsrv_t *PFC_RESTRICT dsrv,
                          pfc_ipcsess_t *PFC_RESTRICT ssess, uint32_t begin,
                          uint32_t end) {
  return 0;
}

int
pfc_ipcclnt_forward(pfc_ipcsess_t *PFC_RESTRICT dsess,
                    pfc_ipcsess_t *PFC_RESTRICT ssess, uint32_t begin,
                    uint32_t end) {
  return 0;
}

namespace unc {
namespace tc {

pfc::core::ipc::ClientSession*
    TcClientSessionUtils::create_tc_client_session(std::string channel_name,
                                                   uint32_t service_id,
                                                   pfc_ipcconn_t &conn,
                                                   pfc_bool_t infinite_timeout) {
      if ( stub_create_session ) {
        pfc_log_error("create client session = NULL");
        return NULL;
      }

      pfc::core::ipc::ClientSession* result_session;
      int err = 0;
      result_session = new pfc::core::ipc::ClientSession(conn,
                                                         "tclib",
                                                         service_id,
                                                         err);
      if ( result_session == NULL ) {
        pfc_log_fatal("creating a ClientSession failed");
        return NULL;
      }

      return result_session;
    }

TcUtilRet
    TcClientSessionUtils::tc_session_close
    (pfc::core::ipc::ClientSession** csess,
     pfc_ipcconn_t conn) {
      pfc_log_debug("tc_session_close invoked");
      if (*csess != NULL) {
        delete *csess;
        *csess = NULL;
      }
      return TCUTIL_RET_SUCCESS;
    }


TcUtilRet
    TcClientSessionUtils::tc_session_invoke
    (pfc::core::ipc::ClientSession* csess,
     pfc_ipcresp_t& response) {
      if ( stub_session_invoke ) {
        pfc_log_error("session_invoke failed");
        return TCUTIL_RET_FATAL;
      }
      if ( stub_response ) {
        response = FAILURE;
      } else {
        response = SUCCESS;
      }
      return TCUTIL_RET_SUCCESS;
    }

TcUtilRet
TcClientSessionUtils::get_uint8(pfc::core::ipc::ClientSession* csess,
                                uint32_t index,
                                uint8_t* data) {
  if ( index == 0 ) {
    *data = DRIVER_COUNT;
  } else if ( index == 1 ) {
    *data = UNC_CT_PFC;
  } else if ( index == 2 || index == 7 ) {
    *data = CNTRL_COUNT;
  } else if ( index == 6 ) {
    if ( stub_same_driverid ) {
      *data = UNC_CT_PFC;
    } else {
      *data = UNC_CT_VNP;
    }
  }
  return TCUTIL_RET_SUCCESS;
}

TcUtilRet
    TcClientSessionUtils::get_uint64(pfc::core::ipc::ClientSession* csess,
                                    uint32_t index,
                                    uint64_t* data) {
      if ( index == 0 ) {
        *data = DRIVER_COUNT;
      } else if ( index == 1 ) {
        *data = UNC_CT_PFC;
      } else if ( index == 2 || index == 7 ) {
        *data = CNTRL_COUNT;
      } else if ( index == 6 ) {
        if ( stub_same_driverid ) {
          *data = UNC_CT_PFC;
        } else {
          *data = UNC_CT_VNP;
        }
      }
      return TCUTIL_RET_SUCCESS;
    }



TcUtilRet
TcClientSessionUtils::set_uint64(pfc::core::ipc::ClientSession* csess,
                                 uint64_t data) {
  if ( stub_set_arg ) {
    pfc_log_error("set_uint failed");
    return TCUTIL_RET_FAILURE;
  }

  uint8_t err = 0;
  if ( csess == NULL ) {
    pfc_log_error("Session param is NULL");
    return TCUTIL_RET_FAILURE;
  }
  err = csess->addOutput(data);
  if ( err != 0 ) {
    pfc_log_fatal("Appending data to ClientSession failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}



TcUtilRet TcServerSessionUtils::get_int32(
                          pfc::core::ipc::ServerSession* ssess,
                          uint32_t index,
                          int32_t * data) {
return TCUTIL_RET_SUCCESS;
}


TcUtilRet
    TcClientSessionUtils::set_uint32(pfc::core::ipc::ClientSession* csess,
                                     uint32_t data) {
      if ( stub_set_arg ) {
        pfc_log_error("set_uint failed");
        return TCUTIL_RET_FAILURE;
      }
      if (csess != NULL)
        return TCUTIL_RET_SUCCESS;
      else
        return TCUTIL_RET_FAILURE;
    }

TcUtilRet
    TcClientSessionUtils::set_uint8(pfc::core::ipc::ClientSession* csess,
                                    uint8_t data) {
      if (stub_set_arg) {
        pfc_log_error("set_uint failed");
        return TCUTIL_RET_FAILURE;
      }
      if (csess != NULL)
        return TCUTIL_RET_SUCCESS;
      else
        return TCUTIL_RET_FAILURE;
    }

TcUtilRet
    TcClientSessionUtils::set_string(pfc::core::ipc::ClientSession* csess,
                                     std::string& data) {
      if (csess != NULL)
        return TCUTIL_RET_SUCCESS;
      else
        return TCUTIL_RET_FAILURE;
    }
/*
TcUtilRet
    TcClientSessionUtils::get_uint8(pfc::core::ipc::ClientSession* csess,
                                    uint32_t index,
                                    uint8_t* data) {
      if ( index == 0 ) {
        *data = DRIVER_COUNT;
      } else if ( index == 1 ) {
        *data = UNC_CT_PFC;
      } else if ( index == 2 || index == 7 ) {
        *data = CNTRL_COUNT;
      } else if ( index == 6 ) {
        if ( stub_same_driverid ) {
          *data = UNC_CT_PFC;
        } else {
          *data = UNC_CT_VNP;
        }
      }
      return TCUTIL_RET_SUCCESS;
    }
*/
TcUtilRet TcClientSessionUtils::get_uint32(
    pfc::core::ipc::ClientSession* csess,
    uint32_t index,
    uint32_t* data) {
  if (csess != NULL)
    return TCUTIL_RET_SUCCESS;
  else
    return TCUTIL_RET_FAILURE;
}

TcUtilRet TcClientSessionUtils::get_string(pfc::core::ipc::ClientSession* csess,
                                           uint32_t index,
                                           std::string& data) {
  if ( index == 3 || index == 8 ) {
    const char *temp= "C1";
    data.assign(temp);
  } else if ( index == 4 || index == 9 ) {
    const char *temp= "C2";
    data.assign(temp);
  } else if ( index == 5 || index == 10 ) {
    const char *temp= "C3";
    data.assign(temp);
  }

  return TCUTIL_RET_SUCCESS;
}

}  // namespace tc
}  // namespace unc
