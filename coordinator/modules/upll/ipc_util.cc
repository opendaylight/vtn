/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <sstream>

#include "pfc/log.h"
#include "ipct_st.hh"
#include "unc/uppl_common.h"
#include "unc/upll_svc.h"

#include "unc/pfcdriver_include.h"
#include "unc/vnpdriver_include.h"
#include "unc/polcdriver_include.h"
#include "unc/odcdriver_include.h"
#include "unc/unc_base.h"

#include "ctrlr_mgr.hh"
#include "uncxx/upll_log.hh"
#include "upll_util.hh"
#include "kt_util.hh"
#include "ipc_util.hh"

namespace unc {
namespace upll {
namespace ipc_util {

using unc::upll::upll_util::upll_strncpy;

bool IpcUtil::shutting_down_ = false;
pfc::core::ReadWriteLock IpcUtil::sys_state_rwlock_;

std::list<ConfigNotification*> ConfigNotifier::buffered_notifs;
pfc::core::Mutex ConfigNotifier::notif_lock;

void ConfigVal::AppendCfgVal(ConfigVal *cfg_val) {
  ConfigVal *prev_val = this;
  while (prev_val->next_cfg_val_) {
    prev_val = prev_val->next_cfg_val_;
  }
  prev_val->next_cfg_val_ = cfg_val;
}

std::string ConfigVal::ToStr() const {
  std::stringstream ss;
  ss  << "  -------------ConfigVal--------------" << std::endl
      << "    st_num:     " << st_num_ << std::endl
      << "    val:\n" << KtUtil::KtStructToStr(st_num_, val_);
  return ss.str();
}

std::string ConfigVal::ToStrAll() const {
  std::stringstream ss;
  int n = 0;
  ss << "  CV" << ++n << ToStr();
  for (ConfigVal *ncv = next_cfg_val_; ncv; ncv = ncv->next_cfg_val_) {
    ss << std::endl << "  CV" << ++n << ncv->ToStr();
  }
  return ss.str();
}

void ConfigKeyVal::AppendCfgKeyVal(ConfigKeyVal *cfg_kv) {
  ConfigKeyVal *prev_kv = this;
  while (prev_kv->next_ckv_) {
    prev_kv = prev_kv->next_ckv_;
  }
  prev_kv->next_ckv_ = cfg_kv;
}


ConfigKeyVal *ConfigKeyVal::FindNext(unc_key_type_t keytype) {
  for (ConfigKeyVal *ckv = next_ckv_; ckv != NULL; ckv = ckv->next_ckv_) {
    if (ckv->get_key_type() == keytype)
      return ckv;
  }
  return NULL;
}

// Note: shallow Dup
ConfigVal *ConfigVal::DupVal() const {
  ConfigVal *dup = new ConfigVal(st_num_, NULL);
  if (val_ != NULL) {
    const pfc_ipcstdef_t *st_def = IpctSt::GetIpcStdef(st_num_);
    if (st_def == NULL) {
      UPLL_LOG_DEBUG("Unknown structure %d ", st_num_);
      delete dup;
      return NULL;
    }
    dup->val_ = ConfigKeyVal::Malloc(st_def->ist_size);
    memcpy(dup->val_, val_, st_def->ist_size);
  }
  return dup;
}

// Note: shallow Dup
ConfigKeyVal *ConfigKeyVal::DupKey() const {
  return DupKeyVal(true, false);
}

// Note: shallow Dup
ConfigKeyVal *ConfigKeyVal::DupKeyVal() const {
  return DupKeyVal(true, true);
}

// Note: shallow Dup
ConfigKeyVal *ConfigKeyVal::DupKeyVal(bool dup_key, bool dup_val) const {
  ConfigKeyVal *dup = new ConfigKeyVal(key_type_, st_num_, NULL, NULL);
  if ((key_ != NULL) && (dup_key == true)) {
    const pfc_ipcstdef_t *st_def = IpctSt::GetIpcStdef(st_num_);
    if (st_def == NULL) {
      UPLL_LOG_DEBUG("Unknown structure %d ", st_num_);
      delete dup;
      return NULL;
    }
    dup->key_ = ConfigKeyVal::Malloc(st_def->ist_size);
    memcpy(dup->key_, key_, st_def->ist_size);
  }
  if ((cfg_val_ != NULL) && (dup_val == true)) {
    dup->cfg_val_ = cfg_val_->DupVal();
  }
  return dup;
}


std::string ConfigKeyVal::ToStr() const {
  std::stringstream ss;
  ss  << "-------------ConfigKeyVal--------------" << std::endl
      << "  key_type:   " << key_type_ << std::endl
      << "  st_num:     " << st_num_ << std::endl
      << "  key:\n" << KtUtil::KtStructToStr(st_num_, key_);
  if (cfg_val_) {
    ss << std::endl << cfg_val_->ToStrAll();
  }
  return ss.str();
}

std::string ConfigKeyVal::ToStrAll() const {
  std::stringstream ss;
  int n = 0;
  ss << "CKV" << ++n << ToStr();
  for (ConfigKeyVal *nckv = next_ckv_; nckv; nckv = nckv->next_ckv_) {
    ss << std::endl << "CKV" << ++n << nckv->ToStr();
  }
  return ss.str();
}

upll_rc_t IpcUtil::GetCtrlrTypeFromPhy(const char *ctrlr_name,
                                       upll_keytype_datatype_t datatype,
                                       unc_keytype_ctrtype_t *ctrlr_type) {
  if (ctrlr_type == NULL) {
    UPLL_LOG_DEBUG("Null argument ctrlr_type");
    return UPLL_RC_ERR_GENERIC;
  }
  PFC_ASSERT(strlen(ctrlr_name) < KtUtil::kCtrlrNameLenWith0);

  key_ctr_t ctr_key;
  upll_strncpy(ctr_key.controller_name, ctrlr_name, KtUtil::kCtrlrNameLenWith0);

  IpcRequest req;
  bzero(&req, sizeof(req));
  req.header.operation = UNC_OP_READ;
  req.header.datatype = datatype;
  req.ckv_data = new ConfigKeyVal(UNC_KT_CONTROLLER, IpctSt::kIpcStKeyCtr,
                                  &ctr_key);

  IpcResponse resp;
  bzero(&resp, sizeof(resp));

  if (SendReqToPhysical(UPPL_IPC_SVC_NAME, UPPL_SVC_READREQ, &req, &resp) ==
      true) {
    if (resp.header.result_code == UPLL_RC_SUCCESS) {
      if ((resp.ckv_data != NULL) &&
          (resp.ckv_data->get_cfg_val() != NULL) &&
          (resp.ckv_data->get_cfg_val()->get_val() != NULL)) {
        if (resp.ckv_data->get_cfg_val()->get_st_num() ==
            IpctSt::kIpcStValCtr) {
          val_ctr_t *ctr_val = reinterpret_cast<val_ctr_t*>(
              resp.ckv_data->get_cfg_val()->get_val());
          *ctrlr_type = (unc_keytype_ctrtype_t)(ctr_val->type);
          return UPLL_RC_SUCCESS;
        }
      }
    }
  }
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t IpcUtil::DriverResultCodeToKtURC(
    unc_keytype_operation_t operation, uint32_t driver_result_code) {
  switch (operation) {
    // Transaction specific operations
    case UNC_OP_CREATE:
    case UNC_OP_DELETE:
    case UNC_OP_UPDATE:
      {
        switch (driver_result_code) {
          case UNC_RC_SUCCESS:
            return UPLL_RC_SUCCESS;

            // case UNC_DRV_RC_DAEMON_INACTIVE: move to default case
            // It will not be sent to controller only in the case of
            // vote, and that too if previous vote failed. Since we do not send
            // VOTE request from here, let us convert it to GENERIC error.
            // case UNC_RC_REQ_NOT_SENT_TO_CTR: move to default case
            // case UNC_DRV_RC_INVALID_REQUEST_FORMAT: move to default case
            // case UNC_DRV_RC_INVALID_SESSION_ID: move to default case
            // case UNC_DRV_RC_INVALID_CONFIG_ID: move to default case
            // case UNC_DRV_RC_INVALID_OPERATION: move to default case
            // case UNC_DRV_RC_INVALID_OPTION1: move to default case
            // case UNC_DRV_RC_INVALID_OPTION2: move to default case
            // case UNC_DRV_RC_INVALID_DATATYPE: move to default case
            // case UNC_DRV_RC_INVALID_KEYTYPE: move to default case

          case UNC_DRV_RC_ERR_ATTRIBUTE_SYNTAX:
            return static_cast<upll_rc_t>(UNC_RC_CONFIG_INVAL);
          case UNC_DRV_RC_ERR_ATTRIBUTE_SEMANTIC:
            return static_cast<upll_rc_t>(UNC_RC_CONFIG_INVAL);

          case UNC_RC_CTR_DISCONNECTED:
            return UPLL_RC_ERR_CTR_DISCONNECTED;

          case UNC_RC_CONFIG_INVAL:
          case UNC_RC_CTR_CONFIG_STATUS_ERR:
          case UNC_RC_CTR_BUSY:
            return static_cast<upll_rc_t>(driver_result_code);

          case UNC_RC_NO_SUCH_INSTANCE:
            return UPLL_RC_ERR_NO_SUCH_INSTANCE;

            // case UNC_DRV_RC_MISSING_KEY_STRUCT: move to default case
            // case UNC_DRV_RC_MISSING_VAL_STRUCT: move to default case
            // case UNC_DRV_RC_ERR_GENERIC: move to default case
            // case UNC_RC_INTERNAL_ERR: move to default case
            // case UNC_RC_UNSUPPORTED_CTRL_CONFIG: only for READ operation

          case UNC_RC_CTRLAPI_FAILURE:
          case UNC_RC_ERR_DRIVER_NOT_PRESENT:
            return static_cast<upll_rc_t>(driver_result_code);
          default:
            UPLL_LOG_INFO("Received error %d from driver,"
                          " converting to GENERIC error",
                          driver_result_code);
            return UPLL_RC_ERR_GENERIC;
        }
      }
    default: // Not-transaction specific operations: READ, CONTROL operations
      {
        switch (driver_result_code) {
          case UNC_RC_SUCCESS:
            return UPLL_RC_SUCCESS;
            // case UNC_DRV_RC_DAEMON_INACTIVE: move to default case
            // It will not be sent to controller only in the case of
            // vote, and that too if previous vote failed. Since we do not send
            // VOTE request from here, let us convert it to GENERIC error.
            // case UNC_RC_REQ_NOT_SENT_TO_CTR: move to default case
            // case UNC_DRV_RC_INVALID_REQUEST_FORMAT: move to default case
            // case UNC_DRV_RC_INVALID_SESSION_ID: move to default case
            // case UNC_DRV_RC_INVALID_CONFIG_ID: move to default case
            // case UNC_DRV_RC_INVALID_OPERATION: move to default case
            // case UNC_DRV_RC_INVALID_OPTION1: move to default case
            // case UNC_DRV_RC_INVALID_OPTION2: move to default case
            // case UNC_DRV_RC_INVALID_DATATYPE: move to default case
            // case UNC_DRV_RC_INVALID_KEYTYPE: move to default case

          case UNC_DRV_RC_ERR_ATTRIBUTE_SYNTAX:
            return UPLL_RC_ERR_CFG_SYNTAX;
          case UNC_DRV_RC_ERR_ATTRIBUTE_SEMANTIC:
            return UPLL_RC_ERR_CFG_SEMANTIC;

          case UNC_RC_CTR_DISCONNECTED:
            return UPLL_RC_ERR_CTR_DISCONNECTED;

            // case UNC_RC_CONFIG_INVAL: move to default case
            // case UNC_RC_CTR_CONFIG_STATUS_ERR: move to default case
            // case UNC_RC_CTR_BUSY: move to default case
            // TODO: assumption driver does not give this error. Please Verify

          case UNC_RC_NO_SUCH_INSTANCE:
            return UPLL_RC_ERR_NO_SUCH_INSTANCE;

          case UNC_RC_UNSUPPORTED_CTRL_CONFIG:
          case UNC_RC_ERR_DRIVER_NOT_PRESENT:
            return static_cast<upll_rc_t>(driver_result_code);

            // case UNC_DRV_RC_MISSING_KEY_STRUCT: move to default case
            // case UNC_DRV_RC_MISSING_VAL_STRUCT: move to default case
            // case UNC_DRV_RC_ERR_GENERIC: move to default case
            // case UNC_RC_INTERNAL_ERR: move to default case
          case UNC_RC_CTRLAPI_FAILURE:
          // TODO: need to check with Driver on whether this is possible
          // for these operations. If so, what should it get converted in upll.
          // In U10 we were sending GENERIC error, so we can convert to default case
          default:
            UPLL_LOG_INFO("Received error %d from driver,"
                          " converting to GENERIC error",
                          driver_result_code);
            return UPLL_RC_ERR_GENERIC;
        }
      }
  }
}

// domain_id can be NULL
bool IpcUtil::SendReqToDriver(const char *ctrlr_name, char *domain_id,
                              const char * /* service_name*/,
                              pfc_ipcid_t /* service_id */,
                              IpcRequest *req, bool /* edit_conn */,
                              IpcResponse *resp) {
  UPLL_FUNC_TRACE
  PFC_ASSERT(ctrlr_name != NULL);

  unc_keytype_ctrtype_t ctrlr_type = UNC_CT_UNKNOWN;
  const char *channel_name = NULL;
  const char *service_name = NULL;
  pfc_ipcid_t service_id;

  if (unc::upll::config_momgr::CtrlrMgr::GetInstance()->GetCtrlrType(
      ctrlr_name, req->header.datatype, &ctrlr_type) == false) {
    if ((req->header.operation == UNC_OP_DELETE) &&
        (req->header.datatype != UPLL_DT_RUNNING)) {
      if (unc::upll::config_momgr::CtrlrMgr::GetInstance()->GetCtrlrType(
          ctrlr_name, UPLL_DT_RUNNING, &ctrlr_type) == false) {
        UPLL_LOG_WARN("Unable to get controller type for %s", ctrlr_name);
        resp->header.result_code = UPLL_RC_ERR_GENERIC;
        return false;
      }
    } else {
      UPLL_LOG_WARN("Unable to get controller type for %s", ctrlr_name);
      resp->header.result_code = UPLL_RC_ERR_GENERIC;
      return false;
    }
  }
  switch (ctrlr_type) {
    case UNC_CT_PFC:
      channel_name = PFCDRIVER_CHANNEL_NAME;
      service_name = PFCDRIVER_SERVICE_NAME;
      service_id = PFCDRIVER_SVID_LOGICAL;
      break;
    case UNC_CT_VNP:
      channel_name = VNPDRIVER_CHANNEL_NAME;
      service_name = VNPDRIVER_SERVICE_NAME;
      service_id = VNPDRV_SVID_LOGICAL;
      break;
    case UNC_CT_POLC:
      channel_name = POLCDRIVER_CHANNEL_NAME;
      service_name = POLCDRIVER_SERVICE_NAME;
      service_id = POLCDRV_SVID_LOGICAL;
      break;
    case UNC_CT_ODC:
      channel_name = ODCDRIVER_CHANNEL_NAME;
      service_name = ODCDRIVER_SERVICE_NAME;
      service_id = ODCDRV_SVID_PLATFORM;
      break;
    default:
      UPLL_LOG_WARN("Unknown controller type %d", ctrlr_type);
      resp->header.result_code = UPLL_RC_ERR_GENERIC;
      return false;
  }

  bool ok = SendReqToServer(channel_name, service_name, service_id,
                            true, ctrlr_name, domain_id, req, resp);
  if (ok) {
    resp->header.result_code = DriverResultCodeToKtURC(
        req->header.operation , resp->header.result_code);
  }
  return ok;
}

upll_rc_t IpcUtil::PhysicalResultCodeToKtURC(uint32_t result_code) {
  switch (result_code) {
    case UNC_RC_SUCCESS:
      return UPLL_RC_SUCCESS;
    case UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE:
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    default:
      return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_ERR_GENERIC;
}

bool IpcUtil::SendReqToPhysical(const char *service_name,
                                pfc_ipcid_t service_id,
                                IpcRequest *req,
                                IpcResponse *resp) {
  if (service_name == NULL) {
    service_name = UPPL_IPC_SVC_NAME;
  }
  bool ok = SendReqToServer(UPPL_IPC_CHN_NAME, service_name, service_id,
                            false, NULL, NULL, req, resp);
  if (ok) {
    // translate UPPL error code to UPLL error code
    resp->header.result_code = PhysicalResultCodeToKtURC(
        resp->header.result_code);
  }
  return ok;
}

bool IpcUtil::SendReqToServer(const char *channel_name,
                              const char *service_name,
                              pfc_ipcid_t service_id,
                              bool driver_msg,
                              const char *ctrlr_name, char *domain_id,
                              IpcRequest *req, IpcResponse *resp) {
  UPLL_FUNC_TRACE
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
    if (resp != NULL) {
      resp->header.result_code = UPLL_RC_ERR_GENERIC;
      resp->return_code = 0;
    }
    return false;
  }

  if (IsShuttingDown()) {
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
  pfc_ipcconn_t connid;
  int err = pfc_ipcclnt_altopen(channel_name, &connid);
  if (err != 0) {
    UPLL_LOG_DEBUG("Failed to create IPC alternative connection to %s. Err=%d",
                  channel_name, err);
    resp->header.result_code = UPLL_RC_ERR_GENERIC;
    resp->return_code = PFC_IPCRESP_FATAL;
    return false;
  }

  pfc::core::ipc::ClientSession cl_sess(connid, service_name, service_id, err);
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

  // Increasing IPC session timeout
  if (req->header.operation == UNC_OP_CONTROL &&
      req->header.option2 == UNC_OPT2_PING) {
    pfc_timespec_t sess_timeout;
    sess_timeout.tv_sec  = kIpcTimeoutPing;
    sess_timeout.tv_nsec = 0;
    cl_sess.setTimeout(&sess_timeout);
    UPLL_LOG_TRACE("IPC Client Session timeout for channel %s set to %d secs"
                   " for operation %d",
                   channel_name, kIpcTimeoutPing, req->header.operation);

  } else if ((driver_msg) && (req->header.operation == UNC_OP_READ_BULK)) {
    // Import takes lot of time
    cl_sess.setTimeout(NULL);
    UPLL_LOG_TRACE("IPC Client Session timeout for channel %s set to infinity"
                   " for operation %d",
                   channel_name, req->header.operation);
  } else if (req->ckv_data->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER) {
    pfc_timespec_t sess_timeout;
    sess_timeout.tv_sec  = kIpcTimeoutVtnstation;
    sess_timeout.tv_nsec = 0;
    cl_sess.setTimeout(&sess_timeout);
    UPLL_LOG_TRACE("IPC Client Session timeout for channel %s set to %d secs"
                   " for operation %d on VTN_STATION_CONTROLLER",
                   channel_name, kIpcTimeoutVtnstation, req->header.operation);
  }


  bool ret = WriteKtRequest(&cl_sess, driver_msg, ctrlr_name, domain_id,
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
  IpcResponse local_resp;
  pfc_ipcresp_t ipcresp;
  err = cl_sess.invoke(ipcresp);
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
      UPLL_LOG_INFO("Failed to close the IPC connection %s:%s:%d. Err=%d",
                    channel_name, service_name, service_id, err);
    }
    return false;
  }
  if (ipcresp != 0) {
    UPLL_LOG_INFO("Error at IPC server %s:%s:%d. ErrResp=%d",
                  channel_name, service_name, service_id, ipcresp);
    err = pfc_ipcclnt_altclose(connid);  // Close the IPC connection handle
    if (err != 0) {
      UPLL_LOG_INFO("Failed to close the IPC connection %s:%s:%d. Err=%d",
                    channel_name, service_name, service_id, err);
    }
    resp->header.result_code = UPLL_RC_ERR_GENERIC;
    resp->return_code = PFC_IPCRESP_FATAL;
    return false;
  }

  ret = ReadKtResponse(&cl_sess, service_id, driver_msg, domain_id,
                       &local_resp.header, &local_resp.ckv_data);
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
  PFC_ASSERT(local_resp.ckv_data != NULL);

  resp->header = local_resp.header;
  if (resp->ckv_data == NULL) {
    resp->ckv_data = new ConfigKeyVal(local_resp.ckv_data->get_key_type());
  }
  resp->ckv_data->ResetWith(local_resp.ckv_data);
  delete local_resp.ckv_data;
  resp->return_code = 0;

  /* Close the IPC connection handle. */
  err = pfc_ipcclnt_altclose(connid);
  if (err != 0) {
    UPLL_LOG_INFO("Failed to close the IPC connection %s:%s:%d. Err=%d",
                  channel_name, service_name, service_id, err);
  }

  UncRespCode unc_rc = (UncRespCode)resp->header.result_code;
  if (unc_rc != UNC_RC_SUCCESS &&
      unc_rc != UNC_RC_ERR_DRIVER_NOT_PRESENT &&
      unc_rc != UNC_RC_CTR_DISCONNECTED &&
      unc_rc != UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_INFO("dest=%s:%s:%d, controller_name=%s domain_id=%s\n"
                  "Response: %s\n%s",
                  channel_name, service_name, service_id,
                  ctrlr_name, domain_id,
                  IpcUtil::IpcResponseToStr(resp->header).c_str(),
                  resp->ckv_data->ToStrAll().c_str());
  } else {
    UPLL_LOG_TRACE("dest=%s:%s:%d, controller_name=%s domain_id=%s\n"
                   "Response: %s\n%s",
                   channel_name, service_name, service_id,
                   ctrlr_name, domain_id,
                   IpcUtil::IpcResponseToStr(resp->header).c_str(),
                   resp->ckv_data->ToStrAll().c_str());
  }
  return true;
}

#define READ_PRIMARY_IPCTYPE(                                                 \
    ipc_sess, sess_read_api, index, data_type, data_var_ptr)                  \
{                                                                             \
  ret = true;                                                                 \
  data_var_ptr = ConfigKeyVal::Malloc(sizeof(data_type));                     \
  int ipc_ret = ipc_sess->sess_read_api(                                      \
      index, *(reinterpret_cast<data_type*>(data_var_ptr)));                  \
  if (ipc_ret != 0) {                                                         \
    UPLL_LOG_DEBUG("Failed to read argument in the IPC request, Err=%d",      \
                   ipc_ret);                                                  \
    ret = false;                                                              \
  }                                                                           \
}

#define READ_PRIMARY_IPCTYPE_FROM_SERVER(                                     \
    ipc_srv_sess, index, data_type, data_var_ptr) {                           \
  READ_PRIMARY_IPCTYPE(ipc_srv_sess, getArgument,                             \
                       index, data_type, data_var_ptr) \
}

#define READ_PRIMARY_IPCTYPE_FROM_CLIENT(                                     \
    ipc_clnt_sess, index, data_type, data_var_ptr) {                          \
  READ_PRIMARY_IPCTYPE(ipc_clnt_sess, getResponse,                            \
                       index, data_type, data_var_ptr) \
}

#define WRITE_PRIMARY_IPCTYPE(                                                \
    ipc_sess, sess_write_api, data_type, data_var_ptr)                        \
{                                                                             \
  ret = true;                                                                 \
  int ipc_ret = ipc_sess->sess_write_api(                                     \
                                *((data_type*)(data_var_ptr)));  /* NOLINT */ \
  if (ipc_ret != 0) {                                                         \
    UPLL_LOG_DEBUG("Failed to read argument in the IPC request, Err=%d",      \
                   ipc_ret);                                                  \
    ret = false;                                                              \
  }                                                                           \
}


