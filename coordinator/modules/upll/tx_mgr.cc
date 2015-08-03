/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfc/debug.h"
#include "cxx/pfcxx/synch.hh"
#include "unc/component.h"
#include "alarm.hh"
#include "uncxx/upll_log.hh"
#include "ctrlr_mgr.hh"
#include "unw_spine_domain_momgr.hh"
#include "config_mgr.hh"
namespace unc {
namespace upll {
namespace config_momgr {

using unc::upll::dal::DalOdbcMgr;
namespace uud = unc::upll::dal;
namespace uuds = unc::upll::dal::schema;
namespace uudst = unc::upll::dal::schema::table;

#define CALL_MOMGRS_PREORDER(func, ...)                                     \
  {                                                                         \
    const std::list<unc_key_type_t> *lst = cktt_.get_preorder_list();       \
    for (std::list<unc_key_type_t>::const_iterator it = lst->begin();       \
         it != lst->end(); it++) {                                          \
      const unc_key_type_t kt(*it);                                         \
      std::map<unc_key_type_t, MoManager*>::iterator momgr_it =             \
        upll_kt_momgrs_.find(kt);                                           \
      if (momgr_it != upll_kt_momgrs_.end()) {                              \
        UPLL_LOG_DEBUG("KT: %u; kt_name: %s", kt, kt_name_map_[kt].c_str());\
        MoManager *momgr = momgr_it->second;                                \
        urc = momgr->func(kt, __VA_ARGS__);                                 \
        if (urc == UPLL_RC_ERR_DRIVER_NOT_PRESENT) {                        \
          UPLL_LOG_WARN("Driver not present error for KT: %s",              \
                        kt_name_map_[kt].c_str());                          \
          continue;                                                         \
        }                                                                   \
        if (urc != UPLL_RC_SUCCESS) {                                       \
          UPLL_LOG_WARN("Error = %d, KT: %s", urc, kt_name_map_[kt].c_str());\
          break;                                                            \
        }                                                                   \
        if ((urc = ContinueActiveProcess()) != UPLL_RC_SUCCESS) {           \
          UPLL_LOG_WARN("Error = %d, KT: %s", urc, kt_name_map_[kt].c_str());\
          break;                                                            \
        }                                                                   \
      }                                                                     \
    }                                                                       \
  }

#define CALL_MOMGRS_REVERSE_ORDER(func, ...)                                \
  {                                                                         \
    const std::list<unc_key_type_t> *lst = cktt_.get_reverse_postorder_list();\
    for (std::list<unc_key_type_t>::const_iterator it = lst->begin();       \
         it != lst->end(); it++) {                                          \
      const unc_key_type_t kt(*it);                                         \
      std::map<unc_key_type_t, MoManager*>::iterator momgr_it =             \
        upll_kt_momgrs_.find(kt);                                           \
      if (momgr_it != upll_kt_momgrs_.end()) {                              \
        UPLL_LOG_DEBUG("KT: %u; kt_name: %s", kt, kt_name_map_[kt].c_str());\
        MoManager *momgr = momgr_it->second;                                \
        urc = momgr->func(kt, __VA_ARGS__);                                 \
        if (urc == UPLL_RC_ERR_DRIVER_NOT_PRESENT) {                        \
          UPLL_LOG_WARN("Driver not present error for KT: %s",              \
                        kt_name_map_[kt].c_str());                          \
          continue;                                                         \
        }                                                                   \
        if (urc != UPLL_RC_SUCCESS) {                                       \
          UPLL_LOG_WARN("Error = %d, KT: %s", urc, kt_name_map_[kt].c_str());\
          break;                                                            \
        }                                                                   \
        if ((urc = ContinueActiveProcess()) != UPLL_RC_SUCCESS) {           \
          UPLL_LOG_WARN("Error = %d, KT: %s", urc, kt_name_map_[kt].c_str());\
          break;                                                            \
        }                                                                   \
      }                                                                     \
    }                                                                       \
  }

upll_rc_t UpllConfigMgr::ValidateCommit(const char *caller) {
  UPLL_FUNC_TRACE;
  if (IsShuttingDown()) {
    UPLL_LOG_WARN("%s: Shutting down, cannot commit", caller);
    return UPLL_RC_ERR_SHUTTING_DOWN;
  }

  if (!IsActiveNode()) {
    UPLL_LOG_WARN("%s: Node is not active, cannot commit", caller);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
  }

  pfc::core::ScopedMutex lock(commit_mutex_lock_);
  if (commit_in_progress_ == false) {
    UPLL_LOG_WARN("%s: Commit is not in progress", caller);
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::ValidateAudit(const char *caller,
                                       const char *req_ctrlr_id) {
  UPLL_FUNC_TRACE;
  if (IsShuttingDown()) {
    UPLL_LOG_WARN("%s: Shutting down, cannot audit", caller);
    return UPLL_RC_ERR_SHUTTING_DOWN;
  }

  if (!IsActiveNode()) {
    UPLL_LOG_WARN("%s: Node is not active, cannot audit", caller);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
  }

  pfc::core::ScopedMutex lock(audit_mutex_lock_);
  if (audit_in_progress_ == false) {
    UPLL_LOG_WARN("%s: Audit is not in progress", caller);
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }
  if (0 != audit_ctrlr_id_.compare(req_ctrlr_id)) {
    UPLL_LOG_WARN("%s: Requested controller: %s, audit_ctrlr_id_: %s",
                caller, req_ctrlr_id, audit_ctrlr_id_.c_str());
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }

  return UPLL_RC_SUCCESS;
}

// TODO(PCM): Bug: OnTxStart, candidate DB is modified,
// but if TxVote fails, then candidate will not be in a
// consistent state to use for further operations.
upll_rc_t UpllConfigMgr::OnTxStart(uint32_t session_id, uint32_t config_id,
                                   ConfigKeyVal **err_ckv,
                                   TcConfigMode config_mode,
                                   std::string vtn_name) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;
  UPLL_LOG_TRACE("ConfigMode in TxStart: %d", config_mode);
  commit_mutex_lock_.lock();
  if (commit_in_progress_ == true) {
    UPLL_LOG_WARN("Another commit is in progress");
    commit_mutex_lock_.unlock();
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }
  commit_in_progress_ = true;
  commit_mutex_lock_.unlock();

  if (UPLL_RC_SUCCESS != (urc = ValidateCommit(__FUNCTION__))) {
    return urc;
  }

  // TODO(PCM): Recheck on PCM impact on/with DbCommitIfInBatchMode
  // If in BATCH mode, perform DB Commit
  urc = DbCommitIfInBatchMode(__FUNCTION__, kCriticalTaskPriority);
  if (urc != UPLL_RC_SUCCESS) {
    return urc;
  }

  affected_ctrlr_set_.clear();

  ScopedConfigLock scfg_lock(cfg_lock_, kCriticalTaskPriority,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  tx_util_->Activate();

  unc_key_type_t init_kts[] = { UNC_KT_POLICING_PROFILE,
                                  UNC_KT_FLOWLIST
                                };
  for (unsigned int i = 0; i < sizeof(init_kts)/
       sizeof(init_kts[0]); i++) {
    const unc_key_type_t init_kt(init_kts[i]);
    std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
      upll_kt_momgrs_.find(init_kt);
    if (momgr_it != upll_kt_momgrs_.end()) {
      UPLL_LOG_DEBUG("KT: %u; kt_name: %s",
                     init_kt, kt_name_map_[init_kt].c_str());
      MoManager *momgr = momgr_it->second;
      if (momgr == NULL)
        continue;
      urc = momgr->TxUpdateController(init_kt, session_id, config_id,
                                      kUpllUcpInit, &affected_ctrlr_set_,
                                      dbinst, err_ckv, tx_util_,
                                      config_mode, vtn_name);
      if (urc == UPLL_RC_SUCCESS) {
         urc = ContinueActiveProcess();
      }
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_WARN("Error = %d, KT: %s",
                      urc, kt_name_map_[init_kt].c_str());
        break;
      }
    }
  }



  if (urc == UPLL_RC_SUCCESS) {
    CALL_MOMGRS_REVERSE_ORDER(TxUpdateController, session_id, config_id,
                              kUpllUcpDelete, &affected_ctrlr_set_, dbinst,
                              err_ckv, tx_util_, config_mode, vtn_name);
  }

  if (urc == UPLL_RC_SUCCESS) {
    CALL_MOMGRS_PREORDER(TxUpdateController, session_id, config_id,
                         kUpllUcpCreate, &affected_ctrlr_set_, dbinst, err_ckv,
                         tx_util_, config_mode, vtn_name);
  }

  if (urc == UPLL_RC_SUCCESS) {
    CALL_MOMGRS_PREORDER(TxUpdateController, session_id, config_id,
                         kUpllUcpUpdate, &affected_ctrlr_set_, dbinst, err_ckv,
                         tx_util_, config_mode, vtn_name);
  }

  if (urc == UPLL_RC_SUCCESS) {
    unc_key_type_t phase2_kts[] = { UNC_KT_POLICING_PROFILE_ENTRY,
                                    UNC_KT_POLICING_PROFILE,
                                    UNC_KT_FLOWLIST_ENTRY,
                                    UNC_KT_FLOWLIST
                                  };
    for (unsigned int i = 0; i < sizeof(phase2_kts)/
         sizeof(phase2_kts[0]); i++) {
      const unc_key_type_t phase2_kt(phase2_kts[i]);
      std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
        upll_kt_momgrs_.find(phase2_kt);
      if (momgr_it != upll_kt_momgrs_.end()) {
        UPLL_LOG_DEBUG("KT: %u; kt_name: %s",
                       phase2_kt, kt_name_map_[phase2_kt].c_str());
        MoManager *momgr = momgr_it->second;
        if (momgr == NULL)
          continue;
        urc = momgr->TxUpdateController(phase2_kt, session_id, config_id,
                                        kUpllUcpDelete2, &affected_ctrlr_set_,
                                        dbinst, err_ckv, tx_util_,
                                        config_mode, vtn_name);
        if (urc == UPLL_RC_SUCCESS) {
           urc = ContinueActiveProcess();
        }
        if (urc != UPLL_RC_SUCCESS) {
          UPLL_LOG_WARN("Error = %d, KT: %s",
                        urc, kt_name_map_[phase2_kt].c_str());
          break;
        }
      }
    }
  }
  tx_util_->WaitForCompletion(err_ckv, &urc);
  tx_util_->Deactivate();
  tx_util_->ReInitializeTaskQParams();

  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, false);
  dbcm_->ReleaseRwConn(dbinst);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  return urc;
}

upll_rc_t UpllConfigMgr::OnAuditTxStart(const char *ctrlr_id,
                                        uint32_t session_id,
                                        uint32_t config_id,
                                        ConfigKeyVal **err_ckv) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  if (UPLL_RC_SUCCESS != (urc = ValidateAudit(__FUNCTION__, ctrlr_id))) {
    return urc;
  }

