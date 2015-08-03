/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_CTRLR_MGR_HH_
#define UPLL_CTRLR_MGR_HH_

#include <string>
#include <list>
#include <map>
#include <set>

#include "cxx/pfcxx/synch.hh"
#include "uncxx/upll_log.hh"
#include "upll/keytype_upll_ext.h"
#include "unc/upll_errno.h"
#include "dal/dal_odbc_mgr.hh"

namespace unc {
namespace upll {
namespace config_momgr {

class CtrlrMgr {
 public:
  static const uint32_t CTRLR_NAME_LEN = 31;
  class Ctrlr {
    public:
      friend class CtrlrMgr;
      Ctrlr(const char *name, unc_keytype_ctrtype_t type, const char *version,
            bool audit_type)
        :name_(name), type_(type), version_(version), audit_type_(audit_type) {
        invalid_config_ = false;
        audit_done_ = false;
        config_done_ = false;
        datatype_ = UPLL_DT_CANDIDATE;
      }
      Ctrlr(const Ctrlr &ctrlr, upll_keytype_datatype_t datatype)
        :name_(ctrlr.name_), version_(ctrlr.version_) {
        UPLL_FUNC_TRACE;
        type_ = ctrlr.type_;
        invalid_config_ = ctrlr.invalid_config_;
        audit_done_ = ctrlr.audit_done_;
        config_done_ = ctrlr.config_done_;
        audit_type_ = ctrlr.audit_type_;
        datatype_ = datatype;
      }
      ~Ctrlr() {
        UPLL_FUNC_TRACE;
        name_.clear();
        version_.clear();
      }

    private:
      std::string name_;
      unc_keytype_ctrtype_t type_;
      std::string version_;
      bool audit_done_;      // true, if audit is completed.
                             // available only in running
      bool config_done_;     // available only in running
      bool invalid_config_;  // Invalid config status after audit failure
                             // available only in running
      bool audit_type_;      //  Auto Audit disable or Enable
      upll_keytype_datatype_t datatype_;
        // if it is Running, Ctrlr is available in both candidate and running
  };  // Class Ctrlr

  upll_rc_t Add(const Ctrlr &ctrlr, const upll_keytype_datatype_t datatype);

  upll_rc_t Delete(const std::string &ctrlr_name,
                   const upll_keytype_datatype_t datatype);

  upll_rc_t UpdateVersion(const std::string &ctrlr_name,
                          const upll_keytype_datatype_t datatype,
                          const std::string &ctrlr_version);

  upll_rc_t UpdateAuditDone(const std::string &ctrlr_name,
                            const bool audit_done);

  upll_rc_t UpdateConfigDone(const std::string &ctrlr_name,
                             const bool config_done);

  upll_rc_t UpdateInvalidConfig(const std::string &ctrlr_name,
                                const bool invalid_config);

  static CtrlrMgr *GetInstance() {
    if (!singleton_instance_) {
      singleton_instance_ = new CtrlrMgr();
    }
    return singleton_instance_;
  }

