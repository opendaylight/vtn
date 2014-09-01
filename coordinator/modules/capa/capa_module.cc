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
#include "pfc/conf.h"
#include "unc/keytype.h"

#include "ctrlr_capa_defines.hh"

#include "capa_module.hh"

namespace unc {
namespace capa {

CapaModule* CapaModule::capability_mgr_ = NULL;

const uint32_t CapaIntf::kNumberOfAvailability;

CapaModule::CapaModule(const pfc_modattr_t *attr)
    :pfc::core::Module(attr) {
      /*  */
    }

CapaModule::~CapaModule() {
}

pfc_bool_t CapaModule::init(void) {
  pfc_log_trace("Init");
  /* Assign capability object into local reference object*/
  capability_mgr_ = this;

  ctrlr_common_map_[UNC_CT_PFC] = new CapaCtrlrCommon;
  // ctrlr_common_map_[UNC_CT_LEGACY] = new CapaCtrlrCommon;
  ctrlr_common_map_[UNC_CT_VNP] = new CapaCtrlrCommon;
  ctrlr_common_map_[UNC_CT_POLC] = new CapaCtrlrCommon;
  ctrlr_common_map_[UNC_CT_ODC] = new CapaCtrlrCommon;

  LoadCapabilityFiles();

  /* Test Code */
#ifdef PFC_VERBOSE_DEBUG
  VerboseDumpAll();
  /*
  VerboseDump(UNC_CT_PFC, "PFC_NEW");
  // VerboseDump(UNC_CT_LEGACY, "PFC_5.0");
  VerboseDump(UNC_CT_VNP, "PFC_1.0.2");
  */
#endif
  return PFC_TRUE;
}

pfc_bool_t CapaModule::fini(void) {
  pfc_log_trace("Fini");

  ScopedReadWriteLock lock(capa_module_lock_, true);

#if 0  // Do not free as it is a multi-threaded system
  std::map<unc_keytype_ctrtype_t, CapaCtrlrCommon*>::iterator comm_ctrlr_it;
  for (comm_ctrlr_it = ctrlr_common_map_.begin();
       (comm_ctrlr_it != ctrlr_common_map_.end()); comm_ctrlr_it++) {
    struct CapaCtrlrCommon *ccc = comm_ctrlr_it->second;
    /* Clear CtrlrCapability Map */
    std::map<std::string, CtrlrCapability*>::iterator verit;
    for (verit = ccc->capa_map.begin(); verit != ccc->capa_map.end(); verit++) {
      CtrlrCapability *cap_ptr = verit->second;
      delete cap_ptr;
    }
    ccc->clear();
    delete ccc;
  }
  ctrlr_common_map_.clear();
#endif 
  return PFC_TRUE;
}

CapaModule* CapaModule::get_capability_mgr() {
  /** No Need to check NULL and create capabilit object.*/
  return capability_mgr_;
}

void CapaModule::LoadParentVersion(pfc_conf_t confp,
                                      const std::string version) {
  pfc_cfblk_t ver_def_cfblk = pfc_conf_get_map(confp, "version_definition",
                                               version.c_str());
  if (PFC_CFBLK_INVALID == ver_def_cfblk) {
    // No parent
    parent_map_[version] = "";
    return;
  }
  parent_map_[version] = pfc_conf_get_string(ver_def_cfblk, "parent", "");
}

bool CapaModule::LoadActualVersion(pfc_conf_t confp, const std::string version,
                                   std::list<ActualVersion> &version_list) {

  pfc_cfblk_t ver_def_cfblk = pfc_conf_get_map(confp, "version_definition",
                                               version.c_str());

  ActualVersion      actual_version;
  uint32_t           actual_count         = 0;
  uint32_t           actual_version_size  = 0;
  uint32_t           array_index          = 0;
  
  /* Clear actual version list */
  version_list.clear();

  actual_count = pfc_conf_get_uint32(ver_def_cfblk, "actual_version_count", 0);

  pfc_log_verbose(" Actual version count is %u", actual_count);

  for (uint32_t index = 0; index <actual_count; index++) {

    actual_version_size = pfc_conf_array_size(ver_def_cfblk, "actual_version");

    if ((actual_version_size == 0) || (actual_version_size%4 != 0)) {
      pfc_log_warn("Actual version array size is not multiples of 4");
      return false;
    }

    memset(&actual_version, 0, sizeof(ActualVersion));
    actual_version.major1  = pfc_conf_array_int32at(ver_def_cfblk,
                                                 "actual_version", array_index++, 0);
    actual_version.major2  = pfc_conf_array_int32at(ver_def_cfblk,
                                                 "actual_version", array_index++, 0);
    actual_version.minor   = pfc_conf_array_int32at(ver_def_cfblk,
                                                 "actual_version", array_index++, 0);
    actual_version.update  = pfc_conf_array_int32at(ver_def_cfblk,
                                                 "actual_version", array_index++, 0);
    version_list.push_back(actual_version);
  }
  return true;
}

bool CapaModule::ValidateVersion(unc_keytype_ctrtype_t ctrlr_type,
                                 std::string config_version,
                                 uint8_t pfc_version_major1, 
                                 uint8_t pfc_version_major2,
                                 uint8_t pfc_version_minor,
                                 uint8_t pfc_version_update) {

  pfc_log_info("Validates Configuration version and actual version ");

  struct CapaCtrlrCommon    *ccc = ctrlr_common_map_.find(ctrlr_type)->second;
  std::list<ActualVersion>  actual_version_list;
  ActualVersion             actual;
  std::map<std::string, std::list<ActualVersion> >::iterator act_ver_it;

  if (ccc == NULL) {
    pfc_log_warn("Failed to find Capa controller common");
    return false;
  }
  memset(&actual, 0, sizeof(ActualVersion));

  act_ver_it = ccc->actual_version_map_.find(config_version);
  if (act_ver_it == ccc->actual_version_map_.end()) {
    pfc_log_debug("Configuration version is not a member of "
                  "actual version map");
    return false;
  }

  actual_version_list =  act_ver_it->second;

  while (!actual_version_list.empty()) {
    actual = actual_version_list.front();

    actual_version_list.pop_front();
    pfc_log_verbose("Actual version : : (%d.%d.%d.%d)",
                    actual.major1, actual.major2, actual.minor, actual.update);

    if ((actual.major1 == -1) || ((uint8_t)actual.major1 < pfc_version_major1)) {
      return true;
    } 

    if (((uint8_t)actual.major1 == pfc_version_major1)) {
      if ((actual.major2 == -1) || ((uint8_t)actual.major2 < pfc_version_major2)) {
        return true;
      }

      if ((uint8_t)actual.major2 == pfc_version_major2) {
        if ((actual.minor == -1) || ((uint8_t)actual.minor < pfc_version_minor)) {
          return true;
        }

        if ((uint8_t)actual.minor == pfc_version_minor) {
          if ((actual.update == -1) || ((uint8_t)actual.update <= pfc_version_update)) {
            return true;
          }
        }
      }
    }
  }
  return false;
}


bool CapaModule::LoadCapabilityFile(unc_keytype_ctrtype_t ctrlr_type) {
  int         ret;
  pfc_conf_t  confp;
  pfc_cfblk_t cfblk;
  const char *version_name;
  const char *capa_file;

  if (ctrlr_type == UNC_CT_PFC) {
    pfc_log_info("\n\n *******CAPA**** UNC_CT_PFC type %d", ctrlr_type);
    capa_file = CAPA_CONF_FILE_PFC;
  /*
  } else if (ctrlr_type == UNC_CT_LEGACY) {
      pfc_log_info("\n \n ******CAPA**** UNC_CT_LEGACY type %d", ctrlr_type);
      capa_file = CAPA_CONF_FILE_LEGACY;
  */      
  } else if (ctrlr_type == UNC_CT_VNP) {
      pfc_log_info("\n \n *****CAPA**** UNC_CT_VNP type %d", ctrlr_type);
      capa_file = CAPA_CONF_FILE_VNP;
  } else if (ctrlr_type == UNC_CT_POLC) {
      pfc_log_info("\n \n *****CAPA**** UNC_CT_POLC type %d", ctrlr_type);
      capa_file = CAPA_CONF_FILE_POLC;
  } else if (ctrlr_type == UNC_CT_ODC) {
      pfc_log_info("\n \n *****CAPA**** UNC_CT_ODC type %d", ctrlr_type);
      capa_file = CAPA_CONF_FILE_ODC;
  } else {
     return false;
  }

  // std::string current_version;

  /* Lock capability database */
  ScopedReadWriteLock lock(capa_module_lock_, true);

  std::map<unc_keytype_ctrtype_t, CapaCtrlrCommon*>::iterator ctrlr_comm_it =
    ctrlr_common_map_.find(ctrlr_type);

  if (ctrlr_comm_it == ctrlr_common_map_.end()) {
    return false;
  }
  struct CapaCtrlrCommon *ccc = ctrlr_comm_it->second;

  /* Clear file */
  ccc->capa_map.clear();



  /* Open PFC Capablity Config file */
  ret = pfc_conf_open(&confp, capa_file, &ctrlr_capa_conf_defs);
  if (0 != ret) {
    pfc_log_error("Failed to open %s", capa_file);
    return false;
  }
  cfblk = pfc_conf_get_block(confp, "version_list");
  if (PFC_CFBLK_INVALID == cfblk) {
    pfc_log_error("Failed to read version list in %s", capa_file);
    return false;
  }
  pfc_log_trace("Opened %s", capa_file);

  int num_versions = pfc_conf_array_size(cfblk, "names");
  pfc_log_verbose("Number of version in version list are : %d", num_versions);

  // Load Parent Version Map for each controller version
  for (int index = 0; index < num_versions; index++) {
    version_name = pfc_conf_array_stringat(cfblk, "names", index, NULL);
    if (NULL == version_name) {
      pfc_log_warn("Version name in version_list block is empty");
      continue;
    }
    LoadParentVersion(confp, version_name);
  }

  ccc->actual_version_map_.clear();
  // Load KT capability for each controller version
  for (int index = 0; index < num_versions; index++) {
    /* Read version name from names filed in version list block */
    version_name = pfc_conf_array_stringat(cfblk, "names", index, NULL);
    if (NULL == version_name) {
      // pfc_log_warning("Version name in version_list block is empty");
      continue;
    }
    pfc_log_verbose("Loading capability for version: %s", version_name);
    // current_version = version_name;
    // TODO Why it is repeated LoadParentCapability(confp, version_name);
    CtrlrCapability *cap_ptr = new CtrlrCapability;
    if (false == cap_ptr->LoadCtrlrCapability(confp, version_name)) {
      pfc_log_error("Failed to load capability for %s", version_name);
      delete cap_ptr;
      return false;
    }
    ccc->capa_map[version_name] = cap_ptr;

    pfc_log_verbose("Loading Actual version: %s", version_name);
    std::list<ActualVersion> version_list;
    if (false == LoadActualVersion(confp, version_name, version_list)) {
      pfc_log_warn("Failed to load capability for %s", version_name);
      return false;
    }
    ccc->actual_version_map_[version_name] = version_list;
  }

  /* Close file */
  pfc_conf_close(confp);

  return true;
}


// Even if capability files are not loaded the module returns success.
// If file was not loaded, then this module assumes no feature is supported
// for that controller type

bool CapaModule::LoadCapabilityFiles(void) {
	LoadCapabilityFile(UNC_CT_PFC);
	// LoadCapabilityFile(UNC_CT_LEGACY);
	LoadCapabilityFile(UNC_CT_VNP);
	LoadCapabilityFile(UNC_CT_POLC);
	LoadCapabilityFile(UNC_CT_ODC);
	return true;
}

bool CapaModule::GetCreateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                     const std::string &version,
                                     unc_key_type_t keytype,
                                     uint32_t *instance_count,
                                     uint32_t *num_attrs,
                                     const uint8_t  **attrs) {
  ScopedReadWriteLock lock(capa_module_lock_, false);
  std::map<unc_keytype_ctrtype_t, CapaCtrlrCommon*>::iterator ctrlr_comm_it =
    ctrlr_common_map_.find(ctrlr_type);

  if (ctrlr_comm_it == ctrlr_common_map_.end()) {
    pfc_log_verbose("Bad ctrlr_type %d", ctrlr_type);
    return false;
  }
  struct CapaCtrlrCommon *ccc = ctrlr_comm_it->second;
  std::map<std::string, CtrlrCapability*>::iterator verit =
       ccc->capa_map.find(version);
  if (verit == ccc->capa_map.end()) {
    pfc_log_verbose("Version %s not found", version.c_str());
    return false;
  }
  CtrlrCapability* cap_ptr = verit->second;
  bool ret = cap_ptr->GetCreateCapability(keytype, instance_count,
                                          num_attrs, attrs);
  return ret;
}

bool CapaModule::GetUpdateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                     const std::string &version,
                                     unc_key_type_t keytype,
                                     uint32_t *num_attrs,
                                     const uint8_t  **attrs) {
  ScopedReadWriteLock lock(capa_module_lock_, false);

  std::map<unc_keytype_ctrtype_t, CapaCtrlrCommon*>::iterator ctrlr_comm_it =
    ctrlr_common_map_.find(ctrlr_type);
  if (ctrlr_comm_it == ctrlr_common_map_.end()) {
    pfc_log_verbose("Bad ctrlr_type %d", ctrlr_type);
    return false;
  }
  struct CapaCtrlrCommon *ccc = ctrlr_comm_it->second;
  std::map<std::string, CtrlrCapability*>::iterator verit =
      ccc->capa_map.find(version);
  if (verit == ccc->capa_map.end()) {
    return false;
  }
  CtrlrCapability* cap_ptr = verit->second;
  bool ret = cap_ptr->GetUpdateCapability(keytype, num_attrs, attrs);
  return ret;
}

