/*
 * Copyright (c) 2012-2013 NEC Corporation
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
      Ctrlr(const char *name, unc_keytype_ctrtype_t type, const char *version)
        :name_(name), type_(type), version_(version) {
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

 private:
  CtrlrMgr() { }

  virtual ~CtrlrMgr() {
  }

  static CtrlrMgr *singleton_instance_;
  std::list<Ctrlr*> ctrlrs_;
};

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
                                                                       // NOLINT
#endif  // UPLL_CTRLR_MGR_HH_