bool IpcUtil::ReadIpcArg(pfc::core::ipc::ServerSession *sess, uint32_t index,
                         IpctSt::IpcStructNum *st_num, void **ipc_struct) {
    pfc_ipctype_t arg_type;
    if (0 != sess->getArgType(index, arg_type)) {
      UPLL_LOG_DEBUG("Failed to get arg type in the IPC request");
      return false;
    }

    bool ret;

    switch (arg_type) {
      case PFC_IPCTYPE_INT8:
        *st_num = IpctSt::kIpcStInt8;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, int8_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_UINT8:
        *st_num = IpctSt::kIpcStUint8;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, uint8_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_INT16:
        *st_num = IpctSt::kIpcStInt16;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, int16_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_UINT16:
        *st_num = IpctSt::kIpcStUint16;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, uint16_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_INT32:
        *st_num = IpctSt::kIpcStInt32;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, int32_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_UINT32:
        *st_num = IpctSt::kIpcStUint32;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, uint32_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_INT64:
        *st_num = IpctSt::kIpcStInt64;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, int64_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_UINT64:
        *st_num = IpctSt::kIpcStUint64;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, uint64_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_FLOAT:
        *st_num = IpctSt::kIpcStFloat;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, float, *ipc_struct);
        break;
      case PFC_IPCTYPE_DOUBLE:
        *st_num = IpctSt::kIpcStDouble;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, double, *ipc_struct);
        break;
      case PFC_IPCTYPE_IPV4:
        *st_num = IpctSt::kIpcStIpv4;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, struct in_addr,
                                         *ipc_struct);
        break;
      case PFC_IPCTYPE_IPV6:
        *st_num = IpctSt::kIpcStIpv6;
        READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, struct in6_addr,
                                         *ipc_struct);
        break;
      case PFC_IPCTYPE_STRING:
        {
          *st_num = IpctSt::kIpcStString;
          READ_PRIMARY_IPCTYPE_FROM_SERVER(sess, index, const char *,
                                           *ipc_struct);
          char *str = strdup(*(reinterpret_cast<char **>(*ipc_struct)));
          ConfigKeyVal::Free(*ipc_struct);
          *ipc_struct = str;
        }
        break;
      case PFC_IPCTYPE_STRUCT:
        ret = ReadIpcStruct(sess, index, st_num, ipc_struct);
        break;
      case PFC_IPCTYPE_BINARY:            // NOT USED
      default:
        ret = false;
    }
    return ret;
}

