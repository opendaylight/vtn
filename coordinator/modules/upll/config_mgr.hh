/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_CONFIG_MGR_HH_
#define UPLL_CONFIG_MGR_HH_

#include <string>
#include <list>
#include <set>
#include <map>
#include <vector>

#include "pfcxx/ipc_server.hh"
#include "pfcxx/timer.hh"
#include "pfcxx/task_queue.hh"
#include "cxx/pfcxx/synch.hh"
#include "unc/uppl_common.h"
#include "unc/pfcdriver_include.h"
#include "unc/upll_errno.h"
#include "unc/upll_ipc_enum.h"
#include "dal/dal_odbc_mgr.hh"
#include "capa_intf.hh"
#include "tclib_module.hh"

#include "unc/upll_svc.h"
#include "upll_util.hh"
#include "ipc_util.hh"
#include "key_tree.hh"
#include "momgr_intf.hh"
#include "tclib_intf_impl.hh"
#include "config_lock.hh"
#include "dbconn_mgr.hh"
#include "ctrlr_mgr.hh"

namespace unc {
namespace upll {
namespace config_momgr {

using unc::upll::ipc_util::IpcReqRespHeader;
using unc::upll::ipc_util::ConfigKeyVal;
using unc::upll::ipc_util::ConfigVal;
using unc::upll::keytree::KeyTree;
using unc::upll::dal::DalOdbcMgr;

enum upll_alarm_category_t {
  UPLL_INVALID_CONFIGURATION_ALARM = 0,
  UPLL_OPERSTATUS_ALARM = 1,
  UPLL_PATHFAULT_ALARM = 2
};

enum upll_alarm_kind_t {
  UPLL_CLEAR_WITH_TRAP = 0,
  UPLL_ASSERT_WITH_TRAP = 1,
  UPLL_CLEAR_WITHOUT_TRAP = 0xC0
};

class UpllConfigMgr {
 public:
   /**
    * @brief Creates a singleton instance of UpllConfigMgr if one is not already
    * constructed.
    *
    * @return Pointer to newly constructed or exsting instance of UpllConfigMgr
    */
  static UpllConfigMgr *GetUpllConfigMgr() {
    if (!singleton_instance_) {
      singleton_instance_ = new UpllConfigMgr();
      singleton_instance_->Init();
    }
    return singleton_instance_;
  }

  /**
   * @brief Initializes UpllConfigMgr
   *
   * @return  On Success returns true, otherwise false.
   */
  bool Init();

  /**
   * @brief Accessor for getting CapaIntf
   *
   * @return CapaIntf
   */
  static const unc::capa::CapaIntf *GetCapaInterface();

  /**
   * @brief Accessor for getting TcLibModule instance
   *
   * @return  TcLibModule instance
   */
  static unc::tclib::TcLibModule *GetTcLibModule();

  /**
   * @brief Handler for all KeyTree services
   *
   * @param[in] service UPLL service id
   * @param[in,out] msghdr  On input it contains request params, on output it
   * contains response params
   * @param[in,out] ckv     On input, contains request key type information, on
   * output it contains response key type information.
   *
   * @return 0 on success, otherwise the error code PFC_IPCRESP_FATAL
   */
  pfc_ipcresp_t KtServiceHandler(upll_service_ids_t service,
                                 IpcReqRespHeader *msghdr, ConfigKeyVal *ckv);

  // GlobalConfigService

  upll_rc_t IsCandidateDirtyShallow(bool *dirty);
  /**
   * @brief Checks if candidate configuration is not same as running configuration.
   *
   * @param[out] dirty Set to true/false if candidate configuration is dirty or
   * not.
   *
   * @return @see upll_rc_t
   */
  upll_rc_t IsCandidateDirty(bool *dirty);
  // caller should have taken the READ lock on CANDIDATE and RUNNING
  upll_rc_t IsCandidateDirtyNoLock(bool *dirty);

  /**
   * @brief Imports controller configuration to DT_IMPORT
   *
   * @param[in] ctrlr_id    Name of the controller that is imported
   * @param[in] session_id  Session ID of the service user
   * @param[in] config_id   Config ID of the service user
   *
   * @return  @see upll_rc_t
   */
  upll_rc_t StartImport(const char *ctrlr_id, uint32_t session_id,
                        uint32_t config_id, upll_import_type import_type);

