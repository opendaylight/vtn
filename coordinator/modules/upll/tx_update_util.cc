/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "config_mgr.hh"
#include "upll_util.hh"
#include "tx_update_util.hh"
#include "momgr_impl.hh"

namespace unc {
namespace upll {
namespace tx_update_util {

using std::vector;

using unc::upll::config_momgr::UpllConfigMgr;
using unc::upll::config_momgr::MoManager;
using unc::upll::ipc_util::IpcUtil;
using unc::upll::ipc_util::IpcResponse;
using unc::upll::ipc_util::controller_domain;
using unc::upll::ipc_util::key_user_data_t;
using unc::upll::ipc_util::KtUtil;
namespace uuu = unc::upll::upll_util;

TxUpdateUtil::TxUpdateUtil(uint32_t concurrency)
    :concurrency_(concurrency),
    next_ava_que_idx_(0),
    error_count_(0),
    update_completed_count_(0),
    tx_error_(false),
    err_ckv_(NULL),
    ctrlr_err_code_(UPLL_RC_SUCCESS),
    active_(false) {
}

bool TxUpdateUtil::Init() {
  active_ = false;
  update_completed_count_ = 0;
  return CreateTaskQueues();
}

TxUpdateUtil::~TxUpdateUtil() {
    DestroyTaskQueues();
}

bool TxUpdateUtil::CreateTaskQueues() {
  UPLL_FUNC_TRACE;

  // Assumes there are no valid_taskq_ids_
  valid_taskq_ids_.clear();

  for (uint32_t taskq = 0; taskq < concurrency_; taskq++) {
    // create task queue  for Driver update
    pfc_taskq_t taskq_id = PFC_TASKQ_INVALID_ID;
    int err = pfc_taskq_create(&taskq_id, NULL, 1);
    if (err != 0) {
      UPLL_LOG_FATAL("Failed to create taskq.");
      return false;
    }  else {
      UPLL_LOG_INFO("taskq created successfully.\n");
      valid_taskq_ids_.push_back(taskq_id);
    }
  }
  UPLL_LOG_INFO("%" PFC_PFMT_SIZE_T " task queues created",
                valid_taskq_ids_.size());
  return true;
}

void TxUpdateUtil::DestroyTaskQueues() {
  UPLL_FUNC_TRACE;
  for (vector<pfc_taskq_t>::const_iterator itr = valid_taskq_ids_.begin();
       itr != valid_taskq_ids_.end(); itr++) {
    if (*itr != PFC_TASKQ_INVALID_ID) {
      int err = pfc_taskq_destroy(*itr);
      if (err != 0) {
        UPLL_LOG_FATAL("Failed to destroy ctrlr_taskq_ err=%d", err);
      } else {
        UPLL_LOG_INFO("ctrlr_taskq_ destroyed successfully");
      }
    }
  }
}

upll_rc_t TxUpdateUtil::AddTask(pfc_taskq_t taskq_id, pfc_taskfunc_t task_func,
                                void *task_data) {
  UPLL_FUNC_TRACE;
  if (taskq_id == PFC_TASKQ_INVALID_ID) {
    UPLL_LOG_INFO("Invalid taskq specified");
    return UPLL_RC_ERR_GENERIC;
  }
  pfc_taskfunc_t func = task_func;
  pfc_task_t tid = PFC_TASKQ_INVALID_TASKID;
  int err = pfc_taskq_dispatch(taskq_id, func, task_data,
                               0,
                               &tid);
  if (err != 0) {
    UPLL_LOG_INFO("Dispatch to %u failed . err=%d", taskq_id, err);
    return UPLL_RC_ERR_GENERIC;
  }

  return UPLL_RC_SUCCESS;
}

pfc_taskq_t TxUpdateUtil::GetCtrlrQueue(const char *ctrlr_name) {
  typedef std::pair<pfc_taskq_t, const char*> req_pair;
  if (queue_ctrlr_map_.size() == 0) {
    queue_ctrlr_map_.insert(req_pair(valid_taskq_ids_[next_ava_que_idx_],
                             ctrlr_name));
    return valid_taskq_ids_[next_ava_que_idx_++];
  } else {
    typedef std::multimap<pfc_taskq_t, std::string>::iterator multi_iterator;
    std::pair <multi_iterator, multi_iterator> ret;
    vector<pfc_taskq_t>::const_iterator itr;
    for (itr = valid_taskq_ids_.begin(); itr != valid_taskq_ids_.end(); itr++) {
      ret = queue_ctrlr_map_.equal_range(*itr);
      multi_iterator mitr;
      for (mitr = ret.first; mitr != ret.second; mitr++) {
        if ((mitr->second).compare(ctrlr_name) == 0) {
           return *itr;
        }
      }
    }
    if (next_ava_que_idx_ >=  concurrency_) {
      next_ava_que_idx_ = 0;
    }
    queue_ctrlr_map_.insert(req_pair(valid_taskq_ids_[next_ava_que_idx_],
                             ctrlr_name));
    return valid_taskq_ids_[next_ava_que_idx_++];
  }
  return PFC_TASKQ_INVALID_ID;
}

upll_rc_t TxUpdateUtil::EnqueueRequest(uint32_t session_id,
                                       uint32_t config_id,
                                       upll_keytype_datatype_t dt_type,
                                       unc_keytype_operation_t op,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ck_main,
                                       ConfigKeyVal *ckv_req,
                                       std::string domain_type) {
  UPLL_FUNC_TRACE;

  if (!ck_main) {
    UPLL_LOG_INFO("ck_main is null");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t urc = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);

  pfc_taskq_t taskq_id = GetCtrlrQueue(reinterpret_cast<const char*>
                                       (ctrlr_dom.ctrlr));
  UPLL_LOG_DEBUG("Controller %s assigned to queue %u",
                 reinterpret_cast<const char*>(ctrlr_dom.ctrlr),
                 taskq_id);

  pfc::core::ScopedMutex sm(access_mutex_);

  if (!active_) {
    // Drop the request
    UPLL_LOG_DEBUG("Dropping the request as I am inactive");
    return UPLL_RC_ERR_GENERIC;
  }

  IpcRequest  *ipc_req = new IpcRequest;
  memset(ipc_req, 0, sizeof(*ipc_req));
  ipc_req->header.clnt_sess_id = session_id;
  ipc_req->header.config_id = config_id;
  ipc_req->header.operation = op;
  ipc_req->header.datatype = dt_type;
  ipc_req->ckv_data = ck_main;

  pfc_taskfunc_t task_func = &HandleTxUpdateRequestStatic;
  TaskData* task_data = new TaskData(dmi, ckv_req, ipc_req, this, domain_type);
  if (UPLL_RC_SUCCESS != (urc = AddTask(taskq_id, task_func,
                                        static_cast <void*>(task_data)))) {
    UPLL_LOG_INFO("EnqueueRequest is failed for controller:%s",
                  reinterpret_cast<const char*>(ctrlr_dom.ctrlr));
    DELETE_IF_NOT_NULL(ipc_req);
    delete task_data;
    return urc;
  }
  UPLL_LOG_DEBUG("Request is added to queue successfully.");
  return urc;
}

bool TxUpdateUtil::Activate() {
  set_active(true);
  return true;
}

bool TxUpdateUtil::Deactivate() {
  set_active(false);
  return true;
}

bool TxUpdateUtil::ClearRequestsFrmQueues() {
  UPLL_FUNC_TRACE;
  for (vector<pfc_taskq_t>::iterator itr = valid_taskq_ids_.begin();
       ((itr != valid_taskq_ids_.end()) && (*itr != PFC_TASKQ_INVALID_ID));
       itr++) {
    int err = pfc_taskq_clear(*itr, NULL);
    if (err != 0) {
      UPLL_LOG_FATAL("Failed to clear taskq %u err=%d", *itr, err);
      return false;
    } else {
      UPLL_LOG_DEBUG("Taskq %u cleared successfully", *itr);
    }
  }
  return true;
}

bool TxUpdateUtil::NotifyTxUpdateCompletion() {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  void *task_data = this;
  pfc_taskfunc_t task_func=&HandleTxUpdateCompletionStatic;
  for (vector<pfc_taskq_t>::const_iterator iter = valid_taskq_ids_.begin();
       iter != valid_taskq_ids_.end(); iter++) {
    if (UPLL_RC_SUCCESS != (urc = AddTask(*iter, task_func, task_data))) {
      UPLL_LOG_FATAL("Failed to Add Task function for TxUpdateCompletion");
      return false;
    }
  }
  return true;
}
void TxUpdateUtil::HandleTxUpdateRequestStatic(void *task_data) {
  TaskData *td = reinterpret_cast<TaskData*>(task_data);
  td->tx_util_->HandleTxUpdateRequest(td);
}

void TxUpdateUtil::HandleTxUpdateRequest(TaskData *task_data) {
  // TODO(venkat) How to handle read-write connection
  upll_rc_t urc = UPLL_RC_SUCCESS;
  if (!active_) {
    // Drop the request
    UPLL_LOG_DEBUG("Dropping the request as I am inactive");
    DELETE_IF_NOT_NULL(task_data->ckv_req_);
    DELETE_IF_NOT_NULL(task_data->req_->ckv_data);
    DELETE_IF_NOT_NULL(task_data->req_);
    DELETE_IF_NOT_NULL(task_data);
    return;
  }
  IpcResponse ipc_resp;
  memset(&ipc_resp, 0, sizeof(IpcResponse));
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  GET_USER_DATA_CTRLR_DOMAIN(task_data->req_->ckv_data, ctrlr_dom);
  char drv_domain[KtUtil::kDrvDomainNameLenWith0];
  drv_domain[0] = 0;
  if ((ctrlr_dom.domain != NULL) && (ctrlr_dom.domain[0] != '\0')) {
    uuu::upll_strncpy(drv_domain, (task_data->domain_type_).c_str(),
                      KtUtil::kDrvDomainNameLenWith0);
    strncat(drv_domain, reinterpret_cast<char *>(ctrlr_dom.domain),
            strlen(reinterpret_cast<char *>(ctrlr_dom.domain))+1);
  }
  if (!IpcUtil::SendReqToDriver(reinterpret_cast<const char *>(ctrlr_dom.ctrlr),
                                ((drv_domain[0]) ? drv_domain :
                                 reinterpret_cast<char *>(ctrlr_dom.domain)),
                                NULL, static_cast<pfc_ipcid_t>(0),
                                task_data->req_, true, &ipc_resp)) {
    UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                    task_data->req_->ckv_data->get_key_type(),
                    reinterpret_cast<char *>(ctrlr_dom.ctrlr));
  }
  urc = ipc_resp.header.result_code;
  if (urc == UPLL_RC_ERR_CTR_DISCONNECTED) {
    UPLL_LOG_DEBUG("Driver result code - %s controller disconnected",
                   ctrlr_dom.ctrlr);
    urc = UPLL_RC_SUCCESS;
  }