bool IpcUtil::ReadIpcArg(pfc::core::ipc::ClientSession *sess, uint32_t index,
                         IpctSt::IpcStructNum *st_num, void **ipc_struct) {
    pfc_ipctype_t arg_type;
    if (0 != sess->getResponseType(index, arg_type)) {
      UPLL_LOG_DEBUG("Failed to get arg type in the IPC request");
      return false;
    }

    bool ret;

    switch (arg_type) {
      case PFC_IPCTYPE_INT8:
        *st_num = IpctSt::kIpcStInt8;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, int8_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_UINT8:
        *st_num = IpctSt::kIpcStUint8;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, uint8_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_INT16:
        *st_num = IpctSt::kIpcStInt16;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, int16_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_UINT16:
        *st_num = IpctSt::kIpcStUint16;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, uint16_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_INT32:
        *st_num = IpctSt::kIpcStInt32;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, int32_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_UINT32:
        *st_num = IpctSt::kIpcStUint32;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, uint32_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_INT64:
        *st_num = IpctSt::kIpcStInt64;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, int64_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_UINT64:
        *st_num = IpctSt::kIpcStUint64;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, uint64_t, *ipc_struct);
        break;
      case PFC_IPCTYPE_FLOAT:
        *st_num = IpctSt::kIpcStFloat;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, float, *ipc_struct);
        break;
      case PFC_IPCTYPE_DOUBLE:
        *st_num = IpctSt::kIpcStDouble;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, double, *ipc_struct);
        break;
      case PFC_IPCTYPE_IPV4:
        *st_num = IpctSt::kIpcStIpv4;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, struct in_addr,
                                         *ipc_struct);
        break;
      case PFC_IPCTYPE_IPV6:
        *st_num = IpctSt::kIpcStIpv6;
        READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, struct in6_addr,
                                         *ipc_struct);
        break;
      case PFC_IPCTYPE_STRING:
        {
          *st_num = IpctSt::kIpcStString;
          READ_PRIMARY_IPCTYPE_FROM_CLIENT(sess, index, const char *,
                                           *ipc_struct);
          char *str = strdup(*(reinterpret_cast<char **>(*ipc_struct)));
          ConfigKeyVal::Free(*ipc_struct);
          *ipc_struct = str;
        }
        break;
      case PFC_IPCTYPE_STRUCT:
        ret = ReadIpcStruct(sess, index, st_num, ipc_struct);
        break;
      case PFC_IPCTYPE_BINARY:            // NOT USED
      default:
        ret = false;
    }
    return ret;
}