  // XXX Though running is updated with config status, take READ lock
  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_AUDIT, ConfigLock::CFG_READ_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetAuditRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  affected_ctrlr_set_.clear();

  KTxCtrlrAffectedState ctrlr_affected = kCtrlrAffectedNoDiff;
  audit_ctrlr_affected_state_ = ctrlr_affected;
  CALL_MOMGRS_REVERSE_ORDER(AuditUpdateController, ctrlr_id, session_id,
                            config_id, kUpllUcpDelete, dbinst,
                            err_ckv, &ctrlr_affected);
  if (urc != UPLL_RC_SUCCESS) {
    dbcm_->DalTxClose(dbinst,
                      (((urc != UPLL_RC_ERR_AUDIT_CANCELLED &&
                         urc != UPLL_RC_ERR_CTR_DISCONNECTED &&
                         urc != static_cast<upll_rc_t>(UNC_RC_CTR_BUSY)) &&
                       (*err_ckv != NULL))));
    dbcm_->ReleaseRwConn(dbinst);
    // Invalid configuration alarm is raised only
    // when key type configuration error occurs
    if (*err_ckv != NULL) {
      TriggerInvalidConfigAlarm(urc, ctrlr_id);
    }
    return urc;
  }

  CALL_MOMGRS_PREORDER(AuditUpdateController, ctrlr_id, session_id, config_id,
                       kUpllUcpCreate, dbinst, err_ckv,
                       &ctrlr_affected);
  if (urc != UPLL_RC_SUCCESS) {
    dbcm_->DalTxClose(dbinst,
                      (((urc != UPLL_RC_ERR_AUDIT_CANCELLED &&
                         urc != UPLL_RC_ERR_CTR_DISCONNECTED &&
                         urc != static_cast<upll_rc_t>(UNC_RC_CTR_BUSY)) &&
                         (*err_ckv != NULL))));
    dbcm_->ReleaseRwConn(dbinst);
    if (*err_ckv != NULL) {
      TriggerInvalidConfigAlarm(urc, ctrlr_id);
    }
    return urc;
  }

  CALL_MOMGRS_PREORDER(AuditUpdateController, ctrlr_id, session_id, config_id,
                       kUpllUcpUpdate, dbinst, err_ckv, &ctrlr_affected);

  if (urc != UPLL_RC_SUCCESS) {
    dbcm_->DalTxClose(dbinst,
                      (((urc != UPLL_RC_ERR_AUDIT_CANCELLED &&
                         urc != UPLL_RC_ERR_CTR_DISCONNECTED &&
                         urc != static_cast<upll_rc_t>(UNC_RC_CTR_BUSY)) &&
                         (*err_ckv != NULL))));
    dbcm_->ReleaseRwConn(dbinst);
    if (*err_ckv != NULL) {
      TriggerInvalidConfigAlarm(urc, ctrlr_id);
    }
    return urc;
  }

  unc_key_type_t phase2_kts[] = { UNC_KT_POLICING_PROFILE_ENTRY,
                                  UNC_KT_POLICING_PROFILE,
                                  UNC_KT_FLOWLIST_ENTRY,
                                  UNC_KT_FLOWLIST
                                };
  for (unsigned int i = 0; i < sizeof(phase2_kts)/sizeof(phase2_kts[0]); i++) {
    const unc_key_type_t phase2_kt(phase2_kts[i]);
    std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
        upll_kt_momgrs_.find(phase2_kt);
    if (momgr_it != upll_kt_momgrs_.end()) {
      UPLL_LOG_DEBUG("KT: %u; kt_name: %s", phase2_kt,
                     kt_name_map_[phase2_kt].c_str());
      MoManager *momgr = momgr_it->second;
      if (momgr == NULL)
        continue;
      urc = momgr->AuditUpdateController(phase2_kt, ctrlr_id, session_id,
                                         config_id, kUpllUcpDelete2,
                                         dbinst, err_ckv, &ctrlr_affected);
      if (urc == UPLL_RC_SUCCESS) {
         urc = ContinueActiveProcess();
      }
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_WARN("Error = %d, KT: %s", urc,
                      kt_name_map_[phase2_kt].c_str());
        // TriggerInvalidConfigAlarm is called at the end of the loop
        break;
      }
    }
  }

