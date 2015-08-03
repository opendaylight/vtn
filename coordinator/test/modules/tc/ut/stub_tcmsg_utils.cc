/*
 * Copyright (c) 2013-2015 NEC Corporation
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
extern int stub_get_arg;
extern int stub_get_arg1;
extern int stub_get_arg_fail;
extern int stub_clnt_forward;
extern int stub_same_driverid;
extern int stub_set_string;

#define CNTRL_COUNT 3
#define DRIVER_COUNT 2

#define SUCCESS 0
#define FAILURE 1

uint32_t resp_count = 0;
int arg_count = 0;

uint32_t
pfc_ipcclnt_getrescount(pfc_ipcsess_t *sess) {
    pfc_log_debug("resp_count:%d", resp_count);
    return resp_count;
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

int
pfc_ipcsrv_getarg_structname(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t index,
           const char **PFC_RESTRICT namepp)
{
      pfc_log_debug("arg_count:%d", arg_count);

      return arg_count;

}
namespace unc {
namespace tc {

pfc::core::ipc::ClientSession*
    TcClientSessionUtils::create_tc_client_session(std::string channel_name,
                                              uint32_t service_id,
                                              pfc_ipcconn_t &conn,
                                              pfc_bool_t infinite_timeout,
                                              pfc_bool_t to_tclib_test) {
  if ( stub_create_session ) {
    pfc_log_error("create client session = NULL");
    return NULL;
  }

  pfc::core::ipc::ClientSession* result_session;
  int err = 0;
  err = pfc_ipcclnt_altopen(channel_name.c_str(), &conn);
  if ( err != 0 || conn == 0 ) {
    pfc_log_fatal("pfc_ipcclnt_altopen failed");
    return NULL;
  }
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

/*
void deleteSession()
{
  if ( result_session_ != 0 )
  {
    delete result_session_;
    result_session_ =NULL;
  }

}
*/

TcUtilRet
TcClientSessionUtils::tc_session_invoke(pfc::core::ipc::ClientSession* csess,
                                        pfc_ipcresp_t& response) {
  if ( stub_session_invoke ) {
    pfc_log_error("session_invoke failed");
    return TCUTIL_RET_FATAL;
  }
  if ( stub_response ) {
    /*TC_FAILURE == UNC_CT_PFC = 1*/
    response = FAILURE;
  } else {
    response = SUCCESS;
  }
  return TCUTIL_RET_SUCCESS;
}


TcUtilRet
TcClientSessionUtils::set_uint32(pfc::core::ipc::ClientSession* csess,
                                 uint32_t data) {
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

TcUtilRet
TcClientSessionUtils::get_uint32(pfc::core::ipc::ClientSession* csess,
                                 uint32_t index,
                                 uint32_t* data) {
  if ( stub_get_arg == 1) {
    pfc_log_error("set_uint failed");
    return TCUTIL_RET_FAILURE;
  }
  else if(stub_get_arg == 2) {
    int value = 101;
    *data = value;
    if(index == 1) {
      *data = 0;
       if(stub_get_arg_fail == 1) { 
          *data = 1;
       }
    }
  }
  uint8_t err = 0;
  if ( csess == NULL ) {
    pfc_log_error("Session param is NULL");
    return TCUTIL_RET_FAILURE;
  }
  //err = csess->addOutput(data);
  if ( err != 0 ) {
    pfc_log_fatal("Appending data to ClientSession failed");
    return TCUTIL_RET_FATAL;
  }
  return TCUTIL_RET_SUCCESS;
}




TcUtilRet
TcClientSessionUtils::set_uint8(pfc::core::ipc::ClientSession* csess,
                                uint8_t data) {
  if (stub_set_arg) {
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
    pfc_log_error("Appending data to ClientSession failed");
    return TCUTIL_RET_FATAL;
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
    //*data = 0;
    if (stub_get_arg1 == 1) {
    *data = 0;
    }
    if (stub_get_arg1 == 2) {
      return TCUTIL_RET_FAILURE;
    }
    if (stub_get_arg1 == 3) {
      *data = 1;
    }
  } else if ( index == 6 ) {
    if ( stub_same_driverid ) {
      *data = UNC_CT_PFC;
    } else {
      *data = UNC_CT_VNP;
    }
  }
  return TCUTIL_RET_SUCCESS;
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

TcUtilRet TcClientSessionUtils::set_string(
                          pfc::core::ipc::ClientSession* csess,
                          std::string& data) {
if (stub_set_string)
{
  return TCUTIL_RET_SUCCESS;
}
else
{
 return TCUTIL_RET_FAILURE;
}
return TCUTIL_RET_SUCCESS;
}




}  // namespace tc
}  // namespace unc
