/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 @brief   IPCClientConfigurationHandler
 @file    ipc_client_configuration_handler.cc
 @ Desc:  This header file contains the definition of
  IPCClientConfigurationHandler class
 *
 */


#include "ipc_client_configuration_handler.hh"
#include "physicallayer.hh"
#include "physical_common_def.hh"
#include "unc/pfcdriver_include.h"
#include "unc/vnpdriver_include.h"
#include "unc/polcdriver_include.h"
#include "unc/odcdriver_include.h"
#include "unc/unc_base.h"

using unc::uppl::IPCClientDriverHandler;


/**
 *@Description:This function is called to send the request to Driver and
This return response. This function is called by all
Ktclasses,Audit,Import and ITC class.
 *@param[in]  :ClientSession object
 *@return     :PFC_TRUE or PFC_FALSE;
 **/


IPCClientDriverHandler::IPCClientDriverHandler(
    unc_keytype_ctrtype_t cntr_type, UncRespCode &err) {
  if (cntr_type == UNC_CT_PFC ||
      cntr_type == UNC_CT_VNP ||
      cntr_type == UNC_CT_POLC ||
      cntr_type == UNC_CT_ODC) {
    controller_type = cntr_type;
    PhysicalCore* physical_core = PhysicalCore::get_physical_core();
    /* Getting the driver name from uppl.conf */
    UncRespCode driver_name_status = physical_core->GetDriverName(
        controller_type, driver_name);
    if (driver_name_status != UNC_RC_SUCCESS) {
      pfc_log_error("Unable to get the driver name for controller type %d",
                    cntr_type);
      err = UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_debug("Creating a session to driver %s", driver_name.c_str());
    connp = 0;
    chn_name = PFCDRIVER_IPC_CHN_NAME;
    if (cntr_type == UNC_CT_VNP) {
      chn_name = VNPDRIVER_IPC_CHN_NAME;
    } else if (cntr_type == UNC_CT_POLC) {
      chn_name = POLCDRIVER_IPC_CHN_NAME;
    } else if (cntr_type == UNC_CT_ODC) {
      chn_name = ODCDRIVER_IPC_CHN_NAME;
    }
    int clnt_err = pfc_ipcclnt_altopen(chn_name.c_str(), &connp);
    if (clnt_err != 0) {
      pfc_log_error("Could not open driver ipc session");
      err = UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
    }
    pfc_ipcid_t service = PFCDRIVER_SVID_PHYSICAL;
    if (cntr_type == UNC_CT_VNP) {
      service = VNPDRV_SVID_PHYSICAL;
    } else if (cntr_type == UNC_CT_POLC) {
      service = POLCDRV_SVID_PHYSICAL;
    } else if (cntr_type == UNC_CT_ODC) {
      service = ODCDRV_SVID_PLATFORM;
    }
    cli_session = new ClientSession(connp, driver_name, service, clnt_err);
    if (cli_session == NULL) {
      pfc_log_error("Could not get driver ipc session");
      err = UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
    } else if (clnt_err != 0) {
      err = ConvertDriverErrorCode(clnt_err);
      pfc_log_error("Could not get driver ipc session, error is %d", err);
    }
  } else {
    // Default case
    cli_session = NULL;
    connp = 0;
    controller_type = UNC_CT_UNKNOWN;
    driver_name = "";
    chn_name =  "";
    err = UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
}

IPCClientDriverHandler::~IPCClientDriverHandler() {
  int err = pfc_ipcclnt_altclose(connp);
  if (err != 0) {
    pfc_log_info("Unable to close the connection");
  }
  cli_session->cancel(PFC_TRUE);
  delete cli_session;
}

ClientSession* IPCClientDriverHandler::ResetAndGetSession() {
  pfc_ipcid_t service = PFCDRIVER_SVID_PHYSICAL;
  if (controller_type == UNC_CT_VNP) {
    service = VNPDRV_SVID_PHYSICAL;
  } else if (controller_type == UNC_CT_POLC) {
    service = POLCDRV_SVID_PHYSICAL;
  } else if (controller_type == UNC_CT_ODC) {
    service = ODCDRV_SVID_PLATFORM;
  }
  cli_session->reset(driver_name, service);
  return cli_session;
}

UncRespCode IPCClientDriverHandler::SendReqAndGetResp(
    driver_response_header &rsp) {
  pfc_ipcresp_t resp = 0;
  uint8_t err = cli_session->invoke(resp);
  pfc_log_debug("DriverHandler err = %d, resp = %d",
                err, resp);
  if (err != 0 || resp != 0) {
    if (err == ECONNREFUSED) {
      pfc_log_debug("DriverHandler err = %d, driver not present", err);
      return UNC_RC_ERR_DRIVER_NOT_PRESENT;
    }
    return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
  } else {
    err = PhyUtil::sessGetDriverRespHeader(*cli_session, rsp);
    pfc_log_debug("DriverHandler resp err = %d, rsp.result_code=%d",
                  err, rsp.result_code);
    if (err != 0) {
      return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
    } else {
      return ConvertDriverErrorCode(rsp.result_code);
    }
  }
  return UNC_RC_SUCCESS;
}

UncRespCode IPCClientDriverHandler::ConvertDriverErrorCode(
    uint32_t drv_err_code) {
  switch (controller_type) {
    case UNC_CT_PFC:
    case UNC_CT_VNP:
    case UNC_CT_POLC:
    case UNC_CT_ODC:
      switch (drv_err_code) {
        case UNC_RC_SUCCESS:
          return UNC_RC_SUCCESS;
        case UNC_RC_CTRLAPI_FAILURE:
          return UNC_RC_CTRLAPI_FAILURE;
        case UNC_DRV_RC_DAEMON_INACTIVE:
          return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
        case UNC_DRV_RC_INVALID_REQUEST_FORMAT:
        case UNC_DRV_RC_INVALID_SESSION_ID:
        case UNC_DRV_RC_INVALID_CONFIG_ID:
        case UNC_DRV_RC_INVALID_OPERATION:
        case UNC_DRV_RC_INVALID_OPTION1:
        case UNC_DRV_RC_INVALID_OPTION2:
        case UNC_DRV_RC_INVALID_DATATYPE:
        case UNC_DRV_RC_INVALID_KEYTYPE:
        case UNC_DRV_RC_MISSING_KEY_STRUCT:
        case UNC_DRV_RC_MISSING_VAL_STRUCT:
        case UNC_RC_CONFIG_INVAL:
          return UNC_UPPL_RC_ERR_INVALID_STATE;
        case UNC_DRV_RC_ERR_ATTRIBUTE_SYNTAX:
          return UNC_UPPL_RC_ERR_CFG_SYNTAX;
        case UNC_RC_CTR_CONFIG_STATUS_ERR:
        case UNC_DRV_RC_ERR_ATTRIBUTE_SEMANTIC:
          return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
        case UNC_RC_CTR_DISCONNECTED:
        case UNC_RC_REQ_NOT_SENT_TO_CTR:
          return UNC_UPPL_RC_ERR_CTRLR_DISCONNECTED;
        case UNC_RC_CTR_BUSY:
          return UNC_RC_CTR_BUSY;
        case UNC_RC_NO_SUCH_INSTANCE:
          return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
        case UNC_RC_INTERNAL_ERR:
          return UNC_UPPL_RC_ERR_INVALID_STATE;
        case UNC_RC_UNSUPPORTED_CTRL_CONFIG:
        case UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR:
        case UNC_DRV_RC_ERR_GENERIC:
          return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
        default:
          return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
      }
      default:
        return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
  }
}

