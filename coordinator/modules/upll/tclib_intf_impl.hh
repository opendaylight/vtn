/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef UPLL_TCLIB_INTF_IMPL_HH_
#define UPLL_TCLIB_INTF_IMPL_HH_

#include <string>
#include <set>
#include <list>

#include "cxx/pfcxx/synch.hh"

#include "unc/keytype.h"
#include "unc/upll_errno.h"

#include "uncxx/tclib/tclib_defs.hh"
#include "uncxx/tclib/tclib_interface.hh"

#include "momgr_intf.hh"

namespace unc {
namespace upll {
namespace config_momgr {

class UpllConfigMgr;

using unc::tclib::TcLibInterface;
using unc::tclib::TcCommonRet;
using unc::tclib::TcDriverInfoMap;
using unc::tclib::TcCommitPhaseType;
using unc::tclib::TcCommitPhaseResult;
using unc::tclib::TcControllerList;
using unc::tclib::TcCommitOpAbortPhase;
using unc::tclib::TcAuditOpAbortPhase;
using unc::tclib::TcTransEndResult;
using unc::tclib::TcAuditResult;


class TcLibIntfImpl : public unc::tclib::TcLibInterface {
 public:
  static const char * const kUpllCtrlrId;

  explicit TcLibIntfImpl(UpllConfigMgr *ucm);
  virtual ~TcLibIntfImpl();
  /* commit related interfaces */
  virtual TcCommonRet HandleCommitTransactionStart(uint32_t session_id,
                      uint32_t config_id, TcConfigMode config_mode,
                      std::string vtn_name);
  virtual TcCommonRet HandleCommitTransactionEnd(uint32_t session_id,
                      uint32_t config_id, TcConfigMode config_mode,
                      std::string vtn_name, TcTransEndResult end_result);
  virtual TcCommonRet HandleCommitVoteRequest(
      uint32_t session_id, uint32_t config_id,
      TcConfigMode config_mode, std::string vtn_name,
      TcDriverInfoMap& driver_info);
  /* Below function For Driver Modules */
  virtual TcCommonRet HandleCommitVoteRequest(
      uint32_t session_id, uint32_t config_id, TcControllerList controllers) {
    PFC_ASSERT(0);
    return unc::tclib::TC_FAILURE;
  }
  virtual TcCommonRet HandleCommitGlobalCommit(
      uint32_t session_id, uint32_t config_id,
      TcConfigMode config_mode, std::string vtn_name,
      TcDriverInfoMap& driver_info);

  /* Below function For Driver Modules */
  virtual TcCommonRet HandleCommitGlobalCommit(
      uint32_t session_id, uint32_t config_id, TcControllerList controllers) {
    PFC_ASSERT(0);
    return unc::tclib::TC_FAILURE;
  }

  virtual TcCommonRet HandleCommonTxDriverResult(
      uint32_t session_id, uint32_t config_id,
      TcConfigMode config_mode, std::string vtn_name,
      TcCommitPhaseType phase, TcCommitPhaseResult driver_result);

  virtual TcCommonRet HandleCommitDriverResult(
      uint32_t session_id, uint32_t config_id,
      TcConfigMode config_mode, std::string vtn_name,
      TcCommitPhaseType phase, TcCommitPhaseResult driver_result);

  virtual TcCommonRet HandleCommitGlobalAbort(
      uint32_t session_id, uint32_t config_id,
      TcConfigMode config_mode, std::string vtn_name,
      TcCommitOpAbortPhase fail_phase);


  /* audit related interfaces */
  virtual TcCommonRet HandleAuditStart(
      uint32_t session_id, unc_keytype_ctrtype_t ctr_type,
      std::string controller_id, TcAuditType audit_type,
      uint64_t commit_number, uint64_t commit_date,
      std::string commit_application);

  virtual TcCommonRet HandleAuditStart(
      uint32_t session_id, unc_keytype_ctrtype_t ctr_type,
      std::string controller_id, pfc_bool_t force_reconnect,
      TcAuditType audit_type) {
    PFC_ASSERT(0);  // This function should never have been called
    return unc::tclib::TC_FAILURE;
  }

  virtual TcCommonRet HandleAuditEnd(
      uint32_t session_id, unc_keytype_ctrtype_t ctr_type,
      std::string controller_id, TcAuditResult audit_result);

  virtual TcCommonRet HandleAuditCancel(uint32_t session_id,
                                       unc_keytype_ctrtype_t ctr_type,
                                       std::string controller_id);

  virtual TcCommonRet HandleAuditTransactionStart(
      uint32_t session_id, unc_keytype_ctrtype_t ctr_type,
      std::string controller_id);

  virtual TcCommonRet HandleAuditTransactionEnd(
      uint32_t session_id, unc_keytype_ctrtype_t ctr_type,
      std::string controller_id, TcTransEndResult end_result);

  virtual TcCommonRet HandleAuditVoteRequest(
      uint32_t session_id, uint32_t driver_id, std::string controller_id,
      TcDriverInfoMap& driver_info);

