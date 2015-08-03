/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef _UNC_TCLIB_INTERFACE_H_
#define _UNC_TCLIB_INTERFACE_H_

#include <uncxx/tclib/tclib_defs.hh>
#include <unc/tc/external/tc_services.h>
#include <string>

namespace unc {
namespace tclib {

class TcLibInterface {
  public:
  /* commit related interfaces */
  /**
   * @brief      Handling of transaction start in commit operation
   * @param[in]  session_id session on which commit request sent
   * @param[in]  config_id config id generated on acquire config mode for
   *             session_id
   * @retval     TC_SUCCESS Transaction start success
   * @retval     TC_FAILURE Transaction start failed
   */
  virtual TcCommonRet HandleCommitTransactionStart(uint32_t session_id,
                                                   uint32_t config_id,
                                                   TcConfigMode config_mode, 
                                                   std::string vtn_name)=0;

  /**
   * @brief      Handling of transaction end in commit operation
   * @param[in]  session_id session on which commit request sent
   * @param[in]  config_id config id generated on acquire config mode for
   *             session_id
   * @retval     TC_SUCCESS Transaction end success
   * @retval     TC_FAILURE Transaction end failed
   */
  virtual TcCommonRet HandleCommitTransactionEnd(uint32_t session_id,
                                                 uint32_t config_id,
                                                 TcConfigMode config_mode,
                                                 std::string vtn_name,
                                                 TcTransEndResult end_result)=0;

  /**
   * @brief      Handling of vote request for UPLL/UPPL in commit operation
   * @param[in]  session_id session on which commit request sent
   * @param[in]  config_id config id generated on acquire config mode for
   *             session_id
   * @param[out] driver_info map which contains the controller type and their
   *             assosciated controller ids
   * @retval     TC_SUCCESS vote request success 
   * @retval     TC_FAILURE vote request failed
   */
  virtual TcCommonRet HandleCommitVoteRequest(uint32_t session_id,
                                              uint32_t config_id,
                                              TcConfigMode config_mode,
                                              std::string vtn_name,
                                              TcDriverInfoMap& driver_info)=0;

  /**
   * @brief      Handling of vote request for driver modules in commit operation
   * @param[in]  session_id session on which commit request sent
   * @param[in]  config_id config id generated on acquire config mode for
   *             session_id
   * @param[in]  controllers list of controllers involved in commit operation 
   * @retval     TC_SUCCESS vote request success 
   * @retval     TC_FAILURE vote request failed
   */
  virtual TcCommonRet HandleCommitVoteRequest(uint32_t session_id,
                                              uint32_t config_id,
                                              TcControllerList controllers)=0;

  /**
   * @brief      Handling of global commit for UPLL/UPPL in commit operation
   * @param[in]  session_id session on which commit request sent
   * @param[in]  config_id config id generated on acquire config mode for
   *             session_id
   * @param[out] driver_info map which contains the controller type and their
   *             assosciated controller ids
   * @retval     TC_SUCCESS global commit success 
   * @retval     TC_FAILURE global commit failed
   */
  virtual TcCommonRet HandleCommitGlobalCommit(uint32_t session_id,
                                               uint32_t config_id,
                                               TcConfigMode config_mode,
                                               std::string vtn_name,
                                               TcDriverInfoMap& driver_info)=0;

  /**
   * @brief      Handling of global commit for driver modules in commit operation
   * @param[in]  session_id session on which commit request sent
   * @param[in]  config_id config id generated on acquire config mode for
   *             session_id
   * @param[in]  controllers list of controllers involved in commit operation 
   * @retval     TC_SUCCESS global commit success 
   * @retval     TC_FAILURE global commit failed
   */
  virtual TcCommonRet HandleCommitGlobalCommit(uint32_t session_id,
                                               uint32_t config_id,
                                               TcControllerList controllers)=0;

