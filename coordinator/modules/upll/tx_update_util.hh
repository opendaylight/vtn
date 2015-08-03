/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef TX_UPDATE_UTIL_HH_
#define TX_UPDATE_UTIL_HH_

#include <string>
#include <algorithm>
#include <map>
#include <vector>

#include "cxx/pfcxx/synch.hh"
#include "pfcxx/task_queue.hh"
#include "dal/dal_odbc_mgr.hh"
#include "ipc_util.hh"
#include "upll_util.hh"
#include "kt_util.hh"
#include "uncxx/upll_log.hh"

namespace unc {
namespace upll {
namespace tx_update_util {

using unc::upll::ipc_util::IpcRequest;
using unc::upll::ipc_util::ConfigKeyVal;
using unc::upll::ipc_util::controller_domain_t;
using unc::upll::dal::DalDmlIntf;

// forward declaration
class TxUpdateUtil;

class TaskData {
 public:
  TaskData(DalDmlIntf *dmi,
           ConfigKeyVal *ckv_req,
           IpcRequest *req,
           TxUpdateUtil *tx_util,
           std::string domain_type):
  dmi_(dmi),
  ckv_req_(ckv_req),
  req_(req),
  tx_util_(tx_util),
  domain_type_(domain_type) {}
  ~TaskData() {}
 public:
  DalDmlIntf *dmi_;
  ConfigKeyVal *ckv_req_;
  IpcRequest *req_;
  TxUpdateUtil *tx_util_;
  std::string domain_type_;
};

class TxUpdateUtil {
 public:
  explicit TxUpdateUtil(uint32_t concurrency);
  bool Init();

  ~TxUpdateUtil();

  bool Activate();
  void set_active(bool active) {
    access_mutex_.lock();
    active_ = active;
    update_completed_count_ = 0;
    access_mutex_.unlock();
  }
  void get_active() {
    access_mutex_.lock();
    active_ = false;
    access_mutex_.unlock();
  }

  bool Deactivate();

  upll_rc_t EnqueueRequest(uint32_t session_id,
                           uint32_t config_id,
                           upll_keytype_datatype_t dt_type,
                           unc_keytype_operation_t op,
                           DalDmlIntf *dmi,
                           ConfigKeyVal *ck_main,
                           ConfigKeyVal *ckv_req,
                           std::string domain_type);
  inline void WaitForCompletion(ConfigKeyVal **err_ckv, upll_rc_t *urc) {
    Lock();
    if (NotifyTxUpdateCompletion() == false) {
      *urc = UPLL_RC_ERR_GENERIC;
      UPLL_LOG_FATAL("Failed to notify update completion to TxUtil");
      Unlock();
    } else {
      while (IsTxInProgress()) {  // While (no error)
      UPLL_LOG_INFO("Waiting for Completion");
      Wait();
    }
    Unlock();
    if (GetErrCount() > 0) {
      *err_ckv = err_ckv_;
      *urc = ctrlr_err_code_;
    }
    }
  }

  bool NotifyTxUpdateCompletion();
  inline bool IsTxInProgress() const {
    // Tx is in progress update_completed_count_ is FULL
    access_mutex_.lock();
    bool in_progress =  (update_completed_count_ != concurrency_);
    access_mutex_.unlock();
    return in_progress;
  }

  uint32_t GetErrCount() const  {
    return error_count_;
  }

  inline void ReInitializeTaskQParams() {
    UPLL_FUNC_TRACE;
    pfc::core::ScopedMutex sm(access_mutex_);
    ClearRequestsFrmQueues();
    update_completed_count_ = 0;
    // error related
    error_count_ = 0;
    tx_error_ = false;
    err_ckv_ = NULL;

    queue_ctrlr_map_.clear();
    next_ava_que_idx_ = 0;
  }

  inline void Lock() { sync_mutex_.lock(); }
  inline void Unlock() { sync_mutex_.unlock(); }
  inline void Wait() { sync_cond_.wait(sync_mutex_); }
  inline void Signal() { sync_cond_.signal(); }

 private:
  bool CreateTaskQueues();
  pfc_taskq_t GetCtrlrQueue(const char *ctrlr_name);
  upll_rc_t AddTask(pfc_taskq_t taskq_id, pfc_taskfunc_t task_func, void*
                    task_data);
  void DestroyTaskQueues();
  void ClearTaskQueues();
  bool ClearRequestsFrmQueues();

  // Parallel TxUpdate feature
  static void HandleTxUpdateRequestStatic(void *task_data);
  void HandleTxUpdateRequest(TaskData *task_data);
  static void HandleTxUpdateCompletionStatic(void *tx_update_util);
  void HandleTxUpdateCompletion();


 private:
  // task related
  uint32_t concurrency_;  // equivalent of no of task queues.
  uint32_t next_ava_que_idx_;
  std::multimap <pfc_taskq_t, std::string> queue_ctrlr_map_;
  // error related
  uint32_t error_count_;
  uint32_t update_completed_count_;
  bool tx_error_;  // send error or controller error
  ConfigKeyVal *err_ckv_;
  upll_rc_t ctrlr_err_code_;
  bool active_;
  mutable pfc::core::Mutex access_mutex_;
  // Synchronization related
  mutable pfc::core::Mutex sync_mutex_;
  mutable pfc::core::Condition sync_cond_;
  std::vector<pfc_taskq_t> valid_taskq_ids_;
};
}  // namespace tx_update_util
}  // namespace upll
}  // namespace unc

#endif  // TX_UPDATE_UTIL_HH_
