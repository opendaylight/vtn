/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *@brief   IPCServerHandler header
 *@file    ipc_server_handler.hh
 *@ Desc:  This header file contains the declaration of IPCServerHandler class
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
 *                sends a message to is received for this server.This function
 *                validates whether the provided service id is valid
 * @param[in]   : ServerSession object, service id
 * @return      : PFC_TRUE or PFC_FALSE
 **/

pfc_ipcresp_t IPCServerHandler::IpcService(ServerSession &session,
                                           pfc_ipcid_t service_id) {
  if (service_id > 2) {
    pfc_log_error("Fatal Error:Invalid service id");
    return UPPL_RC_ERR_BAD_REQUEST;
  } else {
    PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
    uint32_t err = physical_layer->get_physical_core()->
        get_internal_transaction_coordinator()->
        ProcessReq(session, service_id);
    pfc_log_info("Returning from IpcService of IPCServerHandler with %d", err);
    return err;
  }
}


/**
 * @Description :This function is used to send events via IPC post() command
 *               This function will be called from ITC to send events to UPLL
 *               or VTN
 * @param[in]   : ServerEvent object
 * @return      : Success code or any associated error code
 **/

uint32_t IPCServerHandler::SendEvent(ServerEvent *serv_event) {
  uint32_t resp = serv_event->post();
  pfc_log_info("Post Event");
  return resp;
}

/**
 * @Description :This function is called by IPCConnectionMAnager to
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

void IPCServerHandler::release_ipc_server_handler() {
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  physical_layer->ipc_server_hdlr_mutex_.lock();
  if (ipc_server_handler_ != NULL) {
    delete ipc_server_handler_;
    ipc_server_handler_ = NULL;
  }
  physical_layer->ipc_server_hdlr_mutex_.unlock();
}