  /**
   * @brief      Handling of driver result for UPLL/UPPL in commit operation
   * @param[in]  session_id session on which commit request sent
   * @param[in]  config_id config id generated on acquire config mode for
   *             session_id
   * @param[in]  phase commit phase type of vote/global commit 
   * @param[in]  driver_result driver result from either vote/global commit
   *             phase
   * @retval     TC_SUCCESS driver result handling success 
   * @retval     TC_FAILURE driver result handling failed
   */
  virtual TcCommonRet HandleCommitDriverResult(uint32_t session_id,
                                               uint32_t config_id,
                                               TcConfigMode config_mode,
                                               std::string vtn_name,
                                               TcCommitPhaseType phase,
                                               TcCommitPhaseResult
                                               driver_result)=0;

  /**
   * @brief      Handling of global abort in commit operation
   * @param[in]  session_id session on which commit request sent
   * @param[in]  config_id config id generated on acquire config mode for
   *             session_id
   * @param[in]  fail_phase phase at which transaction got failed transaction
   *             start or vote request
   * @retval     TC_SUCCESS global abort handling success 
   * @retval     TC_FAILURE global abort handling failed
   */
  virtual TcCommonRet HandleCommitGlobalAbort(uint32_t session_id,
                                              uint32_t config_id,
                                              TcConfigMode config_mode,
                                              std::string vtn_name,
                                              TcCommitOpAbortPhase fail_phase)
                                              =0;


  /* audit related interfaces */
  /**
   * @brief      Handling of audit start in UPLL/UPPL for audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  ctr_type controller type openflow/overlay/legacy 
   * @param[in]  controller_id controller id intended for audit
   * @param[in]  simplified_audit if set simplified audit is to be executed
   * @param[in]  commit_number Current Commit version of PFC
   * @param[in]  commit_date Latest commit time of PFC
   * @param[in]  commit_application Application that performed commit operation
   * @retval     TC_SUCCESS audit start success
   * @retval     TC_FAILURE audit start failed
   */
  virtual TcCommonRet HandleAuditStart(uint32_t session_id,
                                       unc_keytype_ctrtype_t ctr_type,
                                       std::string controller_id,
                                       TcAuditType audit_type,
                                       uint64_t commit_number,
                                       uint64_t commit_date,
                                       std::string commit_application) = 0;

  /**
   * @brief      Handling of audit start in driver modules for audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  ctr_type controller type openflow/overlay/legacy 
   * @param[in]  controller_id controller id intended for audit
   * @param[in]  force_reconnect set to audit controller after reconnecting.
   * @retval     TC_SUCCESS audit start success
   * @retval     TC_FAILURE audit start failed
   * @retval     TC_SIMPLIFIED_AUDIT Execute simplified Audit operation
   */
  virtual TcCommonRet HandleAuditStart(uint32_t session_id,
                                       unc_keytype_ctrtype_t ctr_type,
                                       std::string controller_id,
                                       pfc_bool_t force_reconnect,
                                       TcAuditType audit_type) = 0;


  /**
   * @brief      Handling of audit end in audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  ctr_type controller type openflow/overlay/legacy 
   * @param[in]  controller_id controller id intended for audit
   * @param[in]  audit_result contains the consolidated audit result of
   *             global commit and driver reult functions from UPLL/UPPL
   * @retval     TC_SUCCESS audit end success
   * @retval     TC_FAILURE audit end failed
   */
  virtual TcCommonRet HandleAuditEnd(uint32_t session_id,
                                     unc_keytype_ctrtype_t ctr_type,
                                     std::string controller_id,
                                     TcAuditResult audit_result)=0;

  /**
   * @brief      Handling of transaction start in audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  ctr_type controller type openflow/overlay/legacy 
   * @param[in]  controller_id controller id intended for audit
   * @retval     TC_SUCCESS transaction start success
   * @retval     TC_FAILURE transaction start failed
   */
  virtual TcCommonRet HandleAuditTransactionStart(uint32_t session_id,
                                                unc_keytype_ctrtype_t ctr_type,
                                                std::string controller_id)=0;