  upll_rc_t db_urc = dbcm_->DalTxClose(
      dbinst, (urc == UPLL_RC_SUCCESS) ||
      (((urc != UPLL_RC_ERR_AUDIT_CANCELLED &&
         urc != UPLL_RC_ERR_CTR_DISCONNECTED &&
         urc != static_cast<upll_rc_t>(UNC_RC_CTR_BUSY)) &&
         (*err_ckv != NULL))));
  dbcm_->ReleaseRwConn(dbinst);
  if (*err_ckv != NULL) {
    TriggerInvalidConfigAlarm(urc, ctrlr_id);
  }
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  if (urc == UPLL_RC_SUCCESS) {
    audit_ctrlr_affected_state_ = ctrlr_affected;
    if (ctrlr_affected == kCtrlrAffectedConfigDiff) {
      audit_mutex_lock_.lock();
      affected_ctrlr_set_.insert(audit_ctrlr_id_);
      audit_mutex_lock_.unlock();
    } else if (ctrlr_affected == kCtrlrAffectedOnlyCSDiff) {
      UPLL_LOG_INFO("Audit - only cs diff");
    } else {
      UPLL_LOG_INFO("No audit diff");
      TriggerInvalidConfigAlarm(UPLL_RC_SUCCESS, ctrlr_id);
    }
  }

  return urc;
}

upll_rc_t UpllConfigMgr::OnTxVote(
    const std::set<std::string> **affected_ctrlr_set,
    TcConfigMode config_mode, std::string vtn_name,
    ConfigKeyVal **err_ckv) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  if (UPLL_RC_SUCCESS != (urc = ValidateCommit(__FUNCTION__))) {
    return urc;
  }

  ScopedConfigLock scfg_lock(cfg_lock_, kCriticalTaskPriority,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_READ_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  CALL_MOMGRS_PREORDER(TxVote, dbinst, config_mode,
                       vtn_name, err_ckv);

  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, (urc == UPLL_RC_SUCCESS));
  dbcm_->ReleaseRwConn(dbinst);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  if (urc == UPLL_RC_SUCCESS)
    *affected_ctrlr_set = &affected_ctrlr_set_;

  return urc;
}

upll_rc_t UpllConfigMgr::OnAuditTxVote(
    const char *ctrlr_id, const std::set<std::string> **affected_ctrlr_set) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc;
  if (UPLL_RC_SUCCESS != (urc = ValidateAudit(__FUNCTION__, ctrlr_id))) {
    return urc;
  }
  *affected_ctrlr_set = &affected_ctrlr_set_;

  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::OnTxVoteCtrlrStatus(
    list<CtrlrVoteStatus*> *ctrlr_vote_status,
    TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  if (UPLL_RC_SUCCESS != (urc = ValidateCommit(__FUNCTION__))) {
    return urc;
  }

  ScopedConfigLock scfg_lock(cfg_lock_, kCriticalTaskPriority,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_READ_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  CALL_MOMGRS_PREORDER(TxVoteCtrlrStatus, ctrlr_vote_status, dbinst,
                       config_mode, vtn_name);

  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, (urc == UPLL_RC_SUCCESS));
  dbcm_->ReleaseRwConn(dbinst);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  return urc;
}

upll_rc_t UpllConfigMgr::OnAuditTxVoteCtrlrStatus(
    CtrlrVoteStatus *ctrlr_vote_status) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  if (UPLL_RC_SUCCESS != (urc = ValidateAudit(
              __FUNCTION__, ctrlr_vote_status->ctrlr_id.c_str()))) {
    return urc;
  }

  // XXX Though running is updated with config status, take READ lock
  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_AUDIT, ConfigLock::CFG_READ_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetAuditRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  CALL_MOMGRS_PREORDER(AuditVoteCtrlrStatus, ctrlr_vote_status, dbinst);

  upll_rc_t db_urc;
  if (UPLL_RC_SUCCESS != (db_urc = dbcm_->DalTxClose(
              dbinst, (urc == UPLL_RC_SUCCESS)))) {
    dbcm_->ReleaseRwConn(dbinst);
    return ((urc != UPLL_RC_SUCCESS) ? urc : db_urc);
  }
  dbcm_->ReleaseRwConn(dbinst);

  // Send an alarm
  if (ctrlr_vote_status->upll_ctrlr_result != UPLL_RC_SUCCESS &&
      ctrlr_vote_status->upll_ctrlr_result != UPLL_RC_ERR_CTR_DISCONNECTED &&
      ctrlr_vote_status->upll_ctrlr_result != UPLL_RC_ERR_AUDIT_CANCELLED &&
      ctrlr_vote_status->upll_ctrlr_result !=
          static_cast<upll_rc_t>(UNC_RC_CTR_BUSY)) {
    upll_rc_t ctrmgr_urc;
    bool config_invalid = false;
    if ((ctrmgr_urc = (CtrlrMgr::GetInstance()->IsConfigInvalid(
                    ctrlr_vote_status->ctrlr_id.c_str(),
                    &config_invalid))) != UPLL_RC_SUCCESS) {
      UPLL_LOG_WARN("Error in IsConfigInvalid(%s). Urc=%d",
                    ctrlr_vote_status->ctrlr_id.c_str(), ctrmgr_urc);
      return ctrmgr_urc;
    }
    if (config_invalid != true) {
      CtrlrMgr::GetInstance()->UpdateInvalidConfig(
          ctrlr_vote_status->ctrlr_id.c_str(), true);
      SendInvalidConfigAlarm(ctrlr_vote_status->ctrlr_id, true);
    } else {
      UPLL_LOG_WARN("More than one time audit failed for controller %s",
                    ctrlr_vote_status->ctrlr_id.c_str());
    }
  }

  return urc;
}