  if (urc == UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(task_data->ckv_req_);
    DELETE_IF_NOT_NULL(task_data->req_->ckv_data);
    DELETE_IF_NOT_NULL(task_data->req_);
    DELETE_IF_NOT_NULL(task_data);
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return;
  }

  UpllConfigMgr *cfg_mgr = UpllConfigMgr::GetUpllConfigMgr();
  if (cfg_mgr == NULL) {
    UPLL_LOG_FATAL("NULL UpllConfigMgr");
    return;
  }
/*
  MoManager *momgr = NULL;
  if (ipc_resp.ckv_data != NULL) {
    momgr = cfg_mgr->GetMoManager(ipc_resp.ckv_data->get_key_type());
    if (momgr == NULL) {
      UPLL_LOG_FATAL("NULL momgr");
      return;
    }
  } else {
    DELETE_IF_NOT_NULL(task_data->ckv_req_);
    DELETE_IF_NOT_NULL(task_data->req_->ckv_data);
    DELETE_IF_NOT_NULL(task_data->req_);
    DELETE_IF_NOT_NULL(task_data);
    return;
  }
*/

  access_mutex_.lock();
  if (!active_) {
    DELETE_IF_NOT_NULL(task_data->ckv_req_);
    DELETE_IF_NOT_NULL(task_data->req_->ckv_data);
    DELETE_IF_NOT_NULL(task_data->req_);
    DELETE_IF_NOT_NULL(task_data);
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    access_mutex_.unlock();
    return;
  }
  error_count_++;
  ctrlr_err_code_ = urc;
  tx_error_= true;

