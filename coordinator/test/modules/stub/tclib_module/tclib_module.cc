/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "tclib_module.hh"

namespace unc {
namespace tclib {

std::map<TcLibModule::TCCommonRet,TcCommonRet> TcLibModule::method_tccommon_map;
std::map<TcLibModule::TCApiCommonRet,TcApiCommonRet> TcLibModule::method_tcapi_map;

unc_keytype_ctrtype_t TcLibModule::driverId;
unc_keytype_ctrtype_t TcLibModule::controllerType;
uint32_t TcLibModule::keyIndex;
TcLibInterface* TcLibModule::pTcLibInterface_= 0;

//static TcLibModule  theInstance(NULL);
/*
void
    TcLibModule::stub_loadtcLibModule(void) {
      pfc::core::Module::tcLib = &theInstance;
    }

void
    TcLibModule::stub_unloadtcLibModule(void) {
      pfc::core::Module::tcLib = NULL;
    }
*/
TcLibModule::TcLibModule(const pfc_modattr_t *mattr):
  pfc::core::Module(NULL) {}

TcApiCommonRet TcLibModule::TcLibRegisterHandler(TcLibInterface* handler) {
  pTcLibInterface_ = handler;
  return stub_getMappedResultCode(TcLibModule::REGISTER);
}

TcApiCommonRet TcLibModule::TcLibAuditControllerRequest
    (std::string controller_id) {
  return stub_getMappedResultCode(TcLibModule::AUDIT_CONTROLLER);
}

void TcLibModule::GetSessionAttributes(uint32_t* session_id, uint32_t* config_id) {
}

TcApiCommonRet TcLibModule::TcLibValidateUpdateMsg
         (uint32_t sessionid, uint32_t configid, TcConfigMode config_mode,
          std::string vtn_name) {
  return stub_getMappedResultCode(TcLibModule::VALIDATE_UPDATE);
}
TcApiCommonRet TcLibModule::TcLibGetConfigMode(uint32_t sessionid,
                                               uint32_t configid,
                                               TcConfigMode& config_mode,
                                               std::string& vtn_name) {
  return stub_getMappedResultCode(TcLibModule::GET_CONFIG_MODE);
}


TcApiCommonRet TcLibModule::TcLibReadKeyValueDataInfo(std::string controller_id,
                                                      uint32_t err_pos,
                                                      uint32_t key_type,
                                                      pfc_ipcstdef_t key_def,
                                                      pfc_ipcstdef_t value_def,
                                                      void* key_data,
                                                      void* value_data) {
  return stub_getMappedResultCode(TcLibModule::READ_KEY_VAL);
}

TcApiCommonRet TcLibModule::TcLibWriteControllerInfo(std::string controller_id,
                                                     uint32_t response_code,
                                                     uint32_t num_of_errors) {
  return stub_getMappedResultCode(TcLibModule::WRITE_CONTROLLER);
}

TcApiCommonRet TcLibModule::TcLibWriteKeyValueDataInfo(std::string
                                                       controller_id,
                                                       uint32_t key_type,
                                                       pfc_ipcstdef_t key_def,
                                                       pfc_ipcstdef_t value_def,
                                                       void* key_data,
                                                       void* value_data) {
  return stub_getMappedResultCode(TcLibModule::WRITE_KEY_VALUE);
}

TcCommonRet TcLibModule::ValidateOperTypeSequence(TcMsgOperType oper_type) {
  return stub_getMappedResultCode(TcLibModule::VALIDATE_OPER_TYPE);
}


TcCommonRet TcLibModule::ValidateCommitOperSequence(TcMsgOperType oper_type) {
  return stub_getMappedResultCode(TcLibModule::VALIDATE_COMMIT_OPER);
}


TcCommonRet TcLibModule::TcLibModule::ValidateAuditOperSequence
                (TcMsgOperType oper_type) {
  return stub_getMappedResultCode(TcLibModule::VALIDATE_AUDIT_OPER);
}


TcCommonRet TcLibModule::ValidateUpllUpplCommitSequence
              (TcMsgOperType oper_type) {
  return stub_getMappedResultCode(TcLibModule::VALIDATE_UPLL_COMMIT);
}


TcCommonRet TcLibModule::ValidateDriverCommitSequence(TcMsgOperType oper_type) {
  return stub_getMappedResultCode(TcLibModule::VALIDATE_DRIVER_COMMIT);
}


TcCommonRet TcLibModule::ValidateUpllUpplAuditSequence
                 (TcMsgOperType oper_type) {
  return stub_getMappedResultCode(TcLibModule::VALIDATE_UPLL_AUDIT);
}


TcCommonRet TcLibModule::ValidateDriverAuditSequence(TcMsgOperType oper_type) {
  return stub_getMappedResultCode(TcLibModule::VALIDATE_DRIVER_AUDIT);
}


TcApiCommonRet TcLibModule::IsReadKeyValueAllowed() {
  return stub_getMappedResultCode(TcLibModule::IS_READ_KEY);
}

TcApiCommonRet TcLibModule::IsWriteKeyValueAllowed() {
  return stub_getMappedResultCode(TcLibModule::IS_WRITE_KEY);
}


TcCommonRet TcLibModule::UpdateControllerKeyList() {
  return stub_getMappedResultCode(TcLibModule::UPDATE_CONTROLLER_KEY);
}

TcCommonRet TcLibModule::NotifySessionConfig() {
  return stub_getMappedResultCode(TcLibModule::NOTIFY_SESSION_CONFIG);
}


TcCommonRet TcLibModule::CommitTransStartEnd(TcMsgOperType oper_type,
                                TcCommitTransactionMsg commit_trans_msg) {
  return stub_getMappedResultCode(TcLibModule::COMMIT_TRANS_START);
}


TcCommonRet TcLibModule::CommitVoteGlobal(TcMsgOperType oper_type,
                                 TcCommitTransactionMsg commit_trans_msg) {
  return stub_getMappedResultCode(TcLibModule::COMMIT_VOTE_GLOBAL);
}


TcCommonRet TcLibModule::CommitTransaction() {
  return stub_getMappedResultCode(TcLibModule::COMMIT_TRANS);
}


TcCommonRet TcLibModule::CommitDriverVoteGlobal() {
  return stub_getMappedResultCode(TcLibModule::COMMIT_DRIVER_VOTE);
}


TcCommonRet TcLibModule::CommitDriverResult() {
  return stub_getMappedResultCode(TcLibModule::COMMIT_DRIVER_RESULT);
}


TcCommonRet TcLibModule::CommitGlobalAbort() {
  return stub_getMappedResultCode(TcLibModule::COMMIT_GLOBAL_ABORT);
}

TcCommonRet TcLibModule::AuditTransStartEnd(TcMsgOperType oper_type,
                                    TcAuditTransactionMsg audit_trans_msg) {
  return stub_getMappedResultCode(TcLibModule::AUDIT_TRANS_START);
}

TcCommonRet TcLibModule::AuditVoteGlobal(TcMsgOperType oper_type,
                                 TcAuditTransactionMsg audit_trans_msg) {
  return stub_getMappedResultCode(TcLibModule::AUDIT_VOTE_GLOBAL);
}

TcCommonRet TcLibModule::AuditTransaction() {
  return stub_getMappedResultCode(TcLibModule::AUDIT_TRANS);
}

TcCommonRet TcLibModule::AuditDriverVoteGlobal() {
  return stub_getMappedResultCode(TcLibModule::AUDIT_DRIVER_VOTE);
}

TcCommonRet TcLibModule::AuditDriverResult() {
  return stub_getMappedResultCode(TcLibModule::AUDIT_DRIVER_RESULT);
}

TcCommonRet TcLibModule::AuditGlobalAbort() {
  return stub_getMappedResultCode(TcLibModule::AUDIT_GLOBAL_ABORT);
}

void ReleaseTransactionResources() {
}

TcCommonRet TcLibModule::SaveConfiguaration() {
  return stub_getMappedResultCode(TcLibModule::SAVE_CONFIGURATION);
}


TcCommonRet TcLibModule::ClearStartup() {
  return stub_getMappedResultCode(TcLibModule::CLEAR_START_UP);
}

TcCommonRet TcLibModule::AbortCandidate() {
  return stub_getMappedResultCode(TcLibModule::ABORT_CANDIDATE);
}

TcCommonRet TcLibModule::AuditConfig() {
  return stub_getMappedResultCode(TcLibModule::AUDIT_CONFIG);
}

TcCommonRet TcLibModule::Setup()  {
  return stub_getMappedResultCode(TcLibModule::SET_UP);
}

TcCommonRet TcLibModule::SetupComplete() {
  return stub_getMappedResultCode(TcLibModule::SET_UP_COMPLETE);
}

unc_keytype_ctrtype_t TcLibModule::GetDriverId() {
  return driverId;
}

unc_keytype_ctrtype_t TcLibModule::GetControllerType() {
  return controllerType;
}

TcApiCommonRet TcLibModule::GetKeyIndex(std::string controller_id,
                                        uint32_t err_pos,
                                        uint32_t &key_index) {
  return stub_getMappedResultCode(TcLibModule::KEY_INDEX);
}

TcApiCommonRet TcLibModule::stub_getMappedResultCode
                         (TcLibModule::TCApiCommonRet methodType) {
  if (0 != method_tcapi_map.count(methodType)) {
    return method_tcapi_map[methodType];
  }
  return TC_API_COMMON_FAILURE;
}

TcCommonRet TcLibModule::stub_getMappedResultCode
                               (TcLibModule::TCCommonRet methodType) {
  if (0 != method_tccommon_map.count(methodType)) {
    return method_tccommon_map[methodType];
  }
  return TC_FAILURE;
}
}  // namespace tclib
}  // namespace unc
