/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include "pfcxx/module.hh"
#include "pfcxx/event.hh"
#include "capa_module.hh"
#include "ctrlr_capability.hh"
#include "ctrlr_capa_defines.hh"

namespace unc {
namespace capa {

CtrlrCapability::CtrlrCapability() {
}

CtrlrCapability::~CtrlrCapability() {
#if 0   // Do not free as it is a mulithreaded system
  std::map<uint32_t, KtCapability*>::iterator ktit;
  ktit = kt_cap_map_.begin();
  while (ktit != kt_cap_map_.end()) {
    KtCapability *kt_cap_ptr = ktit->second;
    delete kt_cap_ptr;
    ktit++;
  }
  kt_cap_map_.clear(ktit);
#endif
}

uint32_t CtrlrCapability::LoadInstanceCount(const pfc_conf_t confp,
                                            std::string ktname,
                                            std::string version) {
  std::string ver = version;
  uint32_t instance_count;
  while (!ver.empty()) {
    pfc_cfblk_t kt_blk = pfc_conf_get_map(confp, ktname.c_str(), ver.c_str());
    if (( PFC_CFBLK_INVALID == kt_blk) ||
        (PFC_FALSE == pfc_conf_is_defined(kt_blk, "instance_count"))) {
      /**
       * Configuration of specified version is not found.
       * Search parent version.
       */
      ver = CapaModule::get_capability_mgr()->GetCtrlrParentVersion(ver);
      continue;
    }
    instance_count = pfc_conf_get_uint32(kt_blk, "instance_count", 0);
    pfc_log_verbose("Configuration is found in version %s", ver.c_str());
    return instance_count;
  }
  pfc_log_verbose("Configuration is not found in versiom %s and in key type %s",
                 version.c_str(), ktname.c_str());
  return 0;
}

bool CtrlrCapability::LoadCtrlrCapability(const pfc_conf_t confp,
                                          const std::string &version) {
  KtCapability *kt_ptr = NULL;
  uint32_t  instance_count;
  unc_key_type_t  keytype;

  /** Loop to read keytype value and name */
  for (int index = 0; NULL != kt_map[index].kt_name; index++) {
    std::string ktname(kt_map[index].kt_name);
    std::string ver = version;
    keytype = kt_map[index].keytype;
    pfc_log_verbose("ktname : %s", ktname.c_str());
    /* Read configuration for specified version*/
    while (!ver.empty()) {
      pfc_cfblk_t kt_blk = pfc_conf_get_map(confp, ktname.c_str(), ver.c_str());
      if (PFC_CFBLK_INVALID == kt_blk) {
        /**
         * Configuration of specified version is not found.
         * Search parent version.
         */
        ver = CapaModule::get_capability_mgr()->GetCtrlrParentVersion(ver);
        continue;
      }
      /* Specified ktname is found in config file */
      kt_ptr = new KtCapability;

      /** Insert into kt_cap_map_ */
      kt_cap_map_[keytype] = kt_ptr;
      instance_count = LoadInstanceCount(confp, ktname, ver);
      pfc_log_verbose("instance_count: %d", instance_count);
      kt_ptr->set_instance_count(instance_count);
      kt_ptr->LoadKtCapability(confp, index, ver);
      break;
    }
  } /* End of Keytype value and name loop */
  return true;
}

bool CtrlrCapability::GetCreateCapability(unc_key_type_t keytype,
                                          uint32_t *instance_count,
                                          uint32_t *num_attrs,
                                          const uint8_t  **attrs) {
  pfc_log_trace("In %s()", __PRETTY_FUNCTION__);
  std::map<uint32_t, KtCapability*>::iterator ktit = kt_cap_map_.find(keytype);
  if (ktit == kt_cap_map_.end()) {
    pfc_log_verbose("INFO: KeyType %u is not found in kt map ", keytype);
    return false;
  }
  KtCapability* kt_cap = ktit->second;
  bool ret = kt_cap->GetKtCreateCapability(instance_count, num_attrs, attrs);
  return ret;
}

bool CtrlrCapability::GetUpdateCapability(unc_key_type_t keytype,
                                          uint32_t *num_attrs,
                                          const uint8_t  **attrs) {
  std::map<uint32_t, KtCapability*>::iterator ktit = kt_cap_map_.find(keytype);
  if (ktit == kt_cap_map_.end()) {
    pfc_log_verbose("INFO: KeyType is not found in kt map ");
    return false;
  }
  KtCapability* kt_cap_ptr = ktit->second;
  bool ret = kt_cap_ptr->GetKtUpdateCapability(num_attrs, attrs);
  return ret;
}

bool CtrlrCapability::GetReadCapability(unc_key_type_t keytype,
                                        uint32_t *num_attrs,
                                        const uint8_t  **attrs) {
  std::map<uint32_t, KtCapability*>::iterator ktit = kt_cap_map_.find(keytype);
  if (ktit == kt_cap_map_.end()) {
    pfc_log_verbose("INFO: KeyType is not found in kt map ");
    return false;
  }
  KtCapability* kt_cap_ptr = ktit->second;
  bool ret = kt_cap_ptr->GetKtReadCapability(num_attrs, attrs);
  return ret;
}

bool CtrlrCapability::GetStateCapability(unc_key_type_t keytype,
                                         uint32_t *num_attrs,
                                         const uint8_t  **attrs) {
  std::map<uint32_t, KtCapability*>::iterator ktit = kt_cap_map_.find(keytype);
  if (ktit == kt_cap_map_.end()) {
    pfc_log_verbose("INFO: KeyType is not found in kt map ");
    return false;
  }
  KtCapability* kt_cap_ptr = ktit->second;
  bool ret = kt_cap_ptr->GetKtStateCapability(num_attrs, attrs);
  return ret;
}

bool CtrlrCapability::GetCapability(unc_key_type_t keytype,
                                    uint32_t &instance_count) {
  std::map<uint32_t, KtCapability*>::iterator ktit = kt_cap_map_.find(keytype);
  if (ktit == kt_cap_map_.end()) {
    pfc_log_verbose("INFO: KeyType is not found in kt map ");
    return false;
  }
  KtCapability* kt_cap_ptr = ktit->second;
  instance_count = kt_cap_ptr->get_instance_count();
  return true;
}

KtCapability::KtCapability() {
  instance_count_ = 0;
  attr_cap_ = NULL;
}

KtCapability::~KtCapability() {
  if (NULL != attr_cap_) {
    delete attr_cap_;
    attr_cap_ = NULL;
  }
}

bool KtCapability::LoadAttrCapability(const pfc_conf_t confp,
                                      std::string ktname,
                                      std::string attrname,
                                      uint32_t attr_index,
                                      const std::string version) {
  std::string ver = version;
  uint8_t capa_list[4];

  while (!ver.empty()) {
    pfc_cfblk_t kt_blk = pfc_conf_get_map(confp, ktname.c_str(), ver.c_str());
    if (( PFC_CFBLK_INVALID == kt_blk) ||
        (PFC_FALSE == pfc_conf_is_defined(kt_blk, attrname.c_str()))) {
      pfc_log_verbose("%s:%s is not in version %s", ktname.c_str(),
                    attrname.c_str(), ver.c_str());
      ver = CapaModule::get_capability_mgr()->GetCtrlrParentVersion(ver);
      continue;
    }

    pfc_log_verbose("%s:%s is found in version %s", ktname.c_str(),
                  attrname.c_str(), ver.c_str());

    pfc_bool_t buffer[CapaIntf::kNumberOfAvailability];
    pfc_conf_array_bool_range(kt_blk, attrname.c_str(), 0, 4, buffer);
    for (uint32_t op = 0; op < CapaIntf::kNumberOfAvailability; op++) {
      capa_list[op] = (buffer[op] == PFC_TRUE) ? 1 : 0;
    }
    attr_cap_->SetAttrCap(attr_index, capa_list);
    return true;
  }
  /* Configuration is not found. */
  pfc_log_trace("Configuration is not found in versiom %s and in key type %s",
                 version.c_str(), ktname.c_str());
  return false;
}

pfc_bool_t KtCapability::LoadKtCapability(const pfc_conf_t confp,
                                          uint32_t kt_map_index,
                                          std::string version) {
  uint32_t num_attrs;


  std::string ktname(kt_map[kt_map_index].kt_name);
  num_attrs = kt_map[kt_map_index].attr_count;

  attr_cap_ = new KtAttrCapability;
  if (false == attr_cap_->Init((num_attrs))) {
    pfc_log_error("KtAttrCapability::Init failed");
  }

  std::string ver = version;
  KtAttrMap *attr_map = kt_map[kt_map_index].attr_map;

  for (uint32_t index = 0; index < num_attrs; index++) {
    // Get capabilities of the attribute
    std::string attr_name(attr_map[index].attr_name);
    PFC_ASSERT_PRINTF(index == attr_map[index].attr_index,
                      "Error: index %d %d", index , attr_map[index].attr_index);
    if (index != attr_map[index].attr_index) {
      pfc_log_fatal("Error: index %d %d", index , attr_map[index].attr_index);
    }
    if (false == LoadAttrCapability(confp, ktname, attr_name, index, ver)) {
      pfc_log_verbose("LoadAttrCapability(%s, %s, %s) failed", ktname.c_str(),
                    attr_name.c_str(), ver.c_str());
      return false;
    }
  }
  return true;
}

bool KtCapability::GetKtCreateCapability(uint32_t *instance_count,
                                         uint32_t *num_attrs,
                                         const uint8_t  **attrs) {
  pfc_log_trace("In %s()", __PRETTY_FUNCTION__);
  *instance_count = instance_count_;
  if (NULL == attr_cap_) {
    pfc_log_verbose("Attr Cap is NULL ");
    return false;
  }
  bool ret = attr_cap_->GetCreateCapability(num_attrs, attrs);
  return ret;
}

bool KtCapability::GetKtUpdateCapability(uint32_t *num_attrs,
                                         const uint8_t  **attrs) {
  if (NULL == attr_cap_) {
    pfc_log_verbose("Attr Cap is NULL ");
    return false;
  }
  bool ret = attr_cap_->GetUpdateCapability(num_attrs, attrs);
  return ret;
}

bool KtCapability::GetKtReadCapability(uint32_t *num_attrs,
                                       const uint8_t  **attrs) {
  if (NULL == attr_cap_) {
    pfc_log_verbose("Attr Cap is NULL ");
    return false;
  }
  bool ret = attr_cap_->GetReadCapability(num_attrs, attrs);
  return ret;
}

bool KtCapability::GetKtStateCapability(uint32_t *num_attrs,
                                        const uint8_t  **attrs) {
  if (NULL == attr_cap_) {
    pfc_log_verbose("Attr Cap is NULL ");
    return false;
  }
  bool ret = attr_cap_->GetStateCapability(num_attrs, attrs);
  return ret;
}

KtAttrCapability::KtAttrCapability() {
  num_attrs_ = 0;
  // Is it required? attr_cap_ = NULL;
  create_cap_ = NULL;
  update_cap_ = NULL;
  read_cap_ = NULL;
  state_cap_ = NULL;
}

KtAttrCapability::~KtAttrCapability() {
  /*
     if (NULL !=attr_cap_) {
     delete[] attr_cap_;
     attr_cap_ = NULL;
     }
   */
  if (NULL != create_cap_) {
    delete[] create_cap_;
    create_cap_ = NULL;
  }
  if (NULL != update_cap_) {
    delete[] update_cap_;
    update_cap_ = NULL;
  }
  if (NULL != read_cap_) {
    delete[] read_cap_;
    read_cap_ = NULL;
  }
  if (NULL != state_cap_) {
    delete[] state_cap_;
    state_cap_ = NULL;
  }
}

bool KtAttrCapability::Init(uint32_t num_attrs) {
  num_attrs_ = num_attrs;
  if (num_attrs == 0 ) {
    // KTs without any configuration attributes exist.
    // Just to avoid null pointers, and zero size allocation, keep 1 byte memory
    // Class varaible num_attrs_ still will be zero
    num_attrs = 1;
  }
  create_cap_ = new uint8_t[num_attrs];
  update_cap_ = new uint8_t[num_attrs];
  read_cap_   = new uint8_t[num_attrs];
  state_cap_  = new uint8_t[num_attrs];

  for (uint32_t i = 0; i < num_attrs; i++) {
    create_cap_[i] = 0;
    update_cap_[i] = 0;
    read_cap_[i]   = 0;
    state_cap_[i]  = 0;
  }

  return true;
}

bool KtAttrCapability::SetAttrCap(uint32_t attr_index, uint8_t capa_list[4]) {
  if (attr_index > num_attrs_) {
    pfc_log_verbose("attr_index > num_attrs_");
    return false;
  }

  create_cap_[attr_index] = capa_list[kCapaConfCreate];
  update_cap_[attr_index] = capa_list[kCapaConfUpdate];
  read_cap_[attr_index]   = capa_list[kCapaConfRead];
  state_cap_[attr_index]  = capa_list[kCapaConfStateRead];
  return true;
}

bool KtAttrCapability::GetCreateCapability(uint32_t *num_attrs,
                                           const uint8_t **create) {
  pfc_log_trace("In %s()", __PRETTY_FUNCTION__);
  *num_attrs = num_attrs_;
  if (NULL == create_cap_) {
    pfc_log_verbose("create_cap_ is NULL");
    return false;
  }
  *create = (const uint8_t *)create_cap_;
  return true;
}

bool KtAttrCapability::GetUpdateCapability(uint32_t *num_attrs,
                                           const uint8_t **update) {
  *num_attrs = num_attrs_;
  if (NULL == update_cap_) {
    pfc_log_verbose("update_cap_ is NULL");
    return false;
  }
  *update = (const uint8_t *)update_cap_;
  return true;
}

bool KtAttrCapability::GetReadCapability(uint32_t *num_attrs,
                                         const uint8_t **read) {
  *num_attrs = num_attrs_;
  if (NULL == read_cap_) {
    pfc_log_verbose("read_cap_ is NULL");
    return false;
  }
  *read = (const uint8_t *)read_cap_;
  return true;
}

bool KtAttrCapability::GetStateCapability(uint32_t *num_attrs,
                                          const uint8_t **state) {
  *num_attrs = num_attrs_;
  if (NULL == state_cap_) {
    pfc_log_verbose("state_cap_ is NULL");
    return false;
  }
  *state = (const uint8_t *)state_cap_;
  return true;
}
}  /* capa */
}  /* unc */
