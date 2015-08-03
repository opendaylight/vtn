/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfc/log.h"
#include "unc/keytype.h"
#include "tclib_intf_impl.hh"
#include "unc/pfcdriver_include.h"
#include "unc/unc_base.h"
#include "kt_util.hh"
#include "ctrlr_mgr.hh"
#include "config_mgr.hh"
#include "ipct_st.hh"
#include "uncxx/upll_log.hh"

namespace unc {
namespace upll {
namespace config_momgr {

using unc::upll::ipc_util::IpctSt;
using unc::upll::ipc_util::KtUtil;

const char * const TcLibIntfImpl::kUpllCtrlrId = "";

TcLibIntfImpl::TcLibIntfImpl(UpllConfigMgr *ucm) {
  ucm_ = ucm;
  session_id_ = 0;
  config_id_ = 0;
  shutting_down_ = false;
  node_active_ = false;
}

TcLibIntfImpl::~TcLibIntfImpl() {
}

// NOLINT
TcCommonRet TcLibIntfImpl::HandleCommitTransactionStart(uint32_t session_id,
                                                      uint32_t config_id,
                                                      TcConfigMode config_mode,
                                                      std::string vtn_name) {
  if (config_mode == TC_CONFIG_REAL) {
    UPLL_LOG_DEBUG("config mode is real mode");
    return unc::tclib::TC_SUCCESS;
  }
  ConfigKeyVal *err_ckv =  NULL;

  TaskScheduler::Task *task = ucm_->fifo_scheduler_->AllowExecution(
                              kCriticalTaskPriority, __FUNCTION__);
  if (!task) {
    UPLL_LOG_FATAL("Failed to add priority task in FIFO scheduler");
    return unc::tclib::TC_FAILURE;
  }
  upll_rc_t urc = ucm_->OnTxStart(session_id, config_id, &err_ckv,
                                 config_mode, vtn_name);
  if (!(ucm_->fifo_scheduler_->DoneExecution(task))) {
    UPLL_LOG_FATAL("Failed to complete the priority task");
    return unc::tclib::TC_FAILURE;
  }

  if (urc != UPLL_RC_SUCCESS) {
    // If local error in UPLL, send it to TC with controller id as ""
    const char *ctrlr_id = kUpllCtrlrId;
    if (err_ckv != NULL) {
      if (err_ckv->get_user_data()) {
        ctrlr_id = reinterpret_cast<char *>(
            (reinterpret_cast<unc::upll::ipc_util::key_user_data*>(
                    err_ckv->get_user_data()))->ctrlr_id);
        if (ctrlr_id == NULL) {
          UPLL_LOG_INFO("Controller Id is null");
          ctrlr_id = kUpllCtrlrId;
        }
        UPLL_LOG_TRACE("Controller Id set in response: %s", ctrlr_id);
      }
    }
    list<CtrlrTxResult*> tx_res_list;
    CtrlrTxResult unc_res(ctrlr_id, urc, urc);
    unc_res.err_ckv = err_ckv;   // err_ckv will be freed by CtlrTxResult
    tx_res_list.push_back(&unc_res);
    if (WriteBackTxResult(tx_res_list) != true) {
      urc = UPLL_RC_ERR_GENERIC;
    }
    if (urc != UPLL_RC_ERR_DRIVER_NOT_PRESENT) {
      // There was an error, let us end tx as TC won't call TxEnd
      ucm_->OnTxEnd(config_mode, vtn_name);
    }
  }
  return ((urc == UPLL_RC_SUCCESS ||
           urc == UPLL_RC_ERR_DRIVER_NOT_PRESENT) ?
           unc::tclib::TC_SUCCESS : unc::tclib::TC_FAILURE);
}

TcCommonRet TcLibIntfImpl::HandleCommitTransactionEnd(
    uint32_t session_id, uint32_t config_id, TcConfigMode config_mode,
    std::string vtn_name, TcTransEndResult end_result) {
  if (config_mode == TC_CONFIG_REAL) {
    UPLL_LOG_DEBUG("config mode is real mode");
    return unc::tclib::TC_SUCCESS;
  }

  UPLL_LOG_TRACE("TxEnd result: %d", end_result);

  TaskScheduler::Task *task = ucm_->fifo_scheduler_->AllowExecution(
                              kCriticalTaskPriority, __FUNCTION__);
  if (!task) {
    UPLL_LOG_FATAL("Failed to add priority task in FIFO scheduler");
    return unc::tclib::TC_FAILURE;
  }
  upll_rc_t urc = ucm_->OnTxEnd(config_mode, vtn_name);
  if (!(ucm_->fifo_scheduler_->DoneExecution(task))) {
    UPLL_LOG_FATAL("Failed to complete the priority task");
    return unc::tclib::TC_FAILURE;
  }

  WriteUpllErrorBlock(&urc);
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}

upll_rc_t TcLibIntfImpl::FillTcDriverInfoMap(
    TcDriverInfoMap *driver_info, const std::set<std::string> *ctrlr_set,
    bool audit) {
  UPLL_FUNC_TRACE;
  PFC_ASSERT(driver_info != NULL);
  PFC_ASSERT(ctrlr_set != NULL);

  std::vector<std::string> openflow_list, /*legacy_list,*/
      overlay_list, polc_list, odc_list;
  int openflow_cnt = 0, /*legacy_cnt = 0,*/
      overlay_cnt = 0, polc_cnt = 0, odc_cnt = 0;

  for (std::set<std::string>::iterator ctr_it = ctrlr_set->begin();
       ctr_it != ctrlr_set->end(); ++ctr_it) {
    const std::string ctrlr_name = *ctr_it;
    unc_keytype_ctrtype_t ctrlr_type;
    upll_keytype_datatype_t datatype;
    datatype = audit ? UPLL_DT_RUNNING : UPLL_DT_CANDIDATE;
    if (false == CtrlrMgr::GetInstance()->GetCtrlrType(
        ctrlr_name.c_str(), datatype, &ctrlr_type)) {
      if (datatype != UPLL_DT_RUNNING) {
        if (false == CtrlrMgr::GetInstance()->GetCtrlrType(
            ctrlr_name.c_str(), UPLL_DT_RUNNING, &ctrlr_type)) {
          UPLL_LOG_WARN("Unable to get controller type for %s",
                        ctrlr_name.c_str());
          return UPLL_RC_ERR_GENERIC;
        }
      } else {
        UPLL_LOG_WARN("Unable to get controller type for %s",
                      ctrlr_name.c_str());
        return UPLL_RC_ERR_GENERIC;
      }
    }
    UPLL_LOG_TRACE("Affected controller name=%s, type=%d",
                  ctrlr_name.c_str(), ctrlr_type);
    if (ctrlr_type == UNC_CT_PFC) {
      openflow_cnt++;
      openflow_list.push_back(ctrlr_name);
    /*
    } else if (ctrlr_type == UNC_CT_LEGACY) {
      legacy_cnt++;
      legacy_list.push_back(ctrlr_name);
    */
    } else if (ctrlr_type == UNC_CT_VNP) {
      overlay_cnt++;
      overlay_list.push_back(ctrlr_name);
    } else if (ctrlr_type == UNC_CT_POLC) {
      polc_cnt++;
      polc_list.push_back(ctrlr_name);
    } else if (ctrlr_type == UNC_CT_ODC) {
      odc_cnt++;
      odc_list.push_back(ctrlr_name);
    } else {
      UPLL_LOG_WARN("Unknown controller type in %d", ctrlr_type);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  if (openflow_cnt) {
    (*driver_info)[UNC_CT_PFC] = openflow_list;
  }
  /*
  if (legacy_cnt) {
    (*driver_info)[UNC_CT_LEGACY] = legacy_list;
  }
  */
  if (overlay_cnt) {
    (*driver_info)[UNC_CT_VNP] = overlay_list;
  }
  if (polc_cnt) {
    (*driver_info)[UNC_CT_POLC] = polc_list;
  }
  if (odc_cnt) {
    (*driver_info)[UNC_CT_ODC] = odc_list;
  }
  return UPLL_RC_SUCCESS;
}

TcCommonRet TcLibIntfImpl::HandleCommitVoteRequest(
    uint32_t session_id, uint32_t config_id,
    TcConfigMode config_mode, std::string vtn_name,
    TcDriverInfoMap& driver_info) {
  if (config_mode == TC_CONFIG_REAL) {
    UPLL_LOG_DEBUG("config mode is real mode");
    return unc::tclib::TC_SUCCESS;
  }
  const std::set<std::string> *affected_ctrlr_list = NULL;
  ConfigKeyVal *err_ckv =  NULL;

  TaskScheduler::Task *task = ucm_->fifo_scheduler_->AllowExecution(
                              kCriticalTaskPriority, __FUNCTION__);
  if (!task) {
    UPLL_LOG_FATAL("Failed to add priority task in FIFO scheduler");
    return unc::tclib::TC_FAILURE;
  }
  upll_rc_t urc = ucm_->OnTxVote(&affected_ctrlr_list, config_mode,
                                 vtn_name, &err_ckv);
  if (!(ucm_->fifo_scheduler_->DoneExecution(task))) {
    UPLL_LOG_FATAL("Failed to complete the priority task");
    return unc::tclib::TC_FAILURE;
  }

  if (urc == UPLL_RC_SUCCESS) {
    urc = FillTcDriverInfoMap(&driver_info, affected_ctrlr_list, false);
  } else {
    // Local error in UPLL, send it to TC with controller id as ""
    list<CtrlrTxResult*> tx_res_list;
    CtrlrTxResult unc_res(kUpllCtrlrId, urc, urc);
    unc_res.err_ckv = err_ckv;   // err_ckv will be freed by CtlrTxResult
    tx_res_list.push_back(&unc_res);
    if (WriteBackTxResult(tx_res_list) != true) {
      urc = UPLL_RC_ERR_GENERIC;
    }
  }
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}

TcCommonRet TcLibIntfImpl::HandleCommitGlobalCommit(
    uint32_t session_id, uint32_t config_id,
    TcConfigMode config_mode, std::string vtn_name,
    TcDriverInfoMap& driver_info) {
  if (config_mode == TC_CONFIG_REAL) {
    UPLL_LOG_DEBUG("config mode is real mode");
    return unc::tclib::TC_SUCCESS;
  }
  // lock event handling
  ucm_->LockAlarmEvent();

  const std::set<std::string> *affected_ctrlr_list = NULL;

  TaskScheduler::Task *task = ucm_->fifo_scheduler_->AllowExecution(
                              kCriticalTaskPriority, __FUNCTION__);
  if (!task) {
    UPLL_LOG_FATAL("Failed to add priority task in FIFO scheduler");
    return unc::tclib::TC_FAILURE;
  }
  upll_rc_t urc = ucm_->OnTxGlobalCommit(&affected_ctrlr_list,
                                         config_mode,
                                         vtn_name);
  if (!(ucm_->fifo_scheduler_->DoneExecution(task))) {
    UPLL_LOG_FATAL("Failed to complete the priority task");
    return unc::tclib::TC_FAILURE;
  }

  if (urc == UPLL_RC_SUCCESS) {
    urc = FillTcDriverInfoMap(&driver_info, affected_ctrlr_list, false);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_FATAL("Failed to fill TcDriverInfoMap in Commit-GlobalCommit");
    }
  }
  WriteUpllErrorBlock(&urc);
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}

/**
 * Given the key_list, reads the key-val information from the TcLib and returns
 * it in the form ConfigKeyVal. On failure frees all the allocated memory.
 */
bool TcLibIntfImpl::GetTxKtResult(const string &ctrlr_id,
                                  const std::list<uint32_t> *key_list,
                                  ConfigKeyVal **err_ckv) {
  UPLL_FUNC_TRACE;
  PFC_ASSERT(key_list != NULL);
  PFC_ASSERT(err_ckv != NULL);
  UpllConfigMgr *ucm = UpllConfigMgr::GetUpllConfigMgr();
  PFC_ASSERT(ucm != NULL);
  const KeyTree config_keytree = ucm->GetConfigKeyTree();

  *err_ckv = NULL;

  unc::tclib::TcLibModule *tclib = UpllConfigMgr::GetTcLibModule();
  if (tclib == NULL) {
    UPLL_LOG_ERROR("Unable to get tclib module");
    return false;
  }

  int err_pos = 0;
  for (std::list<uint32_t>::const_iterator key_it = key_list->begin();
       key_it != key_list->end();
       ++key_it, ++err_pos) {
    unc_key_type_t keytype = (unc_key_type_t)(*key_it);
    UPLL_LOG_TRACE("err_pos:%d KeyType: %u", err_pos, keytype);

    if (!config_keytree.IsValidKeyType(keytype)) {
      // Not in config_keytree, so the key type is not managed by Logical
      UPLL_LOG_TRACE("KeyType %u is not known to UPLL", keytype);
      continue;
    }
    // Key type managed by logical found.
    const std::vector<IpctSt::IpcStructNum>&msg_tmpl =
        KtUtil::GetKtUtil()->GetCfgMsgTemplate(keytype);
    // For some operations like CREATE/DELETE there won't be val structure
    // returned.
    IpctSt::IpcStructNum key_stnum = IpctSt::kIpcInvalidStNum;
    IpctSt::IpcStructNum val_stnum = IpctSt::kIpcInvalidStNum;
    bool val_exists = false;
    switch (msg_tmpl.size()) {
      case 2:
        val_exists = true;
        val_stnum = msg_tmpl[1];
        // fall through
      case 1:
        key_stnum = msg_tmpl[0];
        break;
      default:
        if (*err_ckv != NULL) {
          delete *err_ckv;
          *err_ckv = NULL;
        }
        return false;
    }

    // Read Key and Value from the Tclib Module.
    const pfc_ipcstdef_t *key_stdef = IpctSt::GetIpcStdef(key_stnum);
    // Need a dummy for val_stdef;
    pfc_ipcstdef_t val_stdef_dummy;
    const pfc_ipcstdef_t *val_stdef;
    if (val_exists) {
      val_stdef = IpctSt::GetIpcStdef(val_stnum);
    } else {
        bzero(&val_stdef_dummy, sizeof(val_stdef_dummy));
        val_stdef = &val_stdef_dummy;
    }
    if (key_stdef == NULL || (val_exists && (val_stdef == NULL))) {
      UPLL_LOG_INFO("key_stnum=%d key_stdef=%p val_stnum=%d val_stdef=%p",
                    key_stnum, key_stdef, val_stnum, val_stdef);
      if (*err_ckv != NULL) {
        delete *err_ckv;
        *err_ckv = NULL;
      }
      return false;
    }

    void *key = ConfigKeyVal::Malloc(key_stdef->ist_size);
    void *val = (val_exists) ? ConfigKeyVal::Malloc(val_stdef->ist_size) : NULL;
    if (key == NULL || (val_exists && (val == NULL))) {
      UPLL_LOG_ERROR("Failed to allocate key=%p val=%p", key, val);
      if (key != NULL) ConfigKeyVal::Free(key);
      if (val != NULL) ConfigKeyVal::Free(val);
      if (*err_ckv != NULL) {
        delete *err_ckv;
        *err_ckv = NULL;
      }
      return false;
    }
    unc::tclib::TcApiCommonRet tclib_ret = tclib->TcLibReadKeyValueDataInfo(
        ctrlr_id, err_pos, keytype, *key_stdef, *val_stdef, key, val);
    if (tclib_ret != unc::tclib::TC_API_COMMON_SUCCESS) {
      UPLL_LOG_ERROR("Failed to read from tclib %d", tclib_ret);
      ConfigKeyVal::Free(key);
      if (val != NULL) {
        ConfigKeyVal::Free(val);
      }
      if (*err_ckv != NULL) {
        delete *err_ckv;
        *err_ckv = NULL;
      }
      return false;
    }
    ConfigKeyVal *ckv = new ConfigKeyVal(keytype, key_stnum, key);
    if (val_exists) {
      ckv->AppendCfgVal(new ConfigVal(val_stnum, val));
    }
    if (*err_ckv == NULL) {
      *err_ckv = ckv;
    } else {
      (*err_ckv)->AppendCfgKeyVal(ckv);
    }
  }
  return true;
}

upll_rc_t TcLibIntfImpl::DriverResultCodeToTxURC(
    unc_keytype_ctrtype_t ctrlr_type, int driver_result_code) {
  switch (ctrlr_type) {
    case UNC_CT_PFC:
    case UNC_CT_VNP:
    case UNC_CT_ODC:
    case UNC_CT_POLC:
      {
        switch (driver_result_code) {
          case UNC_RC_SUCCESS:
            return UPLL_RC_SUCCESS;
          case UNC_DRV_RC_DAEMON_INACTIVE:
            return UPLL_RC_ERR_GENERIC;
          case UNC_RC_CTR_DISCONNECTED:
          case UNC_RC_ERR_DRIVER_NOT_PRESENT:  // To update CS as not_applied
            return UPLL_RC_ERR_CTR_DISCONNECTED;
          case UNC_DRV_RC_ERR_ATTRIBUTE_SYNTAX:
            return UPLL_RC_ERR_CFG_SYNTAX;
          case UNC_DRV_RC_ERR_ATTRIBUTE_SEMANTIC:
            return UPLL_RC_ERR_CFG_SEMANTIC;
          case UNC_RC_REQ_NOT_SENT_TO_CTR:
            // It will not be sent to controller only in the case of
            // vote phase For now map it to SUCCESS as we are looking for
            // errors here in vote phase.
            return UPLL_RC_SUCCESS;
          case UNC_RC_UNSUPPORTED_CTRL_CONFIG:
            return static_cast<upll_rc_t>(driver_result_code);
          case UNC_DRV_RC_ERR_GENERIC:
          case UNC_RC_CTRLAPI_FAILURE:
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
          case UNC_RC_NO_SUCH_INSTANCE:
          default:
            return UPLL_RC_ERR_GENERIC;
        }
      }
      break;
    default:
      UPLL_LOG_INFO("Unknown controller type."
                    " Error %d converted to UPLL_RC_ERR_GENERIC",
                    driver_result_code);
      return UPLL_RC_ERR_GENERIC;
  }
}

static UncRespCode ConvertToTcErrorCode(uint32_t ctr_err_code) {
  switch (ctr_err_code) {
    case UNC_RC_SUCCESS:
      return UNC_RC_SUCCESS;

    case UNC_UPLL_RC_ERR_GENERIC:
    case UNC_UPLL_RC_ERR_DB_ACCESS:
           // If ctrlr is disconnected then it is success.
    case UNC_UPLL_RC_ERR_RESOURCE_DISCONNECTED:
      return UNC_RC_INTERNAL_ERR;
    case UNC_UPLL_RC_ERR_BAD_REQUEST:
    case UNC_UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID:
    case UNC_UPLL_RC_ERR_NO_SUCH_OPERATION:
    case UNC_UPLL_RC_ERR_INVALID_OPTION1:
    case UNC_UPLL_RC_ERR_INVALID_OPTION2:
    case UNC_UPLL_RC_ERR_NO_SUCH_NAME:
    case UNC_UPLL_RC_ERR_NO_SUCH_DATATYPE:
    case UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY:
    case UNC_UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT:
    case UNC_UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT:
    case UNC_UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME:
    case UNC_UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT:
    case UNC_UPLL_RC_ERR_MERGE_CONFLICT:
    case UNC_UPLL_RC_ERR_CANDIDATE_IS_DIRTY:
    case UNC_UPLL_RC_ERR_SHUTTING_DOWN:
      UPLL_LOG_DEBUG("Error code %d is unexpected", ctr_err_code);
      return UNC_RC_INTERNAL_ERR;

    case UNC_UPLL_RC_ERR_CFG_SYNTAX:
    case UNC_UPLL_RC_ERR_CFG_SEMANTIC:
    case UNC_UPLL_RC_ERR_NO_SUCH_INSTANCE:
    case UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR:
    case UNC_UPLL_RC_ERR_PARENT_DOES_NOT_EXIST:
    case UNC_UPLL_RC_ERR_INSTANCE_EXISTS:
      return UNC_RC_CONFIG_INVAL;

      /* Transaction errors */
    case UNC_RC_INTERNAL_ERR:
    case UNC_RC_CONFIG_INVAL:
    case UNC_RC_CTRLAPI_FAILURE:
    case UNC_RC_UNSUPPORTED_CTRL_CONFIG:
    case UNC_RC_ERR_DRIVER_NOT_PRESENT:
    case UNC_RC_CTR_CONFIG_STATUS_ERR:
    case UNC_RC_CTR_BUSY:
    case UNC_RC_CTR_DISCONNECTED:
    case UNC_RC_REQ_NOT_SENT_TO_CTR:
    case UNC_RC_NO_SUCH_INSTANCE:
    case UNC_RC_REQUEST_CANCELLED:  // expected only in the case of audit
      return static_cast<UncRespCode>(ctr_err_code);

    default:
      UPLL_LOG_DEBUG("Error code %d is unexpected", ctr_err_code);
      return UNC_RC_INTERNAL_ERR;
  }
}

/**
 * Driver result is converted to UPLL specific format.
 * On error (false return), tx_res_list will be empty.
 * NOTE: Only controllers with failures are returned.
 */
bool TcLibIntfImpl::GetTxResult(const TcCommitPhaseResult *driver_result,
                                list<CtrlrTxResult*> *tx_res_list) {
  UPLL_FUNC_TRACE;
  PFC_ASSERT(driver_result != NULL);
  PFC_ASSERT(tx_res_list != NULL);

  UPLL_LOG_INFO("Controller count=%u",
                static_cast<uint32_t>(driver_result->size()));

  // Iterate over each controller status
  for (TcCommitPhaseResult::const_iterator ctr_it = driver_result->begin();
       ctr_it != driver_result->end(); ++ctr_it) {
    unc_keytype_ctrtype_t ctrlr_type = UNC_CT_UNKNOWN;
    if (false == CtrlrMgr::GetInstance()->GetCtrlrType(
        ctr_it->controller_id.c_str(), UPLL_DT_CANDIDATE, &ctrlr_type)) {
      if (false == CtrlrMgr::GetInstance()->GetCtrlrType(
          ctr_it->controller_id.c_str(), UPLL_DT_RUNNING, &ctrlr_type)) {
        UPLL_LOG_INFO("Controller %s does not exist",
                       ctr_it->controller_id.c_str());
        // empty tx_res_list and free its contents
        for (list<CtrlrTxResult*>::iterator tx_res_it = tx_res_list->begin();
            tx_res_it != tx_res_list->end(); tx_res_it++) {
          CtrlrTxResult *tx_res = *tx_res_it;
          delete tx_res;
        }
        tx_res_list->clear();
        return false;
      }
    }
    /*
    if (ctrlr_type == UNC_CT_PFC) {
      // TODO(a) NOT_SENT error will be used only in Global Commit Phase
      if (ctr_it->resp_code == UNC_RC_REQ_NOT_SENT_TO_CTR)
        continue;  // skip this controller
    } else {
      continue;  // Not implemeted other controller types
    }
    */
    upll_rc_t converted_result = DriverResultCodeToTxURC(ctrlr_type,
                                                         ctr_it->resp_code);
    /*
    if (converted_result != UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
      if (ctr_it->num_of_errors == 0) {
        UPLL_LOG_TRACE("Skipping Controller ID=%s CtrResult=%d",
                  ctr_it->controller_id.c_str(), ctr_it->resp_code);
        continue;
      }
    }
    */

    UPLL_LOG_INFO("Controller ID=%s CtrlrResult=%d Urc=%d ErrCount=%d",
                  ctr_it->controller_id.c_str(), ctr_it->resp_code,
                  converted_result, ctr_it->num_of_errors);

    CtrlrTxResult *tx_res = new CtrlrTxResult(ctr_it->controller_id,
                                              converted_result,
                                              ctr_it->resp_code);
    tx_res->err_ckv = NULL;

    if (GetTxKtResult(tx_res->ctrlr_id, &ctr_it->key_list, &tx_res->err_ckv) ==
        false) {
      // empty tx_res_list and free its contents
      for (list<CtrlrTxResult*>::iterator tx_res_it = tx_res_list->begin();
           tx_res_it != tx_res_list->end(); tx_res_it++) {
        CtrlrTxResult *tx_res = *tx_res_it;
        delete tx_res;
      }
      tx_res_list->clear();
      delete tx_res;
      return false;
    }
    if (tx_res->err_ckv != NULL) {
      UPLL_LOG_INFO("Error CKV received from controller <%s>: %s",
                     ctr_it->controller_id.c_str(),
                     tx_res->err_ckv->ToStrAll().c_str());
    }
    tx_res_list->push_back(tx_res);
    /*
    if (tx_res->err_ckv->size() != 0) {
      tx_res_list->push_back(tx_res);
    } else {
      if (converted_result == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
        tx_res_list->push_back(tx_res);
      }
    }
    */
  }
  return true;
}

bool TcLibIntfImpl::WriteBackTxResult(const list<CtrlrTxResult*> &tx_res_list) {
  UPLL_FUNC_TRACE;

  unc::tclib::TcLibModule *tclib = UpllConfigMgr::GetTcLibModule();
  if (tclib == NULL) {
    UPLL_LOG_ERROR("Unable to get tclib module");
    return false;
  }
  for (std::list<CtrlrTxResult*>::const_iterator ctrlr_it = tx_res_list.begin();
       ctrlr_it != tx_res_list.end(); ctrlr_it++) {
    const CtrlrTxResult *res = *ctrlr_it;
    // Skip controllers whose status is success.
    // During Vote/G-Commit, disconnected controller information is not skipped
    // During Vote, UNC_RC_REQ_NOT_SENT_TO_CTR is not skipped.
    if ((res->ctrlr_id.compare(kUpllCtrlrId) != 0) &&
        (res->ctrlr_orig_result == UNC_RC_SUCCESS)) {
      continue;
    }
    UncRespCode unc_rc = ConvertToTcErrorCode(res->ctrlr_orig_result);
    UPLL_LOG_INFO("Sending error %d to TC", unc_rc);
    if (unc_rc == UNC_RC_CTR_BUSY) {
      // If error is UNC_RC_CTR_BUSY, there is no keytype error. So, err_ckv
      // should not be used.
      unc::tclib::TcApiCommonRet tclib_ret = tclib->TcLibWriteControllerInfo(
          res->ctrlr_id, unc_rc, 0);
      if (tclib_ret != unc::tclib::TC_API_COMMON_SUCCESS) {
        UPLL_LOG_ERROR("Failed to write to tclib %d", tclib_ret);
        return false;
      }
      continue;
    }
    unc::tclib::TcApiCommonRet tclib_ret = tclib->TcLibWriteControllerInfo(
        res->ctrlr_id, unc_rc, ((res->err_ckv)?res->err_ckv->size():0));
    if (tclib_ret != unc::tclib::TC_API_COMMON_SUCCESS) {
      UPLL_LOG_ERROR("Failed to write to tclib %d", tclib_ret);
      return false;
    }
    if (res->err_ckv) {
      UPLL_LOG_INFO("Proccessed Error CKV for controller <%s>: %s",
                    res->ctrlr_id.c_str(), res->err_ckv->ToStrAll().c_str());
      for (const ConfigKeyVal *ckv = res->err_ckv; ckv;
           ckv = ckv->get_next_cfg_key_val()) {
        if (ckv->get_key() == NULL) {
          UPLL_LOG_INFO("Bad ConfigKeyVal key is NULL");
          return false;
        }
        const pfc_ipcstdef_t *key_stdef =
            IpctSt::GetIpcStdef(ckv->get_st_num());
        if (key_stdef == NULL) {
          UPLL_LOG_INFO("Unknown ConfigKeyVal key=%d", ckv->get_st_num());
          return false;
        }
        const pfc_ipcstdef_t *val_stdef = NULL;
        // Need a dummy for val_stdef;
        pfc_ipcstdef_t val_stdef_dummy;
        const ConfigVal *cv = ckv->get_cfg_val();
        if (cv != NULL) {
          if (cv->get_val() == NULL) {
            UPLL_LOG_INFO("Bad ConfigKeyVal val is NULL");
            return false;
          }
          val_stdef = IpctSt::GetIpcStdef(cv->get_st_num());
          if (val_stdef == NULL) {
            UPLL_LOG_INFO("Unknown ConfigKeyVal val=%d", cv->get_st_num());
            return false;
          }
        } else {
          bzero(&val_stdef_dummy, sizeof(val_stdef_dummy));
          val_stdef = &val_stdef_dummy;
        }
        unc::tclib::TcApiCommonRet tclib_ret =
            tclib->TcLibWriteKeyValueDataInfo(
            res->ctrlr_id, ckv->get_key_type(), *key_stdef, *val_stdef,
            ckv->get_key(), ((cv) ? cv->get_val() : NULL));
        if (tclib_ret != unc::tclib::TC_API_COMMON_SUCCESS) {
          UPLL_LOG_ERROR("Failed to write to tclib %d", tclib_ret);
          return false;
        }
      }
    }
  }
  return true;
}

/**
 * This function is used by both regular commit and audit commit.
 */
TcCommonRet TcLibIntfImpl::HandleCommonTxDriverResult(
    uint32_t session_id, uint32_t config_id,
    TcConfigMode config_mode, std::string vtn_name,
    TcCommitPhaseType tx_phase, TcCommitPhaseResult driver_result) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("Commit phase: %d", tx_phase);

  // Convert TC format to upll format
  list<CtrlrTxResult*> tx_res_list;
  if (GetTxResult(&driver_result, &tx_res_list) == false) {
    UPLL_LOG_DEBUG("Failed to convert driver result");
    PFC_ASSERT(tx_res_list.empty() == true);
    upll_rc_t urc = UPLL_RC_ERR_GENERIC;
    WriteUpllErrorBlock(&urc);
    return unc::tclib::TC_FAILURE;
  }


  // Now call tx_mgr's commit handler
  upll_rc_t urc;
  switch (tx_phase) {
    case unc::tclib::TC_COMMIT_VOTE_PHASE:
      urc = ucm_->OnTxVoteCtrlrStatus(&tx_res_list,
                                      config_mode,
                                      vtn_name);
      break;
    case unc::tclib::TC_COMMIT_GLOBAL_COMMIT_PHASE:
      urc = ucm_->OnTxCommitCtrlrStatus(session_id, config_id, &tx_res_list,
                                        config_mode, vtn_name);
      break;
    case unc::tclib::TC_AUDIT_VOTE_PHASE:
      if (tx_res_list.size() != 1) {
        // Some odd case in TC design
        UPLL_LOG_DEBUG("In audit vote result found %u controllers",
                       static_cast<uint32_t>(tx_res_list.size()));
        urc = UPLL_RC_SUCCESS;
      } else {
        urc = ucm_->OnAuditTxVoteCtrlrStatus(tx_res_list.front());
      }
      break;
    case unc::tclib::TC_AUDIT_GLOBAL_COMMIT_PHASE:
      if (tx_res_list.size() != 1) {
        // Some odd case in TC design
        UPLL_LOG_DEBUG("In audit commit result found %u controllers",
                       static_cast<uint32_t>(tx_res_list.size()));
        urc = UPLL_RC_SUCCESS;
      } else {
        urc = ucm_->OnAuditTxCommitCtrlrStatus(tx_res_list.front());
      }
      break;
    default:
      urc = UPLL_RC_ERR_BAD_REQUEST;
  }

  TcCommonRet tcr;
  if (urc == UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("ConfigMgr successfully processed the commit result");
    // Now write UPLL result back to TCLIB
    tcr = ((WriteBackTxResult(tx_res_list) == true) ?  unc::tclib::TC_SUCCESS :
           unc::tclib::TC_FAILURE);
  } else if (urc == UPLL_RC_ERR_AUDIT_CANCELLED &&
             tx_phase == unc::tclib::TC_AUDIT_VOTE_PHASE) {
    tcr = unc::tclib::TC_CANCELLED_AUDIT;
  } else {
    UPLL_LOG_TRACE("ConfigMgr failed to process the commit result %d", urc);
    WriteUpllErrorBlock(&urc);
    tcr = unc::tclib::TC_FAILURE;
  }
  // free tx_res_list and also compute final TC result;
  for (list<CtrlrTxResult*>::iterator tx_res_it = tx_res_list.begin();
       tx_res_it != tx_res_list.end(); tx_res_it++) {
    CtrlrTxResult *tx_res = *tx_res_it;
    UPLL_LOG_TRACE("Driver received error %d from controller %s",
                   tx_res->ctrlr_orig_result, tx_res->ctrlr_id.c_str());
    // For Vote phase, tcr will be failure if controller result is failure.
    // For Global-Commit phase, tcr is not dependent on the controller result.
    if (tcr == unc::tclib::TC_SUCCESS) {
      if (tx_phase == unc::tclib::TC_COMMIT_VOTE_PHASE) {
        if ((tx_res->ctrlr_orig_result != UNC_RC_SUCCESS) &&
            (tx_res->ctrlr_orig_result != UNC_RC_CTR_DISCONNECTED) &&
            (tx_res->ctrlr_orig_result != UNC_RC_ERR_DRIVER_NOT_PRESENT)) {
          /* Assumption: If err is UNC_RC_REQ_NOT_SENT_TO_CTR, then
           * atleast one controller failed the vote. */
          tcr = unc::tclib::TC_FAILURE;
        }
      } else if (tx_phase == unc::tclib::TC_AUDIT_VOTE_PHASE) {
        if (tx_res->ctrlr_orig_result != UNC_RC_SUCCESS) {
          /* Note: If err is UNC_RC_CTR_DISCONNECTED, then
           * Audit for the controller is incomplete, so tcr is failure. */
          tcr = unc::tclib::TC_FAILURE;
        }
      }
    }
    delete tx_res;
  }
  tx_res_list.clear();
  return tcr;
}

TcCommonRet TcLibIntfImpl::HandleCommitDriverResult(
    uint32_t session_id, uint32_t config_id,
    TcConfigMode config_mode, std::string vtn_name,
    TcCommitPhaseType tx_phase, TcCommitPhaseResult driver_result) {
  if (config_mode == TC_CONFIG_REAL) {
    UPLL_LOG_DEBUG("config mode is real mode");
    return unc::tclib::TC_SUCCESS;
  }

  TaskScheduler::Task *task = ucm_->fifo_scheduler_->AllowExecution(
                              kCriticalTaskPriority, __FUNCTION__);
  if (!task) {
    UPLL_LOG_FATAL("Failed to add priority task in FIFO scheduler");
    return unc::tclib::TC_FAILURE;
  }
  TcCommonRet tcr = HandleCommonTxDriverResult(session_id, config_id,
                                               config_mode, vtn_name,
                                               tx_phase, driver_result);
  if (!(ucm_->fifo_scheduler_->DoneExecution(task))) {
    UPLL_LOG_FATAL("Failed to complete the priority task");
    return unc::tclib::TC_FAILURE;
  }

  // unlock event handling
  ucm_->UnlockAlarmEvent();
  return tcr;
}

TcCommonRet TcLibIntfImpl::HandleCommitGlobalAbort(
    uint32_t session_id, uint32_t config_id,
    TcConfigMode config_mode, std::string vtn_name,
    TcCommitOpAbortPhase fail_phase) {
  return unc::tclib::TC_SUCCESS;
}



/* audit related interfaces */
TcCommonRet TcLibIntfImpl::HandleAuditStart(uint32_t session_id,
                                            unc_keytype_ctrtype_t ctr_type,
                                            std::string controller_id,
                                            TcAuditType audit_type,
                                            uint64_t commit_number,
                                            uint64_t commit_date,
                                            std::string commit_application) {
  upll_rc_t urc = ucm_->OnAuditStart(controller_id.c_str(), audit_type);
  if (urc != UPLL_RC_SUCCESS &&
      urc != UPLL_RC_ERR_AUDIT_CANCELLED) {
    ucm_->OnAuditEnd(controller_id.c_str(), false);
  }
  if (urc != UPLL_RC_ERR_AUDIT_CANCELLED) {
    WriteUpllErrorBlock(&urc);
  }
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          ((urc == UPLL_RC_ERR_AUDIT_CANCELLED) ?
           unc::tclib::TC_CANCELLED_AUDIT : unc::tclib::TC_FAILURE));
}


TcCommonRet TcLibIntfImpl::HandleAuditEnd(uint32_t session_id,
                                          unc_keytype_ctrtype_t ctr_type,
                                          std::string controller_id,
                                          TcAuditResult audit_result) {
  UPLL_LOG_TRACE("AuditEnd result: %d", audit_result);
  upll_rc_t urc = ucm_->OnAuditEnd(controller_id.c_str(), false);
  WriteUpllErrorBlock(&urc);
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}

/**
 * @brief Message sent by TC to UPLL to cancel the ongoing audit operation
 */
TcCommonRet TcLibIntfImpl::HandleAuditCancel(uint32_t session_id,
                                             unc_keytype_ctrtype_t ctr_type,
                                             std::string controller_id) {
  upll_rc_t urc = ucm_->OnAuditCancel();
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}


TcCommonRet TcLibIntfImpl::HandleAuditTransactionStart(
    uint32_t session_id, unc_keytype_ctrtype_t ctr_type,
    std::string controller_id) {
  ConfigKeyVal *err_ckv =  NULL;
  upll_rc_t urc = ucm_->OnAuditTxStart(controller_id.c_str(), session_id,
                                       0, &err_ckv);
  if (urc != UPLL_RC_SUCCESS &&
      urc != UPLL_RC_ERR_AUDIT_CANCELLED) {
    // If local error in UPLL, send it to TC with controller id as ""
    const char *ctrlr_id = kUpllCtrlrId;
    if (err_ckv != NULL) {
      if (err_ckv->get_user_data()) {
        ctrlr_id = reinterpret_cast<char *>(
            (reinterpret_cast<unc::upll::ipc_util::key_user_data*>(
                    err_ckv->get_user_data()))->ctrlr_id);
        if (ctrlr_id == NULL) {
          UPLL_LOG_INFO("Controller Id is null");
          ctrlr_id = kUpllCtrlrId;
        }
        UPLL_LOG_TRACE("Controller Id set in response: %s", ctrlr_id);
      }
    }
    list<CtrlrTxResult*> tx_res_list;
    CtrlrTxResult unc_res(ctrlr_id, urc, urc);
    unc_res.err_ckv = err_ckv;   // err_ckv will be freed by CtlrTxResult
    tx_res_list.push_back(&unc_res);
    if (WriteBackTxResult(tx_res_list) != true) {
      urc = UPLL_RC_ERR_GENERIC;
    }
  }
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          ((urc == UPLL_RC_ERR_AUDIT_CANCELLED) ?
           unc::tclib::TC_CANCELLED_AUDIT : unc::tclib::TC_FAILURE));
}


TcCommonRet TcLibIntfImpl::HandleAuditTransactionEnd(
    uint32_t session_id, unc_keytype_ctrtype_t ctr_type,
    std::string controller_id, TcTransEndResult end_result) {
  upll_rc_t urc = ucm_->OnAuditTxEnd(controller_id.c_str());
  WriteUpllErrorBlock(&urc);
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}


TcCommonRet TcLibIntfImpl::HandleAuditVoteRequest(
    uint32_t session_id, uint32_t driver_id,
    std::string controller_id, TcDriverInfoMap& driver_info) {
  const std::set<std::string> *affected_ctrlr_list = NULL;
  upll_rc_t urc = ucm_->OnAuditTxVote(controller_id.c_str(),
                                      &affected_ctrlr_list);
  if (urc == UPLL_RC_SUCCESS) {
    urc = FillTcDriverInfoMap(&driver_info, affected_ctrlr_list, true);
  }
  WriteUpllErrorBlock(&urc);
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}

TcCommonRet TcLibIntfImpl::HandleAuditGlobalCommit(
    uint32_t session_id, uint32_t driver_id,
    std::string controller_id, TcDriverInfoMap& driver_info,
    TcAuditResult& audit_result) {
  // Lock event;
  ucm_->LockAlarmEvent();

  const std::set<std::string> *affected_ctrlr_list = NULL;
  upll_rc_t urc = ucm_->OnAuditTxGlobalCommit(controller_id.c_str(),
                                              &affected_ctrlr_list);
  if (urc == UPLL_RC_SUCCESS) {
    urc = FillTcDriverInfoMap(&driver_info, affected_ctrlr_list, true);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_FATAL("Failed to fill TcDriverInfoMap in AuditGlobalCommit");
    }
  }
  WriteUpllErrorBlock(&urc);
  audit_result = ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_AUDIT_SUCCESS :
                  unc::tclib::TC_AUDIT_FAILURE);
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
                  unc::tclib::TC_FAILURE);
}

