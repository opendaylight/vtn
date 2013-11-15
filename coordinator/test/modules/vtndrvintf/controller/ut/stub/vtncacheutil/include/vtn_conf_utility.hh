/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __VTN_CONF_UTILTY_HH__
#define __VTN_CONF_UTILTY_HH__

#include <pfc/ipc_struct.h>
#include <unc/keytype.h>
#include <string>

namespace unc {
namespace vtndrvcache {
class  ConfUtil {
 public:
  /**
   ** This method returns the Keytype for UNC_KT_VTN
   ** @param [out] - key_Type
   **/
  static unc_key_type_t  get_key_type(key_vtn_t&  key) {
    pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
    pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
    return UNC_KT_VTN;
  }

  /**
   ** This method returns the search Key for UNC_KT_VTN
   ** @param [out] - search key - string
   **/
  static std::string get_search_key(key_vtn_t&  key) {
    pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
    std::string key_str(reinterpret_cast<const char*>(key.vtn_name));
    pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
    return key_str;
  }

  /**
   ** This method returns the parent Key for UNC_KT_VTN
   ** @param [out] - parent key - string
   **/
  static std::string get_parent_key(key_vtn_t&  key) {
    pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
    pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
    return "ROOT";
  }

  /**
   ** This method returns the Keytype for UNC_KT_VBRIDGE
   ** @param [out] - key_Type
   **/
  static unc_key_type_t  get_key_type(key_vbr_t&  key) {
    pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
    pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
    return UNC_KT_VBRIDGE;
  }

  /**
   ** This method returns the search Key for UNC_KT_VBRIDGE
   ** @param [out] - search key - string
   **/
  static std::string get_search_key(key_vbr_t&  key) {
    pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
    std::string key_str(reinterpret_cast<const char*>
                        (key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.vbridge_name));
    pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
    return key_str;
  }
  /**
   ** This method returns the parent Key for UNC_KT_VBRIDGE
   ** @param [out] - parent key - string
   **/

  static std::string get_parent_key(key_vbr_t&  key) {
    pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
    pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
    return std::string(reinterpret_cast<const char*>
                       (key.vtn_key.vtn_name));
  }

  /**
   ** This method returns the Keytype for UNC_KT_VBR_IF
   ** @param [out] - key_Type
   **/
  static unc_key_type_t  get_key_type(key_vbr_if_t&  key) {
    pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
    pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
    return UNC_KT_VBR_IF;
  }

  /**
   ** This method returns the search Key for UNC_KT_VBR_IF
   ** @param [out] - search key - string
   **/
  static std::string get_search_key(key_vbr_if_t&  key) {
    pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
    std::string key_str(reinterpret_cast<const char*>
                        (key.vbr_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.vbr_key.vbridge_name));
    key_str += std::string(reinterpret_cast<const char*>(key.if_name));
    pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
    return key_str;
  }


  /**
   ** This method returns the parent Key for UNC_KT_VBR_IF
   ** @param [out] - parent key - string
   **/
  static std::string get_parent_key(key_vbr_if_t&  key) {
    pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
    std::string key_str(reinterpret_cast<const char*>
                        (key.vbr_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.vbr_key.vbridge_name));
    pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
    return key_str;
  }
};
}  // namespace vtndrvcache
}  // namespace unc


#endif
