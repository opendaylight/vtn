/*
 * Copyright (c) 2012-2014 NEC Corporation
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
#include <list>

#include "pfcxx/module.hh"
#include "pfcxx/synch.hh"
#include "unc/config.h"
#include "unc/keytype.h"
#include "ctrlr_capability.hh"
#include "capa_intf.hh"

extern pfc_cfdef_t  ctrlr_capa_conf_defs;

namespace unc {
namespace capa {

#define CAPA_CONF_DIR UNC_SYSCONFDIR "/capa"

#define CAPA_CONF_FILE_PFC      CAPA_CONF_DIR "/pfc_capa.conf"
// #define CAPA_CONF_FILE_LEGACY   CAPA_CONF_DIR "/legacy_capa.conf"
#define CAPA_CONF_FILE_VNP      CAPA_CONF_DIR "/vnp_capa.conf"
#define CAPA_CONF_FILE_POLC      CAPA_CONF_DIR "/polc_capa.conf"
#define CAPA_CONF_FILE_ODC      CAPA_CONF_DIR "/odc_capa.conf"

class CapaModule : public pfc::core::Module, CapaIntf {
 public:
  /**
   * @brief  Constructor.
   */
  explicit CapaModule(const pfc_modattr_t *attr);

  /**
   * @brief  Destructor.
   */
  ~CapaModule();

  /**
   *  @brief  Initialize capa module.
   *  @retval PFC_TRUE   Successful completion.
   *  @retval PFC_FALSE  Failure occurred.
   */
  pfc_bool_t init();

  /**
   * @brief  Finalize capa module.
   * @retval  This function always returns PFC_TRUE.
   */
  pfc_bool_t fini(void);

  /**
   * @brief Get the capability  mananger instance.
   * @return NULL or pointer to capability  manager
   */
  static CapaModule* get_capability_mgr();

  /**
   * @brief  Load Capability information from configuation file into memory
   * 
   * @retval true  Loaded Successful
   * @retval false Failure occured
   */
  bool  LoadCapabilityFiles();

  /**
   * @brief  Return parent controller version of `version'.
   *
   * @param[in] version  Controller version.
   *
   * @retval Empty string  Parent version is not found.
   * @retval other         Successful completion
   */
  std::string GetCtrlrParentVersion(const std::string &version);

  /**
   * @brief  Return instance count of specified key type.
   *
   * @param[in] ctrlr_type  controller name.
   * @param[in] version     Controller version.
   * @param[in] keytype     key type
   * @param[out] instance_count  Instance count for specified keytype.
   *
   * @retval true   Successful
   * @retval false  keytype is not found for the given ctrlr type
   */
  bool  GetInstanceCount(unc_keytype_ctrtype_t ctrlr_type, const std::string &version,
                         unc_key_type_t keytype, uint32_t &instance_count);


  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  keytype    Key type.
   * @param[in]  version    controller version
   * @param[out] num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * 
   * @retval true   Successful
   * @retval false  keytype is  not found for the given ctrlr type
   */

  bool GetCreateCapability(unc_keytype_ctrtype_t ctrlr_type, const std::string &version,
                           unc_key_type_t keytype,
                           uint32_t *instnace_count,
                           uint32_t *num_attrs, const uint8_t  **attrs);

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  keytype    Key type.
   * @param[in]  version    controller version
   * @param[out] num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * 
   * @retval true   Successful
   * @retval false  keytype is  not found for the given ctrlr type
   */

  bool GetUpdateCapability(unc_keytype_ctrtype_t ctrlr_type, const std::string &version,
                           unc_key_type_t keytype,
                           uint32_t *num_attrs, const uint8_t  **attrs);

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  keytype    Key type.
   * @param[in]  version    controller version
   * @param[out] num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * 
   * @retval true   Successful
   * @retval false  keytype is  not found for the given ctrlr type
   */

  bool GetReadCapability(unc_keytype_ctrtype_t ctrlr_type, const std::string &version,
                         unc_key_type_t keytype,
                         uint32_t *num_attrs, const uint8_t  **attrs);

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  keytype    Key type.
   * @param[in]  version    controller version
   * @param[out] num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * 
   * @retval true   Successful
   * @retval false  keytype is  not found for the given ctrlr type
   */

  bool GetStateCapability(unc_keytype_ctrtype_t ctrlr_type, const std::string &version,
                          unc_key_type_t keytype,
                          uint32_t *num_attrs, const uint8_t  **attrs);


  void VerboseDumpAll();
  void VerboseDump(unc_keytype_ctrtype_t ctrlr_type, std::string version);

  /*
   * @brief Validates configuration version and actual version of specified
   * controller type
   *
   * @param[in] ctrlr_type       controller type
   * @param[in] config_version   configuration version
   * @param[in] pfc_version      actual version
   *
   * @retval true   Successful
   * @retval false  validation of configuration version and actual version.
   *
   * */
  bool ValidateVersion(unc_keytype_ctrtype_t ctrlr_type,
                       std::string config_version, uint8_t pfc_version_major1,
                       uint8_t pfc_version_major2, uint8_t pfc_version_minor,
                       uint8_t pfc_version_update);

bool GetSupportedVersion(unc_keytype_ctrtype_t ctrlr_type,
                                 std::string config_version,
                                 uint8_t* version_major1,
                                 uint8_t* version_major2,
                                 uint8_t* version_minor,
                                 uint8_t* version_update);

 private:
  struct ActualVersion {
    int32_t major1;
    int32_t major2;
    int32_t minor;
    int32_t update;
  };

  bool LoadCapabilityFile(unc_keytype_ctrtype_t ctrlr_type);

  void LoadParentVersion(pfc_conf_t confp, const std::string version);

  bool LoadActualVersion(pfc_conf_t confp, const std::string version,
                         std::list<ActualVersion> &version_list);

  /**
   * @brief Capability Manager Instance.
   */
  static CapaModule* capability_mgr_;

  /**
   * @brief  A map which stores pairs of version string and parent version .
   */
  std::map<std::string, std::string> parent_map_;

  struct CapaCtrlrCommon {
    std::map<std::string, CtrlrCapability*> capa_map;
    std::map<std::string, std::list<ActualVersion> > actual_version_map_;
  };

  std::map<unc_keytype_ctrtype_t, CapaCtrlrCommon*> ctrlr_common_map_;
  pfc::core::ReadWriteLock capa_module_lock_;

  class ScopedReadWriteLock {
   public:
    explicit ScopedReadWriteLock(pfc::core::ReadWriteLock &lock,
                                 bool take_write_lock) {
      lock_ = &lock;
      (take_write_lock) ? lock_->wrlock() : lock_->rdlock();
    }
    ~ScopedReadWriteLock() {
      lock_->unlock();
    }
   private:
    pfc::core::ReadWriteLock *lock_;
  };

};
// NOLINT


} /* namespace capctrl */
} /* namespace unc */

#endif  // CAPABILITY_MGR_HH_