  /**
   * @brief      Handling of transaction end in audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  ctr_type controller type openflow/overlay/legacy 
   * @param[in]  controller_id controller id intended for audit
   * @retval     TC_SUCCESS transaction end success
   * @retval     TC_FAILURE transaction end failed
   */
  virtual TcCommonRet HandleAuditTransactionEnd(uint32_t session_id,
                                                unc_keytype_ctrtype_t ctr_type,
                                                std::string controller_id,
                                                TcTransEndResult end_result)=0;

  /**
   * @brief      Handling of vote request for UPLL/UPPL in audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  ctr_type controller type openflow/overlay/legacy 
   * @param[in]  controller_id controller id intended for audit
   * @param[out] driver_info map which contains the controller type and their
   *             assosciated controller ids
   * @retval     TC_SUCCESS vote request success 
   * @retval     TC_FAILURE vote request failed
   */
  virtual TcCommonRet HandleAuditVoteRequest(uint32_t session_id,
                                             uint32_t ctr_type,
                                             std::string controller_id,
                                             TcDriverInfoMap& driver_info)=0;

  /**
   * @brief      Handling of vote request for driver modules in audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  controller_id controller id intended for audit
   * @param[in]  controllers list of controllers involved in commit operation 
   * @retval     TC_SUCCESS vote request success 
   * @retval     TC_FAILURE vote request failed
   */
  virtual TcCommonRet HandleAuditVoteRequest(uint32_t session_id,
                                             std::string controller_id,
                                             TcControllerList controllers)=0;

  /**
   * @brief      Handling of global commit for UPLL/UPPL in audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  ctr_type controller type openflow/overlay/legacy 
   * @param[in]  controller_id controller id intended for audit
   * @param[out] driver_info map which contains the controller type and their
   *             assosciated controller ids
   * @param[out] audit_result result of the audit operation to be filled by both
   *             UPLL and UPPL (TC_AUDIT_SUCCESS/TC_AUDIT_FAILURE)
   * @retval     TC_SUCCESS global commit success 
   * @retval     TC_FAILURE global commit failed
   */
  virtual TcCommonRet HandleAuditGlobalCommit(uint32_t session_id,
                                              uint32_t ctr_type,
                                              std::string controller_id,
                                              TcDriverInfoMap& driver_info,
                                              TcAuditResult& audit_result)=0;

  /**
   * @brief      Handling of global commit for driver modules in audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  controller_id controller id intended for audit
   * @param[in]  controllers list of controllers involved in commit operation 
   * @retval     TC_SUCCESS global commit success 
   * @retval     TC_FAILURE global commit failed
   */
  virtual TcCommonRet HandleAuditGlobalCommit(uint32_t session_id,
                                              std::string controller_id,
                                              TcControllerList controllers)=0;

  /**
   * @brief      Handling of driver result for UPLL/UPPL in audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  controller_id controller id intended for audit
   * @param[in]  phase commit phase type of vote/global commit 
   * @param[in]  driver_result driver result from either vote/global commit
   *             phase
   * @param[out] audit_result result of the audit operation to be filled by both
   *             UPLL and UPPL (TC_AUDIT_SUCCESS/TC_AUDIT_FAILURE)
   * @retval     TC_SUCCESS driver result handling success 
   * @retval     TC_FAILURE driver result handling failed
   */
  virtual TcCommonRet HandleAuditDriverResult(uint32_t session_id,
                                              std::string controller_id,
                                              TcCommitPhaseType phase,
                                              TcCommitPhaseResult
                                              driver_result,
                                              TcAuditResult& audit_result)=0;

  /**
   * @brief      Handling of audit cancel in driver, uppl, upll  modules 
   * @param[in]  session_id 
   *             session on which audit request sent
   *             session id will be 0 for driver audit
   * @param[in]  ctr_type controller type openflow/overlay/legacy 
   * @param[in]  controller_id controller id intended for audit
   * @retval     TC_SUCCESS audit cancel success
   * @retval     TC_FAILURE audit cancel failed
   */
  virtual TcCommonRet HandleAuditCancel(uint32_t session_id,
                                       unc_keytype_ctrtype_t ctr_type,
                                       std::string controller_id) = 0;

