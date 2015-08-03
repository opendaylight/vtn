/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 @brief   IPCClientLogicalHandler
 @file    ipc_client_logical_handler.cc
 @ Desc:  This file contains the definition of
          IPCClientConfigurationHandler class
 *
 */


#include "ipc_client_logical_handler.hh"
#include "physicallayer.hh"
#include "unc/upll_svc.h"
#include "unc/upll_errno.h"

using unc::uppl::IPCClientLogicalHandler;

IPCClientLogicalHandler* IPCClientLogicalHandler::
ipc_client_logical_handler_ = NULL;

const unsigned int UPLL_RESPONSE_COUNT = 3;

/**CheckInUseInLogical
 * @Description : This function checks that the key is used in logical by
 *                sending message only to logical layer. This is also needed
 *                to perform semantic validation during KT_CONTROLLER,
 *                KT_SWITCH and KT_BOUNDARY delete operation.
 * @param[in]   : key_type - Specifies the key instances of respective key
 *                types
 *                key_str - void pointer
 *                data_type - UNC_DT_* Specifies the datatype
 * @return      : UNC_RC_SUCCESS is returned when the response
 *                is added to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not
 *                be added to session
 **/
UncRespCode IPCClientLogicalHandler::CheckInUseInLogical(
    unc_key_type_t key_type,
    void* key_str,
    uint32_t data_type) {
  pfc_log_debug("Inside CheckInUseInLogical func");
  pfc_ipcconn_t connp = 0;
  uint8_t in_use = 1;
  int err = pfc_ipcclnt_altopen(UPLL_IPC_CHANNEL_NAME, &connp);
  if (err != 0) {
    pfc_log_error("Could not open upll ipc session");
    return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
  }
  UncRespCode return_code = UNC_RC_SUCCESS;
  ClientSession sess(connp, UPLL_IPC_SERVICE_NAME,
                     UPLL_GLOBAL_CONFIG_SVC_ID, err);
  pfc_log_debug("After client session object creation");
  if (err != 0) {
    pfc_log_error("Session creation to logical failed");
    err = pfc_ipcclnt_altclose(connp);
    if (err != 0) {
      pfc_log_info("Unable to close ipc connection");
    }
    return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
  }
  err |= sess.addOutput((uint32_t)UPLL_IS_KEY_TYPE_IN_USE_OP);
  err |= sess.addOutput(data_type);
  err |= sess.addOutput((uint32_t)key_type);
  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  pfc_log_debug("Key type: %d data_type %d", (uint32_t)key_type, data_type);
  switch (key_type) {
    case UNC_KT_CONTROLLER: {
      key_ctr_t kt_ctr_str;
      kt_ctr_str = *(reinterpret_cast <key_ctr*>(key_str));
      pfc_log_debug("Added controller key to session");
      err = sess.addOutput(kt_ctr_str);
      break;
    }
    case UNC_KT_BOUNDARY: {
      key_boundary_t kt_bdry_str;
      kt_bdry_str = *(reinterpret_cast <key_boundary*>(key_str));
      pfc_log_debug("Added boundary key to session");
      err = sess.addOutput(kt_bdry_str);
      break;
    }
    default :
      pfc_log_error("Unsupported key type");
      err = pfc_ipcclnt_altclose(connp);
  }
  if (err != 0) {
    pfc_log_error("Server session addOutput failed while adding keystructure");
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  uint32_t oper_type;
  uint32_t result_code = UPLL_RC_SUCCESS;
  pfc_ipcresp_t resp;
  err = sess.invoke(resp);
  if (err != 0 || resp != UNC_RC_SUCCESS) {
    pfc_log_error("Session invocation to logical failed");
    err = pfc_ipcclnt_altclose(connp);
    if (err != 0) {
      pfc_log_error("Unable to close ipc connection");
    }
    return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
  }
  if (sess.getResponseCount() != UPLL_RESPONSE_COUNT) {
    pfc_log_error("Proper response not received from logical");
    err = pfc_ipcclnt_altclose(connp);
    if (err != 0) {
      pfc_log_error("Unable to close ipc connection");
    }
    return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
  }
  uint32_t resp_index = 0;
  int err1 = sess.getResponse(resp_index++, oper_type);
  int err2 = sess.getResponse(resp_index++, result_code);
  int err3 = sess.getResponse(resp_index, in_use);
  if (err1 != 0 || err2 != 0 || err3 != 0) {
    pfc_log_info("err1: %d, err2: %d, err3 %d, result_code %d",
                err1, err2, err3, result_code);
    pfc_log_error(
        "getResponse failed while receiving response from logical");
    return_code = UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
  } else {
    if (result_code == UPLL_RC_ERR_NO_SUCH_NAME ||
        result_code == UPLL_RC_ERR_NO_SUCH_DATATYPE ||
        result_code == UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT) {
      in_use = 0;
    } else if (result_code != UPLL_RC_SUCCESS) {
      in_use = 1;
    }
  }
  err = pfc_ipcclnt_altclose(connp);
  if (err != 0) {
    pfc_log_error("Unable to close ipc connection");
  }
  pfc_log_info("in_use value:%d, result_code=%d, return_code=%d"
      , in_use, result_code, return_code);
  if (return_code == UNC_RC_SUCCESS && in_use == 1)
    return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
  return return_code;
}

/**get_ipc_client_logical_handler
 * @Description : This function will be called from IPCConnectionManager
 *                to get the IPCClientLogicalHandler object.
 * @param[in]   : None
 * @return      : Returns class pointer to IPCClientLogicalHandler
 * */
IPCClientLogicalHandler* IPCClientLogicalHandler::
get_ipc_client_logical_handler() {
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  physical_layer->ipc_client_logical_hdlr_mutex_.lock();
  if (ipc_client_logical_handler_ == NULL) {
    ipc_client_logical_handler_= new IPCClientLogicalHandler();
  }
  physical_layer->ipc_client_logical_hdlr_mutex_.unlock();
  return ipc_client_logical_handler_;
}


/**release_ipc_client_logical_handler
 * @Description : This function will be called from IPCConnectionManager
                  to release the IPCClientLogicalHandler object.
 * @param[in]   : None
 * @return      : void
 **/
void IPCClientLogicalHandler::
release_ipc_client_logical_handler() {
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  physical_layer->ipc_client_logical_hdlr_mutex_.lock();
  if (ipc_client_logical_handler_ != NULL) {
    delete ipc_client_logical_handler_;
    ipc_client_logical_handler_ = NULL;
  }
  physical_layer->ipc_client_logical_hdlr_mutex_.unlock();
}