  /**
   * @brief Merge the imported configuration after validating if merge is
   * possible/allowed.
   *
   * @param[in] session_id  Session ID of the service user
   * @param[in] config_id   Config ID of the service user
   *
   * @return @see upll_rc_t
   */
  upll_rc_t OnMerge(uint32_t session_id, uint32_t config_id,
                    upll_import_type import_type);

  /**
   * @brief Validate if merge is possible / allowed
   *
   * @return @see upll_rc_t
   */
  upll_rc_t MergeValidate(upll_import_type import_type);
  /**
   * @brief Clear the ctrlr sepecific information
   * from the UNC candidate configuration
   *
   * @return @see upll_rc_t
   */
  upll_rc_t ClearCtrlrConfigInCandidate();
  /**
   * @brief Remove the deleted configuratoin from candiate
   * configuration during partial import 
   *
   * @return @see upll_rc_t
   */
  upll_rc_t PurgeCandidate();
  /**
   * @brief Does the database merging of import configuraiton to candidate
   * configuration.
   *
   * @return @see upll_rc_t
   */
  upll_rc_t MergeImportToCandidate(upll_import_type import_type);

  /**
   * @brief Clears imported configuration in the database
   *
   * @param[in] session_id  Session ID of the service user
   * @param[in] config_id   Config ID of the service user
   * @param[in] sby2act_trans   state transition to active
   *
   * @return
   */
  upll_rc_t ClearImport(uint32_t session_id, uint32_t config_id,
                        bool sby2act_trans);

  /**
   * @brief Check if the specified key is under use by UPLL.
   *
   * @param[in] datatype  Datatype
   * @param[in] ckv       Specifies one key type
   * @param[out] in_use   true if in use or false otherwise.
   *
   * @return @see upll_rc_t
   */
  upll_rc_t IsKeyInUse(upll_keytype_datatype_t datatype,
                       const ConfigKeyVal *ckv, bool *in_use);
  // Caller should have taken the READ lock on the datatype
  upll_rc_t IsKeyInUseNoLock(upll_keytype_datatype_t datatype,
                             const ConfigKeyVal *ckv, bool *in_use);

  /**
   * @brief Controller down event handler
   *
   * @param[in] ctrlr_name  name of the controller
   * @param[in] operstatus  operational status of controller 
   */
  void OnControllerStatusChange(const char *ctrlr_name, uint8_t operstatus);

  /**
   * @brief Logical Port operstatus change event handler
   *
   * @param[in] ctrlr_name
   * @param[in] domain_name
   * @param[in] logical_port_id
   * @param[in] oper_status
   */
  void OnLogicalPortStatusChange(const char *ctrlr_name,
                                 const char *domain_name,
                                 const char *logical_port_id,
                                 uint8_t oper_status);

  /**
   * @brief Bounday operstatus change event handler
   *
   * @param[in] boundary_id
   * @param[in] oper_status
   */
  void OnBoundaryStatusChange(const char *boundary_id, bool oper_status);

  /**
   * @brief Policer Full alarm handler
   *
   * @param[in] ctrlr_name
   * @param[in] domain_id
   * @param[in] key_vtn
   * @param[in] alarm_data
   * @param[in] alarm_raised
   */
  void OnPolicerFullAlarm(string ctrlr_name, string domain_id,
                          const key_vtn_t &key_vtn,
                          const pfcdrv_policier_alarm_data_t &alarm_data,
                          bool alarm_raised);

  /**
   * @brief Policer Fail alarm handler
   *
   * @param[in] ctrlr_name
   * @param[in] domain_id
   * @param[in] key_vtn
   * @param[in] alarm_data
   * @param[in] alarm_raised
   */
  void OnPolicerFailAlarm(string ctrlr_name, string domain_id,
                          const key_vtn_t &key_vtn,
                          const pfcdrv_policier_alarm_data_t &alarm_data,
                          bool alarm_raised);

