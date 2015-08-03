/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _INCLUDE_TCLIB_INTF_STUB_HH_
#define _INCLUDE_TCLIB_INTF_STUB_HH_

#include <uncxx/tclib/tclib_interface.hh>
#include <string>

using namespace std;

namespace unc {
namespace tclib {

class TcLibInterfaceStub : public TcLibInterface {
 public:
  TcLibInterfaceStub();
  /**
   * @Description : This function will be called back when TC sends user
   *                commit-transaction start request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleCommitTransactionStart(uint32_t config_id,
                                           uint32_t session_id, TcConfigMode config_mode,
                                                       std::string vtn_name) {
    if (tclib_stub_failure_ == PFC_TRUE)
      return TC_FAILURE;

    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                Audit-transaction start request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleAuditTransactionStart(uint32_t session_id,
                                          unc_keytype_ctrtype_t driver_id,
                                          string controller_id) {
    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                commit-transaction end request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleCommitTransactionEnd(uint32_t config_id,
                                         uint32_t session_id,
                                         TcConfigMode config_mode,
                                         std::string vtn_name,
                                         TcTransEndResult end_result) {
    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                Audit-transaction End request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleAuditTransactionEnd(uint32_t session_id,
                                        unc_keytype_ctrtype_t driver_id,
                                        string controller_id,
                                        TcTransEndResult end_result) {
    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                commit vote request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleCommitVoteRequest(uint32_t session_id,
                                      uint32_t config_id,
                                      TcConfigMode config_mode,
                                      std::string vtn_name,
                                      TcDriverInfoMap& driver_info);
  /*
     {
     return TC_SUCCESS;
     }
     */
  /**
   * @Description : This function will be called back when TC sends user
   *                Audit vote request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleAuditVoteRequest(uint32_t session_id,
                                     uint32_t driver_id,
                                     string controller_id,
                                     TcDriverInfoMap &driver_info);
  /*{
    return TC_SUCCESS;
    }*/

  /**
   * @Description : This function will be called back when TC sends user
   *                Global commit request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleCommitGlobalCommit(uint32_t config_id,
                                       uint32_t session_id,
                                       TcConfigMode config_mode,
                                       std::string vtn_name,
                                       TcDriverInfoMap& driver_info) {
    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                Audit Global Commit request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleAuditGlobalCommit(uint32_t session_id,
                                      uint32_t driver_id,
                                      string controller_id,
                                      TcDriverInfoMap &driver_info,
                                      TcAuditResult& audit_result);
  /*{
    return TC_SUCCESS;
    }*/

  /**
   * @Description : This function will be called back when TC sends user
   *                Driver Result to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleCommitDriverResult(uint32_t session_id,
                                       uint32_t config_id,
                                       TcConfigMode config_mode,
                                       std::string vtn_name,
                                       TcCommitPhaseType commitphase,
                                       TcCommitPhaseResult driver_result) {
    if (tclib_stub_failure_ == PFC_TRUE)
      return TC_FAILURE;

    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                Driver Result to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleAuditDriverResult(uint32_t session_id,
                                      string controller_id,
                                      TcCommitPhaseType commitphase,
                                      TcCommitPhaseResult driver_result,
                                      TcAuditResult& audit_result) {
    if (tclib_stub_failure_ == PFC_TRUE)
      return TC_FAILURE;

    audit_result = TC_AUDIT_FAILURE;
    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                Audit Start request to UPPL/UPLL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleAuditStart(uint32_t session_id,
                               unc_keytype_ctrtype_t driver_id,
                               string controller_id,
                               TcAuditType audit_type,
                               uint64_t commit_number,
                               uint64_t commit_date,
                               std::string commit_application){
    if(tclib_stub_failure_ == PFC_TRUE) {
      return TC_FAILURE;
    }
    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                Audit Start request to driver modules
   *                This is a virtual function in TCLib
   */
  TcCommonRet HandleAuditStart(uint32_t session_id,
                               unc_keytype_ctrtype_t driver_id,
                               string controller_id,
                               pfc_bool_t force_reconnect,
                               TcAuditType audit_type) {
    if (tclib_stub_failure_ == PFC_TRUE) {
      return TC_FAILURE;
    }
    return TC_SUCCESS;
  }