bool CapaModule::GetReadCapability(unc_keytype_ctrtype_t ctrlr_type,
                                   const std::string &version,
                                   unc_key_type_t keytype,
                                   uint32_t *num_attrs,
                                   const uint8_t  **attrs) {
  ScopedReadWriteLock lock(capa_module_lock_, false);

  std::map<unc_keytype_ctrtype_t, CapaCtrlrCommon*>::iterator ctrlr_comm_it =
    ctrlr_common_map_.find(ctrlr_type);
  if (ctrlr_comm_it == ctrlr_common_map_.end()) {
    pfc_log_verbose("Bad ctrlr_type %d", ctrlr_type);
    return false;
  }
  struct CapaCtrlrCommon *ccc = ctrlr_comm_it->second;
  std::map<std::string, CtrlrCapability*>::iterator verit =
        ccc->capa_map.find(version);
  if (verit == ccc->capa_map.end()) {
     return false;
  }
  CtrlrCapability* cap_ptr = verit->second;
  bool ret = cap_ptr->GetReadCapability(keytype, num_attrs, attrs);
  return ret;
}

bool CapaModule::GetStateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                    const std::string &version,
                                    unc_key_type_t keytype,
                                    uint32_t *num_attrs,
                                    const uint8_t  **attrs) {
  ScopedReadWriteLock lock(capa_module_lock_, false);

  std::map<unc_keytype_ctrtype_t, CapaCtrlrCommon*>::iterator ctrlr_comm_it =
    ctrlr_common_map_.find(ctrlr_type);
  if (ctrlr_comm_it == ctrlr_common_map_.end()) {
    pfc_log_verbose("Bad ctrlr_type %d", ctrlr_type);
    return false;
  }
  struct CapaCtrlrCommon *ccc = ctrlr_comm_it->second;
  std::map<std::string, CtrlrCapability*>::iterator verit =
  ccc->capa_map.find(version);
  if (verit == (ccc->capa_map).end()) {
     return false;
  }
  CtrlrCapability* cap_ptr = verit->second;
  bool ret = cap_ptr->GetStateCapability(keytype, num_attrs, attrs);
  return ret;
}

