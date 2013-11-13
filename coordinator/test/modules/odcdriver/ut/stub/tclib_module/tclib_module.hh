/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef _UNC_TCLIB_MODULE_HH_
#define _UNC_TCLIB_MODULE_HH_

#include <pfcxx/module.hh>
#include <pfcxx/synch.hh>
#include <uncxx/tclib/tclib_interface.hh>
#include <tclib_struct_defs.hh>
#include <string>
#include <map>

namespace unc {
namespace tclib {

class TcLibInterface;

class TcLibModule : public pfc::core::Module {
 public:
  enum TCApiCommonRet {
    REGISTER,
    AUDIT_CONTROLLER,
    VALIDATE_UPDATE,
    READ_KEY_VAL,
    WRITE_CONTROLLER,
    WRITE_KEY_VALUE,
    IS_READ_KEY,
    IS_WRITE_KEY,
    KEY_INDEX,
  };

  enum TCCommonRet {
    VALIDATE_OPER_TYPE,
    VALIDATE_COMMIT_OPER,
    VALIDATE_AUDIT_OPER,
    VALIDATE_UPLL_COMMIT,
    VALIDATE_DRIVER_COMMIT,
    VALIDATE_UPLL_AUDIT,
    VALIDATE_DRIVER_AUDIT,
    UPDATE_CONTROLLER_KEY,
    NOTIFY_SESSION_CONFIG,
    COMMIT_TRANS_START,
    COMMIT_VOTE_GLOBAL,
    COMMIT_TRANS,
    COMMIT_DRIVER_VOTE,
    COMMIT_DRIVER_RESULT,
    COMMIT_GLOBAL_ABORT,
    AUDIT_TRANS_START,
    AUDIT_VOTE_GLOBAL,
    AUDIT_TRANS,
    AUDIT_DRIVER_VOTE,
    AUDIT_DRIVER_RESULT,
    AUDIT_GLOBAL_ABORT,
    SAVE_CONFIGURATION,
    CLEAR_START_UP,
    ABORT_CANDIDATE,
    AUDIT_CONFIG,
    SET_UP,
    SET_UP_COMPLETE,
  };

  explicit TcLibModule(const pfc_modattr_t *mattr) : pfc::core::Module(mattr) {
  }

  ~TcLibModule() {
  }

  pfc_bool_t init() {
    return true;
  }

  pfc_bool_t fini() {
    return true;
  }

  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession& sess,
                           pfc_ipcid_t service) {
    return pfc_ipcresp_t();
  }

  TcApiCommonRet TcLibRegisterHandler(TcLibInterface* handler);

  TcApiCommonRet TcLibAuditControllerRequest(std::string controller_id);

  TcApiCommonRet TcLibValidateUpdateMsg(uint32_t sessionid, uint32_t configid);

  TcApiCommonRet TcLibReadKeyValueDataInfo(std::string controller_id,
                                           uint32_t err_pos,
                                           uint32_t key_type,
                                           pfc_ipcstdef_t key_def,
                                           pfc_ipcstdef_t value_def,
                                           void* key_data,
                                           void* value_data);

  TcApiCommonRet TcLibWriteControllerInfo(std::string controller_id,
                                          uint32_t response_code,
                                          uint32_t num_of_errors);
  TcApiCommonRet TcLibWriteKeyValueDataInfo(std::string controller_id,
                                            uint32_t key_type,
                                            pfc_ipcstdef_t key_def,
                                            pfc_ipcstdef_t value_def,
                                            void* key_data,
                                            void* value_data);

 private:
  TcCommonRet ValidateOperTypeSequence(TcMsgOperType oper_type);

  TcCommonRet ValidateCommitOperSequence(TcMsgOperType oper_type);

  TcCommonRet ValidateAuditOperSequence(TcMsgOperType oper_type);

  TcCommonRet ValidateUpllUpplCommitSequence(TcMsgOperType oper_type);

  TcCommonRet ValidateDriverCommitSequence(TcMsgOperType oper_type);

  TcCommonRet ValidateUpllUpplAuditSequence(TcMsgOperType oper_type);

  TcCommonRet ValidateDriverAuditSequence(TcMsgOperType oper_type);

  TcApiCommonRet IsReadKeyValueAllowed();

  TcApiCommonRet IsWriteKeyValueAllowed();

  TcCommonRet UpdateControllerKeyList();

  TcCommonRet NotifySessionConfig();

  TcCommonRet CommitTransStartEnd(TcMsgOperType oper_type,
                                  TcCommitTransactionMsg commit_trans_msg);

  TcCommonRet CommitVoteGlobal(TcMsgOperType oper_type,
                               TcCommitTransactionMsg commit_trans_msg);

  TcCommonRet CommitTransaction();

  TcCommonRet CommitDriverVoteGlobal();

  TcCommonRet CommitDriverResult();

  TcCommonRet CommitGlobalAbort();

  TcCommonRet AuditTransStartEnd(TcMsgOperType oper_type,
                                 TcAuditTransactionMsg audit_trans_msg);

  TcCommonRet AuditVoteGlobal(TcMsgOperType oper_type,
                              TcAuditTransactionMsg audit_trans_msg);

  TcCommonRet AuditTransaction();

  TcCommonRet AuditDriverVoteGlobal();

  TcCommonRet AuditDriverResult();

  TcCommonRet AuditGlobalAbort();

  void ReleaseTransactionResources();

  TcCommonRet SaveConfiguaration();

  TcCommonRet ClearStartup();

  TcCommonRet AbortCandidate();

  TcCommonRet AuditConfig();

  TcCommonRet Setup();

  TcCommonRet SetupComplete();

  unc_keytype_ctrtype_t GetDriverId();

  unc_keytype_ctrtype_t GetControllerType();

  TcApiCommonRet GetKeyIndex(std::string controller_id,
                             uint32_t err_pos,
                             uint32_t &key_index);

  static inline void Stub_setDriverId(unc_keytype_ctrtype_t driverid ) {
    driverId = driverid;
  }

  static inline void stub_setControllerType(unc_keytype_ctrtype_t cntrlrType) {
    controllerType = cntrlrType;
  }

  static inline void stub_setKeyIndex(uint32_t index) {
    keyIndex = index;
  }

  static void stub_setTcCommonRetcode(TcLibModule::TCCommonRet methodType,
                                      TcCommonRet res_code) {
    method_tccommon_map.insert(std::make_pair(methodType, res_code));
  }

  static void stub_setTCApiCommonRetcode(TcLibModule::TCApiCommonRet methodType,
                                         TcApiCommonRet res_code) {
    method_tcapi_map.insert(std::make_pair(methodType, res_code));
  }

  static void stub_loadtcLibModule(void);
  static void stub_unloadtcLibModule(void);

  static void stub_clearTcLibStubData() {
    method_tccommon_map.clear();
    method_tcapi_map.clear();
  }

  static TcApiCommonRet stub_getMappedResultCode(TcLibModule::TCApiCommonRet);
  static TcCommonRet stub_getMappedResultCode(TcLibModule::TCCommonRet);

  static std::map<TcLibModule::TCCommonRet, TcCommonRet> method_tccommon_map;
  static std::map<TcLibModule::TCApiCommonRet, TcApiCommonRet> method_tcapi_map;

  static TcLibInterface *pTcLibInterface_;

  static unc_keytype_ctrtype_t driverId;
  static unc_keytype_ctrtype_t controllerType;
  static uint32_t keyIndex;
};
}  // namespace tclib
}  // namespace unc

#endif /* _UNC_TCLIB_MODULE_HH_ */
