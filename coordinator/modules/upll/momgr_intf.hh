/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_MOMGR_INTF_HH_
#define UPLL_MOMGR_INTF_HH_

#include <string>
#include <list>
#include <set>

#include "pfc/event.h"
#include "cxx/pfcxx/synch.hh"

#include "unc/upll_errno.h"
#include "unc/upll_ipc_enum.h"
#include "dal/dal_dml_intf.hh"
#include "ipc_util.hh"
#include "upll_util.hh"
#include "kt_util.hh"
#include "tx_update_util.hh"
#include "tclib_module.hh"

namespace unc {
namespace upll {
namespace config_momgr {

using std::string;
using std::list;
using std::set;
using unc::upll::ipc_util::IpcReqRespHeader;
using unc::upll::ipc_util::IpcResponse;
using unc::upll::ipc_util::ConfigKeyVal;
using unc::upll::ipc_util::ConfigNotification;
using unc::upll::ipc_util::controller_domain_t;
using unc::upll::dal::DalDmlIntf;
using unc::upll::tx_update_util::TxUpdateUtil;


struct CtrlrTxResult {
  string ctrlr_id;
  upll_rc_t upll_ctrlr_result;
  uint32_t ctrlr_orig_result;
  ConfigKeyVal *err_ckv;
  CtrlrTxResult(string ctr_name, upll_rc_t upll_ret, uint32_t ctrlr_ret) {
    ctrlr_id = ctr_name;
    upll_ctrlr_result = upll_ret;
    ctrlr_orig_result = ctrlr_ret;
    err_ckv = NULL;
  }
  ~CtrlrTxResult() {
    if (err_ckv != NULL)
      delete err_ckv;
  }
};
typedef CtrlrTxResult CtrlrVoteStatus;
typedef CtrlrTxResult CtrlrCommitStatus;

// Virtual and Vtn views are non-overlapping except for KT_ROOT.
enum UpllKtView {  // bit map
  kUpllKtViewVirtual = 0x01,
  kUpllKtViewVtn = 0x02,
  kUpllKtViewState = 0x04,
};

// Interface class for edit/read/control operations
class MoCfgServiceIntf {
 public:
  virtual ~MoCfgServiceIntf() {}
  virtual upll_rc_t CreateMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                          DalDmlIntf *dmi) = 0;
  // domain_id is guaranteed for VTN, for vnode it might be there (driver team
  // need to confirm), for others it is optional and not present does not mean
  // no domain id.
  // impor_type represets the  impor types is partial import / full import
  virtual upll_rc_t CreateImportMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                                DalDmlIntf *dmi, const char *ctrlr_id,
                                const char *domain_id,
                                upll_import_type import_type) = 0;
  virtual upll_rc_t DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                             DalDmlIntf *dmi) = 0;
  virtual upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                             DalDmlIntf *dmi) = 0;
  virtual upll_rc_t RenameMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                             DalDmlIntf *dmi, const char *ctrlr_id) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  virtual upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                           DalDmlIntf *dmi) = 0;
  virtual upll_rc_t ReadSiblingMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                                  bool begin, DalDmlIntf *dal) = 0;
  virtual upll_rc_t ReadSiblingCount(IpcReqRespHeader *req, ConfigKeyVal *key,
                                     DalDmlIntf *dmi) = 0;
  virtual upll_rc_t ControlMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                              DalDmlIntf *dmi) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
};

enum UpdateCtrlrPhase {
  kUpllUcpInvalid = 0,
  kUpllUcpCreate,
  kUpllUcpUpdate,
  kUpllUcpDelete,
  // kUpllUcpDelete2: Used to clean up some entries where references need to be
  // deleted first using Update.
  kUpllUcpDelete2,
  kUpllUcpInit
};