bool CapaModule::GetInstanceCount(unc_keytype_ctrtype_t ctrlr_type,
                                  const std::string &version,
                                  unc_key_type_t keytype,
                                  uint32_t &instance_count) {
  ScopedReadWriteLock lock(capa_module_lock_, false);

  std::map<unc_keytype_ctrtype_t, CapaCtrlrCommon*>::iterator ctrlr_comm_it =
    ctrlr_common_map_.find(ctrlr_type);
  if (ctrlr_comm_it == ctrlr_common_map_.end()) {
    pfc_log_verbose("Bad ctrlr_type %d", ctrlr_type);
    return false;
  }
  struct CapaCtrlrCommon *ccc = ctrlr_comm_it->second;
  std::map<std::string, CtrlrCapability*>::iterator verit =
       (ccc->capa_map).find(version);
  if (verit == (ccc->capa_map).end()) {
     return false;
  }
  CtrlrCapability* cap_ptr = verit->second;
  bool ret = cap_ptr->GetCapability(keytype, instance_count);
  return ret;
}

std::string CapaModule::GetCtrlrParentVersion(const std::string &version) {
  std::string parent_version("");
  std::map<std::string, std::string>::iterator verit =
      parent_map_.find(version);
  if (verit == parent_map_.end()) {
    pfc_log_info("INFO: Parent Not Found");
    return parent_version;
  }
  parent_version = verit->second;
  return parent_version;
}