 TcCommonRet HandleAuditCancel(uint32_t session_id,
                               unc_keytype_ctrtype_t ctr_type,
                               std::string controller_id) {
 if (tclib_stub_failure_ == PFC_TRUE) {
          return TC_FAILURE;
        }
        return TC_SUCCESS;
}
  /**
   * @Description : This function will be called back when TC sends user
   *                Audit End request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleAuditEnd(uint32_t session_id,
                             unc_keytype_ctrtype_t driver_id,
                             string controller_id,
                             TcAuditResult audit_result) {
    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                save configuration request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleSaveConfiguration(uint32_t session_id, uint64_t save_version) {
    if (tclib_stub_failure_ == PFC_TRUE)
      return TC_FAILURE;

    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                clear startup request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleClearStartUp(uint32_t session_id) {
    if (tclib_stub_failure_ == PFC_TRUE)
      return TC_FAILURE;

    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends user
   *                Abort Candidate request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleAbortCandidate(uint32_t session_id, uint32_t config_id,
                                   TcConfigMode config_mode,
                                   std::string vtn_name,
                                   uint64_t abort_version) {
    if (tclib_stub_failure_ == PFC_TRUE)
      return TC_FAILURE;

    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends
   *                db recovery requests during failover
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleAuditConfig(unc_keytype_datatype_t db_target,
                                TcServiceType fail_oper,
                                TcConfigMode config_mode,
                                std::string vtn_name,
                                uint64_t version) {
    if (tclib_stub_failure_ == PFC_TRUE)
      return TC_FAILURE;
    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends
   *                abort during transaction phase
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleGlobalAbort(uint32_t session_id, uint32_t config_id,
                                TcCommitPhaseType operation_phase) {
    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends
   *                abort during audit transaction phase
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleAuditGlobalAbort(uint32_t session_id,
                                     unc_keytype_ctrtype_t driver_id,
                                     string controller_id,
                                     TcAuditOpAbortPhase operation_phase);
  /*{
    return TC_SUCCESS;
    }*/

  /**
   * @Description : This function will be called back when TC sends
   *                load startup configuration request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleSetup() {
    if (tclib_stub_failure_ == PFC_TRUE)
      return TC_FAILURE;
    return TC_SUCCESS;
  }

  /**
   * @Description : This function will be called back when TC sends
   *                load startup configuration request to UPPL
   *                This is a virtual function in TCLib
   */

  TcCommonRet HandleSetupComplete() {
    if (tclib_stub_failure_ == PFC_TRUE)
      return TC_FAILURE;
    return TC_SUCCESS;
  }

  /**
   * @Description : This is a dummy function. TcLibInterface has pure virtual
   *                functions. All functions has to have an implementiation to
   *                avoid runtime conflicts
   */

  TcCommonRet HandleCommitVoteRequest(uint32_t session_id, uint32_t config_id,
                                      TcControllerList controllers) {
    return TC_SUCCESS;
  }
  /**
   * @Description : This is a dummy function. TcLibInterface has pure virtual
   *                functions. All functions has to have an implementiation to
   *                avoid runtime conflicts
   */

  TcCommonRet HandleCommitGlobalCommit(uint32_t session_id, uint32_t config_id,
                                       TcControllerList controllers) {
    return TC_SUCCESS;
  }
  /**
   * @Description : This is a dummy function. TcLibInterface has pure virtual
   *                functions. All functions has to have an implementiation to
   *                avoid runtime conflicts
   */

  TcCommonRet HandleCommitGlobalAbort(uint32_t session_id, uint32_t config_id,
                                      TcConfigMode config_mode,
                                      std::string vtn_name,
                                      TcCommitOpAbortPhase fail_phase);
  /*{
    return TC_SUCCESS;
    }*/
  /**
   * @Description : This is a dummy function. TcLibInterface has pure virtual
   *                functions. All functions has to have an implementiation to
   *                avoid runtime conflicts
   */
  inline
      TcCommonRet HandleAuditVoteRequest(uint32_t session_id,
                                         string controller_id,
                                         TcControllerList controllers) {
        return TC_SUCCESS;
      }
  /**
   * @Description : This is a dummy function. TcLibInterface has pure virtual
   *                functions. All functions has to have an implementiation to
   *                avoid runtime conflicts
   */

  TcCommonRet HandleAuditGlobalCommit(uint32_t session_id, string controller_id,
                                      TcControllerList controllers) {
    return TC_SUCCESS;
  }
  /**
   * @Description : This is a dummy function. TcLibInterface has pure virtual
   *                functions. All functions has to have an implementiation to
   *                avoid runtime conflicts
   */

  // TODO(phycore) : to be implemented
  TcCommonRet HandleClearStartup(uint32_t session_id) {
    if (tclib_stub_failure_ == PFC_TRUE)
      return TC_FAILURE;

    return TC_SUCCESS;
  }
  /**
   * @Description : This is a dummy function. TcLibInterface has pure virtual
   *                functions. All functions has to have an implementiation to
   *                avoid runtime conflicts
   */

  // TODO(phycore) : to be implemented
  unc_keytype_ctrtype_t HandleGetControllerType(std::string controller_id) {
    return UNC_CT_PFC;
  }
  /**
   * @Description : This is a dummy function. TcLibInterface has pure virtual
   *                functions. All functions has to have an implementiation to
   *                avoid runtime conflicts
   */

  unc_keytype_ctrtype_t HandleGetControllerType();

  unc_keytype_ctrtype_t ctr_type;
  pfc_bool_t fill_driver_info_;
  pfc_bool_t tclib_stub_failure_;
};

}  // namespace tclib
}  // namespace unc

#endif
