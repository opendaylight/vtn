/*
 * Copyright (c) 2013-2014 NEC Corporation
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
  static unc_key_type_t  get_key_type(const key_vtn_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VTN;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VTN
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vtn_t &key,
                                    const val_vtn_t &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VTN
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vtn_t &key) {
    ODC_FUNC_TRACE;
    return (reinterpret_cast<const char*>(&key.vtn_name));
  }
  /**
   * @brief       : This method returns the parent Key for UNC_KT_VTN
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_parent_key(const key_vtn_t &key) {
    ODC_FUNC_TRACE;
    return "ROOT";
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VBRIDGE
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vbr_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VBRIDGE;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VBRIDGE
   * @param [in]  : key
   * @retval      : string
   **/
  static std::string get_search_key(const key_vbr_t &key,
                                    const val_vbr_t &val) {
    ODC_FUNC_TRACE;
    return (get_key(key.vtn_key) + (get_key(key)));
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VBRIDGE
   * @param [in]  : key
   * @retval      : string
   **/
  static std::string get_key(const key_vbr_t &key) {
    ODC_FUNC_TRACE;
    return (reinterpret_cast<const char*>(key.vbridge_name));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VBRIDGE
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_parent_key(const key_vbr_t &key) {
    ODC_FUNC_TRACE;
    return (reinterpret_cast<const char*>(key.vtn_key.vtn_name));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VBR_IF
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vbr_if_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VBR_IF;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VBR_IF
   * @param [in]  : key
   * @retval      : search key - string
   */
  static std::string get_search_key(const key_vbr_if_t &key,
                                    const pfcdrv_val_vbr_if_t &val) {
    ODC_FUNC_TRACE;
    return (get_parent_key(key) + get_key(key));
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VBR_IF
   * @param [in]  : key
   * @retval      : search key - string
   */
  static std::string get_key(const key_vbr_if_t &key) {
    ODC_FUNC_TRACE;
    return (reinterpret_cast<const char*>(key.if_name));
  }
  /**
   * @brief       : This method returns the parent Key for UNC_KT_VBR_IF
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vbr_if_t &key) {
    ODC_FUNC_TRACE;
    return (get_key(key.vbr_key.vtn_key) + get_key(key.vbr_key));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VBR_VLANMAP
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vlan_map_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VBR_VLANMAP;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VBR_VLANMAP
   * @param [in]  : key
   * @retval      : search key - string
   */
  static std::string get_search_key(const key_vlan_map_t &key,
                                    const pfcdrv_val_vlan_map_t &val) {
    ODC_FUNC_TRACE;
    std::string key_str = get_parent_key(key);
    if (1 == key.logical_port_id_valid) {
     key_str += std::string(reinterpret_cast<const char*>
                            (key.logical_port_id));
    } else {
     key_str += std::string("ANY.");
     char buff[10];
     snprintf(buff, sizeof(buff), "%u", val.vm.vlan_id);
     key_str += std::string(buff);
    }
    return key_str;
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VBR_VLANMAP
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vlan_map_t &key) {
    ODC_FUNC_TRACE;
    return (get_key(key.vbr_key.vtn_key) + get_key(key.vbr_key));
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VBR_VLANMAP
   * @param [in]  : key
   * @retval      : search key - string
   */
  static std::string get_key(const key_vlan_map_t &key) {
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

  /**
   * @brief       : This method returns the Keytype for UNC_KT_SWITCH
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_switch_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_SWITCH;
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_SWITCH
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_switch_t &key) {
    ODC_FUNC_TRACE;
    return "ROOT";
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_SWITCH
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_switch_t &key,
                                    const val_switch_st &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_SWITCH
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_switch_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.ctr_key.controller_name));
    return (key_str + (reinterpret_cast<const char*>(key.switch_id)));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_PORT
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_port_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_PORT;
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_PORT
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_port_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.sw_key.ctr_key.controller_name));
    return (key_str + (reinterpret_cast<const char*>(key.sw_key.switch_id)));
  }


  /**
   * @brief       : This method returns the search Key for UNC_KT_PORT
   * @param [in]  : key
   * @retval      : string
   **/
  static std::string get_search_key(const key_port_t &key,
                                    const val_port_st_t &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_PORT
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_port_t &key) {
    ODC_FUNC_TRACE;
    return (get_parent_key(key) +
           (reinterpret_cast<const char*>(key.port_id)));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_LINK
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_link_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_LINK;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_LINK
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_link_t &key,
                                    const val_link_st_t &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_LINK
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_link_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.ctr_key.controller_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.switch_id1));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.port_id1));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.switch_id2));
    return (key_str + (reinterpret_cast<const char*>(key.port_id2)));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_LINK
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_link_t &key) {
    ODC_FUNC_TRACE;
    return "ROOT";
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_FLOWLIST
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_flowlist_t  &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_FLOWLIST;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_FLOWLIST
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_flowlist_t &key,
                                    const val_flowlist_t &val) {
    ODC_FUNC_TRACE;
    return (reinterpret_cast<const char*>(key.flowlist_name));
  }

  /**
   * @brief       : This method returns the name for UNC_KT_FLOWLIST
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_flowlist_t &key) {
    ODC_FUNC_TRACE;
    return (reinterpret_cast<const char*>(key.flowlist_name));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_FLOWLIST
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_flowlist_t &key) {
    ODC_FUNC_TRACE;
    return "ROOT";
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_FLOWLIST_ENTRY
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_flowlist_entry_t  &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_FLOWLIST_ENTRY;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_FLOWLIST_ENTRY
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_flowlist_entry_t &key,
                                    const val_flowlist_entry_t &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_FLOWLIST_ENTRY
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_flowlist_entry_t &key) {
    ODC_FUNC_TRACE;
    char buff[10];
    snprintf(buff, sizeof(buff), "%u", key.sequence_num);
    return (reinterpret_cast<const char*>(key.flowlist_key.flowlist_name)
            + std::string(buff));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_FLOWLIST_ENTRY
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_flowlist_entry_t &key) {
    ODC_FUNC_TRACE;
    return reinterpret_cast<const char*>(key.flowlist_key.flowlist_name);
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VTN_FLOWFILTER
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vtn_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VTN_FLOWFILTER;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VTN_FLOWFILTER
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vtn_flowfilter_t &key,
                                    const val_flowfilter_t &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VTN_FLOWFILTER
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vtn_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    char buff[10];
    snprintf(buff, sizeof(buff), "%u", key.input_direction);
    return (get_key(key.vtn_key) + std::string(buff));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VTN_FLOWFILTER
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vtn_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    return (reinterpret_cast<const char*>(key.vtn_key.vtn_name));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VTN_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vtn_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VTN_FLOWFILTER_ENTRY;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VTN_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vtn_flowfilter_entry_t &key,
                                    const val_vtn_flowfilter_entry_t &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VTN_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vtn_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%u", key.sequence_num);
    pfc_log_trace("sequence number for vtnflowfilter entry : %s", buffer);
    return (get_parent_key(key) + std::string(buffer));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VTN_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vtn_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    char buff[10];
    snprintf(buff, sizeof(buff), "%u", key.flowfilter_key.input_direction);
    return (reinterpret_cast<const char*>(key.flowfilter_key.vtn_key.vtn_name)
            + std::string(buff));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VBR_FLOWFILTER
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vbr_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VBR_FLOWFILTER;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VBR_FLOWFILTER
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vbr_flowfilter_t &key,
                                    const val_flowfilter_t  &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VBR_FLOWFILTER
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vbr_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    char buff[10];
    snprintf(buff, sizeof(buff), "%u", key.direction);
    pfc_log_trace("direction for vtnflowfilter entry : %s", buff);
    return (get_parent_key(key) + std::string(buff));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VBR_FLOWFILTER
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vbr_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str = reinterpret_cast<const char*>
                        (key.vbr_key.vtn_key.vtn_name);
    return (key_str + reinterpret_cast<const char*>(key.vbr_key.vbridge_name));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VBR_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vbr_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VBR_FLOWFILTER_ENTRY;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VBR_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vbr_flowfilter_entry_t &key,
                                    const val_flowfilter_entry_t  &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VBR_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vbr_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    char buff[10];
    snprintf(buff, sizeof(buff), "%u", key.sequence_num);
    pfc_log_trace("sequence for vtnflowfilter entry : %s", buff);
    return (get_parent_key(key) + std::string(buff));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VBR_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vbr_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.flowfilter_key.vbr_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.flowfilter_key.vbr_key.vbridge_name));
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%u", key.flowfilter_key.direction);
    return (key_str + std::string(buffer));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VBRIF_FLOWFILTER
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vbr_if_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VBRIF_FLOWFILTER;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VBRIF_FLOWFILTER
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vbr_if_flowfilter_t &key,
                                    const pfcdrv_val_vbrif_vextif_t  &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VBRIF_FLOWFILTER
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vbr_if_flowfilter_t  &key) {
    ODC_FUNC_TRACE;
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%u", key.direction);
    return (get_parent_key(key) + std::string(buffer));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VBRIF_FLOWFILTER
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vbr_if_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.if_key.vbr_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.if_key.vbr_key.vbridge_name));
    return (key_str + reinterpret_cast<const char*>(key.if_key.if_name));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VBRIF_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t get_key_type(const key_vbr_if_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VBRIF_FLOWFILTER_ENTRY;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VBRIF_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vbr_if_flowfilter_entry_t &key,
                                    const pfcdrv_val_flowfilter_entry_t  &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VBRIF_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vbr_if_flowfilter_entry_t  &key) {
    ODC_FUNC_TRACE;
    char buff[10];
    snprintf(buff, sizeof(buff), "%u", key.sequence_num);
    pfc_log_trace("sequence number for vbriflow-filter entry : %s", buff);
    return (get_parent_key(key) + std::string(buff));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VBRIF_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vbr_if_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.flowfilter_key.if_key.vbr_key.vbridge_name));
    key_str += std::string(reinterpret_cast<const char*>
                           (key.flowfilter_key.if_key.if_name));
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%u", key.flowfilter_key.direction);
    return (key_str + std::string(buffer));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VTERMINAL
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vterm_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VTERMINAL;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VTERMINAL
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vterm_t &key,
                                    const val_vterm_t  &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VTERMINAL
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vterm_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.vtn_key.vtn_name));
    return (key_str + reinterpret_cast<const char*>(key.vterminal_name));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VTERMINAL
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vterm_t &key) {
    ODC_FUNC_TRACE;
    return (reinterpret_cast<const char*>(key.vtn_key.vtn_name));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VTERM_IF
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vterm_if_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VTERM_IF;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VTERM_IF
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vterm_if_t &key,
                                    const val_vterm_if_t  &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VTERM_IF
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vterm_if_t &key) {
    ODC_FUNC_TRACE;
    return (get_parent_key(key) + (reinterpret_cast<const char*>(key.if_name)));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VTERM_IF
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vterm_if_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.vterm_key.vtn_key.vtn_name));
    return (key_str + reinterpret_cast<const char*>
                       (key.vterm_key.vterminal_name));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VTERMIF_FLOWFILTER
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t  get_key_type(const key_vterm_if_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VTERMIF_FLOWFILTER;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VTERMIF_FLOWFILTER
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vterm_if_flowfilter_t &key,
                                    const val_flowfilter_t &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VTERMIF_FLOWFILTER
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vterm_if_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%u", key.direction);
    return (get_parent_key(key) + std::string(buffer));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VTERMIF_FLOWFILTER
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(const key_vterm_if_flowfilter_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.if_key.vterm_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                        (key.if_key.vterm_key.vterminal_name));
    return (key_str + reinterpret_cast<const char*>(key.if_key.if_name));
  }

  /**
   * @brief       : This method returns the Keytype for UNC_KT_VTERMIF_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : unc_key_type_t
   */
  static unc_key_type_t get_key_type(
      const key_vterm_if_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    return UNC_KT_VTERMIF_FLOWFILTER_ENTRY;
  }

  /**
   * @brief       : This method returns the search Key for UNC_KT_VTERMIF_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_search_key(const key_vterm_if_flowfilter_entry_t &key,
                                    const val_flowfilter_entry_t &val) {
    ODC_FUNC_TRACE;
    return get_key(key);
  }

  /**
   * @brief       : This method returns the name for UNC_KT_VTERMIF_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : string
   */
  static std::string get_key(const key_vterm_if_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    char buff[10];
    snprintf(buff, sizeof(buff), "%u", key.sequence_num);
    return ((get_parent_key(key)) + (std::string(buff)));
  }

  /**
   * @brief       : This method returns the parent Key for UNC_KT_VTERMIF_FLOWFILTER_ENTRY
   * @param [in]  : key
   * @retval      : parent key - string
   */
  static std::string get_parent_key(
      const key_vterm_if_flowfilter_entry_t &key) {
    ODC_FUNC_TRACE;
    std::string key_str(reinterpret_cast<const char*>
                        (key.flowfilter_key.if_key.vterm_key.vtn_key.vtn_name));
    key_str += std::string(reinterpret_cast<const char*>
                        (key.flowfilter_key.if_key.vterm_key.vterminal_name));
    key_str += std::string(reinterpret_cast<const char*>
                        (key.flowfilter_key.if_key.if_name));
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%u", key.flowfilter_key.direction);
    return (key_str + std::string(buffer));
  }
};
}  // namespace vtndrvcache
}  // namespace unc
#endif