  // Get and Translate err_ckv
  err_ckv_ = NULL;
  if (ipc_resp.ckv_data != NULL) {
    MoManager *momgr = cfg_mgr->GetMoManager(ipc_resp.ckv_data->get_key_type());
    if (momgr == NULL) {
      UPLL_LOG_FATAL("NULL momgr");
    } else if ((urc = momgr->TxUpdateErrorHandler(
                    task_data->ckv_req_,
                    task_data->req_->ckv_data,
                    task_data->dmi_,
                    task_data->req_->header.datatype,
                    &err_ckv_,
                    &ipc_resp)) != UPLL_RC_SUCCESS) {
      // Do nothing
    }
  }
  // TxUpdateErrorHandler cleared all memory
  active_ = false;
  access_mutex_.unlock();

  Lock();
  Signal();
  Unlock();
  DELETE_IF_NOT_NULL(task_data->req_);
  DELETE_IF_NOT_NULL(task_data);
}

void TxUpdateUtil::HandleTxUpdateCompletionStatic(void *tx_update_util) {
  TxUpdateUtil *tx_util = reinterpret_cast<TxUpdateUtil*>(tx_update_util);
  tx_util->HandleTxUpdateCompletion();
}

void TxUpdateUtil::HandleTxUpdateCompletion() {
  bool do_signal = false;
  access_mutex_.lock();
  update_completed_count_++;
  if (update_completed_count_ == concurrency_) {
    UPLL_LOG_INFO("All tx updates completed.");
    do_signal = true;
  }
  access_mutex_.unlock();

  if (do_signal) {
    Lock();
    Signal();
    Unlock();
  }
}

}  // namespace tx_update_util
}  // namespace upll
}  // namespace unc