  bool GetCtrlrType(const char *ctrlr_name,
                    const upll_keytype_datatype_t datatype,
                    unc_keytype_ctrtype_t *ctrlr_type);
  bool GetCtrlrTypeAndVersion(const char *ctrlr_name,
                              const upll_keytype_datatype_t datatype,
                              unc_keytype_ctrtype_t *ctrlr_type,
                              std::string *ctrlr_version);
  upll_rc_t IsConfigInvalid(const char *ctrlr_name, bool *config_invalid);
  upll_rc_t IsConfigDone(const char *ctrlr_name, bool *config_done);
  upll_rc_t IsAuditDone(const char *ctrlr_name, bool *audit_done);
  upll_rc_t GetFirstCtrlrName(const upll_keytype_datatype_t datatype,
                              std::string *first_name);
  upll_rc_t GetNextCtrlrName(const std::string name,
                             const upll_keytype_datatype_t datatype,
                             std::string *next_name);
  upll_keytype_datatype_t MapDataType(const upll_keytype_datatype_t datatype);
  void CleanUp();
  void PrintCtrlrList();
  upll_rc_t GetAuditType(const std::string &ctrlr_name,
                        const upll_keytype_datatype_t datatype,
                        bool *audit_type);
  upll_rc_t UpdateAuditType(const std::string &ctrlr_name,
                            const upll_keytype_datatype_t datatype,
                            bool audit_type);
  // Get list of invalid-config ctrlr
  void GetInvalidConfigList(std::list<std::string>
                            &invalid_config_ctr);
  bool UpdatePathFault(const char *ctr_name, const char *domain_name,
                       bool assert_alarm);
  bool IsPathFaultOccured(const char *ctr_name, const char *domain_name);
  void ClearPathfault(const char *ctr_name, const char *domain_name);

  // disconnect port status handling
  bool GetLogicalPortSt(const char *ctr_name, const char *logical_port,
                        uint8_t &state);
  void AddLogicalPort(const char *ctr_name, const char *logical_port,
                      uint8_t state);
  bool IsCtrDisconnected(const char *ctr_name);
  void AddCtrToDisconnectList(const char *ctr_name);
  void RemoveCtrFromDisconnectList(const char *ctr_name);

  void AddtoUnknownCtrlrList(const char* ctr_name);
  bool IsCtrlrInUnknownList(const char* ctr_name) {
    UPLL_FUNC_TRACE;
    bool bDisconnect(false);

    if (ctr_name == NULL) {
      UPLL_LOG_ERROR("Invalid argument ctr_name");
      return false;
    }
    discon_ctrlr_lock_.rdlock();
    if (discon_ctrlr_list_.end() != discon_ctrlr_list_.find(ctr_name)) {
      bDisconnect = true;
    }
    discon_ctrlr_lock_.unlock();
    return bDisconnect;
  }
  void DeleteFromUnknownCtrlrList(const char *ctr_name);
  void AddCtrToIgnoreList(const char *ctr_name);
  bool IsCtrlrInIgnoreList(const char *ctr_name);
  void RemoveCtrlrFromIgnoreList(const char *ctr_name);
  bool GetPathFaultDomains(const char *ctr_na,
                           std::set<std::string> *domain_names);

  // vtn exhaustion alarm APIs
  bool UpdateVtnExhaustionMap(const char *vtn_na, const char *ctr_na,
                              const char *domain_na, bool assert_alarm);
  bool HasVtnExhaustionOccured(const char *vtn_na,
                     const char *ctr_na, const char *domain_na);
  void ClearVtnExhaustionMap();

 private:
  CtrlrMgr() { }

  virtual ~CtrlrMgr() {
    CleanUp();
  }

  static CtrlrMgr *singleton_instance_;
  std::list<Ctrlr*> ctrlrs_;

  // path fault map <ctrlr, <domain, fault-count>>
  std::map<std::string, std::map<std::string, uint32_t> > path_fault_map_;
  pfc::core::ReadWriteLock path_fault_lock_;

  std::map<std::string,
           std::map<std::string, std::set<std::string> > > vtn_exhaust_map_;
  pfc::core::ReadWriteLock vtn_exhaust_lock_;

  // controller disconnect handling
  std::map<std::string, std::map<std::string, uint8_t> > ctr_discon_map_;
  pfc::core::ReadWriteLock ctr_discon_lock_;

  // controller disconnect handling for DT_STATE READs
  std::set<std::string> discon_ctrlr_list_;
  pfc::core::ReadWriteLock discon_ctrlr_lock_;

  // controller disconnect handling for VTN DT_STATE READs
  std::set<std::string> ctrlr_ignore_list_;
};

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
                                                                       // NOLINT
#endif  // UPLL_CTRLR_MGR_HH_
