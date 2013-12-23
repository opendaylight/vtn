/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef CAPABILITY_MGR_HH_
#define CAPABILITY_MGR_HH_

#include <string>
#include <map>

#include "cxx/pfcxx/module.hh"
#include "cxx/pfcxx/synch.hh"
#include "unc/config.h"
#include "unc/keytype.h"
#include "capa_intf.hh"

namespace unc {
namespace capa {

class CapaModuleStub : public pfc::core::Module, CapaIntf {
 public:
  enum CapaMethod {
    INIT,
    FINI,
    LOAD_CAPABILLITY_FILES,
    GET_INSTANCE_COUNT,
    GET_CREATE_CAPABILITY,
    GET_UPDATE_CAPABILITY,
    GET_READ_CAPABILITY,
    GET_STATE_CAPABILITY,
  };

  explicit CapaModuleStub(const pfc_modattr_t *attr)
      : pfc::core::Module(attr) {
  }

  ~CapaModuleStub() {
  }

  inline pfc_bool_t init() {
    return stub_getMappedResultCode(CapaModuleStub::INIT);
  }

  inline pfc_bool_t fini(void) {
    return stub_getMappedResultCode(CapaModuleStub::FINI);
  }

  static CapaModuleStub* get_capability_mgr() {
    pfc_modattr_t attr;
    return new CapaModuleStub(&attr);
  }

  virtual bool  LoadCapabilityFiles() {
    return stub_getMappedResultCode(CapaModuleStub::LOAD_CAPABILLITY_FILES);
  }

  virtual std::string GetCtrlrParentVersion(const std::string &version) {
    return std::string("ParentVersion");
  }

  virtual bool GetInstanceCount(unc_keytype_ctrtype_t ctrlr_type,
                                const std::string &version,
                                unc_key_type_t keytype,
                                uint32_t &instance_count) {
    return stub_getMappedResultCode(CapaModuleStub::GET_INSTANCE_COUNT);
  }

  virtual bool GetCreateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                   const std::string &version,
                                   unc_key_type_t keytype,
                                   uint32_t *instnace_count,
                                   uint32_t *num_attrs,
                                   const uint8_t  **attrs) {
    *instnace_count = max_instance_count_local;
    *num_attrs  = *num_attrs_local;
    *attrs = attrs_local;
    return stub_getMappedResultCode(CapaModuleStub::GET_CREATE_CAPABILITY);
  }

  virtual bool GetUpdateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                   const std::string &version,
                                   unc_key_type_t keytype,
                                   uint32_t *num_attrs,
                                   const uint8_t  **attrs) {
    *num_attrs = *num_attrs_local;
    *attrs = attrs_local;
    return stub_getMappedResultCode(CapaModuleStub::GET_UPDATE_CAPABILITY);
  }

  virtual bool GetReadCapability(unc_keytype_ctrtype_t ctrlr_type,
                                 const std::string &version,
                                 unc_key_type_t keytype,
                                 uint32_t *num_attrs, const uint8_t  **attrs) {
    *num_attrs = *num_attrs_local;
    *attrs = attrs_local;

    return stub_getMappedResultCode(CapaModuleStub::GET_READ_CAPABILITY);
  }

  virtual bool GetStateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                  const std::string &version,
                                  unc_key_type_t keytype,
                                  uint32_t *num_attrs, const uint8_t  **attrs) {
    return stub_getMappedResultCode(CapaModuleStub::GET_STATE_CAPABILITY);
  }

  inline void VerboseDumpAll() {
  }

  inline void VerboseDump(unc_keytype_ctrtype_t ctrlr_type,
                          std::string version) {
  }

  static void stub_setResultcode(CapaModuleStub::CapaMethod methodType,
                                 bool res_code);
  static void stub_clearStubData();

  static void  stub_setCreatecapaParameters(uint32_t max_instance_count,
                                            uint32_t *num_attrs,
                                            uint8_t *attrs) {
    max_instance_count_local = max_instance_count;
    num_attrs_local = num_attrs;
    attrs_local = attrs;
  }

  static void stub_loadCapaModule(void);
  static void stub_unloadCapaModule(void);

 private:
  static bool stub_getMappedResultCode(CapaModuleStub::CapaMethod methodType);
  static std::map<CapaModuleStub::CapaMethod, bool> method_capa_map;
  static uint32_t max_instance_count_local;
  static uint32_t *num_attrs_local;
  static uint8_t *attrs_local;
};
// NOLINT

} /* namespace capa */
} /* namespace unc */

#endif  // CAPABILITY_MGR_HH_