bool IpcUtil::ReadIpcStruct(pfc::core::ipc::ServerSession *sess,
                            uint32_t index,
                            IpctSt::IpcStructNum *st_num,
                            void **ipc_struct) {
  std::string st_name;
  if (0 != sess->getArgStructName(index, st_name)) {
    UPLL_LOG_DEBUG("Failed to get struct_name in the ipc message");
    return false;
  }
  const pfc_ipcstdef_t *st_def = IpctSt::GetIpcStdef(st_name.c_str());
  if (st_def == NULL) {
    UPLL_LOG_DEBUG("Unknown structure %s in the ipc message",
                  st_name.c_str());
    return false;
  }
  *st_num = IpctSt::GetIpcStNum(st_name.c_str());

  *ipc_struct = ConfigKeyVal::Malloc(st_def->ist_size);
  if (0 != sess->getArgument(index, *st_def, *ipc_struct)) {
    UPLL_LOG_DEBUG("Failed to get structure in the ipc message");
    ConfigKeyVal::Free(*ipc_struct);
    *ipc_struct = NULL;
    return false;
  }

  return true;
}

bool IpcUtil::ReadIpcStruct(pfc::core::ipc::ClientSession *sess,
                            uint32_t index,
                            IpctSt::IpcStructNum *st_num,
                            void **ipc_struct) {
  std::string st_name;
  if (0 != sess->getResponseStructName(index, st_name)) {
    UPLL_LOG_DEBUG("Failed to get struct_name in the ipc message");
    return false;
  }
  const pfc_ipcstdef_t *st_def = IpctSt::GetIpcStdef(st_name.c_str());
  if (st_def == NULL) {
    UPLL_LOG_DEBUG("Unknown structure %s in the ipc message",
                  st_name.c_str());
    return false;
  }
  *st_num = IpctSt::GetIpcStNum(st_name.c_str());

  *ipc_struct = ConfigKeyVal::Malloc(st_def->ist_size);
  if (0 != sess->getResponse(index, *st_def, *ipc_struct)) {
    UPLL_LOG_DEBUG("Failed to get structure in the ipc message");
    ConfigKeyVal::Free(*ipc_struct);
    *ipc_struct = NULL;
    return false;
  }

  return true;
}

