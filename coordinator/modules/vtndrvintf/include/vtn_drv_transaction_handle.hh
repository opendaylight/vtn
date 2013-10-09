/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the  terms of the Eclipse Public License v1.0 which
 * accompanies this  distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TCLIB_VTNDRVINTF_HH_
#define _TCLIB_VTNDRVINTF_HH_

#include <pfcxx/module.hh>
#include <uncxx/tclib/tclib_interface.hh>
#include <tclib_module.hh>
#include <unc/keytype.h>
#include <unc/tc/external/tc_services.h>
#include <controller_fw.hh>
#include <kt_handler.hh>
#include <confignode.hh>
#include <string>
#include <map>

namespace unc {
namespace driver {

class DriverTxnInterface : public unc::tclib::TcLibInterface {
 public:
  /**
   * @brief  - DriverTxnInterface constructor
   * */
  DriverTxnInterface(ControllerFramework *,
                     std::map <unc_key_type_t, KtHandler*>&);
  /**
   * @brief  - DriverTxnInterface destructor
   * */
  ~DriverTxnInterface() {}

  unc::tclib::TcCommonRet HandleCommitVoteRequest(uint32_t session_id,
                                uint32_t config_id,
                                unc::tclib::TcDriverInfoMap& driver_info) {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet  HandleCommitGlobalCommit(uint32_t session_id,
                                uint32_t config_id,
                                unc::tclib::TcDriverInfoMap& driver_info) {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet HandleCommitDriverResult(uint32_t session_id,
                                uint32_t config_id,
                                unc::tclib::TcCommitPhaseType phase,
                                unc::tclib::TcCommitPhaseResult driver_result) {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet HandleAuditVoteRequest(uint32_t session_id,
                                uint32_t ctr_type,
                                std::string controller_id,
                                unc::tclib::TcDriverInfoMap& driver_info) {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet HandleAuditGlobalCommit(uint32_t session_id,
                                uint32_t ctr_type,
                                std::string controller_id,
                                unc::tclib::TcDriverInfoMap& driver_info,
                                unc::tclib::TcAuditResult& audit_result) {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet HandleAuditDriverResult(uint32_t session_id,
                                std::string controller_id,
                                unc::tclib::TcCommitPhaseType phase,
                                unc::tclib::TcCommitPhaseResult driver_result,
                                unc::tclib::TcAuditResult& audit_result) {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet HandleSaveConfiguration(uint32_t session_id) {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet HandleAbortCandidate(uint32_t session_id,
                                               uint32_t config_id) {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet HandleClearStartup(uint32_t session_id) {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet HandleSetup() {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet HandleSetupComplete() {
    return unc::tclib::TC_FAILURE;
  }
  unc_keytype_ctrtype_t HandleGetControllerType(std::string controller_id) {
    return UNC_CT_ODC;
  }

  unc::tclib::TcCommonRet HandleAuditConfig(unc_keytype_datatype_t db_target,
                                            TcServiceType fail_oper) {
    return unc::tclib::TC_FAILURE;
  }
  unc::tclib::TcCommonRet HandleCommitTransactionStart(uint32_t session_id,
                                                       uint32_t config_id) {
    return unc::tclib::TC_SUCCESS;
  }

  unc::tclib::TcCommonRet HandleCommitTransactionEnd(uint32_t session_id,
                                   uint32_t config_id,
                                   unc::tclib::TcTransEndResult end_result) {
    return unc::tclib::TC_SUCCESS;
  }

  unc::tclib::TcCommonRet HandleCommitGlobalAbort(uint32_t session_id,
                                 uint32_t config_id,
                                 unc::tclib::TcCommitOpAbortPhase fail_phase) {
    return unc::tclib::TC_SUCCESS;
  }

  unc::tclib::TcCommonRet HandleAuditStart(uint32_t session_id,
                                           unc_keytype_ctrtype_t ctr_type,
                                           std::string controller_id) {
    return unc::tclib::TC_SUCCESS;
  }

  unc::tclib::TcCommonRet HandleAuditEnd(uint32_t session_id,
                                    unc_keytype_ctrtype_t ctr_type,
                                    std::string controller_id,
                                    unc::tclib::TcAuditResult audit_result) {
    return unc::tclib::TC_SUCCESS;
  }

  unc::tclib::TcCommonRet HandleAuditTransactionStart(uint32_t session_id,
                                     unc_keytype_ctrtype_t ctr_type,
                                     std::string controller_id) {
    return unc::tclib::TC_SUCCESS;
  }

  unc::tclib::TcCommonRet HandleAuditTransactionEnd(uint32_t session_id,
                                     unc_keytype_ctrtype_t ctr_type,
                                     std::string controller_id,
                                     unc::tclib::TcTransEndResult end_result) {
    return unc::tclib::TC_SUCCESS;
  }


  unc::tclib::TcCommonRet HandleAuditVoteRequest(uint32_t session_id,
                                     std::string controller_id,
                                    unc::tclib::TcControllerList controllers) {
    return unc::tclib::TC_SUCCESS;
  }

  unc::tclib::TcCommonRet HandleAuditGlobalCommit(uint32_t session_id,
                                   std::string controller_id,
                                   unc::tclib::TcControllerList controllers) {
    return unc::tclib::TC_SUCCESS;
  }

  unc::tclib::TcCommonRet HandleAuditGlobalAbort(uint32_t session_id,
                                   unc_keytype_ctrtype_t ctr_type,
                                   std::string controller_id,
                                   unc::tclib::TcAuditOpAbortPhase fail_phase) {
    return unc::tclib::TC_SUCCESS;
  }

  unc_keytype_ctrtype_t HandleGetControllerType() {
    return UNC_CT_ODC;
  }

  /**
   * @brief  - Method to handle TC vote request
   *  */
  unc::tclib::TcCommonRet HandleCommitVoteRequest(uint32_t session_id,
                                   uint32_t config_id,
                                   unc::tclib::TcControllerList
                                               controllers);

  /**
   * @brief  - Method to handle TC commit request
   *  */
  unc::tclib::TcCommonRet HandleCommitGlobalCommit(uint32_t session_id,
                                    uint32_t config_id,
                                    unc::tclib::TcControllerList controllers);
  /**
   * @brief  - Method to initialize map_kt_
   *  */
  void  initialize_map(void);
  /**
   * @brief  - Method to Abort the controller which failed in VOTE
   *  */
  void AbortControllers(unc::tclib::TcControllerList
                                          controllers);

 private:
  ControllerFramework* crtl_inst_;
  std::map<unc_key_type_t, pfc_ipcstdef_t*> key_map_;
  std::map<unc_key_type_t, pfc_ipcstdef_t*> val_map_;
  std::map <unc_key_type_t, KtHandler*> kt_handler_map_;
};
}  // namespace driver
}  // namespace unc

#endif
