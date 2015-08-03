/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfc/log.h"
#include "capa/capa_module.hh"
#include "ctrlr_mgr.hh"
#include "config_mgr.hh"

namespace unc {
namespace upll {
namespace config_momgr {


const MoManager *MoManager::GetMoManager(unc_key_type_t kt) {
  UpllConfigMgr *ucm = UpllConfigMgr::GetUpllConfigMgr();
  if (ucm != NULL) {
    return ucm->GetMoManager(kt);
  }
  return NULL;
}

upll_rc_t MoAuditServiceIntf::ContinueAuditProcess() {
  return UpllConfigMgr::GetUpllConfigMgr()->ContinueAuditProcess();
}

bool MoManager::GetMaxInstanceCount(const char *ctrlr_name,
                                    unc_key_type_t keytype,
                                    uint32_t &instance_count,
                                    upll_keytype_datatype_t datatype) {
  unc_keytype_ctrtype_t type;
  std::string version;

  if (ctrlr_name == NULL) {
    UPLL_LOG_WARN("Null argument ctrlr_name");
    return false;
  }

  unc::capa::CapaIntf *capa = reinterpret_cast<unc::capa::CapaIntf *>(
      pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
    UPLL_LOG_FATAL("CapaModule is not found");
    return false;
  }

  if (false == GetCtrlrTypeAndVersion(ctrlr_name, datatype, &type, &version)) {
    UPLL_LOG_TRACE("ctrlr_name %s is not found", ctrlr_name);
    return false;
  }

  return capa->GetInstanceCount(type, version, keytype, instance_count);
}

bool MoManager::GetCreateCapability(const char *ctrlr_name,
                                    unc_key_type_t keytype,
                                    uint32_t *instance_count,
                                    uint32_t *num_attrs,
                                    const uint8_t **attrs,
                                    upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  unc_keytype_ctrtype_t type;
  std::string version;

  if (ctrlr_name == NULL) {
    UPLL_LOG_WARN("Null argument ctrlr_name");
    return false;
  }

  unc::capa::CapaModule *capa = reinterpret_cast<unc::capa::CapaModule *>(
    pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
    UPLL_LOG_FATAL("CapaModule is not found");
    return false;
  }

  if (false == GetCtrlrTypeAndVersion(ctrlr_name, datatype, &type, &version)) {
    UPLL_LOG_TRACE("ctrlr_name %s is not found", ctrlr_name);
    return false;
  }

  return capa->GetCreateCapability(type, version, keytype, instance_count,
                                   num_attrs, attrs);
}

bool MoManager::GetUpdateCapability(const char *ctrlr_name,
                                    unc_key_type_t keytype,
                                    uint32_t *num_attrs,
                                    const uint8_t **attrs,
                                    upll_keytype_datatype_t datatype) {
  unc_keytype_ctrtype_t type;
  std::string version;

  if (ctrlr_name == NULL) {
    UPLL_LOG_WARN("Null argument ctrlr_name");
    return false;
  }

  unc::capa::CapaModule *capa = reinterpret_cast<unc::capa::CapaModule *>(
    pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
    UPLL_LOG_FATAL("CapaModule is not found");
    return false;
  }

  if (false == GetCtrlrTypeAndVersion(ctrlr_name, datatype, &type, &version)) {
    UPLL_LOG_TRACE("ctrlr_name %s is not found", ctrlr_name);
    return false;
  }

  return capa->GetUpdateCapability(type, version, keytype, num_attrs, attrs);
}

bool MoManager::GetReadCapability(const char *ctrlr_name,
                                  unc_key_type_t keytype,
                                  uint32_t *num_attrs,
                                  const uint8_t **attrs,
                                  upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  unc_keytype_ctrtype_t type;
  std::string version;

  if (ctrlr_name == NULL) {
    UPLL_LOG_WARN("Null argument ctrlr_name");
    return false;
  }

  unc::capa::CapaModule *capa = reinterpret_cast<unc::capa::CapaModule *>(
    pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
    UPLL_LOG_FATAL("CapaModule is not found");
    return false;
  }

  if (false == GetCtrlrTypeAndVersion(ctrlr_name, datatype, &type, &version)) {
    UPLL_LOG_TRACE("ctrlr_name %s is not found", ctrlr_name);
    return false;
  }

  return capa->GetReadCapability(type, version, keytype, num_attrs, attrs);
}

bool MoManager::GetStateCapability(const char *ctrlr_name,
                                   unc_key_type_t keytype,
                                   uint32_t *num_attrs,
                                   const uint8_t **attrs,
                                   upll_keytype_datatype_t datatype) {
  unc_keytype_ctrtype_t type;
  std::string version;

  if (ctrlr_name == NULL) {
    UPLL_LOG_WARN("Null argument ctrlr_name");
    return false;
  }

  unc::capa::CapaModule *capa = reinterpret_cast<unc::capa::CapaModule *>(
    pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
    UPLL_LOG_FATAL("CapaModule is not found");
    return false;
  }

  if (false == GetCtrlrTypeAndVersion(ctrlr_name, datatype, &type, &version)) {
    UPLL_LOG_TRACE("ctrlr_name %s is not found", ctrlr_name);
    return false;
  }

  return capa->GetStateCapability(type, version, keytype, num_attrs, attrs);
}

bool MoManager::GetCtrlrTypeAndVersion(const char *ctrlr_name,
                                       upll_keytype_datatype_t datatype,
                                       unc_keytype_ctrtype_t *ctrlr_type,
                                       std::string *version) {
  CtrlrMgr::GetInstance()->PrintCtrlrList();
  return CtrlrMgr::GetInstance()->GetCtrlrTypeAndVersion(ctrlr_name,
                                                         datatype,
                                                         ctrlr_type, version);
}

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