bool IpcUtil::WriteIpcArg(pfc::core::ipc::ServerSession *sess,
                          IpctSt::IpcStructNum st_num, const void *ipc_struct) {
  bool ret;
  switch (st_num) {
    case IpctSt::kIpcStInt8:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, int8_t, ipc_struct);
      break;
    case IpctSt::kIpcStUint8:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, uint8_t, ipc_struct);
      break;
    case IpctSt::kIpcStInt16:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, int16_t, ipc_struct);
      break;
    case IpctSt::kIpcStUint16:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, uint16_t, ipc_struct);
      break;
    case IpctSt::kIpcStInt32:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, int32_t, ipc_struct);
      break;
    case IpctSt::kIpcStUint32:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, uint32_t, ipc_struct);
      break;
    case IpctSt::kIpcStInt64:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, int64_t, ipc_struct);
      break;
    case IpctSt::kIpcStUint64:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, uint64_t, ipc_struct);
      break;
    case IpctSt::kIpcStFloat:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, float, ipc_struct);
      break;
    case IpctSt::kIpcStDouble:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, double, ipc_struct);
      break;
    case IpctSt::kIpcStIpv4:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, struct in_addr, ipc_struct);
      break;
    case IpctSt::kIpcStIpv6:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, struct in6_addr, ipc_struct);
      break;
    case IpctSt::kIpcStString:
      /* With -03 the following line gives compilation error.
       * So macro isn't used.
       * WRITE_PRIMARY_IPCTYPE(sess, addOutput, const char *, &ipc_struct);
      */
      {
        ret = true;
        int ipc_ret = sess->addOutput(
            reinterpret_cast<const char *>(ipc_struct));
        if (ipc_ret != 0) {
          UPLL_LOG_DEBUG("Failed to read argument in the IPC request, Err=%d",
                         ipc_ret);
          ret = false;
        }
      }
      break;
    case IpctSt::kIpcStBinary:
      return false;
    default:
      ret = WriteIpcStruct(sess, st_num, ipc_struct);
  }
  return ret;
}

bool IpcUtil::WriteIpcArg(pfc::core::ipc::ClientSession *sess,
                          IpctSt::IpcStructNum st_num, const void *ipc_struct) {
  bool ret;
  switch (st_num) {
    case IpctSt::kIpcStInt8:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, int8_t, ipc_struct);
      break;
    case IpctSt::kIpcStUint8:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, uint8_t, ipc_struct);
      break;
    case IpctSt::kIpcStInt16:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, int16_t, ipc_struct);
      break;
    case IpctSt::kIpcStUint16:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, uint16_t, ipc_struct);
      break;
    case IpctSt::kIpcStInt32:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, int32_t, ipc_struct);
      break;
    case IpctSt::kIpcStUint32:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, uint32_t, ipc_struct);
      break;
    case IpctSt::kIpcStInt64:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, int64_t, ipc_struct);
      break;
    case IpctSt::kIpcStUint64:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, uint64_t, ipc_struct);
      break;
    case IpctSt::kIpcStFloat:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, float, ipc_struct);
      break;
    case IpctSt::kIpcStDouble:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, double, ipc_struct);
      break;
    case IpctSt::kIpcStIpv4:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, struct in_addr, ipc_struct);
      break;
    case IpctSt::kIpcStIpv6:
      WRITE_PRIMARY_IPCTYPE(sess, addOutput, struct in6_addr, ipc_struct);
      break;
    case IpctSt::kIpcStString:
      /* With -03 the following line gives compilation error.
       * So macro isn't used.
       * WRITE_PRIMARY_IPCTYPE(sess, addOutput, const char *, &ipc_struct);
      */
      {
        ret = true;
        int ipc_ret = sess->addOutput(
            reinterpret_cast<const char *>(ipc_struct));
        if (ipc_ret != 0) {
          UPLL_LOG_DEBUG("Failed to read argument in the IPC request, Err=%d",
                         ipc_ret);
          ret = false;
        }
      }
      break;
    case IpctSt::kIpcStBinary:
      return false;
    default:
      ret = WriteIpcStruct(sess, st_num, ipc_struct);
  }
  return ret;
}

bool IpcUtil::WriteIpcStruct(pfc::core::ipc::ServerSession *sess,
                             IpctSt::IpcStructNum st_num,
                             const void *ipc_struct) {
  PFC_ASSERT(sess != NULL);
  PFC_ASSERT(ipc_struct != NULL);
  if (sess == NULL || ipc_struct == NULL)
    return false;

  const pfc_ipcstdef_t *st_def = IpctSt::GetIpcStdef(st_num);
  if (st_def == NULL) {
    UPLL_LOG_DEBUG("Unknown structure %d", st_num);
    return false;
  }
  if (0 != sess->addOutput(*st_def, ipc_struct)) {
    UPLL_LOG_DEBUG("Failed to write structure %d", st_num);
    return false;
  }
  return true;
}

bool IpcUtil::WriteIpcStruct(pfc::core::ipc::ClientSession *sess,
                             IpctSt::IpcStructNum st_num,
                             const void *ipc_struct) {
  PFC_ASSERT(sess != NULL);
  PFC_ASSERT(ipc_struct != NULL);
  if (sess == NULL || ipc_struct == NULL)
    return false;

  const pfc_ipcstdef_t *st_def = IpctSt::GetIpcStdef(st_num);
  if (st_def == NULL) {
    UPLL_LOG_DEBUG("Unknown structure %d", st_num);
    return false;
  }
  if (0 != sess->addOutput(*st_def, ipc_struct)) {
    UPLL_LOG_DEBUG("Failed to write structure %d", st_num);
    return false;
  }
  return true;
}

