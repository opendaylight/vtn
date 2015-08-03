/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *@brief   IPC Server Handler
 *@file    ipc_server_handler.cc
 *@ Desc:  This file contains the definition of IPCServerHandler class
 **
 **/


#include<pfc/debug.h>
#include <pfc/log.h>
#include "ipc_server_handler.hh"
#include "physicallayer.hh"

using unc::uppl::IPCServerHandler;

IPCServerHandler* IPCServerHandler::ipc_server_handler_ = NULL;

/**
 * @Description : This function will be called by PhysicalCore when IPC client
 *                sends a message to be received by UPPL's IPC server.
 *                This function validates whether the provided service id
 *                is valid. It sends the request to Internal Transaction
 *                Coordinator for further processing
 * @param[in]   : session    - Object of ServerSession where the request
 *                argument present
 *                service_id - service id to classify the type of request.
 *                UPPL expects 1, 2 or 3.
 * @return      : Response code back to the caller.
 *   The system/generic level errors or common errors are generally
 *   returned in this function otherwise SUCCESS(0) will be returned.
 *   When an error code is being returned in this function, the caller
 *   cannot expect more specific error in response.
 *   When SUCCESS(0) is returned, the caller should further check
 *   the operation result_code in the response for more specific error.
 **/

pfc_ipcresp_t IPCServerHandler::IpcService(ServerSession &session,
                                           pfc_ipcid_t service_id) {
  if (service_id > 2) {
    pfc_log_error("Fatal Error:Invalid service id");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  } else {
    PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
    uint32_t err = physical_layer->get_physical_core()->
        get_internal_transaction_coordinator()->
        ProcessReq(session, service_id);
    pfc_log_debug("Returning from IpcService of IPCServerHandler with %d", err);
    return err;
  }
}


/**
 * @Description :This function is used to send events via IPC post() command.
 *               This function will be called from ITC to send events to UPLL
 *               or VTN
 * @param[in]   : ServerEvent object
 * @return      : 0 is returned if event is posted successfully
 *                otherwise 'non 0' is returned to denote error
 **/

uint32_t IPCServerHandler::SendEvent(ServerEvent *serv_event) {
  pfc_timespec_t time_out;
  time_out.tv_sec = 300;
  time_out.tv_nsec = 0;
  uint32_t resp = serv_event->setTimeout(&time_out);
  if (resp != 0) {
    pfc_log_info("setTimeout failed in SendEvent");
  }

  resp = serv_event->post();
  if (resp != 0)
    pfc_log_error("PostEvent failed with resp:%d", resp);
  return resp;
}

/**
 * @Description :This function is called by IPCConnectionManager to
 *               get the IPCServerHandler object
 * @param[in]   :none
 * @return      :Pointer to IPCServerHandler
 * **/
IPCServerHandler* IPCServerHandler::get_ipc_server_handler() {
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  physical_layer->ipc_server_hdlr_mutex_.lock();
  if (ipc_server_handler_ == NULL) {
    ipc_server_handler_ = new IPCServerHandler();
  }
  physical_layer->ipc_server_hdlr_mutex_.unlock();
  return ipc_server_handler_;
}

/**
 * @Description :This function is used to delete the IPCServerHandler object
 * @param[in]   :none
 * @return      :none
 * **/
void IPCServerHandler::release_ipc_server_handler() {
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  physical_layer->ipc_server_hdlr_mutex_.lock();
  if (ipc_server_handler_ != NULL) {
    delete ipc_server_handler_;
    ipc_server_handler_ = NULL;
  }
  physical_layer->ipc_server_hdlr_mutex_.unlock();
}
