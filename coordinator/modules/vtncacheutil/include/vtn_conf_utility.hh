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
   * @brief       : This method returns the Keytype for UNC_KT_VTN
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vtn_t&  key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VTN;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VTN
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vtn_t& key,
                                    const val_vtn_t& val) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>(&key.vtn_name));
    return key_str;
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VTN
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vtn_t&  key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>(&key.vtn_name));
    return key_str;
  }
  /**
   * @brief       : This method returns the parent Key for UNC_KT_VTN
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_parent_key(const key_vtn_t&  key) {
    ODC_FUNC_TRACE;
    return "ROOT";
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VBRIDGE
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vbr_t&  key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VBRIDGE;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VBRIDGE
   * @param [in]  : key
   * @retval      : string
   **/
  static std::string get_search_key(const key_vbr_t& key,
                                    const val_vbr_t& val) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.vbridge_name));
    return key_str;
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VBRIDGE
   * @param [in]  : key
   * @retval      : string
   **/
  static std::string get_key(const key_vbr_t&  key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.vbridge_name));
    return key_str;
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VBRIDGE
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_parent_key(const key_vbr_t&  key) {
    ODC_FUNC_TRACE;
    std::string key_str = reinterpret_cast<const char*>
        (key.vtn_key.vtn_name);
    pfc_log_debug("Exiting Function %s.. vbr parentkey: %s", PFC_FUNCNAME,
                  key_str.c_str());
    return key_str;
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VBR_IF
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vbr_if_t&  key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VBR_IF;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VBR_IF
   * @param [in]  : key
   * @retval      : search key - string
   */
  static std::string get_search_key(const key_vbr_if_t& key,
                                    const pfcdrv_val_vbr_if_t& val) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.vbr_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.vbr_key.vbridge_name));
    key_str += std::string(reinterpret_cast<const char*>(key.if_name));
    return key_str;
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VBR_IF
   * @param [in]  : key
   * @retval      : search key - string
   */
  static std::string get_key(const key_vbr_if_t&  key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.if_name));
    return key_str;
  }
  /**
   * @brief       : This method returns the parent Key for UNC_KT_VBR_IF
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vbr_if_t&  key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.vbr_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.vbr_key.vbridge_name));
    return key_str;
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VBR_VLANMAP
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vlan_map_t&  key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VBR_VLANMAP;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VBR_VLANMAP
   * @param [in]  : key
   * @retval      : search key - string
   */
  static std::string get_search_key(const key_vlan_map_t& key,
                                    const val_vlan_map_t& val) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.vbr_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.vbr_key.vbridge_name));
    if (1 == key.logical_port_id_valid) {
     key_str += std::string(reinterpret_cast<const char*>
                            (key.logical_port_id));
    } else {
     key_str += std::string("ANY.");
     char buff[10];
     snprintf(buff, sizeof(buff), "%u", val.vlan_id);
     key_str += std::string(buff);
    }
    return key_str;
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VBR_VLANMAP
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vlan_map_t&  key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.vbr_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.vbr_key.vbridge_name));
    return key_str;
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VBR_VLANMAP
   * @param [in]  : key
   * @retval      : search key - string
   */
  static std::string get_key(const key_vlan_map_t&  key) {
    ODC_FUNC_TRACE;
    std::string key_str = "";
    if (1 == key.logical_port_id_valid) {
      std::string key_str(reinterpret_cast<const char*>
                       (key.logical_port_id));
    } else {
      std::string key_str("ANY");
    }
    return key_str;
  }
};
}  // namespace vtndrvcache
}  // namespace unc
#endif
