/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TCLIB_VTNDRVINTF_HH_
#define _TCLIB_VTNDRVINTF_HH_

#include <uncxx/tclib/tclib_interface.hh>
#include <tclib_module.hh>
#include <kt_handler.hh>
#include <controller_utils.hh>
#include <list>
#include <string>
#include <map>

namespace unc {
namespace driver {

class DriverTxnInterface : public unc::tclib::TcLibInterface {
 public:
  typedef std::list<std::string> ::iterator ctr_iter;
  typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;
  /**
   * @brief  - DriverTxnInterface constructor
   */
  DriverTxnInterface(ControllerFramework *,
                     kt_handler_map &);
  /**
   * @brief  - DriverTxnInterface destructor
   */
  ~DriverTxnInterface();

  /**
   * @brief     - Handles commit vote request
   * @param[in] - session id
   * @param[in] - config id
   * @param[in] - driver info
   * @retval    - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleCommitVoteRequest(uint32_t session_id,
                                                  uint32_t config_id,
                                                  TcConfigMode config_mode,
                                                  std::string vtn_name,
                                                  unc::tclib::TcDriverInfoMap&
                                                  driver_info) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief      - Handles commit global commit
   * @param[in]  - session id
   * @param[in]  - config id
   * @param[out] - driver info
   * @retval     - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet  HandleCommitGlobalCommit(uint32_t session_id,
                                                    uint32_t config_id,
                                                    TcConfigMode config_mode,
                                                    std::string vtn_name,
                                                    unc::tclib::TcDriverInfoMap&
                                                    driver_info) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief     - Handles commit driver result
   * @param[in] - config id
   * @param[in] - commit phase type
   * @param[in] - driver result
   * @retval    - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleCommitDriverResult(uint32_t session_id,
                                                   uint32_t config_id,
                                                   TcConfigMode config_mode,
                                                   std::string vtn_name,
                                                   unc::tclib::TcCommitPhaseType
                                                   phase,
                                                   unc::tclib
                                                   ::TcCommitPhaseResult
                                                   driver_result) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief      - Handles Audit vote request
   * @param[in]  - session id
   * @param[in]  - controller type
   * @param[in]  - controller id
   * @param[out] - driver info
   * @retval     - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleAuditVoteRequest(uint32_t session_id,
                                                 uint32_t ctr_type,
                                                 std::string controller_id,
                                                 unc::tclib::TcDriverInfoMap&
                                                 driver_info) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief       - Handles Audit Global commit
   * @param[in]   - session id
   * @param[in]   - controller type
   * @param[in]   - controller id
   * @param[out]  - driver info
   * @param[out]  - audit result
   * @retval      - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleAuditGlobalCommit(uint32_t session_id,
                                                  uint32_t ctr_type,
                                                  std::string controller_id,
                                                  unc::tclib::TcDriverInfoMap&
                                                  driver_info,
                                                  unc::tclib::TcAuditResult&
                                                  audit_result) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief       - Handles Audit Driver Request
   * @param[in]   - session id
   * @param[in]   - controller id
   * @param[in]   - commit phase type
   * @param[in]   - driver result
   * @param[out]  - audit result
   * @retval      - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleAuditDriverResult(uint32_t session_id,
                                                  std::string controller_id,
                                                  unc::tclib::TcCommitPhaseType
                                                  phase,
                                                  unc::tclib
                                                  ::TcCommitPhaseResult
                                                  driver_result,
                                                  unc::tclib
                                                  ::TcAuditResult&
                                                  audit_result) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief      - Handles save configuration
   * @param[in]  - session id
   * @retval     - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleSaveConfiguration(uint32_t session_id,
                                                  uint64_t ver) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief       - Handles Abort candidate
   * @param[in]   - session id
   * @param[in]   - config id
   * @retval      - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleAbortCandidate(uint32_t session_id,
                                               uint32_t config_id,
                                               TcConfigMode config_mode,
                                               std::string vtn_name,
                                               uint64_t ver) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief     - Handles clear startup
   * @param[in] - session id
   * @retval    - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleClearStartup(uint32_t session_id) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief  - Handle set up
   * @retval - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleSetup() {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief   - Handle set up complete
   * @retval  - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleSetupComplete() {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief   - Retrieves the controller type
   * @retval  - returns UNC_CT_ODC
   */
  unc_keytype_ctrtype_t HandleGetControllerType(std::string controller_id) {
    return UNC_CT_ODC;
  }

