/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the
 * terms of the Eclipse Public License v1.0 which
 * accompanies this
 * distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_root.hh>

namespace unc {
namespace odcdriver {

// Read all vtns from Controller
drv_resp_code_t ODCROOTCommand::read_root_child(
    std::vector<unc::vtndrvcache::ConfigNode*> &cfg_ptr,
    unc::driver::controller* ctrl_ptr) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  ODCVTNCommand vtn_obj;
  drv_resp_code_t ret_code = DRVAPI_RESPONSE_FAILURE;
  ret_code = vtn_obj.read_all(ctrl_ptr, cfg_ptr);
  if (ret_code == DRVAPI_RESPONSE_NO_SUCH_INSTANCE) {
    pfc_log_debug("vtn_obj.read_all no object in ctl");
    return DRVAPI_RESPONSE_NO_SUCH_INSTANCE;
  }
  std::vector<unc::vtndrvcache::ConfigNode*> ::iterator it_begin =
      cfg_ptr.begin();
  std::vector<unc::vtndrvcache::ConfigNode*> ::iterator it_last =
      cfg_ptr.end();
  for (; it_begin != it_last; it_begin++) {
    unc::vtndrvcache::ConfigNode* ptr =
        static_cast<unc::vtndrvcache::ConfigNode*>(*it_begin);
    pfc_log_debug("odcdriver:read_root_child-get_key:%s ",
                  ptr->get_key().c_str());
  }
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return ret_code;
}

// Read all childs from vtns
drv_resp_code_t
ODCROOTCommand::read_all_child(unc::vtndrvcache::ConfigNode* cfg_ptr,
                               std::vector< unc::vtndrvcache
                               ::ConfigNode*>
                               &child_list,
                               unc::driver::controller* ctrl_ptr) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  unc_key_type_t keytype = cfg_ptr->get_type();
  pfc_log_debug("read_all_child for key Type:%d", keytype);
  char* vtn_name = NULL;
  char* vbr_name = NULL;
  drv_resp_code_t ret_code = DRVAPI_RESPONSE_FAILURE;

  switch (keytype) {
    case UNC_KT_VTN: {
      unc::vtndrvcache::CacheElementUtil <key_vtn_t, val_vtn_t, uint32_t>
          * cache_util_ptr = dynamic_cast <unc::vtndrvcache::CacheElementUtil
          <key_vtn_t, val_vtn_t, uint32_t> * >(cfg_ptr);
      if (cache_util_ptr == NULL) {
        pfc_log_error(" %s VTN. Exiting Function" , PFC_FUNCNAME);
        return ret_code;
      }
      ODCVTNCommand vtn_obj;
      key_vtn_t *key_struct = cache_util_ptr->getkey();
      if (NULL == key_struct) {
        return ret_code;
      }
      vtn_name = reinterpret_cast<char*>(key_struct->vtn_name);
      pfc_log_debug("name %s", vtn_name);
      ret_code = vtn_obj.get_vtn_child(vtn_name, ctrl_ptr, child_list);
      pfc_log_debug("VTN child_list... zise: %d", static_cast<int>
                    (child_list.size()));
      break;
    }

    case UNC_KT_VBRIDGE: {
      unc::vtndrvcache::CacheElementUtil <key_vbr_t, val_vbr_t, uint32_t>
          * cache_util_ptr = dynamic_cast <unc::vtndrvcache::CacheElementUtil
          <key_vbr_t, val_vbr_t, uint32_t> * >(cfg_ptr);
      ODCVBRCommand vbr_obj;
      if (cache_util_ptr == NULL) {
        pfc_log_error(" %s VBR. Exiting Function" , PFC_FUNCNAME);
        return ret_code;
      }
      key_vbr_t *key_struct = cache_util_ptr->getkey();
      if (NULL == key_struct) {
        return ret_code;
      }
      vtn_name = reinterpret_cast<char*> (key_struct->vtn_key.vtn_name);
      vbr_name = reinterpret_cast<char*> (key_struct->vbridge_name);
      ret_code = vbr_obj.get_vbr_child(vtn_name, vbr_name, ctrl_ptr,
                                       child_list);
      pfc_log_debug("VBR child_list... zise: %d", static_cast<int>
                    (child_list.size()));
      break;
    }

    case UNC_KT_VBR_IF: {
      //  no child for vbrif
      break;
    }

    default:
      ret_code = DRVAPI_RESPONSE_FAILURE;
      break;
  };
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return ret_code;
}
}  //  namespace odcdriver
}  //  namespace unc
