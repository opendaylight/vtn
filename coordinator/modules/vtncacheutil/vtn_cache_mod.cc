/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include <vtn_cache_mod.hh>

namespace unc {
namespace vtndrvcache {

/**
 ** @brief : VtnDrvCacheMod Constructor
 **/
VtnDrvCacheMod::VtnDrvCacheMod(const pfc_modattr_t *mattr):
    pfc::core::Module(mattr) {
      keyTypeStrMap.clear();
}

/**
 ** @brief : Call init function to initialize module
 **/
pfc_bool_t  VtnDrvCacheMod::init() {
  // Intialize keyTypeStrMap Map
  keyTypeStrMap.clear();
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_ROOT, "UNC_KT_ROOT"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VTN, "UNC_KT_VTN"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VBRIDGE, "UNC_KT_VBRIDGE"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VBR_IF, "UNC_KT_VBR_IF"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VBR_VLANMAP, "UNC_KT_VBR_VLANMAP"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_SWITCH, "UNC_KT_SWITCH"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_PORT, "UNC_KT_PORT"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_LINK, "UNC_KT_LINK"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_FLOWLIST, "UNC_KT_FLOWLIST"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_FLOWLIST_ENTRY, "UNC_KT_FLOWLIST_ENTRY"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VTN_FLOWFILTER, "UNC_KT_VTN_FLOWFILTER"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VTN_FLOWFILTER_ENTRY, "UNC_KT_VTN_FLOWFILTER_ENTRY"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VBR_FLOWFILTER, "UNC_KT_VBR_FLOWFILTER"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VBR_FLOWFILTER_ENTRY, "UNC_KT_VBR_FLOWFILTER_ENTRY"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VBRIF_FLOWFILTER, "UNC_KT_VBRIF_FLOWFILTER"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VBRIF_FLOWFILTER_ENTRY, "UNC_KT_VBRIF_FLOWFILTER_ENTRY"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VTERMINAL, "UNC_KT_VTERMINAL"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VTERM_IF, "UNC_KT_VTERM_IF"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VTERMIF_FLOWFILTER, "UNC_KT_VTERMIF_FLOWFILTER"));
  keyTypeStrMap.insert(std::pair<unc_key_type_t, std::string>(
          UNC_KT_VTERMIF_FLOWFILTER_ENTRY, "UNC_KT_VTERMIF_FLOWFILTER_ENTRY"));

  return PFC_TRUE;
}

// Static variable keyTypeStrMap
std::map<unc_key_type_t, std::string> VtnDrvCacheMod::keyTypeStrMap;

/**
 ** @brief : Call fini function to release module
 **/
pfc_bool_t  VtnDrvCacheMod::fini() {
  keyTypeStrMap.clear();
  return PFC_TRUE;
}
}  // namespace vtndrvcache
}  // namespace unc
PFC_MODULE_IPC_DECL(unc::vtndrvcache::VtnDrvCacheMod, 0);