// TODO(PCM): Pass config mode into OnTxGlobalCommit
upll_rc_t UpllConfigMgr::OnTxGlobalCommit(
    const std::set<std::string> **affected_ctrlr_set,
    TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc;
  if (UPLL_RC_SUCCESS != (urc = ValidateCommit(__FUNCTION__))) {
    UPLL_LOG_FATAL("TxGlobalCommit failed. Urc=%d", urc);
    return urc;
  }

  *affected_ctrlr_set = &affected_ctrlr_set_;
  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::OnAuditTxGlobalCommit(
    const char *ctrlr_id, const std::set<std::string> **affected_ctrlr_set) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc;
  if (UPLL_RC_SUCCESS != (urc = ValidateAudit(__FUNCTION__, ctrlr_id))) {
    UPLL_LOG_FATAL("AuditTxGlobalCommit failed. Urc=%d", urc);
    return urc;
  }

  *affected_ctrlr_set = &affected_ctrlr_set_;

  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::OnTxCommitCtrlrStatus(
    uint32_t session_id, uint32_t config_id,
    list<CtrlrCommitStatus*> *ctrlr_commit_status,
    TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  using unc::upll::ipc_util::ConfigNotifier;

  // During normal transaction ctrlr_commit_status will not be NULL.
  // For TcLibInterface::HandleAuditConfig, if TcServiceType is
  // TC_OP_CANDIDATE_COMMIT, then ctrlr_commit_status is NULL
  if (ctrlr_commit_status != NULL) {
    if (UPLL_RC_SUCCESS != (urc = ValidateCommit(__FUNCTION__))) {
      return urc;
    }
  }

  // TODO(PCM): Investigate whether WRITE lock on candidate
  //            can be changed to READ lock
  ScopedConfigLock scfg_lock(cfg_lock_, kCriticalTaskPriority,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_WRITE_LOCK);
  // TODO(a): Why do we need write lock on CANDIDATE?

  DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  unc::upll::kt_momgr::UNWSpineDomainMoMgr *spd_mgr =
      reinterpret_cast<unc::upll::kt_momgr::UNWSpineDomainMoMgr *>(
       const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));
  if (spd_mgr) {
    spd_mgr->set_threshold_alarm_flag(false);
  }

  UPLL_LOG_INFO("*** TxCopyCandidateToRunning ***");
  CALL_MOMGRS_PREORDER(TxCopyCandidateToRunning, ctrlr_commit_status, dbinst,
                       config_mode, vtn_name);

  UPLL_LOG_INFO("*** Alarm Processing ***");
  if (urc == UPLL_RC_SUCCESS) {
    if (spd_mgr != NULL) {
      urc = spd_mgr->ProcessAlarm(config_mode, vtn_name, dbinst);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Failed to process UNW SPINE DOMAIN alarm, urc=%d", urc);
      }
      spd_mgr->set_threshold_alarm_flag(false);
    }
  }

  UPLL_LOG_INFO("*** TxUpdateDtState ***");
  if (urc == UPLL_RC_SUCCESS) {
    unc_key_type_t state_kts[] = { UNC_KT_VLINK, UNC_KT_VBRIDGE, UNC_KT_VROUTER,
                                   UNC_KT_VTERMINAL };
    for (unsigned int i = 0; i < sizeof(state_kts)/sizeof(state_kts[0]); i++) {
      MoManager *momgr = GetMoManager(state_kts[i]);
      if (momgr == NULL)
        continue;
// TODO(PCM): Bug: Does TxUpdateDtState() update only current mode objects?
//            Why the config mode is not passed? Can he use session_id and
//            config_id instead?
      urc = momgr->TxUpdateDtState(state_kts[i], session_id, config_id,
                                   dbinst);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Failed to update status for KT %d", state_kts[i]);
        break;
      }
      if ((urc = ContinueActiveProcess()) != UPLL_RC_SUCCESS) {
        UPLL_LOG_WARN("Error = %d, TxUpdateDtState KT: %d", urc, state_kts[i]);
        break;
      }
    }
  }

  if (urc == UPLL_RC_SUCCESS) {
    // At the end of commit operation, Clear all c_flag
    // and u_flag from candidate tables
    CALL_MOMGRS_PREORDER(TxClearCreateUpdateFlag, UPLL_DT_CANDIDATE, dbinst,
                         config_mode, vtn_name);
  }
  if (urc == UPLL_RC_SUCCESS) {
    // At the end of commit operation, Clear all the records
    // from ca_del table.
    CALL_MOMGRS_REVERSE_ORDER(ClearConfiguration, dbinst,
                              UPLL_DT_CANDIDATE_DEL,
                              config_mode, vtn_name);
  }

  const uint8_t *vtnname = reinterpret_cast<const uint8_t *>(vtn_name.c_str());
  if (urc == UPLL_RC_SUCCESS) {
    // Reset dirty flags for all tables in the DB.
  // TODO(PCM): Do UT for the following change in vtn_name usage
    if (TC_CONFIG_VIRTUAL == config_mode) {
      for (std::list<unc_key_type_t>::const_iterator it =
           preorder_virtual_kt_list_.begin();
           it != preorder_virtual_kt_list_.end(); it++) {
        const unc_key_type_t kt(*it);
        std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
          upll_kt_momgrs_.find(kt);
        if (momgr_it != upll_kt_momgrs_.end()) {
          // ClearVirtualKtDirtyInGlobal clears it in both DB and cache
          urc = momgr_it->second->ClearVirtualKtDirtyInGlobal(dbinst);
        }
      }
    } else {
      urc = UpllDbConnMgr::ConvertDalResultCode(
          dbinst->ClearAllDirtyTblInDB(UPLL_DT_CANDIDATE, config_mode,
                                       vtnname));
    }
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_WARN("Error = %d, Failed updating dirty table", urc);
    }
  }
  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, (urc == UPLL_RC_SUCCESS));
  // Release DB Connection after ClearDirtyTblCache
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  if (urc == UPLL_RC_SUCCESS) {
    // Clearing the dirty flags(used for skipping DB Diff operation) if stored
    // For TC_CONFIG_VIRTUAL, cachec is cleared with ClearVirtualKtDirtyInGlobal
    if (TC_CONFIG_VIRTUAL != config_mode) {
      dbinst->ClearDirtyTblCache(config_mode, vtnname);
    }

    // Send Config Notifications
    ConfigNotifier::SendBufferedNotificationsToUpllUser();
  } else {
    ConfigNotifier::CancelBufferedNotificationsToUpllUser();
  }
  dbcm_->ReleaseRwConn(dbinst);

  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("Committing candidate to running failed. Urc=%d", urc);
  }
  return urc;
}

upll_rc_t UpllConfigMgr::OnAuditTxCommitCtrlrStatus(
    CtrlrCommitStatus * ctrlr_commit_status) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  if (UPLL_RC_SUCCESS != (urc = ValidateAudit(
              __FUNCTION__, ctrlr_commit_status->ctrlr_id.c_str()))) {
    return urc;
  }

  // XXX Though running is updated with config status, take READ lock
  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_AUDIT, ConfigLock::CFG_READ_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetAuditRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  CALL_MOMGRS_PREORDER(AuditCommitCtrlrStatus, ctrlr_commit_status, dbinst);

  upll_rc_t db_urc;
  if (UPLL_RC_SUCCESS != (db_urc = dbcm_->DalTxClose(
              dbinst, (urc == UPLL_RC_SUCCESS)))) {
    dbcm_->ReleaseRwConn(dbinst);
    return ((urc != UPLL_RC_SUCCESS) ? urc : db_urc);
  }
  dbcm_->ReleaseRwConn(dbinst);
  TriggerInvalidConfigAlarm(ctrlr_commit_status->upll_ctrlr_result,
                            ctrlr_commit_status->ctrlr_id);

  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("AuditCommitCtrlrStatus failed. Urc=%d", urc);
  }

  return urc;
}

upll_rc_t UpllConfigMgr::OnTxEnd(TcConfigMode config_mode,
                                 std::string vtn_name) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  if (UPLL_RC_SUCCESS != (urc = ValidateCommit(__FUNCTION__))) {
    return urc;
  }

  ScopedConfigLock scfg_lock(cfg_lock_, kCriticalTaskPriority,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_WRITE_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  CALL_MOMGRS_PREORDER(TxEnd, dbinst, config_mode, vtn_name);

  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, (urc == UPLL_RC_SUCCESS));
  dbcm_->ReleaseRwConn(dbinst);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("TxEnd failed. Urc=%d", urc);
  }

  commit_mutex_lock_.lock();
  commit_in_progress_ = false;
  commit_mutex_lock_.unlock();

  affected_ctrlr_set_.clear();

  return urc;
}