enum KTxCtrlrAffectedState {
  kCtrlrAffectedNoDiff = 0,  // There is no diff in UNC and Ctrlr Configuration
  kCtrlrAffectedOnlyCSDiff,  // No config diff, only diff in the config status
  kCtrlrAffectedConfigDiff   // There is a diff in the configuration,
                             // add the controller to affected ctrlr list
};
// Interface class for normal transaction operations
class MoTxServiceIntf {
 public:
  virtual ~MoTxServiceIntf() {}
  virtual upll_rc_t TxUpdateController(unc_key_type_t keytype,
                                    uint32_t session_id, uint32_t config_id,
                                    UpdateCtrlrPhase phase,
                                    set<string> *affected_ctrlr_set,
                                    DalDmlIntf *dmi,
                                    ConfigKeyVal **err_ckv,
                                    TxUpdateUtil *tx_util,
                                    TcConfigMode config_mode,
                                    std::string vtn_name) = 0;

  virtual upll_rc_t TxUpdateErrorHandler(ConfigKeyVal *ckv_unc,
                                          ConfigKeyVal *ck_driver,
                                          DalDmlIntf *dmi,
                                          upll_keytype_datatype_t dt_type,
                                          ConfigKeyVal **err_ckv,
                                          IpcResponse *ipc_resp) = 0;

  // To clear c_flag and u_flag from candidate db
  virtual upll_rc_t TxClearCreateUpdateFlag(unc_key_type_t keytype,
                                            upll_keytype_datatype_t cfg_type,
                                            DalDmlIntf *dmi,
                                            TcConfigMode config_mode,
                                            std::string vtn_name) = 0;

  virtual upll_rc_t TxVote(unc_key_type_t keytype, DalDmlIntf *dmi,
                           TcConfigMode config_mode, std::string vtn_name,
                           ConfigKeyVal **err_ckv) = 0;
  virtual upll_rc_t TxVoteCtrlrStatus(
      unc_key_type_t keytype,
      list<CtrlrVoteStatus*> *ctrlr_vote_status,
      DalDmlIntf *dmi,
      TcConfigMode config_mode,
      std::string vtn_name) = 0;
  // NOTE:
  // ctrlr_commit_status list will contain errors pertaining to UPLL returned by
  // drivers during normal user initiated commit operation.
  // The list will be NULL if on failover, TC does rerun the commit. In this
  // case momgr should commit the configuration with cs_status as NOT_APPLIED.
  virtual upll_rc_t TxCopyCandidateToRunning(
      unc_key_type_t keytype,
      list<CtrlrCommitStatus*> *ctrlr_commit_status,
      DalDmlIntf *dmi, TcConfigMode config_mode,
      std::string vtn_name) = 0;

  virtual upll_rc_t TxUpdateDtState(unc_key_type_t ktype,
                                    uint32_t session_id,
                                     uint32_t config_id,
                                     DalDmlIntf *dmi) {
    UPLL_LOG_DEBUG("kt: %u", ktype);
    return UPLL_RC_SUCCESS;
  }
  virtual upll_rc_t TxEnd(unc_key_type_t keytype, DalDmlIntf *dmi,
                          TcConfigMode config_mode, std::string vtn_name) = 0;

  virtual upll_rc_t ClearVirtualKtDirtyInGlobal(DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_ERR_GENERIC;
  }
  virtual upll_rc_t ProcessAlarm(TcConfigMode config_mode,
                                 std::string vtn_name, DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_ERR_GENERIC;
  }
};

// Interface class for audit operations
class MoAuditServiceIntf {
 public:
  virtual ~MoAuditServiceIntf() {}
  virtual upll_rc_t AuditUpdateController(unc_key_type_t keytype,
                                    const char *ctrlr_id,
                                    uint32_t session_id,
                                    uint32_t config_id,
                                    UpdateCtrlrPhase phase,
                                    DalDmlIntf *dmi,
                                    ConfigKeyVal **err_ckv,
                                    KTxCtrlrAffectedState *ctrlr_affected) = 0;

  // virtual upll_rc_t AuditVote(const char *ctrlr_id) = 0;
  virtual upll_rc_t AuditVoteCtrlrStatus(unc_key_type_t keytype,
                                         CtrlrVoteStatus *vote_satus,
                                         DalDmlIntf *dmi) = 0;
  virtual upll_rc_t AuditCommitCtrlrStatus(unc_key_type_t keytype,
                                           CtrlrCommitStatus *commit_satus,
                                           DalDmlIntf *dmi) = 0;
  // NOTE: During SBY to ACT transition, AuditEnd will be called with empty
  // controlller-id ""
  virtual upll_rc_t AuditEnd(unc_key_type_t keytype, const char *ctrlr_id,
                             DalDmlIntf *dmi) = 0;