TcCommonRet TcLibIntfImpl::HandleAuditDriverResult(
    uint32_t session_id, std::string controller_id,
    TcCommitPhaseType phase, TcCommitPhaseResult driver_result,
    TcAuditResult &audit_result) {
  TcConfigMode config_mode = TC_CONFIG_GLOBAL;
  std::string vtn_name = "";
  TcCommonRet tcr = HandleCommonTxDriverResult(session_id, 0, config_mode,
                                               vtn_name, phase, driver_result);
  /* Note: audit_result is applicable only to TC_AUDIT_GLOBAL_COMMIT_PHASE.
   * For TC_AUDIT_VOTE_PHASE, audit_result will be set to SUCCESS irrespective
   * of the vote result at UNC / Controler. */
  audit_result = unc::tclib::TC_AUDIT_SUCCESS;
  if (phase == unc::tclib::TC_AUDIT_GLOBAL_COMMIT_PHASE) {
    if (tcr != unc::tclib::TC_SUCCESS) {
      audit_result = unc::tclib::TC_AUDIT_FAILURE;
    } else {
      /* Iterate over each controller status to find audit_result */
      for (TcCommitPhaseResult::const_iterator ctr_it = driver_result.begin();
           ctr_it != driver_result.end(); ++ctr_it) {
        if (ctr_it->resp_code != UNC_RC_SUCCESS) {
          /* audit_result is failure even if ctrlr is disconnected or
           * vote request not sent to controller. */
          audit_result = unc::tclib::TC_AUDIT_FAILURE;
          break;
        }
      }
    }
  }
  // unlock event handling
  ucm_->UnlockAlarmEvent();
  return tcr;
}