  /**
   * @brief Network Monitor fault handler
   *
   * @param[in] ctrlr_name
   * @param[in] domain_id
   * @param[in] key_vtn
   * @param[in] alarm_data
   * @param[in] alarm_raised
   */
  void OnNwmonFaultAlarm(string ctrlr_name, string domain_id,
                         const key_vtn_t &key_vtn,
                         const pfcdrv_network_mon_alarm_data_t &alarm_data,
                         bool alarm_raised);
  upll_rc_t GetControllerSatusFromPhysical(uint8_t *ctrlr_id,
                            bool &is_audit_wait,
                            uint32_t session_id,
                            uint32_t config_id);
  upll_rc_t OnTxStart(uint32_t session_id, uint32_t config_id,
                      ConfigKeyVal **err_ckv);
  upll_rc_t OnAuditTxStart(const char *ctrlr_id,
                        uint32_t session_id, uint32_t config_id,
                        ConfigKeyVal **err_ckv);
  upll_rc_t OnTxVote(const std::set<std::string> **affected_ctrlr_list,
                     ConfigKeyVal **err_ckv);
  upll_rc_t OnAuditTxVote(const char *ctrlr_id,
                          const std::set<std::string> **affected_ctrlr_list);
  upll_rc_t OnTxVoteCtrlrStatus(std::list<CtrlrVoteStatus*> *ctrlr_vote_status);
  upll_rc_t OnAuditTxVoteCtrlrStatus(CtrlrVoteStatus *ctrlr_vote_status);
  upll_rc_t OnTxGlobalCommit(const std::set<std::string> **affected_ctrlr_list);
  upll_rc_t OnAuditTxGlobalCommit(
      const char *ctrlr_id, const std::set<std::string> **affected_ctrlr_list);
  upll_rc_t OnTxCommitCtrlrStatus(
      uint32_t session_id, uint32_t config_id,
      std::list<CtrlrCommitStatus*> *ctrlr_commit_status);
  upll_rc_t OnAuditTxCommitCtrlrStatus(CtrlrCommitStatus *ctrlr_commit_status);
  upll_rc_t OnAuditStart(const char *ctrlr_id, bool skip_audit);
  upll_rc_t OnTxEnd();
  upll_rc_t OnAuditTxEnd(const char *ctrlr_id);
  upll_rc_t OnAuditEnd(const char *ctrlr_id, bool sby2act_trans);

  upll_rc_t OnLoadStartup();
  upll_rc_t OnSaveRunningConfig(uint32_t session_id);
  upll_rc_t OnAbortCandidateConfig(uint32_t session_id);
  upll_rc_t OnClearStartupConfig(uint32_t session_id);

  // BATCH configuration support
  upll_rc_t OnBatchStart(uint32_t session_id, uint32_t config_id);
  upll_rc_t OnBatchAlive(uint32_t session_id, uint32_t config_id);
  upll_rc_t OnBatchEnd(uint32_t session_id, uint32_t config_id, bool timedout);

  const KeyTree &GetConfigKeyTree() { return cktt_; }

  MoManager *GetMoManager(unc_key_type_t kt) {
    
    std::map<unc_key_type_t, MoManager*>::iterator it =
      upll_kt_momgrs_.find(kt);
    if (it != upll_kt_momgrs_.end() )
      return it->second;
    UPLL_LOG_INFO("No manager for kt %d", kt);
    return NULL;
  }

  void set_shutting_down(bool shutdown) {
    sys_state_rwlock_.wrlock();
    shutting_down_ = shutdown;
    TerminateBatch();
    tclib_impl_.set_shutting_down(shutdown);
    sys_state_rwlock_.unlock();
  }
  bool IsShuttingDown() {
    bool state;
    sys_state_rwlock_.rdlock();
    state = shutting_down_;
    sys_state_rwlock_.unlock();
    return state;
  }