void CapaModule::VerboseDumpAll() {
  std::map<unc_keytype_ctrtype_t, CapaCtrlrCommon*>::iterator comm_ctrlr_it;
  ScopedReadWriteLock lock(capa_module_lock_, false);
  for (comm_ctrlr_it = ctrlr_common_map_.begin();
       comm_ctrlr_it != ctrlr_common_map_.end(); comm_ctrlr_it++) {
    unc_keytype_ctrtype_t ctrlr_type = comm_ctrlr_it->first;
    struct CapaCtrlrCommon *ccc = comm_ctrlr_it->second;
    std::map<std::string, CtrlrCapability*>::iterator verit;
    for (verit = ccc->capa_map.begin(); verit != ccc->capa_map.end(); verit++) {
      VerboseDump(ctrlr_type, verit->first);
    }
  }
}


void CapaModule::VerboseDump(unc_keytype_ctrtype_t ctrlr_type,
                             std::string version) {
  uint32_t instance_count = 0;
  uint32_t num_attrs = 0;
  const uint8_t *attrs = NULL;

  for (int index = 0; NULL != kt_map[index].kt_name; index++) {
    std::string capastr;
    unc_key_type_t keytype = kt_map[index].keytype;
    pfc_log_verbose(" Version : %s  --  Kt Name : %s", version.c_str(),
                  kt_map[index].kt_name);
    // ("----------------- CREATE -----------------");
    if (GetCreateCapability(ctrlr_type, version, keytype, &instance_count,
                            &num_attrs, &attrs)) {
      capastr = "create:   {";
      for (uint32_t index = 0; index < num_attrs; index++) {
        capastr += ((index > 0) ? ", " : " ");
        capastr += (attrs[index] ? 'T' : 'F');
      }
      capastr += " }";
      pfc_log_verbose("%s", capastr.c_str());
    }

    // ("----------------- UPDATE -----------------");
    if (GetUpdateCapability(ctrlr_type, version, keytype,
                            &num_attrs, &attrs)) {
      capastr = "update:   {";
      for (uint32_t index = 0; index < num_attrs; index++) {
        capastr += ((index > 0) ? ", " : " ");
        capastr += (attrs[index] ? 'T' : 'F');
      }
      capastr += " }";
      pfc_log_verbose("%s", capastr.c_str());
    }

    // ("----------------- READ -----------------");
    if (GetReadCapability(ctrlr_type, version, keytype,
                          &num_attrs, &attrs)) {
      capastr = "read:     {";
      for (uint32_t index = 0; index < num_attrs; index++) {
        capastr += ((index > 0) ? ", " : " ");
        capastr += (attrs[index] ? 'T' : 'F');
      }
      capastr += " }";
      pfc_log_verbose("%s", capastr.c_str());
    }

    // ("----------------- STATE -----------------");
    if (GetStateCapability(ctrlr_type, version, keytype,
                           &num_attrs, &attrs)) {
      capastr = "stateread: {";
      for (uint32_t index = 0; index < num_attrs; index++) {
        capastr += ((index > 0) ? ", " : " ");
        capastr += (attrs[index] ? 'T' : 'F');
      }
      capastr += " }";
      pfc_log_verbose("%s", capastr.c_str());
    }
  }
}