  /**
   * @brief      Handling of global abort in audit operation
   * @param[in]  session_id 
   *             session on which audit request sent (user audit)
   *             session id will be 0 for driver audit
   * @param[in]  ctr_type controller type openflow/overlay/legacy 
   * @param[in]  controller_id controller id intended for audit
   * @param[in]  fail_phase phase at which transaction got failed audit start/
   *             transaction start/vote request
   * @retval     TC_SUCCESS global abort handling success 
   * @retval     TC_FAILURE global abort handling failed
   */
  virtual TcCommonRet HandleAuditGlobalAbort(uint32_t session_id,
                                             unc_keytype_ctrtype_t ctr_type,
                                             std::string controller_id,
                                             TcAuditOpAbortPhase fail_phase)=0;

  /** 
   * @brief      Save configuration 
   * @param[in]  session_id session on which save configuration request sent
   * @retval     TC_SUCCESS save configuration handling success 
   * @retval     TC_FAILURE save configuration handling failed
   */
  virtual TcCommonRet HandleSaveConfiguration(uint32_t session_id,
                                              uint64_t save_version)=0;

  /** 
   * @brief      Abort candidate configuration 
   * @param[in]  session_id session on which abort configuration request sent
   * @param[in]  config_id config id generated on acquire config mode for
   *             session_id
   * @retval     TC_SUCCESS abort candidate handling success 
   * @retval     TC_FAILURE abort candidate handling failed
   */
  virtual TcCommonRet HandleAbortCandidate(uint32_t session_id,
                                           uint32_t config_id,
                                           TcConfigMode config_mode,
                                           std::string vtn_name,
                                           uint64_t abort_version)=0;

  /** 
   * @brief      Clear Startup Configuration
   * @param[in]  session_id session on which clear startup request sent
   * @retval     TC_SUCCESS clear startup handling success 
   * @retval     TC_FAILURE clear startup handling failed
   */
  virtual TcCommonRet HandleClearStartup(uint32_t session_id)=0;

  /** 
   * @brief      HandleAuditConfig DB
   * @param[in]  db_target db on which action to be taken 
   * @param[in]  fail_oper phase of failure on either commit/audit
   * @retval     TC_SUCCESS clear startup handling success 
   * @retval     TC_FAILURE clear startup handling failed
   */
  virtual TcCommonRet HandleAuditConfig(unc_keytype_datatype_t db_target,
                                        TcServiceType fail_oper,
                                        TcConfigMode config_mode,
                                        std::string vtn_name,
                                        uint64_t version)=0;

  /** 
   * @brief      Setup Configuration Message sent to UPPL at the end of startup
   *             operation to send messages to driver
   * @retval     TC_SUCCESS clear startup handling success 
   * @retval     TC_FAILURE clear startup handling failed
   */
  virtual TcCommonRet HandleSetup()=0;

  /** 
   * @brief      Setup Complete Message sent to UPPL during state changes
   * @retval     TC_SUCCESS clear startup handling success 
   * @retval     TC_FAILURE clear startup handling failed
   */
  virtual TcCommonRet HandleSetupComplete()=0;

  /** 
   * @brief      Get controller type invoked from TC to detect the controller type 
   *             for a controller
   * @param[in]  controller_id controller id intended for audit
   * @retval     openflow/overlay/legacy if controller id matches
   * @retval     UNC_CT_UNKNOWN if controller id does not belong to 
   *             any of controller type
   */
  virtual unc_keytype_ctrtype_t HandleGetControllerType
                                         (std::string controller_id)=0;

   /** 
   * @brief      Get Controller Type 
   *             Invoked from TC to detect the type of the controller
   *             Intended for the driver modules
   * @retval     openflow/overlay/legacy if controller id matches
   * @retval     none if requested for other than driver modules
   *             UPPL/UPLL modules should return UNC_CT_UNKNOWN 
   */
  virtual unc_keytype_ctrtype_t HandleGetControllerType()=0;

  virtual ~TcLibInterface() {}
};
}  // tclib
}  // unc


#endif /* _TCLIB_INTERFACE_HH */