upll_rc_t UpllConfigMgr::OnAuditTxEnd(const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc;
  if (UPLL_RC_SUCCESS != (urc = ValidateAudit(__FUNCTION__, ctrlr_id))) {
    return urc;
  }

  // Nothing to do in DB

  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::OnAuditStart(const char *ctrlr_id,
                                      TcAuditType audit_type) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s(%s, %d) ***", __FUNCTION__, ctrlr_id, audit_type);

  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  unc_keytype_ctrtype_t ctrlr_type;
  CtrlrMgr *ctr_mgr = CtrlrMgr::GetInstance();
  if (false == ctr_mgr->GetCtrlrType(
      ctrlr_id, UPLL_DT_RUNNING, &ctrlr_type)) {
    UPLL_LOG_INFO("Unknown controller. Cannot do audit for %s ",
                  ctrlr_id);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  audit_mutex_lock_.lock();
  if (audit_in_progress_ == true) {
    audit_mutex_lock_.unlock();
    UPLL_LOG_WARN("Another audit is in progress for %s", ctrlr_id);
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }
  audit_in_progress_ = true;
  audit_ctrlr_id_ = ctrlr_id;
  audit_mutex_lock_.unlock();

  if (UPLL_RC_SUCCESS != (urc = ValidateAudit(__FUNCTION__, ctrlr_id))) {
    return urc;
  }

  // In case of simplified audit or real network audit,
  // UPLL is not required to read
  // runnning configuration from controller
  if (audit_type == TC_AUDIT_SIMPLIFIED ||
      audit_type == TC_AUDIT_REALNETWORK) {
    return urc;
  }

  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_AUDIT, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  // Auto rename in audit uses same func of partial import auto rename,
  // so import type is passed as UPLL_IMPORT_TYPE_PARTIAL and this will
  // cover normal audit cases also
  urc = ImportCtrlrConfig(ctrlr_id, UPLL_DT_AUDIT, UPLL_IMPORT_TYPE_PARTIAL);

  return urc;
}

// UpllConfigMgr::OnAuditEnd() is called in two scenarios:
// 1. In normal audit operation at the end of Audit process either successful
// audit or failed audit. If updating to contorller failed, OnAuditEnd() is
// locall called.
// 2. During transition from Standby to Active.
upll_rc_t UpllConfigMgr::OnAuditEnd(const char *ctrlr_id, bool sby2act_trans) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  if (!sby2act_trans && !audit_cancelled_) {
    if (UPLL_RC_SUCCESS != (urc = ValidateAudit(__FUNCTION__, ctrlr_id))) {
      return urc;
    }
    // If only cs status is diffrent in the configuration then the
    // Call TxCommitCtrlrStatus to Update the config Status
    // During IMPORT->MERGE->COMMIT->AUDIT case, TC jumpt from
    // AuditTxStart to AuditEnd sice  there is no diff in the
    // configurations except cs-status in this case.
    //  In such case the configuration is not being sent to the controller
    //  during AUDIT operation
    if (audit_ctrlr_affected_state_ == kCtrlrAffectedOnlyCSDiff) {
      // Reset the audit controller affected state. Without reset,
      // stale value is present during failover
      audit_ctrlr_affected_state_ = kCtrlrAffectedNoDiff;
      CtrlrCommitStatus ccs(ctrlr_id, UPLL_RC_SUCCESS, UPLL_RC_SUCCESS);
      urc = OnAuditTxCommitCtrlrStatus(&ccs);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_FATAL("Failed to update config status in AuditEnd, Urc=%u",
                       urc);
        return urc;
      }
    }
  }
  // Reset the audit controller affected state. Without reset,
  // stale value is present during failover
  audit_ctrlr_affected_state_ = kCtrlrAffectedNoDiff;

  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_AUDIT, ConfigLock::CFG_WRITE_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetAuditRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  CALL_MOMGRS_REVERSE_ORDER(AuditEnd, ctrlr_id, dbinst);

  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("AuditEnd failed. Error:%d", urc);
  }

  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, (urc == UPLL_RC_SUCCESS));
  dbcm_->ReleaseRwConn(dbinst);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("AuditEnd failed. Urc=%d", urc);
    return urc;
  }

  audit_mutex_lock_.lock();
  audit_in_progress_ = false;
  audit_ctrlr_id_ = "";
  audit_cancelled_ = false;
  audit_mutex_lock_.unlock();

  affected_ctrlr_set_.clear();

  return urc;
}

upll_rc_t UpllConfigMgr::OnAuditCancel() {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_SUCCESS;
  audit_mutex_lock_.lock();
/*
  if (audit_in_progress_) {
*/
    if (audit_cancelled_) {
      urc = UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
    } else {
      audit_cancelled_ = true;
    }
/*
  } else {
    urc = UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }
*/
  audit_mutex_lock_.unlock();
  return urc;
}

// TODO(PCM): Bug: On system startup, scrath tables need to be emptied.
//            Use TRUNCATE for emptying the table
upll_rc_t UpllConfigMgr::OnLoadStartup() {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);

  if (IsShuttingDown()) {
    UPLL_LOG_WARN("Shutting down, cannot load startup");
    return UPLL_RC_ERR_SHUTTING_DOWN;
  }

  if (!IsActiveNode()) {
    UPLL_LOG_WARN("Node is not active, cannot load startup");
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
  }

  upll_rc_t urc = UPLL_RC_ERR_GENERIC;
  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_STARTUP, ConfigLock::CFG_READ_LOCK,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_WRITE_LOCK);
  // Audit and Import tables have been already cleared as part of transitioning
  // to ACTIVE
  /*
  ScopedConfigLock scfg_lck2(cfg_lock_,
                             UPLL_DT_IMPORT, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_AUDIT, ConfigLock::CFG_WRITE_LOCK);
  */

  DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
  if (dbinst == NULL) {
    UPLL_LOG_FATAL(
        "Loading startup configuration failed as no DB conn is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  // clear all the records from ca_del table during load startup.
  dbinst->MakeAllTableDirtyInCache();
  std::string vtn_name = "";
  CALL_MOMGRS_REVERSE_ORDER(ClearConfiguration, dbinst, UPLL_DT_CANDIDATE_DEL,
                            TC_CONFIG_GLOBAL, vtn_name);
  dbinst->ClearDirtyTblCache(TC_CONFIG_GLOBAL, NULL);
  if (urc != UPLL_RC_SUCCESS) {
    dbcm_->DalTxClose(dbinst, false);
    dbcm_->ReleaseRwConn(dbinst);
    UPLL_LOG_FATAL("Loading startup configuration failed. Urc=%d", urc);
    return urc;
  }
  CALL_MOMGRS_REVERSE_ORDER(ClearConfiguration, dbinst, UPLL_DT_CANDIDATE,
                            TC_CONFIG_GLOBAL, vtn_name);
  if (urc != UPLL_RC_SUCCESS) {
    dbcm_->DalTxClose(dbinst, false);
    dbcm_->ReleaseRwConn(dbinst);
    UPLL_LOG_FATAL("Loading startup configuration failed. Urc=%d", urc);
    return urc;
  }

  unc::tclib::TcLibModule *tclib_module =
      unc::upll::config_momgr::UpllConfigMgr::GetTcLibModule();
  if (tclib_module == NULL) {
    UPLL_LOG_FATAL("Unable to get tclib module");
    return urc;
  }

  // Check for validity of startup configuration. If startup validity is true,
  // clear running since startup will be copied to running
  if (tclib_module->IsStartupConfigValid() == PFC_TRUE) {
    CALL_MOMGRS_REVERSE_ORDER(ClearConfiguration, dbinst, UPLL_DT_RUNNING,
                              TC_CONFIG_GLOBAL, vtn_name);
    if (urc != UPLL_RC_SUCCESS) {
      dbcm_->DalTxClose(dbinst, false);
      dbcm_->ReleaseRwConn(dbinst);
      UPLL_LOG_FATAL("Loading startup configuration failed. Urc=%d", urc);
      return urc;
    }
  }

  CALL_MOMGRS_PREORDER(LoadStartup, dbinst);

  if (urc == UPLL_RC_SUCCESS) {
    // Initialize the  abort version number
    urc = UpdateSystemProperty(SYS_PROP_ABORT_OP, "0",  dbinst);
    if (urc != UPLL_RC_SUCCESS) {
      dbcm_->DalTxClose(dbinst, false);
      dbcm_->ReleaseRwConn(dbinst);
      UPLL_LOG_FATAL("Failed to update version info. Urc=%d", urc);
      return urc;
    }
    // Initialize the save version number
    urc = UpdateSystemProperty(SYS_PROP_SAVE_OP, "0", dbinst);
    if (urc != UPLL_RC_SUCCESS) {
      dbcm_->DalTxClose(dbinst, false);
      dbcm_->ReleaseRwConn(dbinst);
      UPLL_LOG_FATAL("Failed to update version info! Urc=%d", urc);
      return urc;
    }

    // Reset dirty flags for all tables in the DB.
    urc = UpllDbConnMgr::ConvertDalResultCode(
        dbinst->ClearAllDirtyTblInDB(UPLL_DT_CANDIDATE,
                                     TC_CONFIG_GLOBAL, NULL));
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_WARN("Error = %d, Failed updating dirty table", urc);
    }
  }

  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, (urc == UPLL_RC_SUCCESS));
  // Release DB Connection after ClearDirtyTblCache
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("Loading startup configuration failed. Urc=%d", urc);
  }

  if (urc == UPLL_RC_SUCCESS) {
    // After loading startup, candidate and running are in sync
    dbinst->ClearDirtyTblCache(TC_CONFIG_GLOBAL, NULL);
  }
  dbcm_->ReleaseRwConn(dbinst);
  return urc;
}