/* Read KT request from UPLL user */
bool IpcUtil::ReadKtRequest(pfc::core::ipc::ServerSession *sess,
                            pfc_ipcid_t /* service */,
                            IpcReqRespHeader *msg_hdr,
                            ConfigKeyVal **first_ckv) {
  UPLL_FUNC_TRACE;
  if (sess == NULL || msg_hdr == NULL || first_ckv == NULL) {
    UPLL_LOG_DEBUG("Null argument");
    return false;
  }
  *first_ckv = NULL;

  bzero(msg_hdr, sizeof(*msg_hdr));

  uint32_t arg_cnt = sess->getArgCount();
  if (arg_cnt < unc::upll::ipc_util::kKeyTreeReqMandatoryFields) {
    UPLL_LOG_DEBUG("Not enough arguments in key tree request. Has only %u.",
                  arg_cnt);
    return false;
  }

  uint32_t keytype;
  uint32_t operation;
  uint32_t option1;
  uint32_t option2;
  uint32_t datatype;
  uint32_t arg = 0;

  if ((0 != sess->getArgument(arg++, msg_hdr->clnt_sess_id)) ||
      (0 != sess->getArgument(arg++, msg_hdr->config_id)) ||
      (0 != sess->getArgument(arg++, operation)) ||
      (0 != sess->getArgument(arg++, msg_hdr->rep_count)) ||
      (0 != sess->getArgument(arg++, option1)) ||
      (0 != sess->getArgument(arg++, option2)) ||
      (0 != sess->getArgument(arg++, datatype)) ||
      (0 != sess->getArgument(arg++, keytype)) ) {
    UPLL_LOG_DEBUG("Invalid header field #%u in the key tree request", arg);
    return false;
  }
  msg_hdr->operation = (unc_keytype_operation_t)operation;
  msg_hdr->option1 = (unc_keytype_option1_t)option1;
  msg_hdr->option2 = (unc_keytype_option2_t)option2;
  msg_hdr->datatype = (upll_keytype_datatype_t)datatype;

  IpctSt::IpcStructNum st_num;
  void *ipc_st;
  if (!IpcUtil::ReadIpcStruct(sess, arg++, &st_num, &ipc_st)) {
    UPLL_LOG_DEBUG("Failed to get structure at %u in the key tree request",
                   arg);
    return false;
  }
  *first_ckv = new ConfigKeyVal((unc_key_type_t)keytype, st_num, ipc_st, NULL);
  ConfigKeyVal *curr_ckv = *first_ckv;

  while (arg < arg_cnt) {
    pfc_ipctype_t arg_type;
    if (0 != sess->getArgType(arg, arg_type)) {
      UPLL_LOG_DEBUG("Failed to get arg type in the key tree request");
      delete *first_ckv;
      *first_ckv = NULL;
      return false;
    }
    if (arg_type == PFC_IPCTYPE_NULL) {
      arg++;    // skip NULL
      // new key type is expected.
      if (0 != sess->getArgument(arg++, keytype)) {
        UPLL_LOG_DEBUG("Failed to get key type at %u in the key tree request",
                      arg);
        delete *first_ckv;
        *first_ckv = NULL;
        return false;
      }
      if (!IpcUtil::ReadIpcStruct(sess, arg++, &st_num, &ipc_st)) {
        UPLL_LOG_DEBUG("Failed to get structure at %u in the key tree request",
                      arg);
        delete *first_ckv;
        *first_ckv = NULL;
        return false;
      }
      ConfigKeyVal *next_ckv = new ConfigKeyVal((unc_key_type_t)keytype,
                                                st_num, ipc_st, NULL);
      curr_ckv->set_next_cfg_key_val(next_ckv);
      curr_ckv = next_ckv;
    } else {
      if (!IpcUtil::ReadIpcArg(sess, arg++, &st_num, &ipc_st)) {
        UPLL_LOG_DEBUG("Failed to get structure at %u in the key tree request",
                      arg);
        delete *first_ckv;
        *first_ckv = NULL;
        return false;
      }
      curr_ckv->AppendCfgVal(st_num, ipc_st);
    }
  }
  return true;
}

/* Send response to UPLL user */
bool IpcUtil::WriteKtResponse(pfc::core::ipc::ServerSession *sess,
                             const IpcReqRespHeader &msg_hdr,
                             const ConfigKeyVal *first_ckv) {
  if (sess == NULL) {
    UPLL_LOG_DEBUG("Null argument");
    return false;
  }

  int err;
  if ((0 != (err = sess->addOutput(msg_hdr.clnt_sess_id))) ||
      (0 != (err = sess->addOutput(msg_hdr.config_id))) ||
      (0 != (err = sess->addOutput((uint32_t)msg_hdr.operation))) ||
      (0 != (err = sess->addOutput(msg_hdr.rep_count))) ||
      (0 != (err = sess->addOutput((uint32_t)msg_hdr.option1))) ||
      (0 != (err = sess->addOutput((uint32_t)msg_hdr.option2))) ||
      (0 != (err = sess->addOutput((uint32_t)msg_hdr.datatype))) ||
      (0 != (err = sess->addOutput((uint32_t)msg_hdr.result_code))) ) {
    UPLL_LOG_DEBUG("Failed to add header to key tree response. Err=%d", err);
    return false;
  }

  bool add_err = false;
  for (const ConfigKeyVal *ckv = first_ckv;
       ckv; ckv = ckv->get_next_cfg_key_val()) {
    if (ckv != first_ckv) {
      // For every additional CKV, add NULL separator
      if (0 != (err = sess->addOutput())) {
        UPLL_LOG_DEBUG("Failed to add NULL to key tree response. Err=%d", err);
        add_err = true;
        break;
      }
    }
    if (0 != (err = sess->addOutput((uint32_t)ckv->get_key_type()))) {
      UPLL_LOG_DEBUG("Failed to add key type to key tree response. Err=%d",
                     err);
      add_err = true;
      break;
    }
    if (!IpcUtil::WriteIpcArg(sess, ckv->get_st_num(), ckv->get_key())) {
      add_err = true;
      break;
    }
    for (ConfigVal *cv = ckv->get_cfg_val(); cv; cv =
         cv->get_next_cfg_val()) {
      if (!IpcUtil::WriteIpcArg(sess, cv->get_st_num(), cv->get_val())) {
        add_err = true;
        break;       // break from for loop
      }
    }
    if (add_err == true) {
      break;         // break from while (ckv != NULL)
    }
  }

  if (add_err == true) {
    return false;
  }
  return true;
}

/* Write KT request to Server */
                             // ctrlr_name: Non-null if sent to driver
bool IpcUtil::WriteKtRequest(pfc::core::ipc::ClientSession *sess,
                             bool driver_msg,
                             const char *ctrlr_name, const char *domain_id,
                             const IpcReqRespHeader &msg_hdr,
                             const ConfigKeyVal *first_ckv) {
  if (sess == NULL || first_ckv == NULL) {
    UPLL_LOG_DEBUG("Null argument");
    return false;
  }
  if (driver_msg == true && ctrlr_name == NULL) {
    UPLL_LOG_DEBUG("Null argument");
    return false;
  }

  PFC_ASSERT_PRINTF(msg_hdr.datatype != UPLL_DT_AUDIT,
                    "UPLL_DT_AUDIT cannot be sent in IPC message");

  int err;

  if ((0 != (err = sess->addOutput(msg_hdr.clnt_sess_id))) ||
      (0 != (err = sess->addOutput(msg_hdr.config_id)))) {
    UPLL_LOG_DEBUG("Failed to write key tree message header. Err=%d", err);
    return false;
  }
  // insert controller name and domain id if this message is for a controller
  if (ctrlr_name != NULL) {
    if (0 != (err =sess->addOutput(ctrlr_name))) {
      UPLL_LOG_DEBUG("Failed to write key tree message header. Err=%d", err);
      return false;
    }
    if (domain_id == NULL) {
      domain_id = "";
    }
    if (0 != (err =sess->addOutput(domain_id))) {
      UPLL_LOG_DEBUG("Failed to write key tree message header. Err=%d", err);
      return false;
    }
  }
  if ((0 != (err = sess->addOutput((uint32_t)msg_hdr.operation))) ||
      (0 != (err = sess->addOutput(msg_hdr.rep_count))) ||
      (0 != (err = sess->addOutput((uint32_t)msg_hdr.option1))) ||
      (0 != (err = sess->addOutput((uint32_t)msg_hdr.option2))) ||
      (0 != (err = sess->addOutput((uint32_t)msg_hdr.datatype))) ) {
    UPLL_LOG_DEBUG("Failed to add header to key tree request. Err=%d", err);
    return false;
  }

  bool add_err = false;
  for (const ConfigKeyVal *ckv = first_ckv;
       ckv; ckv = ckv->get_next_cfg_key_val()) {
    if (ckv != first_ckv) {
      // For every additional CKV, add NULL separator
      if (0 != sess->addOutput()) {
        UPLL_LOG_DEBUG("Failed to add NULL to key tree request. Err=%d", err);
        add_err = true;
        break;
      }
    }
    if (0 != (err = sess->addOutput((uint32_t)ckv->get_key_type()))) {
      UPLL_LOG_DEBUG("Failed to add key type to key tree request. Err=%d", err);
      add_err = true;
      break;
    }
    if (!IpcUtil::WriteIpcArg(sess, ckv->get_st_num(), ckv->get_key())) {
      add_err = true;
      break;
    }
    for (ConfigVal *cv = ckv->get_cfg_val(); cv;
         cv = cv->get_next_cfg_val()) {
      if (!IpcUtil::WriteIpcArg(sess, cv->get_st_num(), cv->get_val())) {
        add_err = true;
        break;       // break from for loop
      }
    }
    if (add_err == true) {
      break;         // break from while (ckv != NULL)
    }
  }

  if (add_err == true) {
    return false;
  }

  return true;
}