TcCommonRet TcLibIntfImpl::HandleAuditGlobalAbort(
    uint32_t session_id, unc_keytype_ctrtype_t ctr_type,
    std::string controller_id, TcAuditOpAbortPhase fail_phase) {
  return unc::tclib::TC_SUCCESS;
}


/**
 * @brief Save Configuration
 */
TcCommonRet TcLibIntfImpl::HandleSaveConfiguration(uint32_t session_id,
                                                   uint64_t version_no) {
  bool prv_op_failed = false;
  upll_rc_t urc = ucm_->OnSaveRunningConfig(session_id, false,
                                            version_no, &prv_op_failed);
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}


/**
 * @brief Abort Candidate Configuration
 */
TcCommonRet TcLibIntfImpl::HandleAbortCandidate(uint32_t session_id,
                                                uint32_t config_id,
                                                TcConfigMode config_mode,
                                                std::string vtn_name,
                                                uint64_t version_no) {
  if (config_mode == TC_CONFIG_REAL) {
    UPLL_LOG_DEBUG("config mode is real mode");
    return unc::tclib::TC_SUCCESS;
  }
  TaskScheduler::Task *task = ucm_->fifo_scheduler_->AllowExecution(
                              kCriticalTaskPriority, __FUNCTION__);
  if (!task) {
    UPLL_LOG_FATAL("Failed to add priority task in FIFO scheduler");
    return unc::tclib::TC_FAILURE;
  }
  bool prv_op_failed = false;
  upll_rc_t urc = ucm_->OnAbortCandidateConfig(session_id, false,
                                              version_no, &prv_op_failed,
                                              config_mode, vtn_name);
  if (!(ucm_->fifo_scheduler_->DoneExecution(task))) {
    UPLL_LOG_FATAL("Failed to complete the priority task");
    return unc::tclib::TC_FAILURE;
  }

  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}