  // cl_switched is true if cluster swithover is happening. false if just
  // cold-started
  void SetClusterState(bool active, bool cl_switched) {
    UPLL_LOG_INFO("active=%d, cl_switched=%d", active, cl_switched_);
    // At the start/switchover, set dirty_qc to true and unintialized.
    candidate_dirty_qc_lock_.lock();
    candidate_dirty_qc_ = true;
    candidate_dirty_qc_initialized_ = false;
    candidate_dirty_qc_lock_.unlock();

    sys_state_rwlock_.wrlock();
    node_active_ = active;
    cl_switched_ = cl_switched;
    tclib_impl_.SetClusterState(active);
    sys_state_rwlock_.unlock();
    dbcm_->TerminateAndInitializeDbConns(active);
    if (active) {
      upll_rc_t urc;
      if (!cl_switched_) {
        DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
        if (dbinst == NULL) {
          UPLL_LOG_FATAL("Failed to get config rw conn");
          return;
        }
        unc::upll::dal::DalResultCode drc = dbinst->UpdateDirtyTblCacheFromDB();
        if (drc != unc::upll::dal::kDalRcSuccess) {
          UPLL_LOG_FATAL("Failed to read dirty table from DB. drc=%d", drc);
        }
        candidate_dirty_qc_ = dbinst->IsAnyTableDirtyShallow();
        candidate_dirty_qc_initialized_ = true;

        dbcm_->ReleaseRwConn(dbinst);
      }

      urc = OnAuditEnd("", true);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_FATAL("Failed to clear audit tables, urc=%d", urc);
      }
      urc = ClearImport(0, 0, true);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_FATAL("Failed to clear import tables, urc=%d", urc);
      }
      // Clearing the controllers
      CtrlrMgr::GetInstance()->CleanUp();
      CtrlrMgr::GetInstance()->PrintCtrlrList();
    } else {
      // dbcm_->TerminateAllDbConns();
      TerminateBatch();
      // clear All alarms
      ClearAllAlarms();
    }
  }

  bool IsActiveNode() {
    bool state;
    sys_state_rwlock_.rdlock();
    state = node_active_;
    sys_state_rwlock_.unlock();
    return state;
  }

  upll_rc_t ContinueActiveProcess() {
    if (IsShuttingDown()) {
      UPLL_LOG_INFO("Shutting down, cannot preform the request");
      return UPLL_RC_ERR_SHUTTING_DOWN;
    }

    if (!IsActiveNode()) {
      UPLL_LOG_INFO("Node is not active, cannot preform the request");
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
    }
    return UPLL_RC_SUCCESS;
  }

  uint32_t GetDbROConnLimitFrmConfFile();
  bool SendOperStatusAlarm(const char *vtn_name, const char *vnode_name,
                           const char *vif_name, bool assert_alarm);
  bool SendPathFaultAlarm(const char *ctr_name, const char *domain_name,
                          const char *vtn_name, upll_alarm_kind_t alarm_kind);
  void OnPathFaultAlarm(const char *ctrlr_name,
                        const char *domain_name,
                        bool alarm_asserted);
  bool SendInvalidConfigAlarm(string ctrlr_name, bool assert_alarm);

  // init operstatus
  upll_rc_t InitAllOperStatus();
  void ClearAllAlarms();
  void ClearPathFaultAlarms(const char *ctrlr_name);
  inline void LockPathFaultEvent() {
    pfc_sem_init(&sem_path_fault_, 0);
  }
  inline void UnlockPathFaultEvent() {
    pfc_sem_post(&sem_path_fault_);
  }

 private:
  UpllConfigMgr();
  virtual ~UpllConfigMgr();

  bool BuildKeyTree();
  void DumpKeyTree();
  void RegisterIpcStruct();
  bool CreateMoManagers();

  upll_rc_t ValidSession(uint32_t clnt_sess_id, uint32_t config_id);
  upll_rc_t ValidIpcOption1(unc_keytype_option1_t option1);
  upll_rc_t ValidIpcDatatype(upll_keytype_datatype_t datatype);
  upll_rc_t ValidIpcOperation(upll_service_ids_t service,
                              upll_keytype_datatype_t datatype,
                              unc_keytype_operation_t operation);
  upll_rc_t ValidateKtRequest(upll_service_ids_t service,
                              const IpcReqRespHeader &msghdr,
                              const ConfigKeyVal &ckv);

  bool FindRequiredLocks(unc_keytype_operation_t oper,
                         upll_keytype_datatype_t datatype,
                         upll_keytype_datatype_t *lck_dt_1,
                         ConfigLock::LockType *lck_type_1,
                         upll_keytype_datatype_t *lck_dt_2,
                         ConfigLock::LockType *lck_type_2);
  bool TakeConfigLock(unc_keytype_operation_t oper,
                      upll_keytype_datatype_t datatype);
  bool ReleaseConfigLock(unc_keytype_operation_t oper,
                         upll_keytype_datatype_t datatype);

