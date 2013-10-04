/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * upll_main.cc - UPLL module
 */

#include <map>

#include "pfc/debug.h"
#include "pfc/log.h"

#include "unc/upll_ipc_enum.h"
#include "pfc/ipc_struct.h"

#include "unc/component.h"
#include "unc/keytype.h"
#include "unc/unc_events.h"

#include "alarm.hh"
#include <clstat_api.h>

#include "unc/uppl_common.h"
#include "unc/pfcdriver_include.h"

#include "unc/upll_svc.h"
#include "uncxx/upll_log.hh"

#include "kt_util.hh"
#include "ctrlr_mgr.hh"
#include "config_svc.hh"

namespace unc {
namespace upll {
namespace config_svc {

using pfc::core::ipc::IpcEvent;
using unc::upll::ipc_util::IpcReqRespHeader;
using unc::upll::ipc_util::IpcUtil;
using unc::upll::ipc_util::IpctSt;
using unc::upll::ipc_util::ConfigKeyVal;
using unc::upll::ipc_util::ConfigVal;
using unc::upll::ipc_util::ConfigNotification;
using unc::upll::ipc_util::KtUtil;
using unc::upll::config_momgr::UpllConfigMgr;
using unc::upll::config_momgr::CtrlrMgr;

unsigned int UpllConfigSvc::kNumCfgServices = 4;

/*
 * UpllConfigSvc::UpllConfigSvc(const pfc_modattr_t *mattr)
 * Constructor of UpplModule instance.
 */
UpllConfigSvc::UpllConfigSvc(const pfc_modattr_t *mattr)
    : pfc::core::Module(mattr), sys_evhid_(EVHANDLER_ID_INVALID),
    cls_evhid_(EVHANDLER_ID_INVALID) {
  UPLL_LOG_NOTICE("constructor");
  shutting_down_ = false;
  node_active_ = false;
  config_mgr_ = NULL;
  physical_evhdlr_ = NULL;
  pfcdriver_evhdlr_ = NULL;
}

/*
 * UpllConfigSvc::~UpllConfigSvc(void)
 * Destructor of UpllConfigSvc instance.
 */
UpllConfigSvc::~UpllConfigSvc(void) {
  UPLL_LOG_NOTICE("destructor");
}

/*
 * pfc_bool_t
 * UpllConfigSvc::init(void)
 * Initialize UpllConfigSvc.
 */
pfc_bool_t UpllConfigSvc::init(void) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_NOTICE("init() called");

  // Create UpllConfigMgr
  config_mgr_ = UpllConfigMgr::GetUpllConfigMgr();

  if (!RegisterForModuleEvents()) {
    return PFC_FALSE;
  }