  virtual upll_rc_t ContinueAuditProcess();
};

// Interface class for import operations
class MoImportServiceIntf {
 public:
  virtual ~MoImportServiceIntf() {}
  virtual upll_rc_t MergeValidate(
      unc_key_type_t keytype,
      const char *ctrlr_id,
      ConfigKeyVal *conflict_ckv,
      DalDmlIntf *dmi,
      upll_import_type import_type = UPLL_IMPORT_TYPE_FULL) = 0;
  virtual upll_rc_t MergeImportToCandidate(unc_key_type_t keytype,
                                           const char *ctrlr_id,
                                           DalDmlIntf *dmi,
                                           upll_import_type import_type) = 0;
  // NOTE: During SBY to ACT transition, ImportClear will be called with empty
  // controlller-id ""
  virtual upll_rc_t ImportClear(unc_key_type_t keytype,
                                const char *ctrlr_id,
                                DalDmlIntf *dmi) = 0;
  virtual upll_rc_t CopyRenameTables(const char *ctrlr_id,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dom) = 0;
  virtual upll_rc_t CopyVtnUnifiedTable(upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dom) = 0;

  virtual upll_rc_t PurgeCandidate(unc_key_type_t keytype,
                                     const char *ctrlr_id,
                                     DalDmlIntf *dom) = 0;
  virtual upll_rc_t PurgeRenameTable(unc_key_type_t keytype,
                                     const char *ctrlr_id,
                                     DalDmlIntf *dom) = 0;
  virtual upll_rc_t GetRenamedUncKeyWithRedirection(unc_key_type_t kt_type,
                                          upll_keytype_datatype_t dt_type,
                                          const char *ctrlr_id,
                                          DalDmlIntf *dmi) = 0;
};

// Interface class for database operations
class MoDbServiceIntf {
 public:
  virtual ~MoDbServiceIntf() {}
  virtual upll_rc_t CopyRunningToStartup(unc_key_type_t kt,
                                         DalDmlIntf *dmi) = 0;
  virtual upll_rc_t ClearConfiguration(unc_key_type_t kt, DalDmlIntf *dmi,
                                       upll_keytype_datatype_t cfg_type,
                                       TcConfigMode config_mode,
                                       std::string vtn_name) = 0;
  virtual upll_rc_t ClearStartup(unc_key_type_t kt, DalDmlIntf *dmi) = 0;
  virtual upll_rc_t LoadStartup(unc_key_type_t kt, DalDmlIntf *dmi) = 0;
  virtual upll_rc_t CopyRunningToCandidate(unc_key_type_t kt,
                                           DalDmlIntf *dmi,
                                           unc_keytype_operation_t op,
                                           TcConfigMode config_mode,
                                           std::string vtn_name) = 0;
  virtual upll_rc_t IsCandidateDirtyInGlobal(
      unc_key_type_t kt, bool *dirty, DalDmlIntf *dmi,
      bool shallow_check) = 0;

  virtual upll_rc_t IsCandidateDirtyShallowInPcm(unc_key_type_t kt,
                                                 TcConfigMode config_mode,
                                                 std::string vtn_name,
                                                 bool *dirty,
                                                 DalDmlIntf *dmi) = 0;
};