  upll_rc_t ReadNextMo(IpcReqRespHeader *req, ConfigKeyVal *ckv,
                       DalDmlIntf *dmi);
  upll_rc_t ReadBulkMo(IpcReqRespHeader *req, ConfigKeyVal *ckv,
                       DalDmlIntf *dmi);
  upll_rc_t ReadBulkMo(IpcReqRespHeader *req, const ConfigKeyVal *user_req_ckv,
                       ConfigKeyVal **user_resp_ckv, DalDmlIntf *dmi);
  upll_rc_t ReadBulkGetSubtree(const KeyTree &keytree,
                               upll_keytype_datatype_t datatype,
                               const ConfigKeyVal *user_req_ckv,
                               uint32_t requested_cnt,
                               ConfigKeyVal **user_resp_ckv,
                               uint32_t *added_cnt, DalDmlIntf *dmi);

  upll_rc_t ValidateCommit(const char *caller);
  upll_rc_t ValidateAudit(const char *caller, const char *ctrlr_id);
  upll_rc_t ValidateImport(uint32_t session_id, uint32_t config_id,
                           const char *ctrlr_id, uint32_t operation,
                           upll_import_type import_type);
  upll_rc_t ImportCtrlrConfig(const char *ctrlr_id, upll_keytype_datatype_t dt,
                              upll_import_type import_type);

  void TriggerInvalidConfigAlarm(upll_rc_t ctrlt_result, string ctrlr_id);


  void GetBatchParamsFrmConfFile();
  upll_rc_t ValidateBatchConfigMode(uint32_t session_id, uint32_t config_id);
  upll_rc_t RegisterBatchTimeoutCB(uint32_t session_id, uint32_t config_id);
  upll_rc_t DbCommitIfInBatchMode(const char* func);
  void TerminateBatch();

  bool node_active_;  // cluster state
  bool cl_switched_;   // whether cluster state switched. On coldstart false.
  bool shutting_down_;
  pfc::core::ReadWriteLock sys_state_rwlock_;
  KeyTree cktt_;
  KeyTree iktt_;
  std::map<unc_key_type_t, std::string> kt_name_map_;
  ConfigLock cfg_lock_;
  bool candidate_dirty_qc_;
  bool candidate_dirty_qc_initialized_; // Whether deterministically initialized or not
  pfc::core::Mutex candidate_dirty_qc_lock_;
  std::map<unc_key_type_t, MoManager*> upll_kt_momgrs_;
  TcLibIntfImpl tclib_impl_;


  bool commit_in_progress_;      // Regular transaction in progress
  bool audit_in_progress_;
  bool import_in_progress_;
  upll_import_type current_import_type;
  upll_import_type import_type;
  uint8_t import_state_progress;
  string import_ctrlr_id_;
  string audit_ctrlr_id_;
  KTxCtrlrAffectedState audit_ctrlr_affected_state_;
  pfc::core::Mutex  commit_mutex_lock_;
  pfc::core::Mutex  import_mutex_lock_;
  pfc::core::Mutex  audit_mutex_lock_;
  std::set<std::string> affected_ctrlr_set_;
  pfc_sem_t sem_path_fault_;

  // Initail database connections. When node turns ACTIVE, intial connections
  // are mode, and when node turns STANDBY from ACTIVE, initial connections are
  // closed.
  UpllDbConnMgr *dbcm_;

  int32_t alarm_fd;

  // Batch configuration mode support
  class BatchTimeoutHandler {
   public:
    BatchTimeoutHandler() { }
    void operator() () {
      UpllConfigMgr* ucm = UpllConfigMgr::GetUpllConfigMgr();
      UPLL_LOG_INFO("Batch operation timed-out and BATCH-END is called");
      //we don't have to perform session validation on  batch-timeout
      //Session validation will be ignored in BatchEnd on passing true.
      ucm->OnBatchEnd(0, 0, true);
    }
  };

  pfc::core::Mutex batch_mutex_lock_;
  bool batch_mode_in_progress_;
  pfc::core::Timer* batch_timer_;
  pfc::core::TaskQueue* batch_taskq_;
  pfc_timeout_t  batch_timeout_id_;
  uint32_t batch_timeout_;  // in seconds
  BatchTimeoutHandler batch_timeout_fctr_;
  pfc::core::timer_func_t batch_timer_func_;
  uint32_t batch_commit_limit_;
  uint32_t batch_op_cnt_;

  static UpllConfigMgr *singleton_instance_;
};


}  // namespace config_momgr
}  // namespace upll
}  // namespace unc

#endif  // UPLL_CONFIG_MGR_HH_