  /* Below function For Driver Modules */
  virtual TcCommonRet HandleAuditVoteRequest(
      uint32_t session_id, std::string controller_id,
      TcControllerList controllers) {
    PFC_ASSERT(0);  // This function should never have been called
    return unc::tclib::TC_FAILURE;
  }

  virtual TcCommonRet HandleAuditGlobalCommit(
      uint32_t session_id, uint32_t driver_id, std::string controller_id,
      TcDriverInfoMap& driver_info, TcAuditResult& audit_result);

  /* Below function For Driver Modules */
  virtual TcCommonRet HandleAuditGlobalCommit(
      uint32_t session_id, std::string controller_id,
      TcControllerList controllers) {
    PFC_ASSERT(0);  // This function should never have been called
    return unc::tclib::TC_FAILURE;
  }

  virtual TcCommonRet HandleAuditDriverResult(
      uint32_t session_id, std::string controller_id, TcCommitPhaseType phase,
      TcCommitPhaseResult driver_result, TcAuditResult &audit_result);
  virtual TcCommonRet HandleAuditGlobalAbort(
      uint32_t session_id, unc_keytype_ctrtype_t ctr_type,
      std::string controller_id, TcAuditOpAbortPhase fail_phase);

  /**
   * @brief Save Configuration
   */
  virtual TcCommonRet HandleSaveConfiguration(uint32_t session_id,
                                              uint64_t version_no);

  /**
   * @brief Abort Candidate Configuration
   */
  virtual TcCommonRet HandleAbortCandidate(uint32_t session_id,
                                           uint32_t config_id,
                                           TcConfigMode config_mode,
                                           std::string vtn_name,
                                           uint64_t version_no);

  /**
   * @brief Clear Startup Configuration
   */
  virtual TcCommonRet HandleClearStartup(uint32_t session_id);

  /**
   * @brief HandleAuditConfig DB
   */
  virtual TcCommonRet HandleAuditConfig(unc_keytype_datatype_t db_target,
                                        TcServiceType fail_oper,
                                        TcConfigMode config_mode,
                                        std::string vtn_name,
                                        uint64_t version_no);
  /**
   * @brief Setup Configuration
   * Message sent to UPPL at the end of startup operation to send messages to
   * driver
   */
  virtual TcCommonRet HandleSetup();

  /**
   * @brief Setup Complete
   * Message sent to UPPL during state changes
   */
  virtual TcCommonRet HandleSetupComplete();

  /**
   * @brief Get Driver Id
   * Invoked from TC to detect the driver id for a controller
   */
  // virtual unc_keytype_ctrtype_t HandleGetDriverId(std::string controller_id);

  /**
   * @brief      Get controller type invoked from TC to detect the controller
   *             type for a controller
   * @param[in]  controller_id controller id intended for audit
   * @retval     openflow/overlay/legacy if controller id matches
   * @retval     UNC_CT_UNKNOWN if controller id does not belong to
   *             any of controller type
   */
  virtual unc_keytype_ctrtype_t HandleGetControllerType(
      std::string controller_id);

  /**
   * @brief Get Controller Type
   * Invoked from TC to detect the type of the controller
   * Intended for the driver modules
   */
  virtual unc_keytype_ctrtype_t HandleGetControllerType();

  inline void set_shutting_down(bool shutdown) {
    sys_state_rwlock_.wrlock();
    shutting_down_ = shutdown;
    sys_state_rwlock_.unlock();
  }

  inline bool IsShuttingDown() {
    bool state;
    sys_state_rwlock_.rdlock();
    state = shutting_down_;
    sys_state_rwlock_.unlock();
    return state;
  }

  void SetClusterState(bool active) {
    sys_state_rwlock_.wrlock();
    node_active_ = active;
    sys_state_rwlock_.unlock();
  }

  bool IsActiveNode() {
    bool state;
    sys_state_rwlock_.rdlock();
    state = node_active_;
    sys_state_rwlock_.unlock();
    return state;
  }

 private:
  static upll_rc_t FillTcDriverInfoMap(TcDriverInfoMap *driver_info,
                                       const std::set<std::string> *ctrlr_set,
                                       bool audit);
  static bool WriteBackTxResult(const std::list<CtrlrTxResult*> &tx_res_list);
  static bool GetTxKtResult(const std::string &ctrlr_id,
                            const std::list<uint32_t> *key_list,
                            ConfigKeyVal **err_ckv);
  static bool GetTxResult(const TcCommitPhaseResult *driver_result,
                          std::list<CtrlrTxResult*> *tx_res_list);
  static upll_rc_t DriverResultCodeToTxURC(unc_keytype_ctrtype_t ctrlr_type,
                                           int driver_result_code);
  static void WriteUpllErrorBlock(upll_rc_t *urc);

  UpllConfigMgr *ucm_;
  uint32_t session_id_;
  uint32_t config_id_;
  bool node_active_;
  bool shutting_down_;
  pfc::core::ReadWriteLock sys_state_rwlock_;
};
                                                                       // NOLINT
}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
                                                                       // NOLINT
#endif  // UPLL_TCLIB_INTF_IMPL_HH_