class MoManager : public MoCfgServiceIntf, public MoTxServiceIntf,
    public MoAuditServiceIntf, public MoImportServiceIntf,
    public MoDbServiceIntf {
 public:
  static const uint32_t kMaxReadBulkCount = 10000;

  virtual ~MoManager() {}

  const MoManager *GetMoManager(unc_key_type_t kt);
  // virtual void UpplNotificationHandler(pfc_event_t event, pfc_ptr_t arg);
  // virtual void DriverNotificationHandler(pfc_event_t event, pfc_ptr_t arg);
  virtual upll_rc_t IsKeyInUse(upll_keytype_datatype_t datatype,
                               const ConfigKeyVal *ckv, bool *in_use,
                               DalDmlIntf *dmi) {
    *in_use = true;
    return UPLL_RC_ERR_GENERIC;
  }

  /**
   * @brief      Method to get a configkeyval of a specified keytype from
   * an input configkeyval
   *
   * @param[in/out]  okey                 pointer to output ConfigKeyVal
   * @param[in]      parent_key           pointer to the configkeyval from
   * which the output configkey val is initialized.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  virtual upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) = 0;

  /**
   * @brief      Method to get a configkeyval of the parent keytype
   *
   * @param[in/out]  okey           pointer to parent ConfigKeyVal
   * @param[in]      ikey           pointer to the child configkeyval from
   * which the parent configkey val is obtained.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   **/
  virtual upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                       ConfigKeyVal *ikey) = 0;
  // Capabiltiy functions
  /**
   * @brief  Return instance count of specified key type.
   *
   * @param[in] ctrlr_name  controller name.
   * @param[in] keytype     key type
   * @param[out] instance_count  Max instance count for specified keytype.
   * @param[in] datatype    Datatype. default is CANDIDATE
   *
   * @retval true   Successful
   * @retval false  controller/keytype is not found
   */
  virtual bool GetMaxInstanceCount(const char *ctrlr_name,
                   unc_key_type_t keytype,
                   uint32_t &instance_count,
                   upll_keytype_datatype_t datatype = UPLL_DT_CANDIDATE);
  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   *
   * @param[in]  ctrlr_name controller name.
   * @param[in]  version    controller version
   * @param[in]  keytype    Key type.
   * @param[out] instance_count  Instance count for specified keytype.
   * @param[in]  num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * @param[in] datatype    Datatype. default is CANDIDATE
   *
   * @retval true   Successful
   * @retval false  controller or keytype is not found
   */
  virtual bool GetCreateCapability(const char *ctrlr_name,
                   unc_key_type_t keytype,
                   uint32_t *instnace_count,
                   uint32_t *num_attrs,
                   const uint8_t **attrs,
                   upll_keytype_datatype_t datatype = UPLL_DT_CANDIDATE);
  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   *
   * @param[in]  ctrlr_type controller type.
   * @param[in]  keytype    Key type.
   * @param[in]  num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * @param[in] datatype    Datatype. default is CANDIDATE
   *
   * @retval true   Successful
   * @retval false  controler or keytype is not found
   */
  virtual bool GetUpdateCapability(const char *ctrlr_name,
                   unc_key_type_t keytype,
                   uint32_t *num_attrs,
                   const uint8_t **attrs,
                   upll_keytype_datatype_t datatype = UPLL_DT_CANDIDATE);
  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   *
   * @param[in]  ctrlr_name controller name.
   * @param[in]  keytype    Key type.
   * @param[in]  num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * @param[in] datatype    Datatype. default is CANDIDATE
   *
   * @retval true   Successful
   * @retval false  controller or keytype is not found
   */
  virtual bool GetReadCapability(const char *ctrlr_name,
                   unc_key_type_t keytype,
                   uint32_t *num_attrs,
                   const uint8_t **attrs,
                   upll_keytype_datatype_t datatype = UPLL_DT_CANDIDATE);
  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   *
   * @param[in]  ctrlr_name controller name.
   * @param[in]  keytype    Key type.
   * @param[in]  num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * @param[in] datatype    Datatype. default is CANDIDATE
   *
   * @retval true   Successful
   * @retval false  controller or keytype is not found
   */
  virtual bool GetStateCapability(const char *ctrlr_name,
                   unc_key_type_t keytype,
                   uint32_t *num_attrs,
                   const uint8_t **attrs,
                   upll_keytype_datatype_t datatype = UPLL_DT_CANDIDATE);


 private:
  static bool GetCtrlrTypeAndVersion(const char *ctrlr_name,
                                     upll_keytype_datatype_t datatype,
                                     unc_keytype_ctrtype_t *ctrlr_type,
                                     std::string *version);
};

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc

#endif  // UPLL_MOMGR_INTF_HH_