  return PFC_TRUE;
}


/*
 * pfc_bool_t
 * UpllConfigSvc::fini(void)
 * Finalize UpllConfigSvc.
 */
pfc_bool_t
UpllConfigSvc::fini(void) {
  UPLL_LOG_NOTICE("fini() called.");

  UnregisterModuleEventHandlers();
  // UnregisterIpcEventHandlers(); - Not required. module.c release ipc ev hdlrs

  return PFC_TRUE;
}

// IPC service handler
//
pfc_ipcresp_t UpllConfigSvc::ipcService(
    pfc::core::ipc::ServerSession &sess,                     // NOLINT for &sess
    pfc_ipcid_t service) {
  UPLL_FUNC_TRACE;

  upll_service_ids_t svcid = (upll_service_ids_t) service;
  bool shutdown_in_progress = IsShuttingDown();

  if (shutdown_in_progress) {
    IpcReqRespHeader msghdr;
    UPLL_LOG_WARN("UPLL is shutting down. Does not process any IPC requests");
    bzero(&msghdr, sizeof(msghdr));
    msghdr.result_code = UPLL_RC_ERR_SHUTTING_DOWN;
    if (!IpcUtil::WriteKtResponse(&sess, msghdr, NULL)) {
      UPLL_LOG_DEBUG("Failed to send response to UPLL IPC service user");
      return PFC_IPCRESP_FATAL;
    }
  }
  if ((!IsActiveNode()) && (svcid != UPLL_READ_SVC_ID)) {
    IpcReqRespHeader msghdr;
    UPLL_LOG_WARN("Standby node does not support service %u", svcid);
    bzero(&msghdr, sizeof(msghdr));
    msghdr.result_code = UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
    if (!IpcUtil::WriteKtResponse(&sess, msghdr, NULL)) {
      UPLL_LOG_DEBUG("Failed to send response to UPLL IPC service user");
      return PFC_IPCRESP_FATAL;
    }
  }

  switch (svcid) {
    case UPLL_EDIT_SVC_ID:
    case UPLL_CONTROL_SVC_ID:
    case UPLL_READ_SVC_ID:
      return KtService(&sess, svcid);
      break;
    case UPLL_GLOBAL_CONFIG_SVC_ID:
      return GlobalConfigService(&sess, svcid);
      break;
    default:
      {
        UPLL_LOG_DEBUG("Invalid service %u", svcid);
        IpcReqRespHeader msghdr;
        bzero(&msghdr, sizeof(msghdr));
        msghdr.result_code = UPLL_RC_ERR_BAD_REQUEST;
        if (!IpcUtil::WriteKtResponse(&sess, msghdr, NULL)) {
          UPLL_LOG_DEBUG("Failed to send response to UPLL IPC service user");
          return PFC_IPCRESP_FATAL;
        }
      }
  }
  return 0;
}

pfc_ipcresp_t UpllConfigSvc::KtService(pfc::core::ipc::ServerSession *sess,
                                       upll_service_ids_t service) {
  UPLL_FUNC_TRACE;
  IpcReqRespHeader msghdr;
  ConfigKeyVal *ckv;

  if (!IpcUtil::ReadKtRequest(sess, service, &msghdr, &ckv)) {
    UPLL_LOG_DEBUG("Failed in reading the key tree request");
    return PFC_IPCRESP_FATAL;
  }

  // Increasing IPC session timeout only for PING operation
  if (msghdr.operation == UNC_OP_CONTROL && msghdr.option2 == UNC_OPT2_PING) {
    pfc_timespec_t ping_tspec;
    ping_tspec.tv_sec = kIpcTimeoutPing;
    ping_tspec.tv_nsec = 0;
    sess->setTimeout(&ping_tspec);
    UPLL_LOG_DEBUG("IPC Server Session timeout for Ping set to %d",
                  kIpcTimeoutPing);
  }

  pfc_ipcresp_t ret = config_mgr_->KtServiceHandler(service, &msghdr, ckv);
  if (ret != 0) {
    UPLL_LOG_DEBUG("Failed in processing the key tree request");
    delete ckv;
    return ret;
  }

  // send the response
  if (!IpcUtil::WriteKtResponse(sess, msghdr, ckv)) {
    UPLL_LOG_DEBUG("Failed to send response to key tree request");
    delete ckv;
    return PFC_IPCRESP_FATAL;
  }

  delete ckv;

  return 0;
}

pfc_ipcresp_t UpllConfigSvc::GlobalConfigService(
    pfc::core::ipc::ServerSession *sess, upll_service_ids_t service) {
  UPLL_FUNC_TRACE;
  uint32_t operation;
  int ipc_err;
  upll_rc_t urc;
  int arg = 0;

  if (0 != (ipc_err = sess->getArgument(arg++, operation))) {
    UPLL_LOG_DEBUG("Unable to read operation from IPC request. Err=%d",
                  ipc_err);
    return PFC_IPCRESP_FATAL;
  }
  switch (operation) {
    case UPLL_IS_CANDIDATE_DIRTY_OP: {
      bool dirty = true;
      sess->setTimeout(NULL);
      UPLL_LOG_DEBUG("IPC Server Session timeout for IS_CANDIDATE_DIRTY is set "
                     " to infinite");
      urc = config_mgr_->IsCandidateDirty(&dirty);
      // Write response
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc))) ||
          (0 != (ipc_err = sess->addOutput((uint8_t)dirty)))) {
        UPLL_LOG_DEBUG("Unable to write IPC response. Err=%d", ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }
    break;

    case UPLL_IMPORT_CTRLR_CONFIG_OP: {
      const char *ctrlr_name;
      uint32_t session_id, config_id;
      if ((0 != (ipc_err = sess->getArgument(arg++, ctrlr_name))) ||
          (0 != (ipc_err = sess->getArgument(arg++, session_id))) ||
          (0 != (ipc_err = sess->getArgument(arg++, config_id)))) {
        UPLL_LOG_DEBUG("Unable to read field at %u from IPC request. Err=%d",
                       arg, ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      if (ctrlr_name == NULL || strlen(ctrlr_name) == 0 ||
          (strlen(ctrlr_name) > (KtUtil::kCtrlrNameLenWith0-1))) {
        UPLL_LOG_DEBUG("controller id wrong");
        if (ctrlr_name != NULL) {
          UPLL_LOG_DEBUG("controller id is '%s'", ctrlr_name);
        }
        // Send error reply
        if (0 != (ipc_err =
                  sess->addOutput((uint32_t)UPLL_RC_ERR_BAD_REQUEST))) {
          UPLL_LOG_DEBUG("Unable to write IPC response. Err=%d", ipc_err);
          return PFC_IPCRESP_FATAL;
        }
        return 0;
      }

      /*
      pfc_timespec_t sess_timeout;
      sess_timeout.tv_sec = kIpcTimeoutImport;
      sess_timeout.tv_nsec = 0;
      sess->setTimeout(&sess_timeout);
      UPLL_LOG_DEBUG("IPC Server Session timeout for Import set to %d",
                     kIpcTimeoutImport);
      */
      sess->setTimeout(NULL);
      UPLL_LOG_DEBUG("IPC Server Session timeout for Import set to infinite.");

      urc = config_mgr_->StartImport(ctrlr_name, session_id, config_id);
      UPLL_LOG_TRACE("StartImport: urc=%d, ctrlr_name=%s,"
                     " session_id=%d, config_id=%d",
                     urc, ctrlr_name, session_id, config_id);
      // Write response
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
        UPLL_LOG_DEBUG("Unable to write IPC response. Err=%d", ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }
    break;

    case UPLL_MERGE_IMPORT_CONFIG_OP: {
      uint32_t session_id, config_id;
      if ((0 != (ipc_err = sess->getArgument(arg++, session_id))) ||
          (0 != (ipc_err = sess->getArgument(arg++, config_id)))) {
        UPLL_LOG_DEBUG("Unable to read field at %u from IPC request. Err=%d",
                       arg, ipc_err);
        return PFC_IPCRESP_FATAL;
      }

      /*
      pfc_timespec_t sess_timeout;
      sess_timeout.tv_sec = kIpcTimeoutImport;
      sess_timeout.tv_nsec = 0;
      sess->setTimeout(&sess_timeout);
      UPLL_LOG_DEBUG("IPC Server Session timeout for ImportMerge set to %d",
                     kIpcTimeoutImport);
      */
      sess->setTimeout(NULL);
      UPLL_LOG_DEBUG("IPC Server Session timeout for Merge set to infinite");

      urc = config_mgr_->OnMerge(session_id, config_id);
      UPLL_LOG_TRACE("Merge: urc=%d, session_id=%d, config_id=%d",
                     urc, session_id, config_id);
      // Write response
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
        UPLL_LOG_DEBUG("Unable to write IPC response. Err=%d", ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }
    break;

    case UPLL_CLEAR_IMPORT_CONFIG_OP: {
      uint32_t session_id, config_id;
      if ((0 != (ipc_err = sess->getArgument(arg++, session_id))) ||
          (0 != (ipc_err = sess->getArgument(arg++, config_id)))) {
        UPLL_LOG_DEBUG("Unable to read field at %u from IPC request. Err=%d",
                       arg, ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      urc = config_mgr_->ClearImport(session_id, config_id, false);
      UPLL_LOG_TRACE("ClearImport: urc=%d, session_id=%d, config_id=%d",
                     urc, session_id, config_id);
      // Write response
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
        UPLL_LOG_DEBUG("Unable to write IPC response. Err=%d", ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }
    break;

    case UPLL_IS_KEY_TYPE_IN_USE_OP:
      return HandleIsKeyInUse(sess, arg);
      break;

    case UPLL_UPPL_UPDATE_OP:
      return HandleUpplUpdate(sess, arg);
      break;
  }
  return PFC_IPCRESP_FATAL;
}

pfc_ipcresp_t UpllConfigSvc::HandleIsKeyInUse(
    pfc::core::ipc::ServerSession *sess, int index) {
  UPLL_FUNC_TRACE;
  bool in_use = true;
  uint32_t datatype;
  uint32_t keytype;
  IpctSt::IpcStructNum st_num;
  void *ipc_st;
  int ipc_err;
  upll_rc_t urc;

  if (0 != (ipc_err = sess->getArgument(index++, datatype))) {
    UPLL_LOG_DEBUG("Unable to read datatype from IPC request. Err=%d", ipc_err);
    return PFC_IPCRESP_FATAL;
  }

  if (0 != (ipc_err = sess->getArgument(index++, keytype))) {
    UPLL_LOG_DEBUG("Unable to read keytype from IPC request. Err=%d", ipc_err);
    return PFC_IPCRESP_FATAL;
  }

  if (keytype != UNC_KT_CONTROLLER && keytype != UNC_KT_BOUNDARY) {
    if ((0 != (ipc_err = sess->addOutput(
                    (uint32_t)UPLL_IS_KEY_TYPE_IN_USE_OP))) ||
        (0 != (ipc_err = sess->addOutput(
                    (uint32_t)UPLL_RC_ERR_NO_SUCH_NAME))) ||
        (0 != (ipc_err = sess->addOutput((uint8_t)in_use)))) {
      UPLL_LOG_DEBUG("Unable to write IPC response. Err=%d", ipc_err);
      return PFC_IPCRESP_FATAL;
    }
  }

  if (!IpcUtil::ReadIpcStruct(sess, index++, &st_num, &ipc_st)) {
    UPLL_LOG_DEBUG("Failed to read key from the IPC request");
    return PFC_IPCRESP_FATAL;
  }

  ConfigKeyVal *ckv = new ConfigKeyVal((unc_key_type_t)keytype,
                                       st_num, ipc_st, NULL);
  urc = config_mgr_->IsKeyInUse((upll_keytype_datatype_t)datatype,
                                ckv, &in_use);
  delete ckv;

  // Write response
  if ((0 != (ipc_err = sess->addOutput(
                  (uint32_t)UPLL_IS_KEY_TYPE_IN_USE_OP))) ||
      (0 != (ipc_err = sess->addOutput((uint32_t)urc))) ||
      (0 != (ipc_err = sess->addOutput((uint8_t)in_use)))) {
    UPLL_LOG_DEBUG("Unable to write IPC response. Err=%d", ipc_err);
    return PFC_IPCRESP_FATAL;
  }

  return 0;
}

pfc_ipcresp_t UpllConfigSvc::HandleUpplUpdate(
    pfc::core::ipc::ServerSession *sess, int index) {
  UPLL_FUNC_TRACE;
  uint32_t datatype;
  uint32_t operation;
  uint32_t keytype;
  key_ctr_t ctr_key;
  val_ctr_t ctr_val;
  memset(&ctr_key, 0, sizeof(key_ctr_t));
  memset(&ctr_val, 0, sizeof(val_ctr_t));
  int ipc_err;
  upll_rc_t urc = UPLL_RC_SUCCESS;

  if ((0 != (ipc_err = sess->getArgument(index++, datatype))) ||
      (0 != (ipc_err = sess->getArgument(index++, operation))) ||
      (0 != (ipc_err = sess->getArgument(index++, keytype))) ||
      (0 != (ipc_err = sess->getArgument(index++, ctr_key)))) {
    UPLL_LOG_DEBUG("Unable to read field at %u from IPC request. Err=%d",
                   index, ipc_err);
    return PFC_IPCRESP_FATAL;
  }
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    if (0 != (ipc_err = sess->getArgument(index++, ctr_val))) {
      UPLL_LOG_DEBUG("Unable to read field at %u from IPC request. Err=%d",
                     index, ipc_err);
      return PFC_IPCRESP_FATAL;
    }
  }

  if (keytype != UNC_KT_CONTROLLER) {
    urc = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if ((datatype != UPLL_DT_CANDIDATE) && (datatype != UPLL_DT_RUNNING)) {
    urc = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  } else if ((operation != UNC_OP_CREATE) && (operation != UNC_OP_DELETE) &&
             (operation != UNC_OP_UPDATE)) {
    urc = UPLL_RC_ERR_BAD_REQUEST;
  }
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Bad UPPL Update: keytype=%d, datatype=%d operation=%d",
                   keytype, datatype, operation);
    if ((0 != (ipc_err = sess->addOutput((uint32_t)UPLL_UPPL_UPDATE_OP))) ||
        (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
      UPLL_LOG_DEBUG("Unable to write IPC response. Err=%d", ipc_err);
      return PFC_IPCRESP_FATAL;
    }
    return 0;
  }

  UPLL_LOG_TRACE("ctr_val data: ctr_type : [%d, %d], ctr_version : [%d, %s]",
                 (uint32_t)ctr_val.valid[kIdxType], (uint32_t)ctr_val.type,
                 (uint32_t)ctr_val.valid[kIdxVersion], ctr_val.version);

  CtrlrMgr *ctrlr_mgr = CtrlrMgr::GetInstance();
  if (operation == UNC_OP_CREATE) {
    urc = UPLL_RC_SUCCESS;
    if (ctr_val.valid[kIdxType] == UNC_VF_VALID) {
      // For Unknown, version is optional.
      if (((unc_keytype_ctrtype_t)ctr_val.type == UNC_CT_PFC ||
          (unc_keytype_ctrtype_t)ctr_val.type == UNC_CT_VNP || 
          (unc_keytype_ctrtype_t)ctr_val.type == UNC_CT_ODC)) {
        if (ctr_val.valid[kIdxVersion] == UNC_VF_INVALID) {
          urc = UPLL_RC_ERR_CFG_SYNTAX;
        }
      } else if ((unc_keytype_ctrtype_t)ctr_val.type == UNC_CT_UNKNOWN) {
        if (ctr_val.valid[kIdxVersion] == UNC_VF_INVALID) {
          memset(ctr_val.version, 0, sizeof(ctr_val.version));
        }
      } else {
        urc = UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      urc = UPLL_RC_ERR_CFG_SYNTAX;
    }

    if (urc != UPLL_RC_SUCCESS) {
      if ((0 != (ipc_err = sess->addOutput((uint32_t)UPLL_UPPL_UPDATE_OP))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
        UPLL_LOG_DEBUG("Unable to write IPC response. Err=%d", ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }

    CtrlrMgr::Ctrlr ctrlr(reinterpret_cast<char*>(ctr_key.controller_name),
                          (unc_keytype_ctrtype_t)ctr_val.type,
                          reinterpret_cast<char*>(ctr_val.version));
    urc = ctrlr_mgr->Add(ctrlr, (upll_keytype_datatype_t)datatype);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Controller add failed %s",
                     reinterpret_cast<char*>(ctr_key.controller_name));
    }
  } else if (operation == UNC_OP_DELETE) {
    urc = ctrlr_mgr->Delete(reinterpret_cast<char *>(ctr_key.controller_name),
                            (upll_keytype_datatype_t)datatype);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Controller delete failed %s",
                     reinterpret_cast<char*>(ctr_key.controller_name));
    }
  } else if (operation == UNC_OP_UPDATE) {
    if (ctr_val.valid[kIdxVersion] == UNC_VF_VALID) {
      urc = ctrlr_mgr->UpdateVersion(
          reinterpret_cast<char *>(ctr_key.controller_name),
          (upll_keytype_datatype_t)datatype,
          reinterpret_cast<char*>(ctr_val.version));
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Controller update version failed %s, %s",
                       reinterpret_cast<char*>(ctr_key.controller_name),
                       reinterpret_cast<char*>(ctr_val.version));
      }
    }
  }

  // Write response
  if ((0 != (ipc_err = sess->addOutput(
                  (uint32_t)UPLL_IS_KEY_TYPE_IN_USE_OP))) ||
      (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
    UPLL_LOG_DEBUG("Unable to write IPC response. Err=%d", ipc_err);
    return PFC_IPCRESP_FATAL;
  }

  return 0;
}

bool UpllConfigSvc::RegisterForModuleEvents() {
  UPLL_FUNC_TRACE;
  int err;
  pfc_evmask_t mask;

  // Register event handler to receive SYS_START and SYS_STOP events
  pfc_event_mask_empty(&mask);
  pfc_event_mask_add(&mask, PFC_EVTYPE_SYS_START);
  pfc_event_mask_add(&mask, PFC_EVTYPE_SYS_STOP);
  err = pfc_event_add_handler(&sys_evhid_, pfc_event_global_source(),
                              &UpllConfigSvc::HandleSystemEventStatic, this,
                              &mask, kEventPriority);
  if (err != 0) {
    UPLL_LOG_ERROR("Failed to add register for system events. err=%d", err);
    return PFC_FALSE;
  }

  // Register event handler to receive Cluster ACT events
  pfc_event_mask_empty(&mask);
  pfc_event_mask_add(&mask, CLSTAT_EVTYPE_ACT);
  err = pfc_event_add_handler(&cls_evhid_, clstat_event_getsource(),
                              &UpllConfigSvc::HandleClusterEventStatic, this,
                              &mask, kEventPriority);
  if (err != 0) {
    UPLL_LOG_ERROR("Failed to add register for cluster events. err=%d", err);
    UnregisterModuleEventHandlers();
    return PFC_FALSE;
  }

  return PFC_TRUE;
}

void UpllConfigSvc::HandleSystemEventStatic(pfc_event_t event, pfc_ptr_t arg) {
  UPLL_FUNC_TRACE;
  UpllConfigSvc *ucs = reinterpret_cast<UpllConfigSvc *>(arg);
  ucs->HandleSystemEvent(event);
}

void UpllConfigSvc::HandleSystemEvent(pfc_event_t event) {
  UPLL_FUNC_TRACE;
  pfc_evtype_t type = pfc_event_type(event);

  /* Change the status of service. */
  sys_state_rwlock_.wrlock();
  if (type == PFC_EVTYPE_SYS_START) {
    UPLL_LOG_INFO("Received SYS_START.");
    shutting_down_ = false;
  } else if (type == PFC_EVTYPE_SYS_STOP) {
    UPLL_LOG_INFO("Received SYS_STOP.");
    shutting_down_ = true;
  }
  IpcUtil::set_shutting_down(shutting_down_);
  config_mgr_->set_shutting_down(shutting_down_);
  sys_state_rwlock_.unlock();
}

void UpllConfigSvc::HandleClusterEventStatic(pfc_event_t event, pfc_ptr_t arg) {
  UPLL_FUNC_TRACE;
  UpllConfigSvc *ucs = reinterpret_cast<UpllConfigSvc *>(arg);
  ucs->HandleClusterEvent(event);
}

void UpllConfigSvc::HandleClusterEvent(pfc_event_t event) {
  UPLL_FUNC_TRACE;
  pfc_evtype_t type = pfc_event_type(event);

  /* Change the status of service. */
  sys_state_rwlock_.wrlock();
  if (type == CLSTAT_EVTYPE_ACT) {
    UPLL_LOG_INFO("Received cluster event ACT.");
    if (node_active_ == true) {
      UPLL_LOG_WARN("Already in ACTIVE state");
    }
    node_active_ = true;
    RegisterForIpcEvents();
  }
  config_mgr_->SetClusterState(node_active_);
  pfc::alarm::pfc_alarm_clear(UNCCID_LOGICAL);
  sys_state_rwlock_.unlock();
}

bool UpllConfigSvc::RegisterForIpcEvents() {
  UPLL_FUNC_TRACE;

  /* Set Physical module as a target. */
  pfc::core::ipc::IpcEventMask phy_mask;
  phy_mask.empty();
  phy_mask.add(UPPL_ALARMS_PHYS_PATH_FAULT);
  phy_mask.add(UPPL_EVENTS_KT_LOGICAL_PORT);
  phy_mask.add(UPPL_EVENTS_KT_BOUNDARY);
  phy_mask.add(UPPL_EVENTS_KT_CONTROLLER);

  pfc::core::ipc::IpcEventAttr phy_attr;
  phy_attr.addTarget(UPPL_IPC_SVC_NAME, phy_mask);
  phy_attr.setPriority(kIpcEventPriority);

  physical_evhdlr_ = new UpllIpcEventHandler(
      this, UpllIpcEventHandler::IPC_EVENT_SOURCE_PHYSICAL);
  int err = addIpcEventHandler(UPPL_IPC_CHN_NAME, physical_evhdlr_, &phy_attr);
  if (err != 0) {
    UPLL_LOG_ERROR("Failed to add IPC event handler for Physical. err=%d", err);
    return false;
  }

  /* Set PFC Driver module as a target. */
  pfc::core::ipc::IpcEventMask pfc_mask;
  pfc_mask.empty();
  // Mask adjustment needed once PFC code is checkedin
  pfc_mask.add(UNC_ALARMS);

  pfc::core::ipc::IpcEventAttr pfc_attr;
  pfc_attr.addTarget(PFCDRIVER_EVENT_SERVICE_NAME, pfc_mask);
  pfc_attr.setPriority(kIpcEventPriority);

  pfcdriver_evhdlr_ = new UpllIpcEventHandler(
      this, UpllIpcEventHandler::IPC_EVENT_SOURCE_PFCDRIVER);
  err = addIpcEventHandler(PFCDRIVER_CHANNEL_NAME, pfcdriver_evhdlr_,
                           &pfc_attr);
  if (err != 0) {
    UPLL_LOG_ERROR("Failed to add IPC event handler for PFC Driver. err=%d",
                   err);
    return false;
  }

  return true;
}

void UpllConfigSvc::PhysicalEventHandler(const IpcEvent &event) {
  pfc::core::ScopedMutex lock(ipc_event_mutex_);    // One IPC event at a time
  UPLL_FUNC_TRACE;
  int err;
  pfc_ipcevtype_t event_type = event.getType();
  uint32_t operation;
  uint32_t keytype;
  uint32_t datatype;

  pfc::core::ipc::ClientSession sess(event.getSession());

  uint32_t arg_cnt = sess.getResponseCount();
  if (arg_cnt < unc::upll::ipc_util::kPhyConfigNotificationMandatoryFields) {
    UPLL_LOG_DEBUG("Not enough arguments in Physical event. Has only %u.",
                  arg_cnt);
    return;
  }

  uint32_t arg = 0;
  if ((0 != (err = sess.getResponse(arg++, operation))) ||
      (0 != (err = sess.getResponse(arg++, datatype))) ||
      (0 != (err = sess.getResponse(arg++, keytype))) ) {
    UPLL_LOG_DEBUG("Failed to get header field #%u in Physical event. Err=%d",
                   arg, err);
    return;
  }

  switch (datatype) {
    case UNC_DT_STATE:
      break;
    case UNC_DT_CANDIDATE:
    case UNC_DT_RUNNING:
      // Not interested in candidate and running notifications
      return;
    case UNC_DT_STARTUP:
    case UNC_DT_IMPORT:
    default:
      UPLL_LOG_DEBUG("Unexepected datatype %d in the Physical event", datatype);
      return;
  }

  // First handle alarms
  if (event_type == UPPL_ALARMS_PHYS_PATH_FAULT) {
    uint32_t alarm_type;  // Only in the case of alarms
    key_ctr_domain_t kcd;
    val_path_fault_alarm_t pf_alarm;
    if ((0 != (err = sess.getResponse(arg++, alarm_type))) ||
        (0 != (err = sess.getResponse(arg++, kcd))) ||
        (0 != (err = sess.getResponse(arg++, pf_alarm))) ) {
      UPLL_LOG_DEBUG("Failed to get header field #%u in Physical event. Err=%d",
                     arg, err);
      return;
    }
    std::vector<std::string> ingress_ports;
    std::vector<std::string> egress_ports;
    for (uint32_t i = 0; i < pf_alarm.ingress_num_of_ports; i++) {
      const char *port;
      if (0 != (err = sess.getResponse(arg++, port))) {
        UPLL_LOG_DEBUG("Failed to get port at #%u in path fault alarm."
                       " Err=%d", arg, err);
        return;
      }
      ingress_ports.push_back(std::string(port));
    }
    for (uint32_t i = 0; i < pf_alarm.egress_num_of_ports; i++) {
      const char *port;
      if (0 != (err = sess.getResponse(arg++, port))) {
        UPLL_LOG_DEBUG("Failed to get port at #%u in path fault alarm."
                       " Err=%d", arg, err);
        return;
      }
      egress_ports.push_back(std::string(port));
    }
    // Got the path alarm, now process the alarm
    const bool alarm_asserted = (operation == UNC_OP_CREATE) ? true : false;
    config_mgr_->OnPathFaultAlarm(
        reinterpret_cast<char *>(kcd.ctr_key.controller_name),
        reinterpret_cast<char *>(kcd.domain_name),
        ingress_ports, egress_ports, alarm_asserted);
    return;
  }

  // now we handle events
  if (operation == UNC_OP_DELETE) {
    // not to do anything
    return;
  }
  switch (event_type) {
    case UPPL_EVENTS_KT_CONTROLLER:
      {
        key_ctr_t key_ctr;
        val_ctr_st_t new_ctr_st;
        val_ctr_st_t old_ctr_st;
        if ((0 != (err = sess.getResponse(arg++, key_ctr))) ||
            (0 != (err = sess.getResponse(arg++, new_ctr_st))) ) {
          UPLL_LOG_DEBUG("Failed to get field at #%u in Physical event. Err=%d",
                         arg, err);
          return;
        }
        if (operation == UNC_OP_CREATE) {
          // To do nothing?
        } else if (operation == UNC_OP_UPDATE) {
          if (0 != (err = sess.getResponse(arg++, old_ctr_st))) {
            UPLL_LOG_DEBUG("Failed to get field at #%u in Physical event. "
                           "Err=%d", arg, err);
            return;
          }
          if ((old_ctr_st.oper_status == UPPL_CONTROLLER_OPER_UP)  &&
              (new_ctr_st.oper_status != UPPL_CONTROLLER_OPER_UP)) {
            config_mgr_->OnControllerStatusChange(
                            reinterpret_cast<char *>(key_ctr.controller_name),
                            UPPL_CONTROLLER_OPER_DOWN);
          }  else if ((old_ctr_st.oper_status != UPPL_CONTROLLER_OPER_UP)  &&
                     (new_ctr_st.oper_status == UPPL_CONTROLLER_OPER_UP)) {
            config_mgr_->OnControllerStatusChange(
                             reinterpret_cast<char *>(key_ctr.controller_name),
                             UPPL_CONTROLLER_OPER_UP);
          } 
        }
      }
      return;

    case UPPL_EVENTS_KT_LOGICAL_PORT:
      {
        key_logical_port_t key_port;
        val_logical_port_st_t new_port_st;
        val_logical_port_st_t old_port_st;
        if ((0 != (err = sess.getResponse(arg++, key_port))) ||
            (0 != (err = sess.getResponse(arg++, new_port_st))) ) {
          UPLL_LOG_DEBUG("Failed to get field at #%u in Physical event. Err=%d",
                         arg, err);
          return;
        }
        if (new_port_st.oper_status == UPPL_LOGICAL_PORT_OPER_UNKNOWN) {
          return;
        }
        if (operation == UNC_OP_CREATE) {
          // To do nothing?
        } else if (operation == UNC_OP_UPDATE) {
          if (0 != (err = sess.getResponse(arg++, old_port_st))) {
            UPLL_LOG_DEBUG("Failed to get field at #%u in Physical event. "
                           "Err=%d", arg, err);
            return;
          }
          if (new_port_st.oper_status != old_port_st.oper_status) {
            config_mgr_->OnLogicalPortStatusChange(
                reinterpret_cast<char *>(
                    key_port.domain_key.ctr_key.controller_name),
                reinterpret_cast<char *>(key_port.domain_key.domain_name),
                reinterpret_cast<char *>(key_port.port_id),
                new_port_st.oper_status);
          }
        }
      }
      return;

    case UPPL_EVENTS_KT_BOUNDARY:
      {
        key_boundary_t key_bndry;
        val_boundary_st_t new_bndry_st;
        val_boundary_st_t old_bndry_st;
        if ((0 != (err = sess.getResponse(arg++, key_bndry))) ||
            (0 != (err = sess.getResponse(arg++, new_bndry_st))) ) {
          UPLL_LOG_DEBUG("Failed to get field at #%u in Physical event. Err=%d",
                         arg, err);
          return;
        }
        if (new_bndry_st.oper_status == UPPL_BOUNDARY_OPER_UNKNOWN) {
          return;
        }
        if (operation == UNC_OP_CREATE) {
          // To do nothing?
        } else if (operation == UNC_OP_UPDATE) {
          if (0 != (err = sess.getResponse(arg++, old_bndry_st))) {
            UPLL_LOG_DEBUG("Failed to get field at #%u in Physical event. "
                           "Err=%d", arg, err);
            return;
          }
          if (new_bndry_st.oper_status != old_bndry_st.oper_status) {
            config_mgr_->OnBoundaryStatusChange(
                reinterpret_cast<char *>(key_bndry.boundary_id),
                new_bndry_st.oper_status);
          }
        }
      }
      return;
    default:
      UPLL_LOG_DEBUG("Unknown physical event %d", event_type);
      return;
  }

  return;
}

void UpllConfigSvc::PfcDriverAlarmHandler(const IpcEvent &event) {
  pfc::core::ScopedMutex lock(ipc_event_mutex_);    // One IPC event at a time

  UPLL_FUNC_TRACE;
  int err;
  const char *ctrlr_name;
  const char *domain_id;
  uint32_t operation;
  uint32_t keytype;
  uint32_t datatype;
  uint32_t alarm_type;


  pfc::core::ipc::ClientSession sess(event.getSession());

  uint32_t arg_cnt = sess.getResponseCount();
  if (arg_cnt < unc::upll::ipc_util::kPfcDrvierAlarmMandatoryFields) {
    UPLL_LOG_DEBUG("Not enough arguments in the alarm from PFC Driver."
                   " Has only %u.", arg_cnt);
    return;
  }

  uint32_t arg = 0;
  if ((0 != (err = sess.getResponse(arg++, ctrlr_name))) ||
      (0 != (err = sess.getResponse(arg++, domain_id))) ||
      (0 != (err = sess.getResponse(arg++, operation))) ||
      (0 != (err = sess.getResponse(arg++, datatype))) ||
      (0 != (err = sess.getResponse(arg++, keytype))) ||
      (0 != (err = sess.getResponse(arg++, alarm_type)))) {
    UPLL_LOG_DEBUG("Failed to get header field #%u in the alarm from PFCDriver."
                  " Err=%d", arg, err);
    return;
  }
  if (ctrlr_name[0] == 0) {
    UPLL_LOG_DEBUG("Empty controller name in PFC Drvier alarm");
    return;
  }
  if ((operation != UNC_OP_CREATE) && (operation != UNC_OP_DELETE)) {
    UPLL_LOG_DEBUG("Bad operaton %d in PFC Drvier alarm", operation);
    return;
  }
  switch (keytype) {
    case UNC_KT_VTN:
    case UNC_KT_VBR_NWMONITOR:
      break;
    default:
      UPLL_LOG_DEBUG("Unexpected KT %d in PFC Drvier alarm", keytype);
      return;
  }
  if (datatype != UNC_DT_STATE) {
    UPLL_LOG_DEBUG("Bad datatype %d in PFC Drvier alarm", datatype);
    return;
  }
  const bool alarm_asserted = (operation == UNC_OP_CREATE) ? true : false;
  switch (alarm_type) {
    case UNC_POLICER_FULL:
    case UNC_POLICER_FAIL:
      {
        key_vtn key_vtn;
        pfcdrv_policier_alarm_data_t policer_alarm_data;
        if ((0 != (err = sess.getResponse(arg++, key_vtn))) ||
            (0 != (err = sess.getResponse(arg++, policer_alarm_data)))) {
          UPLL_LOG_DEBUG("Failed to get header field #%u in the alarm from "
                         "PFC Driver. Err=%d", arg, err);
          return;
        }
        if (alarm_type == UNC_POLICER_FULL) {
          config_mgr_->OnPolicerFullAlarm(ctrlr_name, domain_id, key_vtn,
                                          policer_alarm_data, alarm_asserted);
        } else {
          config_mgr_->OnPolicerFailAlarm(ctrlr_name, domain_id, key_vtn,
                                          policer_alarm_data, alarm_asserted);
        }
      }
      break;
    case UNC_NWMON_FAULT:
      {
        key_vtn key_vtn;
        pfcdrv_network_mon_alarm_data_t nwmon_alarm_data;
        if ((0 != (err = sess.getResponse(arg++, key_vtn))) ||
            (0 != (err = sess.getResponse(arg++, nwmon_alarm_data)))) {
          UPLL_LOG_DEBUG("Failed to get header field #%u in the alarm from "
                         "PFC Driver. Err=%d", arg, err);
          return;
        }
        config_mgr_->OnNwmonFaultAlarm(ctrlr_name, domain_id, key_vtn,
                                       nwmon_alarm_data, alarm_asserted);
      }
      break;
    default:
      UPLL_LOG_DEBUG("Unknown alarm type %d in PFC Drvier alarm", alarm_type);
      return;
  }
  return;
}

void UpllConfigSvc::UnregisterModuleEventHandlers() {
  int err;
  if (sys_evhid_ != EVHANDLER_ID_INVALID) {
    if (0 != (err = pfc_event_remove_handler(sys_evhid_, NULL))) {
      UPLL_LOG_INFO("Failed to remove system event handler %d", err);
    } else {
      sys_evhid_ = EVHANDLER_ID_INVALID;
    }
  }
  if (cls_evhid_ != EVHANDLER_ID_INVALID) {
    if (0 != (err = pfc_event_remove_handler(cls_evhid_, NULL))) {
      UPLL_LOG_INFO("Failed to remove cluster event handler %d", err);
    } else {
      cls_evhid_ = EVHANDLER_ID_INVALID;
    }
  }
}

void UpllConfigSvc::UnregisterIpcEventHandlers() {
  int err;
  if (physical_evhdlr_ != NULL) {
    if (0 != (err = removeIpcEventHandler(physical_evhdlr_))) {
      UPLL_LOG_INFO("Failed to remove physical event handler %d", err);
    }
    physical_evhdlr_ = NULL;
  }
  if (pfcdriver_evhdlr_ != NULL) {
    if (0 != (err = removeIpcEventHandler(pfcdriver_evhdlr_))) {
      UPLL_LOG_INFO("Failed to remove pfcdriver event handler %d", err);
    }
    pfcdriver_evhdlr_ = NULL;
  }
}
                                                                       // NOLINT
}  // namesapce config_svc
}  // namesapce upll
}  // namesapce unc

/* Declare C++ module. */
PFC_MODULE_IPC_DECL(unc::upll::config_svc::UpllConfigSvc,
                    unc::upll::config_svc::UpllConfigSvc::kNumCfgServices);