upll_rc_t UpllConfigMgr::OnSaveRunningConfig(uint32_t session_id,
                                             bool failover_operation,
                                             uint64_t version_no,
                                             bool *prv_op_failed) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  if (IsShuttingDown()) {
    UPLL_LOG_WARN("Shutting down, cannot save running");
    return UPLL_RC_ERR_SHUTTING_DOWN;
  }

  if (!IsActiveNode()) {
    UPLL_LOG_WARN("Node is not active, cannot save running");
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
  }

  // If in BATCH mode, perform DB Commit
  urc = DbCommitIfInBatchMode(__FUNCTION__, kNormalTaskPriority);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("SaveRunningConfig failed as BATCH-DB-Commit failed. Urc=%d",
                   urc);
    return urc;
  }

  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_STARTUP, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  if (failover_operation) {
    // Below validations are also done for audit-db during cold-start
    char upll_ver_buf[32];
    memset(upll_ver_buf, 0, 32);
    urc = GetSystemProperty(SYS_PROP_SAVE_OP, upll_ver_buf, dbinst);
    if (urc != UPLL_RC_SUCCESS) {
      dbcm_->DalTxClose(dbinst, false);
      dbcm_->ReleaseRwConn(dbinst);
      UPLL_LOG_FATAL("Save operation failed. Urc=%d", urc);
      return urc;
    }
    uint64_t upll_version = strtoull(reinterpret_cast<char*>(upll_ver_buf),
                                    NULL, 0);
    UPLL_LOG_INFO("TC Version: %"PFC_PFMT_u64 " UPLL Version: %"PFC_PFMT_u64,
                  version_no, upll_version);
    if (upll_version > version_no) {
      *prv_op_failed = false;
      UPLL_LOG_INFO("Save was successful. No recovery required");
      dbcm_->DalTxClose(dbinst, true);
      dbcm_->ReleaseRwConn(dbinst);
      return UPLL_RC_SUCCESS;
    } else if (upll_version == version_no) {
      UPLL_LOG_INFO("Previous save operation failed."
                    " User needs to reissue save running command");
      dbcm_->DalTxClose(dbinst, true);
      dbcm_->ReleaseRwConn(dbinst);
      // NOTE: Here the success return code is set alng with the
      // prv_op_failed flag as false which indiacates that
      // there is nothing to do in the logcal for the current operation
      // since the previous operation is failed.
      *prv_op_failed = true;
      return UPLL_RC_SUCCESS;
    } else {
      // This scenario should never occur if TC and this module design and code
      // is correct
      // If for some reason we come here, continue with the save operation
      *prv_op_failed = false;
      UPLL_LOG_INFO("Continuing with Save recovery.");
    }
  }
  // Clearing StartUp configuration
  std::string vtn_name = "";
  CALL_MOMGRS_REVERSE_ORDER(ClearConfiguration, dbinst, UPLL_DT_STARTUP,
                            TC_CONFIG_GLOBAL, vtn_name);
  if (urc != UPLL_RC_SUCCESS) {
    dbcm_->DalTxClose(dbinst, false);
    dbcm_->ReleaseRwConn(dbinst);
    UPLL_LOG_FATAL("SaveRunningConfig failed. Urc=%d", urc);
    return urc;
  }

  // Copying Running configuration to Startup
  CALL_MOMGRS_PREORDER(CopyRunningToStartup, dbinst);
  if (urc == UPLL_RC_SUCCESS) {
    char version_no_str[32];
    memset(version_no_str, 0, 32);
    snprintf(version_no_str, sizeof(version_no_str), "%"PFC_PFMT_u64,
             version_no);

    urc = UpdateSystemProperty(SYS_PROP_SAVE_OP, version_no_str, dbinst);
    if (urc != UPLL_RC_SUCCESS) {
      dbcm_->DalTxClose(dbinst, false);
      dbcm_->ReleaseRwConn(dbinst);
      UPLL_LOG_FATAL("Failed to update version info. Save failed. Urc=%d", urc);
      return urc;
    }
  }

  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, (urc == UPLL_RC_SUCCESS));
  dbcm_->ReleaseRwConn(dbinst);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("SaveRunningConfig failed. Urc=%d", urc);
  }

  return urc;
}