void DumpResponse(pfc::core::ipc::ClientSession *sess) {
  UPLL_FUNC_TRACE;
  int err;
  uint32_t arg = 0;
  pfc_ipctype_t arg_type;
  uint32_t arg_cnt = sess->getResponseCount();
  while (arg < arg_cnt) {
    if (0 != (err = sess->getResponseType(arg, arg_type))) {
      UPLL_LOG_TRACE("GetResponseType Failed \n");
      return;
    }
    UPLL_LOG_TRACE("IpcReponse: Pos %d: %d", arg, arg_type);
    switch (arg_type) {
      case PFC_IPCTYPE_INT8:
      case PFC_IPCTYPE_UINT8:
      case PFC_IPCTYPE_INT16:
      case PFC_IPCTYPE_UINT16:
      case PFC_IPCTYPE_INT32:
      case PFC_IPCTYPE_UINT32:
      case PFC_IPCTYPE_INT64:
      case PFC_IPCTYPE_UINT64:
      case PFC_IPCTYPE_FLOAT:
      case PFC_IPCTYPE_DOUBLE:
      case PFC_IPCTYPE_IPV4:
      case PFC_IPCTYPE_IPV6:
      case PFC_IPCTYPE_STRING:
      case PFC_IPCTYPE_BINARY:            // NOT USED
      case PFC_IPCTYPE_NULL:
        break;
      case PFC_IPCTYPE_STRUCT:
        {
          std::string st_name;
          if (0 != sess->getResponseStructName(arg, st_name)) {
            return;
          }
          UPLL_LOG_TRACE("IpcReponse: Pos %d: %s", arg, st_name.c_str());
        }
    }
    arg++;
  }
}

/* Read KT response from Server */
/* if driver_msg is true, then the message is from driver, otherwise it is from
 * Physical. If the driver_msg is true and non-null domain_id pointer is passed
 * then the domain_id from the message is placed in the domain_id pointer.
 * Caller should have sent a valid pointer for copying domain id */
