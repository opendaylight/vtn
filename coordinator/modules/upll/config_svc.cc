/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "unc/usess_ipc.h"

#include "unc/upll_svc.h"
#include "uncxx/upll_log.hh"

#include "kt_util.hh"
#include "ctrlr_mgr.hh"
#include "config_svc.hh"

bool fatal_done;
pfc::core::Mutex  fatal_mutex_lock;

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

namespace uuc = unc::upll::ctrlr_events;

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

  event_queue_.Create();

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
  ConfigKeyVal *ckv = NULL;

  if (!IpcUtil::ReadKtRequest(sess, service, &msghdr, &ckv)) {
    UPLL_LOG_INFO("Failed in reading the key tree request");
    return PFC_IPCRESP_FATAL;
  }

  // Handle the case of shutting down or standby node case.
  upll_rc_t urc = config_mgr_->ContinueActiveProcess();
  if (urc == UPLL_RC_ERR_SHUTTING_DOWN) {
    msghdr.result_code = urc;
    if (!IpcUtil::WriteKtResponse(sess, msghdr, ckv)) {
      UPLL_LOG_INFO("Failed to send response to UPLL IPC service user");
      delete ckv;
      return PFC_IPCRESP_FATAL;
    }
    delete ckv;
    return 0;
  } else if (urc == UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY) {
    if (service != UPLL_READ_SVC_ID) {
      msghdr.result_code = urc;
      if (!IpcUtil::WriteKtResponse(sess, msghdr, ckv)) {
        UPLL_LOG_INFO("Failed to send response to UPLL IPC service user");
        delete ckv;
        return PFC_IPCRESP_FATAL;
      }
      delete ckv;
      return 0;
    }
  } else if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Something went wrong.");
    delete ckv;
    return PFC_IPCRESP_FATAL;
  }

  upll_keytype_datatype_t req_dt = msghdr.datatype;

  if (req_dt == UPLL_DT_STARTUP) {
    unc::tclib::TcLibModule *tclib_module = config_mgr_->GetTcLibModule();
    if (tclib_module == NULL) {
      UPLL_LOG_ERROR("Unable to get tclib module");
      return UPLL_RC_ERR_GENERIC;
    }
    // If startup validity is false and read* is requested on startup, read the
    // data from  running
    if (tclib_module->IsStartupConfigValid() == PFC_FALSE) {
      UPLL_LOG_INFO("Read* will be done on running instead of startup");
      msghdr.datatype = UPLL_DT_RUNNING;
    }
  }

  if ((msghdr.operation == UNC_OP_READ ||
       msghdr.operation == UNC_OP_READ_SIBLING_BEGIN ||
       msghdr.operation == UNC_OP_READ_SIBLING) &&
      (msghdr.datatype == UPLL_DT_STATE)) {
    pfc_timespec_t tspec;
    tspec.tv_sec = kIpcTimeoutReadState;
    tspec.tv_nsec = 0;
    sess->setTimeout(&tspec);
    UPLL_LOG_DEBUG("IPC Server Session timeout for Read State set to %d",
                   kIpcTimeoutReadState);
  }

  // Increasing IPC session timeout only for PING operation
  if (msghdr.operation == UNC_OP_CONTROL && msghdr.option2 == UNC_OPT2_PING) {
    pfc_timespec_t ping_tspec;
    ping_tspec.tv_sec = kIpcTimeoutPing;
    ping_tspec.tv_nsec = 0;
    sess->setTimeout(&ping_tspec);
    UPLL_LOG_DEBUG("IPC Server Session timeout for Ping set to %d",
                  kIpcTimeoutPing);
  } else if (ckv->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER) {
    pfc_timespec_t tspec;
    tspec.tv_sec = kIpcTimeoutVtnstation;
    tspec.tv_nsec = 0;
    sess->setTimeout(&tspec);
    UPLL_LOG_DEBUG("IPC Server Session timeout for VTN_STATION_CONTROLLER "
                   "set to %d", kIpcTimeoutVtnstation);
  }

  if (ckv->get_key_type() == UNC_KT_VTN_DATAFLOW) {
    pfc_timespec_t dataflow_tspec;
    dataflow_tspec.tv_sec = kIpcTimeoutDataflow;
    dataflow_tspec.tv_nsec = 0;
    sess->setTimeout(&dataflow_tspec);
    UPLL_LOG_DEBUG("IPC Server Session timeout for data-flow set to %d",
                   kIpcTimeoutDataflow);
    // XXX: store server session object in CKV user_data CKV
    // for data-flow library
    ckv->set_user_data(sess);
  }

  // Set timeout as 10 minutes for candidate operations
  if ((msghdr.datatype == UPLL_DT_CANDIDATE) &&
      (msghdr.operation == UNC_OP_CREATE ||
       msghdr.operation == UNC_OP_UPDATE ||
       msghdr.operation == UNC_OP_DELETE)) {
    pfc_timespec_t tspec;
    tspec.tv_sec = kIpcTimeoutCandidate;
    tspec.tv_nsec = 0;
    sess->setTimeout(&tspec);
    UPLL_LOG_DEBUG("IPC Server Session timeout for candidate operations"
                   " set to %d", kIpcTimeoutCandidate);
  }

  pfc_ipcresp_t ret = config_mgr_->KtServiceHandler(service, &msghdr, ckv);
  if (msghdr.datatype != req_dt) {
    UPLL_LOG_DEBUG("Read* response is sent with original datatype %d", req_dt);
    msghdr.datatype = req_dt;
  }

  if (ret != 0) {
    UPLL_LOG_DEBUG("Failed in processing the key tree request");
    delete ckv;
    return ret;
  }

  if (ckv->get_key_type() == UNC_KT_VTN_DATAFLOW) {
    UPLL_LOG_TRACE("KT:UNC_KT_VTN_DATAFLOW result:%d", msghdr.result_code);
    ckv->set_user_data(NULL);
    if (msghdr.result_code == UPLL_RC_SUCCESS) {
      delete ckv;
      return 0;
    }
  }
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
  static uint32_t import_type = UPLL_IMPORT_TYPE_FULL;
  if (0 != (ipc_err = sess->getArgument(arg++, operation))) {
    UPLL_LOG_DEBUG("Unable to read operation from IPC request. Err=%d",
                  ipc_err);
    return PFC_IPCRESP_FATAL;
  }

  UPLL_LOG_TRACE("Service_id %d and Operation %d", service, operation);

  urc = config_mgr_->ContinueActiveProcess();

  switch (operation) {
    case UPLL_IS_CANDIDATE_DIRTY_OP: {
      bool dirty = true;
      if (urc == UPLL_RC_SUCCESS) {
        sess->setTimeout(NULL);  // Shallow checking also checks DB in somecase
        UPLL_LOG_DEBUG("IPC Server Session timeout for IS_CANDIDATE_DIRTY is"
                       " set to infinite");

        uint32_t session_id, config_id;
        if ((0 != (ipc_err = sess->getArgument(arg++, session_id))) ||
            (0 != (ipc_err = sess->getArgument(arg++, config_id)))) {
          UPLL_LOG_INFO("Unable to read field at %u from IPC request. Err=%d",
                        arg, ipc_err);
          return UPLL_RC_ERR_BAD_REQUEST;
        }

        TcConfigMode cfg_mode = TC_CONFIG_INVALID;
        std::string cfg_vtn_name;
        if (session_id != USESS_ID_TC) {
          urc = config_mgr_->GetConfigMode(session_id, config_id,
                                           &cfg_mode, &cfg_vtn_name);
          if (urc != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Invalid session id  %u and/or config id %u",
                           session_id, config_id);
          }
        } else {
          cfg_mode = TC_CONFIG_GLOBAL;
          cfg_vtn_name = "";
        }

        if (urc == UPLL_RC_SUCCESS) {
          urc = config_mgr_->IsCandidateDirtyShallow(cfg_mode, cfg_vtn_name,
                                                     &dirty);
        }
      }
      // Write response
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc))) ||
          (0 != (ipc_err = sess->addOutput((uint8_t)dirty)))) {
        UPLL_LOG_INFO("Unable to write IPC response. Err=%d", ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }
    break;

    case UPLL_IMPORT_CTRLR_CONFIG_OP: {
      const char *ctrlr_name = NULL;
      uint32_t session_id, config_id;
      if ((0 != (ipc_err = sess->getArgument(arg++, ctrlr_name))) ||
          (0 != (ipc_err = sess->getArgument(arg++, session_id))) ||
          (0 != (ipc_err = sess->getArgument(arg++, config_id))) ||
          (0 != (ipc_err = sess->getArgument(arg++, import_type)))) {
        UPLL_LOG_INFO("Unable to read field at %u from IPC request. Err=%d",
                       arg, ipc_err);
        urc = UPLL_RC_ERR_BAD_REQUEST;
      }
      if (UPLL_IMPORT_TYPE_FULL != import_type &&
          UPLL_IMPORT_TYPE_PARTIAL != import_type) {
        urc = UPLL_RC_ERR_BAD_REQUEST;
      }

      // Validate controller name
      if ((urc == UPLL_RC_SUCCESS) &&
          (ctrlr_name == NULL || strlen(ctrlr_name) == 0 ||
           (strlen(ctrlr_name) > (KtUtil::kCtrlrNameLenWith0-1)))) {
        UPLL_LOG_INFO("controller id wrong");
        if (ctrlr_name != NULL) {
          UPLL_LOG_INFO("controller id is '%s'", ctrlr_name);
        }
        urc = UPLL_RC_ERR_BAD_REQUEST;
      }


      // Start Import
      if (urc == UPLL_RC_SUCCESS) {
        sess->setTimeout(NULL);
        UPLL_LOG_DEBUG(
            "IPC Server Session timeout for Import set to infinite.");
        urc = config_mgr_->StartImport(ctrlr_name, session_id, config_id,
                                       (upll_import_type)import_type);
        UPLL_LOG_TRACE("StartImport: urc=%d, ctrlr_name=%s,"
                       " session_id=%d, config_id=%d",
                       urc, ctrlr_name, session_id, config_id);
      }

      // Write response
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
        UPLL_LOG_INFO("Unable to write IPC response. Err=%d", ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }
    break;

    case UPLL_MERGE_IMPORT_CONFIG_OP: {
      uint32_t session_id, config_id;
      if ((0 != (ipc_err = sess->getArgument(arg++, session_id))) ||
          (0 != (ipc_err = sess->getArgument(arg++, config_id)))) {
        UPLL_LOG_INFO("Unable to read field at %u from IPC request. Err=%d",
                       arg, ipc_err);
        urc = UPLL_RC_ERR_BAD_REQUEST;
      }

      if (urc == UPLL_RC_SUCCESS) {
        sess->setTimeout(NULL);
        UPLL_LOG_DEBUG("IPC Server Session timeout for Merge set to infinite");
        urc = config_mgr_->OnMerge(session_id, config_id,
                                   (upll_import_type)import_type);

        UPLL_LOG_TRACE("Merge: urc=%d, session_id=%d, config_id=%d",
                       urc, session_id, config_id);
      }

      // Write response
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
        UPLL_LOG_INFO("Unable to write IPC response. Err=%d", ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }
    break;

    case UPLL_CLEAR_IMPORT_CONFIG_OP: {
      uint32_t session_id, config_id;
      if ((0 != (ipc_err = sess->getArgument(arg++, session_id))) ||
          (0 != (ipc_err = sess->getArgument(arg++, config_id)))) {
        UPLL_LOG_INFO("Unable to read field at %u from IPC request. Err=%d",
                       arg, ipc_err);
        urc = UPLL_RC_ERR_BAD_REQUEST;
      }
      if (urc == UPLL_RC_SUCCESS) {
        urc = config_mgr_->ClearImport(session_id, config_id, false);
        UPLL_LOG_TRACE("ClearImport: urc=%d, session_id=%d, config_id=%d",
                       urc, session_id, config_id);
      }

      // Write response
      // No need to convert UNC_RC_CTR_DISCONNECTED error code
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
        UPLL_LOG_INFO("Unable to write IPC response. Err=%d", ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }
    break;

    case UPLL_CFG_BATCH_START_OP: {
      uint32_t session_id, config_id;
      if ((0 != (ipc_err = sess->getArgument(arg++, session_id))) ||
          (0 != (ipc_err = sess->getArgument(arg++, config_id)))) {
        UPLL_LOG_INFO("BATCH-START: Unable to read field at %u from"
                      " IPC request. Err=%d", arg, ipc_err);
        urc = UPLL_RC_ERR_BAD_REQUEST;
      }
      if (urc == UPLL_RC_SUCCESS) {
        urc = config_mgr_->OnBatchStart(session_id, config_id);
        UPLL_LOG_TRACE("BATCH-START: urc=%d, session_id=%d, config_id=%d",
                       urc, session_id, config_id);
      }
      // Write response
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
        UPLL_LOG_INFO("BATCH-START: Unable to write IPC response. Err=%d",
                      ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }
    break;

    case UPLL_CFG_BATCH_ALIVE_OP: {
      uint32_t session_id, config_id;
      if ((0 != (ipc_err = sess->getArgument(arg++, session_id))) ||
          (0 != (ipc_err = sess->getArgument(arg++, config_id)))) {
        UPLL_LOG_INFO(
            "BATCH-ALIVE: Unable to read field at %u from IPC request."
            " Err=%d", arg, ipc_err);
        urc = UPLL_RC_ERR_BAD_REQUEST;
      }
      if (urc == UPLL_RC_SUCCESS) {
        urc = config_mgr_->OnBatchAlive(session_id, config_id);
        UPLL_LOG_TRACE("BATCH-ALIVE: urc=%d, session_id=%d, config_id=%d",
                       urc, session_id, config_id);
      }
      // Write response
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
        UPLL_LOG_INFO("BATCH-ALIVE: Unable to write IPC response. Err=%d",
                      ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }
    break;

    case UPLL_CFG_BATCH_END_OP: {
      uint32_t session_id, config_id;
      if ((0 != (ipc_err = sess->getArgument(arg++, session_id))) ||
          (0 != (ipc_err = sess->getArgument(arg++, config_id)))) {
          UPLL_LOG_INFO("BATCH-END: Unable to read field at %u from"
                        " IPC request. Err=%d", arg, ipc_err);
          urc = UPLL_RC_ERR_BAD_REQUEST;
      }
      if (urc == UPLL_RC_SUCCESS) {
        urc = config_mgr_->OnBatchEnd(session_id, config_id, false);
        UPLL_LOG_TRACE("BATCH-END: urc=%d, session_id=%d, config_id=%d",
                       urc, session_id, config_id);
      }
      // Write response
      if ((0 != (ipc_err = sess->addOutput((uint32_t)operation))) ||
          (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
        UPLL_LOG_INFO("BATCH-END: Unable to write IPC response. Err=%d",
                      ipc_err);
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
    UPLL_LOG_INFO("Unable to read datatype from IPC request. Err=%d", ipc_err);
    return PFC_IPCRESP_FATAL;
  }

  if (0 != (ipc_err = sess->getArgument(index++, keytype))) {
    UPLL_LOG_INFO("Unable to read keytype from IPC request. Err=%d", ipc_err);
    return PFC_IPCRESP_FATAL;
  }

  if (keytype != UNC_KT_CONTROLLER && keytype != UNC_KT_BOUNDARY) {
    if ((0 != (ipc_err = sess->addOutput(
                    (uint32_t)UPLL_IS_KEY_TYPE_IN_USE_OP))) ||
        (0 != (ipc_err = sess->addOutput(
                    (uint32_t)UPLL_RC_ERR_NO_SUCH_NAME))) ||
        (0 != (ipc_err = sess->addOutput((uint8_t)in_use)))) {
      UPLL_LOG_INFO("Unable to write IPC response. Err=%d", ipc_err);
      return PFC_IPCRESP_FATAL;
    }
  }

  if (!IpcUtil::ReadIpcStruct(sess, index++, &st_num, &ipc_st)) {
    UPLL_LOG_INFO("Failed to read key from the IPC request");
    return PFC_IPCRESP_FATAL;
  }

  urc = config_mgr_->ContinueActiveProcess();
  if (urc == UPLL_RC_SUCCESS) {
    ConfigKeyVal *ckv = new ConfigKeyVal((unc_key_type_t)keytype,
                                         st_num, ipc_st, NULL);
    urc = config_mgr_->IsKeyInUse((upll_keytype_datatype_t)datatype,
                                  ckv, &in_use);
    delete ckv;
  } else {
      if (ipc_st != NULL)
        ConfigKeyVal::Free(ipc_st);
  }

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

  urc = config_mgr_->ContinueActiveProcess();
  if (urc != UPLL_RC_SUCCESS) {
    return PFC_IPCRESP_FATAL;
  }

  if ((0 != (ipc_err = sess->getArgument(index++, datatype))) ||
      (0 != (ipc_err = sess->getArgument(index++, operation))) ||
      (0 != (ipc_err = sess->getArgument(index++, keytype))) ||
      (0 != (ipc_err = sess->getArgument(index++, ctr_key)))) {
    UPLL_LOG_INFO("Unable to read field at %u from IPC request. Err=%d",
                   index, ipc_err);
    return PFC_IPCRESP_FATAL;
  }
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    if (0 != (ipc_err = sess->getArgument(index++, ctr_val))) {
      UPLL_LOG_INFO("Unable to read field at %u from IPC request. Err=%d",
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
      UPLL_LOG_INFO("Unable to write IPC response. Err=%d", ipc_err);
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
          (unc_keytype_ctrtype_t)ctr_val.type == UNC_CT_POLC ||
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
        UPLL_LOG_INFO("Unable to write IPC response. Err=%d", ipc_err);
        return PFC_IPCRESP_FATAL;
      }
      return 0;
    }

    CtrlrMgr::Ctrlr ctrlr(reinterpret_cast<char*>(ctr_key.controller_name),
                          (unc_keytype_ctrtype_t)ctr_val.type,
                          reinterpret_cast<char*>(ctr_val.version),
                          ctr_val.enable_audit);
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
        UPLL_LOG_DEBUG("Controller update audit type failed %s, %s",
                       reinterpret_cast<char*>(ctr_key.controller_name),
                       reinterpret_cast<char*>(ctr_val.version));
      }
    }
    /*
     * Updat the audit type initial ctrlr creation or
     * after some time
     */
    if (ctr_val.valid[kIdxEnableAudit] == UNC_VF_VALID_NO_VALUE ||
        ctr_val.valid[kIdxEnableAudit] == UNC_VF_VALID) {
      urc = ctrlr_mgr->UpdateAuditType(
          reinterpret_cast<char *>(ctr_key.controller_name),
          (upll_keytype_datatype_t)datatype,
          (ctr_val.enable_audit));
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Controller update audit type failed %s, %d",
                       reinterpret_cast<char*>(ctr_key.controller_name),
                       (ctr_val.enable_audit));
      }
    }
  }

  // Write response
  if ((0 != (ipc_err = sess->addOutput(
                  (uint32_t)UPLL_IS_KEY_TYPE_IN_USE_OP))) ||
      (0 != (ipc_err = sess->addOutput((uint32_t)urc)))) {
    UPLL_LOG_INFO("Unable to write IPC response. Err=%d", ipc_err);
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
  if (type == PFC_EVTYPE_SYS_STOP) {
    UnregisterIpcEventHandlers();
  }
  CtrlrMgr *ctr_mgr = CtrlrMgr::GetInstance();
  ctr_mgr->DeleteFromUnknownCtrlrList("*");
  ctr_mgr->RemoveCtrlrFromIgnoreList("*");
  ctr_mgr->ClearVtnExhaustionMap();
  ctr_mgr->ClearPathfault("*", "*");
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
  bool coldstart = !(clstat_event_isactive(event));
  UPLL_LOG_INFO("clstat_event_isactive is %d", coldstart);
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
  // For ACT, clear all the Alarm at the beginning
  if (node_active_ == true) {
    // Clear all the alarms in the beginning of ACT transition
    UPLL_LOG_INFO("Clear all the alarms during ACT transition");
    pfc::alarm::pfc_alarm_clear(UNCCID_LOGICAL);
  }
  config_mgr_->SetClusterState(node_active_, !coldstart);
  // For SBY, Clear all the Alarm at the at the end of transition
  if (node_active_ == false) {
    UPLL_LOG_INFO("Clear all the alarms during SBY transition");
    pfc::alarm::pfc_alarm_clear(UNCCID_LOGICAL);
  }
  sys_state_rwlock_.unlock();
}

bool UpllConfigSvc::RegisterForIpcEvents() {
  UPLL_FUNC_TRACE;

  /* Set Physical module as a target. */
  pfc::core::ipc::IpcEventMask phy_mask;
  phy_mask.empty();
  phy_mask.add(UPPL_ALARMS_PHYS_PATH_FAULT);
  if (config_mgr_->get_map_phy_resource_status() == true) {
    phy_mask.add(UPPL_EVENTS_KT_LOGICAL_PORT);
    phy_mask.add(UPPL_EVENTS_KT_BOUNDARY);
  }
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
  pfc_mask.add(UNC_UPLL_ALARMS);

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
    // Got the path alarm, now process the alarm
    const bool alarm_asserted = (operation == UNC_OP_CREATE) ? true : false;
    uuc::PathFaultArg *arg_ptr = new uuc::PathFaultArg;
    arg_ptr->event_type_ = uuc::EventArgument::UPPL_PATH_FAULT_ALARM;
    arg_ptr->ctrlr_name_ =
        reinterpret_cast<char *>(kcd.ctr_key.controller_name);
    arg_ptr->domain_name_ = reinterpret_cast<char *>(kcd.domain_name);
    arg_ptr->alarm_raised_ = alarm_asserted;
    event_queue_.AddEvent(arg_ptr);
    return;
  }

  // now we handle events
  switch (event_type) {
    case UPPL_EVENTS_KT_CONTROLLER:
      {
        if (operation == UNC_OP_DELETE) {
          return;  // not to do anything
        }
        key_ctr_t key_ctr;
        val_ctr_st_t new_ctr_st;
        val_ctr_st_t old_ctr_st;
        if ((0 != (err = sess.getResponse(arg++, key_ctr))) ||
            (0 != (err = sess.getResponse(arg++, new_ctr_st))) ) {
          UPLL_LOG_ERROR("Failed to get field at #%u in Physical event. Err=%d",
                         arg, err);
          return;
        }
        if (operation == UNC_OP_CREATE) {
          // To do nothing?
        } else if (operation == UNC_OP_UPDATE) {
          if (0 != (err = sess.getResponse(arg++, old_ctr_st))) {
            UPLL_LOG_ERROR("Failed to get field at #%u in Physical event. "
                           "Err=%d", arg, err);
            return;
          }
          char *ctrlr_name = reinterpret_cast<char *>
                              (key_ctr.controller_name);
          uuc::CtrlrStatusArg *arg_ptr = new uuc::CtrlrStatusArg;
          arg_ptr->event_type_ =
              uuc::EventArgument::UPPL_CTRLR_STATUS_EVENT;
          arg_ptr->ctrlr_name_ = ctrlr_name;
          arg_ptr->operstatus_ = new_ctr_st.oper_status;
          event_queue_.AddEvent(arg_ptr);
        }
      }
      return;

    case UPPL_EVENTS_KT_LOGICAL_PORT: {
        key_logical_port_t key_port;
        val_logical_port_st_t new_port_st;
        val_logical_port_st_t old_port_st;
        if (0 != (err = sess.getResponse(arg++, key_port))) {
            UPLL_LOG_ERROR(
                "Failed to get field at #%u in Physical event. Err=%d",
                arg, err);
            return;
        }
        switch (operation) {
          case UNC_OP_UPDATE: {
            if ((0 != (err = sess.getResponse(arg++, new_port_st))) ||
                (0 != (err = sess.getResponse(arg++, old_port_st)))) {
              UPLL_LOG_ERROR(
                  "Failed to get field at #%u in Physical event. Err=%d",
                 arg, err);
              return;
            }
            if (new_port_st.oper_status == old_port_st.oper_status) {
              UPLL_LOG_DEBUG("No change in port status");
              return;
            }
          }
          break;
          case UNC_OP_CREATE: {
            if (0 != (err = sess.getResponse(arg++, new_port_st))) {
              UPLL_LOG_ERROR(
                  "Failed to get field at #%u in Physical event. Err=%d",
                 arg, err);
              return;
            }
          }
          break;
          case UNC_OP_DELETE:
            new_port_st.oper_status = UPPL_LOGICAL_PORT_OPER_UNKNOWN;
          break;
          default:
            return;
      }
        uuc::LogicalPortArg *arg_ptr = new uuc::LogicalPortArg;
        arg_ptr->event_type_ =
            uuc::EventArgument::UPPL_LOGICAL_PORT_STATUS_EVENT;
        arg_ptr->ctrlr_name_ = reinterpret_cast<char *>(
            key_port.domain_key.ctr_key.controller_name);
        arg_ptr->domain_name_ =
            reinterpret_cast<char *>(key_port.domain_key.domain_name);
        arg_ptr->logical_port_id_ =
            reinterpret_cast<char *>(key_port.port_id);
        arg_ptr->operstatus_ = new_port_st.oper_status;
        event_queue_.AddEvent(arg_ptr);
      return;
    }
    case UPPL_EVENTS_KT_BOUNDARY: {
        key_boundary_t key_bndry;
        val_boundary_st_t new_bndry_st;
        val_boundary_st_t old_bndry_st;
        if ((0 != (err = sess.getResponse(arg++, key_bndry))) ||
            (0 != (err = sess.getResponse(arg++, new_bndry_st))) ) {
          UPLL_LOG_ERROR("Failed to get field at #%u in Physical event. Err=%d",
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
            UPLL_LOG_ERROR("Failed to get field at #%u in Physical event. "
                           "Err=%d", arg, err);
            return;
          }
          if (new_bndry_st.oper_status != old_bndry_st.oper_status) {
            CtrlrMgr *ctr_mgr = CtrlrMgr::GetInstance();
            if (ctr_mgr->IsCtrDisconnected(reinterpret_cast<const char*>(
                        new_bndry_st.boundary.controller_name1))
                || ctr_mgr->IsCtrDisconnected(reinterpret_cast<const char*>(
                        new_bndry_st.boundary.controller_name2))) {
              return;
            }
            uuc::BoundaryStatusArg *arg_ptr = new uuc::BoundaryStatusArg;
            arg_ptr->event_type_ =
                uuc::EventArgument::UPPL_BOUNDARY_STATUS_EVENT;
            arg_ptr->boundary_id_ = reinterpret_cast<char *>(
                key_bndry.boundary_id);
            arg_ptr->operstatus_ = new_bndry_st.oper_status;
            event_queue_.AddEvent(arg_ptr);
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
        uuc::PolicierAlarmArg *arg_ptr = new uuc::PolicierAlarmArg;
        arg_ptr->ctrlr_name_ = ctrlr_name;
        arg_ptr->domain_name_ = domain_id;
        memcpy(reinterpret_cast<void *>(&(arg_ptr->key_vtn_)),
               reinterpret_cast<const void *>(&key_vtn),
               sizeof(key_vtn));
        memcpy(reinterpret_cast<void *>(&(arg_ptr->policier_data_)),
               reinterpret_cast<const void *>(&policer_alarm_data),
               sizeof(pfcdrv_policier_alarm_data_t));
        arg_ptr->alarm_raised_ = alarm_asserted;
        if (alarm_type == UNC_POLICER_FULL) {
          arg_ptr->event_type_ = uuc::EventArgument::PFCDRV_POLICER_FULL_ALARM;
        } else {
          arg_ptr->event_type_ = uuc::EventArgument::PFCDRV_POLICER_FAIL_ALARM;
        }
        event_queue_.AddEvent(arg_ptr);
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
        uuc::NwmonFaultArg *arg_ptr = new uuc::NwmonFaultArg;
        arg_ptr->ctrlr_name_ = ctrlr_name;
        arg_ptr->domain_name_ = domain_id;
        memcpy(reinterpret_cast<void *>(&(arg_ptr->key_vtn_)),
               reinterpret_cast<const void *>(&key_vtn),
               sizeof(key_vtn));
        memcpy(reinterpret_cast<void *>(&(arg_ptr->network_mon_data_)),
               reinterpret_cast<const void *>(&nwmon_alarm_data),
               sizeof(pfcdrv_network_mon_alarm_data_t));
        arg_ptr->alarm_raised_ = alarm_asserted;
        arg_ptr->event_type_ = uuc::EventArgument::PFCDRV_NWMON_FAULT_ALARM;
        event_queue_.AddEvent(arg_ptr);
      }
      break;
    case UNC_VTN_ID_EXHAUSTION:
      {
        if (config_mgr_->get_map_phy_resource_status() == false) {
          UPLL_LOG_TRACE("Mapping flag is disabled - "
                         "not processing VTN_ID_EXHAUSTION alarm");
          return;
        }
        key_vtn key_vtn;
        if (0 != (err = sess.getResponse(arg++, key_vtn))) {
          UPLL_LOG_DEBUG("Failed to get header field #%u in the alarm from "
                         "PFC Driver. Err=%d", arg, err);
          return;
        }
        uuc::VtnIdExhaustionArg *arg_ptr = new uuc::VtnIdExhaustionArg;
        arg_ptr->ctrlr_name_ = ctrlr_name;
        arg_ptr->domain_name_ = domain_id;
        memcpy(reinterpret_cast<void *>(&(arg_ptr->key_vtn_)),
               reinterpret_cast<const void *>(&key_vtn),
               sizeof(key_vtn));
        arg_ptr->alarm_raised_ = alarm_asserted;
        arg_ptr->event_type_ =
                 uuc::EventArgument::PFCDRV_VTNID_EXHAUSTION_ALARM;
        event_queue_.AddEvent(arg_ptr);
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
  UPLL_FUNC_TRACE;
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
  event_queue_.Clear();
}
                                                                       // NOLINT
}  // namespace config_svc
}  // namespace upll
}  // namespace unc

/* Declare C++ module. */
PFC_MODULE_IPC_DECL(unc::upll::config_svc::UpllConfigSvc,
                    unc::upll::config_svc::UpllConfigSvc::kNumCfgServices);