upll_rc_t UpllConfigMgr::OnAbortCandidateConfig(uint32_t session_id,
                                                bool failover_operation,
                                                uint64_t version_no,
                                                bool *prv_op_failed,
                                                TcConfigMode config_mode,
                                                std::string vtn_name) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);

  upll_rc_t urc = UPLL_RC_ERR_GENERIC;
  const uint8_t *vtnname = reinterpret_cast<const uint8_t *>(vtn_name.c_str());

  if (IsShuttingDown()) {
    UPLL_LOG_WARN("Shutting down, cannot abort candidate");
    return UPLL_RC_ERR_SHUTTING_DOWN;
  }

  if (!IsActiveNode()) {
    UPLL_LOG_WARN("Node is not active, cannot abort candidate");
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
  }

  // TODO(PCM): Verify DbCommitIfInBatchMode usage for PCM
  // TODO(Batch): This is an extra commit. Instead rollback can also
  //              be performed.
  // If in BATCH mode, perform DB Commit
  urc = DbCommitIfInBatchMode(__FUNCTION__, kCriticalTaskPriority);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("Abort candidate failed as BATCH-DB-Commit failed. Urc=%d",
                   urc);
    return urc;
  }
  // Abort is considered as critical task
  ScopedConfigLock scfg_lock(cfg_lock_, kCriticalTaskPriority,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  if (failover_operation) {
    char upll_ver_buf[32];
    memset(upll_ver_buf, 0, 32);
    urc = GetSystemProperty(SYS_PROP_ABORT_OP, upll_ver_buf, dbinst);
    if (urc != UPLL_RC_SUCCESS) {
      dbcm_->DalTxClose(dbinst, false);
      dbcm_->ReleaseRwConn(dbinst);
      UPLL_LOG_FATAL("Abort candidate failed. Urc=%d", urc);
      return urc;
    }
    uint64_t upll_version = strtoull(
        reinterpret_cast<char*>(upll_ver_buf), NULL, 0);
    UPLL_LOG_INFO("TC Version :%"PFC_PFMT_u64 "UPLL Version %"PFC_PFMT_u64,
                  version_no, upll_version);
    if (upll_version == version_no) {
      UPLL_LOG_INFO("Previous abort operation failed."
                    " User needs to reissue abort command");
      dbcm_->DalTxClose(dbinst, true);
      // NOTE: Here the success return code is set alng with the
      // prv_op_failed flag as false which indiacates that
      // there is nothing to do in the logcal for the current operation
      // since the previous operation is failed.
      dbcm_->ReleaseRwConn(dbinst);
      *prv_op_failed = true;
      return UPLL_RC_SUCCESS;
    } else if (upll_version > version_no) {
      UPLL_LOG_INFO("Abort was successful. No recovery required");
      dbcm_->DalTxClose(dbinst, true);
      dbcm_->ReleaseRwConn(dbinst);
      *prv_op_failed = false;
      return UPLL_RC_SUCCESS;
    } else {
      // This scenario should never occur if TC and this module design and code
      // is correct
      // If for some reason we come here, continue with the abort operation
      *prv_op_failed = false;
      UPLL_LOG_INFO("Continuing with abort recovery.");
    }
  }

  CALL_MOMGRS_REVERSE_ORDER(CopyRunningToCandidate, dbinst, UNC_OP_DELETE,
                            config_mode, vtn_name);
  if (urc == UPLL_RC_SUCCESS) {
    CALL_MOMGRS_PREORDER(CopyRunningToCandidate, dbinst, UNC_OP_CREATE,
                         config_mode, vtn_name);
  }
  if (urc == UPLL_RC_SUCCESS) {
    CALL_MOMGRS_PREORDER(CopyRunningToCandidate, dbinst, UNC_OP_UPDATE,
                         config_mode, vtn_name);
  }

  if (urc == UPLL_RC_SUCCESS) {
    char version_no_str[32];
    memset(version_no_str, 0, 32);
    snprintf(version_no_str, sizeof(version_no_str), "%"PFC_PFMT_u64,
             version_no);
    urc = UpdateSystemProperty(SYS_PROP_ABORT_OP, version_no_str, dbinst);
    if (urc != UPLL_RC_SUCCESS) {
      dbcm_->DalTxClose(dbinst, false);
      dbcm_->ReleaseRwConn(dbinst);
      UPLL_LOG_FATAL("Failed to update version info. Abort op failed. Urc=%d",
                     urc);
      return urc;
    }
  }

  if (urc == UPLL_RC_SUCCESS) {
    // Clear all c_flag and u_flag from candidate tables at the end of abort
    CALL_MOMGRS_PREORDER(TxClearCreateUpdateFlag, UPLL_DT_CANDIDATE, dbinst,
                         config_mode, vtn_name);
  }

  if (urc == UPLL_RC_SUCCESS) {
    // At the end of abort operation, clear all the records
    // from ca_del table when executing the abort command in global mode.
    CALL_MOMGRS_REVERSE_ORDER(ClearConfiguration, dbinst,
                              UPLL_DT_CANDIDATE_DEL, config_mode,
                              vtn_name);
  }
  if (urc == UPLL_RC_SUCCESS) {
    // Reset dirty flags for all tables in the DB.
    if (TC_CONFIG_VIRTUAL == config_mode) {
      for (std::list<unc_key_type_t>::const_iterator it =
           preorder_virtual_kt_list_.begin();
           it != preorder_virtual_kt_list_.end(); it++) {
        const unc_key_type_t kt(*it);
        std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
          upll_kt_momgrs_.find(kt);
        if (momgr_it != upll_kt_momgrs_.end()) {
          // ClearVirtualKtDirtyInGlobal clears it in both DB and cache
          urc = momgr_it->second->ClearVirtualKtDirtyInGlobal(dbinst);
        }
      }
    } else {
      urc = UpllDbConnMgr::ConvertDalResultCode(
          dbinst->ClearAllDirtyTblInDB(UPLL_DT_CANDIDATE, config_mode,
                                       vtnname));
    }
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_WARN("Error = %d, Failed to clear dirty table", urc);
    }
  }

  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, (urc == UPLL_RC_SUCCESS));
  // Release DB Connection after ClearDirtyTblCache
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("Abort candidate failed. Urc=%d", urc);
  }

  if (urc == UPLL_RC_SUCCESS) {
    // Clearing the dirty flags(used for skipping DB Diff operation) if stored
    // For TC_CONFIG_VIRTUAL, cachec is cleared with ClearVirtualKtDirtyInGlobal
    if (TC_CONFIG_VIRTUAL != config_mode) {
      dbinst->ClearDirtyTblCache(config_mode, vtnname);
    }
  }

  dbcm_->ReleaseRwConn(dbinst);
  return urc;
}

upll_rc_t UpllConfigMgr::OnClearStartupConfig(uint32_t session_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;

  if (IsShuttingDown()) {
    UPLL_LOG_WARN("Shutting down, cannot clear startup");
    return UPLL_RC_ERR_SHUTTING_DOWN;
  }

  if (!IsActiveNode()) {
    UPLL_LOG_WARN("Node is not active, cannot clear startup");
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
  }

  // If in BATCH mode, perform DB Commit
  urc = DbCommitIfInBatchMode(__FUNCTION__, kNormalTaskPriority);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("Clearing startup failed as BATCH-DB-Commit failed. Urc=%d",
                   urc);
    return urc;
  }

  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_STARTUP, ConfigLock::CFG_WRITE_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  CALL_MOMGRS_REVERSE_ORDER(ClearStartup, dbinst);

  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, (urc == UPLL_RC_SUCCESS));
  dbcm_->ReleaseRwConn(dbinst);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("ClearStartupConfig failed. Urc=%d", urc);
  }

  return urc;
}

// Note: ClearImport is part of GlobalConfigService.  It is coded here to make
// use of CALL_MOMGRS_PREORDER
upll_rc_t UpllConfigMgr::ClearImport(uint32_t session_id, uint32_t config_id,
                                     bool sby2act_trans) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = UPLL_RC_ERR_GENERIC;


  pfc::core::ScopedMutex lock(import_mutex_lock_);

  if (!sby2act_trans) {
    if (import_in_progress_ == false) {
      UPLL_LOG_INFO("Import is not in progress. Clear cannot be done");
      return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
    }
    if (UPLL_RC_SUCCESS != (urc = ValidateImport(
                session_id, config_id, import_ctrlr_id_.c_str(),
                UPLL_CLEAR_IMPORT_CONFIG_OP, UPLL_IMPORT_TYPE_FULL))) {
      return urc;
    }
  }


  // If in BATCH mode, perform DB Commit
  urc = DbCommitIfInBatchMode(__FUNCTION__, kNormalTaskPriority);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("Clearing startup failed as BATCH-DB-Commit failed. Urc=%d",
                   urc);
    return urc;
  }

  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_IMPORT, ConfigLock::CFG_WRITE_LOCK);

  DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
  if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }

  CALL_MOMGRS_REVERSE_ORDER(ImportClear, import_ctrlr_id_.c_str(), dbinst);

  upll_rc_t db_urc = dbcm_->DalTxClose(dbinst, (urc == UPLL_RC_SUCCESS));
  dbcm_->ReleaseRwConn(dbinst);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_FATAL("ClearImportConfig failed. Urc=%d", urc);
    return urc;
  }

  import_in_progress_ = false;
  import_ctrlr_id_ = "";
  import_state_progress =0;
  return urc;
}