bool IpcUtil::ReadKtResponse(pfc::core::ipc::ClientSession *sess,
                             pfc_ipcid_t /* service */,
                             bool driver_msg, char *domain_id,
                             IpcReqRespHeader *msg_hdr,
                             ConfigKeyVal **first_ckv) {
  UPLL_FUNC_TRACE;
  if (sess == NULL || msg_hdr == NULL || first_ckv == NULL) {
    UPLL_LOG_DEBUG("Null argument");
    return false;
  }
  *first_ckv = NULL;

  bzero(msg_hdr, sizeof(*msg_hdr));

  uint32_t arg_cnt = sess->getResponseCount();
  uint32_t mandatory_fields = (driver_msg) ? kKeyTreeDriverRespMandatoryFields :
      kKeyTreeRespMandatoryFields;
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
  uint32_t arg = 0;

  if ((0 != (err = sess->getResponse(arg++, msg_hdr->clnt_sess_id))) ||
      (0 != (err = sess->getResponse(arg++, msg_hdr->config_id)))) {
    UPLL_LOG_DEBUG("Failed to get header field #%u in the key tree response."
                  " Err=%d", arg, err);
    return false;
  }
  if (driver_msg == true) {
    if ((0 != (err = sess->getResponse(arg++, msg_ctrlr_name))) ||
        (0 != (err = sess->getResponse(arg++, msg_domain_id)))) {
      UPLL_LOG_DEBUG("Failed to get header field #%u in the key tree response."
          " Err=%d", arg, err);
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
  msg_hdr->operation = (unc_keytype_operation_t)operation;
  msg_hdr->option1 = (unc_keytype_option1_t)option1;
  msg_hdr->option2 = (unc_keytype_option2_t)option2;
  msg_hdr->datatype = (upll_keytype_datatype_t)datatype;
  msg_hdr->result_code = (upll_rc_t) result_code;

  IpctSt::IpcStructNum st_num;
  void *ipc_st;
  if (!IpcUtil::ReadIpcStruct(sess, arg++, &st_num, &ipc_st)) {
    UPLL_LOG_DEBUG("Failed to get structure at %u in the key tree response",
                  arg);
    return false;
  }
  *first_ckv = new ConfigKeyVal((unc_key_type_t)keytype, st_num, ipc_st, NULL);
  ConfigKeyVal *curr_ckv = *first_ckv;
  ConfigVal *curr_cv = NULL;

  // read all key type and value structures
  while (arg < arg_cnt) {
    pfc_ipctype_t arg_type;
    if (0 != (err = sess->getResponseType(arg, arg_type))) {
      UPLL_LOG_DEBUG("Failed to get arg type at %u in the key tree response."
                    " Err=%d", arg, err);
      delete *first_ckv;
      *first_ckv = NULL;
      return false;
    }
    if (arg_type == PFC_IPCTYPE_NULL) {
      if ((arg+2) >= arg_cnt) {
        UPLL_LOG_DEBUG("Not enough fields after NULL in the key tree response."
                       " pos:%d, cnt:%d", arg, arg_cnt);
        delete *first_ckv;
        *first_ckv = NULL;
        return false;
      }
      arg++;    // skip NULL
      // new key type is expected.
      if (0 != (err = sess->getResponse(arg++, keytype))) {
        UPLL_LOG_DEBUG("Failed to get key type at %u in the key tree response."
                      " Err=%d", arg, err);
        DumpResponse(sess);
        delete *first_ckv;
        *first_ckv = NULL;
        return false;
      }
      if (!IpcUtil::ReadIpcStruct(sess, arg++, &st_num, &ipc_st)) {
        UPLL_LOG_DEBUG("Failed to get structure at %u in the key tree response",
                      arg);
        delete *first_ckv;
        *first_ckv = NULL;
        return false;
      }
      ConfigKeyVal *next_ckv = new ConfigKeyVal((unc_key_type_t)keytype,
                                                st_num, ipc_st, NULL);
      curr_ckv->set_next_cfg_key_val(next_ckv);
      curr_ckv = next_ckv;
      curr_cv = NULL;
    } else {
      if (!IpcUtil::ReadIpcArg(sess, arg++, &st_num, &ipc_st)) {
        UPLL_LOG_DEBUG("Failed to get structure at %u in the key tree response",
                      arg);
        delete *first_ckv;
        *first_ckv = NULL;
        return false;
      }
      if (curr_cv == NULL) {
        curr_ckv->AppendCfgVal(st_num, ipc_st);
        curr_cv = curr_ckv->get_cfg_val();
      } else {
        curr_cv->AppendCfgVal(st_num, ipc_st);
        curr_cv = curr_cv->get_next_cfg_val();
      }
    }
  }
  return true;
}

std::string IpcUtil::IpcRequestToStr(const IpcReqRespHeader &msghdr) {
  std::stringstream ss;
  ss << "-------------IpcRequest--------------" << std::endl
     << "  Hdr=clnt_sess_id:" << msghdr.clnt_sess_id
     << ",  config_id:" << msghdr.config_id
     << ",  operation:" << msghdr.operation
     << ",  max_rep_count:" << msghdr.rep_count
     << ",  option1:" << msghdr.option1
     << ",  option2:" << msghdr.option2
     << ",  datatype:" << msghdr.datatype;
  return ss.str();
}

std::string IpcUtil::IpcResponseToStr(const IpcReqRespHeader &msghdr) {
  std::stringstream ss;
  ss << "-------------IpcResponse--------------" << std::endl
     << "  Hdr=clnt_sess_id:" << msghdr.clnt_sess_id
     << ",  config_id:" << msghdr.config_id
     << ",  operation:" << msghdr.operation
     << ",  rep_count:" << msghdr.rep_count
     << ",  option1:" << msghdr.option1
     << ",  option2:" << msghdr.option2
     << ",  datatype:" << msghdr.datatype
     << ",  result_code:" << msghdr.result_code;
  return ss.str();
}


// User need to allocate ConfigNotification using new. This function deletes the
// notification when the notification is sent out.
bool ConfigNotifier::BufferNotificationToUpllUser(ConfigNotification *notif) {
  if (notif == NULL) {
    UPLL_LOG_DEBUG("ConfigNotification passed is NULL");
    return false;
  }
  // Val1 in cvk_ is current key, Meaningful in the case of create and update.
  // Val2 in cvk_ is old value. Meaningful in the case of update.
  ConfigKeyVal *ckv = notif->get_ckv();
  if (ckv == NULL) {
    UPLL_LOG_DEBUG("Bad config notification - no key");
    return false;
  }
  ConfigVal *val1 = ckv->get_cfg_val();
  ConfigVal *val2 = (val1 ? val1->get_next_cfg_val() : NULL);
  switch (notif->get_operation()) {
    case UNC_OP_CREATE:
      {
        if (val1 == NULL) {
          UPLL_LOG_DEBUG("Bad create config notification - Val1 is none - %s",
                         ckv->ToStr().c_str());
          return false;
        }
        if (val2 != NULL) {
          UPLL_LOG_DEBUG("Bad create config notification - Val2 given - %s",
                         ckv->ToStr().c_str());
          return false;
        }
      }
      break;
    case UNC_OP_UPDATE:
      {
        if (val1 == NULL || val2 == NULL) {
          UPLL_LOG_DEBUG("Bad update config notification - %s",
                         ckv->ToStr().c_str());
          return false;
        }
      }
      break;
    case UNC_OP_DELETE:
      {
        if (!(val1 == NULL && val2 == NULL)) {
          UPLL_LOG_DEBUG("Bad delete config notification - %s",
                         ckv->ToStr().c_str());
          return false;
        }
      }
      break;
    default:
      UPLL_LOG_DEBUG("Bad opeartion in config notification - %s",
                     ckv->ToStr().c_str());
      return false;
  }
  pfc::core::ScopedMutex sm(notif_lock);
  buffered_notifs.push_back(notif);
  return true;
}

bool ConfigNotifier::SendBufferedNotificationsToUpllUser() {
  pfc::core::ScopedMutex sm(notif_lock);

  // Send out buffered notifications
  ConfigNotification *cn;
  std::list<ConfigNotification*>::iterator it = buffered_notifs.begin();
  for (; (it != buffered_notifs.end());
       it++, (delete cn)) {
    cn = *it;
    if (IpcUtil::IsShuttingDown()) {
      UPLL_LOG_WARN("Cannot send config nofitications as shutdown in progress");
      break;
    }
    int err;
    pfc::core::ipc::ServerEvent event(UPLL_EV_CONFIG_NOTIFICATION, err);
    if (err != 0) {
      UPLL_LOG_DEBUG("Failed to create configuration notification event."
                    " Err:%d", err);
      continue;
    }

    ConfigKeyVal *ckv = cn->get_ckv();
    if (ckv == NULL)
      continue;

    // Event format: operation, datatype, keytype, key, new-val, old-val
    if ((0 != (err = event.addOutput((uint32_t)cn->get_operation()))) ||
        (0 != (err = event.addOutput((uint32_t)cn->get_datatype()))) ||
        (0 != (err = event.addOutput((uint32_t)cn->get_ckv()->get_key_type())))
        ) {
      UPLL_LOG_DEBUG("Failed to add output to configuration notification."
                    " Err=%d", err);
      continue;
    }
    ConfigVal *val1 = ckv->get_cfg_val();
    ConfigVal *val2 = (val1 ? val1->get_next_cfg_val() : NULL);

    if (!IpcUtil::WriteIpcStruct(&event, ckv->get_st_num(), ckv->get_key())) {
      UPLL_LOG_DEBUG("Failed to add output to configuration notification.");
      continue;
    }

    if (val1 && !IpcUtil::WriteIpcStruct(&event, val1->get_st_num(),
                                         val1->get_val())) {
      UPLL_LOG_DEBUG("Failed to add output to configuration notification.");
      continue;
    }
    if (val2 && !IpcUtil::WriteIpcStruct(&event, val2->get_st_num(),
                                         val2->get_val())) {
      UPLL_LOG_DEBUG("Failed to add output to configuration notification.");
      continue;
    }
    err = event.post();
    if (err != 0) {
      UPLL_LOG_DEBUG("Failed to post configuration notification event."
                    " Err=%d", err);
      continue;
    }
  }
  buffered_notifs.clear();
  return true;
}

bool ConfigNotifier::CancelBufferedNotificationsToUpllUser() {
  pfc::core::ScopedMutex sm(notif_lock);
  for (std::list<ConfigNotification*>::iterator it = buffered_notifs.begin();
       it != buffered_notifs.end(); it++) {
    ConfigNotification *cn = *it;
    delete cn;
  }
  buffered_notifs.clear();
  return true;
}

bool ConfigNotifier::SendOperStatusNotification(const ConfigKeyVal *ckv) {
  if (IpcUtil::IsShuttingDown()) {
    UPLL_LOG_WARN("Cannot send operstatus notification. Shutdown in progress");
    return false;
  }
  if (ckv == NULL) {
    UPLL_LOG_DEBUG("Bad operstatus notification - no key");
    return false;
  }
  const ConfigVal *val1 = ckv->get_cfg_val();
  const ConfigVal *val2 = (val1 ? val1->get_next_cfg_val() : NULL);
  if (val1 == NULL || val2 == NULL) {
    UPLL_LOG_DEBUG("Bad operstatus update notification - %s",
                   ckv->ToStr().c_str());
    return false;
  }

  int err;
  pfc::core::ipc::ServerEvent event(UPLL_EV_OPER_NOTIFICATION, err);
  if (err != 0) {
    UPLL_LOG_DEBUG("Failed to create operstatus notification event. Err:%d",
                   err);
    return false;
  }

  // Event format: operation, datatype, keytype, key, new-val, old-val
  if ((0 != (err = event.addOutput((uint32_t)UNC_OP_UPDATE))) ||
      (0 != (err = event.addOutput((uint32_t)UNC_DT_STATE))) ||
      (0 != (err = event.addOutput((uint32_t)ckv->get_key_type())))
     ) {
    UPLL_LOG_DEBUG("Failed to add output to operstatus notification. Err=%d",
                   err);
    return false;
  }
  if ((!IpcUtil::WriteIpcStruct(&event, ckv->get_st_num(), ckv->get_key())) ||
      (!IpcUtil::WriteIpcStruct(&event, val1->get_st_num(), val1->get_val())) ||
      (!IpcUtil::WriteIpcStruct(&event, val2->get_st_num(), val2->get_val()))) {
    UPLL_LOG_DEBUG("Failed to add output to operstatus notification.");
    return false;
  }
  err = event.post();
  if (err != 0) {
    UPLL_LOG_DEBUG("Failed to post operstatus notification event. Err=%d", err);
    return false;
  }

  return true;
}

}  // namespace ipc_util
}  // namespace upll
}  // namespace unc