/**
 * @brief Clear Startup Configuration
 */
TcCommonRet TcLibIntfImpl::HandleClearStartup(uint32_t session_id) {
  upll_rc_t urc = ucm_->OnClearStartupConfig(session_id);
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}


/**
 * @brief HandleAuditConfig DB
 */
TcCommonRet TcLibIntfImpl::HandleAuditConfig(unc_keytype_datatype_t db_target,
                                             TcServiceType fail_oper,
                                             TcConfigMode config_mode,
                                             std::string vtn_name,
                                             uint64_t version_no) {
  if (config_mode == TC_CONFIG_REAL) {
    UPLL_LOG_DEBUG("config mode is real mode");
    return unc::tclib::TC_SUCCESS;
  }
  if (!IsActiveNode()) {
    UPLL_LOG_WARN("Node is not active, cannot handle Tclib audit config");
    return unc::tclib::TC_FAILURE;
  }
  if (IsShuttingDown()) {
    UPLL_LOG_WARN("Shutting down, cannot handle Tclib audit config");
    return unc::tclib::TC_FAILURE;
  }

  // NOTE: UPLL does not validate AUDIT-DB preconditions after cold-start
  // It is expected TC calls UPLL only for Commit failure if auto-save is
  // enabled and Save failure & Clear failure if auto-save is disabled.
  if (!ucm_->UpdateDirtyCache()) {
    UPLL_LOG_INFO("UpdateDirtyCache failed. dt:%d, fail_op:%d, cfg_mode:%d, "
                  "vtn_name:%s, version_no:%"PFC_PFMT_u64, db_target, fail_oper,
                  config_mode, vtn_name.c_str(), version_no);
    return unc::tclib::TC_FAILURE;
  }
  upll_rc_t urc;

  switch (fail_oper) {
    case TC_OP_CANDIDATE_COMMIT:
      {
        // Redo Copy Candidate to Running
        // NOTE: Each MoMgr on NULL tx_res_list, should just copy candidate to
        // running with cs_status as NOT_APPLIED for all diff-config.
        list<CtrlrTxResult*> tx_res_list;
        urc = ucm_->OnTxCommitCtrlrStatus(0, 0, NULL, config_mode, vtn_name);
        return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
                unc::tclib::TC_FAILURE);
      }
      break;
    case TC_OP_CANDIDATE_ABORT:
      {
        bool prv_op_failed = false;
        urc = ucm_->OnAbortCandidateConfig(0, true, version_no,
                                           &prv_op_failed, config_mode,
                                           vtn_name);
        return ((urc == UPLL_RC_SUCCESS) ? ((prv_op_failed) ?
                                           unc::tclib::TC_LAST_DB_OPER_FAILURE :
                                           unc::tclib::TC_SUCCESS) :
                                           unc::tclib::TC_FAILURE);
      }
      break;
    case TC_OP_RUNNING_SAVE:
      {
        bool prv_op_failed = false;
        urc = ucm_->OnSaveRunningConfig(0, true, version_no,
                                        &prv_op_failed);
        return ((urc == UPLL_RC_SUCCESS) ? ((prv_op_failed) ?
                                            unc::tclib::TC_LAST_DB_OPER_FAILURE:
                                            unc::tclib::TC_SUCCESS) :
                                            unc::tclib::TC_FAILURE);
      }
      break;
    case TC_OP_CLEAR_STARTUP:
      return HandleClearStartup(0);
      break;
    case TC_OP_USER_AUDIT:
    case TC_OP_DRIVER_AUDIT:
      {
        /* Always done as part transitioning to ACT, so no need to do it here.
        upll_rc_t urc = ucm_->OnAuditEnd("TcRecovery", true);
        return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
                unc::tclib::TC_FAILURE);
        */
        return unc::tclib::TC_SUCCESS;
      }
      break;
    default:
      return unc::tclib::TC_FAILURE;
  }
  return unc::tclib::TC_FAILURE;
}