/**
 * @brief : This function sends invalid configuration alarm to node
 * manager
 */
bool UpllConfigMgr::SendInvalidConfigAlarm(string ctrlr_name,
                                           bool assert_alarm) {
  UPLL_FUNC_TRACE;
  const string vtn_name = "";
  std::string message;
  std::string message_summary;
  pfc::alarm::alarm_info_with_key_t data;

  if (assert_alarm) {
    message.assign("Invalid configuration for ");
    message += ctrlr_name;
    message_summary = "Invalid Configuration";
    data.alarm_class = pfc::alarm::ALM_WARNING;
    data.alarm_kind = 1;   // assert alarm
    UPLL_LOG_WARN("Assert Alarm: %s", message.c_str());
  } else {
    message.assign("No invalid configuration for ");
    message += ctrlr_name;
    message_summary = "No invalid Configuration";
    data.alarm_class = pfc::alarm::ALM_NOTICE;
    data.alarm_kind = 0;   // clear alarm
    UPLL_LOG_INFO("Clear Alarm: %s", message.c_str());
  }
  data.apl_No = UNCCID_LOGICAL;
  data.alarm_key_size = ctrlr_name.length();
  data.alarm_key = const_cast<uint8_t *>(
      reinterpret_cast<const uint8_t *>(ctrlr_name.c_str()));

  data.alarm_category = UPLL_INVALID_CONFIGURATION_ALARM;

  pfc::alarm::alarm_return_code_t ret = pfc::alarm::pfc_alarm_send_with_key(
      vtn_name, message, message_summary, &data, alarm_fd);
  if (ret != pfc::alarm::ALM_OK) {
    UPLL_LOG_INFO("Failed to %s invalid configuration alarm",
                  (assert_alarm) ? "assert" : "clear");
    return false;
  }

  return true;
}

upll_rc_t UpllConfigMgr::DbCommitIfInBatchMode(const char *calling_func,
                                               TaskPriority task_priority) {
  upll_rc_t urc = UPLL_RC_SUCCESS;

  ScopedConfigLock scfg_lock(cfg_lock_, task_priority,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_IMPORT, ConfigLock::CFG_WRITE_LOCK,
                             // UPLL_DT_AUDIT, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_STARTUP, ConfigLock::CFG_WRITE_LOCK);
  pfc::core::ScopedMutex mutex(batch_mutex_lock_);

  if (batch_mode_in_progress_) {
    UPLL_LOG_DEBUG("%s: Performing DB Commit as BATCH mode is in progress",
                   calling_func);
    DalOdbcMgr *dbinst = dbcm_->GetConfigRwConn();
    if (dbinst == NULL) { return UPLL_RC_ERR_GENERIC; }
    urc = dbcm_->DalTxClose(dbinst, true);
    dbcm_->ReleaseRwConn(dbinst);
    if (urc != UPLL_RC_SUCCESS)
      UPLL_LOG_INFO("%s: BATCH mode DB Commit failed", calling_func);
      return urc;
  }
  return urc;
}

/**
 * @brief : This function triggers InvalidConfig Alarm based on
 * upll result during audit start, audit vote
 */
void UpllConfigMgr::TriggerInvalidConfigAlarm(upll_rc_t upll_ctrlr_result,
                                              string ctrlr_id) {
  // Invalid config alarm need not be raised for the
  // below specific errors since it does not indicate
  // any configuration mismatch between UNC and Controller
  if (upll_ctrlr_result == UPLL_RC_ERR_AUDIT_CANCELLED ||
      upll_ctrlr_result == UPLL_RC_ERR_CTR_DISCONNECTED ||
      upll_ctrlr_result == static_cast<upll_rc_t>(UNC_RC_CTR_BUSY)) {
    return;
  }
  bool assert_alarm = (upll_ctrlr_result ==
                       UPLL_RC_SUCCESS) ? false : true;
  bool alarm_exists = false;
  upll_rc_t ctrmgr_urc;
  if ((ctrmgr_urc = (CtrlrMgr::GetInstance()->IsConfigInvalid(
                  ctrlr_id.c_str(),
                  &alarm_exists))) != UPLL_RC_SUCCESS) {
    UPLL_LOG_WARN("Error in IsConfigInvalid(%s). Urc=%d",
                  ctrlr_id.c_str(), ctrmgr_urc);
  } else {
    if (assert_alarm) {
      UPLL_LOG_ERROR("Audit failed for %s",
                     ctrlr_id.c_str());
      if (!alarm_exists) {
        SendInvalidConfigAlarm(ctrlr_id, assert_alarm);
        CtrlrMgr::GetInstance()->UpdateInvalidConfig(
            ctrlr_id.c_str(), assert_alarm);
      }
    } else {
      UPLL_LOG_INFO("Audit successful for %s",
                    ctrlr_id.c_str());
      if (alarm_exists) {
        SendInvalidConfigAlarm(ctrlr_id, assert_alarm);
        CtrlrMgr::GetInstance()->UpdateInvalidConfig(
            ctrlr_id.c_str(), assert_alarm);
      }
    }
  }
}

upll_rc_t UpllConfigMgr::GetSystemProperty(const char *property, char *value,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  using unc::upll::dal::DalBindInfo;
  DalBindInfo bind_info(uudst::kDbiUpllSystemTbl);
  bind_info.BindMatch(uudst::upll_system_tbl::kDbiProperty,
                      uud::kDalChar, 64, property);
  bind_info.BindOutput(uudst::upll_system_tbl::kDbiValue,
                      uud::kDalChar, 128, value);

  result_code = UpllDbConnMgr::ConvertDalResultCode(
      dmi->GetSingleRecord(UPLL_DT_SYSTEM, uudst::kDbiUpllSystemTbl,
                           &bind_info));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to read value for %s, Urc=%d",
                  property, result_code);
  } else {
    UPLL_LOG_DEBUG("System property=%s, Value=%s", property, value);
  }
  return result_code;
}

upll_rc_t UpllConfigMgr::UpdateSystemProperty(const char *property,
                                              const char *value,
                                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  using unc::upll::dal::DalBindInfo;
  DalBindInfo bind_info(uudst::kDbiUpllSystemTbl);
  bind_info.BindMatch(uudst::upll_system_tbl::kDbiProperty,
                      uud::kDalChar, 64, property);
  bind_info.BindInput(uudst::upll_system_tbl::kDbiValue,
                      uud::kDalChar, 128, value);

  result_code = UpllDbConnMgr::ConvertDalResultCode(
      dmi->UpdateRecords(UPLL_DT_SYSTEM, uudst::kDbiUpllSystemTbl, &bind_info,
                         TC_CONFIG_GLOBAL, NULL));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to update the property %s with value %s, Urc=%d",
                   property, value, result_code);
  } else {
    UPLL_LOG_DEBUG("Updated system property=%s, Value=%s", property, value);
  }
  return result_code;
}

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