  /**
   * @brief       - Handles audit config
   * @param[in]   - datatype target
   * @param[in]   - enum of TCServiceType for operation failure
   * @retval      - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleAuditConfig(unc_keytype_datatype_t db_target,
                                            TcServiceType fail_oper,
                                            TcConfigMode config_mode,
                                            std::string vtn_name,
                                            uint64_t ver) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief      - Handles commit transaction start
   * @param[in]  - session id
   * @param[in]  - config id
   * @retval     - returns TC_SUCCESS
   */
  unc::tclib::TcCommonRet HandleCommitTransactionStart(uint32_t session_id,
                                                       uint32_t config_id,
                                                       TcConfigMode config_mode,
                                                       std::string vtn_name) {
    return unc::tclib::TC_SUCCESS;
  }

  /**
   * @brief     - Handles commit transaction end
   * @param[in] - session id
   * @param[in] - config id
   * @param[in] - transaction end result
   * @retval    - returns TC_SUCCESS
   */
  unc::tclib::TcCommonRet HandleCommitTransactionEnd(uint32_t session_id,
                                                     uint32_t config_id,
                                                     TcConfigMode config_mode,
                                                     std::string vtn_name,
                                                     unc::tclib
                                                     ::TcTransEndResult
                                                     end_result) {
    return unc::tclib::TC_SUCCESS;
  }

  /**
   * @brief        - Handles COmmit Global Abort
   * @param[in]    - session id
   * @param[in]    - config id
   * @param[in]    - Tc Commit operation abort phase status
   * @retval       - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleCommitGlobalAbort(uint32_t session_id,
                                                  uint32_t config_id,
                                                  TcConfigMode config_mode,
                                                  std::string vtn_name,
                                                  unc::tclib
                                                  ::TcCommitOpAbortPhase
                                                  fail_phase) {
    return unc::tclib::TC_FAILURE;
  }

  /**
   * @brief     - Handles Audit Start
   * @param[in] - session id
   * @param[in] - controller type
   * @param[in] - controller id
   * @retval    - returns TC_SUCCESS
   */
  unc::tclib::TcCommonRet HandleAuditStart(uint32_t session_id,
                                           unc_keytype_ctrtype_t ctr_type,
                                           std::string controller_id,
                                           TcAuditType audit_type,
                                           uint64_t commit_number,
                                           uint64_t commit_date,
                                           std::string commit_application);

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
   */
  unc::tclib::TcCommonRet HandleAuditStart(uint32_t session_id,
                                           unc_keytype_ctrtype_t ctr_type,
                                           std::string controller_id,
                                           pfc_bool_t force_reconnect,
                                           TcAuditType audit_type) {
    return unc::tclib::TC_SUCCESS;
  }

  unc::tclib::TcCommonRet HandleAuditCancel(uint32_t session_id,
                                            unc_keytype_ctrtype_t ctr_type,
                                            std::string controller_id);

  /**
   * @brief       - Handles Audit end
   * @param[in]   - session id
   * @param[in]   - controller type
   * @param[in]   - controller id
   * @param[in]   - audit result
   * @retval      - returns TC_SUCCESS
   */
  unc::tclib::TcCommonRet HandleAuditEnd(uint32_t session_id,
                                         unc_keytype_ctrtype_t ctr_type,
                                         std::string
                                         controller_id,
                                         unc::tclib::TcAuditResult
                                         audit_result);