bool CapaModule::GetSupportedVersion(unc_keytype_ctrtype_t ctrlr_type,
                                 std::string config_version,
                                 uint8_t* version_major1,
                                 uint8_t* version_major2,
                                 uint8_t* version_minor,
                                 uint8_t* version_update) {
  pfc_log_info("Fetches th actual version from configured version");

  struct CapaCtrlrCommon    *ccc = ctrlr_common_map_.find(ctrlr_type)->second;
  std::list<ActualVersion>  actual_version_list;
  ActualVersion             actual;
  std::map<std::string, std::list<ActualVersion> >::iterator act_ver_it;

  if (ccc == NULL) {
    pfc_log_warn("Failed to find Capa controller common");
    return false;
  }
  memset(&actual, 0, sizeof(ActualVersion));

  act_ver_it = ccc->actual_version_map_.find(config_version);
  if (act_ver_it == ccc->actual_version_map_.end()) {
    pfc_log_debug("Configuration version is not a member of "
                  "actual version map");
    return false;
  }

  actual_version_list =  act_ver_it->second;

  actual = actual_version_list.front();

  *version_major1 = actual.major1;
  *version_major2 = actual.major2;
  *version_minor  = actual.minor;
  *version_update  = actual.update;

  return true;
}
                                                                       // NOLINT
}  // capa
}  // unc

PFC_MODULE_DECL(unc::capa::CapaModule);