/**
 * @brief Setup Configuration
 */
TcCommonRet TcLibIntfImpl::HandleSetup() {
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = ucm_->OnLoadStartup();
  return ((urc == UPLL_RC_SUCCESS) ? unc::tclib::TC_SUCCESS :
          unc::tclib::TC_FAILURE);
}


/**
 * @brief Setup Complete
 * Message sent to UPPL during state changes
 */
TcCommonRet TcLibIntfImpl::HandleSetupComplete() {
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  ucm_->PopulateCtrlrInfo();
  upll_rc_t urc = ucm_->HandleSpineThresholdAlarm(true);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to send threshold alarms, urc=%d", urc);
  }
  return unc::tclib::TC_SUCCESS;
}


/**
 * @brief Get Driver Id
 * Invoked from TC to detect the driver id for a controller
 */
/*
unc_keytype_ctrtype_t TcLibIntfImpl::HandleGetDriverId(
    std::string controller_id) {
  // As per TC design, this API shouldn't have been called.
  UPLL_LOG_DEBUG("Shouldn't be here.");
  return UNC_CT_UNKNOWN;
}
*/

/**
 * @brief      Get controller type invoked from TC to detect the controller type
 *             for a controller
 * @param[in]  controller_id controller id intended for audit
 * @retval     openflow/overlay/legacy if controller id matches
 * @retval     UNC_CT_UNKNOWN if controller id does not belong to
 *             any of controller type
 */