  /**
   * @brief      - Handles Audit Transaction Start
   * @param[in]  - session id
   * @param[in]  - controller type
   * @param[in]  - controller id
   * @retval     - returns TC_SUCCESS
   */
  unc::tclib::TcCommonRet HandleAuditTransactionStart(uint32_t session_id,
                                                      unc_keytype_ctrtype_t
                                                      ctr_type,
                                                      std::string
                                                      controller_id) {
    return unc::tclib::TC_SUCCESS;
  }

  /**
   * @brief     - Handles Audit Transaction End
   * @param[in] - session id
   * @param[in] - controller type
   * @param[in] - controller id
   * @param[in] - TcCommonRet enum value
   * @retval    - returns TC_SUCCESS
   */
  unc::tclib::TcCommonRet HandleAuditTransactionEnd(uint32_t session_id,
                                                    unc_keytype_ctrtype_t
                                                    ctr_type,
                                                    std::string
                                                    controller_id,
                                                    unc::tclib::TcTransEndResult
                                                    end_result) {
    return unc::tclib::TC_SUCCESS;
  }

  /**
   * @brief     -  Handles Audit Vote
   * @param[in] - session id
   * @param[in] - controller id
   * @param[in] - Tc controllers list
   * @retval    - returns TcCommonRet enum value
   */
  unc::tclib::TcCommonRet HandleAuditVoteRequest(uint32_t session_id,
                                                 std::string controller_id,
                                                 unc::tclib
                                                 ::TcControllerList
                                                 controllers);

  /**
   * @brief       - Handles Audit Global commit
   * @param[in]   - controller id
   * @param[in]   - Tc controller list
   * @retval      - returns TcCommitRet enum
   */
  unc::tclib::TcCommonRet HandleAuditGlobalCommit(uint32_t session_id,
                                                  std::string controller_id,
                                                  unc::tclib::TcControllerList
                                                  controllers);

  /**
   * @brief      - Handles Audit Global Abort
   * @param[in]  - controller type
   * @param[in]  - controller id
   * @param[in]  - Tc Audit Operation Abort Phase
   * @retval     - returns TC_FAILURE
   */
  unc::tclib::TcCommonRet HandleAuditGlobalAbort(uint32_t session_id,
                                                 unc_keytype_ctrtype_t ctr_type,
                                                 std::string controller_id,
                                                 unc::tclib::TcAuditOpAbortPhase
                                                 fail_phase) {
    return unc::tclib::TC_SUCCESS;
  }

  /**
   * @brief          - Retrieves the controller type
   * @retval         - returns UNC_CT_ODC
   */

  unc_keytype_ctrtype_t HandleGetControllerType() {
    return UNC_CT_ODC;
  }

  /**
   * @brief      - Method to handle TC vote request
   * @param[in]  - session id
   * @param[in]  - config id
   * @param[in]  - Tc controllers list
   * @retval     - TcCommonRet enum value
   */
  unc::tclib::TcCommonRet HandleCommitVoteRequest(uint32_t session_id,
                                                  uint32_t config_id,
                                                  unc::tclib::TcControllerList
                                                  controllers);

  /**
   * @brief       - Method to handle TC commit request
   * @param[in]   - session id
   * @param[in]   - config id
   * @param[in]   - Tc controllers list
   * @retval      - TcCommonRet enum value
   */
  unc::tclib::TcCommonRet HandleCommitGlobalCommit(uint32_t session_id,
                                                   uint32_t config_id,
                                                   unc::tclib::TcControllerList
                                                   controllers);
  /**
   * @brief       - Method to Abort the controller which failed in VOTE
   * @param[in]   - Tc controller list
   * @retval      - None
   */
  void AbortControllers(unc::tclib::TcControllerList
                        controllers);
  /**
   * @brief       - Method to Handle the controller cache
   * @param[in]   - controller name
   * @param[in]   - controller*
   * @param[in]   - driver*
   * @retval      - TcCommonRet enum value
   */
  unc::tclib::TcCommonRet HandleCommitCache(std::string ctr_id,
                                            controller* ctr,
                                            driver* drv);

 private:
  ControllerFramework* crtl_inst_;
  kt_handler_map kt_handler_map_;
};
}  // namespace driver
}  // namespace unc

#endif
