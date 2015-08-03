/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "kt_util.hh"
#include "ipc_client_handler.hh"
#include "ipc_util.hh"
#include "unc/odcdriver_include.h"


using unc::upll::upll_util::upll_strncpy;

namespace unc {
namespace upll {
namespace ipc_util {

bool IpcClientHandler::SendReqToDriver(const char *ctrlr_name, char *domain_id,
                                       IpcRequest *req ) {
  UPLL_FUNC_TRACE

  unc_keytype_ctrtype_t ctrlr_type = UNC_CT_UNKNOWN;
  const char *channel_name = NULL;
  const char *service_name = NULL;
  pfc_ipcid_t service_id;

  if (unc::upll::config_momgr::CtrlrMgr::GetInstance()->GetCtrlrType(
      ctrlr_name, req->header.datatype, &ctrlr_type) == false) {
        UPLL_LOG_WARN("Unable to get controller type for %s", ctrlr_name);
    ipc_resp.header.result_code = UPLL_RC_ERR_GENERIC;
    return false;
  }
  // Currently supported controllers for Dataflow are PFC and ODC
  switch (ctrlr_type) {
    case UNC_CT_PFC:
      channel_name = PFCDRIVER_CHANNEL_NAME;
      service_name = PFCDRIVER_SERVICE_NAME;
      service_id = PFCDRIVER_SVID_LOGICAL;
      break;
    case UNC_CT_ODC:
      channel_name = ODCDRIVER_CHANNEL_NAME;
      service_name = ODCDRIVER_SERVICE_NAME;
      service_id = ODCDRV_SVID_PLATFORM;
      break;
    default:
      UPLL_LOG_WARN("Unknown controller type %d", ctrlr_type);
      ipc_resp.header.result_code = UPLL_RC_ERR_GENERIC;
      return false;
  }

  bool ok = SendReqToServer(channel_name, service_name, service_id,
                            true, ctrlr_name, domain_id, req);
  if (ok) {
    ipc_resp.header.result_code = IpcUtil::DriverResultCodeToKtURC(
        req->header.operation , ipc_resp.header.result_code);
  }
  return ok;
}



bool IpcClientHandler::SendReqToServer(const char *channel_name,
                              const char *service_name,
                              pfc_ipcid_t service_id,
                              bool driver_msg,
                              const char *ctrlr_name, char *domain_id,
                              IpcRequest *req) {
  UPLL_FUNC_TRACE
  IpcResponse *resp = &ipc_resp;

  if (channel_name == NULL || service_name == NULL ||
      req == NULL || resp == NULL) {
    UPLL_LOG_DEBUG("NULL argument");
    if (resp != NULL) {
      resp->header.result_code = UPLL_RC_ERR_GENERIC;
      resp->return_code = 0;
    }
    return false;
  }
  if (driver_msg == true && ctrlr_name == NULL) {
    UPLL_LOG_DEBUG("NULL controller argument");
    resp->header.result_code = UPLL_RC_ERR_GENERIC;
    resp->return_code = 0;
    return false;
  }

  if (IpcUtil::IsShuttingDown()) {
    UPLL_LOG_WARN("Cannot send request to %s as shutdown in progress",
                  channel_name);
    resp->header.result_code = UPLL_RC_ERR_SHUTTING_DOWN;
    resp->return_code = 0;
    return false;
  }

  UPLL_LOG_TRACE("dest=%s:%s:%d, controller_name=%s domain_id=%s\n"
                 "Request: %s\n%s",
                 channel_name, service_name, service_id,
                 ctrlr_name, domain_id,
                 IpcUtil::IpcRequestToStr(req->header).c_str(),
                 req->ckv_data->ToStrAll().c_str());

  // Create an alternative IPC connection handle.
  int err = pfc_ipcclnt_altopen(channel_name, &connid);
  if (err != 0) {
    UPLL_LOG_DEBUG("Failed to create IPC alternative connection to %s. Err=%d",
                  channel_name, err);
    resp->header.result_code = UPLL_RC_ERR_GENERIC;
    resp->return_code = PFC_IPCRESP_FATAL;
    return false;
  }

  cl_sess = new pfc::core::ipc::ClientSession(connid, service_name,
                                              service_id, err);
  if (err != 0) {
    UPLL_LOG_DEBUG("Failed to create IPC client session %s:%s:%d. Err=%d",
                  channel_name, service_name, service_id, err);
    err = pfc_ipcclnt_altclose(connid);  // Close the IPC connection handle
    if (err != 0) {
      UPLL_LOG_DEBUG("Failed to close the IPC connection %s:%s:%d. Err=%d",
                    channel_name, service_name, service_id, err);
    }
    resp->header.result_code = UPLL_RC_ERR_GENERIC;
    resp->return_code = PFC_IPCRESP_FATAL;
    return false;
  }
  if ((req->ckv_data != NULL) &&
      (req->ckv_data->get_key_type() == UNC_KT_VTN_DATAFLOW)) {
    pfc_timespec_t sess_timeout;
    sess_timeout.tv_sec  = kIpcTimeoutDataflow;
    sess_timeout.tv_nsec = 0;
    cl_sess->setTimeout(&sess_timeout);
    UPLL_LOG_TRACE("IPC Client Session timeout for channel %s set to %d secs",
                   channel_name, kIpcTimeoutDataflow);
  }
  bool ret = IpcUtil::WriteKtRequest(cl_sess, driver_msg, ctrlr_name, domain_id,
                            req->header, req->ckv_data);
  if (!ret) {
    UPLL_LOG_DEBUG("Failed to send IPC request to %s:%s:%d",
                  channel_name, service_name, service_id);
    err = pfc_ipcclnt_altclose(connid);  // Close the IPC connection handle
    if (err != 0) {
      UPLL_LOG_DEBUG("Failed to close the IPC connection %s:%s:%d. Err=%d",
                    channel_name, service_name, service_id, err);
    }
    resp->header.result_code = UPLL_RC_ERR_GENERIC;
    resp->return_code = PFC_IPCRESP_FATAL;
    return false;
  }
  pfc_ipcresp_t ipcresp;
  err = cl_sess->invoke(ipcresp);
  if (err != 0) {
    resp->header.result_code = UPLL_RC_ERR_GENERIC;
    if (err == ETIMEDOUT) {
      UPLL_LOG_INFO("IPC Session to %s:%s:%d has timed out",
                    channel_name, service_name, service_id);
      resp->return_code = PFC_IPCRESP_FATAL;
    } else if ((err == ECONNREFUSED) && (driver_msg)) {
      UPLL_LOG_INFO("Connection to IPC Session %s:%s:%d is refused",
                    channel_name, service_name, service_id);
      resp->return_code = PFC_IPCRESP_FATAL;
      resp->header.result_code = UPLL_RC_ERR_DRIVER_NOT_PRESENT;
    } else {
      UPLL_LOG_FATAL("Failed to send IPC request to %s:%s:%d. Err=%d",
                     channel_name, service_name, service_id, err);
      resp->return_code = PFC_IPCRESP_FATAL;
    }
    err = pfc_ipcclnt_altclose(connid);  // Close the IPC connection handle
    if (err != 0) {
      UPLL_LOG_DEBUG("Failed to close the IPC connection %s:%s:%d. Err=%d",
                    channel_name, service_name, service_id, err);
    }
    return false;
  }
  if (ipcresp != 0) {
    UPLL_LOG_DEBUG("Error at IPC server %s:%s:%d. ErrResp=%d",
                  channel_name, service_name, service_id, ipcresp);
    err = pfc_ipcclnt_altclose(connid);  // Close the IPC connection handle
    if (err != 0) {
      UPLL_LOG_DEBUG("Failed to close the IPC connection %s:%s:%d. Err=%d",
                    channel_name, service_name, service_id, err);
    }
    resp->header.result_code = UPLL_RC_ERR_GENERIC;
    resp->return_code = PFC_IPCRESP_FATAL;
    return false;
  }

  ret = ReadKtResponse(cl_sess, service_id, driver_msg, domain_id);
  if (!ret) {
    UPLL_LOG_DEBUG("Failed to read IPC response from %s:%s:%d",
                  channel_name, service_name, service_id);
    err = pfc_ipcclnt_altclose(connid);  // Close the IPC connection handle
    if (err != 0) {
      UPLL_LOG_DEBUG("Failed to close the IPC connection %s:%s:%d. Err=%d",
                    channel_name, service_name, service_id, err);
    }
    resp->header.result_code = UPLL_RC_ERR_GENERIC;
    resp->return_code = PFC_IPCRESP_FATAL;
    return false;
  }
  resp->return_code = 0;

  UPLL_LOG_TRACE("dest=%s:%s:%d, controller_name=%s domain_id=%s\n"
                 "Response: %s\n",
                 channel_name, service_name, service_id,
                 ctrlr_name, domain_id,
                 IpcUtil::IpcResponseToStr(resp->header).c_str());
  return true;
}


/* Read KT response from Server */
/* if driver_msg is true, then the message is from driver, otherwise it is from
 * Physical. If the driver_msg is true and non-null domain_id pointer is passed
 * then the domain_id from the message is placed in the domain_id pointer.
 * Caller should have sent a valid pointer for copying domain id */
bool IpcClientHandler::ReadKtResponse(pfc::core::ipc::ClientSession *sess,
                             pfc_ipcid_t /* service */,
                             bool driver_msg, char *domain_id) {
  UPLL_FUNC_TRACE;
  if (sess == NULL) {
    UPLL_LOG_DEBUG("Null argument");
    return false;
  }
  IpcReqRespHeader *msg_hdr = &ipc_resp.header;
  ConfigKeyVal **first_ckv =  &ipc_resp.ckv_data;
  *first_ckv = NULL;

  bzero(msg_hdr, sizeof(*msg_hdr));

  uint32_t arg_cnt = sess->getResponseCount();
  uint32_t mandatory_fields = (driver_msg) ?
            unc::upll::ipc_util::kKeyTreeDriverRespMandatoryFields :
      unc::upll::ipc_util::kKeyTreeRespMandatoryFields;
  if (arg_cnt < mandatory_fields) {
    UPLL_LOG_DEBUG("Not enough arguments in key tree response."
                   " Has only %u, expected %d",
                  arg_cnt, mandatory_fields);
    return false;
  }

  int err;
  uint32_t keytype;
  uint32_t operation;
  const char *msg_ctrlr_name;
  const char *msg_domain_id;
  uint32_t option1;
  uint32_t option2;
  uint32_t datatype;
  uint32_t result_code;
  arg = 0;
  if ((0 != (err = sess->getResponse(arg++, msg_hdr->clnt_sess_id))) ||
      (0 != (err = sess->getResponse(arg++, msg_hdr->config_id)))) {
    UPLL_LOG_DEBUG("Failed to get header field #%u in the key tree response."
                  " Err=%d", arg, err);
    UPLL_LOG_TRACE("Config and Sess id is not able to get");
    return false;
  }
  if (driver_msg == true) {
    if ((0 != (err = sess->getResponse(arg++, msg_ctrlr_name))) ||
        (0 != (err = sess->getResponse(arg++, msg_domain_id)))) {
      UPLL_LOG_DEBUG("Failed to get header field #%u in the key tree response."
          " Err=%d", arg, err);
      UPLL_LOG_TRACE("Ctrlr and domain name not able to get");
      return false;
    }
    if (domain_id != NULL) {
      upll_strncpy(domain_id, msg_domain_id, KtUtil::kCtrlrNameLenWith0);
    }
  }
  if ((0 != (err = sess->getResponse(arg++, operation))) ||
      (0 != (err = sess->getResponse(arg++, msg_hdr->rep_count))) ||
      (0 != (err = sess->getResponse(arg++, option1))) ||
      (0 != (err = sess->getResponse(arg++, option2))) ||
      (0 != (err = sess->getResponse(arg++, datatype))) ||
      (0 != (err = sess->getResponse(arg++, result_code))) ||
      (0 != (err = sess->getResponse(arg++, keytype))) ) {
    UPLL_LOG_DEBUG("Failed to get header field #%u in the key tree response."
                  " Err=%d", arg, err);
    return false;
  }
  arg++; /* Key struct skkipped */
  msg_hdr->operation = (unc_keytype_operation_t)operation;
  msg_hdr->option1 = (unc_keytype_option1_t)option1;
  msg_hdr->option2 = (unc_keytype_option2_t)option2;
  msg_hdr->datatype = (upll_keytype_datatype_t)datatype;
  msg_hdr->result_code = (upll_rc_t) result_code;

#if 0
  IpctSt::IpcStructNum st_num;
  void *ipc_st;
  if (!IpcUtil::ReadIpcStruct(sess, arg++, &st_num, &ipc_st)) {
    UPLL_LOG_DEBUG("Failed to get structure at %u in the key tree response",
                  arg);
    return false;
  }
  *first_ckv = new ConfigKeyVal((unc_key_type_t)keytype, st_num, ipc_st, NULL);
#endif
  return true;
}

}  // namespace ipc_util
}  // namespace upll
}  // namespace unc