unc_keytype_ctrtype_t TcLibIntfImpl::HandleGetControllerType(
    std::string controller_id) {
  // TC should get the controller type from Physical.
  UPLL_LOG_DEBUG("Shouldn't be here.");
  return UNC_CT_UNKNOWN;
}

/**
 * @brief Get Controller Type
 * Invoked from TC to detect the type of the controller
 * Intended for the driver modules
 */
unc_keytype_ctrtype_t TcLibIntfImpl::HandleGetControllerType() {
  // As per TC design, this API shouldn't have been called.
  UPLL_LOG_DEBUG("Shouldn't be here.");
  return UNC_CT_UNKNOWN;
}

/**
 * @brief      Write the logical error block to TC in case of internal error
 * @param[in]  upll response code urc
 */
void TcLibIntfImpl::WriteUpllErrorBlock(upll_rc_t *urc) {
  if (*urc != UPLL_RC_SUCCESS) {
    const char *ctrlr_id = kUpllCtrlrId;
    list<CtrlrTxResult*> tx_res_list;
    CtrlrTxResult unc_res(ctrlr_id, *urc, *urc);
    unc_res.err_ckv = NULL;
    tx_res_list.push_back(&unc_res);
    if (WriteBackTxResult(tx_res_list) != true) {
      *urc = UPLL_RC_ERR_GENERIC;
    }
  }
}

                                                                       // NOLINT
}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
